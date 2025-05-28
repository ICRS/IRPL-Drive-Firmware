#include <task.h>
#define BAUD_RATE 115200
TaskHandle_t samsCerealTaskHandle = nullptr;

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
        String motor_out = "<MOTOR_L:"+ String(motor_l_val) +'>';
        Serial.println(motor_out);
    }else if (key=="MOTOR_R"){
        output.type = MessageType::MOTOR_R;
        output.motorValue = value.toInt();
        motor_r_val = output.motorValue;
        String motor_out = "<MOTOR_R:"+ String(motor_r_val) +'>';
        Serial.println(motor_out);
    }
    else{
        output.type = MessageType::ERROR;
        output.errorCode = 3; // Unknown key
    }

    return output;
}



void samsCerealTask(void * parameter){

    Serial.begin(BAUD_RATE);
    Message incomingMessage;

    for(;;){
        String incoming;
        if(Serial.available()>0){
            incoming = Serial.readStringUntil('\n');
            incomingMessage = parseMessage(incoming);
            
            if(incomingMessage.type==MessageType::ERROR){
                Serial.print("Failed to parse message with code: ");
                Serial.println(incomingMessage.errorCode);
            }
        }
        vTaskDelay((1000/SAMS_CEREAL_FREQ) / portTICK_PERIOD_MS);
    }
}

