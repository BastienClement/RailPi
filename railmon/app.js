//----------------------------------------------------------------------------//
// Raild Socket
//----------------------------------------------------------------------------//
var net = require("net");
var EventEmitter = require("events").EventEmitter;

var emitter = new EventEmitter();
var sock_lock = false;
var online = false;
var sync;

function socketConnect() {
    if(sock_lock) return;
    sock_lock = true;

    // Open socket to Raild
    var socket = net.connect(9000, "localhost", function() {
        online = true;
        emitter.emit("event", { type: "online" });

        socket.write("RailMon:Bind()\f");

        sync = function() {
            socket.write("RailMon:Sync()\f");
        };

        // Buffers list
        var buffers = [];

        // Read data from Raild
        socket.on("data", function(buf) {
            // Buffer length and start offset
            var len = buf.length;
            var from = 0;

            // Search for \n
            for(var i = from; i < len; ++i) {
                if(buf[i] == 10) {
                    buffers.push(buf.slice(from, i + 1));
                    try {
                        var json = JSON.parse(Buffer.concat(buffers).toString("ascii"));
                    } catch(e) {
                        console.error("Unable to parse JSON message");
                        console.error(e);
                        process.exit();
                    }
                    emitter.emit("event", { type: "raild", payload: json });
                    buffers = [];
                    from = i + 1;
                }
            }

            // Piece
            if(from < len) {
                buffers.push(from ? buf.slice(from) : buf);
            }
        });
    });

    // Reconnect to Raild
    function retry() {
        sync = null;

        if(online) {
            online = false;
            emitter.emit("event", { type: "offline" });
        }

        sock_lock = false;
        setTimeout(function() {
            socketConnect();
        }, 2000);
    }

    // Handle errors
    socket.on("close", retry);
    socket.on("error", retry);
}

socketConnect();

//----------------------------------------------------------------------------//
// Express app
//----------------------------------------------------------------------------//
var express = require("express");
var directory = require("serve-index");
var app = express();

app.set("title", "RailPi");

app.use(directory(__dirname + '/static'))
app.use(express.static(__dirname + '/static'))

//----------------------------------------------------------------------------//
// HTTP server
//----------------------------------------------------------------------------//
var http = require("http");

var server = http.createServer(app);
server.listen(80);

//----------------------------------------------------------------------------//
// WebSocket server
//----------------------------------------------------------------------------//
var WebSocketServer = require("ws").Server;
var wss = new WebSocketServer({server: server});

wss.on("connection", function(ws) {
    var listener = function(event) {
        ws.send(JSON.stringify(event), function() { /* ignore errors */ });
    };

    emitter.on("event", listener);
    if(online) sync();

    ws.on("close", function() {
        emitter.removeListener("event", listener);
    });
});
