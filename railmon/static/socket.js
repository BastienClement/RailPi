var ws = new WebSocket("ws://rail-pi");

ws.onmessage = function(m) {
    var msg = JSON.parse(m.data);

    switch(msg.type) {
        case "raild":
            var msg = msg.payload;
            console.log(msg);
            switch(msg.event) {
                case "SwitchChange":
                    switches[msg.id].state = msg.state;
                    drawMap();
                    break;
            }
            break;
    }
};
