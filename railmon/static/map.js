var map = [
    {
        // Right
        t_switch: [175, 50],
        flip: true, state: false,
        id: 2, locked: false
    },
    {
        // Bottom right, outer
        tracks: [
            [175, 60],
            [175, 75],
            [135, 115],
            [120, 115],
        ]
    },
    {
        // Bottom right, inner
        tracks: [
            [165, 60],
            [140, 85],
        ]
    },
    {
        // Middle right
        t_switch: [130, 95],
        diagon: true, flip: true, state: false,
        id: 4, locked: false
    },
    {
        // Middle
        tracks: [
            [120, 95],
            [80, 95],
        ]
    },
    {
        // Middle left
        t_switch: [70, 95],
        rotate: 3, state: false,
        id: 3, locked: false
    },
    {
        // Bottom
        t_switch: [110, 115],
        rotate: 3, state: false,
        id: 5, locked: false
    },
    {
        // Left, outer
        tracks: [
            [100, 115],
            [35, 115],
            [0, 85],
            [0, 50],
            [30, 20],
            [50, 20],
        ]
    },
    {
        // Top right
        tracks: [
            [70,20],
            [95,20],
            [115,0],
            [155,0],
            [175,20],
            [175,40]
        ]
    },
    {
        // Top left
        t_switch: [60, 20],
        rotate: 1, state: false,
        id: 1, locked: false
    },
    {
        // Left, inner
        tracks: [
            [50,30],
            [30,50],
            [30,75],
            [50,95],
            [60,95]
        ]
    },
    {
        // Middle inside
        tracks: [
            [80,85],
            [95,70],
            [130,35],
        ]
    }
];

var mapScale = 4;

var switches = {};

map.forEach(function(segment) {
    if(segment.t_switch) {
        switches[segment.id] = segment;
    }
});

var drawMap = (function() {
    var canvas = document.getElementById("map");
    var ctx = canvas.getContext("2d");

    ctx.translate(10, 10);
    ctx.lineWidth = 10;
    ctx.lineCap = "round";
    ctx.lineJoin = "round";

    function drawMap() {
        ctx.clearRect(-10, -10, canvas.width, canvas.height);
        map.forEach(function(segment) {
            if(segment.tracks) {
                ctx.beginPath();
                segment.tracks.forEach(function(track, i) {
                    if(i < 1) {
                        ctx.moveTo(track[0]*mapScale, track[1]*mapScale);
                    } else {
                        ctx.lineTo(track[0]*mapScale, track[1]*mapScale);
                    }
                });
                ctx.strokeStyle = "#eee";
                ctx.stroke();
            } else if(segment.t_switch) {
                ctx.save();

                var color = (segment.locked) ? "orange" : "#eee";

                var sX = mapScale * (segment.flip ? -1 : 1);
                var sY = mapScale;

                ctx.translate(segment.t_switch[0]*mapScale, segment.t_switch[1]*mapScale);
                ctx.rotate(Math.PI * (segment.rotate || 0) / 2);

                ctx.beginPath();
                ctx.moveTo(0, 0);
                ctx.lineTo(segment.diagon ? -10*sX : 0, -10*sY);
                ctx.strokeStyle = color
                ctx.stroke();

                function drawDefault() {
                    ctx.beginPath();
                    ctx.moveTo(0, 0);
                    ctx.lineTo(segment.diagon ? 10*sX : 0, 10*sY);
                    ctx.strokeStyle = (segment.state ? "rgba(255,255,255,0.5)" : color);
                    ctx.stroke();
                }

                function drawAlternate() {
                    ctx.beginPath();
                    ctx.moveTo(0, 0);
                    ctx.lineTo(10*sX, segment.diagon ? 0: 10*sY);
                    ctx.strokeStyle = (segment.state ? color : "rgba(255,255,255,0.5)");
                    ctx.stroke();
                }

                if(segment.state) {
                    drawDefault();
                    drawAlternate();
                } else {
                    drawAlternate();
                    drawDefault();
                }

                ctx.lineWidth = 10;
                ctx.restore();
            }
        });
    }

    drawMap();
    return drawMap;
})();
