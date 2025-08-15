#include <task.h>
#include <Wire.h>

void setup() {

    motion_target.mutex = xSemaphoreCreateMutex();
    i2c_mutex           = xSemaphoreCreateMutex();

#if ENABLE_SAMS_CEREAL == true
    xTaskCreate(
        samsCerealTask,       // Function that should be called
        "Serial Handler",     // Name of the task (for debugging)
        8192,                 // Stack size (bytes)
        NULL,                 // Parameter to pass
        1,                    // Task priority
        &samsCerealTaskHandle // Task handle
    );
#endif

#if ENABLE_MOTOR == true
    xTaskCreate(
        motorTask,       // Function that should be called
        "Motor Control", // Name of the task (for debugging)
        4096,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        &motorTaskHandle // Task handle
    );
#endif

#if ENABLE_CURRENT == true
    xTaskCreate(
        currentTask,       // Function that should be called
        "Current Sensing", // Name of the task (for debugging)
        8192,              // Stack size (bytes)
        NULL,              // Parameter to pass
        1,                 // Task priority
        &currentTaskHandle // Task handle
    );
#endif

    /* Start USB communication */
    Serial.begin(BAUD_RATE);
    Serial.setTimeout(2); // 2 ms read timeout

    /* Start I2C communication */
    Wire.begin(SDA, SCL, 100000);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}