'use strict';
app.service('roomService', function ($http, toaster) {

    var serviceBase = '/api/';

    this.getRooms = function () {
        return $http.get(serviceBase + 'room').then(
            function (results) {
                return results.data;
            });
    };

    this.saveRoom = function (room) {
        return $http.put(serviceBase + 'room/' + room._id, room).then(
            function (results) {
                return results.data;
            });
    };

    this.getSwitchBoards = function () {
        return $http.get(serviceBase + 'switch_board').then(
            function (results) {
                return results.data;
            });
    };

    this.setSwitchStatus = function (deviceId, index, status) {
        return $http.put(serviceBase + 'set_switch_status',
            {
                '_id': deviceId,
                'relay_index': index,
                'status': status
            }).then(
            function (results) {
                console.log("Status posted successfully");
                console.log(results);
            },
            function (results) {
                console.log("Status posted failed");
                console.log(results);
                return results;
            });
    };

    this.getSensors = function () {
        return $http.get(serviceBase + 'sensor').then(
            function (results) {
                return results.data;
            });
    };

    this.getDoorlocks = function () {
        return $http.get(serviceBase + 'door_lock').then(
            function (results) {
                return results.data;
            });
    };

    this.setDoorStatus = function (doorId, status) {
        return $http.put(serviceBase + 'set_door_status',
            {
                '_id': doorId,
                'status': status
            }).then(
            function (results) {
                console.log("Status posted successfully");
                console.log(results);
            },
            function (results) {
                console.log("Status posted failed");
                console.log(results);
                return results;
            });
    };

    this.getCameras = function () {
        return $http.get(serviceBase + 'camera').then(
            function (results) {
                return results.data;
            });
    };

    this.getMusicModes = function () {
        return $http.get(serviceBase + 'music').then(
            function (results) {
                return results.data;
            });
    };

    this.updateMusicMode = function (mode, callback) {
        return $http.put(serviceBase + 'music/' + mode._id, mode);
    };

    this.deleteMusicMode = function (mode, callback) {
        return $http.delete(serviceBase + 'music/' + mode._id);
    };

    this.getMusicFolders = function () {
        return $http.get(serviceBase + 'music_folders').then(
            function (results) {
                return results.data;
            });
    };

    this.playMusicMode = function (modeId) {
        return $http.put(serviceBase + 'play_music_mode',
            {
                '_id': modeId,
            }).then(
            function (results) {
                console.log("Status posted successfully");
                console.log(results);
            },
            function (results) {
                console.log("Status posted failed");
                console.log(results);
                return results;
            });
    };

    this.stopMusicMode = function () {
        return $http.put(serviceBase + 'stop_music_mode', {}).then(
            function (results) {
                console.log("Status posted successfully");
                console.log(results);
            },
            function (results) {
                console.log("Status posted failed");
                console.log(results);
                return results;
            });
    };

    this.getModes = function () {
        return $http.get(serviceBase + 'mode').then(
            function (results) {
                return results.data;
            });
    };

    this.getMode = function (modeId) {
        return $http.get(serviceBase + 'mode/' + modeId).then(
            function (results) {
                return results.data;
            });
    };

    this.runMode = function (modeId) {
        console.log(modeId);
        return $http.post(serviceBase + 'run_mode/', { 'modeId': modeId }).then(
            function (results) {
                return results.data;
            });
    };

    this.updateMode = function (mode) {
        return $http.put(serviceBase + 'mode/' + mode._id, mode).then(
            function (results) {
                return results.data;
        });
    };
    /*
    var config = {
			method: 'POST',
			url: '/api/course',
			headers: {
				'Content-type': 'application/json'
			},
            data: {
                'name': $scope.name,
                'image': $scope.image,
                'description': $scope.description,
            },
        };
        $http(config)
		.success(function (data, status, headers, config) {
              if (data.status == 'OK') {
                  $scope.name = null;
                  $scope.image = null;
                  $scope.url = null;
                  $scope.messages = 'Your form has been sent!';
                  $scope.submitted = false;
              } else {
                  $scope.messages = 'Oops, we received your request, but there was an error.';
                  console.log(data);
              }
          })
          .error(function (data, status, headers, config) {
              console.log(cfpLoadingBar.status());
              $scope.progress = data;
              $scope.messages = 'There was a network error. Try again later.';
              //$log.error(data);
              console.log(data);
          });
          */
});