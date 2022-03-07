#include <time.h>
#include <pthread.h>
#include "modbus.h"
#include "uart.h"
#include "pwm.h"
#include "pid.h"
#include "bme280.h"
#include "linux_userspace.h"
#include "lcd.h"

// Modos de controle
#define POTENCIOMETRO 0
#define CURVAS 1
#define TERMINAL 2

// Comandos UART
#define LIGAR 0x01
#define DESLIGAR 0x02
#define MODO_POTENCIOMETRO 0x03
#define MODO_CURVAS 0x04

int uart;
float tempR, tempI;
char on, tempRefMode = 0;
int totalTime = 0;
int timeC = 0;
double pid, tempE;
FILE* csv;
struct bme280_dev bme;
struct identifier id;

pthread_t thread_update, thread_csv;

void end(int s){	
	printf("\nEncerrando\n");
	sendSisState(uart, 0);
	pwmSet(RESISTOR, 0);
	pwmSet(VENTOINHA, 0);
	pwmClose();
	close(uart);
	fclose(csv);
	exit(0);
}

float curveHandler(){
	FILE* stream = fopen("curva_reflow.csv", "r");
	if (stream == NULL){
		return tempR;
	}
	int seconds, temp;
	int tempB;

	char buffer[25];
	fgets(buffer, 25, stream);
	while(fscanf(stream, "%d, %d", &seconds, &temp) != EOF){
		if(seconds > timeC){
			temp = tempB;
			break;
		}
		tempB = temp;
	}
	if(timeC > 600)
		timeC = 0;
	fclose(stream);

	return temp;
}

void *saveCSV(void *args){
	struct tm *timenow;
	while(1){
		time_t now = time(NULL);
		timenow = gmtime(&now);
		char dateString[20];
		char timeString[20];
		strftime(dateString, sizeof(dateString), "%Y-%m-%d", timenow);
		strftime(timeString, sizeof(timeString), "%H:%M:%S", timenow);
		fprintf(csv, "%s, %s, %f, %f, %f, %.2lf\n", dateString, timeString, tempI, tempE, tempR, pid);
		sleep(1);
		timeC++;
	}
}

void getTemps(){
	tempE = bmeGetTemp(&bme); // Temperatura Externa
	tempI = requestIntTemp(uart); // Temperatura Interna
}

void updateTemp(){
	// Temperatura de referencia
	if(tempRefMode == POTENCIOMETRO){ // Potenciometro
		tempR =	requestPotTemp(uart); 
	} else if (tempRefMode == CURVAS){ // Curvas
		tempR = curveHandler();		
		sendRefFloat(uart, tempR);
	} else // Terminal
		sendRefFloat(uart, tempR);	

	pid_atualiza_referencia(tempR);

	// Calculo PID
	if(tempR < tempE) 
		tempR = tempE;
	pid = pid_controle(tempI);
	sendControlInt(uart, pid);
	// PWM
	if(pid > 0.0)
		pwmSet(RESISTOR, (int) pid);
	else 
		if(pid < -40.0)
			pwmSet(VENTOINHA, (int) -pid);
		
		else{
			pwmSet(VENTOINHA, 40);
			pid = -40;
		}
	sendControlInt(uart, pid);
}


void readCommands() {

	int command = requestUserComm(uart);
	if(command == LIGAR){
		on = 1;
		sendSisState(uart, on);
	} else if (command == DESLIGAR){
		on = 0;
		pwmSet(VENTOINHA, 0);
		pwmSet(RESISTOR, 0);
		sendSisState(uart, on);
		pid = 0;
	} else if (command == MODO_POTENCIOMETRO){
		tempRefMode = POTENCIOMETRO;
		sendControlMode(uart, tempRefMode);
	} else if (command == MODO_CURVAS){
		tempRefMode = CURVAS;
		timeC = 0;
		sendControlMode(uart, tempRefMode);
	}
}

void *update(void *args) {

	while(1) {
		getTemps(); // Medicoes
		if(on) updateTemp(); // Atualiza temperaturas (Aplica PID e PWM)
		lcdWriteTemperatures(on, tempRefMode, tempR, tempE, tempI); // Display LCD
		readCommands();	// Leitura de comandos
	}
}

int main(int argc, const char * argv[]) {

	// Inits		
	signal(SIGINT, end);	
	uart = openDevice("/dev/serial0");
	pwmInit();
	lcd_init();
	bme280Init("/dev/i2c-1", &bme, &id);
	on = 0;
	sendSisState(uart, on);

	// Medicoes iniciais
	getTemps();

	// CSV Start
	char filename[40];
	struct tm *timenow;
	time_t now = time(NULL);
	timenow = gmtime(&now);
	strftime(filename, sizeof(filename), "log/LOG_%Y-%m-%d_%H:%M:%S.csv", timenow);
	csv = fopen(filename, "w");
	if (csv == NULL){
		perror("Error");
		end(SIGINT);
	}
	fprintf(csv, "Data, Hora, TI, TE, TR, Atuadores\n");


	// Constantes
	float KP, KI, KD;
	printf("Insira os valor para KP, KI, KD\n");
	scanf("%f%f%f", &KP, &KI, &KD);
	pid_configura_constantes(KP, KI, KD);
	
	// Threads - Atualizacao e CSV
	pthread_create(&thread_update, NULL, &update, NULL);
	pthread_create(&thread_csv, NULL, &saveCSV, NULL);
	
	// Interface do Terminal
	int choice;
	while(1){
		printf("====================\n");
		printf("Estado: %d\n", on);
		printf("Modo: %d\n", tempRefMode);
		printf("Temperatura de Referência: %f\n", tempR);
		printf("Temperatura Interna: %f\n", tempI);
		printf("Temperatura Externa: %lf\n", tempE);
		if(tempRefMode == CURVAS){
			printf("Tempo na Curva: %d\n", timeC);
		}
		printf("\nDigite 1 para alterar o estado\n");
		printf("Digite 2 para alterar o modo\n");
		printf("Digite outro numero para atualizar a tela\n");		

		scanf("%d", &choice);
			
		switch(choice){
			case 1:
				on = on ? 0 : 1;
				sendSisState(uart, on);
				break;

			case 2:
				printf("Qual será a fonte da temperatura de referência?\n");
				printf("	0 - UART\n");
				printf("	1 - Curvas\n");
				printf("	2 - Teclado\n");
				timeC = 0;
				scanf("%d", &tempRefMode);
				sendControlMode(uart, tempRefMode > 1 ? 1 : tempRefMode);
				if(tempRefMode == TERMINAL){
					printf("Digite o valor para a temperatura de referência\n");
					scanf("%f", &tempR);
				}
				break;
		}	
	}

   return 0;
}
