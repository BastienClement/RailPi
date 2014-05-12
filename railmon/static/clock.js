function easeOut(t, b, c, d, a, p) {
    if(t > d) return b + c;
    if(t == 0) return b;
    if((t /= d) == 1) return b + c;
    if(!p) p = d * .3;
    if(!a || a < Math.abs(c)) {
        a = c;
        var s = p / 4;
    } else {
        var s = p / (2 * Math.PI) * Math.asin(c / a);
    }
    return (a * Math.pow(2, -10 * t) * Math.sin((t * d - s) * (2 * Math.PI) / p) + c + b);
}

var canvas = document.getElementById("clock_canvas");
if(canvas.getContext) {
    var ctx = canvas.getContext('2d');

    x = canvas.width;
    y = canvas.height;

    x_offset = 1;
    y_offset = 0;

    ctx.translate(x / 2 + x_offset, y / 2 + y_offset);

    var img = new Image();
    img.src = "clock.png";

    var d_m = 10;
    var d_h = 5;
    var t_m = d_m;
    var t_h = d_h;
    var min_last = (new Date).getMinutes();
    var hour_last = ((new Date).getHours() % 12) + (new Date).getMinutes() / 60;

    var last_min_tick = (new Date).getMinutes() - 1;

    var spm = 58.5;

    function getTime() {
        ctx.clearRect(-250, -250, 500, 500);
        ctx.drawImage(img, -(x / 2 + x_offset), -(y / 2 + y_offset));

        var date = new Date();
        var h = (date.getHours() % 12) + date.getMinutes() / 60;
        var m = date.getMinutes();
        var s = date.getSeconds() + date.getMilliseconds() / 1000;

        //
        // Hours
        //
        ctx.beginPath();
        ctx.moveTo(0, 0);

        ctx.save();
        if(h > 3 && h < 9)
            ctx.rotate(Math.sin(Math.PI * ((h - 3) / 6)) * 0.013);

        ctx.save();
        ctx.rotate(easeOut(t_h, Math.PI * (((h - (m / 60)) - 3) / 6), Math.PI
                * ((h - 3) / 6) - Math.PI * (((h - (m / 60)) - 3) / 6), d_h));
        if(hour_last != h) {
            t_h = 0;
            hour_last = h;
        }
        if(t_h <= d_h)
            t_h++;

        //ctx.shadowBlur = 10;
        //ctx.shadowOffsetX = 3;
        //ctx.shadowOffsetY = 6;
        //ctx.shadowColor = "rgba(0,0,0,0.4)";
        ctx.fillStyle = "#eee";

        ctx.moveTo(-45, -14);
        ctx.lineTo(130, -9);
        ctx.lineTo(130, 9);
        ctx.lineTo(-45, 14);
        ctx.fill();

        ctx.restore();
        ctx.restore();

        //
        // Minutes
        //
        ctx.beginPath();
        ctx.moveTo(0, 0);

        ctx.save();
        if(m > 15 && m < 45)
            ctx.rotate(Math.sin(Math.PI * ((m - 15) / 30)) * 0.013);

        ctx.save();
        ctx.rotate(easeOut(t_m, Math.PI * ((m - 16) / 30), Math.PI
                * ((m - 15) / 30) - Math.PI * ((m - 16) / 30), d_m));
        if(min_last != m) {
            t_m = 0;
            min_last = m;
        }
        if(s > 59.75 && last_min_tick != m) {
            document.getElementById("clock_tick").play();
            last_min_tick = m;
        }
        if(t_m <= d_m)
            t_m++;

        //ctx.shadowBlur = 20;
        //ctx.shadowOffsetX = 4;
        //ctx.shadowOffsetY = 8;
        //ctx.shadowColor = "rgba(0,0,0,0.4)";
        ctx.fillStyle = "#eee";

        ctx.moveTo(-45, -14);
        ctx.lineTo(180, -9);
        ctx.lineTo(180, 9);
        ctx.lineTo(-45, 14);
        ctx.fill();

        ctx.restore();
        ctx.restore();

        //
        // Seconds
        //
        if(s > spm)
            s = spm;

        ctx.beginPath();
        ctx.moveTo(0, 0);

        ctx.save();
        ctx.rotate((2 * Math.PI * s) / spm - Math.PI / 2);

        //ctx.shadowBlur = 80;
        //ctx.shadowOffsetX = 6;
        //ctx.shadowOffsetY = 12;
        //ctx.shadowColor = "rgba(0,0,0,0.4)";
        ctx.fillStyle = "#e2331e";

        ctx.rect(-60, -3.5, 185, 7);
        ctx.moveTo(130, -3.5);
        ctx.arc(130, 0, 16, 0, Math.PI * 2, true);
        ctx.moveTo(130, 3.5);
        ctx.rect(-60, -3.5, 185, 7);

        ctx.fill();

        //ctx.shadowBlur = 2;
        //ctx.shadowOffsetX = 0;
        //ctx.shadowOffsetY = 0;

        ctx.beginPath();
        ctx.arc(0, 0, 7, 0, Math.PI * 2, true);
        ctx.fill();

        ctx.restore();
    }

    setInterval(getTime, 1000 / 35);
}