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

//extern volatile unsigned int LrLmSquared_times128;
//extern volatile unsigned int LrLmSquared_times128Array[];

extern volatile int largeArrayLoaded;
extern volatile int captureVariable;
extern volatile int dataCounter;
extern volatile int IqRefRef;
extern volatile int IdRefRef;

//extern volatile int dataDumping;
extern volatile int maxRPS_times16;
extern volatile unsigned int faultBits;
extern volatile SavedValuesStruct savedValues;
extern volatile SavedValuesStruct2 savedValues2;
extern volatile unsigned int showDatastreamJustOnce;
extern volatile unsigned int datastreamPeriod;
extern unsigned int revCounterMax;
extern volatile piType pi_Iq;
extern volatile piType pi_Id;
extern volatile int piRatio;

extern volatile int piZeroCrossingIndex;
extern volatile int piGoodValuesIndex;
extern volatile int piGoodValuesArrayLoaded;
extern volatile int huntingForGoodPIValues;
extern volatile int piIterationIndex;
extern volatile int startNewRotorTest;
extern volatile int rotorArrayLoaded;
extern volatile int rotorStartTime;
	
extern unsigned int delayBetweenPITests;
extern unsigned int counter10k;


volatile char newChar = 0;
volatile int echoNewChar = 0;

volatile char string[] = "xxxxxx\r\n";  // just a temp variable to store an integer in the range +/-9999 as text before being transmitted.

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
	// Let's say you typed the command kp-id 1035.  The following would have happened:
	// myUARTCommand.string[] would contain only the text portion of the command, and is terminated with a 0.  string[] = {'k','p',0,?,?,?,?,?,?,?,?,?,?,?,...}
	// Also, myUARTCommand.number = the number argument after the command. So, number = 1035.
	if (!strcmp((char *)&myUARTCommand.string[0], "p")) {
		if (myUARTCommand.number <= 32767u && myUARTCommand.number > 0) {
			savedValues.Kp_Id = (int)(myUARTCommand.number); 
			savedValues.Kp_Iq = (int)(myUARTCommand.number); 

			InitPIStruct();
//			ShowConfig((unsigned)1 << 0);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], "i")){ 
		if (myUARTCommand.number <= 32767u && myUARTCommand.number > 0) {
			savedValues.Ki_Id = (int)(myUARTCommand.number); 
			savedValues.Ki_Iq = (int)(myUARTCommand.number); 
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
			savedValues2.rotorTimeConstantArrayIndex = (int)(myUARTCommand.number-5);   
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
	else if (!strcmp((const char *)&myUARTCommand.string[0], "throttle-type")) { 
		if (myUARTCommand.number <= 2 && myUARTCommand.number >= 1) {
			savedValues2.throttleType = (int)(myUARTCommand.number); 
		}
	}
	else if (!strcmp((const char *)&myUARTCommand.string[0], "encoder-ticks")) {
		if (myUARTCommand.number <= 5000u && myUARTCommand.number >= 64) {
			savedValues2.encoderTicks = (int)(myUARTCommand.number); 
			revCounterMax = 160000L / (4*savedValues2.encoderTicks);  // 4* because I'm doing 4 times resolution for the encoder. 16,000,000 because revolutions per 16 seconds is computed as:  16*10,000*poscnt * rev/(maxPosCnt*revcounter*(16sec)
		}
	}
	else if (!strcmp((const char *)&myUARTCommand.string[0], "sr")) {  // times1024.  So basically mOhms.
		if (myUARTCommand.number > 0) {
			savedValues2.statorResistance_times1024 = (int)(myUARTCommand.number);
		}
	}
	else if (!strcmp((const char *)&myUARTCommand.string[0], "s")) {  // times1024.  So basically mHenries.
		if (myUARTCommand.number > 0) {
			savedValues2.statorInductance_times1024 = (int)(myUARTCommand.number);
		}
	}
	else if (!strcmp((const char *)&myUARTCommand.string[0], "pack-voltage")) {  // units are volts.
		if (myUARTCommand.number < 1000 && myUARTCommand.number > 0) {
			savedValues2.packVoltage = (int)(myUARTCommand.number); 
		}
	}
	else if (!strcmp((const char *)&myUARTCommand.string[0], "pi-ratio")) {
		if (myUARTCommand.number < 1000 && myUARTCommand.number >= 50) {
			piRatio = (int)(myUARTCommand.number); 
		}
	}
//	else if (!strcmp((const char *)&myUARTCommand.string[0], "r")) {
//		if (myUARTCommand.number <= MAX_ROTOR_INDUCTANCE && myUARTCommand.number >= MIN_ROTOR_INDUCTANCE) {
//			savedValues2.rotorInductance_times1024 = (int)(myUARTCommand.number);
			//LrLmSquared_times128 = LrLmSquared_times128Array[savedValues2.rotorInductance_times1024 - MIN_ROTOR_INDUCTANCE]; 
//		}
//	}
	else if (!strcmp((const char *)&myUARTCommand.string[0], "sensorless")) {
		if (myUARTCommand.number == 1) {
			savedValues2.sensorless = 1;
		}
		else {
			savedValues2.sensorless = 0;
		}
	}
	else if (!strcmp((const char *)&myUARTCommand.string[0], "run-pi-test")) {
		startNewRotorTest = 0;
		rotorArrayLoaded = 0;

		huntingForGoodPIValues = 1;
		piIterationIndex = 0;	
		piGoodValuesArrayLoaded = 0;
		piGoodValuesIndex = 0;
		piZeroCrossingIndex = -1;
		delayBetweenPITests = counter10k;
		pi_Iq.P = piRatio;
		pi_Iq.I = 1;
		pi_Id.P = piRatio;
		pi_Id.I = 1;
	}

	else if (!strcmp((const char *)&myUARTCommand.string[0], "run-rotor-test")) {
		huntingForGoodPIValues = 0;
		piGoodValuesArrayLoaded = 0;
		startNewRotorTest = 1;
		savedValues2.rotorTimeConstantArrayIndex = 0;	
		rotorArrayLoaded = 0;
		rotorStartTime = counter10k;
	}
	else if ((!strcmp((const char *)&myUARTCommand.string[0], "config")) || (!strcmp((const char *)&myUARTCommand.string[0], "settings"))) { // "encoder-ticks"
		ShowConfig(0x0FFFF);
	}
	else if (myUARTCommand.string[0] == 0) {
		ShowMenu();
	}
	else if (!strcmp((const char *)&myUARTCommand.string[0], "a")) { //run-inductance-test")) {
		captureVariable = 1;
//		if (dataDumping) {
//			dataDumping = 0;
//			TransmitString(
//		}
//		else dataDumping = 1;
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

	else {
		TransmitString("Bad command or file name.  PC load letter.  That should fix it.\r\n");
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
	// p=xxxxx i=xxxxx\r\n
	if (mask & ((unsigned)1 << 0)) {
		strcpy(showConfigString,"p=xxxxx i=xxxxx\r\n");
		u16_to_str(&showConfigString[2], savedValues.Kp_Iq, 5);	
		u16_to_str(&showConfigString[10], savedValues.Ki_Iq, 5);
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
		u16_to_str(&showConfigString[20], savedValues2.rotorTimeConstantArrayIndex+5, 3);  // for display purposes, add 5 so it's millisec.
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
		u16_to_str(&showConfigString[9], piRatio, 3);  // ADCBUF1 is the raw throttle.
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
