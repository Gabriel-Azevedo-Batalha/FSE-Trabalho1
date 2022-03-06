#ifndef PWM_H
#define PWM_H

#include <wiringPi.h>
#include <softPwm.h>
#define RESISTOR 4
#define VENTOINHA 5

void pwmInit();
void pwmClose();
void pwmSet(int pin, int value);

#endif
