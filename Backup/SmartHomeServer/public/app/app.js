
/*######################*********************#############################
  
 Created by: 

 ######################*********************##############################*/

var app = angular.module('SimpleApp', [
        'ngRoute', 'ngResource', 'ui.router', 'ui.bootstrap', 'toaster', 'chieffancypants.loadingBar', 'dialogs'
    ])
    .run(
        [
            '$rootScope', '$state', '$stateParams', 'roomService',
            function ($rootScope, $state, $stateParams, roomService) {

                // It's very handy to add references to $state and $stateParams to the $rootScope
                // so that you can access them from any scope within your applications.For example,
                // <li ui-sref-active="active }"> will set the <li> // to active whenever
                // 'contacts.list' or one of its decendents is active.
                $rootScope.$state = $state;
                $rootScope.$stateParams = $stateParams;
                roomService.getModes().then(function (data) {
                    $rootScope.modes = data;
                });
            }
        ]
    );

app.config(function ($stateProvider, $urlRouterProvider) {
    $urlRouterProvider.when("", "/main/dashboard");
    $urlRouterProvider.when("/", "/main/dashboard");
    $urlRouterProvider.otherwise("/main/dashboard");
    // Now set up the states
    $stateProvider
        .state('main', {
            url: "/main",
            abstract: true,
            templateUrl: "app/views/main.html",
            controller: "mainController",
        })
        .state('main.dashboard', {
            url: "/dashboard",
            templateUrl: "app/views/dashboard.html",
            controller: "dashboardController",
        })
        .state('main.room', {
            url: "/room",
            templateUrl: "app/views/room.html",
            controller: "roomController",
        })
        .state('main.sensor', {
            url: "/sensor",
            templateUrl: "app/views/sensor.html",
            controller: "sensorController",
        })
        .state('main.doorlock', {
            url: "/doorlock",
            templateUrl: "app/views/doorlock.html",
            controller: "doorlockController",
        })
        .state('main.camera', {
            url: "/camera",
            templateUrl: "app/views/camera.html",
            controller: "cameraController",
        })
        .state('main.music', {
            url: "/music",
            templateUrl: "app/views/music.html",
            controller: "musicController",
        })
        .state('settings', {
            url: "/settings",
            templateUrl: "app/views/settings/settings.html",
        })
        .state('settings.room', {
            url: "/room",
            templateUrl: "/app/views/settings/room.html",
            controller: "roomSettingsController",
        })
        .state('settings.mode', {
            url: "/mode/:modeId",
            templateUrl: "/app/views/settings/mode.html",
            controller: "modeSettingsController",
        })
        .state('settings.music', {
            url: "/music/",
            templateUrl: "/app/views/settings/music.html",
            controller: "musicSettingsController",
        });
    /*
      .state('state2.list', {
          url: "/list",
          templateUrl: "/app/views/test/state2.list.html",
          controller: function ($scope) {
              $scope.things = ["A", "Set", "Of", "Things"];
          }
      }) */

    //$urlRouterProvider.otherwise("/");
});