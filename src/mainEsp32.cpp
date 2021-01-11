#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
const int TRIG_PIN = 2;
const int ECHO_PIN = 15;
const int RELAY_PIN = 12;
const int DOOR_SENSOR = 0;
bool door_state =1;
bool lamp_state =0;
int ultrasound_state = 0;
long detik =100000;//500ms

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
  
    door_state = digitalRead(DOOR_SENSOR);
    delay(1000);
    Serial.println(door_state==1); 
    if (door_state==1){
      int start = millis();
    
      while((millis()- start )<detik){
         ultrasound_state = sensor_ultrasound_HCSR04();
        delay(500);
        digitalWrite(RELAY_PIN, !lamp_state);
     if(ultrasound_state<5) //lock lamp
     {
       digitalWrite(RELAY_PIN, LOW);
       delay(100);
       return;
       }else {
         digitalWrite(RELAY_PIN, lamp_state);
          }
        
        }
    }
}