#include <task.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>


Adafruit_ADS1115  ads;
TaskHandle_t      currentTaskHandle = nullptr;
SemaphoreHandle_t i2c_mutex;

volatile uint32_t current_limit = CURRENT_LIMIT_DEFAULT;
volatile bool overcurrent = false; /* This flag is set when either side experiences an overcurrent error */
volatile bool calibrating = false; /* This flag is set during current calibration to make sure the speed is 0 */


static void readADC(int16_t *callibration_array, int16_t *results) {
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {

        /* Read from the ADC */
        int16_t adc0 = ads.readADC_SingleEnded(0);
        int16_t adc1 = ads.readADC_SingleEnded(1);
        xSemaphoreGive(i2c_mutex);

        /* Compute the volts before the potential divider */
        float volts0 = (ads.computeVolts(adc0) * CURRENT_SENSOR_SUPPLY_VOLTAGE) / CURRENT_MAX_VOLTAGE;
        float volts1 = (ads.computeVolts(adc1) * CURRENT_SENSOR_SUPPLY_VOLTAGE) / CURRENT_MAX_VOLTAGE;

        /* Work out the current in mA from the offset and sensitivity */
        int16_t current_r = (uint16_t) constrain((volts0 - (CURRENT_SENSOR_SUPPLY_VOLTAGE / 2)) / CURRENT_SENSOR_SENSITIVITY, -20000, 20000); /* Current is between -20A and 20A */
        int16_t current_l = (uint16_t) constrain((volts1 - (CURRENT_SENSOR_SUPPLY_VOLTAGE / 2)) / CURRENT_SENSOR_SENSITIVITY, -20000, 20000);

        /* Apply callibration and fill output buffer */
        if (callibration_array == nullptr) {
            results[0] = current_r;
            results[1] = current_l;
        } else {
            results[0] = current_r - callibration_array[0];
            results[1] = current_l - callibration_array[1];
        }

    } else {
        Serial.println("I2C bus busy, skipping this read.");
    }
}


static status_t callibrateADC(int16_t *callibration_array) {

    status_t status = STATUS_OK;

    /* Set the callibrating flag. This prevents the serial task from setting a new motion target */
    calibrating = true;

    /* Take the mutex before updating the shared struct (also acts as a memory barrier so volatile isn't needed) */
    if (xSemaphoreTake(motion_target.mutex, portMAX_DELAY) != pdTRUE) {
        status = STATUS_ERROR;
        return status;
    }

    /* Set the target speed to 0 */
    motion_target.linear_velocity  = 0;
    motion_target.angular_velocity = 0;

    /* Release the mutex and wait for the motor task to set the speed to 0 */
    if (xSemaphoreGive(motion_target.mutex) != pdTRUE) {
        status = STATUS_ERROR;
        return status;
    }
    vTaskDelay(1000);

    /* Read the offset of the ADC */
    readADC(nullptr, callibration_array);

    /* Clear the calibrating flag to allow writing new motion targets */
    calibrating = false;

    return status;
}


void currentTask(void *parameter) {

    /* Create currents arrays */
    int16_t currents[2];
    int16_t callibrations[2];

    /* Start the ADC */
    ads.setGain(GAIN_ONE);       // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
    if (!ads.begin(0x48, &Wire)) // 0x49 is decimal 73
    {
        Serial.println("Failed to initialize ADS.");
        vTaskDelete(nullptr);
    }

    /* Callibrate the ADC */
    callibrateADC(callibrations);

    /* Set timing parameters */
    TickType_t        xLastWakeTime = xTaskGetTickCount();
    static TickType_t LOOP_PERIOD   = pdMS_TO_TICKS(1000 / CURRENT_TASK_FREQ);

    for (;;) {
        xTaskDelayUntil(&xLastWakeTime, LOOP_PERIOD);

        /* Read the current */
        readADC(callibrations, currents);

        /* Set the overcurrent flag */
        if ((abs(currents[0]) > current_limit) || (abs(currents[1]) > current_limit)) {
            overcurrent = true;
        } else {
            overcurrent = false;
        }

        /* Send the current message to the host */
        Serial.printf("<R_CURRENT:%u>\n", abs(currents[0]));
        Serial.printf("<L_CURRENT:%u>\n", abs(currents[1]));
    }
}