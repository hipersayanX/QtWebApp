/* QtWebApp, webapp made with Qt.
 * Copyright (C) 2019  Gonzalo Exequiel Pedone
 *
 * QtWebApp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * QtWebApp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Webcamoid. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QTcpSocket>
#include <QTranslator>
#include <QUrl>
#include <QWebChannel>

#include "webserver.h"
#include "socketserver.h"

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

        explicit WebServerPrivate(WebServer *self);
        static QString currentTime();
        inline void handleMessage(QTcpSocket *socket, const Message &message);
};

WebServer::WebServer(QTranslator *translator, QObject *parent):
    QTcpServer(parent)
{
    this->d = new WebServerPrivate(this);
    this->d->m_translator = translator;
    QObject::connect(this,
                     &QTcpServer::newConnection,
                     this,
                     &WebServer::handleConnection);
    this->d->m_socketServer.channel()->registerObject("WebServer", this);
    this->d->m_socketServer.listen();
    this->listen();

    qDebug() << "Web server URL:" << this->url();
    qDebug() << "Socket server URL:" << this->d->m_socketServer.url();
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

void WebServer::handleConnection()
{
    auto socket = this->nextPendingConnection();
    connect(socket,
            &QAbstractSocket::disconnected,
            socket,
            &QObject::deleteLater);
    connect(socket,
            &QAbstractSocket::readyRead,
            this,
            &WebServer::dataReady);
}

void WebServer::dataReady()
{
    auto socket = reinterpret_cast<QTcpSocket *>(sender());
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

    this->d->handleMessage(socket, message);
    socket->disconnectFromHost();
    socket->waitForDisconnected();
}

WebServerPrivate::WebServerPrivate(WebServer *self):
    self(self)
{

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

void WebServerPrivate::handleMessage(QTcpSocket *socket, const Message &message)
{
    static const QMap<QString, QString> redirect {
        {"/"          , "/QtWebApp/share/html/index.html"},
        {"/index.html", "/QtWebApp/share/html/index.html"},
    };

    QUrl url;
    url.setScheme("http");
    url.setHost(self->serverAddress().toString());
    url.setPort(self->serverPort());
    url.setPath(redirect.value(message.path, message.path));

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

    if (QFile::exists(path)) {
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
