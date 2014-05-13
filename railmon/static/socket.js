(function() {
    var ws = new WebSocket("ws://rail-pi");

    function updateSwitchState(id, state) {
        if(!switches[id]) return;
        switches[id].state = state;
    }

    function updateSwitchLock(id, locked) {
        if(!switches[id]) return;
        switches[id].locked = locked;
    }

    function updateSensorActive(id, active) {
        if(!sensors[id]) return;
        sensors[id].active = active;
    }

    ws.onmessage = function(m) {
        var msg = JSON.parse(m.data);

        switch(msg.type) {
            case "raild":
                var msg = msg.payload;
                console.log(msg);
                switch(msg.event) {
                    case "SwitchChange":
                        updateSwitchState(msg.id, msg.state);
                        drawMap();
                        break;

                    case "SwitchLock":
                    case "SwitchUnlock":
                        updateSwitchLock(msg.id, msg.event == "SwitchLock");
                        drawMap();
                        break;

                    case "SensorChange":
                        updateSensorActive(msg.id, msg.state);
                        drawMap();
                        break;

                    case "Ready":
                        $("#statusReady").removeClass("off").addClass("on");
                        break;

                    case "Disconnect":
                        $("#statusReady").removeClass("on").addClass("off");
                        break;

                    case "Sync":
                        msg.switches.forEach(function(state, idx) {
                            updateSwitchState(idx + 1, state);
                        });

                        msg.sensors.forEach(function(state, idx) {
                            updateSensorActive(idx + 1, state);
                        });

                        if(msg.powered) {
                            $("#statusPowered").removeClass("off").addClass("on");
                        } else {
                            $("#statusPowered").removeClass("on").addClass("off");
                        }

                        if(msg.ready) {
                            $("#statusReady").removeClass("off").addClass("on");
                        } else {
                            $("#statusReady").removeClass("on").addClass("off");
                        }

                        drawMap();
                        break;
                }
                break;
        }
    };
})();
