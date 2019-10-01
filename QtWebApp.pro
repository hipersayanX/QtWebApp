# QtWebApp, webapp made with Qt.
# Copyright (C) 2019  Gonzalo Exequiel Pedone
#
# QtWebApp is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation.
#
# QtWebApp is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Webcamoid. If not, see <http://www.gnu.org/licenses/>.

QT += widgets qml quick websockets webchannel
qtHaveModule(webengine): {
    QT += webengine
} else: {
    qtHaveModule(webview): QT += webview
}

qtHaveModule(webenginewidgets): QT += webenginewidgets

SOURCES = \
    src/channeltransport.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/qmlwindow.cpp \
    src/socketserver.cpp \
    src/webappwindow.cpp \
    src/webserver.cpp

HEADERS = \
    src/channeltransport.h \
    src/mainwindow.h \
    src/qmlwindow.h \
    src/socketserver.h \
    src/webappwindow.h \
    src/webserver.h

lupdate_only {
    SOURCES += $$files(share/qml/*.qml)
}

RESOURCES += \
    QtWebApp.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    share/android/AndroidManifest.xml \
    share/android/build.gradle \
    share/android/gradle/wrapper/gradle-wrapper.jar \
    share/android/gradle/wrapper/gradle-wrapper.properties \
    share/android/gradlew \
    share/android/gradlew.bat \
    share/android/res/values/libs.xml

contains(ANDROID_TARGET_ARCH,x86_64) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/share/android
}

FORMS += \
    share/ui/mainwindow.ui
