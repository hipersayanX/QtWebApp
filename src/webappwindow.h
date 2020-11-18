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

#ifndef WEBAPPWINDOW_H
#define WEBAPPWINDOW_H

#include <QObject>

class WebAppWindowPrivate;
class QWindow;
class WebServer;

class WebAppWindow: public QObject
{
    Q_OBJECT

    public:
        explicit WebAppWindow(WebServer *server, QObject *parent=nullptr);
        ~WebAppWindow();

        Q_INVOKABLE QWindow *window();

    private:
        WebAppWindowPrivate *d;

    public slots:
        void show();
};

#endif // WEBAPPWINDOW_H
