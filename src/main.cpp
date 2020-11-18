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

#include <QApplication>

#ifdef QT_WEBENGINE_LIB
#include <QtWebEngine>
#elif defined(QT_WEBVIEW_LIB)
#include <QtWebView>
#endif

#include "webserver.h"
#include "webappwindow.h"
#include "mainwindow.h"
#include "qmlwindow.h"
#include "webkitwindow.h"

enum FrontEnd
{
    WebApp,  // Linux Windows
    Widgets, // Linux Mac
    Qml,     // Linux Mac Android
    Webkit   // Linux Mac Windows
};

int main(int argc, char **argv)
{
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    // Install translations.
    QTranslator translator;
    translator.load(QLocale::system().name(), ":/QtWebApp/share/ts");
    QApplication::installTranslator(&translator);

    WebServer server(&translator);
    FrontEnd frontEnd = FrontEnd::Widgets;

#ifdef QT_WEBENGINE_LIB
    QtWebEngine::initialize();
#elif defined(QT_WEBVIEW_LIB)
    QtWebView::initialize();
#endif

    MainWindow window(&server);
    WebAppWindow webAppWindow(&server);
    QmlWindow qmlWindow(&server);
    WebkitWindow webkitWindow(&server);

    switch (frontEnd) {
    case FrontEnd::Widgets:
        window.show();

        break;

    case FrontEnd::WebApp:
        webAppWindow.show();

        break;

    case FrontEnd::Qml:
        qmlWindow.show();

        break;

    case FrontEnd::Webkit:
        webkitWindow.show();

        break;
    }

    return QApplication::exec();
}
