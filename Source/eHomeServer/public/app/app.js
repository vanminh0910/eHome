
/*######################*********************#############################
  
 Created by: Minh Ha 2015

 ######################*********************##############################*/

var app = angular.module('eHomeApp', [
        'ngRoute', 'ngResource', 'ui.router', 'ui.bootstrap', 'toaster',
        'chieffancypants.loadingBar', 'socket.io', 'ngMorris' ]).run([
            '$rootScope', '$state', '$stateParams',
            function ($rootScope, $state, $stateParams, roomService) {

                // It's very handy to add references to $state and $stateParams to the $rootScope
                // so that you can access them from any scope within your applications.For example,
                // <li ui-sref-active="active }"> will set the <li> // to active whenever
                // 'contacts.list' or one of its decendents is active.
                $rootScope.$state = $state;
                $rootScope.$stateParams = $stateParams;
            }
        ]
    );

app.config(function ($socketProvider) {
    $socketProvider.setConnectionUrl('http://localhost');
});    

app.config(function ($stateProvider, $urlRouterProvider) {
    $urlRouterProvider.when("", "/dashboard");
    $urlRouterProvider.when("/", "/dashboard");
    $urlRouterProvider.otherwise("/dashboard");
    // Now set up the states
    $stateProvider
        .state('dashboard', {
            url: "/dashboard",
            templateUrl: "app/views/dashboard.html",
            controller: "dashboardController",
        })
        .state('viewGraph', {
            url: "/viewGraph/:nodeId/:fieldId/:min/:max",
            templateUrl: "app/views/viewGraph.html",
            controller: "viewGraphController",
        })
        .state('sensor', {
            url: "/sensor",
            templateUrl: "app/views/sensor.html",
            controller: "sensorController",
        });
});