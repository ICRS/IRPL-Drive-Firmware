#include <task.h>
#define BAUD_RATE 115200
TaskHandle_t samsCerealTaskHandle = nullptr;
static constexpr TickType_t LOOP_PERIOD =
    pdMS_TO_TICKS(1000UL / SAMS_CEREAL_FREQ);   // ticks per iteration

// Takes a single message like "<MOTOR:20>" and outputs the correct message 
Message parseMessage(String input){
    Message output;
    String key;
    String value;
    if (input.startsWith("<") && input.endsWith(">")) {
        input = input.substring(1, input.length() - 1);
    } else {
        output.type = MessageType::ERROR;
        output.errorCode = 0; // Not a samsCereal message 
        return output;  
    }

    int colonIndex = input.indexOf(':');
    
    if (colonIndex!=-1){
        key = input.substring(0,colonIndex);
        value = input.substring(colonIndex+1);
        value.toLowerCase();
        key.toUpperCase();
    } else {
        output.type = MessageType::ERROR;
        output.errorCode = 1; // No key-value pair
        return output;  
    }

    // Big if else to sort message types (change to switch case with enums at some point)
    if (key=="PING"){
        output.type = MessageType::PING_IN;
        output.pingValue = (value=="true");
        String ping_out = "<PONG:"+ String(output.pingValue) +'>';
        Serial.println(ping_out);
    }else if (key=="MOTOR_L"){
        output.type = MessageType::MOTOR_L;
        output.motorValue = value.toInt();
        motor_l_val = output.motorValue;
        //String motor_out = "<MOTOR_L:"+ String(motor_l_val) +'>';
    }else if (key=="MOTOR_R"){
        output.type = MessageType::MOTOR_R;
        output.motorValue = value.toInt();
        motor_r_val = output.motorValue;
        //String motor_out = "<MOTOR_R:"+ String(motor_r_val) +'>';
    }
    else{
        output.type = MessageType::ERROR;
        output.errorCode = 3; // Unknown key
    }

    return output;
}



void samsCerealTask(void *) {
    Serial.begin(BAUD_RATE);           // host side should match
    Serial.setTimeout(2);           // 2 ms read timeout
    TickType_t xLastWakeTime = xTaskGetTickCount();   // initialise once

    static String rx;

    for (;;) {
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n') {
                parseMessage(rx);
                rx = "";
            } else if (isPrintable(c) && rx.length() < 64) {
                rx += c;
            }
        }
        xTaskDelayUntil(&xLastWakeTime, LOOP_PERIOD);
    }
}
