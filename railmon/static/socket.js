var ws = new WebSocket("ws://rail-pi");

ws.onmessage = function(m) {
    var msg = JSON.parse(m.data);

    switch(msg.type) {
        case "raild":
            var msg = msg.payload;
            console.log(msg);
            switch(msg.event) {
                case "SwitchChange":
                    if(!switches[msg.id]) return;
                    switches[msg.id].state = msg.state;
                    drawMap();
                    break;

                case "SwitchLock":
                case "SwitchUnlock":
                    if(!switches[msg.id]) return;
                    switches[msg.id].locked = (msg.event == "SwitchLock");
                    drawMap();
                    break;

                case "SensorChange":
                    if(!sensors[msg.id]) return;
                    sensors[msg.id].active = msg.state;
                    drawMap();
                    break;
            }
            break;
    }
};
