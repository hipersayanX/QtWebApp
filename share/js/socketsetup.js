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

function qsTr(sourceText, disambiguation="", n=-1, callback=null)
{
    qsTrInternal("socketsetup", sourceText, disambiguation, n, callback)
}

window.onload = function() {
    window.canvas = document.getElementById('canvas');
    window.canvasCtx = window.canvas.getContext('2d');
    let socket = new WebSocket("%%SOCKETSERVER_URL%%");

    socket.onopen = function() {
        window.channel = new QWebChannel(socket, function(channel) {
            document.getElementById("webServerUrl").innerHTML =
                    window.channel.objects.WebServer.url
            document.getElementById("socketServerUrl").innerHTML =
                    window.channel.objects.SocketServer.url
            qsTr("Click Me!", function(str) {
                document.getElementById("pushButton").innerHTML = str
            })

            let t0 = performance.now();
            let nframes = 0;

            window.channel.objects.WebServer.frameReady.connect(function() {
                var img = new Image();
                img.src = window.channel.objects.WebServer.url
                        + "/video_frame.bmp?"
                        + "w=" + window.canvas.width
                        + "&h=" + window.canvas.height
                        + "&q=-1"
                        + "&t=" + (new Date()).getTime();
                img.onload = function(){
                    window.canvasCtx.drawImage(img,
                                               0,
                                               0,
                                               window.canvas.width,
                                               window.canvas.height);
                    let tf = performance.now();
                    nframes++;
                    let fps = 1000 * nframes / (tf - t0);
                    document.getElementById("frameRate").innerHTML = Math.round(fps);
                }
            })
        })
    }

    socket.onclose = function() {
    }

    socket.onmessage = function(event) {
    }

    socket.onerror = function(error) {
    }
}
