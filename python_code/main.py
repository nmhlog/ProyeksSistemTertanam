import paho.mqtt.client as mqtt
import serial
import time
import json

ser = serial.Serial(
    port='COM3',    # Ganti sama serial port nya (misal COM1 atau /dev/usbTTY0)
    baudrate=9600,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)
buffer = ser.read(ser.in_waiting)

while True:
    try:
        str = ser.read_until().decode().strip()
        # print(str)
        jobj = json.loads(str)
        print(jobj)
        # if(jobj["gpio"] == 54):
        #     data = {"potentiometer" : jobj["value"]}
        #     client.publish("v1/devices/me/telemetry", json.dumps(data))
    except KeyboardInterrupt:
        break
    except:
        pass