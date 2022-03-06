#ifndef UART_H
#define UART_H
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <stdio.h>

int openDevice(char *name);
void send(int device, unsigned char *msg, int tam);
void receive(int device, int tam, void *buffer);

#endif
