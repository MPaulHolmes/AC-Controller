#ifndef UART4011_H
#define UART4011_H
#include "p30F4011.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uart.h"


#define MAX_COMMAND_LENGTH 40

typedef struct UARTCommand_typ {
	volatile int i;
	volatile int complete;
	volatile char string[MAX_COMMAND_LENGTH];
	volatile unsigned int number;  // the number part of the command.  Ex:  kp 3000.  string = {'k','p'}.  number = 3000.
} UARTCommand;

typedef struct dataStreamTyp {
	volatile unsigned int timeOfLastTransmission;
	volatile unsigned int period;
	volatile unsigned int startTime;
	volatile unsigned int timer;
	volatile int showStreamOnce;
	volatile int Id_times10;
	volatile int Iq_times10;
	volatile int IdRef_times10;
	volatile int IqRef_times10;
	volatile int Vd;
	volatile int Vq;
	volatile int Ia_times10;
	volatile int Ib_times10;
	volatile int Ic_times10;
	volatile int Va;
	volatile int Vb;
	volatile int Vc;
	volatile int percentOfVoltageDiskBeingUsed;
	volatile int batteryAmps_times10;
	volatile int rawThrottle;
	volatile int throttle;
	volatile int temperature;
	volatile int slipSpeedRPM;
	volatile int electricalSpeedRPM;
	volatile int mechanicalSpeedRPM;
} dataStream;

void InitUART2();
int TransmitReady();
int TransmitReadyAlt();
void SendCharacter(char ch);
int ReceiveBufferHasData();
unsigned char GetCharacter();
void ClearReceiveBuffer();
void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void);

#endif
