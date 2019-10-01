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

#ifdef QT_WEBENGINEWIDGETS_LIB
#include <QWebEngineView>
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "webserver.h"

class MainWindowPrivate
{
    public:
        QString m_url;
};

MainWindow::MainWindow(WebServer *server,
                       QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->d = new MainWindowPrivate;
    this->d->m_url = server->url();
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete this->d;
}

void MainWindow::showEvent(QShowEvent *event)
{
#ifdef QT_WEBENGINEWIDGETS_LIB
    if (this->isVisible()) {
        auto webView = ui->mainLayout->findChild<QWebEngineView *>("WebEngineView");

        if (!webView) {
            webView = new QWebEngineView;
            webView->setObjectName("WebEngineView");
            webView->setUrl(QUrl(this->d->m_url));
            ui->mainLayout->addWidget(webView);
        }
    }
#endif

    QMainWindow::showEvent(event);
}
