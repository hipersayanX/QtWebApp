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

#include <QtDebug>
#include <QApplication>
#include <QProcess>
#include <QShowEvent>
#include <QThread>
#include <QWindow>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

#include "webappwindow.h"
#include "webserver.h"

//#define APPPROGRAM "chrome"
#define APPPROGRAM "chromium"

class WebAppWindowPrivate
{
    public:
        WebAppWindow *self {nullptr};
        QProcess m_process;
        QWindow *m_window {nullptr};
        QString m_url;
        WId m_winId {0};

        explicit WebAppWindowPrivate(WebAppWindow *self);
#ifdef Q_OS_WIN32
        static WINBOOL CALLBACK enumChilds(HWND child, LPARAM userData);
#endif
        void readWinId();
        void programStarted();
};

WebAppWindow::WebAppWindow(WebServer *server, QObject *parent):
    QObject(parent)
{
    this->d = new WebAppWindowPrivate(this);
    this->d->m_url = server->url();
}

WebAppWindow::~WebAppWindow()
{
    delete this->d;
}

QWindow *WebAppWindow::window()
{
    return this->d->m_window;
}

void WebAppWindow::show()
{
    QObject::connect(&this->d->m_process,
                     &QProcess::started,
                     [this] () {
        this->d->programStarted();
    });
    QObject::connect(&this->d->m_process,
                     QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                         [=](int exitCode, QProcess::ExitStatus exitStatus){
        Q_UNUSED(exitStatus)

        QApplication::exit(exitCode);
    });
    QObject::connect(&this->d->m_process,
                     &QProcess::errorOccurred,
                     [this] (QProcess::ProcessError error) {
        Q_UNUSED(error)

        QApplication::exit(this->d->m_process.exitCode());
    });

    this->d->m_process.setProgram(APPPROGRAM);
    this->d->m_process.setArguments({"--incognito",
                                     "--new-window",
//                                     "--window-size=640,480",
//                                     "--window-position=0,0",
                                     QString("--app=%1").arg(this->d->m_url),
                                    });
    this->d->m_process.start();
}

WebAppWindowPrivate::WebAppWindowPrivate(WebAppWindow *self):
    self(self)
{

}

#ifdef Q_OS_WIN32
WINBOOL WebAppWindowPrivate::enumChilds(HWND child, LPARAM userData)
{
    auto self = reinterpret_cast<WebAppWindowPrivate *>(userData);

    DWORD pid;
    GetWindowThreadProcessId(child, &pid);

    if (IsWindowVisible(child) && pid == self->m_process.processId()) {
        self->m_winId = WId(child);

        return FALSE;
    }

    return TRUE;
}
#endif

void WebAppWindowPrivate::readWinId()
{
#ifdef Q_OS_WIN32
    do {
        EnumWindows(WebAppWindowPrivate::enumChilds,
                    LPARAM(this));

        if (!this->m_winId)
            QThread::msleep(1000);
    } while (!this->m_winId && this->m_process.state() == QProcess::Running);
#endif
}

void WebAppWindowPrivate::programStarted()
{
    this->readWinId();

    if (this->m_winId)
        this->m_window = QWindow::fromWinId(this->m_winId);
}
