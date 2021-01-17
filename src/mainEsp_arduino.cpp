#include <Arduino.h>
#include <string.h>
#include <ArduinoJson.h>


#include "HardwareSerial.h"
#define RXD2 16
#define TXD2 17

HardwareSerial serial_from_arduino(1);
StaticJsonDocument<200> buffer_json;
char* temp; 
char* person_state;
char* hum;

void writeString(String stringData) { // Used to serially push out a String with Serial.write()

  for (int i = 0; i < stringData.length(); i++)
  {
    serial_from_arduino.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }

}

void setup()
{
  Serial.begin(115200); // starts the serial port at 9600

  serial_from_arduino.begin(115200, SERIAL_8N1, RXD2, TXD2);
}
  
void loop()
{   
        while(serial_from_arduino.available()){
            String  data = serial_from_arduino.readString();
            Serial.print(data);
            char* buffer = strcpy(buffer,data.c_str());
             if(strstr(buffer, "humidity")!=NULL){
            Serial.print("h");
            hum=buffer;
                } 
           if(strstr(buffer, "temperature")!=NULL) 
           {
            Serial.print("t");
            temp=buffer;
           }
           if(strstr(buffer, "person_onchair")!=NULL)
           {
            Serial.print("p");
                person_state=buffer;
           }
           
            }
}