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

RELAY1_LAMP_PIN = 30
RELAY2_FAN_PIN = 31
LED_PIN = 32
US_WORKDESK = 3
DHT11_PIN = 7
LDR_PIN = 54
LAMP_PIN_RASPBERRY = 11
FAN_PIN_RASPBERRY = 12
Auto_Fan = False
Auto_lamp = False
Automatic=False
PersonDetected= False
light_state=False
fan_state=False
GPIO.setup(LAMP_PIN_RASPBERRY, GPIO.OUT)
GPIO.setup(FAN_PIN_RASPBERRY, GPIO.OUT)
accesstoken= 'ARDUINOMEGA'
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
        global Auto_Fan
        if(automatic):
            result=automatic or state
            GPIO.output(LAMP_PIN_RASPBERRY,result)
            print(f"Lampu dalam keadaan :{result}")
        else :
            GPIO.output(LAMP_PIN_RASPBERRY,state)
            print(f"Lampu dalam keadaan : {state}")
    def setFan(state,automatic):
        global Auto_lamp
        if(automatic):
            result=automatic or state
            GPIO.output(FAN_PIN_RASPBERRY,result)
            print(f"Lampu dalam keadaan :"+ result)
        else :
            GPIO.output(FAN_PIN_RASPBERRY,state)
            print(f"Lampu dalam keadaan : {state}")
    
    # def on_server_side_rpc_request(request_id, request_body):
    #     log.info('received rpc: {}, {}'.format(request_id, request_body))
    #     if request_body['method'] == 'getValueAutomatic':
    #         Automatic= request_body['params']
    #     elif request_body['method'] == 'getValueAutomatic':
    #         client.send_rpc_reply(request_id, {"Automatic":Automatic})   
    #     elif request_body['method'] == 'setValueLampWorkDesk':
    #         light_state = request_body['params']
    #         setLamp(light_state,Auto_lamp)
    #     elif request_body['method'] == 'getValueLampWorkDesk':
    #         client.send_rpc_reply(request_id, {"light_state":light_state})
    #     elif request_body['method'] == 'setValueFan':
    #         fan_state = request_body['params']
    #         setFan(fan_state,Auto_Fan)
    #     elif request_body['method'] == 'getValueFan':
    #         client.send_rpc_reply(request_id, {"fan_state":fan_state})
         
            
    # client = TBDeviceMqttClient(broker, accesstoken)
    # client.set_server_side_rpc_request_handler(on_server_side_rpc_request)
    # client.connect()  
    def on_connect(client, userdata, rc, *extra_params):
        print('Connected with result code ',end='')
        print(rc)
        # Subscribing to receive RPC requests
        client.subscribe('v1/devices/me/rpc/request/+')
        # Sending current GPIO status
        # client.publish('v1/devices/me/attributes', get_gpio_status(), 1)
    
    def on_message(client, userdata, msg):
        global light_state
        global fan_state
        global Automatic
        global PersonDetected
        print('Topic: '+ msg.topic + '\nMessage: ',end='' )
        print(msg.payload)
        # Decode JSON request
        data = json.loads(msg.payload)
        if data['method'] == 'getValueAutomatic':
            # print(data)
            client.publish(msg.topic.replace('request', 'response'), Automatic, 1)
        elif data['method'] == 'setValueAutomatic':
            Automatic = data['params']
        elif data['method'] == 'setValueLampWorkDesk':
            light_state = data['params']
            setLamp(light_state,PersonDetected)
        elif data['method'] == 'getValueLampWorkDesk':
            client.publish(msg.topic.replace('request', 'response'), light_state, 1)
        elif data['method'] == "setPersonState":
            PersonDetected = data['params']
            setLamp(light_state,PersonDetected)
            setFan(fan_state,PersonDetected)
        elif data['method'] == 'setValueFan':
            setFan(fan_state,PersonDetected)
        elif data['method'] == 'getValueFan':
            client.publish(msg.topic.replace('request', 'response'), fan_state, 1)
            
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
                client.publish(topic, json.dumps(data))
            if(jobj["gpio"] == DHT11_PIN):
                data_temp = {"temperature" : jobj["value"]["temperature"]}
                data_hum = {"humidity": jobj["value"]["humidity"]}
                client.publish(topic,json.dumps(data_temp))
                client.publish(topic,json.dumps(data_hum))
            
        except KeyboardInterrupt:
            break
        except:
            pass
    
if __name__ == '__main__':  
    main()
    
