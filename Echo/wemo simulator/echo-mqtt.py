#!/usr/bin/env python
import subprocess
import paho.mqtt.client as mqtt
import fauxmo
from debounce_handler import debounce_handler
import threading
import logging
import time
logging.basicConfig(level=logging.DEBUG)

DEVICES = {
    "first light": {
        'port': 52002,
        'on_command_topic': '/easytech.vn/LivingRoom/Light1/Command',
        'on_command_message': '1',
        'off_command_topic': '/easytech.vn/LivingRoom/Light1/Command',
        'off_command_message': '0',
    },
    "second light": {
        'port': 52003,
        'on_command_topic': '/easytech.vn/LivingRoom/Light2/Command',
        'on_command_message': '1',
        'off_command_topic': '/easytech.vn/LivingRoom/Light2/Command',
        'off_command_message': '0',
    },
    "kitchen first light": {
        'port': 52004,
        'on_command_topic': '/easytech.vn/LivingRoom/Light3/Command',
        'on_command_message': '1',
        'off_command_topic': '/easytech.vn/LivingRoom/Light3/Command',
        'off_command_message': '0',
    },
    "kitchen second light": {
        'port': 52005,
        'on_command_topic': '/easytech.vn/LivingRoom/Light4/Command',
        'on_command_message': '1',
        'off_command_topic': '/easytech.vn/LivingRoom/Light4/Command',
        'off_command_message': '0',
    },
    "TV": {
        'port': 52006,
        'on_command_topic': '/IR1/1',
        'on_command_message': '1',
        'off_command_topic': '/IR1/1',
        'off_command_message': '0',
    },
    "AC": {
        'port': 52007,
        'on_command_topic': '/IR1/2',
        'on_command_message': '1',
        'off_command_topic': '/IR1/2',
        'off_command_message': '0',
    },
    "fan": {
        'port': 52008,
        'on_command_topic': '/IR1/3',
        'on_command_message': '1',
        'off_command_topic': '/IR1/3',
        'off_command_message': '0',
    },
}
# ---------- Network constants -----------
MQTT_HOST = "192.168.43.55"
MQTT_PORT = 1883

# ---------- Device callback functions ----------
class device_handler(debounce_handler):
    """Publishes state to two different lighting MQTT topics
       depending on which Echo the request came from.
    """
    

    def __init__(self, mqtt, device_info):
        debounce_handler.__init__(self)
        self.mqtt = mqtt
        self.device_info = device_info

    def act(self, client_address, state):
        topic = ''
        message = ''
        if state:
            topic = self.device_info['on_command_topic']
            message = self.device_info['on_command_message']
        else:
            topic = self.device_info['off_command_topic']
            message = self.device_info['off_command_message']
        print topic
        print message
        self.mqtt.publish(topic, message)
        return True

if __name__ == "__main__":
    # Startup the MQTT client in a separate thread
    client = mqtt.Client()
    client.connect(MQTT_HOST, MQTT_PORT, 60)
    ct = threading.Thread(target=client.loop_forever)
    ct.daemon = True
    ct.start()

    # Startup the fauxmo server
    fauxmo.DEBUG = True
    p = fauxmo.poller()
    u = fauxmo.upnp_broadcast_responder()
    u.init_socket()
    p.add(u)

    for name, device_info in DEVICES.items():
        # Register each device callback as a fauxmo handler
        h = device_handler(client, device_info)
        fauxmo.fauxmo(name, u, p, None, device_info['port'], h)

    # Loop and poll for incoming Echo requests
    logging.debug("Entering fauxmo polling loop")
    while True:
        try:
            # Allow time for a ctrl-c to stop the process
            p.poll(100)
            time.sleep(0.1)
        except Exception, e:
            logging.critical("Critical exception: " + str(e))
            break