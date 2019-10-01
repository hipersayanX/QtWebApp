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

#include <QJsonDocument>
#include <QJsonObject>
#include <QWebSocket>

#include "channeltransport.h"

class ChannelTransportPrivate
{
    public:
        QWebSocket *m_socket {nullptr};
};

ChannelTransport::ChannelTransport(QWebSocket *socket):
    QWebChannelAbstractTransport(socket)
{
    this->d = new ChannelTransportPrivate;
    this->d->m_socket = socket;

    connect(socket,
            &QWebSocket::textMessageReceived,
            [this] (const QString &messageData) {
        QJsonParseError error;
        auto message = QJsonDocument::fromJson(messageData.toUtf8(), &error);

        if (error.error || !message.isObject())
            return;

        emit messageReceived(message.object(), this);
    });
    connect(socket,
            &QWebSocket::disconnected,
            this,
            &ChannelTransport::deleteLater);
}

ChannelTransport::~ChannelTransport()
{
    this->d->m_socket->deleteLater();
    delete this->d;
}

void ChannelTransport::sendMessage(const QJsonObject &message)
{
    auto json = QJsonDocument(message).toJson(QJsonDocument::Compact);
    this->d->m_socket->sendTextMessage(QString::fromUtf8(json));
}
