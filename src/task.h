#ifndef TASK_H
#define TASK_H

#include <SPI.h>
#include <Arduino.h>
#include "stdint-gcc.h"

#ifdef V3
#include "v3/pins.h"
#endif

#ifdef SMITH32
#include "smith32/pins.h"
#endif


#define BAUD_RATE                        115200

#define MSG_PING                         "PING"
#define MSG_LINEAR                       "LINEAR"
#define MSG_ANGULAR                      "ANGULAR"
#define MSG_I_LIMIT                      "I_LIMIT"

#define MOTION_TARGET_TIMEOUT            500 /* ms, if a message isn't received for this amount of time then stop moving */
#define MAXIMUM_SPEED                    255 /* 0 to 255 */
#define PRINT_SPEEDS                     true
#define INVERT_TURN                      false
#define INVERT_LEFT                      false
#define INVERT_LEFT                      false
#define INVERT_RIGHT                     true

#define CURRENT_SENSOR_SUPPLY_VOLTAGE    5.0f
#define CURRENT_MAX_VOLTAGE              3.3112583f                      /* Due to CURRENT_SENSOR_SUPPLY_VOLTAGE through a 5.1k, 10k potential divider */
#define CURRENT_SENSOR_SENSITIVITY       0.0001f                         /* V/mA  = 100 mV/A */
#define CURRENT_LIMIT_DEFAULT            10000                           /* mA, if the current for either side is greater than this the overcurrent flag will be triggered */
#define OVERCURRENT_CORRECTION_DEFAULT   50                              /* 0 to 255, the minimum ammount to reduce the wheel speeds by when an overcurrent condition is detected. Default is 50 */
#define OVERCURRENT_CORRECTION_INCREMENT ((1000 / MOTOR_TASK_FREQ) / 10) /* 0 to 255, how fast the speed is reduced to 0 in case of a prolonged overcurrent. t = (255 - OVERCURRENT_CORRECTION_DEFAULT) / (MOTOR_TASK_FREQ * OVERCURRENT_CORRECTION_INCREMENT). Default is 2 which will stop after 2.05 seconds */


/*<-----    Shared structs  ----->*/

typedef enum {
    STATUS_OK,
    STATUS_ERROR,
    STATUS_INVALID,
    STATUS_BUSY,
} status_t;

typedef struct {
    int16_t           linear_velocity;
    int16_t           angular_velocity;
    uint32_t          timestamp;
    SemaphoreHandle_t mutex;
} motion_target_t;


/*<-----    Task functions  ----->*/

void samsCerealTask(void *parameter);
void motorTask(void *parameter);
void currentTask(void *parameter);

/*<-----    Task handles    ----->*/

extern TaskHandle_t samsCerealTaskHandle;
extern TaskHandle_t motorTaskHandle;
extern TaskHandle_t currentTaskHandle;


/*<-----    Task frequencies    ----->*/

#define SAMS_CEREAL_FREQ  50 /* Frequency in Hz*/
#define MOTOR_TASK_FREQ   50 /* Frequency in Hz*/
#define CURRENT_TASK_FREQ 10 /* Frequency in Hz*/


/*<-----    Task enables    ----->*/

#define ENABLE_SAMS_CEREAL true
#define ENABLE_MOTOR       true
#define ENABLE_CURRENT     true

/*<-----    Shared variables    ----->*/
extern volatile uint32_t current_limit;
extern volatile bool overcurrent;
extern volatile bool calibrating;

extern SemaphoreHandle_t i2c_mutex;
extern motion_target_t   motion_target;


#endif