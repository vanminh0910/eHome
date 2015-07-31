var fs = require('fs');
var path = require('path');

var sys = require('sys');
var exec = require('child_process').exec;

var music_path = "/media/USB/music/";

musicHelper = function() {

};

musicHelper.prototype.listFolders = function(callback) {
    fs.readdir(music_path, function (err, files) {
        var dirs = [];
        for (var i = 0; i < files.length; i++) {
            if (files[i][0] !== '.') { // ignore hidden
                var filePath = music_path + '/' + files[i];
                if (fs.statSync(filePath).isDirectory()) {
                    dirs.push(files[i]);
                }
            }
        }
        callback(dirs);
    });
}

musicHelper.prototype.playMusic = function (paths) {
    this.stopMusic();
    function puts(error, stdout, stderr) {
        console.log('stdout: ' + stdout);
        console.log('stderr: ' + stderr);
        if (error !== null) {
            console.log('exec error: ' + error);
        }
    }
    //TODO: Only play music files in first path item
    var childProcess = exec('python tools/player.py ' + '"' + music_path + paths[0].path + '"', puts);
    childProcess.stdout.on('data', function (data) {
        console.log('stdout: ' + data);
    });
    childProcess.stderr.on('data', function (data) {
        console.log('stdout: ' + data);
    });
    childProcess.on('close', function (code) {
        console.log('closing code: ' + code);
    });
}

musicHelper.prototype.stopMusic = function () {
    //hack, don't know why but run this many times to make sure music stops
    for (var i = 0; i < 4; i++) {
        exec("kill -9 $(ps aux | grep '[p]layer.py' | awk '{print $2}')", function (err, stdout, stderr) {
            if (stdout) {
                console.log('stdout:' + stdout);
            }
            if (stderr) {
                console.log('stderr:' + stderr);
            }
            if (err) {
                console.log(err);
            };
        });
        exec("kill -9 $(ps aux | grep '[o]mxplayer.bin' | awk '{print $2}')", function (err, stdout, stderr) {
            if (stdout) {
                console.log('stdout:' + stdout);
            }
            if (stderr) {
                console.log('stderr:' + stderr);
            }
            if (err) {
                console.log(err);
            };
        });
    };
    return;
}

exports.musicHelper = musicHelper;