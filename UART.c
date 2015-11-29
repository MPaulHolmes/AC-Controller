#include "UART.h"

void ShowMenu(void);
void ShowConfig(unsigned int mask);
void u16x_to_str(char *str, unsigned val, unsigned char digits);
void u16_to_str(char *str, unsigned val, unsigned char digits);
void int16_to_str(char *str, int val, unsigned char digits);
int TransmitString(const char* str);
char IntToCharHex(unsigned int i);
void FetchRTData(void);
extern void InitPIStruct(void);
extern void EESaveValues(void);
extern void InitializeThrottleAndCurrentVariables(void);


volatile UARTCommand myUARTCommand = {0,0,{0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0},0};

extern volatile int IqRefRef;
extern volatile int IdRefRef;

extern volatile int maxRPS_times16;
extern volatile unsigned int faultBits;
extern volatile SavedValuesStruct savedValues;
extern volatile SavedValuesStruct2 savedValues2;
extern unsigned int revCounterMax;
	
extern volatile unsigned int counter10k;
extern volatile unsigned int counter1k;
extern volatile piType myPI;
extern volatile rotorType myRotor;

volatile dataStream myDataStream;

volatile char newChar = 0;
volatile int echoNewChar = 0;
volatile dataStream myDataStream;
volatile char intString[] = "xxxxxxxxxx";
					//      0         1         2         3         4
					//      01234567890123456789012345678901234567890	
char showConfigString[] = {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"};

void InitUART2() {
	U2BRG = 15; //For 14.7MHz, 115200bps=7 38.4kbps==23.  For 29.5MHz, 115200bps == 15.
	U2MODE = 0;  // initialize to 0.
	U2MODEbits.PDSEL = 0b00; // 8 N 
	U2MODEbits.STSEL = 0; // 1 stop bit.

	IEC1bits.U2RXIE = 1;  // enable receive interrupts.
	IPC6bits.U2RXIP = 2;	// INTERRUPT priority of 2.
//bit 7-6 URXISEL<1:0>: Receive Interrupt Mode Selection bit
//11 =Interrupt flag bit is set when Receive Buffer is full (i.e., has 4 data characters)
//10 =Interrupt flag bit is set when Receive Buffer is 3/4 full (i.e., has 3 data characters)
//0x =Interrupt flag bit is set when a character is received
	U2STAbits.URXISEL = 0b00;  // 0b11 later..

	U2MODEbits.UARTEN = 1; // enable the uart
	asm("nop");
	U2STAbits.UTXEN = 1; // Enable transmissions
}

void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void) {
	IFS1bits.U2RXIF = 0;  // clear the interrupt.
	echoNewChar = 1;

	if (myUARTCommand.complete == 1) {	// just ignore everything until the command is processed.
		return;
	}
	newChar = U2RXREG;		// get the character that caused the interrupt.

	if (newChar == 0x0d) {	// carriage return.
		myUARTCommand.complete = 1;
		myUARTCommand.string[myUARTCommand.i] = 0;  // instead of placing a carriage return, place a 0 to null terminate the string.
		return;
	}
	if (myUARTCommand.i >= MAX_COMMAND_LENGTH) {  // the command was too long.  It's just garbage anyway, so start over.
		//myUARTCommand.complete = 0;  // It can't make it here unless myUARTCommand.complete == 0 anyway.
		myUARTCommand.i = 0;	// just clear the array, and start over.
		myUARTCommand.string[0] = 0;
//		myUARTCommand.number = 0;  // This is done in "ProcessCommand", so you don't need to do it here.
		return;
	}
	myUARTCommand.string[myUARTCommand.i] = newChar; // save the character that caused the interrupt!
	myUARTCommand.i++;
}

// process the command, and reset UARTCommandPtr back to zero.
// myUARTCommand is of the form XXXXXXXXX YYYYY<enter>
void ProcessCommand(void) {
	static int i = 0;
	if (echoNewChar) {
		while (echoNewChar) {
			if (U2STAbits.UTXBF == 0) { // TransmitReady();
				U2TXREG = newChar; 	// SendCharacter(newChar);
				if (newChar == 0x0d) {
					while (1) { 
						if (U2STAbits.UTXBF == 0) { // TransmitReady();
							U2TXREG = 0x0a; 	// SendCharacter(line feed);
							break;
						}
					}
				}
				echoNewChar = 0;
			}
		}
	}
	else {
		if (myUARTCommand.complete != 1) {	// if the command isn't yet complete, don't try to process it!  Maybe someone is only half-way done with their command.  Ex:  "sav".  Process "sav"?  No!  wait until they type "save<cr>"
			return;
		}
		myUARTCommand.number = 0;	
		for (i = 0; myUARTCommand.string[i] != 0; i++) {
			if (myUARTCommand.string[i] == ' ') {
				myUARTCommand.number = atoi((char *)&myUARTCommand.string[i+1]);
				myUARTCommand.string[i] = 0;  // null terminate the text portion.			
				break;
			}
		}
		// Let's say you typed the command "kp 1035".  The following would have happened:
		// myUARTCommand.string[] would contain only the text portion of the command, and is terminated with a 0.  string[] = {'p',0,?,?,?,?,?,?,?,?,?,?,?,...}
		// Also, myUARTCommand.number = the number argument after the command. So, number = 1035.
		if (!strcmp((char *)&myUARTCommand.string[0], "kp")) {
			if (myUARTCommand.number <= 32767u && myUARTCommand.number > 0) {
				savedValues.Kp = (int)(myUARTCommand.number); 
				InitPIStruct();
	//			ShowConfig((unsigned)1 << 0);
			}
		}
		else if (!strcmp((char *)&myUARTCommand.string[0], "ki")){ 
			if (myUARTCommand.number <= 32767u && myUARTCommand.number > 0) {
				savedValues.Ki = (int)(myUARTCommand.number); 
				InitPIStruct();
	//			ShowConfig((unsigned)1 << 0);
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "current-sensor-amps-per-volt")) {  // 
			if (myUARTCommand.number <= 480 && myUARTCommand.number > 0) {
				savedValues.currentSensorAmpsPerVolt = (int)(myUARTCommand.number); 
				InitializeThrottleAndCurrentVariables();
			}	
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "max-regen-position")) { 
			if (myUARTCommand.number <= 1023u && myUARTCommand.number > 0) {
				savedValues.maxRegenPosition = (int)(myUARTCommand.number); 
			}	
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "min-regen-position")) { 
			if (myUARTCommand.number <= 1023u && myUARTCommand.number > 0) {
				savedValues.minRegenPosition = (int)(myUARTCommand.number); 
			}	
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "min-throttle-position")) { 
			if (myUARTCommand.number <= 1023u && myUARTCommand.number > 0) {
				savedValues.minThrottlePosition = (int)(myUARTCommand.number); 
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "max-throttle-position")) { 
			if (myUARTCommand.number <= 1023u && myUARTCommand.number > 0) {
				savedValues.maxThrottlePosition = (int)(myUARTCommand.number); 
			}	
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "fault-throttle-position")) { 
			if (myUARTCommand.number <= 1023u && myUARTCommand.number > 0) {
				savedValues.throttleFaultPosition = (int)(myUARTCommand.number); 
			}	
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "max-battery-amps")) { 
			if (myUARTCommand.number <= 9999 && myUARTCommand.number > 0) {
				savedValues.maxBatteryAmps = (int)(myUARTCommand.number); 
				InitializeThrottleAndCurrentVariables();
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "max-battery-amps-regen")) { 
			if (myUARTCommand.number <= 9999 && myUARTCommand.number > 0) {
				savedValues.maxBatteryAmpsRegen = (int)(myUARTCommand.number); 
				InitializeThrottleAndCurrentVariables();
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "max-motor-amps")) { 
			if (myUARTCommand.number <= 999 && myUARTCommand.number > 0) {
				savedValues.maxMotorAmps = (int)(myUARTCommand.number); 
				InitializeThrottleAndCurrentVariables();
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "max-motor-amps-regen")) { 
			if (myUARTCommand.number <= 999 && myUARTCommand.number > 0) {
				savedValues.maxMotorAmpsRegen = (int)(myUARTCommand.number); 
				InitializeThrottleAndCurrentVariables();
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "precharge-time")) { 
			if (myUARTCommand.number <= 9999 && myUARTCommand.number > 0) {
				savedValues.prechargeTime = (int)(myUARTCommand.number); 
			}
		}
		// NOW WE ARE ON SavedValues2...
		else if (!strcmp((const char *)&myUARTCommand.string[0], "rotor-time-constant")) { 
			if (myUARTCommand.number <= ROTOR_TIME_CONSTANT_ARRAY_SIZE+5 && myUARTCommand.number >= 5) {
				myRotor.timeConstantIndex = savedValues2.rotorTimeConstantIndex = (int)(myUARTCommand.number-5);
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "pole-pairs")) {
			if (myUARTCommand.number <= 999 && myUARTCommand.number >= 1) {
				savedValues2.numberOfPolePairs = (int)(myUARTCommand.number); 
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "max-rpm")) { 
			if (myUARTCommand.number <= 32767 && myUARTCommand.number > 0) {
				savedValues2.maxRPM = (int)(myUARTCommand.number); 
				maxRPS_times16 = (((long)savedValues2.maxRPM) << 2) / 15;  // 4/15 to convert to rps_times16
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "throttle-type")) { // 0 means hall effect throttle, or maxOHms to 0 Ohms. 1 means 0 Ohms to maxOhms throttle 
			if (myUARTCommand.number <= 1) {
				savedValues2.throttleType = (int)(myUARTCommand.number); 
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "encoder-ticks")) {
			if (myUARTCommand.number <= 5000u && myUARTCommand.number >= 64) {
				savedValues2.encoderTicks = (int)(myUARTCommand.number); 
				revCounterMax = 160000L / (4*savedValues2.encoderTicks);  // 4* because I'm doing 4 times resolution for the encoder. 160,000 because revolutions per 16 seconds is computed as:  16*10,000*poscnt * rev/(maxPosCnt*revcounter*(16sec)
			}
		}
	//	else if (!strcmp((const char *)&myUARTCommand.string[0], "stator-resistance")) {  // times1024.  So basically mOhms.
	//		if (myUARTCommand.number > 0) {
	//			savedValues2.statorResistance_times1024 = (int)(myUARTCommand.number);
	//		}
	//	}
	//	else if (!strcmp((const char *)&myUARTCommand.string[0], "stator-inductance")) {  // times1024.  So basically mHenries.
	//		if (myUARTCommand.number > 0) {
	//			savedValues2.statorInductance_times1024 = (int)(myUARTCommand.number);
	//		}
	//	}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "pack-voltage")) {  // units are volts.
			if (myUARTCommand.number < 1000 && myUARTCommand.number > 0) {
				savedValues2.packVoltage = (int)(myUARTCommand.number); 
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "pi-ratio")) {
			if (myUARTCommand.number < 1000 && myUARTCommand.number >= 50) {
				myPI.ratioKpKi = (int)(myUARTCommand.number); 
			}
		}
	//	else if (!strcmp((const char *)&myUARTCommand.string[0], "rotor-inductance")) {
	//		if (myUARTCommand.number <= MAX_ROTOR_INDUCTANCE && myUARTCommand.number >= MIN_ROTOR_INDUCTANCE) {
	//			savedValues2.rotorInductance_times1024 = (int)(myUARTCommand.number);
				//LrLmSquared_times128 = LrLmSquared_times128Array[savedValues2.rotorInductance_times1024 - MIN_ROTOR_INDUCTANCE]; 
	//		}
	//	}
	//	else if (!strcmp((const char *)&myUARTCommand.string[0], "sensorless")) {
	//		if (myUARTCommand.number == 1) {
	//			savedValues2.sensorless = 1;
	//		}
	//		else {
	//			savedValues2.sensorless = 0;
	//		}
	//	}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "run-pi-test")) {
			myDataStream.period = 0;  // stop the data stream during this test.
	
			myRotor.testRunning = 0;
			myRotor.testFinished = 0;
	
			myPI.testRunning = 1;
			myPI.testFailed = 1;	
			myPI.testFinished = 0;
			myPI.zeroCrossingIndex = -1;
			myPI.previousTestCompletionTime = counter10k;
			myPI.Kp = myPI.ratioKpKi;
			myPI.Ki = 1;
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "run-rotor-test")) {
			myDataStream.period = 0;  // stop the data stream during this test.
			myPI.testRunning = 0;
			myPI.testFailed = 1;	
			myPI.testFinished = 0;
	
			myRotor.startTime = counter10k;
			myRotor.timeConstantIndex = 0;	// always start at zero, and then it will increment up to around 145, giving each rotorTimeConstant candidate 5 seconds to spin the motor the best it can.
			myRotor.testRunning = 1;
			myRotor.testFinished = 0;
			myRotor.maxTestSpeed = 0;
			myRotor.bestTimeConstantIndex = 0;
		}
		else if ((!strcmp((const char *)&myUARTCommand.string[0], "config")) || (!strcmp((const char *)&myUARTCommand.string[0], "settings"))) {
			ShowConfig(0x0FFFF);
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "data-stream-period")) {  // in milliseconds
			if (myUARTCommand.number > 0) {
				myDataStream.period = myUARTCommand.number;
				myDataStream.showStreamOnce = 0;
		// bit 15 set: display myDataStream.timer
		// Bit 14 set: display myDataStream.Id_times10
		// bit 13 set: display myDataStream.Iq_times10
		// Bit 12 set: display myDataStream.IdRef_times10
		// bit 11 set: display myDataStream.IqRef_times10
		// Bit 10 set: display myDataStream.Vd
		// bit 9 set: display myDataStream.Vq
		// Bit 8 set: display myDataStream.Ia_times10
		// bit 7 set: display myDataStream.Ib_times10
		// bit 6 set: display myDataStream.Ic_times10
		// Bit 5 set: display myDataStream.Va
		// bit 4 set: display myDataStream.Vb
		// bit 3 set: display myDataStream.Vc
		// bit 2 set: display myDataStream.percentOfVoltageDiskBeingUsed
		// bit 1 set: display myDataStream.batteryAmps_times10
		// bit 0 set: future use
		//	int dataToDisplaySet2;
		// Bit 15 set: display myDataStream.rawThrottle
		// bit 14 set: display myDataStream.throttle
		// Bit 13 set: display myDataStream.temperature
		// bit 12 set: display myDataStream.slipSpeedRPM
		// Bit 11 set: display myDataStream.electricalSpeedRPM
		// bit 10 set: display myDataStream.mechanicalSpeedRPM
		// Bit 9-0 set: future use. 
	
				if (savedValues2.dataToDisplaySet1 & 32768) {
					TransmitString("time,");
				}
				if (savedValues2.dataToDisplaySet1 & 16384) {
					TransmitString("Id,");
				}
				if (savedValues2.dataToDisplaySet1 & 8192) {
					TransmitString("Iq,");
				}
				if (savedValues2.dataToDisplaySet1 & 4096) {
					TransmitString("IdRef,");
				}
				if (savedValues2.dataToDisplaySet1 & 2048) {
					TransmitString("IqRef,");
				}
				if (savedValues2.dataToDisplaySet1 & 1024) {
					TransmitString("Vd,");
				}
				if (savedValues2.dataToDisplaySet1 & 512) {
					TransmitString("Vq,");
				}
				if (savedValues2.dataToDisplaySet1 & 256) {
					TransmitString("Ia,");
				}
				if (savedValues2.dataToDisplaySet1 & 128) {
					TransmitString("Ib,");
				}
				if (savedValues2.dataToDisplaySet1 & 64) {
					TransmitString("Ic,");
				}
				if (savedValues2.dataToDisplaySet1 & 32) {
					TransmitString("Va,");
				}
				if (savedValues2.dataToDisplaySet1 & 16) {
					TransmitString("Vb,");
				}
				if (savedValues2.dataToDisplaySet1 & 8) {
					TransmitString("Vc,");
				}
				if (savedValues2.dataToDisplaySet1 & 4) {
					TransmitString("percentVolts,");
				}
				if (savedValues2.dataToDisplaySet1 & 2) {
					TransmitString("batteryAmps,");
				}
				if (savedValues2.dataToDisplaySet2 & 32768) {
					TransmitString("rawThrottle,");
				}
				if (savedValues2.dataToDisplaySet2 & 16384) {
					TransmitString("throttle,");
				}
				if (savedValues2.dataToDisplaySet2 & 8192) {
					TransmitString("temperaure,");
				}
				if (savedValues2.dataToDisplaySet2 & 4096) {
					TransmitString("slipSpeed,");
				}
				if (savedValues2.dataToDisplaySet2 & 2048) {
					TransmitString("electricalSpeed,");
				}
				if (savedValues2.dataToDisplaySet2 & 1024) {
					TransmitString("mechanicalSpeed,");
				}
				TransmitString("\r\n");
				myDataStream.startTime = counter1k;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "data")) {  // show the datastream one time.
			myDataStream.period = 1;
			myDataStream.showStreamOnce = 1;
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-time")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 32768;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~32768;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-id")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 16384;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~16384;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-iq")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 8192;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~8192;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-idref")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 4096;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~4096;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-iqref")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 2048;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~2048;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-vd")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 1024;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~1024;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-vq")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 512;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~512;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-ia")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 256;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~256;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-ib")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 128;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~128;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-ic")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 64;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~64;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-va")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 32;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~32;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-vb")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 16;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~16;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-vc")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 8;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~8;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-percent-volts")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 4;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~4;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-battery-amps")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet1 |= 2;
			}
			else {
				savedValues2.dataToDisplaySet1 &= ~2;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-raw-throttle")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet2 |= 32768;
			}
			else {
				savedValues2.dataToDisplaySet2 &= ~32768;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-throttle")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet2 |= 16384;
			}
			else {
				savedValues2.dataToDisplaySet2 &= ~16384;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-temperature")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet2 |= 8192;
			}
			else {
				savedValues2.dataToDisplaySet2 &= ~8192;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-slip-speed")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet2 |= 4096;
			}
			else {
				savedValues2.dataToDisplaySet2 &= ~4096;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-electrical-speed")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet2 |= 2048;
			}
			else {
				savedValues2.dataToDisplaySet2 &= ~2048;
			}
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "stream-mechanical-speed")) {  // in milliseconds
			if (myUARTCommand.number == 1) {
				savedValues2.dataToDisplaySet2 |= 1024;
			}
			else {
				savedValues2.dataToDisplaySet2 &= ~1024;
			}
		}
		else if (myUARTCommand.string[0] == 0) {  // A carriage return.
			if (myRotor.testRunning) {  // Stop the rotor test if it was running, and just keep the best value of the rotor time constant that you had found up to this point.
				savedValues2.rotorTimeConstantIndex = myRotor.bestTimeConstantIndex;
				myRotor.testRunning = 0;
				myRotor.testFinished = 1;
				IdRefRef = 0;
				IqRefRef = 0;
			}
			else if (myPI.testRunning) { // Stop the PI test if it was running.
				myPI.testRunning = 0;
				myPI.testFailed = 1;
				myPI.testFinished = 1;
			}
			myDataStream.period = 0;  // Stop the data stream if it was running.
			// if the PI test is running, terminate it.
			// if the rotor test is running, stop and use the current best rotorTimeConstant that has been found so far.
			ShowMenu();
		}
		else if (!strcmp((const char*)&myUARTCommand.string[0], "2")) {
			if (IqRefRef < 1000) {
				IqRefRef += 10;
				if (IqRefRef > 0) {
					IdRefRef = IqRefRef;
				}
				else {
					IdRefRef = -IqRefRef;
				}
			}
		}
		else if (!strcmp((const char*)&myUARTCommand.string[0], "1")) {
			if (IqRefRef > -1000) {
				IqRefRef -= 10;
				if (IqRefRef > 0) {
					IdRefRef = IqRefRef;
				}
				else {
					IdRefRef = -IqRefRef;
				}
			}
		}
		else if (!strcmp((const char*)&myUARTCommand.string[0], "0")) {
	//		faultBits |= 16384;
			IdRefRef = 0;
			IqRefRef = 0;
		}
		else if (!strcmp((const char *)&myUARTCommand.string[0], "?")) {  // show the valid list of commands
			TransmitString("List of valid commands:\r\n");
			TransmitString("kp xxx (range 0-32767)\r\n");
			TransmitString("ki xxx (range 0-32767)\r\n");
			TransmitString("current-sensor-amps-per-volt xxx (range 0-480)\r\n");
			TransmitString("max-regen-position xxx (range 0-1023)\r\n");
			TransmitString("min-regen-position xxx (range 0-1023)\r\n");
			TransmitString("min-throttle-position xxx (range 0-1023)\r\n");
			TransmitString("max-throttle-position xxx (range 0-1023)\r\n");
			TransmitString("fault-throttle-position xxx (range 0-1023)\r\n");
			TransmitString("max-battery-amps xxx (range 0-999)\r\n");
			TransmitString("max-battery-amps-regen xxx (range 0-999)\r\n");
			TransmitString("max-motor-amps xxx (range 0-999)\r\n");
			TransmitString("max-motor-amps-regen xxx (range 0-999)\r\n");
			TransmitString("precharge-time xxx (in tenths of a sec. range 0-9999)\r\n");
			TransmitString("rotor-time-constant xxx (in millisec. range 0-150)\r\n");
			TransmitString("pole-pairs xxx (range 0-999)\r\n");
			TransmitString("max-rpm xxx (range 0-32767)\r\n");
			TransmitString("throttle-type xxx (range 0-1)\r\n");
			TransmitString("encoder-ticks xxx (range 64-5000)\r\n");
			TransmitString("pack-voltage xxx (range 0-999)\r\n");
			TransmitString("pi-ratio xxx (range 50-1000.  pi-ratio = Kp/Ki)\r\n");
			TransmitString("run-pi-test\r\n");
			TransmitString("run-rotor-test\r\n");
			TransmitString("config\r\n");
			TransmitString("data-stream-period xxx (range 0-32767)\r\n");
			TransmitString("data\r\n");
			TransmitString("stream-time xxx (range 0-1)\r\n");
			TransmitString("stream-id xxx (range 0-1)\r\n");
			TransmitString("stream-iq xxx (range 0-1)\r\n");
			TransmitString("stream-idref xxx (range 0-1)\r\n");
			TransmitString("stream-iqref xxx (range 0-1)\r\n");
			TransmitString("stream-vd xxx (range 0-1)\r\n");
			TransmitString("stream-vq xxx (range 0-1)\r\n");
			TransmitString("stream-ia xxx (range 0-1)\r\n");
			TransmitString("stream-ib xxx (range 0-1)\r\n");
			TransmitString("stream-ic xxx (range 0-1)\r\n");
			TransmitString("stream-va xxx (range 0-1)\r\n");
			TransmitString("stream-vb xxx (range 0-1)\r\n");
			TransmitString("stream-vc xxx (range 0-1)\r\n");
			TransmitString("stream-percent-volts xxx (range 0-1)\r\n");
			TransmitString("stream-battery-amps xxx (range 0-1)\r\n");
			TransmitString("stream-raw-throttle xxx (range 0-1)\r\n");
			TransmitString("stream-throttle xxx (range 0-1)\r\n");
			TransmitString("stream-temperature xxx (range 0-1)\r\n");
			TransmitString("stream-slip-speed xxx (range 0-1)\r\n");
			TransmitString("stream-electrical-speed xxx (range 0-1)\r\n");
			TransmitString("stream-mechanical-speed xxx (range 0-1)\r\n");
			TransmitString("<carriage return> (this stops the data stream)\r\n");
		}
		else {
			TransmitString("Invalid command.  Type '?' to see a valid list of commands.\r\n");
		}
	
		myUARTCommand.string[0] = 0; 	// clear the string.
		myUARTCommand.i = 0;
		myUARTCommand.number = 0;
		myUARTCommand.complete = 0;  // You processed that command.  Dump it!  Do this last.  The ISR will only run through if the command is NOT yet complete (in other words, if complete == 0). 
	}
}

void ShowConfig(unsigned int mask) {
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// kp=xxxxx ki=xxxxx\r\n
	if (mask & ((unsigned)1 << 0)) {
		strcpy(showConfigString,"kp=xxxxx ki=xxxxx\r\n");
		u16_to_str(&showConfigString[3], savedValues.Kp, 5);	
		u16_to_str(&showConfigString[12], savedValues.Ki, 5);
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// current-sensor-amps-per-volt=xxxx\r\n;
	if (mask & ((unsigned)1 << 5)) {
		strcpy(showConfigString,"current-sensor-amps-per-volt=xxxx\r\n");
		u16_to_str(&showConfigString[29], savedValues.currentSensorAmpsPerVolt, 4);
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5         6         7         8
	// 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345
	// max-regen-position=xxxx\r\n
	// min-regen-position=xxxx\r\n 
	if (mask & ((unsigned)1 << 1)) {
		strcpy(showConfigString,"max-regen-position=xxxx\r\n");
		u16_to_str(&showConfigString[19], savedValues.maxRegenPosition, 4);
		TransmitString(showConfigString);
		strcpy(showConfigString,"min-regen-position=xxxx\r\n");
		u16_to_str(&showConfigString[19], savedValues.minRegenPosition, 4);
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5         6         7         8
	// 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345
	// min-throttle-position=xxxx\r\n
	// max-throttle-position=xxxx\r\n 
	// fault-throttle-position=xxxx\r\n
	if (mask & ((unsigned)1<<2)) {
		strcpy(showConfigString,"min-throttle-position=xxxx\r\n");
		u16_to_str(&showConfigString[22], savedValues.minThrottlePosition, 4);
		TransmitString(showConfigString);

		strcpy(showConfigString,"max-throttle-position=xxxx\r\n");
		u16_to_str(&showConfigString[22], savedValues.maxThrottlePosition, 4);
		TransmitString(showConfigString);

		strcpy(showConfigString,"fault-throttle-position=xxxx\r\n");
		u16_to_str(&showConfigString[24], savedValues.throttleFaultPosition, 4);
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5         6         7         8
	// 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345
	// max-battery-amps=xxxx amps\r\n
	// max-battery-amps-regen=xxxx amps\r\n
	if (mask & ((unsigned)1<<3)) {
		strcpy(showConfigString,"max-battery-amps=xxxz amps\r\n");
		u16_to_str(&showConfigString[17], savedValues.maxBatteryAmps, 4);
		TransmitString(showConfigString);

		strcpy(showConfigString,"max-battery-amps-regen=xzxx amps\r\n");
		u16_to_str(&showConfigString[23], savedValues.maxBatteryAmpsRegen, 4);
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// max-motor-amps=xxx amps\r\n
	// max-motor-amps-regen=xxx amps\r\n
	if (mask & ((unsigned)1 << 4)) {
		strcpy(showConfigString,"max-motor-amps=xxx amps\r\n");
		u16_to_str(&showConfigString[15], savedValues.maxMotorAmps, 3);
		TransmitString(showConfigString);

		strcpy(showConfigString,"max-motor-amps-regen=xxx amps\r\n");
		u16_to_str(&showConfigString[21], savedValues.maxMotorAmpsRegen, 3);
		TransmitString(showConfigString);
	}

	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// precharge-time=xxxx tenths of a sec\r\n
	if (mask & ((unsigned)1 << 6)) {
		strcpy(showConfigString,"precharge-time=xxxx tenths of a sec\r\n");
		u16_to_str(&showConfigString[15], savedValues.prechargeTime, 4);
		TransmitString(showConfigString);
	}


	// **NOW WE ARE IN SavedValues2**
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// rotor-time-constant=xxx ms\r\n
	//
	if (mask & ((unsigned)1 << 8)) {
		strcpy(showConfigString,"rotor-time-constant=xxx ms\r\n");
		u16_to_str(&showConfigString[20], savedValues2.rotorTimeConstantIndex+5, 3);  // for display purposes, add 5 so it's millisec.
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// pole-pairs=xxx\r\n
	//
	if (mask & ((unsigned)1 << 9)) {
		strcpy(showConfigString,"pole-pairs=xxx\r\n");
		u16_to_str(&showConfigString[11], savedValues2.numberOfPolePairs, 3);  
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// max-rpm=xxxxx rev/min\r\n
	//
	if (mask & ((unsigned)1 << 9)) {
		strcpy(showConfigString,"max-rpm=xxxxx rev/min\r\n");
		u16_to_str(&showConfigString[8], savedValues2.maxRPM, 5);  
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// throttle-type=x\r\n
	//
	if (mask & ((unsigned)1 << 9)) {
		strcpy(showConfigString,"throttle-type=x\r\n");
		u16_to_str(&showConfigString[14], savedValues2.throttleType, 1);  
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// encoder-ticks=xxxx ticks/rev\r\n
	//
	if (mask & ((unsigned)1 << 9)) {
		strcpy(showConfigString,"encoder-ticks=xxxx ticks/rev\r\n");
		u16_to_str(&showConfigString[14], savedValues2.encoderTicks, 4);  
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// stator-resistance=xxxxx mOhm\r\n
	//
	if (mask & ((unsigned)1 << 9)) {
		strcpy(showConfigString,"stator-resistance=xxxxx mOhm\r\n");
		u16_to_str(&showConfigString[18], savedValues2.statorResistance_times1024, 5);  // ADCBUF1 is the raw throttle.
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// stator-inductance=xxxxx mHenry\r\n
	//
	if (mask & ((unsigned)1 << 9)) {
		strcpy(showConfigString,"stator-inductance=xxxxx mHenry\r\n");
		u16_to_str(&showConfigString[18], savedValues2.statorInductance_times1024, 5);  // ADCBUF1 is the raw throttle.
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// sensorless=x (1 means true)\r\n
	//
	if (mask & ((unsigned)1 << 9)) {
		strcpy(showConfigString,"sensorless=x (1 means true)\r\n");
		u16_to_str(&showConfigString[11], savedValues2.sensorless, 1);  // ADCBUF1 is the raw throttle.  +5 because 5mH is in index 0.
		TransmitString(showConfigString);
	}

	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// pack-voltage=xxx volts\r\n
	//
	if (mask & ((unsigned)1 << 9)) {
		strcpy(showConfigString,"pack-voltage=xxx volts\r\n");
		u16_to_str(&showConfigString[13], savedValues2.packVoltage, 3);  // ADCBUF1 is the raw throttle.
		TransmitString(showConfigString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// pi-ratio=xxx\r\n
	//
	if (mask & ((unsigned)1 << 8)) {
		strcpy(showConfigString,"pi-ratio=xxx\r\n");
		u16_to_str(&showConfigString[9], myPI.ratioKpKi, 3);  // ADCBUF1 is the raw throttle.
		TransmitString(showConfigString);
	}

	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// raw-throttle=xxxx\r\n
	//
	if (mask & ((unsigned)1 << 7)) {
		strcpy(showConfigString,"raw-throttle=xxxx\r\n");
		u16_to_str(&showConfigString[13], ADCBUF1, 4);  // ADCBUF1 is the raw throttle.
		TransmitString(showConfigString);
	}
}

// Input is an integer from 0 to 15.  Output is a character in '0', '1', '2', ..., '9', 'a','b','c','d','e','f'
char IntToCharHex(unsigned int i) {
	if (i <= 9) {
		return ((unsigned char)(i + 48));
	}
	else {
		return ((unsigned char)(i + 55));
	}
}

void ShowMenu(void)
{
	TransmitString("AC controller firmware, ver. 1.0\r\n");
}

// convert val to string (inside body of string) with specified number of digits
// do NOT terminate string
void u16_to_str(char *str, unsigned val, unsigned char digits)
{
	str = str + (digits - 1); // go from right to left.
	while (digits > 0) { // 
		*str = (unsigned char)(val % 10) + '0';
		val = val / 10;
		str--;
		digits--;
	}
}

// convert val to string (inside body of string) with specified number of digits (not counting the + or - sign).
// do NOT terminate string
// Ex:  -2345 should have length 4. It will be printed as -2345
// 2345 should also have length 4.  It will be printed as +2345.
// So, the first symbol is either '-' or '+'. 
void int16_to_str(char *str, int val, unsigned char digits)
{	
	if (val < 0) {
		str[0] = '-';
		val = -val;
	}
	else {
		str[0] = '+';
	}
	str = str + digits; // go from right to left.
	while (digits > 0) { // 
		*str = (unsigned char)(val % 10) + '0';
		val = val / 10;
		str--;
		digits--;
	}
}

// convert val to hex string (inside body of string) with specified number of digits
// do NOT terminate string
void u16x_to_str(char *str, unsigned val, unsigned char digits)
{
	unsigned char nibble;
	
	str = str + (digits - 1);
	while (digits > 0) {
		nibble = val & 0x000f;
		if (nibble >= 10) nibble = (nibble - 10) + 'A';
		else nibble = nibble + '0';
		*str = nibble;
		val = val >> 4;
		str--;
		digits--;
	}
}

int TransmitString(const char* str) {  // For echoing onto the display
	unsigned int i = 0;
	unsigned int now = 0;
	
	now = TMR5;	// timer 4 runs at 59KHz.  Timer5 is the high word of the 32 bit timer.  So, it updates about 1 time per second.
	while (1) {
		if (str[i] == 0) {
			return 1;
		}
		if (U2STAbits.UTXBF == 0) { // TransmitReady();
			U2TXREG = str[i]; 	// SendCharacter(str[i]);
			i++;
		}
		if (TMR5 - now > 5) { 	// 5 seconds
			faultBits |= UART_FAULT;
			return 0;
		}
		#ifndef DEBUG 
			ClrWdt();
		#endif
	}
}

void StreamData() {
	static volatile int tenths = 0;
	static volatile int temp;
	//	unsigned int dataToDisplaySet1;
	// 0b0000 0000 0000 0000
	// bit 15 set: display myDataStream.timer
	// Bit 14 set: display myDataStream.Id_times10
	// bit 13 set: display myDataStream.Iq_times10
	// Bit 12 set: display myDataStream.IdRef_times10
	// bit 11 set: display myDataStream.IqRef_times10
	// Bit 10 set: display myDataStream.Vd
	// bit 9 set: display myDataStream.Vq
	// Bit 8 set: display myDataStream.Ia_times10
	// bit 7 set: display myDataStream.Ib_times10
	// bit 6 set: display myDataStream.Ic_times10
	// Bit 5 set: display myDataStream.Va
	// bit 4 set: display myDataStream.Vb
	// bit 3 set: display myDataStream.Vc
	// bit 2 set: display myDataStream.percentOfVoltageDiskBeingUsed
	// bit 1 set: display myDataStream.batteryAmps_times10
	// bit 0 set: future use
	//	unsigned int dataToDisplaySet2;
	// Bit 15 set: display myDataStream.rawThrottle
	// bit 14 set: display myDataStream.throttle
	// Bit 13 set: display myDataStream.temperature
	// bit 12 set: display myDataStream.slipSpeedRPM
	// Bit 11 set: display myDataStream.electricalSpeedRPM
	// bit 10 set: display myDataStream.mechanicalSpeedRPM
	// Bit 9-0 set: future use. 

	if (savedValues2.dataToDisplaySet1 & 32768) {
		u16_to_str((char *)&intString[0], myDataStream.timer, 5); // intString[] = "00345".  Now, add a comma and null terminate it.
		intString[5] = ',';
		intString[6] = 0;	
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 16384) {
		temp = abs(myDataStream.Id_times10);
		tenths = temp % 10;
		myDataStream.Id_times10 /= 10;
		int16_to_str((char *)&intString[0], myDataStream.Id_times10, 3);	 // ex: intString[] = "+087"
		intString[4] = '.';
		intString[5] = (char)(tenths + 48);
		intString[6] = ',';
		intString[7] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 8192) {
		temp = abs(myDataStream.Iq_times10);
		tenths = temp % 10;
		myDataStream.Iq_times10 /= 10;
		int16_to_str((char *)&intString[0], myDataStream.Iq_times10, 3);	 // ex: intString[] = "+087"
		intString[4] = '.';
		intString[5] = (char)(tenths + 48);
		intString[6] = ',';
		intString[7] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 4096) {
		temp = abs(myDataStream.IdRef_times10);
		tenths = temp % 10;
		myDataStream.IdRef_times10 /= 10;
		int16_to_str((char *)&intString[0], myDataStream.IdRef_times10, 3);	 // ex: intString[] = "+087"
		intString[4] = '.';
		intString[5] = (char)(tenths + 48);
		intString[6] = ',';
		intString[7] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 2048) {
		temp = abs(myDataStream.IqRef_times10);
		tenths = temp % 10;
		myDataStream.IqRef_times10 /= 10;
		int16_to_str((char *)&intString[0], myDataStream.IqRef_times10, 3);	 // ex: intString[] = "+087"
		intString[4] = '.';
		intString[5] = (char)(tenths + 48);
		intString[6] = ',';
		intString[7] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 1024) {
		int16_to_str((char *)&intString[0], myDataStream.Vd, 3);	 // ex: intString[] = "+087"
		intString[4] = ','; // it is on 4 rather than 3 because there is a + or - as the first character of the string, since it's an int.
		intString[5] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 512) {
		int16_to_str((char *)&intString[0], myDataStream.Vq, 3);	 // ex: intString[] = "+087"
		intString[4] = ','; // it is on 4 rather than 3 because there is a + or - as the first character of the string, since it's an int.
		intString[5] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 256) {
		temp = abs(myDataStream.Ia_times10);
		tenths = temp % 10;
		myDataStream.Ia_times10 /= 10;
		int16_to_str((char *)&intString[0], myDataStream.Ia_times10, 3);	 // ex: intString[] = "+087"
		intString[4] = '.';
		intString[5] = (char)(tenths + 48);
		intString[6] = ',';
		intString[7] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 128) {
		temp = abs(myDataStream.Ib_times10);
		tenths = temp % 10;
		myDataStream.Ib_times10 /= 10;
		int16_to_str((char *)&intString[0], myDataStream.Ib_times10, 3);	 // ex: intString[] = "+087"
		intString[4] = '.';
		intString[5] = (char)(tenths + 48);
		intString[6] = ',';
		intString[7] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 64) {
		temp = abs(myDataStream.Ic_times10);
		tenths = temp % 10;
		myDataStream.Ic_times10 /= 10;
		int16_to_str((char *)&intString[0], myDataStream.Ic_times10, 3);	 // ex: intString[] = "+087"
		intString[4] = '.';
		intString[5] = (char)(tenths + 48);
		intString[6] = ',';
		intString[7] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 32) {
		int16_to_str((char *)&intString[0], myDataStream.Va, 3);	 // ex: intString[] = "+087"
		intString[4] = ','; // it is on 4 rather than 3 because there is a + or - as the first character of the string, since it's an int.
		intString[5] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 16) {
		int16_to_str((char *)&intString[0], myDataStream.Vb, 3);	 // ex: intString[] = "+087"
		intString[4] = ','; // it is on 4 rather than 3 because there is a + or - as the first character of the string, since it's an int.
		intString[5] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 8) {
		int16_to_str((char *)&intString[0], myDataStream.Vc, 3);	 // ex: intString[] = "+087"
		intString[4] = ','; // it is on 4 rather than 3 because there is a + or - as the first character of the string, since it's an int.
		intString[5] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet1 & 4) {
		u16_to_str((char *)&intString[0], myDataStream.percentOfVoltageDiskBeingUsed, 3); // intString[] = "075".  Now, add a comma and null terminate it.
		intString[3] = ',';
		intString[4] = 0;	
		TransmitString((char *)&intString[0]);

	}
	if (savedValues2.dataToDisplaySet1 & 2) {
		temp = abs(myDataStream.batteryAmps_times10);
		tenths = temp % 10;
		myDataStream.batteryAmps_times10 /= 10;
		int16_to_str((char *)&intString[0], myDataStream.batteryAmps_times10, 3);	 // ex: intString[] = "+087"
		intString[4] = '.';
		intString[5] = (char)(tenths + 48);
		intString[6] = ',';
		intString[7] = 0; // null terminate it.					
		TransmitString((char *)&intString[0]);
	}
	//if (savedValues2.dataToDisplaySet1 & 1) {
	//}
	// Bit 15 set: display myDataStream.rawThrottle
	// bit 14 set: display myDataStream.throttle
	// Bit 13 set: display myDataStream.temperature
	// bit 12 set: display myDataStream.slipSpeedRPM
	// Bit 11 set: display myDataStream.electricalSpeedRPM
	// bit 10 set: display myDataStream.mechanicalSpeedRPM
	// Bit  9-0 set: future use. 

	if (savedValues2.dataToDisplaySet2 & 32768) {
		u16_to_str((char *)&intString[0], myDataStream.rawThrottle, 4); // intString[] = "0345".  Now, add a comma and null terminate it.
		intString[4] = ',';
		intString[5] = 0;	
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet2 & 16384) {
		int16_to_str((char *)&intString[0], myDataStream.throttle, 4); // intString[] = "+0345".  Now, add a comma and null terminate it.
		intString[5] = ',';
		intString[6] = 0;	
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet2 & 8192) {
		u16_to_str((char *)&intString[0], myDataStream.temperature, 3); // intString[] = "38".  Now, add a comma and null terminate it.
		intString[3] = ',';
		intString[4] = 0;	
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet2 & 4096) {
		int16_to_str((char *)&intString[0], myDataStream.slipSpeedRPM, 4); // intString[] = "+0345".  Now, add a comma and null terminate it.
		intString[5] = ',';
		intString[6] = 0;	
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet2 & 2048) {
		int16_to_str((char *)&intString[0], myDataStream.electricalSpeedRPM, 5); // intString[] = "+03457".  Now, add a comma and null terminate it.
		intString[6] = ',';
		intString[7] = 0;	
		TransmitString((char *)&intString[0]);
	}
	if (savedValues2.dataToDisplaySet2 & 1024) {
		int16_to_str((char *)&intString[0], myDataStream.mechanicalSpeedRPM, 5); // intString[] = "+03457".  Now, add a comma and null terminate it.
		intString[6] = ',';
		intString[7] = 0;	
		TransmitString((char *)&intString[0]);
	}
	TransmitString("\r\n");
}
