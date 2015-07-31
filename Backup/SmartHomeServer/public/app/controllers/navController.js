'use strict';
app.controller('navController', function ($scope, roomService, $location, cfpLoadingBar) {
    init();

    function init() {
        roomService.getModes().then(function (data) {
            $scope.mainModes = data;
        });
    }
});