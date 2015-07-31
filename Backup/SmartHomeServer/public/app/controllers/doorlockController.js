'use strict';
app.controller('doorlockController', function ($scope, roomService, $stateParams, $http, $timeout, $filter, $modal, cfpLoadingBar) {
    init();

    function init() {
        roomService.getDoorlocks().then(function (data) {
            $scope.doorlocks = data;
        });
    }

    $scope.doorClick = function(doorId) {
        var doorFound = $filter('filter')($scope.doorlocks, { id: doorId }, true);
        if (doorFound.length) {
            // reverse status of the requested door
            doorFound[0].status = 1 - doorFound[0].status;
            //make call to server
            roomService.setDoorStatus(doorFound[0]._id, doorFound[0].status);
        } else {
            alert("Invalid door ID: " + doorId);
        }
    }
});