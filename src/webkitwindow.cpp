/* QtWebApp, webapp made with Qt.
 * Copyright (C) 2020  Gonzalo Exequiel Pedone
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

#ifdef HAVE_QT5WEBKIT
#include <QWebView>
#endif

#include "webkitwindow.h"
#include "ui_mainwindow.h"

WebkitWindow::WebkitWindow(WebServer *server, QWidget *parent):
    MainWindow(server, parent)
{
}

WebkitWindow::~WebkitWindow()
{
}

void WebkitWindow::showEvent(QShowEvent *event)
{
#ifdef HAVE_QT5WEBKIT
    if (this->isVisible()) {
        auto webView = ui->mainLayout->findChild<QWebView *>("WebView");

        if (!webView) {
            webView = new QWebView;
            webView->setObjectName("WebView");
            webView->setUrl(QUrl(this->m_url));
            ui->mainLayout->addWidget(webView);
        }
    }
#endif

    QMainWindow::showEvent(event);
}
