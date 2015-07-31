'use strict';
app.controller('roomSettingsController', function($scope, roomService, $stateParams, $http, $timeout, $filter, $modal, cfpLoadingBar) {
    init();

    function init() {
        roomService.getRooms().then(function(data) {
            $scope.rooms = data;
        });
    }

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

    $scope.saveRoom = function(room) {
        //console.log(room);
        roomService.saveRoom(room).then(function(data) {
            //$scope.rooms = data;
            console.log(data);
        });
    };

    $scope.deleteRoom = function (room) {
        $scope.opts = {
            backdrop: true,
            backdropClick: true,
            dialogFade: false,
            keyboard: true,
            templateUrl: 'app/views/settings/confirmation.html',
            controller: 'deleteRoomController',
            resolve: {
                'room': function() {
                    return room;
                }
            }
        };

        var modalInstance = $modal.open($scope.opts);

        modalInstance.result.then(function () {
            //on ok button press 
            console.log('ok in event');
        }, function () {
            //on cancel button press
            console.log("Modal Closed");
        });
        //roomService.saveRoom(room).then(function (data) {
        //    //$scope.rooms = data;
        //    console.log(data);
        //});
    };

    $scope.addRoom = function () {
        $scope.name = 'theNameHasBeenPassed';
        $scope.opts = {
            backdrop: true,
            backdropClick: true,
            dialogFade: false,
            keyboard: true,
            templateUrl: 'app/views/settings/addroom.html',
            controller: ModalInstanceCtrl,
            resolve: {} // empty storage
        };


        $scope.opts.resolve.item = function () {
            return angular.copy({ name: $scope.name }); // pass name to Dialog
        }

        var modalInstance = $modal.open($scope.opts);

        modalInstance.result.then(function () {
            //on ok button press 
            console.log('ok');
        }, function () {
            //on cancel button press
            console.log("Modal Closed");
        });
    };
});

app.controller('addRoomController', function ($scope, $modalInstance, item) {
    $scope.item = item;

    $scope.ok = function () {
        console.log('ok');
        $modalInstance.close();
    };

    $scope.cancel = function () {
        console.log('cancel');
        $modalInstance.dismiss('cancel');
    };
});

var ModalInstanceCtrl = function($scope, $modalInstance, $modal, item) {

    $scope.item = item;

    $scope.ok = function() {
        $modalInstance.close();
    };

    $scope.cancel = function() {
        $modalInstance.dismiss('cancel');
    };
};

app.controller('deleteRoomController', function ($scope, $modalInstance, room) {
    $scope.room = room;
    $scope.ok = function () {
        console.log('ok in controller');
        $modalInstance.close();
    };

    $scope.cancel = function () {
        $modalInstance.dismiss('cancel');
    };
});
