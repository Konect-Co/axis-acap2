var mysql = require('mysql');
const fs = require('fs');
var express = require('express');

var app = express();

var con = mysql.createConnection({
  host: "18.188.84.0",
  user: "ravit",
  password: "mypass123",
  database: "spaspect"
});

con.connect();
jsonString = fs.readFileSync('./commands.json');

app.get('/', (req, res) =>{
  res.send("Updated");
  var commands = JSON.parse(jsonString);

  for(var i = 0 ; i < Object.keys(commands).length ; i++){
    con.query(commands[Object.keys(commands)[i]], (err, result) => {});
  }
});

var port = 3000;

app.listen(port, () => {
  console.log("listening on port ", port);
})
//con.end();