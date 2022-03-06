#ifndef MODBUS_H
#define MODBUS_H

#include "uart.h"
#include "crc16.h"
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#define ADDRESS 0x01
#define REQUEST 0x23
#define SEND 0x16

int receiveInteger(int device);
float receiveFloat(int device);
float requestIntTemp(int device);
float requestPotTemp(int device);
int requestUserComm(int device);
void sendControlInt(int device, int controlInt);
void sendRefFloat(int device, float refFloat);
int sendSisState(int device, char state);
int sendControlMode(int device, char mode);

#endif
