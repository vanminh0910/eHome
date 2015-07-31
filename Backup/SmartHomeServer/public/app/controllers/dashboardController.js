'use strict';
app.controller('dashboardController', function ($scope, dashboardService, $http, $timeout, $filter, $modal, cfpLoadingBar) {
    init();

    function init() {
        $scope.date = new Date();
        dashboardService.getAlarms().then(function (data) {
                $scope.alarmCount = data.length;
            },
            function (results) {
                $scope.alarmCount = -1;
                if (results.status == 304) {
                    //error
                }
                else {
                    //error
                }
                return results;
            }
        );

        dashboardService.getEnergyStats().then(
            function (data) {
                $scope.energyStats = data;
                $scope.usageRate = Math.round($scope.energyStats.totalOn * 100 / $scope.energyStats.total);
            },
            function (results) {
                $scope.energyStats = null;
            }
        );

        window.setInterval(function () {
            $scope.date = new Date();
            $scope.$apply();
        }, 1000);

    }

    window.initialize = function () {
        // code to execute after your JS file referenced in the loadScript function loads

    }


    var loadScript = function () {
        //if ("geolocation" in navigator) {
        //    $('.js-geolocation').show();
        //} else {
        //    $('.js-geolocation').hide();
        //};

        /* Where in the world are you? */
        //$('.js-geolocation').on('click', function () {
        //    navigator.geolocation.getCurrentPosition(function (position) {
        //        loadWeather(position.coords.latitude + ',' + position.coords.longitude); //load weather using your lat/lng coordinates
        //    });
        //});

        loadWeather('HO CHI MINH CITY', ''); //@params location, woeid

        function loadWeather(location, woeid) {
            $.simpleWeather({
                location: location,
                woeid: woeid,
                unit: 'c',
                success: function (weather) {
                    $scope.weather = weather;
                },
                error: function (error) {
                    $("#weather").html('<p>' + error + '</p>');
                }
            });
        }
    };

    $scope.$on('$viewContentLoaded', function () {
        loadScript();
    });

    $scope.getGaugeClass = function () {
        if ($scope.usageRate < 50)
            return 'col-xs-5 well dash dash-green';
        else if ($scope.usageRate < 75)
            return 'col-xs-5 well dash dash-yellow';
        else
            return 'col-xs-5 well dash dash-red';
    }

});