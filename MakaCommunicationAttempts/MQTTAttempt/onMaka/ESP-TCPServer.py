import paho.mqtt.client as mqtt  # import the client1
import time

def on_message(client, userdata, message):
    print("message received ", str(message.payload.decode("utf-8")))
    print("message topic=", message.topic)
    print("message qos=", message.qos)
    print("message retain flag=", message.retain)


broker_address = "127.0.0.1"
# broker_address="iot.eclipse.org"
print("creating new instance")
client = mqtt.Client("Station")  # create new instance
print("connecting to broker")
client.connect(broker_address)  # connect to broker
print("connected")
client.loop_start()
client.subscribe("GPS", qos=0)
print("subscribed to gps")




