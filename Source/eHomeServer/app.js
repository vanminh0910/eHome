var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');

//var routes = require('./routes/index');
//var users = require('./routes/users');


// *************** configure database ********************
var historyDatabaseUrl = "data/historyData.db"; 
var currentDatabaseUrl = "data/currentData.db"; 
var nodeDataDatabaseUrl = "data/nodeData.db"; 
var groupsDatabaseUrl = "data/groups.db"; 
var nodesDatabaseUrl = "data/nodes.db"; 
var Datastore = require('nedb');
db = {};
//db.users = new Datastore({ filename: databaseUrl, autoload: true }); // to autoload datastore
db.historyData = new Datastore({ filename: historyDatabaseUrl, autoload: true }); // to autoload datastore
db.currentData = new Datastore({ filename: currentDatabaseUrl, autoload: true }); // to autoload datastore
db.nodeData = new Datastore({ filename: nodeDataDatabaseUrl, autoload: true }); // to autoload datastore
db.groups = new Datastore({ filename: groupsDatabaseUrl, autoload: true }); // to autoload datastore
db.nodes = new Datastore({ filename: nodesDatabaseUrl, autoload: true }); // to autoload datastore

//enable database compact hourly
db.historyData.persistence.setAutocompactionInterval(3600000); //3600000
db.currentData.persistence.setAutocompactionInterval(3600000);
db.nodeData.persistence.setAutocompactionInterval(3600000);
db.groups.persistence.setAutocompactionInterval(3600000);
db.nodes.persistence.setAutocompactionInterval(3600000);

// *************** configure serial connection information ********************
var serial_enabled = true;
var serialport = require("serialport");
var SerialPort = serialport.SerialPort; // localize object constructor 
//var portName = 'COM14';
var portName = '/dev/ttyAMA0';
var baudrate = 115200;

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

// *************** data APIs ********************
// init test data
if (false) {

    for (var j=1; j<900; j++) {
        var d = new Date(1433721600000 + j*300000);
        
        // temp data
        var record1 = {
            nodeId: 2,
            index: 1,
            data: Math.floor((Math.random() * 5) + 30),
            date: d
        };
        
        // humidity data
        var record2 = {
            nodeId: 2,
            index: 2,
            data: Math.floor((Math.random() * 20) + 60),
            date: d
        };
        
        // on off
        var record3 = {
            nodeId: 2,
            index: 3,
            data: Math.floor((Math.random() * 2) + 0),
            date: d
        };
        
        // voltage
        var record4 = {
            nodeId: 2,
            index: 4,
            data: Math.floor((Math.random() * 2000) + 3000),
            date: d
        };

        db.nodeData.insert([record1, record2, record3, record4],
            function(err, saved) { // Query in NeDB via NeDB Module
                if( err || !saved )
                    console.log( "Node data not saved"); 
               else console.log( "Node data saved");
           }
        );
    }    
}

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

var processSerialInput = function (cleanData) {
    console.log(cleanData);
    // parse data and save to DB
    var dataArray = cleanData.split(";");
    var nodeId = parseInt(dataArray[0]);
    console.log("Data splitted is ");
    console.log(dataArray);
    var date = new Date();
    for(var index=1; index<=4; index++) {
        var data = parseFloat(dataArray[index]);
        if (data == -1)
            continue;
        var record = {
            nodeId: nodeId,
            index: index,
            data: data,
            date: date
        };
        
        db.nodeData.insert([record],
            function(err, saved) { // Query in NeDB via NeDB Module
                if( err || !saved )
                    console.log( "Node " + nodeId + " data index " + index + " not saved"); 
               else 
                   console.log( "Node " + nodeId + " data index " + index + " saved"); 
           }
        );
        
        db.currentData.remove({ nodeId: nodeId , index: index}, {}, function (err, numRemoved) {
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
}

var cleanData = ''; // this stores the clean data

if (serial_enabled) {
    
    var serialPort = new SerialPort(portName, {
        baudrate: baudrate,
        parser: serialport.parsers.readline("\n")
    });

    serialPort.on("open", function () {
        console.log('Serial open successfully');
        serialPort.on('data', function(data) {
            console.log('Date: ' + new Date());
            console.log('Data received: ' + data);
            //io.sockets.emit('message', data);
            //readData += data.toString(); // append data to buffer
            if (data.indexOf("S;") >= 0 && data.indexOf(";E") >= 0) {
                cleanData = data.substring(data.indexOf("S;") + 2, data.indexOf(";E"));
                processSerialInput(cleanData);
                /*
                // parse data and save to DB
                var dataArray = cleanData.split(";");
                var nodeId = parseInt(dataArray[1]);
                var date = new Date();
                for(var index=1; index<=4; index++) {
                    var record = {
                        nodeId: nodeId,
                        index: index,
                        data: parseFloat(dataArray[index+1]),
                        date: date
                    };
                    
                    db.nodeData.insert([record],
                        function(err, saved) { // Query in NeDB via NeDB Module
                            if( err || !saved )
                                console.log( "Node " + nodeId + " data index " + index + " not saved"); 
                           else 
                               console.log( "Node " + nodeId + " data index " + index + " saved"); 
                       }
                    );
                    
                    db.currentData.remove({ nodeId: nodeId , index: index}, {}, function (err, numRemoved) {
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
                */
            }
        });
      /*
      serialPort.write("ls\n", function(err, results) {
        console.log('err ' + err);
        console.log('results ' + results);
      });
      */
    });
}

var getLastWeek = function (){
    var today = new Date();
    var lastWeek = new Date(today.getFullYear(), today.getMonth(), today.getDate() - 7);
    return lastWeek ;
}