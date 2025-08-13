#include <task.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>

Adafruit_ADS1115 ads;
TaskHandle_t currentTaskHandle = nullptr;
SemaphoreHandle_t i2c_mutex;

volatile float current_l;
volatile float current_r;

void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  Serial.print(bus);
}

void readADC()
{   
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(50)) == pdTRUE)
    {
        int16_t adc0 = ads.readADC_SingleEnded(0);
        int16_t adc1 = ads.readADC_SingleEnded(1);

        float volts0 = ads.computeVolts(adc0);
        float volts1 = ads.computeVolts(adc1);

        current_l = max(0.0,(volts0-1.5)*15);
        current_r = max(0.0,(volts1-1.5)*15);

        Serial.printf("<AIN0: %.2f>\n", current_l);
        Serial.printf("<AIN1: %.2f>\n",  current_r);

        xSemaphoreGive(i2c_mutex);
    }
    else
    {
        Serial.println("I2C bus busy, skipping this read.");
    }
}

void currentTask(void *parameter)
{
    // Make sure this is only called ONCE globally in setup()
    // Wire.begin(SDA, SCL); <- already done

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    i2c_mutex = xSemaphoreCreateMutex();
    
    byte error, address;
    int nDevices;
    Serial.println("Scanning...");
    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknow error at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
    {
        Serial.println("No I2C devices found\n");
    }
    else
    {
        Serial.println("done\n");
    }
    TCA9548A(1);
    ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
    if (!ads.begin(0x48, &Wire)) // 0x49 is decimal 73
    {
        Serial.println("Failed to initialize ADS.");
        vTaskDelete(nullptr);
    }

    for (;;)
    {
        readADC();
        vTaskDelay((1000 / CURRENT_TASK_FREQ) / portTICK_PERIOD_MS);
    }
}