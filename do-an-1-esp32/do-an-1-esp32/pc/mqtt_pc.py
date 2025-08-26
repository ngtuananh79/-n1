# pc/mqtt_pc.py — client mô phỏng trên PC
import json, time, random
from paho.mqtt import client as mqtt

BROKER = "192.168.1.10"
PUB = "lab/sensor/pc"
SUB = "lab/cmd/pc"

def on_message(c, u, m):
    print("CMD:", m.topic, m.payload.decode())

cli = mqtt.Client()
cli.on_message = on_message
cli.connect(BROKER, 1883, 60)
cli.subscribe(SUB)
cli.loop_start()

while True:
    val = round(20 + 10*random.random(), 2)
    cli.publish(PUB, json.dumps({"id":"pc","value":val}))
    time.sleep(5)
