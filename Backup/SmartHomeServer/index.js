var init_db = true;

var path = require('path');

var file = require('./file');
var sys = require('sys');
var async = require('async');
var exec = require('child_process').exec;

musicHelper = require('./musicHelper').musicHelper;
musicHelper = new musicHelper();

MongoClient = require('mongodb').MongoClient,
Server = require('mongodb').Server,
CollectionDriver = require('./collectionDriver').CollectionDriver;
var http = require('http');
express = require('express');
var app = express();
app.set('port', process.env.PORT || 3000);

var totalSwitches = 0;
var totalSwitchesOn = 0;

var mongoHost = 'localhost';
var mongoPort = 27017; 
var collectionDriver;


var childProcess;



var kill = function (pid, signal, callback) {
    signal = signal || 'SIGKILL';
    callback = callback || function () { };
    var killTree = true;
    if (killTree) {
        psTree(pid, function (err, children) {
            [pid].concat(
                children.map(function (p) {
                    return p.PID;
                })
            ).forEach(function (tpid) {
                try { process.kill(tpid, signal) }
                catch (ex) { }
            });
            callback();
        });
    } else {
        try { process.kill(pid, signal) }
        catch (ex) { }
        callback();
    }
};

console.log("Connecting to MongoDB database");
var mongoClient = new MongoClient(new Server(mongoHost, mongoPort));

mongoClient.open(function(err, mongoClient) {
	if (!mongoClient) {
		console.error("Error! Exiting... Must start MongoDB first");
		process.exit(1); //D
	} else{
		console.log("Connected to MongoDB database");
	};
	var db = mongoClient.db("MyDatabase");
	collectionDriver = new CollectionDriver(db);
	if (init_db) {
		init_db_func = require('./initDB').init_db;
		init_db_func(collectionDriver);
	};

	energyTracker = require('./energyTracker').energyTracker;
	energyTracker = new energyTracker(collectionDriver);

    // update switch stats
	energyTracker.getSwitchStatistic(function (totalOn, total) {
	    totalSwitchesOn = totalOn;
	    totalSwitches = total;
	    // create alarm here
	});

	require('./update_monitor').update_monitor;
	update_monitor = new update_monitor(collectionDriver);
	update_monitor.start();

	require('./modeHelper').modeHelper;
	modeHelper = new modeHelper(collectionDriver);
});

require('./send_command').send_command;
send_command = new send_command();

app.use(express.bodyParser());

app.use(express.static(path.join(__dirname, 'public')));


app.get('/api/room_switch', function (req, res) {
    collectionDriver.findAll('room', function (error, objs) {
        if (error) { res.send(400, error); }
        else {
            for (var i=0; i < objs.length; i++) {
                collectionDriver.find('switch_board', { 'room': objs[i].id }, function (err, switches) {
                    console.log(objs[i]);
                    if (objs[i] != null)
                        objs[i].switches = switches;
                    console.log(objs[i]);
                });
            }
            res.set('Content-Type', 'application/json');
            res.send(200, objs);
        }
    });
});

// client asks to update relay status of a device
// request params included in request body
// {'_id': deviceId, 'relay_index': relayIndex, 'status': 0-1}
app.put('/api/set_switch_status', function (req, res) { //A
    var params = req.body;
    console.log(params);
    collectionDriver.get('switch_board', params._id, function (error, device) {
        if (error) {
            console.log(error);
            res.send(400, error);
        } else {
            console.log(device);
            device.relay[params.relay_index].status = params.status;
            console.log(device);
            collectionDriver.update('switch_board', device, params._id, function (error, objs) { //B
                if (error) {
                    res.send(400, error);
                } else {
                    res.send(200, objs);
                    // sync up with RF24 gateway
                    send_command.add_command(device);
                    // update switch stats
                    energyTracker.getSwitchStatistic(function(totalOn, total) {
                        totalSwitchesOn = totalOn;
                        totalSwitches = total;
                        // create alarm here
                    });
                }
            });
        }
    });
    return;
});

// client asks to update door status
// request params included in request body
// {'_id': doorId, 'status': 0-1}
app.put('/api/set_door_status', function (req, res) {
    var params = req.body;
    console.log(params);
    collectionDriver.get('door_lock', params._id, function (error, door) {
        if (error) {
            console.log(error);
            res.send(400, error);
        } else {
            door.status = params.status;
            console.log(door);
            collectionDriver.update('door_lock', door, params._id, function (error, objs) { //B
                if (error) {
                    res.send(400, error);
                } else {
                    res.send(200, objs);
                    // sync up with RF24 gateway

                }
            });
        }
    });
    return;
});

// client asks for energy stats
app.get('/api/energy_stats', function (req, res) {
    res.send(200, {'totalOn': totalSwitchesOn, 'total':totalSwitches});
});

// client asks list of music folders
app.get('/api/music_folders', function (req, res) {
    musicHelper.listFolders(function (list) {
        res.send(200, list);
    });
});

// client asks to play music
// request params included in request body
// {'_id': modeId}
app.put('/api/play_music_mode', function (req, res) {
    var params = req.body;
    console.log(params);
    collectionDriver.get('music', params._id, function (error, mode) {
        if (error || mode == null) {
            res.send(400, error);
        } else {
            // play music here
            console.log('Playing music files in folders: ');
            console.log(mode.folders);
            musicHelper.playMusic(mode.folders);
            res.send(200, null);
        }
    });
    return;
});

// client asks to play music
// request params included in request body
// {'_id': modeId}
app.put('/api/stop_music_mode', function (req, res) {    
    // stop playing music
    console.log('Stop playing music files');
    musicHelper.stopMusic();
    res.send(200, null);
    return;
});

// client asks to run a scenario mode
// request params included in request body
// {'_id': modeId}
app.post('/api/run_mode', function (req, res) {
    console.log('Executing mode: ' + req.body.modeId);
    modeHelper.runMode(req.body.modeId, function (error, result) {
        if (error == null) {
            res.send(200, null);
            //console.log(result);
            for (var i = 0; i < result.length; i++) {
                if (result[i].type == 100) { // music mode
                    console.log('Play music folders: ');
                    console.log(result[i].folders);
                    //musicHelper.playMusic(result[i].folders);
                } else
                    send_command.add_command(result[i]);
            }
            
        } else {
            res.send(500, error);
        }
    });
    return;
});

app.get('/api/:collection', function (req, res) { //A
   var params = req.params; //B
   collectionDriver.findAll(req.params.collection, function(error, objs) { //C
    	  if (error) { res.send(400, error); } //D
	      else { 
	          //if (req.accepts('html')) { //E
    	      //  res.render('data',{objects: objs, collection: req.params.collection}); //F
              //} else {
				res.set('Content-Type','application/json'); //G
                res.send(200, objs); //H
              //}
         }
   	});
});
 
app.get('/api/:collection/:entity', function(req, res) {
   var params = req.params;
   var entity = params.entity;
   var collection = params.collection;
   if (entity) {
       collectionDriver.get(collection, entity, function(error, objs) {
          if (error) { res.send(400, error); }
          else { res.send(200, objs); }
       });
   } else {
      res.send(400, {error: 'bad url', url: req.url});
   }
});



app.post('/api/:collection', function(req, res) {
    var object = req.body;
    var collection = req.params.collection;
    collectionDriver.save(collection, object, function(err,docs) {
          if (err) { res.send(400, err); } 
          else { res.send(201, docs); } //B
     });
});

app.put('/api/:collection/:entity', function(req, res) {
    var params = req.params;
    var entity = params.entity;
    var collection = params.collection;
    console.log("request body: ");
    console.log(req.body);
    if (entity) {
        collectionDriver.update(collection, req.body, entity, function(error, objs) {
            if (error) {
                res.send(400, error);
            } else {
                res.send(200, objs);
            }
       });
    } else {
        var error = { "message" : "Cannot PUT a whole collection" };
        res.send(400, error);
    }
});

app.delete('/api/:collection/:entity', function (req, res) {
    var params = req.params;
    var entity = params.entity;
    var collection = params.collection;
    if (entity) {
        collectionDriver.delete(collection, entity, function (error, doc) { //B
            if (error) {
                res.send(400, error);
            } else {
                res.send(200, null);
            }
       });
   } else {
       var error = { "message" : "Cannot DELETE a whole collection" };
       res.send(400, error);
    }
});

// application -------------------------------------------------------------
app.get('/', function(req, res) {
	res.sendfile('./public/index.html'); // load the single view file (angular will handle the page changes on the front-end)
});

app.use(function (req, res) { //1
    res.send('<html><body><h1>Could not load page: ' + req.url + '</h1></body></html>');
});

http.createServer(app).listen(app.get('port'), function(){
	console.log('Express server listening on port ' + app.get('port'));
});
