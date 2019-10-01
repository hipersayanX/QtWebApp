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

#include <QWebChannel>

#include "socketserver.h"
#include "channeltransport.h"

class SocketServerPrivate
{
    public:
        QWebChannel m_channel;
};

SocketServer::SocketServer(QObject *parent):
    QWebSocketServer("QtWebApp Socket Server",
                     QWebSocketServer::NonSecureMode,
                     parent)
{
    this->d = new SocketServerPrivate;
    connect(this,
            &QWebSocketServer::newConnection,
            [this] () {
        auto transport = new ChannelTransport(this->nextPendingConnection());
        this->d->m_channel.connectTo(transport);
    });
    this->d->m_channel.registerObject("SocketServer", this);
}

SocketServer::~SocketServer()
{
    delete this->d;
}

QWebChannel *SocketServer::channel()
{
    return &this->d->m_channel;
}

QString SocketServer::url() const
{
    return QString("ws://%1:%2")
            .arg(this->serverAddress().toString())
            .arg(this->serverPort());
}
