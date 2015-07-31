var fs = require('fs');
var path = require('path');

//var data_path = "/home/pi/ftp/files/rf24/examples/command/";
var data_path = "./command/";
var trigger_ext = ".trigger";

send_command = function() {

};

send_command.prototype.add_command = function (device) {   
    var content = device.id + ",";
    var i = 0;
    if (device.type <= 30) { // all switch types
        for (i = 0; i < device.relay.length; i++) {
            content += device.relay[i].status + ",";
        };
    } else if (device.type == 60) { // door locks
        content += device.status + ",";
        i++;
    }
    while (i < 6) {
        content += "0,";
        i++;
    }
    console.log("File command content is: " + content);
    var fileName = data_path + Date.now();
    fs.writeFile(fileName, content, function (err) {
        if (err) {
            console.log("Cannot add command file: " + fileName + ". Error: " + err);
        } else {
            console.log("Saved command file: " + fileName);
            fs.writeFile(fileName + trigger_ext, content, function (err) {
                if (err) {
                    console.log("Cannot add command trigger file: " + fileName + +trigger_ext + ". Error: " + err);
                } else {
                    console.log("Saved command trigger file: " + fileName + trigger_ext);
                }
            });
        }
    });
}

exports.send_command = send_command;