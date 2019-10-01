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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindowPrivate;
class WebServer;

class MainWindow: public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(WebServer *server, QWidget *parent=nullptr);
        ~MainWindow();

    private:
        MainWindowPrivate *d;
        Ui::MainWindow *ui;

    protected:
        void showEvent(QShowEvent *event);
};

#endif // MAINWINDOW_H
