import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO
import serial
import time
import json
from tb_device_mqtt import TBDeviceMqttClient
GPIO.setmode(GPIO.BOARD)

# initsialisasi pin arduino mega
LED_PIN = 32
US_WORKDESK = 3
DHT11_PIN = 7
# Inisialiasai pin raspberry pi
LAMP_PIN_RASPBERRY = 11
FAN_PIN_RASPBERRY = 12
# Inisialisasi flag yang digunakan pada rpc
Automatic=False
PersonDetected= False
light_state=False
fan_state=False
#  Inisialisasi Setting GPIO sebagai output
GPIO.setup(LAMP_PIN_RASPBERRY, GPIO.OUT)
GPIO.setup(FAN_PIN_RASPBERRY, GPIO.OUT)
<<<<<<< HEAD

# Inisialiasasi setup untuk mqtt handler
=======
>>>>>>> 1ec80d37a5a413e47ac18d8228f32772dfe1192a
accesstoken= ''
password = '' #not used
broker='demo.thingsboard.io'
topic="v1/devices/me/telemetry"
# inisitalisasi serial sistem
ser = serial.Serial(
    port='/dev/ttyACM0',    
    baudrate=115200,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)
buffer = ser.read(ser.in_waiting)

# Fungsi main yang berjalan
def main():
    
    def setLamp(state,automatic):
        """ 
        Fungsi yang digunakan untuk mengendalikan lampu
        param state     : Merupakan masukan untuk pengontrol dari rpc thingsboard
        param automatic : Merupakan masukan untuk automatic  dari rpc thingsboard
        """
        global PersonDetected
        if(automatic==True):
            result= not state or not PersonDetected
            GPIO.output(LAMP_PIN_RASPBERRY,result)
            print(f"fungsi jalankan lampu nilai:{result}")
        else :
            GPIO.output(LAMP_PIN_RASPBERRY,not state)
            print(f"fungsi jalankan lampu nilai: {state}")
    def setFan(state,automatic):
        """ 
        Fungsi yang digunakan untuk mengendalikan kipas
        param state     : Merupakan masukan untuk pengontrol dari rpc thingsboard
        param automatic : Merupakan masukan untuk automatic  dari rpc thingsboard
        """
        global PersonDetected
        if(automatic==True):
            result= not state or not PersonDetected
            GPIO.output(FAN_PIN_RASPBERRY,result)
            print(f"fungsi jalankan Kipas nilai  : {result}")
        else :
            GPIO.output(FAN_PIN_RASPBERRY,not state)
            print(f"fungsi jalankan Kipas nilai  : {state}")
    

    def on_connect(client, userdata, rc, *extra_params):
        """ 
        Fungsi ynag digunakan saat client inisialisasi mqtt berjalan
        """
         print('Connected with result code ',end='')
        print(rc)
        # Client meminta alamat dari subcribe.
        client.subscribe('v1/devices/me/rpc/request/+')
        
    
    def on_message(client, userdata, msg):
        """ 
        Fungsi yang digunakan untuk menerima pesan subcribe dari thingsboard.
        """
        # Inisialiasasi fungsi agar dapat menerima parameter yang telah di inisialisasi secara global
        global light_state
        global fan_state
        global Automatic
        global PersonDetected
        # Menerima topic yang alamat dari thingsboard
        print('Topic: '+ msg.topic + '\nMessage: ',end='' )
        print(msg.payload)
        # Decode JSON request
        data = json.loads(msg.payload)
        # Fungsi yang diterima dalam bentuk string url yang dan isinya adalah json
        # Seingga di filter sesuai method dengan nama fungsi yang didapati
        # Nama fungsi disetting pada plugin rpc
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
        
    # Inisialisasi client pada mqtt    
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    # Autentikasi Thingsboard
    client.username_pw_set(accesstoken)

    # Connect o ThingsBoard using default MQTT port and 60 seconds keepalive intervalt
    client.connect(broker, 1883, 60)
    
    
    while True:
        # melooping sistem client mqtt agar selalu hidup karena ada limitasi waktu pada inisialisasi client
        client.loop()
        try:
        # Memulai serial usb
            str = ser.read_until().decode().strip()
        # Membaca sistem yang berjalan di diubah menjadi JSON
            jobj = json.loads(str)
            
        # Memfilter sistem berdasarkan pin GPIO
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
    
