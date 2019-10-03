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

# Configure translations
TRANSLATIONS_PRI = $${PWD}/translations.pri

exists(translations.qrc) {
    TRANSLATIONS = $$files(share/ts/*.ts)
    RESOURCES += translations.qrc
}

isEmpty(QMAKE_LRELEASE) {
    LRELEASE_FNAME = lrelease

    exists($$[QT_INSTALL_LIBEXECS]/$${LRELEASE_FNAME}*) {
        QMAKE_LRELEASE = $$[QT_INSTALL_LIBEXECS]/$${LRELEASE_FNAME}
    } else {
        QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/$${LRELEASE_FNAME}
    }
}

isEmpty(QMAKE_LUPDATE) {
    LUPDATE_FNAME = lupdate

    exists($$[QT_INSTALL_LIBEXECS]/$${LUPDATE_FNAME}*) {
        QMAKE_LUPDATE = $$[QT_INSTALL_LIBEXECS]/$${LUPDATE_FNAME}
    } else {
        QMAKE_LUPDATE = $$[QT_INSTALL_BINS]/$${LUPDATE_FNAME}
    }
}

# Update translations.
isEmpty(NOLUPDATE): !isEmpty(TRANSLATIONS_PRI): CONFIG(debug, debug|release) {
    updatetr.commands = $$QMAKE_LUPDATE -no-obsolete $$TRANSLATIONS_PRI
    QMAKE_EXTRA_TARGETS += updatetr
    PRE_TARGETDEPS += updatetr
}

# Compile translations files.
isEmpty(NOLRELEASE): !isEmpty(TRANSLATIONS): CONFIG(debug, debug|release) {
    compiletr.input = TRANSLATIONS
    compiletr.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
    compiletr.commands = $$QMAKE_LRELEASE -removeidentical -compress ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
    compiletr.clean = dummy_file
    compiletr.CONFIG += no_link
    QMAKE_EXTRA_COMPILERS += compiletr
    PRE_TARGETDEPS += compiler_compiletr_make_all
}
