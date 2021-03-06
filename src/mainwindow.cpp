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

#ifdef QT_WEBENGINEWIDGETS_LIB
#include <QWebEngineView>
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "webserver.h"

MainWindow::MainWindow(WebServer *server,
                       QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->m_url = server->url();
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event)
{
#ifdef QT_WEBENGINEWIDGETS_LIB
    if (this->isVisible()) {
        auto webView = ui->mainLayout->findChild<QWebEngineView *>("WebEngineView");

        if (!webView) {
            webView = new QWebEngineView;
            webView->setObjectName("WebEngineView");
            webView->setUrl(QUrl(this->m_url));
            ui->mainLayout->addWidget(webView);
        }
    }
#endif

    QMainWindow::showEvent(event);
}
