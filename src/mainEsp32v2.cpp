#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


char* SSID = "#"; 
const char* WIFI_PASSWORD = "#";
const char* THINGSBOARD = "demo.thingsboard.io";
const char* CLIENT_ID = "ESP32";
const char* USER_NAME = "#";
const char* PASS_MQTT = "#";
const char* topic_pub="v1/devices/me/telemetry";
// const char* PERSON_DETECTED = "door/person detected";
// const char* DOOR_SENSOR_TOPIC = "door/door state";
// const char* LAMP_RELAY_TOPIC = "door/lamp state";
// const char* CLIENT_ID = "door_v1"; // MQTT client ID
// ThingsBoard tb(espClient);
WiFiClient espClient;
PubSubClient client(THINGSBOARD,1883,espClient);
const int TRIG_PIN = 2;
const int ECHO_PIN = 15;
const int RELAY_LAMP_PIN =0;
const int LDR_PIN = A0; //ternyata pin 34
long lastMsg = 0;
char msg[50];
int value = 0;
boolean automatic= false;
boolean lamp_state = false;
boolean door_state =false;
boolean buffer_lamp = false;
int timer =20*60;
int param = 80;
int ultrasound_range ;
bool subscribed = false;



bool setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
 }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

void connect_to_MQTT(bool wifi = true){
  setup_wifi();
  if (client.connect(CLIENT_ID, USER_NAME, PASS_MQTT)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}

void reconnect() {
  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publish_bool(String type_string,int val){
   StaticJsonDocument<256> JSONbuffer;
   char buffer[256];
   JSONbuffer[type_string] = val;
   size_t n = serializeJson(JSONbuffer, buffer);
  //  serializeJson(JSONbuffer, JSONmessageBuffer);
  if (client.publish(topic_pub, buffer,n)) {
      Serial.println(buffer);
    }
  // Again, client.publish will return a boolean value depending on whether it succeded or not.
  // If the message failed to send, we will try again, as the connection may have broken.
  else {
    Serial.println("Reconnecting to MQTT Broker and trying again");
    client.connect(CLIENT_ID, USER_NAME, PASS_MQTT);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    client.publish(topic_pub, buffer,n);
    Serial.println(buffer);
    }
  }

void publish_string(String type_string,String val){
   StaticJsonDocument<256> JSONbuffer;
   char buffer[256];
   JSONbuffer[type_string] = val;
   size_t n = serializeJson(JSONbuffer, buffer);
  if (client.publish(topic_pub, buffer,n)) {
      Serial.println( buffer);
    }
  else {
    Serial.println("Reconnecting to MQTT Broker and trying again");
    client.connect(CLIENT_ID, USER_NAME, PASS_MQTT);
    delay(10); 
    client.publish(topic_pub, buffer  ,n);
    Serial.println(buffer);
    }
  }


int sensor_ultrasound_HCSR04(int trigpin=TRIG_PIN, int echopin=ECHO_PIN, bool read_data =true)
{
/* 
fungsi untuk membaca HC-SR04 sensor 
  param trigpin : Pin trigger
  param echopin : Pin echo
  return integer dari jarak pengukuran
 */
  digitalWrite(trigpin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin, LOW);
  int range = pulseIn(echopin, HIGH)/ 29 / 2;  // Membaca jarak
  if(read_data)
  {  
    Serial.print("Distance from ultraound HCRS04 : ");
    Serial.print(range);
    Serial.println(" cm");
  }
  
  return range;
  }

int read_ultrasound_sensor(bool print =true){
    return sensor_ultrasound_HCSR04(TRIG_PIN,ECHO_PIN,print);
  }
  
bool read_ldr_sensor(bool print =false){
  int sensorValue = analogRead(A6); 
  if (print) Serial.println("LDR Sensor :" +String(sensorValue));
  return (sensorValue<500)? true:false;
  }
bool get_lamp_publish(bool lamp){
        
        int buffer_lamp=read_ldr_sensor();
        if (lamp!=buffer_lamp) {
        buffer_lamp =lamp;
        delay(675);
        Serial.println("Lamp state "+String(lamp));
        }
        return lamp

}
  
void lamp_on(boolean print, bool auto_val ){
  if(!auto_val) digitalWrite(RELAY_LAMP_PIN, LOW);
  delay(600);
  if (print) Serial.println("lampu Hidup");
  }

void lamp_off(boolean print ,bool auto_val ){
  if(!auto_val) digitalWrite(RELAY_LAMP_PIN, HIGH);
  if (print) Serial.println("lampu mati");
  }

bool timer_door(int timer, int range){
      int start = millis();
      int lamp ;
      lamp_on(true,automatic);
      delay(2000);
      Serial.println("Start timer");
      while((start-millis() )>timer){
        lamp_state = get_lamp_publish(lamp_state); 
        int buffer = read_ultrasound_sensor(true);
        delay(675);
        if (buffer>param) 
        {
         publish_bool("door",true);
         return true;
        }
      } 
      lamp_off(true,automatic);
      return false;
}

void timer_lamp_on(int timer){
  int start_timer = millis();
  lamp_on(true,automatic);
  bool lamp =true;
  while((start-millis() )>timer*60000){
    bool buffer_lamp = read_ldr_sensor(false);
    if (buffer_lamp!=lamp)
    {
      lamp=buffer_lamp;
      publish_bool("lamp",buffer_lamp);
    }
    
    // if (person_on_chair)
    // {
    //   return;
    // }

  }lamp_off(false,automatic);
  

}

void setup() {
  
  // put your setup code here, to run once:
  // client.setServer(MQTT_SERVER, 1883);
  //   client.setCallback(callback);
  // pinMode(LDR_PIN,INPUT);
  pinMode(RELAY_LAMP_PIN,OUTPUT);
  pinMode(ECHO_PIN,INPUT);
  pinMode(TRIG_PIN,OUTPUT);
  Serial.begin(115200);
  ultrasound_range = sensor_ultrasound_HCSR04();
  connect_to_MQTT();
}

void loop() {
lamp_state = get_lamp_publish(lamp_state);
delay(500);
int buffer = read_ultrasound_sensor(false);

if (buffer<=10&&buffer>2){
  lamp_on(true,automatic);
  publish_bool("Relay lamp",true);
  Serial.println("ultrasound range : "+String(buffer));
  delay(10000);
  
  
}

if (buffer>20 && buffer<param)
{
  door_state = true;
  Serial.println("ultrasound range : "+String(buffer));
  publish_bool("door",door_state);
  if(timer_door(timer,param)) {
    timer_lamp_on(1);
  }
}

if (!person_on_chair)
{
  timer_lamp_on();
}

if ((buffer-ultrasound_range)>=2)
{ 
  ultrasound_range=buffer;
  Serial.println("ultrasound range : "+String(buffer));
}
if(buffer==0) Serial.println("Ultrasound doesn't work");

delay(500);

}
