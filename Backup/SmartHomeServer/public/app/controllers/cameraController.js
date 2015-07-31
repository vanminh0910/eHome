'use strict';
app.controller('cameraController', function ($scope, roomService, $stateParams, $http, $timeout, $filter, $modal, cfpLoadingBar) {
    init();

    function init() {
        roomService.getCameras().then(function (data) {
            $scope.cameras = data;
        });
    }

    $scope.cameraClick = function(cameraId) {
        var cameraFound = $filter('filter')($scope.cameras, { id: cameraId }, true);
        if (cameraFound.length) {
            //popup window and redirect user to camera url
            //camera.url
            var newWindow = window.open(cameraFound[0].url, cameraFound[0].name, 'height=600,width=600,location=false');
            if (window.focus) {
                newWindow.focus();
            }
        } else {
            alert("Invalid camera ID: " + cameraId);
        }
    }
});