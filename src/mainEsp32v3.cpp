#include <Arduino.h>
#include <WiFi.h>
#include <ThingsBoard.h>   

#define MENIT 60000;
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
char* SSID = "naufalroom"; 
const char* WIFI_PASSWORD = "m0dal_d0ng";
const char* THINGSBOARD = "demo.thingsboard.io";
const char * TOKEN = "ESP32ku";
const char* topic_pub="v1/devices/me/telemetry";

WiFiClient espClient;
ThingsBoard tb(espClient);
PubSubClient client(THINGSBOARD,1883,espClient);
const int TRIG_PIN = 2;
const int ECHO_PIN = 15;
const int RELAY_LAMP_PIN =0;
const int LDR_PIN = A6; //ternyata pin 34
long lastMsg = 0;
char msg[50];
int value = 0;
boolean automatic= true;
boolean lamp_state = false;
boolean door_state =false;
int timer = 10;
int param = 63;
int ultrasound_range ;
bool subscribed = false;
bool buffer_door_state = false;
int status = WL_IDLE_STATUS;
bool buffer_open =false;

void reconnect() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}

void setup_wifi_mqtt() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    reconnect();
    Serial.print(".");
 }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  if (tb.connect(THINGSBOARD, TOKEN)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }

}

void publish_bool(const char* type_string, bool val){

  
  if (tb.sendTelemetryBool(type_string,val)) {
      Serial.println("Publish Berhasil........");
    }
    else {
    Serial.println("Reconnecting to MQTT Broker and trying again");
    tb.connect(THINGSBOARD, TOKEN);
    delay(10); 
    tb.sendTelemetryBool(type_string,val);
    Serial.println("Publish Berhasil........");
    }
  }

int sensor_ultrasound_HCSR04(int trigpin=TRIG_PIN, int echopin=ECHO_PIN, bool read_data=false )
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
        
        bool buffer_lamp=read_ldr_sensor();
        if (lamp!=buffer_lamp) {
        lamp =buffer_lamp;
        delay(675);
        Serial.println("Lamp state "+String(lamp));
        publish_bool("LDR",lamp);
        }
        return lamp;

}

void  get_door_publish(bool door = door_state){
  publish_bool("door",door) ;
  Serial.println("door state " +String(door));

}
void lamp_on(boolean print=true, bool auto_val=automatic ){
  if(auto_val==false) digitalWrite(RELAY_LAMP_PIN, LOW);
  delay(600);
  if (print) Serial.println("lampu Hidup");
  }

void lamp_off(boolean print=true, bool auto_val=automatic ){
  if(auto_val==false) digitalWrite(RELAY_LAMP_PIN, HIGH);
  if (print) Serial.println("lampu mati");
  }


RPC_Response psetAutomatic(const RPC_Data &data){
    Serial.println("Received relay lamp");
    automatic = data;
    if(automatic==true) digitalWrite(RELAY_LAMP_PIN,LOW);
    else digitalWrite(RELAY_LAMP_PIN,HIGH);
    Serial.println("Automatic set to "+String(automatic));

    return RPC_Response(NULL,automatic);
}

RPC_Response pgetAutomatic(const RPC_Data &data)
{
  Serial.println("Received the get value method");

  return RPC_Response(NULL, automatic);
}

RPC_Response psetValueTimer(const RPC_Data &data){
    Serial.println("Received relay lamp");
    timer = data;
    Serial.println("Automatic set to "+String(timer));

    return RPC_Response(NULL,timer);
}

RPC_Response pgetValueTimer(const RPC_Data &data)
{
  Serial.println("Received the get value method");

  return RPC_Response(NULL, timer);
}

RPC_Callback callbacks[] = {
  { "setRelayLamp",     psetAutomatic },
  { "getRelayLamp",     pgetAutomatic },
  { "setValueTimer",    psetValueTimer },
  { "getValueTimer",    pgetValueTimer },
};

bool timer_door(int timer_m=timer, int range=param){

      buffer_open=true;
      if(automatic==true)
        {
        Serial.println("Automatic is turn off");
        return false;
        }
      else
      {
      Serial.println("Timer door ON");
      unsigned long start = millis();
      lamp_on();
      delay(10000);
      while((unsigned long)(millis())<=start+timer_m*5000){
        lamp_state = get_lamp_publish(lamp_state); 
        int buffer = read_ultrasound_sensor(true);
        delay(675);

         if (buffer>param) 
        {
        door_state=false;
         lamp_off();
         return false;
        }
      } 
      Serial.println(millis()-start);
      return true;
      }
      
      
}

void timer_lamp_on(int timer_m=timer){
   if(automatic==true)
    {
      Serial.println("Automatic is turn off");
      return;
    } else{
    buffer_open=false;
    Serial.println("Timer Lamp ON");
    unsigned long start_timer = millis();
    lamp_on();
    while((unsigned long)(millis()-start_timer )<=timer_m*5000){
      lamp_state = get_lamp_publish(lamp_state);
      int buffer = read_ultrasound_sensor();
      Serial.println("Hello timer");
      delay(1000);
      if (buffer>param)
        {
        lamp_off();
        return;
       }
    }
    Serial.println(millis()-start_timer);
    lamp_off();
}
  
}

void setup() {
  pinMode(LDR_PIN,INPUT);
  pinMode(RELAY_LAMP_PIN,OUTPUT);
  pinMode(ECHO_PIN,INPUT);
  pinMode(TRIG_PIN,OUTPUT);

  Serial.begin(115200);
  ultrasound_range = sensor_ultrasound_HCSR04();
 
  setup_wifi_mqtt();
}

void loop() {
delay(500);
Serial.println("Door State is "+String(door_state));
if (WiFi.status() != WL_CONNECTED) {
    reconnect();
    return;
  }
if (tb.connected()) {
    tb.RPC_Subscribe(callbacks, COUNT_OF(callbacks));
    }
   else
   {
    tb.connect(THINGSBOARD, TOKEN);
    tb.RPC_Subscribe(callbacks, COUNT_OF(callbacks));
    Serial.println("Failed to connect");
  }

lamp_state = get_lamp_publish(lamp_state);
delay(500);
int buffer = read_ultrasound_sensor(true);
bool buffer_door ;
  if (buffer<param)
  {
    buffer_door=true;
    
  }else
  {
    buffer_door=false;
   }
if (buffer_door!=door_state)
{
  door_state =buffer_door;
  get_door_publish(door_state);
}


if (buffer>0&&buffer>param && door_state==false&& buffer_open==false)
{ 
  Serial.println("Door Open state");
  delay(600);
  Serial.println("ultrasound range : "+String(buffer));
  buffer_door_state = true;
  if(timer_door(timer,param)==true) {
    timer_lamp_on();
  }

}

if (buffer>0&&buffer<=param&& door_state==true&&buffer_door_state==true){

  Serial.println("ultrasound range : "+String(buffer));
  buffer_open=false;
  buffer_door_state = false;
  timer_lamp_on();
  delay(600);
}

if(buffer==0) Serial.println("Ultrasound doesn't work");
delay(500);
 // Process messages
  tb.loop();
  ultrasound_range=buffer;
}
