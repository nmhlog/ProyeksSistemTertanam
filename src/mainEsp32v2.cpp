#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <dht.h>
#include <string.h>
#include <stdlib.h>

// Deklarasi Variable pin constant
const int RELAY1_LAMP_PIN = 30;
const int RELAY2_FAN_PIN = 31;
const int LED_PIN = 32;
const int ECHO_PIN = 3;
const int TRIG_PIN = 2;
const int DHT11_PIN = 7;
// const int LDR_PIN = A0;

//Deklarasi variable
bool buffer_state = true;
dht DHT;
//Deklarasi Task
void Task_read_sensors( void *pvParameters ); // Task untuk membaca sensor DHT11,LDR
void Task_Ultrasound( void *pvParameters ); // Task untuk HCSR04, untuk mendeteksi keadaan sekitar saat terdeteksi manusia
void Task_Ultrasound_nopeople( void *pvParameters ); // Task untuk HC-SRO4 untuk mendektesi manusia saat tidak ada orang




void serial_print_bool(int pin, bool param ){
        Serial.print("{\"gpio\":");
        Serial.print(pin);
        Serial.print(",\"value\":");
        Serial.print(param);
        Serial.println("}");
}
void serial_print_dht11(int pin, int param1,int param2){
        Serial.print("{\"gpio\":");
        Serial.print(pin);
        Serial.print(",\"value\":");
        Serial.print("{\"temperature\":");
        Serial.print(param1);
        Serial.print(",\"humidity\":");
        Serial.print(param2);
        Serial.println("}}");
}

int sensor_ultrasound_HCSR04(int trigpin, int echopin, bool read_data =false)
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

bool get_person_state(int param, bool print=false){
/* 
Fungsi untuk mendapatkan keadaan manusi sekarang 
param param : set jarak yang ditentukan untuk mengindikasikan seseorang sedang berkerja
return : boolean keadaan manusia 1 indikasi ada manusia dan 0 indikasi tidak ada manusia
 */
  bool flag =0;
  int buffer_range = sensor_ultrasound_HCSR04(TRIG_PIN,ECHO_PIN,print);
  if (buffer_range<param){
    flag=flag||1;
    }
    else flag=flag&&1;
    delay(10);
  return flag;
  }



void set_control(bool lamp_state,bool fan_state, bool led_state ){
/* 
  fungsi untuk mengerakkan aktuator.
  param  lamp_state : keadaan lampu 0 hidup || 1 mati
  param  fan_state  : keadaan kipas 0 hidup || 1 mati
  param  led_state  : keadaan kipas 1 hidup || 0 mati

 */

  digitalWrite(RELAY1_LAMP_PIN, lamp_state);
  digitalWrite(RELAY2_FAN_PIN, fan_state);
  digitalWrite(LED_PIN, led_state);
  delay(500);
  serial_print_bool(RELAY1_LAMP_PIN,lamp_state);
  delay(500);
  serial_print_bool(RELAY2_FAN_PIN,fan_state);
  delay(5000);
  serial_print_bool(LED_PIN,led_state);
  }

void setup()
{   
    // Inisialisasi sensor pin
      pinMode(DHT11_PIN, INPUT); // Sets the LED_PIN as an OUPUT
      pinMode(DHT11_PIN, INPUT); // Sets the LED_PIN as an OUPUT
    //  Inisialisasi Aktuator Pin
      pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an OUTPUT
      pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an INPUT
      pinMode(RELAY1_LAMP_PIN, OUTPUT); // Sets the RELAY1_LAMP_PIN as an OUTPUT
      pinMode(LED_PIN, OUTPUT); // Sets the LED_PIN as an OUPUT
      pinMode(RELAY2_FAN_PIN, OUTPUT); // Sets the RELAY2_FAN_PIN as an OUTPUT
      xTaskCreate(
        Task_read_sensors
        , "Blink" 
        , 128      
        , NULL
        , 1        
        , NULL ); // Fungsi untuk membaca task
 
    xTaskCreate(
        Task_Ultrasound
        , "Detecting People"
        , 128    
        , NULL
        , 2       
        , NULL ); // Fungsi untuk membaca task

}
 
void loop()
{
// Empty. Things are done in Tasks.
}

 
void Task_read_sensors(void *pvParameters) // Task untuk membaca parameter dari sensor
{   
    (void) pvParameters;
    int sensorValue ;
    Serial.begin(115200);
    for (;;) 
    {
      // Start Reading sensor
    
    
    vTaskDelay( 675 / portTICK_PERIOD_MS );
    int chk = DHT.read11(DHT11_PIN);
    serial_print_dht11(DHT11_PIN,DHT.temperature,DHT.humidity);
    vTaskDelay( 20000 / portTICK_PERIOD_MS ); // delay task untuk 20000 = 20s
    } 
}
 
void Task_Ultrasound(void *pvParameters) // This is a task.
{
    (void) pvParameters;
    Serial.begin(115200);
    int param = 97;
    for (;;)
    { 
      bool read_val = get_person_state(param);
      if (!read_val){
        for (int i= 0;i<5;i++){
          read_val= read_val || get_person_state(param);
          delay(10);
          if(read_val){break;} 
        }
      if(!read_val){
          xTaskCreate(Task_Ultrasound_nopeople, "Detecting no People", 128, NULL, 2, NULL );
          vTaskDelete(NULL);
        }

      }
      if (read_val){
          buffer_state = read_val;
          set_control(!read_val,!read_val,!read_val);
          }
      serial_print_bool(ECHO_PIN,read_val);
      vTaskDelay( 100000 / portTICK_PERIOD_MS ); 
    }
}

void Task_Ultrasound_nopeople( void *pvParameters ){
    (void) pvParameters;
    Serial.begin(115200);
    bool buffer = false;
    int param = 97;
    while(true){
      for (int i= 0;i<5;i++)
        {
        buffer= buffer || get_person_state(param);
        delay(10);
        if(buffer){
        xTaskCreate(Task_Ultrasound, "Detecting People", 128, NULL, 2, NULL );
        // Serial.write()
        vTaskDelete(NULL);
          }
        }
        if (buffer == false){
         set_control(!buffer,!buffer,!buffer);
         
          }
        serial_print_bool(ECHO_PIN,buffer);
        // Serial.write()
        vTaskDelay( 20000 / portTICK_PERIOD_MS );
        
    }

}