var express = require("express");
var directory = require("serve-index")
var ws = require("ws");

var app = express();

app.set("title", "RailPi");

app.use(directory(__dirname + '/static'))
app.use(express.static(__dirname + '/static'))

app.listen(80);
