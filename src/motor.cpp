#include <task.h>


TaskHandle_t    motorTaskHandle = nullptr;
motion_target_t motion_target;


static void apply_overcurrent_correction(int16_t *left, int16_t *right) {

    static uint8_t overcurrent_correction = OVERCURRENT_CORRECTION_DEFAULT; /* Persist between function calls */

    if (overcurrent) {

        /* Apply to the left side (don't go past 0) */
        if (*left > 0) {
            *left = *left - constrain(overcurrent_correction, 0, *left);
        } else {
            *left = *left + constrain(overcurrent_correction, 0, abs(*left));
        }

        /* Apply to the right side (don't go past 0) */
        if (*right > 0) {
            *right = *right - constrain(overcurrent_correction, 0, *right);
        } else {
            *right = *right + constrain(overcurrent_correction, 0, abs(*right));
        }

        /* Add to the correction amount in case it is still over the current threshold next time */
        if (255 - (int16_t) overcurrent_correction >= constrain(OVERCURRENT_CORRECTION_INCREMENT, 1, 255)) {
            overcurrent_correction += constrain(OVERCURRENT_CORRECTION_INCREMENT, 1, 255);
        } else {
            overcurrent_correction = 255;
        }

    }

    /* If no overcurrent return to the default slowly */
    else if (overcurrent_correction != OVERCURRENT_CORRECTION_DEFAULT) {
        overcurrent_correction = ((9 * (uint16_t) overcurrent_correction) + OVERCURRENT_CORRECTION_DEFAULT) / 10;
    }
}


void setMotorR(int speed) {
    if (speed > 0) {
        analogWrite(PWM_R, speed);
        digitalWrite(IN1_R, 0);
        digitalWrite(EN_R, 1);
    } else if (speed < 0) {
        analogWrite(PWM_R, -speed);
        digitalWrite(IN1_R, 1);
        digitalWrite(EN_R, 0);
    } else {
        digitalWrite(IN1_R, 1);
        digitalWrite(EN_R, 1);
    }
}

void setMotorL(int speed) {
    if (speed > 0) {
        analogWrite(PWM_L, speed);
        digitalWrite(IN1_L, 0);
        digitalWrite(EN_L, 1);
    } else if (speed < 0) {
        analogWrite(PWM_L, -speed);
        digitalWrite(IN1_L, 1);
        digitalWrite(EN_L, 0);
    } else {
        digitalWrite(IN1_L, 1);
        digitalWrite(EN_L, 1);
    }
}


void motorTask(void *parameter) {

    status_t status = STATUS_OK;

    /* Setup pins */
    pinMode(IN1_L, OUTPUT);
    pinMode(EN_L, OUTPUT);
    pinMode(IN1_R, OUTPUT);
    pinMode(EN_R, OUTPUT);
    pinMode(PWM_R, OUTPUT);
    pinMode(PWM_L, OUTPUT);

    /* Set the PWM resolution to 8 bits (0 to 255) */
    analogWriteResolution(8);

    /* Setup local velocity targets */
    int16_t  linear_velocity_target  = 0;
    int16_t  angular_velocity_target = 0;
    uint32_t last_timestamp          = 0; /* Used to timeout old messages for safety */

    /* Setup motor speeds */
    int16_t left;
    int16_t right;
    int16_t high;

    /* Set timing parameters */
    TickType_t        xLastWakeTime = xTaskGetTickCount();
    static TickType_t LOOP_PERIOD   = pdMS_TO_TICKS(1000 / MOTOR_TASK_FREQ);

    for (;;) {

        status = STATUS_OK;
        xTaskDelayUntil(&xLastWakeTime, LOOP_PERIOD);

#if PRINT_SPEEDS == true
        static uint8_t _i;
        if (_i == 10) {
            Serial.printf("Speeds = %i, %i\nOvercurrent = %i\n", left, right, (int) overcurrent);
            _i = 0;
        } else {
            _i++;
        }
#endif

        /* ---------- Control logic ---------- */

        /* Take the mutex before reading the shared motion target struct (also acts as a memory barrier so volatile isn't needed) */
        if (xSemaphoreTake(motion_target.mutex, pdMS_TO_TICKS(5)) == pdTRUE) {

            /* Store locally */
            linear_velocity_target  = motion_target.linear_velocity;
            angular_velocity_target = motion_target.angular_velocity;
            last_timestamp          = motion_target.timestamp;

            /* Release the mutex */
            if (xSemaphoreGive(motion_target.mutex) != pdTRUE) status = STATUS_ERROR;

        } else {
            status = STATUS_ERROR;
        }

        /* Stop moving if a new command isn't received before the timeout */
        if (xTaskGetTickCount() > last_timestamp + MOTION_TARGET_TIMEOUT) {
            linear_velocity_target  = 0;
            angular_velocity_target = 0;
        }

        /* ---------- Speed logic ---------- */

        /* Mix targets into wheel speeds */

#if INVERT_TURN == true
        left  = linear_velocity_target - angular_velocity_target;
        right = linear_velocity_target + angular_velocity_target;
#else
        left  = linear_velocity_target + angular_velocity_target;
        right = linear_velocity_target - angular_velocity_target;
#endif

        /* Scale the speeds if required */
        high = max(abs(left), abs(right));
        if (high > constrain(MAXIMUM_SPEED, 0, 255)) {
            left  = (left * constrain(MAXIMUM_SPEED, 0, 255)) / high;
            right = (right * constrain(MAXIMUM_SPEED, 0, 255)) / high;
        }

        /* Apply an overcurrent correction */
        apply_overcurrent_correction(&left, &right);

#if INVERT_LEFT == true
        left = -left;
#endif
#if INVERT_RIGHT == true
        right = -right;
#endif

        /* Write the motor speeds */
        setMotorL(left);
        setMotorR(right);
    }
}
