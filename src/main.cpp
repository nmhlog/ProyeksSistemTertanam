#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <dht.h>
#include <string.h>
#include <stdlib.h>

// Deklarasi Variable 
const int RELAY1_LAMP_PIN = 30;
const int RELAY2_FAN_PIN = 31;
const int LED_PIN = 32;
const int ECHO_PIN = 3;
const int TRIG_PIN = 2;
const int DHT11_PIN =7;

//Deklarasi variable
bool buffer_state = true;
dht DHT;
//Deklarasi Task
void Task_Read_SerialData( void *pvParameters );
void Task_Ultrasound( void *pvParameters );
void Task_Ultrasound_nopeople( void *pvParameters );

int sensor_ultrasound_HCSR04(int trigpin, int echopin)
{
  digitalWrite(trigpin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin, LOW);
  int range = pulseIn(echopin, HIGH)/ 29 / 2; 
  Serial.print("Distance from ultraound HCRS04 : ");
  Serial.print(range);
  Serial.println(" cm");
  return range;
  }

bool get_person_state(int param){
  bool flag =0;
  int buffer_range = sensor_ultrasound_HCSR04(TRIG_PIN,ECHO_PIN);
  if (buffer_range<param){
    flag=flag||1;
    }
    else flag=flag&&1;
    delay(10);
  return flag;
  }



void set_control(bool lamp_state,bool pin_state, bool led_state ){
  digitalWrite(RELAY1_LAMP_PIN, lamp_state);
  digitalWrite(RELAY2_FAN_PIN, pin_state);
  digitalWrite(LED_PIN, led_state);
  }

void setup()
{   
      pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an OUTPUT
      pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an INPUT
      pinMode(RELAY1_LAMP_PIN, OUTPUT); // Sets the RELAY1_LAMP as an INPUT
      pinMode(RELAY2_FAN_PIN, OUTPUT); // Sets the RELAY2_FAN as an INPUT
      pinMode(LED_PIN, OUTPUT); // Sets the echoPin as an INPUT
      xTaskCreate(
        Task_Read_SerialData
        , "Blink" 
        , 128      
        , NULL
        , 1        
        , NULL );
 
    xTaskCreate(
        Task_Ultrasound
        , "Detecting People"
        , 128    
        , NULL
        , 2       
        , NULL );

}
 
void loop()
{
// Empty. Things are done in Tasks.
}

 
void Task_Read_SerialData(void *pvParameters) // This is a task.
{   
    (void) pvParameters;
    int sensorValue ;
    Serial.begin(9600);
    for (;;) // A Task shall never return or exit.
    {
    int chk = DHT.read11(DHT11_PIN);
    sensorValue = analogRead(A0); 
    delay(60);
    Serial.println("Lampu = "+ String(sensorValue));
    Serial.print("Temperature = ");
    Serial.println(DHT.temperature);
    Serial.print("Humidity = ");
    Serial.println(DHT.humidity);   
    vTaskDelay( 20000 / portTICK_PERIOD_MS );
    } //20000 -> 20 SECOND
}
 
void Task_Ultrasound(void *pvParameters) // This is a task.
{
    (void) pvParameters;
    Serial.begin(9600);
    int param = 97;
    for (;;)
    { 
      bool start = get_person_state(param);
      if (!start || buffer_state!=start){
        for (int i= 0;i<5;i++)
        {
        start= start || get_person_state(param);
        delay(10);
        if(start){break;} 
        }
        
      if (start != buffer_state)
    {
    buffer_state = start;
    set_control(!start,!start,!start);
    }
        if(!start){
          xTaskCreate(Task_Ultrasound_nopeople, "Detecting no People", 128, NULL, 2, NULL );
          vTaskDelete(NULL);
        }
      }
      
      vTaskDelay( 100000 / portTICK_PERIOD_MS ); 
    }
}

void Task_Ultrasound_nopeople( void *pvParameters ){
    (void) pvParameters;
    Serial.begin(9600);
    bool start = false;
    int param = 97;
    while(true){
      for (int i= 0;i<5;i++)
        {
        start= start || get_person_state(param);
        delay(10);
        if(start){
        xTaskCreate(Task_Ultrasound, "Detecting People", 128, NULL, 2, NULL );
        // Serial.write()
        vTaskDelete(NULL);
          }
        }
        // Serial.write()
        vTaskDelay( 20000 / portTICK_PERIOD_MS );
        
    }

}