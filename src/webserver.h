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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QTcpServer>

class WebServerPrivate;
class QTranslator;

class WebServer: public QTcpServer
{
    Q_OBJECT
    Q_PROPERTY(QString url
               READ url
               CONSTANT)

    public:
        WebServer(QTranslator *translator, QObject *parent=nullptr);
        ~WebServer();

        Q_INVOKABLE QString url() const;
        Q_INVOKABLE QString tr(const QString &context,
                               const QString &sourceText,
                               const QString &disambiguation={},
                               int n=-1);
        Q_INVOKABLE QByteArray readFrame(int width, int height);

    private:
        WebServerPrivate *d;
};

#endif // WEBSERVER_H
