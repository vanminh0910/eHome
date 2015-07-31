'use strict';
app.controller('dashboardController', function ($scope, $http, $timeout, $filter, cfpLoadingBar, toaster, $socket) {

    var serviceBase = '/api/';

    init();

    function init() {
        $scope.groups = [
            { id: 1, name: 'Living room'},
            { id: 2, name: 'Bedroom1'},
            { id: 3, name: 'Bedroom2'},
            { id: 4, name: 'Kitchen'},
            { id: 4, name: 'Kitchen'}];
        $scope.temp1 = 35;
        $scope.humid1 = "80%";
        $scope.volt1 = 3.3;

        $http.get(serviceBase + 'getCurrentData', {dataType: "json"}).then(
            function (results) {
                toaster.pop('success', "Data Retrieved", "Data has been retrieved!");
                console.log(results.data);
            }
        );


        $socket.on('update', function (data) {
            console.log("Receive update data from server: ");
            console.log(data);
            //$scope.serverResponse = data;
            //$socket.emit('clientUpdate', data);
            $scope.temp1 = data.data2;
            $scope.humid1 = data.data1;
            $scope.vol1 = data.data3;
            $scope.lastUpdate = data.date;
        });
        
        
        
        $scope.emitBasic = function emitBasic() {
            console.log('echo event emited');
            $socket.emit('echo', $scope.dataToSend);
            $scope.dataToSend = '';
        };

        //window.setInterval(function () {
            //update graph data
        //}, 2000);
        
        $scope.saveNode = function() {
            alert($scope.nodeId);
            $http.post(serviceBase + 'node',
            {
                'nodeId': $scope.nodeId,
                'nodeName': $scope.nodeName,
                'dataField': $scope.dataField,
                'group': $scope.group,
                'visible': $scope.visible,
            }).then(
            function (results) {
                console.log("Node posted successfully");
                console.log(results);
            },
            function (results) {
                console.log("Node posted failed");
                console.log(results);
                return results;
            });
        }

    }

    window.initialize = function () {
        // code to execute after your JS file referenced in the loadScript function loads

    }


    $scope.$on('$viewContentLoaded', function () {

    });

});