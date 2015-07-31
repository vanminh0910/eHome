'use strict';
app.controller('roomController', function ($scope, roomService, $stateParams, $http, $timeout, $filter, $modal, cfpLoadingBar) {
    init();

    function init() {
        roomService.getRooms().then(function (data) {
            $scope.rooms = data;
        });
        roomService.getSwitchBoards().then(function (data) {
            $scope.switch_boards = data;
        });
    }

    $scope.relayClick = function (switchId, index) {
        //console.log(switchId + ":" + relayName + ":" + index);
        var swFound = $filter('filter')($scope.switch_boards, { id: switchId }, true);
        if (swFound.length) {
            // reverse status of the requested relay
            swFound[0].relay[index].status = 1 - swFound[0].relay[index].status;
            //make call to server
            roomService.setSwitchStatus(swFound[0]._id, index, swFound[0].relay[index].status);
        } else {
            alert("Invalid switch ID: " + switchId);
        }
    }
});