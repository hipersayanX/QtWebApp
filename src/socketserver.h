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

#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <QWebSocketServer>

class SocketServerPrivate;
class QWebChannel;

class SocketServer: public QWebSocketServer
{
    Q_OBJECT
    Q_PROPERTY(QString url
               READ url
               CONSTANT)

    public:
        explicit SocketServer(QObject *parent=nullptr);
        ~SocketServer();

        Q_INVOKABLE QWebChannel *channel();
        Q_INVOKABLE QString url() const;

    private:
        SocketServerPrivate *d;
};

#endif // SOCKETSERVER_H
