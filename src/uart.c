#include "uart.h"

int openDevice(char *name){

	int uart = -1;
	
	uart = open(name, O_RDWR | O_NOCTTY | O_NDELAY);

    if (uart == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
		return -1;
    }
    else
    {
        printf("UART inicializada!\n");
    }    
    struct termios options;
    tcgetattr(uart, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart, TCIFLUSH);
    tcsetattr(uart, TCSANOW, &options);

	return uart;
}

void send(int device, unsigned char *msg, int tam){
	if (device != -1)
    {
        //printf("Escrevendo caracteres na UART ...");
        int count = write(device, msg, tam);
        if (count < 0)
        {
            printf("UART TX error\n");
        }
		/*        
		else
        {
             //printf("escrito.\n");
        }
		*/
    	sleep(1);
    }

}

void receive(int device, int tam, void *buffer){
	if (device != -1)
		{
		    int rx_length = read(device, buffer, tam);
		    if (rx_length < 0)
		    {
		        printf("Erro na leitura.\n");
		    }
		    else if (rx_length == 0)
		    {
		        printf("Nenhum dado disponível.\n");
		    }
		}
}
