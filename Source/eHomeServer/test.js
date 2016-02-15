
// *************** configure MQTT ********************
var mqtt    = require('mqtt');
var mqttClient  = mqtt.connect('mqtt://192.168.1.114');
var mqttTopicTag = 'eHome/OUT/';

mqttClient.on('connect', function () {
    console.log("Connected");
  mqttClient.subscribe('presence');
  mqttClient.publish(mqttTopicTag + '2/1', '25.5');
  mqttClient.publish(mqttTopicTag + '2/4', '3112');
});
 
mqttClient.on('message', function (topic, message) {
  // message is Buffer 
  console.log(message.toString());
  //mqttClient.end();
});
