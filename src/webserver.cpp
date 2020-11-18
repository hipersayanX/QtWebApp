/* QtWebApp, webapp made with Qt.
 * Copyright (C) 2019  Gonzalo Exequiel Pedone
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QBuffer>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMimeDatabase>
#include <QTcpSocket>
#include <QTimer>
#include <QTranslator>
#include <QUrl>
#include <QUrlQuery>
#include <QWebChannel>
#include <random>

#include "webserver.h"
#include "socketserver.h"

struct RGBA
{
    quint8 r;
    quint8 g;
    quint8 b;
    quint8 a;
};

struct Message
{
    QString method;
    QString path;
    QString protocol;
    QMap<QString, QString> headers;
    QByteArray data;
};

class WebServerPrivate
{
    public:
        WebServer *self {nullptr};
        SocketServer m_socketServer;
        QTranslator *m_translator {nullptr};
        QTimer m_timer;

        explicit WebServerPrivate(WebServer *self);
        QByteArray readFrame(int width,
                             int height,
                             const QString &format="BMP",
                             int quality=-1) const;
        static QString currentTime();
        void handleConnection();
        void dataReady(QTcpSocket *socket);
        inline void handleMessage(QTcpSocket *socket, const Message &message);
};

WebServer::WebServer(QTranslator *translator, QObject *parent):
    QTcpServer(parent)
{
    this->d = new WebServerPrivate(this);
    this->d->m_translator = translator;
    QObject::connect(this,
                     &QTcpServer::newConnection,
                     [this] () {
                     this->d->handleConnection();
    });

    this->d->m_socketServer.channel()->registerObject("WebServer", this);
    this->d->m_socketServer.listen();
    this->listen();

    qDebug() << "Web server URL:" << this->url();
    qDebug() << "Socket server URL:" << this->d->m_socketServer.url();

    QObject::connect(&this->d->m_timer,
                     &QTimer::timeout,
                     this,
                     &WebServer::frameReady,
                     Qt::DirectConnection);
    this->d->m_timer.setInterval(33);
    this->d->m_timer.start();
}

WebServer::~WebServer()
{
    delete this->d;
}

QString WebServer::url() const
{
    return QString("http://%1:%2")
            .arg(this->serverAddress().toString())
            .arg(this->serverPort());
}

QString WebServer::tr(const QString &context,
                      const QString &sourceText,
                      const QString &disambiguation,
                      int n)
{
    if (!this->d->m_translator)
        return sourceText;

    QString translated;

    if (disambiguation.isEmpty())
        translated =
                this->d->m_translator->translate(context.toStdString().c_str(),
                                                 sourceText.toStdString().c_str(),
                                                 nullptr,
                                                 n);

    translated =
            this->d->m_translator->translate(context.toStdString().c_str(),
                                             sourceText.toStdString().c_str(),
                                             disambiguation.toStdString().c_str(),
                                             n);

    return translated;
}

WebServerPrivate::WebServerPrivate(WebServer *self):
    self(self)
{

}

QByteArray WebServerPrivate::readFrame(int width,
                                       int height,
                                       const QString &format,
                                       int quality) const
{
    QImage image(width, height, QImage::Format_ARGB32);

    /* Filling the image with random noise is slow, you can disable this code to
     * see the true frame rate.
     */
#if 1
    static std::uniform_int_distribution<quint8> distribution(std::numeric_limits<quint8>::min(),
                                                              std::numeric_limits<quint8>::max());
    static std::default_random_engine engine;

    for (int y = 0; y < height; y++) {
        auto line = reinterpret_cast<RGBA *>(image.scanLine(y));

        for (int x = 0; x < width; x++) {
            line[x].r = distribution(engine);
            line[x].g = distribution(engine);
            line[x].b = distribution(engine);
            line[x].a = 255;
        }
    }
#endif

    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format.toStdString().c_str(), quality);

    return data;
}

QString WebServerPrivate::currentTime()
{
    static const QMap<Qt::DayOfWeek, QString> dayOfWeekToStr {
        {Qt::Monday   , "Mon"},
        {Qt::Tuesday  , "Tue"},
        {Qt::Wednesday, "Wed"},
        {Qt::Thursday , "Thu"},
        {Qt::Friday   , "Fri"},
        {Qt::Saturday , "Sat"},
        {Qt::Sunday   , "Sun"},
    };

    static const QMap<int, QString> monthToStr {
        {1 , "Jan"},
        {2 , "Feb"},
        {3 , "Mar"},
        {4 , "Apr"},
        {5 , "May"},
        {6 , "Jun"},
        {7 , "Jul"},
        {8 , "Aug"},
        {9 , "Sep"},
        {10, "Oct"},
        {11, "Nov"},
        {12, "Dec"},
    };

    auto dateTime = QDateTime::currentDateTimeUtc();
    auto dayOfWeek = dayOfWeekToStr.value(Qt::DayOfWeek(dateTime.date()
                                                                .dayOfWeek()),
                                          "Mon");
    auto month = monthToStr.value(dateTime.date().month(), "Jan");

    return QString("%1, %2 %3 %4 %5 GMT")
            .arg(dayOfWeek)
            .arg(dateTime.toString("dd"))
            .arg(month)
            .arg(dateTime.date().year())
            .arg(dateTime.toString("HH:mm:ss"));
}

void WebServerPrivate::handleConnection()
{
    auto socket = self->nextPendingConnection();
    QObject::connect(socket,
            &QAbstractSocket::disconnected,
            socket,
            &QObject::deleteLater);
    QObject::connect(socket,
                     &QAbstractSocket::readyRead,
                     [this, socket] () {
        this->dataReady(socket);
    });
}

void WebServerPrivate::dataReady(QTcpSocket *socket)
{
    Message message;

    for (int i = 0; !socket->atEnd(); i++) {
        auto line = socket->readLine().trimmed();

        if (line.isEmpty()) {
            break;
        } else if (i == 0) {
            auto parts = line.split(' ');
            message.method = parts.value(0);
            message.path = parts.value(1);
            message.protocol = parts.value(2);
        } else {
            auto index = line.indexOf(':');
            auto key = line.left(index).trimmed();
            auto value = line.right(line.size() - index - 1).trimmed();
            message.headers[key] = value;
        }
    }

    auto dataSize = message.headers.value("Content-Length").toLongLong();

    if (dataSize > 0)
        message.data = socket->read(dataSize);

    qDebug() << message.method << message.path << message.protocol;
    qDebug() << message.headers;
    qDebug() << message.data;
    qDebug();

    this->handleMessage(socket, message);
    socket->disconnectFromHost();
    socket->waitForDisconnected();
}

void WebServerPrivate::handleMessage(QTcpSocket *socket, const Message &message)
{
    static const QMap<QString, QString> redirect {
        {"/"          , "/QtWebApp/share/html/index.html"},
        {"/index.html", "/QtWebApp/share/html/index.html"},
    };

    QUrl url("http://"
             + self->serverAddress().toString()
             + ":"
             + self->serverPort()
             + redirect.value(message.path, message.path));

    if (QFileInfo(':' + url.path()).isDir()) {
        if (!url.path().endsWith('/'))
            url.setPath(url.path() + '/');

        url.setPath(url.path() + "index.html");
    }

    auto contentType = message.headers.value("Content-Type");

    if (message.method == "POST"
        && contentType == "application/x-www-form-urlencoded")
        url.setQuery(message.data);

    // Only allow to access files in the resource system.
    auto path = ":" + url.path();

    if (QRegExp(":/video_frame.*",
                Qt::CaseSensitive,
                QRegExp::Wildcard).exactMatch(path)) {
        auto query = QUrlQuery(url);
        int width = 320;

        if (query.hasQueryItem("w"))
            width = query.queryItemValue("w").toUInt();

        int height = 240;

        if (query.hasQueryItem("h"))
            height = query.queryItemValue("h").toUInt();

        int quality = -1;

        if (query.hasQueryItem("q"))
            quality = query.queryItemValue("q").toInt();

        auto suffix = QFileInfo(path).suffix();

        if (suffix == "jpg")
            suffix = "jpeg";
        else if (suffix == "tif")
            suffix = "tiff";

        auto data = this->readFrame(width, height, suffix.toUpper(), quality);
        socket->write("HTTP/1.1 200 Ok\r\n");
        socket->write("Connection: keep-alive\r\n");
        socket->write(QString("Content-Type: image/%1\r\n").arg(suffix.toLower()).toUtf8());
        socket->write(QString("Content-Length: %1\r\n").arg(data.size()).toUtf8());
        socket->write("Last-Modified: "
                      + WebServerPrivate::currentTime().toUtf8()
                      + "\r\n");
        socket->write("Pragma-directive: no-cache\r\n");
        socket->write("Cache-directive: no-cache\r\n");
        socket->write("Cache-control: no-cache\r\n");
        socket->write("Pragma: no-cache\r\n");
        socket->write("Expires: 0\r\n");
        socket->write("\r\n");

        if (message.method != "HEAD")
            socket->write(data);
    } else if (QFile::exists(path)) {
        QFile file;
        file.setFileName(path);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            auto mimeType = QMimeDatabase()
                            .mimeTypeForFileNameAndData(file.fileName(),
                                                        &file).name();
            auto data = file.readAll();

            if (mimeType == "text/html"
                || mimeType == "application/javascript")
                data.replace("%%SOCKETSERVER_URL%%",
                             this->m_socketServer.url().toUtf8());

            QString contentType;

            if (mimeType == "text/html")
                contentType = QString("Content-Type: %1; charset=utf-8\r\n")
                              .arg(mimeType);
            else
                contentType = QString("Content-Type: %1\r\n").arg(mimeType);

            socket->write("HTTP/1.1 200 Ok\r\n");
            socket->write("Connection: keep-alive\r\n");

            if (mimeType == "text/html")
                socket->write("Content-language: en\r\n");

            socket->write(contentType.toUtf8());
            socket->write(QString("Content-Length: %1\r\n").arg(data.size()).toUtf8());
            socket->write("Last-Modified: "
                          + WebServerPrivate::currentTime().toUtf8()
                          + "\r\n");
            socket->write("\r\n");

            if (message.method != "HEAD")
                socket->write(data);

            file.close();
        } else {
            socket->write("HTTP/1.0 404 FILE NOT FOUND\r\n");
        }
    } else {
        socket->write("HTTP/1.0 404 FILE NOT FOUND\r\n");
    }
}
