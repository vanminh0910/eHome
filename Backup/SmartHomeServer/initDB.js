init_db = function(collectionDriver) {
	if (init_db) {
		// init mongodb database
		// rooms
		collectionDriver.getCollection("room", function(error, collection) {
			if( error ) {
				console.log(error);
			} else {
				collection.remove(function(err, removedCount) {
					//your next actions
					console.log(err);
				});
			}
		});
		collectionDriver.save("room",
			{
				"id": 600,
				"name": "Livingroom",
                "image": "livingroom.jpg"
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add room: "); 
					console.log(err);
				} else {
					console.log("New room added: "); 
					console.log(docs); 
				}
			});
		collectionDriver.save("room",
			{
				"id": 601,
				"name": "Kitchen",
				"image": "kitchen.jpg"
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add room: "); 
					console.log(err);
				} else {
					console.log("New room added: "); 
					console.log(docs); 
				}
			});

		collectionDriver.save("room",
			{
				"id": 602,
				"name": "Bedroom1",
				"image": "bedroom.jpg"
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add room: "); 
					console.log(err);
				} else {
					console.log("New room added: "); 
					console.log(docs); 
				}
			});
		collectionDriver.save("room",
			{
			    "id": 603,
			    "name": "Bedroom",
			    "image": "bedroom.jpg"
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add room: ");
			        console.log(err);
			    } else {
			        console.log("New room added: ");
			        console.log(docs);
			    }
			});
		collectionDriver.save("room",
			{
			    "id": 604,
			    "name": "WC",
			    "image": "bathroom.jpg"
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add room: ");
			        console.log(err);
			    } else {
			        console.log("New room added: ");
			        console.log(docs);
			    }
			});
		collectionDriver.save("room",
			{
			    "id": 605,
			    "name": "Balcony",
			    "image": "balcony.png"
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add room: ");
			        console.log(err);
			    } else {
			        console.log("New room added: ");
			        console.log(docs);
			    }
			});
		// switch_boards
		collectionDriver.getCollection("switch_board", function(error, collection) {
			if( error ) {
				console.log(error);
			} else {
				collection.remove(function(err, removedCount) {
					//your next actions
					console.log(err);
				});
			}
		});
		collectionDriver.save("switch_board",
			{
				"id": 2,
				"type": 13,
				"relay": [
					{"name":"Den tron", "status": 0},
					{"name":"Den tron", "status": 0},
					{ "name": "Den tron", "status": 0 }
				],
                "room": 600
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add switch board: "); 
					console.log(err);
				} else {
					console.log("New switch board added: "); 
					console.log(docs); 
				}
			});
			
		collectionDriver.save("switch_board",
			{
				"id": 3,
				"type": 12,
				"relay": [
					{ "name": "Den tron", "status": 0 },
					{ "name": "Den tron", "status": 1 },
				],
				"room": 601
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add switch board: "); 
					console.log(err);
				} else {
					console.log("New switch board added: "); 
					console.log(docs); 
				}
			});
		collectionDriver.save("switch_board",
			{
				"id": 4,
				"type": 13,
				"relay": [
				    { "name": "Den tron", "status": 0 },
				    { "name": "Den dai", "status": 1 },
				    { "name": "Den tron", "status": 0 },
				],
				"room": 602
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add switch board: "); 
					console.log(err);
				} else {
					console.log("New switch board added: "); 
					console.log(docs); 
				}
			});

		collectionDriver.save("switch_board",
			{
			    "id": 201,
			    "type": 20,
				"relay": [
				    { "name": "Den tron", "status": 0 },
				],
				"room": 603
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add pir switch: "); 
					console.log(err);
				} else {
					console.log("New pir switch added: "); 
					console.log(docs); 
				}
			});
		collectionDriver.save("switch_board",
			{
			    "id": 203,
			    "type": 20,
			    "relay": [
			        { "name": "Den wc", "status": 1 }
			    ],
			    "room": 603
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add pir switch: ");
			        console.log(err);
			    } else {
			        console.log("New pir switch added: ");
			        console.log(docs);
			    }
			});
		collectionDriver.save("switch_board",
			{
			    "id": 204,
			    "type": 20,
			    "relay": [
			        { "name": "Den wc2", "status": 1 }
			    ],
			    "room": 604
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add pir switch: ");
			        console.log(err);
			    } else {
			        console.log("New pir switch added: ");
			        console.log(docs);
			    }
			});
		//sensor
		collectionDriver.getCollection("sensor", function(error, collection) {
			if( error ) {
				console.log(error);
			} else {
				collection.remove(function(err, removedCount) {
					//your next actions
					console.log(err);
				});
			}
		});
		collectionDriver.save("sensor",
			{
				"id": 300,
				"type": 51,
				"name": "Nhiet do phong",
				"value1": 100,
				"value2": 200,
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add sensor: "); 
					console.log(err);
				} else {
					console.log("New sensor added: "); 
					console.log(docs); 
				}
			});
		collectionDriver.save("sensor",
			{
				"id": 301,
				"type": 52,
				"name": "Do am vuon",
				"value1": 700,
				"value2": 0,
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add sensor: "); 
					console.log(err);
				} else {
					console.log("New sensor added: "); 
					console.log(docs); 
				}
			});
		collectionDriver.save("sensor",
			{
				"id": 302,
				"type": 53,
				"name": "Cam bien gas",
				"value1": 1,
				"value2": 1,
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add sensor: "); 
					console.log(err);
				} else {
					console.log("New sensor added: "); 
					console.log(docs); 
				}
			});
			
		collectionDriver.getCollection("door_lock", function(error, collection) {
			if( error ) {
				console.log(error);
			} else {
				collection.remove(function(err, removedCount) {
					//your next actions
					console.log(err);
				});
			}
		});
		collectionDriver.save("door_lock",
			{
			    "id": 400,
			    "type": 60,
				"name": "Cua chinh",
				"status": 0,
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add door lock: "); 
					console.log(err);
				} else {
					console.log("New door lock added: "); 
					console.log(docs); 
				}
			});
		
		collectionDriver.getCollection("camera", function(error, collection) {
			if( error ) {
				console.log(error);
			} else {
				collection.remove(function(err, removedCount) {
					//your next actions
					console.log(err);
				});
			}
		});
		collectionDriver.save("camera",
			{
			    "id": 500,
			    "type": 80,
				"name": "Camera trong nha",
				"url": "http://google.com.vn",
			},
			function(err, docs) {
				if (err) {
					console.log("Cannot add camera: "); 
					console.log(err);
				} else {
					console.log("New camera added: "); 
					console.log(docs); 
				}
			});
		
        // music data
		collectionDriver.getCollection("music", function (error, collection) {
		    if (error) {
		        console.log(error);
		    } else {
		        collection.remove(function (err, removedCount) {
		            //your next actions
		            console.log(err);
		        });
		    }
		});
		collectionDriver.save("music",
			{
			    "id": 700,
			    "type": 100,
			    "name": "Romance",
			    "folders": [{ 'path': 'romance'}],
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add music mode: ");
			        console.log(err);
			    } else {
			        console.log("New music mode added: ");
			        console.log(docs);
			    }
			});

		collectionDriver.save("music",
			{
			    "id": 701,
			    "type": 100,
			    "name": "Red",
			    "folders": [{ 'path': 'red' }],
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add music mode: ");
			        console.log(err);
			    } else {
			        console.log("New music mode added: ");
			        console.log(docs);
			    }
			});

		collectionDriver.save("music",
			{
			    "id": 703,
			    "type": 100,
			    "name": "Rock",
			    "folders": [{ 'path': 'rock' }],
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add music mode: ");
			        console.log(err);
			    } else {
			        console.log("New music mode added: ");
			        console.log(docs);
			    }
			});

	    // mode data
		collectionDriver.getCollection("mode", function (error, collection) {
		    if (error) {
		        console.log(error);
		    } else {
		        collection.remove(function (err, removedCount) {
		            //your next actions
		            console.log(err);
		        });
		    }
		});
		collectionDriver.save("mode",
			{
			    "id": 1,
			    "name": "Night",
			    "commands": [],
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add mode 1: ");
			        console.log(err);
			    } else {
			        console.log("New mode added: ");
			        console.log(docs);
			    }
			});

		collectionDriver.save("mode",
			{
			    "id": 2,
			    "name": "Sleep",
			    "commands": [],
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add mode 2: ");
			        console.log(err);
			    } else {
			        console.log("New mode added: ");
			        console.log(docs);
			    }
			});

		collectionDriver.save("mode",
			{
			    "id": 3,
			    "name": "Go Out",
			    "commands": [],
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add mode 3: ");
			        console.log(err);
			    } else {
			        console.log("New mode added: ");
			        console.log(docs);
			    }
			});

		collectionDriver.save("mode",
			{
			    "id": 4,
			    "name": "Wake Up",
			    "commands": [],
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add mode 4: ");
			        console.log(err);
			    } else {
			        console.log("New mode added: ");
			        console.log(docs);
			    }
			});

		collectionDriver.save("mode",
			{
			    "id": 5,
			    "name": "Dinner",
			    "commands": [],
			},
			function (err, docs) {
			    if (err) {
			        console.log("Cannot add mode 5: ");
			        console.log(err);
			    } else {
			        console.log("New mode added: ");
			        console.log(docs);
			    }
			});
	}
};

exports.init_db = init_db;