'use strict';
app.controller('modeSettingsController', function ($scope, roomService, $stateParams, $http, $timeout, $filter, $modal, cfpLoadingBar) {
    init();

    function init() {
        $scope.message = "";
        $scope.errorMsg = "";
        $scope.mode = {};

        async.series([
            function (callback) {
                roomService.getRooms().then(function (data) {
                    $scope.rooms = data;
                    callback();
                });
            },
            function (callback) {
                roomService.getSwitchBoards().then(function (data) {
                    angular.forEach(data, function (sw, key) {
                        angular.forEach(sw.relay, function (relay, k) {
                            relay.status = 2; //2: ignore status
                        });
                    });
                    $scope.switch_boards = data;
                    callback();
                });
            },
            function (callback) {
                roomService.getDoorlocks().then(function (data) {
                    angular.forEach(data, function (door, key) {
                        door.status = 2; //2: ignore status
                    });
                    $scope.doorlocks = data;
                    callback();
                });
            },
            function (callback) {
                roomService.getMusicModes().then(function (data) {
                    angular.forEach(data, function (musicMode, key) {
                        musicMode.status = 0; //0: ignore status
                    });
                    $scope.musicModes = data;
                    callback();
                });
            }
        ],
        // optional callback
        function (err, results) {
            roomService.getMode($stateParams.modeId).then(function (data) {
                $scope.mode = data;
                angular.forEach($scope.mode.commands, function (cmd, key) {
                    if (cmd.type <= 30) { // all switch types
                        var swFound = $filter('filter')($scope.switch_boards, { id: cmd.id }, true);
                        if (swFound.length) {
                            angular.forEach(cmd.relay, function (relay, relayKey) {
                                if (relay.status < 2)
                                    swFound[0].relay[relayKey].status = relay.status;
                            });
                        } else {
                            alert("Invalid switch ID: " + cmd.id);
                        }
                    } else if (cmd.type == 60) { // door locks
                        var doorFound = $filter('filter')($scope.doorlocks, { id: cmd.id }, true);
                        if (doorFound.length) {
                            if (cmd.status < 2)
                                doorFound[0].status = cmd.status;
                        } else {
                            alert("Invalid door lock ID: " + cmd.id);
                        }
                    } else if (cmd.type == 100) { // music mode
                        var musicModeFound = $filter('filter')($scope.musicModes, { id: cmd.id }, true);
                        if (musicModeFound && musicModeFound.length) {
                            if (cmd.status < 2)
                                musicModeFound[0].status = cmd.status;
                        } else {
                            alert("Invalid music mode ID: " + cmd.id);
                        }
                    }
                });
            });
        });
    }

    $scope.relayClick = function (switchId, index) {
        var swFound = $filter('filter')($scope.switch_boards, { id: switchId }, true);
        if (swFound.length) {
            // reverse status of the requested relay
            swFound[0].relay[index].status++;
            if (swFound[0].relay[index].status > 2)
                swFound[0].relay[index].status = 0;
        } else {
            alert("Invalid switch ID: " + switchId);
        }
    }

    $scope.doorClick = function (doorId) {
        var doorFound = $filter('filter')($scope.doorlocks, { id: doorId }, true);
        if (doorFound.length) {
            // reverse status of the requested door
            doorFound[0].status++;
            if (doorFound[0].status > 2)
                doorFound[0].status = 0;
        } else {
            alert("Invalid door ID: " + doorId);
        }
    }

    $scope.modeClick = function (modeId) {
        var modeFound = $filter('filter')($scope.musicModes, { id: modeId }, true);
        if (modeFound.length) {
            if (modeFound[0].status == undefined)
                modeFound[0].status = 0; //0: ignore status
            modeFound[0].status = 1 - modeFound[0].status;
            console.log(modeFound[0].status);
        } else {
            alert("Invalid music mode ID: " + modeId);
        }
    };

    $scope.saveMode = function () {
        var rules = [];
        angular.forEach($scope.switch_boards, function (sw, key) {
            // check if any change in sw status, if yes, we save it for this mode
            for (var i = 0; i < sw.relay.length; i++) {
                if (sw.relay[i].status < 2) {
                    // save it due to at least one change found
                    rules.push(sw);
                    break;
                }
            }
            
        });
        angular.forEach($scope.doorlocks, function (door, key) {
            if (door.status > 2)
                rules.push(door);
        });
        angular.forEach($scope.musicModes, function (musicMode, key) {
            if (musicMode.status == 1)
                rules.push(musicMode);
        });
        console.log($scope.mode);
        $scope.mode.commands = rules;
        console.log($scope.mode);
        roomService.updateMode($scope.mode).then(function (data) {
            setMessage('Mode update successfully', false);
            console.log('Mode update successfully');
        });
    };

    function setMessage(msg, isError) {
        if (!isError)
            $scope.message = msg;
        else
            $scope.errorMsg = msg;
        $timeout(function (){
            $scope.message = "";
            $scope.errorMsg = "";
        }, 5000);
    };
});
