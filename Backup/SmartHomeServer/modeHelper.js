var async = require('async');
//require('./send_command').send_command;
//send_command = new send_command();

modeHelper = function (collectionDriver) {
    this.collectionDriver = collectionDriver;
};

modeHelper.prototype.runMode = function (modeId, callback) {
    var me = this;
    this.collectionDriver.get('mode', modeId, function (error, result) {
        var cmdsExecuted = [];
        if (error == null) {
            async.forEach(result.commands, function (cmd, cb) {
                // run command
                if (cmd.type <= 30) { // all switch types
                    me.collectionDriver.find('switch_board', { 'id': cmd.id }, function (err, swFound) {
                        if (swFound.length) {
                            for (var j = 0; j < result.commands.length; j++) {
                                if (result.commands[j].id == swFound[0].id) {
                                    for (var k = 0; k < result.commands[j].relay.length; k++) {
                                        if (result.commands[j].relay[k].status < 2)
                                            swFound[0].relay[k].status = result.commands[j].relay[k].status;
                                    }
                                    cmdsExecuted.push(swFound[0]);
                                    break;
                                }
                            }
                        } else {
                            console.log("Invalid switch ID: " + cmd.id);
                        }
                        cb();
                    });
                } else if (cmd.type == 60) { // door locks
                    me.collectionDriver.find('door_lock', { 'id': cmd.id }, function (err, doorFound) {
                        if (doorFound.length) {
                            //console.log('Door found: ');
                            //console.log(doorFound);
                            for (var j = 0; j < result.commands.length; j++) {
                                if (result.commands[j].id == doorFound[0].id) {
                                    if (result.commands[j].status < 2) {
                                        doorFound[0].status = result.commands[j].status;
                                        cmdsExecuted.push(doorFound[0]);
                                    }
                                    break;
                                }
                            }
                        } else {
                            console.log("Invalid door lock ID: " + cmd.id);
                        }
                        cb();
                    });

                } else if (cmd.type == 100) { // music mode
                    me.collectionDriver.find('music', { 'id': cmd.id }, function (err, musicModeFound) {
                        if (musicModeFound.length) {
                            //console.log('Music mode found: ');
                            //console.log(musicModeFound);
                            for (var j = 0; j < result.commands.length; j++) {
                                if (result.commands[j].id == musicModeFound[0].id) {
                                    if (result.commands[j].status < 2) {
                                        musicModeFound[0].status = result.commands[j].status;
                                        cmdsExecuted.push(musicModeFound[0]);
                                    }
                                    break;
                                }
                            }
                        } else {
                            console.log("Invalid music mode ID: " + cmd.id);
                        }
                        cb();
                    });
                }
            }, function (err) {
                if (err) callback(err, result);
                else {
                    callback(null, cmdsExecuted);
                }
            });
        } else {
            console.log(error);
            callback(error, result);
        }
        
    });
}


exports.modeHelper = modeHelper;