fs = require("fs");

file = function() {

};

file.prototype.sendSwitchCommand = function(switchId, relayIndex, status)
{
    // write a record to command file
    // record structure: 
    // 4 first bytes: switch ID
    // 4 next bytes: relay index
    // 4 next bytes: status to set
    var bytes = [100, 1, 1, 1, 1];
    var b = new Buffer(bytes.length);
    for (var i = 0; i < bytes.length; i++) {
        b[i] = bytes[i];
    }

    fs.writeFile("test", b, "binary", function (err) {
        if (err) {
            console.log(err);
        } else {
            console.log("The file was saved!");
        }
    });
};

exports.file = file;
