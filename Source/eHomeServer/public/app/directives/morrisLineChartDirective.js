'use strict';
app.directive('lineChartMorris', function() {

    return {
        restrict: 'A',
        scope: {
          lineData: '=',
          lineXkey: '@',
          lineYkeys: '@',
          lineLabels: '@',
          lineColors: '@'
        },
        link: function (scope, elem, attrs){
          var colors;
          if (scope.lineColors === void 0 || scope.lineColors === '') {
            colors = null;
          } else {
            colors = JSON.parse(scope.lineColors);
          }
          scope.$watch('lineData', function(){
            if(scope.lineData){         
              if(!scope.morris) {
                scope.morris = new Morris.Line({
                  element: elem,
                  data: scope.lineData,
                  xkey: scope.lineXkey,
                  ykeys: JSON.parse(scope.lineYkeys),
                  //labels: JSON.parse(scope.lineLabels),
                  pointSize: 50,
                  lineColors: colors || ['#0b62a4', '#7a92a3', '#4da74d', '#afd8f8', '#edc240', '#cb4b4b', '#9440ed']
                });
              } else {
                scope.morris.setData(scope.lineData);
              }
            }
          });
        }
  };

    return {

        // required to make it work as an element
        restrict: 'E',
        template: '<div></div>',
        replace: true,
        // observe and manipulate the DOM
        link: function($scope, element, attrs) {
            console.log("aaaa");
            var data = $scope[attrs.data],
                xkey = $scope[attrs.xkey],
                ykeys= $scope[attrs.ykeys],
                labels= $scope[attrs.labels];

            Morris.Line({
                element: element,
                data: data,
                xkey: xkey,
                ykeys: ykeys,
                labels: labels
            });
        }
    };

});

app.directive('mySharedScope', function () {
    return {
        template: 'Name: Minh<br /> Street: customer.street'
    };
});