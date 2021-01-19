#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <dht.h>
#include <string.h>
#include <stdlib.h>

// Deklarasi Variable pin constant
const int LED_PIN = 32;
const int ECHO_PIN = 3;
const int TRIG_PIN = 2;
const int DHT11_PIN = 7;

//Deklarasi variable
bool buffer_state = true;
bool automatic = false;
dht DHT;

//Deklarasi Task
void Task_read_sensors( void *pvParameters ); // Task untuk membaca sensor DHT11,LDR
void Task_Ultrasoundworkdesk( void *pvParameters ); // Task untuk HCSR04, untuk mendeteksi keadaan sekitar saat terdeteksi manusia
void Task_Ultrasoundworkdesk_nopeople( void *pvParameters ); // Task untuk HC-SRO4 untuk mendektesi manusia saat tidak ada orang

void serial_print_person_detector(int pin, bool param ){
  /* 
  fungsi untuk membaca HC-SR04 sensor 
  param pin     : Pin dari pin echo ultrasound
  param  param  : Merupakan nilai boolean hasil pendeteksi manusia
  return integer dari jarak pengukuran
 */

        Serial.print("{\"gpio\":");
        Serial.print(pin);
        Serial.print(",\"value\":");
        Serial.print(param);
        Serial.println("}");
}

void serial_print_dht11(int pin, int param1,int param2){
   /* 
  fungsi untuk membaca HC-SR04 sensor 
  param   pin     : Pin dari pin dht11 
  param  param1   : Merupakan nilai hasil pengukuran temperature
  param  param2   : Merupakan nilai hasil pengukuran kelembaban
  return integer dari jarak pengukuran
 */
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

bool get_person_state(int param){
/* 
Fungsi untuk mendapatkan keadaan manusi sekarang 
param param : set jarak yang ditentukan untuk mengindikasikan seseorang sedang berkerja
return : boolean keadaan manusia 1 indikasi ada manusia dan 0 indikasi tidak ada manusia
 */
  bool flag =0;
  int buffer_range = sensor_ultrasound_HCSR04(TRIG_PIN,ECHO_PIN);
  if (buffer_range<param){
    flag=flag||1;
    }
    else flag=flag&&1;
    delay(10);
  return flag;
  }

void set_control(bool led_state ){
/* 
  fungsi untuk mengerakkan aktuator.
  param  led_state  : keadaan kipas 1 hidup || 0 mati

 */
  digitalWrite(LED_PIN, led_state);
  delay(500);

  }

void setup()
{   
    //  Inisialisasi Aktuator Pin
      pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an OUTPUT
      pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an INPUT
      pinMode(LED_PIN, OUTPUT); // Sets the LED_PIN as an OUPUT

      xTaskCreate(
        Task_read_sensors
        , "Blink" 
        , 128      
        , NULL
        , 1        
        , NULL ); // Fungsi untuk membaca task
 
    xTaskCreate(
        Task_Ultrasoundworkdesk
        , "Detecting People"
        , 128    
        , NULL
        , 2       
        , NULL ); // Fungsi untuk membaca task

}
 
void loop()
{

}
 
void Task_read_sensors(void *pvParameters) // Task untuk membaca parameter dari sensor
{   
    (void) pvParameters;
    Serial.begin(115200);
    for (;;) 
    {
    Serial.println("Task_read_sensors work");    
    // Fungsi untuk membaca sensor DHT11
    int chk = DHT.read11(DHT11_PIN);
    // Mengirim 
    serial_print_dht11(DHT11_PIN,DHT.temperature,DHT.humidity);
    vTaskDelay( 20000 / portTICK_PERIOD_MS ); // delay task untuk 20000 = 20s
    } 
}
 
void Task_Ultrasoundworkdesk(void *pvParameters) // This is a task.
{
    (void) pvParameters;
    Serial.begin(115200);
    int param = 80;
    for (;;)
    { 
      Serial.println("Task_Ultrasoundworkdesk work");
      bool read_val = get_person_state(param);
      if (!read_val){
        for (int i= 0;i<5;i++){
          read_val= read_val || get_person_state(param);
          delay(10);
          if(read_val){break;} 
        }
      if(!read_val){
          xTaskCreate(Task_Ultrasoundworkdesk_nopeople, "Detecting no People", 128, NULL, 2, NULL );
          vTaskDelete(NULL);
        }

      }
      if (read_val){
          buffer_state = read_val;
          set_control(!read_val);
          }
      serial_print_person_detector(ECHO_PIN,read_val);
      vTaskDelay( 100000 / portTICK_PERIOD_MS ); 
    }
}

void Task_Ultrasoundworkdesk_nopeople( void *pvParameters ){
    (void) pvParameters;
    Serial.begin(115200);
    bool buffer = false;
    int param = 97;
    while(true){
      // Serial.println("Task_Ultrasoundworkdesk_nopeople work");
      for (int i= 0;i<5;i++)
        {
        buffer= buffer || get_person_state(param);
        delay(10);
        if(buffer){
        xTaskCreate(Task_Ultrasoundworkdesk, "Detecting People", 128, NULL, 2, NULL );
        // Serial.write()
        vTaskDelete(NULL);
          }
        }
        if (buffer == false){

         set_control(!buffer);
         
          }
        serial_print_person_detector(ECHO_PIN,buffer);
        // Serial.write()
        vTaskDelay( 20000 / portTICK_PERIOD_MS );
        
    }

}