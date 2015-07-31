var async = require('async');
var param1 = 'foobar';
function withParams(param1, callback) {
    console.log('withParams function called');
    console.log(param1);
    callback(null, "onewith");
}
function withoutParams(callback) {
    console.log('withoutParams function called');
    callback(null, "twowithout");
}
async.series([
  function (callback) {
      withParams(param1, callback);
  },
  withoutParams
], function (err, results) {
    console.log('all functions complete');
    console.log(results);
})