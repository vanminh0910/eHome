'use strict';
app.controller('musicSettingsController', function ($scope, roomService, $stateParams, $http, $timeout, $filter, $modal, cfpLoadingBar, $log) {
    init();

    function init() {
        $scope.message = "";
        $scope.errorMsg = "";

        async.series([
            function(next) {
                roomService.getMusicModes().then(function(data) {
                    $scope.modes = data;
                    next();
                });
            },
            function(next) {
                roomService.getMusicFolders().then(function(data) {
                    $scope.folders = data;
                });
            }
        ]);
    };

    $scope.saveMode = function (mode) {
        roomService.updateMusicMode(mode).then(function (results) {
            setMessage('Mode update successfully', false);
            mode = results.data;
            console.log(mode);
        });
    };

    $scope.deleteMode = function (mode) {
        $http({
            method: 'DELETE',
            url: '/api/music/' + mode._id
        }).
        success(function (results) {
            setMessage('Mode deleted successfully', false);
            roomService.getMusicModes().then(function (data) {
                $scope.modes = data;
            });
        })
        .error(function (error) {
            console.log(error);
            setMessage('Fail to delete mode. Error: ' + error, true);
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

    $scope.items = ['item1', 'item2', 'item3'];

    $scope.addMode = function (size) {
        var modalInstance = $modal.open({
            templateUrl: 'myModalContent.html',
            controller: ModalInstanceCtrl,
            size: size,
            resolve: {
                items: function () {
                    return $scope.items;
                }
            }
        });

        modalInstance.result.then(function (selectedItem) {
            //$scope.selected = selectedItem;
            console.log("Modal closed: " + selectedItem);
        }, function (reason) {
            console.log("Modal dismissed: " + reason);
        });
        return;
    };
});

var ModalInstanceCtrl = function ($scope, $modalInstance, items) {

    $scope.items = items;
    $scope.selected = {
        item: $scope.items[0]
    };

    $scope.ok = function () {
        //$modalInstance.close($scope.selected.item);
        $modalInstance.close("AAAAAAAAAAAAa");
    };

    $scope.cancel = function () {
        $modalInstance.dismiss('BBBBBBBBBB');
    };
};
