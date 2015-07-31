'use strict';
app.controller('musicController', function ($scope, roomService, $stateParams, $http, $timeout, $filter, $modal, cfpLoadingBar) {
    init();

    function init() {
        roomService.getMusicModes().then(function (data) {
            $scope.musicModes = data;
        });
    };

    $scope.modeClick = function(modeId) {
        var modeFound = $filter('filter')($scope.musicModes, { id: modeId }, true);
        if (modeFound.length) {
            //make call to server
            roomService.playMusicMode(modeFound[0]._id);
        } else {
            alert("Invalid music mode ID: " + modeId);
        }
    };

    $scope.stopMusic = function() {
        //make call to server
        roomService.stopMusicMode();
    };
});