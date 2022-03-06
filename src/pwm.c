#include "pwm.h"

void pwmInit(){
	wiringPiSetup();
	pinMode(RESISTOR, OUTPUT);
	softPwmCreate(RESISTOR, 0, 100);
	pinMode(VENTOINHA, OUTPUT);
	softPwmCreate(VENTOINHA, 0, 100);
}

void pwmClose(){
	softPwmStop(RESISTOR);
	softPwmStop(VENTOINHA);
}

void pwmSet(int pin, int value){
	if(value > 0){
		if(pin == RESISTOR)
			softPwmWrite(VENTOINHA, 0);
		else if(pin == VENTOINHA)
			softPwmWrite(RESISTOR, 0);
	}
	softPwmWrite(pin, value);
}
