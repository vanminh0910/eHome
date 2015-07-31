'use strict';
app.controller('mainController', function ($scope, roomService, $http, $timeout, $filter, $modal, cfpLoadingBar) {
    init();

    function init() {
        roomService.getRooms().then(function (data) {
            $scope.rooms = data;
        });

        roomService.getModes().then(function (data) {
            $scope.modes = data;
        });
    }

    $scope.modeClick = function(modeId) {
        //console.log(modeId);
        roomService.runMode(modeId).then(function(data) {
            console.log(data);
        });
    };
});