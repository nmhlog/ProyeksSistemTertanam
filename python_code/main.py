import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO
import serial
import time
import json
import logging
from tb_device_mqtt import TBDeviceMqttClient
GPIO.setmode(GPIO.BOARD)
# logging.basicConfig(level=logging.INFO,
#                     format='%(asctime)s - %(levelname)s - %(module)s - %(lineno)d - %(message)s',
#                     datefmt='%Y-%m-%d %H:%M:%S')
# log = logging.getLogger(__name__)

LED_PIN = 32
US_WORKDESK = 3
DHT11_PIN = 7
LDR_PIN = 54
LAMP_PIN_RASPBERRY = 11
FAN_PIN_RASPBERRY = 12
Automatic=False
PersonDetected= False
light_state=False
fan_state=False
GPIO.setup(LAMP_PIN_RASPBERRY, GPIO.OUT)
GPIO.setup(FAN_PIN_RASPBERRY, GPIO.OUT)
accesstoken= ''
password = '' #not used
broker='demo.thingsboard.io'
topic="v1/devices/me/telemetry"
attributes="v1/devices/me/attributes"

ser = serial.Serial(
    port='/dev/ttyACM0',    # Ganti sama serial port nya (misal COM1 atau /dev/usbTTY0)
    baudrate=115200,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)

buffer = ser.read(ser.in_waiting)
def main():
    def setLamp(state,automatic):
        global PersonDetected
        if(automatic==True):
            result= not state or not PersonDetected
            GPIO.output(LAMP_PIN_RASPBERRY,result)
            print(f"fungsi jalankan lampu nilai:{result}")
        else :
            GPIO.output(LAMP_PIN_RASPBERRY,state)
            print(f"fungsi jalankan lampu nilai: {state}")
    def setFan(state,automatic):
        global PersonDetected
        if(automatic==True):
            result= not state or not PersonDetected
            GPIO.output(FAN_PIN_RASPBERRY,result)
            print(f"fungsi jalankan Kipas nilai  : {result}")
        else :
            GPIO.output(FAN_PIN_RASPBERRY,state)
            print(f"fungsi jalankan Kipas nilai  : {state}")
    
 
    def on_connect(client, userdata, rc, *extra_params):
        print('Connected with result code ',end='')
        print(rc)
        client.subscribe('v1/devices/me/rpc/request/+')

    
    def on_message(client, userdata, msg):
        global light_state
        global fan_state
        global Automatic
        global PersonDetected
        print('Topic: '+ msg.topic + '\nMessage: ',end='' )
        print(msg.payload)
        # Decode JSON request
        data = json.loads(msg.payload)
        if data['method'] == 'setValueAutomatic':
            Automatic = data['params']
        elif data['method'] == 'getValueAutomatic':
            client.publish(msg.topic.replace('request', 'response'), Automatic, 1)
        elif data['method'] == 'setValueLampWorkDesk':
            light_state = data['params']
            print(f"keluaran dari fungsi thingsboard,lampu {light_state}")
            setLamp(light_state,PersonDetected)
        elif data['method'] == 'getValueLampWorkDesk':
            client.publish(msg.topic.replace('request', 'response'), light_state, 1)
        elif data['method'] == 'setValueFan':
            fan_state = data['params']
            print(f"keluaran dari fungsi thingsboard,kipas {fan_state}")
            setFan(fan_state,PersonDetected)
        elif data['method'] == 'getValueFan':
            client.publish(msg.topic.replace('request', 'response'), fan_state, 1)
        elif data['method'] == "setPersonState":
            PersonDetected = data['params']
            if(Automatic):
                setLamp(light_state,Automatic)
                setFan(fan_state,Automatic)
        
            
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.username_pw_set(accesstoken)

        # Connect to ThingsBoard using default MQTT port and 60 seconds keepalive interval
    client.connect(broker, 1883, 60)
    
    
    while True:
        client.loop()
        try:
            str = ser.read_until().decode().strip()
            jobj = json.loads(str)
            if(jobj["gpio"] == US_WORKDESK):
                if jobj["value"] == 1:
                    data = {"PersonDetected" : True}
                else : data = {"PersonDetected" : False}
                print(data)
                client.publish(topic, json.dumps(data))
            if(jobj["gpio"] == DHT11_PIN):
                data_temp = {"temperature" : jobj["value"]["temperature"]}
                data_hum = {"humidity": jobj["value"]["humidity"]}
                print(data_temp)
                print(data_hum)
                
                client.publish(topic,json.dumps(data_temp))
                client.publish(topic,json.dumps(data_hum))
            
        except KeyboardInterrupt:
            break
        except:
            pass
    
if __name__ == '__main__':  
    main()
    
