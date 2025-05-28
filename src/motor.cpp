#include <task.h>
TaskHandle_t motorTaskHandle = nullptr;
volatile int motor_l_val = 0;
volatile int motor_r_val = 0;

void setMotorR(int speed)
{
    
  if (speed > 0)
  {
    analogWrite(EN_R,speed);
    digitalWrite(IN1_R, 0);
    digitalWrite(IN2_R, 1);
  }
  else if (speed <0)
  {
    analogWrite(EN_R,-speed);
    digitalWrite(IN1_R, 1);
    digitalWrite(IN2_R, 0);
  }
  else{
    digitalWrite(IN1_R, 1);
    digitalWrite(IN2_R, 1);
  }
}

void setMotorL(int speed)
{
  if (speed > 0)
  {
    analogWrite(EN_L,speed);
    digitalWrite(IN1_L, 0);
    digitalWrite(IN2_L, 1);
  }
  else if (speed <0)
  { 
    analogWrite(EN_L,-speed);
    digitalWrite(IN1_L, 1);
    digitalWrite(IN2_L, 0);
  }
  else{
    digitalWrite(IN1_L, 1);
    digitalWrite(IN2_L, 1);
  }
}



void motorTask(void *parameter)
{
    pinMode(IN1_L,OUTPUT);
    pinMode(IN2_L,OUTPUT);
    pinMode(IN1_R,OUTPUT);
    pinMode(IN2_R,OUTPUT);
    pinMode(EN_R,OUTPUT);
    pinMode(EN_L,OUTPUT);

    for (;;)
    {
        setMotorL(motor_l_val);
        setMotorR(motor_r_val);
        vTaskDelay((1000 / MOTOR_TASK_FREQ) / portTICK_PERIOD_MS);
    }
}
