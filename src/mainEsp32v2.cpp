#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>


char* SSID = "$"; 
const char* WIFI_PASSWORD = "$";
const char* MQTT_SERVER = "192.168.2.137";
char* topic_esp="RPI/esp32/status";
// const char* PERSON_DETECTED = "door/person detected";
// const char* DOOR_SENSOR_TOPIC = "door/door state";
// const char* LAMP_RELAY_TOPIC = "door/lamp state";
// const char* CLIENT_ID = "door_v1"; // MQTT client ID

WiFiClient espClient;
PubSubClient client(espClient);
const int TRIG_PIN = 2;
const int ECHO_PIN = 15;
const int RELAY_LAMP_PIN =0;
const int LDR_PIN = A0; //ternyata pin 34
long lastMsg = 0;
char msg[50];
int value = 0;
boolean automatic= false;
boolean lamp = false;
boolean buffer_lamp = false;
int timer =20*60;
int param = 80;
int ultrasound_range ;
// bool get_person_state(int param, bool print=false){
// /* 
// Fungsi untuk mendapatkan keadaan manusi sekarang 
// param param : set jarak yang ditentukan untuk mengindikasikan seseorang sedang berkerja
// return : boolean keadaan manusia 1 indikasi ada manusia dan 0 indikasi tidak ada manusia
//  */
//   bool flag =0;
//   int buffer_range = sensor_ultrasound_HCSR04(TRIG_PIN,ECHO_PIN,print);
//   if (buffer_range<param){
//     flag=flag||1;
//     }
//     else flag=flag&&1;
//     delay(10);
//   return flag;
//   }

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
  
void lamp_on(boolean print, bool auto_val ){
  if(!auto_val) digitalWrite(RELAY_LAMP_PIN, LOW);
  delay(600);
  if (print) Serial.println("lampu Hidup");
  }

void lamp_off(boolean print ,bool auto_val ){
  if(!auto_val) digitalWrite(RELAY_LAMP_PIN, HIGH);
  if (print) Serial.println("lampu mati");
  }

void timer_lamp_on(int timer, int range){
      int start = millis();
      int lamp ;
      lamp_on(true,automatic);
      int ultrasound= read_ultrasound_sensor(false);
      delay(2000);
      Serial.println("Start timer");
      while((start-millis() )>timer){
        lamp=read_ldr_sensor(false);
        if (lamp!=buffer_lamp) {
        buffer_lamp =lamp;
        delay(675);
        Serial.println("Lamp state "+String(lamp));
        }
        int buffer = read_ultrasound_sensor(true);
        delay(675);
        if (buffer>param) 
        {
          lamp_off(true,automatic);
          return;
        }
      } 
      lamp_off(true,automatic);
      return;
}

void setup_wifi() {
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
}
// void callback(char* topic, byte* message, unsigned int length) {
//   Serial.print("Message arrived on topic: ");
//   Serial.print(topic);
//   Serial.print(". Message: ");
//   String messageTemp;
  
//   for (int i = 0; i < length; i++) {
//     Serial.print((char)message[i]);
//     messageTemp += (char)message[i];
//   }
//   Serial.println();

//   // Feel free to add more if statements to control more GPIOs with MQTT

//   // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
//   // Changes the output state according to the message
//   if (String(topic) == "esp32/output") {
//     Serial.print("Changing output to ");
//     if(messageTemp == "on"){
//       Serial.println("on");
//       digitalWrite(ledPin, HIGH);
//     }
//     else if(messageTemp == "off"){
//       Serial.println("off");
//       digitalWrite(ledPin, LOW);
//     }
//   }
// }

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
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

void setup() {
  // put your setup code here, to run once:
  client.setServer(MQTT_SERVER, 1883);
//   client.setCallback(callback);
  // pinMode(LDR_PIN,INPUT);
  pinMode(RELAY_LAMP_PIN,OUTPUT);
  pinMode(ECHO_PIN,INPUT);
  pinMode(TRIG_PIN,OUTPUT);
  Serial.begin(115200);
  ultrasound_range = sensor_ultrasound_HCSR04();
}

void loop() {
lamp = read_ldr_sensor();
delay(500);
if (lamp!=buffer_lamp) {
  buffer_lamp =lamp;
  Serial.println("Lamp state "+String(lamp));
}
int buffer = read_ultrasound_sensor(false);

if (buffer<=10&&buffer>2){
  lamp_on(true,automatic);
  Serial.println("ultrasound range : "+String(buffer));
  delay(10000);
  
}

if (buffer>20 && buffer<param)
{
  Serial.println("ultrasound range : "+String(buffer));
  timer_lamp_on(timer,param);
}

if ((buffer-ultrasound_range)>=2)
{ 
  ultrasound_range=buffer;
  Serial.println("ultrasound range : "+String(buffer));
}
if(buffer==0) Serial.println("Ultrasound doesn't work");

delay(500);

}
