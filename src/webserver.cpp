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
#include <QMimeDatabase>
#include <QTcpSocket>
#include <QWebChannel>

#include "webserver.h"
#include "socketserver.h"

class WebServerPrivate
{
    public:
        SocketServer m_socketServer;

        static QString currentTime();
        inline void handleGet(QTcpSocket *socket, const QString &path);
};

WebServer::WebServer(QObject *parent):
    QTcpServer(parent)
{
    this->d = new WebServerPrivate;
    QObject::connect(this,
                     &QTcpServer::newConnection,
                     this,
                     &WebServer::handleConnection);
    this->d->m_socketServer.channel()->registerObject("WebServer", this);
    this->d->m_socketServer.listen();
    this->listen();

    qDebug() << "Web server URL:" << this->url();
    qDebug() << "Socket server URL:" << this->url();
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
    auto request = QString(socket->readAll()).split("\r\n");
    QString method;
    QString path;
    QString protocol;
    QMap<QString, QString> headers;

    for (auto &line: request) {
        auto i = line.trimmed().indexOf(':');

        if (i < 0) {
            if (!line.isEmpty()) {
                auto parts = line.split(' ');
                method = parts.value(0);
                path = parts.value(1);
                protocol = parts.value(2);
            }
        } else {
            auto key = line.left(i).trimmed();
            auto value = line.right(line.size() - i - 1).trimmed();
            headers[key] = value;
        }
    }

    qDebug() << method << path << protocol;
    qDebug() << headers;
    qDebug();

    if (method == "GET")
        this->d->handleGet(socket, path);

    socket->disconnectFromHost();
    socket->waitForDisconnected();
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

void WebServerPrivate::handleGet(QTcpSocket *socket, const QString &path)
{
    QString filePath = path;

    if (filePath == "/")
        filePath = "/index.html";

    if (filePath == "/index.html")
        filePath = "/QtWebApp/share/html/index.html";

    filePath = ":" + filePath;

    if (QFile::exists(filePath)) {
        QFile file;
        file.setFileName(filePath);

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
            socket->write(data);
            file.close();
        } else {
            socket->write("HTTP/1.0 404 FILE NOT FOUND\r\n");
        }
    } else {
        socket->write("HTTP/1.0 404 FILE NOT FOUND\r\n");
    }
}
