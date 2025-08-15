#include <task.h>


TaskHandle_t samsCerealTaskHandle = nullptr;


/* Takes a single message like "<MOTOR:20>" and sends it where it needs to go */
status_t parseMessage(String input) {

    status_t status = STATUS_OK;
    String   key;
    String   value;
    uint8_t  colonIndex;

    /* Check the message format is valid */
    if (input.startsWith("<") && input.endsWith(">")) {
        input = input.substring(1, input.length() - 1);
    }

    /* Message invalid */
    else {
        status = STATUS_INVALID;
        Serial.println("Message did not begin and end with <>");
        return status;
    }

    /* Get the key-value separator index */
    colonIndex = input.indexOf(':');
    if (colonIndex == -1) {
        status = STATUS_INVALID;
        return status;
    }

    /* Extract the key and value */
    key   = input.substring(0, colonIndex);
    value = input.substring(colonIndex + 1);
    value.toLowerCase();
    key.toUpperCase();

    /* Respond to a ping message with pong */
    if (key == MSG_PING) {
        Serial.println("<PONG:1>");
        return status;
    }

    /* Otherwise the message is about motion */

    /* Ignore message while calibrating */
    if (calibrating) {
        status = STATUS_BUSY;
        Serial.println("Message ignored due to calibration");
        return status;
    }

    /* Take the mutex before updating the shared struct (also acts as a memory barrier so volatile isn't needed) */
    if (xSemaphoreTake(motion_target.mutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        status = STATUS_ERROR;
        return status;
    }

    /* New linear speed */
    if (key == MSG_LINEAR) {
        motion_target.linear_velocity = constrain(value.toInt(), -255, 255);
    }

    /* New angular speed */
    else if (key == MSG_ANGULAR) {
        motion_target.angular_velocity = constrain(value.toInt(), -255, 255);
    }

    /* Invalid type */
    else {
        status = STATUS_INVALID;
    }

    /* Update the timestamp (this is so that commands can be timed out for safety) */
    if (status == STATUS_OK) motion_target.timestamp = xTaskGetTickCount();

    /* Release the mutex */
    if (xSemaphoreGive(motion_target.mutex) != pdTRUE) status = STATUS_ERROR;

    return status;
}


void samsCerealTask(void *) {

    status_t status = STATUS_OK;

    /* Set timing parameters */
    TickType_t        xLastWakeTime = xTaskGetTickCount();
    static TickType_t LOOP_PERIOD   = pdMS_TO_TICKS(1000 / SAMS_CEREAL_FREQ);

    /* Create the message variables */
    static String rx;
    char          c;

    for (;;) {

        /* New message */
        while (Serial.available()) {

            /* Read a new character */
            c = Serial.read();

            /* If the character denotes the end of a message then parse the message */
            if (c == '\n') {
                status = parseMessage(rx);
                if (status != STATUS_OK) {
                    Serial.println("Error while parsing message");
                }
                rx = "";
            }

            /* Otherwise continue accumulating characters */
            else if (isPrintable(c) && rx.length() < 64) {
                rx += c;
            }
        }
        xTaskDelayUntil(&xLastWakeTime, LOOP_PERIOD);
    }
}
