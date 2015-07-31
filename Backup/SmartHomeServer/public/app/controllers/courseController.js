'use strict';
app.controller('courseController', function ($scope, courseService, $http, $timeout, $filter, $modal, cfpLoadingBar) {

    $scope.courses = [];

    //paging
    $scope.totalRecordsCount = 0;
    $scope.pageSize = 10;
    $scope.currentPage = 1;

    init();

    function init() {
        getCourses();
    }

    function getCourses() {

        $scope.courses = [
            {
                'id': 1, 'name': 'Effective Communication', 'image': 'http://cdn.alison-static.net/images/crs/25/276/Alison_CoursewareIntro_276.jpg',
                'description': 'Increase your understanding of psychology theories, ethical issues and much more'
            },
            {
                'id': 2, 'name': 'Assertiveness', 'image': 'http://cdn.alison-static.net/images/crs/25/276/Alison_CoursewareIntro_276.jpg',
                'description': 'Improve your management skills and learn how large-scale organisations are managed'
            },
            {
                'id': 3, 'name': 'Code Review', 'image': 'http://cdn.alison-static.net/images/crs/25/276/Alison_CoursewareIntro_276.jpg',
                'description': 'Improve your management skills and learn how large-scale organisations are managed'
            },
            {
                'id': 4, 'name': 'Negotiation', 'image': 'http://cdn.alison-static.net/images/crs/25/276/Alison_CoursewareIntro_276.jpg',
                'description': 'Improve your management skills and learn how large-scale organisations are managed'
            },
        ];

        //var paginationHeader = angular.fromJson(results.headers("x-pagination"));

        $scope.totalRecordsCount = $scope.courses.length;
    }


    $scope.doSearch = function () {

        $scope.currentPage = 1;
        getCourses();

    };

    $scope.showCourseDetail = function (courseId) {

        courseService.get({ courseId: courseId }, function (courseResult) {

            var modalInstance = $modal.open({
                templateUrl: 'app/views/placesphotos.html',
                controller: 'placesPhotosController',
                resolve: {
                    venueName: function () {
                        return venueName;
                    },
                    venuePhotos: function () {
                        return photosResult.response.photos.items;
                    }
                }
            });

            modalInstance.result.then(function () {
                //$scope.selected = selectedItem;
            }, function () {
                //alert('Modal dismissed at: ' + new Date());
            });

        });

    };

    $scope.enrollCourse = function (courseId) {

    };

    $scope.submitCourse = function (form) {
        // Trigger validation flag.
        $scope.submitted = true;
        console.log(form);

        // If form is invalid, return and let AngularJS show validation errors.
        if (form.$invalid) {
            $scope.messages = 'Please fix the error(s) and submit again.';
            return;
        }

        // Default values for the request.

        
        var config = {
			method: 'POST',
			url: '/api/course',
			headers: {
				'Content-type': 'application/json'
			},
            data: {
                'name': $scope.name,
                'image': $scope.image,
                'description': $scope.description,
            },
        };
        $http(config)
		.success(function (data, status, headers, config) {
              if (data.status == 'OK') {
                  $scope.name = null;
                  $scope.image = null;
                  $scope.url = null;
                  $scope.messages = 'Your form has been sent!';
                  $scope.submitted = false;
              } else {
                  $scope.messages = 'Oops, we received your request, but there was an error.';
                  console.log(data);
              }
          })
          .error(function (data, status, headers, config) {
              console.log(cfpLoadingBar.status());
              $scope.progress = data;
              $scope.messages = 'There was a network error. Try again later.';
              //$log.error(data);
              console.log(data);
          });
          
          

        // Hide the status message which was set above after 3 seconds.
        $timeout(function () {
            $scope.messages = null;
        }, 3000);
    };

    $scope.isLoading = function () {
        return (cfpLoadingBar.status() != 1);
    }

});