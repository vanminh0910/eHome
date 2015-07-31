'use strict';
app.controller('viewGraphController', function ($scope, $http, $timeout, $filter, cfpLoadingBar, $socket, $stateParams) {
    
    var serviceBase = '/api/';

    init();

    function init() {
        $scope.nodeId = $stateParams.nodeId;
        $scope.fieldId = $stateParams.fieldId;
        $scope.min = $stateParams.min;
        $scope.max = $stateParams.max;
        $http.get(serviceBase + 'getData/' + $scope.nodeId + '/' + $scope.fieldId, {dataType: "json"}).then(
            function (results) {
                //console.log(results);
                $scope.xkey = 'date';  
                $scope.ykeys = ['data'];
                $scope.labels = ['data' + $scope.fieldId];
                $scope.sensorData = results.data.data;                
            }
        );
        //window.setInterval(function () {
            //update graph data
        //}, 2000);

    }

    window.initialize = function () {
        // code to execute after your JS file referenced in the loadScript function loads

    }


    $scope.$on('$viewContentLoaded', function () {

    });

});