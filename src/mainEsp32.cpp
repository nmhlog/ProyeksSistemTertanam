#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
const int TRIG_PIN = 2;
const int ECHO_PIN = 15;
const int RELAY_PIN = 12;
const int DOOR_SENSOR = 0;

bool door_state =1;
bool buffer_door_state = 1;
bool lamp_state =0;
int ultrasound_state = 0;
long detik =100000;//500ms

char* SSID = "MASUKAN WIFI"; 
const char* WIFI_PASSWORD = "MASUKAN PASSWORDS";
const char* MQTT_SERVER = "192.168.2.137";

const char* PERSON_DETECTED = "door/person detected";
const char* DOOR_SENSOR_TOPIC = "door/door state";
const char* LAMP_RELAY_TOPIC = "door/lamp state";
const char* MQTTT_USERNAME = "test"; // MQTT username
const char* MQTT_PASSWORD = "test"; // MQTT password
const char* CLIENT_ID = "door_v1"; // MQTT client ID

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
// 1884 is the listener port for the Broker
PubSubClient client(MQTT_SERVER, 1884, wifiClient); 

void connect_MQTT(){
  Serial.print("Connecting to ");
  Serial.println(SSID);

  // Connect to the WiFi
  WiFi.begin(SSID, WIFI_PASSWORD);
  Serial.println(WiFi.status());
  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
 // Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  // If the connection is failing, make sure you are using the correct MQTT Username and Password (Setup Earlier in the Instructable)
  if (client.connect(CLIENT_ID, MQTTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}
void send_topics_bool(const char* topic, bool state){
  const char* buffer_val = state ? "true" : "false";
  if (client.publish(topic,
                     buffer_val)) {
        Serial.println("Temperature sent!");
    }
    // Again, client.publish will return a boolean value depending on whether it succeded or not.
    // If the message failed to send, we will try again, as the connection may have broken.
    else {
      Serial.println("Temperature failed to send. Reconnecting to MQTT Broker and trying again");
      client.connect(CLIENT_ID, MQTTT_USERNAME, MQTT_PASSWORD);
      delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
      client.publish(topic, buffer_val);
  }
}
void send_topics_string(const char* topic, String val){
  const char* buffer_val = val.c_str();
  if (client.publish(topic, 
              buffer_val)) {
        Serial.println("Temperature sent!");
    }
    // Again, client.publish will return a boolean value depending on whether it succeded or not.
    // If the message failed to send, we will try again, as the connection may have broken.
    else {
      Serial.println("Temperature failed to send. Reconnecting to MQTT Broker and trying again");
      client.connect(CLIENT_ID, MQTTT_USERNAME, MQTT_PASSWORD);
      delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
      client.publish(topic, buffer_val);
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


void setup () {
Serial.begin(9600);

pinMode(DOOR_SENSOR,INPUT_PULLUP);
pinMode(RELAY_PIN,OUTPUT);
pinMode(TRIG_PIN,OUTPUT);
pinMode(ECHO_PIN,INPUT);
lamp_state=!door_state;
digitalWrite(RELAY_PIN, !lamp_state);

}

void loop() {
    connect_MQTT();
    Serial.setTimeout(2000);

    door_state = door_state|| digitalRead(DOOR_SENSOR); // 0 pintu tertutup ; 1 pintu terbuka
    delay(1000);
    if (door_state!=buffer_door_state){
      buffer_door_state = door_state;
      send_topics_bool(DOOR_SENSOR_TOPIC,door_state);
    }
    
    
    if (door_state==1){

      int start = millis();

      while((millis()- start )<detik){
        ultrasound_state = sensor_ultrasound_HCSR04();
        delay(500);
        digitalWrite(RELAY_PIN, !lamp_state);
     
     if(ultrasound_state<100 && ultrasound_state>10){
       send_topics_bool(PERSON_DETECTED,1);

     }   
     if(ultrasound_state<=10) //Mengunci lampu tambahkan state untuk membaca publish dari Raspberry pi
     {
       digitalWrite(RELAY_PIN, LOW);
       send_topics_string(LAMP_RELAY_TOPIC,"locked");
       delay(100);
       return;
       }
        
      }
         digitalWrite(RELAY_PIN, !lamp_state);
         send_topics_string(LAMP_RELAY_TOPIC,"automatic");
      
    }
}