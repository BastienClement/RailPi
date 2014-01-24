var express = require("express");
var app = express();

app.set("title", "RailPi");

//app.use(express.static(__dirname + '/static'));
app.use(app.router);
app.use(express.directory(__dirname + '/static'))
app.use(express.static(__dirname + '/static'))

//app.get("/", function(req, res){
	//res.sendfile("app.html");
//});

app.listen(80);
