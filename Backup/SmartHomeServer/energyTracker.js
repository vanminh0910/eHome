
energyTracker = function (collectionDriver) {
    this.collectionDriver = collectionDriver;
};

// get total number of switches in home and number of switches being turned on
// callback = function(totalOn, total)
energyTracker.prototype.getSwitchStatistic = function (callback) {
    var totalOn = 0;
    var total = 0;
    this.collectionDriver.findAll('switch_board', function (error, sws) {
        if (error) { callback(0, 0); }
        else {
            for (var i = 0; i < sws.length; i++) {
                total += sws[i].relay.length;
                for (var j = 0; j < sws[i].relay.length; j++) {
                    if (sws[i].relay[j].status) {
                        totalOn++;
                    }
                }
            }
            callback(totalOn, total);
        }
    });
}

exports.energyTracker = energyTracker;