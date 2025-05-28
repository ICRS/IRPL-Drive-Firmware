#include <task.h>


void setup(){

    #if ENABLE_SAMS_CEREAL == true
    xTaskCreate(
        samsCerealTask,    // Function that should be called
        "Serial Handler",   // Name of the task (for debugging)
        4096,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        &samsCerealTaskHandle             // Task handle
      );
    #endif

     #if ENABLE_MOTOR == true
    xTaskCreate(
        motorTask,    // Function that should be called
        "Motor Control",   // Name of the task (for debugging)
        4096,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        &motorTaskHandle             // Task handle
      );
    #endif

    
}

void loop(){
    vTaskDelay(pdMS_TO_TICKS(1000));
}