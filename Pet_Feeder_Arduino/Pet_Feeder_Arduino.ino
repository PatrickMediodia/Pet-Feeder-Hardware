//Libraries used
#include <Arduino_FreeRTOS.h>
#include <Servo.h>
#include <DHTesp.h>

//Pin configuration
#define DHT11_SENSOR 2
#define SERVO_MOTOR 3
#define LOAD_CELL_DT 4
#define LOAD_CELL_SCK 5
#define WATER_SENSOR A5

//Library Objects
DHTesp dht;
Servo servo;

TaskHandle_t handler_main_thread, 
  handler_dispense, 
  handler_temp_hum_status,
  handler_food_water_level;

float temperature, humidity, water_level;
String foodLevel, waterLevel, command;
int serving;

void setup() {
  Serial.begin(9600);

  dht.setup(DHT11_SENSOR, DHTesp::DHT11);
  
  servo.attach(SERVO_MOTOR);
  servo.write(180);
  
  xTaskCreate(MainThread, "MainThread", 100, NULL, 1, &handler_main_thread);
  
  xTaskCreate(TemperatureHumidityStatus, "TemperatureHumidityStatus", 100, NULL, 2, &handler_temp_hum_status);
  vTaskSuspend(handler_temp_hum_status);
  
  xTaskCreate(FoodWaterLevelStatus, "FoodWaterLevelStatus", 100, NULL, 2, &handler_food_water_level);
  vTaskSuspend(handler_food_water_level);
  
  xTaskCreate(DispenseFood, "DispenseFood", 100, NULL, 2, &handler_dispense);
  vTaskSuspend(handler_dispense);
}

void loop() {}

static void MainThread(void* parameters) {
  while (1) {
    if (Serial.available()) {
      command = Serial.readStringUntil('\n');
      command.trim();
      
      if (command == "TempHum") {
        vTaskResume(handler_temp_hum_status);
      }
      
      else if (command == "FoodWater") {
        vTaskResume(handler_food_water_level);
      }
      
      else if (command.indexOf("Dispense") >= 0) {      
        int index = command.indexOf(',');
        serving = command.substring(index + 1, command.length()).toInt();
        
        vTaskResume(handler_dispense);
      }
      
    }
  }
}

static void TemperatureHumidityStatus(void* parameters) {
  while (1) {
    Serial.print("{\"temp\" : ");
    Serial.print(dht.getTemperature());
    Serial.print(", \"hum\" : ");
    Serial.print(dht.getHumidity());
    Serial.println("}");
    
    vTaskSuspend(handler_temp_hum_status);
  }
}

static void FoodWaterLevelStatus(void* parameters) {
  while (1) {
    //food level implementation
    foodLevel = "Low";

    //water level implementation
    water_level = analogRead(WATER_SENSOR);
    
    int lowerThreshold = 420;
    int upperThreshold = 520;

    if (water_level == 0) {
      waterLevel = "Empty";
    }
    else if (water_level > 0 && water_level <= lowerThreshold) {
      waterLevel = "Low";
    }
    else if (water_level > lowerThreshold && water_level <= upperThreshold) {
      waterLevel = "Medium";
    }
    else if (water_level > upperThreshold) {
      waterLevel = "High";
    }
        
    Serial.print("{\"food\" : \"");
    Serial.print(foodLevel);
    Serial.print("\" , \"water\" : \"");
    Serial.print(waterLevel);
    Serial.println("\"}");
    
    vTaskSuspend(handler_food_water_level);
  }
}

static void DispenseFood(void* parameters) {
  while (1) {
    for(int i = 0; i < serving; i++) {
      servo.write(90);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      servo.write(180);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    Serial.println("Done Dispense");
    
    vTaskSuspend(handler_dispense);
  }
}
