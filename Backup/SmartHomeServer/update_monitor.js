var fs = require('fs');
var path = require('path');

//var data_path = "/home/pi/ftp/files/rf24/examples/update/";
var data_path = "./update/";
var trigger_ext = ".trigger";

update_monitor = function(collectionDriver) {
    this.collectionDriver = collectionDriver;
};

update_monitor.prototype.process_update = function(file) {
    var content;
    var self = this;
	// First I want to read the file
    fs.readFile(file, function read(err, data) {
        if (err) {
            console.log("Cannot process update file: " + err);
            return;
        }
        var updateArray = data.toString().split(',');
        var deviceId = parseInt(updateArray[0]);
        var deviceType = parseInt(updateArray[1]);
        var dataArray = [];
        for(var i=0; i<6; i++)
            dataArray[i] = parseInt(updateArray[i+2]);

        if (deviceType < 30) {
            // switch board, pir switch
            self.collectionDriver.find('switch_board', { 'id': deviceId }, function (error, devices) {
                if (error) {
                    console.log(error);
                    // TODO: error handling here
                } else {
                    console.log("Devices found: " + devices);
                    if (devices.length > 0) {
                        var device = devices[0];
                        if (device.type == 11 || device.type == 20) {
                            // swich board with 1 switch or pir with 1 switch
                            device.relay[0].status = dataArray[0];
                        } else if (device.type == 12) {
                            // swich board with 2 switches
                            device.relay[0].status = dataArray[0];
                            device.relay[1].status = dataArray[1];
                        } else if (device.type == 13) {
                            // swich board with 3 switches
                            device.relay[0].status = dataArray[0];
                            device.relay[1].status = dataArray[1];
                            device.relay[2].status = dataArray[2];
                        } else if (device.type == 4) {
                            // swich board with 4 switches
                            device.relay[0].status = dataArray[0];
                            device.relay[1].status = dataArray[1];
                            device.relay[2].status = dataArray[2];
                            device.relay[3].status = dataArray[3];
                        };

                        self.collectionDriver.update('switch_board', device, device._id, function(error, objs) { //B
                            if (error) {
                                // TODO: error handling here
                                console.log(error);
                            } else {
                                console.log('Device updated. Now removing update file.');
                                // delete file
                                fs.unlink(file+trigger_ext, function (err) {
                                    if (err) console.log(
                                        "Cannot delete update trigger file after processing. Erorr : " + err);
                                    console.log('Successfully deleted update file after processed: ' + file + trigger_ext);
                                });
                                fs.unlink(file, function (err) {
                                    if (err) console.log(
                                        "Cannot delete update file after processing. Erorr : " + err);
                                    console.log('Successfully deleted update file after processed: ' + file);
                                });
                            }
                        });
                    } else {
                        console.log("No device found with that id " + deviceId);
                    };
                }
            });
        };
	});
	
}

update_monitor.prototype.check_update = function() {
    //console.log("Checking update");
	var files = fs.readdirSync(data_path);
	files.sort(
		function(a, b) {
			return fs.statSync(data_path + b).mtime.getTime() - 
				  fs.statSync(data_path + a).mtime.getTime();
	   });
	for (var index in files) {
		if (path.extname(files[index]) == trigger_ext) {
		    console.log("Update file found:");
		    console.log(path.basename(files[index], trigger_ext));
			update_monitor.process_update(path.join(data_path, path.basename(files[index], trigger_ext)));
		}
	};
};

update_monitor.prototype.start = function() {
	setInterval(this.check_update, 2000 );
};


exports.update_monitor = update_monitor;