#ifndef TASK_H
#define TASK_H

#include <SPI.h>
#include <Arduino.h>
#ifdef V3
#include "v3/pins.h"
#endif

#ifdef SMITH32
#include "smith32/pins.h"
#endif



/*<-----    Task functions  ----->*/

void samsCerealTask(void * parameter);
void motorTask(void * parameter);
void currentTask(void * parameter);

/*<-----    Task handles    ----->*/

extern TaskHandle_t samsCerealTaskHandle;
extern TaskHandle_t motorTaskHandle;
extern TaskHandle_t currentTaskHandle;



/*<-----    Task frequencies    ----->*/

#define SAMS_CEREAL_FREQ 50
#define MOTOR_TASK_FREQ 50
#define CURRENT_TASK_FREQ 10



/*<-----    Task enables    ----->*/

#define ENABLE_SAMS_CEREAL true
#define ENABLE_MOTOR true
#define ENABLE_CURRENT true

/*<-----    Shared variables    ----->*/
extern volatile int motor_r_val;
extern volatile int motor_l_val;

/*<-----    Shared structs  ----->*/
enum MessageType {
    PING_IN, 
    PING_OUT,
    ERROR,
    MOTOR_L,
    MOTOR_R,
};
 

struct Message {
    MessageType type;
    union {
        bool pingValue;     // PING_IN, PING_OUT
        int errorCode;      // ERROR
        int motorValue;     // MOTOR
    };
};

#endif