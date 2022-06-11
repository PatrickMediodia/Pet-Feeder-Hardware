//Libraries used
#include <Arduino_FreeRTOS.h>
#include <Servo.h>
#include <DHTesp.h>
#include <HX711.h>

//Pin configuration
#define DHT11_SENSOR 2
#define SERVO_MOTOR 3
#define LOAD_CELL_DT 4
#define LOAD_CELL_SCK 5
#define WATER_SENSOR A5

//Library Objects
DHTesp dht;
Servo servo;
HX711 scale;

TaskHandle_t handler_main_thread,
  handler_dispense, 
  handler_temp_hum_status,
  handler_food_water_level;

//Load Cell Variables
float calibration_factor = 2300;
float zero_factor = 304509;

float temperature, humidity, water_reading, food_reading;
String foodLevel, waterLevel, command;
int serving;

void setup() {
  Serial.begin(9600);

  dht.setup(DHT11_SENSOR, DHTesp::DHT11);
  
  scale.begin(LOAD_CELL_DT, LOAD_CELL_SCK);
  scale.set_scale(calibration_factor);
  scale.set_offset(zero_factor);
  
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
      //Dispense,5
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
    food_reading = scale.get_units(), 10;
    
    if (food_reading <= 80) {
      foodLevel = "Empty";
    }
    else if (food_reading > 80 && food_reading <= 150) {
      foodLevel = "Low";
    }
    else if (food_reading > 150 && food_reading <= 220) {
      foodLevel = "Medium";
    }
    else if (food_reading > 220) {
      foodLevel = "High";
    }

    //water level implementation
    water_reading = analogRead(WATER_SENSOR);
    
    if (water_reading <= 300) {
      waterLevel = "Empty";
    }
    else if (water_reading > 300 && water_reading <= 440) {
      waterLevel = "Low";
    }
    else if (water_reading > 440 && water_reading <= 490) {
      waterLevel = "Medium";
    }
    else if (water_reading > 490) {
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
