var serialport = require("serialport");
var SerialPort = serialport.SerialPort; // localize object constructor 

var portName = 'COM10';
var baudrate = 9600;

var serialPort = new SerialPort(portName, {
  baudrate: baudrate,
  parser: serialport.parsers.readline("\n")
});

var io = require('socket.io').listen(8000); // server listens for socket.io communication at port 8000
io.set('log level', 1); // disables debugging. this is optional. you may remove it if desired.


io.sockets.on('connection', function (socket) {
    // If socket.io receives message from the client browser then 
    // this call back will be executed.
    socket.on('message', function (msg) {
        console.log(msg);
    });
    // If a web browser disconnects from Socket.IO then this callback is called.
    socket.on('disconnect', function () {
        console.log('disconnected');
    });
});

var cleanData = ''; // this stores the clean data
var readData = '';  // this stores the buffer

serialPort.on("open", function () {
  console.log('open');
  serialPort.on('data', function(data) {
    console.log('data received: ' + data);
    io.sockets.emit('message', data);
    readData += data.toString(); // append data to buffer
    // if the letters 'A' and 'B' are found on the buffer then isolate what's in the middle
    // as clean data. Then clear the buffer. 
    if (readData.indexOf("S;") >= 0 && readData.indexOf(";E") >= 0) {
        cleanData = readData.substring(readData.indexOf("S;") + 1, readData.indexOf(";E"));
        readData = '';
        io.sockets.emit('message', cleanData);
    }
  });
  /*
  serialPort.write("ls\n", function(err, results) {
    console.log('err ' + err);
    console.log('results ' + results);
  });
  */
});