var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');

var routes = require('./routes/index');
var users = require('./routes/users');

// *************** configure database ********************
var historyDatabaseUrl = "data/historyData.db"; 
var currentDatabaseUrl = "data/currentData.db"; 
var Datastore = require('nedb');
db = {};
//db.users = new Datastore({ filename: databaseUrl, autoload: true }); // to autoload datastore
db.historyData = new Datastore({ filename: historyDatabaseUrl, autoload: true }); // to autoload datastore
db.currentData = new Datastore({ filename: currentDatabaseUrl, autoload: true }); // to autoload datastore

// *************** configure socket.io ********************

var serialport = require("serialport");
var SerialPort = serialport.SerialPort; // localize object constructor 

var portName = 'COM14';
var baudrate = 115200;

var serialPort = new SerialPort(portName, {
  baudrate: baudrate,
  parser: serialport.parsers.readline("\n")
});

// *************** configure express ********************
var app = express();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

// uncomment after placing your favicon in /public
//app.use(favicon(__dirname + '/public/favicon.ico'));
app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

//app.use('/', routes);
//app.use('/users', users);


// catch 404 and forward to error handler
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
  app.use(function(err, req, res, next) {
    res.status(err.status || 500);
    res.render('error', {
      message: err.message,
      error: err
    });
  });
}

// production error handler
// no stacktraces leaked to user
app.use(function(err, req, res, next) {
  res.status(err.status || 500);
  res.render('error', {
    message: err.message,
    error: {}
  });
});



var http = require('http').Server(app);
//var io = require('socket.io').listen(8000); // server listens for socket.io communication at port 8000
var io = require('socket.io')(http); // server listens for socket.io communication at port 8000
//io.set('log level', 1); // disables debugging. this is optional. you may remove it if desired.


//module.exports = app;
/*
var server = app.listen(3000, function () {

  var host = server.address().address;
  var port = server.address().port;

  console.log('Example app listening at http://%s:%s', host, port);

});
*/

http.listen(3000, function(){
  console.log('listening on *:3000');
});


// *************** configure socket.io events ********************
io.on('connection', function (socket) {
    console.log('client connected');
    // If socket.io receives message from the client browser then 
    // this call back will be executed.
    socket.on('clientUpdate', function (msg) {
        console.log("Receive data update from client:");
        console.log(msg);
    });
    // If a web browser disconnects from Socket.IO then this callback is called.
    socket.on('disconnect', function () {
        console.log('disconnected');
    });
});

// *************** configure serial events ********************

var cleanData = ''; // this stores the clean data

//var readData = '';  // this stores the buffer

serialPort.on("open", function () {
    console.log('Serial open successfully');
    serialPort.on('data', function(data) {
        console.log('Data received: ' + data);
        //io.sockets.emit('message', data);
        //readData += data.toString(); // append data to buffer
        if (data.indexOf("S;") >= 0 && data.indexOf(";E") >= 0) {
            cleanData = data.substring(data.indexOf("S;") + 1, data.indexOf(";E"));
            // parse data and save to DB
            var dataArray = cleanData.split(";");
            var record = {
                nodeId: parseInt(dataArray[1]),
                data1: parseFloat(dataArray[2]),
                data2: parseFloat(dataArray[3]),
                data3: parseFloat(dataArray[4]),
                data4: parseFloat(dataArray[5]),
                date: new Date()};

            db.historyData.insert([record],
                function(err, saved) { // Query in NeDB via NeDB Module
                    if( err || !saved )
                        console.log( "History data not saved"); 
                   else console.log( "History data saved");
               }
            );
            
            db.currentData.remove({ nodeId: record.nodeId }, {}, function (err, numRemoved) {
                if (!err)
                    console.log("Removed last data");
                else
                    console.log("Cannot remove last data: " + err);
            });

            db.currentData.insert([record], function (err, saved) {
                if( err || !saved )
                    console.log( "Current data not saved"); 
               else console.log( "Current data saved");
            });
            io.sockets.emit('update', record);
        }
    });
  /*
  serialPort.write("ls\n", function(err, results) {
    console.log('err ' + err);
    console.log('results ' + results);
  });
  */
});

