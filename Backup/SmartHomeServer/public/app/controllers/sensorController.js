'use strict';
app.controller('sensorController', function ($scope, roomService, $stateParams, $http, $timeout, $filter, $modal, cfpLoadingBar) {
    init();

    function init() {
        roomService.getSensors().then(function (data) {
            $scope.sensors = data;
        });
    }
});