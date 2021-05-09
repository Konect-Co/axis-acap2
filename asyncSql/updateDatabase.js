var mysql = require('mysql');
const fs = require('fs');
var express = require('express');
var bodyParser = require('body-parser')

var app = express();
var jsonParser = bodyParser.json()
var urlencodedParser = bodyParser.urlencoded({ extended: false })

var con = mysql.createConnection({
  host: "18.188.84.0",
  user: "ravit",
  password: "mypass123",
  database: "spaspect"
});

con.connect();

app.post('/', urlencodedParser, (request, response) => {
  if (!request.body) return response.sendStatus(400)
  //response.send("Updated");

  //var jsonString = fs.readFileSync('./commands.json');
  var commands = JSON.parse(JSON.stringify(request.body));

  response.send("updated");
  for(var i = 0 ; i < Object.keys(commands).length ; i++){
    con.query(commands[Object.keys(commands)[i]], (err, result) => {});
    
  }
});

var port = 3000;

app.listen(port, () => {
  console.log("listening on port", port);
})
//con.end();