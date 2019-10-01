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

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "qmlwindow.h"
#include "webserver.h"

class QmlWindowPrivate
{
    public:
        QQmlApplicationEngine m_engine;
        QString m_url;
};

QmlWindow::QmlWindow(WebServer *server, QObject *parent):
    QObject(parent)
{
    this->d = new QmlWindowPrivate;
    this->d->m_url = server->url();
}

QmlWindow::~QmlWindow()
{
    delete this->d;
}

void QmlWindow::show()
{
    auto context = this->d->m_engine.rootContext();
    context->setContextProperty("serverUrl", this->d->m_url);
    const QUrl url(QStringLiteral("qrc:/QtWebApp/share/qml/main.qml"));

    QObject::connect(&this->d->m_engine,
                     &QQmlApplicationEngine::objectCreated,
                     qApp,
                     [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QApplication::exit(-1);
    }, Qt::QueuedConnection);

    this->d->m_engine.load(url);
}
