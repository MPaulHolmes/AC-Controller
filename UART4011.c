#include "UART4011.h"

void ShowMenu(void);
void ShowConfig(unsigned int mask);
void u16x_to_str(char *str, unsigned val, unsigned char digits);
void u16_to_str(char *str, unsigned val, unsigned char digits);
void int16_to_str(char *str, int val, unsigned char digits);
int TransmitString(char* str);
char IntToCharHex(unsigned int i);
void FetchRTData(void);
extern void InitPIStruct(void);
extern void EESaveValues(void);
extern void InitializeThrottleAndCurrentVariables(void);
volatile UARTCommand myUARTCommand = {0,0,{0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0},0};

unsigned int timeOfLastDatastreamTransmission = 0;
extern volatile unsigned int faultBits;
extern SavedValuesStruct savedValues;
extern SavedValuesStruct2 savedValues2;
extern unsigned int counter1ms;
extern unsigned int showDatastreamJustOnce;
extern unsigned int dataStreamPeriod;

char RTDataString[] = "xxxxx xxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxx\r\n";
volatile char newChar = 0;
volatile int echoNewChar = 0;

const char command0[] = "config";
const char command1[] = "save";
const char command2[] = "idle";
const char command3[] = "restart"; // 
const char command4[] = "kp-id"; // proportional gain in PI loop.
const char command5[] = "ki-id";	// integral gain in PI loop.
const char command6[] = "kp-iq";
const char command7[] = "ki-iq";
const char command8[] = "max-motor-amps";
const char command9[] = "current-sensor-amps-per-volt";
// throttle fault < max regen < min regen < min throttle < max throttle.  Wig Wag configuration.
const char command10[] = "max-regen-position";  // The raw A/D value for max regen position.  [max-regen-throttle-point, min-regen-throttle-point] gets mapped to [-(max-regen), 0]
const char command11[] = "min-regen-position";  // The raw A/D value for min regen position.
const char command12[] = "min-throttle-position";  // The raw A/D value for min throttle position.  [min-throttle-point, max-throttle-point] --> [0, max-throttle].
const char command13[] = "max-throttle-position";  // The raw A/D value for max throttle position.  [min-throttle-point, max-throttle-point] --> [0, max-throttle].
const char command14[] = "fault-throttle-position"; // The raw A/D value for throttle fault position.  below this indicates throttle unplugged.
const char command15[] = "rtd-period";
const char command16[] = "rtd";
const char command17[] = "max-battery-amps";  // The largest allowable battery current coming OUT of the batteries.
const char command18[] = "max-battery-amps-regen"; // The largest allowable battery current going INTO the batteries.jop
const char command19[] = "precharge-time";
const char command20[] = "rotor-time-constant-index";
const char command21[] = "pole-pairs";
const char command22[] = "max-rpm";
const char command23[] = "throttle-type";
char savedEEString[] = "Configuration written to EE\r\n";
char menuString[] = "AC controller firmware, ver. 1.0\r\n";

char showConfigKpKiString[] = "kp-id=xxxxx ki-id=xxxxx kp-iq=xxxxx ki-iq=xxxxx\r\n";
char showConfigRegenString[] = "min-regen-position=xxx max-regen-position=xxx\r\n";
char showConfigThrottleString[] = "min-throttle-position=xxx max-throttle-position=xxx fault-throttle-position=xxx\r\n";
char showConfigBatteryAmpsString[] = "max-battery-amps=xxx max-battery-amps-regen=xxx\r\n";  
char showConfigMotorAmpsString[] = "max-motor-amps=xxx\r\n";
char showConfigCurrentSensorString[] = "current-sensor-amps-per-volt=xxxx\r\n";
char showConfigPrechargeString[] = "precharge-time=xxxx\r\n";

void InitUART2() {
	U2BRG = 7; //115200bps=7 38.4kbps==23   47; // Pg. 506 on F.R.M.  Baud rate is 19.2kbps. 14.74MHz
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
	newChar = U2RXREG;		// get the character that caused the interrupt.
	echoNewChar = 1;

	if (myUARTCommand.complete == 1) {	// just ignore everything until the command is processed.
		return;
	}
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
	myUARTCommand.number = 0;	// set number argument to zero.
	for (i = 0; myUARTCommand.string[i] != 0; i++) {
		if (myUARTCommand.string[i] == ' ') {
			myUARTCommand.number = atoi((char *)&myUARTCommand.string[i+1]);
			myUARTCommand.string[i] = 0;  // null terminate the text portion.			
			break;
		}
	}
	// Let's say you typed the command kp 1035.  The following would have happened:
	// myUARTCommand.string[] would contain only the text portion of the command, and is terminated with a 0.  string[] = {'k','p',0,?,?,?,?,?,?,?,?,?,?,?,...}
	// Also, myUARTCommand.number = the number argument after the command. So, number = 1035.
	if (myUARTCommand.string[0] == 0) {	    // it must have been a carriage return only.
		dataStreamPeriod = 0;       // If only a carriage return, stop the datastream if it was going.  Also, show the menu.
		TransmitString(menuString); // "AC controller firmware, ver. 0.0.  I don't ask for much. So, please don't blow up.\r\n"
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command0)) { // "config"
		ShowConfig(0x0FFFF);
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command1)){ // "save"
		TransmitString(savedEEString);  // 	"configuration written to EE"	
		EESaveValues();
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command2)){ // "idle". 
  	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command3)){ // "restart"
		while(1);	// this starts the program over, due to the watchdog.
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command4)){ // "kp-id"
		if (myUARTCommand.number <= 32767u && myUARTCommand.number > 0) {
			savedValues.Kp_Id = (int)(myUARTCommand.number); 
			InitPIStruct();
//			ShowConfig((unsigned)1 << 0);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command5)){ // "ki-id"
		if (myUARTCommand.number <= 32767u && myUARTCommand.number > 0) {
			savedValues.Ki_Id = (int)(myUARTCommand.number); 
			InitPIStruct();
//			ShowConfig((unsigned)1 << 0);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command6)){ // "kp-iq"
		if (myUARTCommand.number <= 32767u && myUARTCommand.number > 0) {
			savedValues.Kp_Iq = (int)(myUARTCommand.number); 
			InitPIStruct();
//			ShowConfig((unsigned)1 << 0);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command7)){ // "ki-iq"
		if (myUARTCommand.number <= 32767u && myUARTCommand.number > 0) {
			savedValues.Ki_Iq = (int)(myUARTCommand.number); 
			InitPIStruct();
//			ShowConfig((unsigned)1 << 0);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command8)){ // "max-motor-amps".  This is the peak line to line amps.  So, the RMS current would be maxMotorAmps / sqrt(2).  So, if your motor is rated for 100AMPRms, you could set this to 141amps.
		if (myUARTCommand.number <= MAX_MOTOR_AMPS && myUARTCommand.number > 0) {  		//
			savedValues.maxMotorAmps = myUARTCommand.number;
			if (!(savedValues.currentSensorAmpsPerVolt > 0 && savedValues.currentSensorAmpsPerVolt <= MAX_CURRENT_SENSOR_AMPS_PER_VOLT)) {
				savedValues.currentSensorAmpsPerVolt = DEFAULT_CURRENT_SENSOR_AMPS_PER_VOLT;
			}
			if (savedValues.maxMotorAmpsRegen > savedValues.maxMotorAmps) {
				savedValues.maxMotorAmpsRegen = savedValues.maxMotorAmps;
			}
			InitializeThrottleAndCurrentVariables();  // some things changed, so reinitialize it all.  Later, figure out what parts don't need re-initializing to speed things up.
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command9)){ // "current-sensor-amps-per-volt".  It's assumed that zero amps corresponds to 2.5volts on the current sensor.
		if (myUARTCommand.number > 0 && myUARTCommand.number <= MAX_CURRENT_SENSOR_AMPS_PER_VOLT) {  // LEM Hass 300-s has an amps/volt of 480.  The LEM Hass 600-s has an amps/volt of 960.  LEM Hass 50-s has an amps/volt of 80.
			savedValues.currentSensorAmpsPerVolt = myUARTCommand.number;
		}
		else {
			savedValues.currentSensorAmpsPerVolt = DEFAULT_CURRENT_SENSOR_AMPS_PER_VOLT;
		}
		InitializeThrottleAndCurrentVariables();  // You changed the current sensor type, so reinitialize it all.  Later, figure out what parts don't need re-initializing to speed things up.
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command10)){ // "max-regen-position"
		if (myUARTCommand.number <= 999u) {		
			savedValues.maxRegenPosition = myUARTCommand.number;
			if (savedValues2.throttleType == ZERO_TO_5K_POT) {
				savedValues.maxRegenPosition = 1023 - savedValues.maxRegenPosition;  // invert it.
			}
			if (((savedValues.throttleFaultPosition < savedValues.maxRegenPosition) && (savedValues.maxRegenPosition < savedValues.minRegenPosition) && 
				(savedValues.minRegenPosition < savedValues.minThrottlePosition) && (savedValues.minThrottlePosition < savedValues.maxThrottlePosition)) == 0) {
				faultBits |= CONFIG_FAULT;
			}
			else {
				faultBits &= (~CONFIG_FAULT);
			}
//			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command11)){ // "min-regen-position"
		if (myUARTCommand.number <= 999u) {
			savedValues.minRegenPosition = myUARTCommand.number;
			if (savedValues2.throttleType == ZERO_TO_5K_POT) {
				savedValues.minRegenPosition = 1023 - savedValues.minRegenPosition;  // invert it.
			}
			if (((savedValues.throttleFaultPosition < savedValues.maxRegenPosition) && (savedValues.maxRegenPosition < savedValues.minRegenPosition) && 
				(savedValues.minRegenPosition < savedValues.minThrottlePosition) && (savedValues.minThrottlePosition < savedValues.maxThrottlePosition)) == 0) {
				faultBits |= CONFIG_FAULT;
			}
			else {
				faultBits &= (~CONFIG_FAULT);
			}
//			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command12)){ // "min-throttle-position"
		if (myUARTCommand.number <= 999u) {
			savedValues.minThrottlePosition = myUARTCommand.number;
			if (savedValues2.throttleType == ZERO_TO_5K_POT) {
				savedValues.minThrottlePosition = 1023 - savedValues.minThrottlePosition;  // invert it.
			}

			if (((savedValues.throttleFaultPosition < savedValues.maxRegenPosition) && (savedValues.maxRegenPosition < savedValues.minRegenPosition) && 
				(savedValues.minRegenPosition < savedValues.minThrottlePosition) && (savedValues.minThrottlePosition < savedValues.maxThrottlePosition)) == 0) {
				faultBits |= CONFIG_FAULT;
			}
			else {
				faultBits &= (~CONFIG_FAULT);
			}
//			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command13)){ // "max-throttle-position"
		if (myUARTCommand.number <= 999u) {
			savedValues.maxThrottlePosition = myUARTCommand.number;
			if (savedValues2.throttleType == ZERO_TO_5K_POT) {
				savedValues.maxThrottlePosition = 1023 - savedValues.maxThrottlePosition;  // invert it.
			}
			if (((savedValues.throttleFaultPosition < savedValues.maxRegenPosition) && (savedValues.maxRegenPosition < savedValues.minRegenPosition) && 
				(savedValues.minRegenPosition < savedValues.minThrottlePosition) && (savedValues.minThrottlePosition < savedValues.maxThrottlePosition)) == 0) {
				faultBits |= CONFIG_FAULT;
			}
			else {
				faultBits &= (~CONFIG_FAULT);
			}
//			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command14)){ // "fault-throttle-position"
		if (myUARTCommand.number <= 999u) {
			savedValues.throttleFaultPosition = myUARTCommand.number;
			if (((savedValues.throttleFaultPosition < savedValues.maxRegenPosition) && (savedValues.maxRegenPosition < savedValues.minRegenPosition) && 
				(savedValues.minRegenPosition < savedValues.minThrottlePosition) && (savedValues.minThrottlePosition < savedValues.maxThrottlePosition)) == 0) {
				faultBits |= CONFIG_FAULT;
			}		
			else {
				faultBits &= (~CONFIG_FAULT);
			}
//			ShowConfig((unsigned)1 << 1);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command15)){ // "rtd-period".  Delay in milliseconds.
		if (myUARTCommand.number <= 60000u) { // 60 seconds between updates is the slowest you can stream the data.
			dataStreamPeriod = myUARTCommand.number;
			showDatastreamJustOnce = 0;
			timeOfLastDatastreamTransmission = counter1ms;
//			ShowConfig((unsigned)1 << 2);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command16)){ // "rtd"
		dataStreamPeriod = 1u;  // just make it nonzero.
		showDatastreamJustOnce = 1;
		timeOfLastDatastreamTransmission = counter1ms;
//		ShowConfig((unsigned)1 << 6);
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command17)){ // "max-battery-amps"
		if (myUARTCommand.number <= MAX_BATTERY_AMPS) {  		// Don't need to test if >= 0, since 'number' is an unsigned int.
			savedValues.maxBatteryAmps = myUARTCommand.number;
			InitializeThrottleAndCurrentVariables();  // You changed maxBatteryAmps, so reinitialize it all.  Later, figure out what parts don't need re-initializing to speed things up.
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command18)){ // "max-battery-amps-regen"
		if (myUARTCommand.number <= MAX_BATTERY_AMPS_REGEN) { 
			savedValues.maxBatteryAmpsRegen = myUARTCommand.number;
			InitializeThrottleAndCurrentVariables();  // You changed maxBatteryAmps, so reinitialize it all.  Later, figure out what parts don't need re-initializing to speed things up.
//			ShowConfig((unsigned)1 << 10);
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command19)){ // "precharge-time" 
		if (myUARTCommand.number <= 9999u) {
			savedValues.prechargeTime = myUARTCommand.number;
//			ShowConfig((unsigned)1 << 11);
		}
	}

	else if (!strcmp((char *)&myUARTCommand.string[0], command20)){ // "rotor-time-constant-index" 
		if (myUARTCommand.number <= MAX_ROTOR_TIME_CONSTANT_INDEX) {
			savedValues2.rotorTimeConstantIndex = myUARTCommand.number;
//			ShowConfig((unsigned)1 << 11);
		}
	}

	else if (!strcmp((char *)&myUARTCommand.string[0], command21)){ // "pole-pairs"
		savedValues2.numberOfPolePairs = myUARTCommand.number;
		if (__builtin_mulss(savedValues2.maxRPM, savedValues2.numberOfPolePairs) > 60000L) {
			faultBits |= CONFIG_FAULT;
		}
		else {
			faultBits &= ~CONFIG_FAULT;
		}
	}

	else if (!strcmp((char *)&myUARTCommand.string[0], command22)){ // "max-rpm"
		savedValues2.maxRPM = myUARTCommand.number;
		if (__builtin_mulss(savedValues2.maxRPM, savedValues2.numberOfPolePairs) > 60000L) {  // That would be 15000 rpm for a 3600rpm motor (1 pole pair), 7500rpm for a motor rated for 1800rpm, etc...
			faultBits |= CONFIG_FAULT;
		}
		else {
			faultBits &= ~CONFIG_FAULT;
		}
	}
	else if (!strcmp((char *)&myUARTCommand.string[0], command23)){ // "throttle-type"
		if (myUARTCommand.number <= 1) {
			savedValues2.throttleType = myUARTCommand.number;
		}
	}

	myUARTCommand.string[0] = 0; 	// clear the string.
	myUARTCommand.i = 0;
	myUARTCommand.number = 0;
	myUARTCommand.complete = 0;  // You processed that command.  Dump it!  Do this last.  The ISR will only run through if the command is NOT yet complete (in other words, if complete == 0). 
	}
}

int TransmitString(char* str) {  // For echoing onto the display
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
//			faultBits |= UART_FAULT;
			return 0;
		}
		#ifndef DEBUG 
			ClrWdt();
		#endif
	}
}

void ShowMenu(void)
{
	TransmitString(menuString);
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

void ShowConfig(unsigned int mask) {
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// kp-id=xxxxx ki-id=xxxxx kp-iq=xxxxx ki-iq=xxxxx\r\n
	if (mask & ((unsigned)1 << 0)) {
		u16_to_str(&showConfigKpKiString[6], savedValues.Kp_Id, 5);	
		u16_to_str(&showConfigKpKiString[18], savedValues.Ki_Id, 5);
		u16_to_str(&showConfigKpKiString[30], savedValues.Kp_Iq, 5);	
		u16_to_str(&showConfigKpKiString[42], savedValues.Ki_Iq, 5);
		TransmitString(showConfigKpKiString);
	}
	// 0         1         2         3         4         5         6         7         8
	// 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345
	// min-regen-position=xxx max-regen-position=xxx\r\n
	if (mask & ((unsigned)1 << 1)) {
		u16_to_str(&showConfigRegenString[19], savedValues.minRegenPosition, 3);
		u16_to_str(&showConfigRegenString[42], savedValues.maxRegenPosition, 3);
		TransmitString(showConfigRegenString);
	}
	// 0         1         2         3         4         5         6         7         8
	// 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345
	// min-throttle-position=xxx max-throttle-position=xxx fault-throttle-position=xxx\r\n
	if (mask & ((unsigned)1<<2)) {
		u16_to_str(&showConfigThrottleString[22], savedValues.minThrottlePosition, 3);
		u16_to_str(&showConfigThrottleString[48], savedValues.maxThrottlePosition, 3);
		u16_to_str(&showConfigThrottleString[76], savedValues.throttleFaultPosition, 3);
		TransmitString(showConfigThrottleString);
	}
	// 0         1         2         3         4         5         6         7         8
	// 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345
	// max-battery-amps=xxx max-battery-amps-regen=xxx\r\n
	if (mask & ((unsigned)1<<3)) {
		u16_to_str(&showConfigBatteryAmpsString[17], savedValues.maxBatteryAmps, 3);
		u16_to_str(&showConfigBatteryAmpsString[44], savedValues.maxBatteryAmpsRegen, 3);
		TransmitString(showConfigBatteryAmpsString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// max-motor-amps=xxx\r\n
	if (mask & ((unsigned)1 << 4)) {
		u16_to_str(&showConfigMotorAmpsString[15], savedValues.maxMotorAmps, 3);
		TransmitString(showConfigMotorAmpsString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// current-sensor-amps-per-volt=xxxx\r\n;
	if (mask & ((unsigned)1 << 5)) {
		u16_to_str(&showConfigCurrentSensorString[29], savedValues.currentSensorAmpsPerVolt, 4);
		TransmitString(showConfigCurrentSensorString);
	}
	// 0         1         2         3         4         5
	// 012345678901234567890123456789012345678901234567890123456789
	// precharge-time=xxxx
	if (mask & ((unsigned)1 << 6)) {
		u16_to_str(&showConfigPrechargeString[15], savedValues.prechargeTime, 4);
		TransmitString(showConfigPrechargeString);
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

