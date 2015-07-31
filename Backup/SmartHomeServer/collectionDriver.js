var ObjectID = require('mongodb').ObjectID;

CollectionDriver = function(db) {
  this.db = db;
};

CollectionDriver.prototype.getCollection = function(collectionName, callback) {
  this.db.collection(collectionName, function(error, the_collection) {
    if( error ) callback(error);
    else callback(null, the_collection);
  });
};

CollectionDriver.prototype.findAll = function(collectionName, callback) {
    this.getCollection(collectionName, function(error, the_collection) { //A
      if( error ) callback(error);
      else {
        the_collection.find().toArray(function(error, results) { //B
          if( error ) callback(error);
          else callback(null, results);
        });
      }
    });
};

CollectionDriver.prototype.find = function (collectionName, query, callback) {
    this.getCollection(collectionName, function (error, the_collection) { //A
        if (error) callback(error);
        else {
            the_collection.find(query).toArray(function (error, results) { //B
                if (error) callback(error);
                else callback(null, results);
            });
        }
    });
};

CollectionDriver.prototype.get = function(collectionName, id, callback) { //A
    this.getCollection(collectionName, function(error, the_collection) {
        if (error) callback(error);
        else {
            var checkForHexRegExp = new RegExp("^[0-9a-fA-F]{24}$"); //B
            if (!checkForHexRegExp.test(id)) callback({error: "Invalid id"});
            else the_collection.findOne({'_id':ObjectID(id)}, function(error,doc) { //C
                if (error) callback(error);
                else callback(null, doc);
            });
        }
    });
};

//save new object
CollectionDriver.prototype.save = function(collectionName, obj, callback) {
    this.getCollection(collectionName, function(error, the_collection) { //A
        if (error) callback(error);
        else {
            obj.created_at = new Date(); //B
            the_collection.insert(obj, function() { //C
                callback(null, obj);
            });
        }
    });
};

//update a specific object
CollectionDriver.prototype.update = function(collectionName, obj, entityId, callback) {
    this.getCollection(collectionName, function(error, the_collection) {
        if (error) callback(error);
        else {
            /*
            console.log(obj);
            the_collection.update({ _id: entityId }, {$set: obj }, function (error, doc) {
                if (error) {
                    console.log(error);
                    callback(error);
                }
                else callback(null, obj);
            });
            */
            obj._id = ObjectID(entityId);
            obj.updated_at = new Date();
            console.log("Object updated: ");
            console.log(obj);
            the_collection.save(obj, function(error, doc) {
                if (error) {
                    console.log("Error when updating object");
                    callback(error);
                }
                else callback(null, doc);
            });
        }
    });
};

//delete a specific object
CollectionDriver.prototype.delete = function(collectionName, entityId, callback) {
    this.getCollection(collectionName, function(error, the_collection) {
        if (error) {
            console.log("Got error, invoking callback now" + error);
            callback(error, null);
        }
        else {
            the_collection.remove({ '_id': ObjectID(entityId) }, function (error, doc) {
                console.log("No error, invoking callback now" + error);
                callback(error, doc);
            });
        }
    });
};

exports.CollectionDriver = CollectionDriver;