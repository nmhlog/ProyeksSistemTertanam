import paho.mqtt.client as mqtt
import serial
import time
import json
RELAY1_LAMP_PIN = 30
RELAY2_FAN_PIN = 31
LED_PIN = 32
DETECTED = 3
DHT11_PIN = 7
LDR_PIN = 54


accesstoken= 'RASPBERRYPI'
password = '' #not used
broker='demo.thingsboard.io'
# topic="v1/devices/me/telemetry"

ser = serial.Serial(
    port='/dev/ttyACM0',    # Ganti sama serial port nya (misal COM1 atau /dev/usbTTY0)
    baudrate=115200,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)

def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)

print("creating new instance")
client = mqtt.Client("P1")
client.on_message=on_message
print("connecting to broker")

client.username_pw_set(accesstoken)
client.connect(broker,1883)
client.loop_start()

buffer = ser.read(ser.in_waiting)

while True:
    try:
        str = ser.read_until().decode().strip()
        # print(str)
        jobj = json.loads(str)
        # print(jobj)
        if(jobj["gpio"] == RELAY1_LAMP_PIN):
            data = {"lampu" : jobj["value"]}
            print(data)
            client.publish("RPI/arduinomega/telemetry", json.dumps(data))
        if(jobj["gpio"] == RELAY2_FAN_PIN):
            data = {"kipas" : jobj["value"]}
            print(data)
            client.publish("RPI/arduinomega/telemetry", json.dumps(data))
        if(jobj["gpio"] == LED_PIN):
            data = {"led" : jobj["value"]}
            print(data)
            client.publish(topic, json.dumps(data))
        if(jobj["gpio"] == DETECTED):
            data = {"chair" : jobj["value"]}
            print(data)
            client.publish("RPI/arduinomega/telemetry", json.dumps(data))
        if(jobj["gpio"] == DHT11_PIN):
            data = {"DHT11" : jobj["value"]}
            print(data)
            client.publish(topic, json.dumps(data))
        if(jobj["gpio"] == LDR_PIN):
            data = {"ldr" : jobj["value"]}
            print(data)
            client.publish("RPI/arduinomega/telemetry", json.dumps(data))
        
    except KeyboardInterrupt:
        break
    except:
        pass