var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');

// *************** configure MQTT ********************
var mqtt    = require('mqtt');
//var mqttClient  = mqtt.connect('mqtt://test.mosquitto.org');
var mqttClient  = mqtt.connect('mqtt://localhost');
var mqttTopicTagIn = 'eHome/IN/';
var mqttTopicTagOut = 'eHome/OUT/';

//var serialEnabled = false;
var serialEnabled = true;
var serialport = require("serialport");
var SerialPort = serialport.SerialPort; // localize object constructor 
var serialPort;
//var portName = 'COM14';
var portName = '/dev/ttyAMA0';
var baudrate = 115200;


mqttClient.on('connect', function () {
    mqttClient.subscribe(mqttTopicTagIn + '#');
    //mqttClient.publish(mqttTopicTag + '3/1', '25.5');
});
 
mqttClient.on('message', function (topic, message) {
    // message is Buffer 
    var topicStr = topic.toString();
    console.log('Received new MQTT message:    ' + topicStr + ':' + message.toString());
    var nodeData = topicStr.substring(
        topicStr.indexOf(mqttTopicTagIn) + mqttTopicTagIn.length).split('/');
    if (nodeData.length != 2) {
        console.log("Topic is invalid");
        return;
    }
    
    var serialMessage = 'S;' + nodeData[0];
    for (var i=1; i<5; i++) {
        if (i == parseInt(nodeData[1]))
            serialMessage += ';' + message.toString();
        else
            serialMessage += ';-1';
    }
    
    serialMessage += ';E\n';
    
    console.log('Serial message: ' + serialMessage);
    
    if (serialEnabled) {
        serialPort.write(serialMessage, function(err, results) {
            if (err != null)
                console.log('Error when writing to Serial port. Error: ' + err);
        });
    }
    
});

// *************** configure serial processing  ********************
var processSerialInput = function (cleanData) {
    // parse data and save to DB
    var dataArray = cleanData.split(";");
    var nodeId = parseInt(dataArray[0]);
    
    for(var index=1; index<=4; index++) {
        var data = parseFloat(dataArray[index]).toString();
        if (data == -1)
            continue;
            
        // publish to MQTT server
        console.log('Publishing to MQTT server. Topic: '
            + mqttTopicTagOut + nodeId + '/' + index + '. Message: ' + data);
        mqttClient.publish(mqttTopicTagOut + nodeId + '/' + index, data);        
    }
}

var cleanData = ''; // this stores the clean data

if (serialEnabled) {
    
    serialPort = new SerialPort(portName, {
        baudrate: baudrate,
        parser: serialport.parsers.readline("\n")
    });

    serialPort.on("open", function () {
        console.log('Serial open successfully');
        serialPort.on('data', function(data) {
            console.log('Data received: ' + data);
            if (data.indexOf("S;") == 0 && data.indexOf(";E") >= 0) {
                cleanData = data.substring(data.indexOf("S;") + 2, data.indexOf(";E"));
                processSerialInput(cleanData);
            }
        });
        
        serialPort.write('AAAAAAAAAAAAAAAAAAAAAAAAaA\n', function(err, results) {
            if (err != null)
                console.log('Error when writing to Serial port. Error: ' + err);
        });
        
        serialPort.write('BBBBBBBBBBBBBBBBB\n', function(err, results) {
            if (err != null)
                console.log('Error when writing to Serial port. Error: ' + err);
        });
    });
}

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

app.get('/api/getCurrentData', function(req, res) {
    db.currentData.find({ }, function (err, docs) {
        //console.log(docs);
        res.json(docs);
    });
});

app.get('/api/getData/:nodeId/:fieldId', function(req, res) {
    //console.log(req.params.nodeId);
    var fieldsToGet = {data1: 1, date: 1};
    if (req.params.fieldId == '2')
        fieldsToGet = {data2: 1, date: 1};
    else if (req.params.fieldId == '3')
        fieldsToGet = {data3: 1, date: 1};
    if (req.params.fieldId == '4')
        fieldsToGet = {data4: 1, date: 1};
    db.nodeData.find({ 
        "nodeId": parseInt(req.params.nodeId),
        "index": parseInt(req.params.fieldId),
        "date": {$gt: getLastWeek()}
    })
    .sort({ "date": 1 })
    .limit(18000).exec(function (err, docs) {
            //console.log(docs);
            res.json({ 
                success: 1,
                data: docs,
                message: ''
            });
    });
});

app.post('/api/node', function(req, res) {
    var nodeId = parseInt(req.body.nodeId);
    var nodeName = req.body.nodeName;
    var dataField = req.body.dataField;
    var group = req.body.group;
    var visible = req.body.visible;

    db.nodes.find({ "nodeId": nodeId }, function (err, docs) {
        if (docs && docs.length > 0) {
            console.log("Node already exists");
            console.log(docs.length);
            res.json({ 
                success: 0,
                data: {},
                message: 'Node already exists'
            });
        } else {
            var newNode = {
                nodeId: nodeId,
                nodeName: nodeName,
                dataField: dataField,
                group: group,
                visible:visible};
            db.nodes.insert(newNode,
                function(err, newDoc) { // Query in NeDB via NeDB Module
                    if( err || !newDoc ) {
                        console.log( "New node not saved"); 
                        res.json({ 
                            success: 0,
                            data: {},
                            message: err
                        });
                    } else {
                        console.log( "New node saved");
                        res.json({ 
                            success: 1,
                            data: newDoc,
                            message: 'Node saved'
                        });
                    }
               }
            );
        }
    });
});



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
var io = require('socket.io')(http); // server listens for socket.io communication at port 8000

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

var getLastWeek = function (){
    var today = new Date();
    var lastWeek = new Date(today.getFullYear(), today.getMonth(), today.getDate() - 7);
    return lastWeek ;
}