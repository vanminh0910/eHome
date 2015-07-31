'use strict';
app.service('dashboardService', function ($http, toaster) {

    var serviceBase = '/api/';

    this.getAlarms = function () {
        return $http.get(serviceBase + 'alarm', {dataType: "json"}).then(
            function (results) {
                toaster.pop('success', "Data Retrieved", "Data has been retrieved!");
                return results.data;
            });
    };

    this.getEnergyStats = function () {
        return $http.get(serviceBase + 'energy_stats', { dataType: "json" }).then(
            function (results) {
                toaster.pop('success', "Data Retrieved", "Data has been retrieved!");
                return results.data;
            });
    };
});