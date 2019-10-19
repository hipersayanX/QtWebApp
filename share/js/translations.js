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

class QString {
    constructor(str) {
        this.str = str;
    }

    toString() {
        return this.str;
    }

    arg(param) {
        let str = this.str

        for (let i = 1; i < 17; i++)
            if (str.indexOf("%" + i) >= 0) {
                str = str.replace("%" + i, param)

                break
            }

        return new QString(str)
    }
}

function qsTrInternal(context, sourceText, disambiguation, n, callback) {
    if (disambiguation instanceof Function) {
        callback = disambiguation
        disambiguation = ""
    }

    if (n instanceof Function) {
        callback = n
        n = -1
    }

    window.channel.objects.WebServer.tr(context,
                                        sourceText,
                                        disambiguation,
                                        n,
                                        function(str) {
                                            callback(new QString(str))
                                        })
}
