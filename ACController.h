#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "p30F4011.h"
#include "UART.h"
#include <libpic30.h>

#define PAULS_MOTOR
//#define AC_INDUCTION_MOTOR_CONFIG

#define I_TRIS_THROTTLE 		_TRISB0
#define I_TRIS_CURRENT1			_TRISB1
#define I_TRIS_CURRENT2			_TRISB2
#define I_TRIS_INDEX			_TRISB3
#define I_TRIS_QEA				_TRISB4
#define I_TRIS_QEB				_TRISB5
#define O_TRIS_CLEAR_FLIP_FLOP	_TRISB6
#define I_TRIS_TEMPERATURE		_TRISB7
#define I_TRIS_REGEN_THROTTLE	_TRISB8
#define I_TRIS_DESAT_FAULT		_TRISC13
#define I_TRIS_OVERCURRENT_FAULT 	_TRISC14
#define I_TRIS_UNDERVOLTAGE_FAULT	_TRISE8
#define O_TRIS_LED  			_TRISD1 // high means turn ON the LED.
#define O_TRIS_PRECHARGE_RELAY	_TRISD3	// HIGH means turn ON the precharge relay.
#define I_TRIS_GLOBAL_FAULT		_TRISD2
#define O_TRIS_CONTACTOR		_TRISD0
#define O_TRIS_CLEAR_DESAT		_TRISF6
#define O_TRIS_PWM_3H			_TRISE5
#define O_TRIS_PWM_3L			_TRISE4
#define O_TRIS_PWM_2H			_TRISE3
#define O_TRIS_PWM_2L			_TRISE2
#define O_TRIS_PWM_1H			_TRISE1
#define O_TRIS_PWM_1L			_TRISE0

#define I_LAT_THROTTLE 			_LATB0
#define I_LAT_CURRENT1			_LATB1
#define I_LAT_CURRENT2			_LATB2
#define I_LAT_INDEX				_LATB3
#define I_LAT_QEA				_LATB4
#define I_LAT_QEB				_LATB5
#define O_LAT_CLEAR_FLIP_FLOP	_LATB6
#define I_LAT_TEMPERATURE		_LATB7
#define I_LAT_REGEN_THROTTLE	_LATB8
#define I_LAT_DESAT_FAULT		_LATC13
#define I_LAT_OVERCURRENT_FAULT _LATC14
#define I_LAT_UNDERVOLTAGE_FAULT	_LATE8
#define O_LAT_LED  				_LATD1 // high means turn ON the LED.
#define O_LAT_PRECHARGE_RELAY	_LATD3	// HIGH means turn ON the precharge relay.
#define I_LAT_GLOBAL_FAULT		_LATD2
#define O_LAT_CONTACTOR			_LATD0
#define O_LAT_CLEAR_DESAT		_LATF6
#define O_LAT_PWM_3H			_LATE5
#define O_LAT_PWM_3L			_LATE4
#define O_LAT_PWM_2H			_LATE3
#define O_LAT_PWM_2L			_LATE2
#define O_LAT_PWM_1H			_LATE1
#define O_LAT_PWM_1L			_LATE0

#define I_PORT_THROTTLE 		_RB0
#define I_PORT_CURRENT1			_RB1
#define I_PORT_CURRENT2			_RB2
#define I_PORT_INDEX			_RB3
#define I_PORT_QEA				_RB4
#define I_PORT_QEB				_RB5
#define O_PORT_CLEAR_FLIP_FLOP	_RB6
#define I_PORT_TEMPERATURE		_RB7
#define I_PORT_REGEN_THROTTLE	_RB8
#define I_PORT_DESAT_FAULT		_RC13
#define I_PORT_OVERCURRENT_FAULT _RC14
#define I_PORT_UNDERVOLTAGE_FAULT	_RE8
#define O_PORT_LED  			_RD1 // high means turn ON the LED.
#define O_PORT_PRECHARGE_RELAY	_RD3	// HIGH means turn ON the precharge relay.
#define I_PORT_GLOBAL_FAULT		_RD2
#define O_PORT_CONTACTOR		_RD0
#define O_PORT_CLEAR_DESAT		_RF6
#define O_PORT_PWM_3H			_RE5
#define O_PORT_PWM_3L			_RE4
#define O_PORT_PWM_2H			_RE3
#define O_PORT_PWM_2L			_RE2
#define O_PORT_PWM_1H			_RE1
#define O_PORT_PWM_1L			_RE0

#define THROTTLE_RAMP_RATE 1

#define THROTTLE_FAULT (1u << 0)
#define DESAT_FAULT (1u << 1)
#define UART_FAULT (1u << 2)
#define UNDERVOLTAGE_FAULT (1u << 3)
#define OVERCURRENT_FAULT (1u << 4)

#define VREF_FAULT (1u << 5)
#define STARTUP_FAULT (1u << 6)
#define ROTOR_FLUX_ANGLE_FAULT (1u << 7)
#define GLOBAL_FAULT (1u << 8)
#define PI_OVERFLOW_FAULT (1u << 9)
#define PDC_FAULT (1u << 10)
#define OVERSPEED_FAULT (1u << 11)
#define MC_OVERFLOW_FAULT (1u << 12)
#define ENCODER_CABLE_UNPLUGGED_FAULT (1u << 13)
#define CONFIG_FAULT (1u << 14)
#define LINE_TO_LINE_CURRENT_FAULT (1u << 15)
#define PI_TEST_FAULT (1u << 14)

#define DEFAULT_THROTTLE_REGEN_START 344
#define DEFAULT_THROTTLE_REGEN_END 103
#define DEFAULT_THROTTLE_START 474
#define DEFAULT_THROTTLE_END 921
#define ZERO_TO_5K_POT 1
#define THERMAL_CUTBACK_START 670	// 75degC
#define THERMAL_CUTBACK_END 726	// 85degC
// max temperature is 726, which is 85degC.

#define R_MAX 1690u	// 1702 * 2/sqrt(3) * 1.5 = MAX_DUTY-2.  Max duty is 2948. due to scaling of inverse clarke. 2/sqrt(3) I forgot now.  haha.
#define R_MAX_TIMES_65536 (1690UL << 16)
#define MAX_VD_VQ 15000L
#define MAX_MAGNETIZING_CURRENT 4096
#define MAX_SLIP_SPEED_RPS_TIMES16 6400
#define MAX_LONG_INT 2147483647L
#define MAX_DUTY 2948 // 

#define MAX_CURRENT_SENSOR_AMPS_PER_VOLT 480  // LEM Hass 300-s is 480.  Hass 50-s is 80.  Hass 600-s is 960.
#define MAX_ROTOR_TIME_CONSTANT_INDEX 145u // valid indices are 0 to 145 inclusive.
#define AC_INDUCTION_MOTOR 1
#define PERMANENT_MAGNET_AC_MOTOR_WITH_ENCODER 2
#define PERMANENT_MAGNET_AC_MOTOR_WITH_RESOLVER 3

#define MAX_CURRENT_ANGLE_OFFSET 511  		// the electrical angle is between 0 and 511 "degrees".  The offset between the index pulse and the magnetic north must be <= 511 electrical degrees.  THIS HAS NOTHING TO DO WITH ENCODER RESOLUTION!!!

#ifdef PAULS_MOTOR
	#define DEFAULT_ANGLE_OFFSET 119
	#define DEFAULT_CURRENT_SENSOR_AMPS_PER_VOLT 80 // default is the LEM Hass 50-s with 1 pass through.
	#define MAX_MOTOR_AMPS 50
	#define MAX_BATTERY_AMPS_REGEN 10
	#define MAX_BATTERY_AMPS 50
	#ifdef AC_INDUCTION_MOTOR_CONFIG
		#define DEFAULT_MOTOR_TYPE AC_INDUCTION_MOTOR
		#define DEFAULT_ENCODER_TICKS 512
		#define DEFAULT_KP 14000
		#define DEFAULT_KI 226
		#define DEFAULT_NUM_POLE_PAIRS 2
	#else
		#define DEFAULT_MOTOR_TYPE PERMANENT_MAGNET_AC_MOTOR_WITH_RESOLVER
		#define DEFAULT_ENCODER_TICKS 256
		#define DEFAULT_KP 5000//9486
		#define DEFAULT_KI 75//153//
		#define DEFAULT_NUM_POLE_PAIRS 4
	#endif

	#define DEFAULT_PRECHARGE_TIME 5
	#define DEFAULT_ROTOR_TIME_CONSTANT_INDEX 29
	#define DEFAULT_MAX_RPS_TIMES16 1600
	#define DEFAULT_MAX_MECHANICAL_RPM 6000
	#define DEFAULT_THROTTLE_TYPE 0 // 0 means either hall effect or MAX Ohms to 0 Ohm for zero throttle to max throttle.
	#define DEFAULT_DATA_TO_DISPLAY_SET1 0b0000000000000000
	#define DEFAULT_DATA_TO_DISPLAY_SET2 0b0000000000000000
#else 
	#define DEFAULT_CURRENT_SENSOR_AMPS_PER_VOLT 480//27 //480 //16 // default is the LEM Hass 50-s with 5 wraps at the moment.
	#define MAX_MOTOR_AMPS 200
	#define MAX_BATTERY_AMPS_REGEN 200
	#define MAX_BATTERY_AMPS 200
	#define DEFAULT_ENCODER_TICKS 64
	#define DEFAULT_KP 4000
	#define DEFAULT_KI 64
	#define DEFAULT_PRECHARGE_TIME 50
	#define DEFAULT_ROTOR_TIME_CONSTANT_INDEX 13
	#define DEFAULT_MAX_RPS_TIMES16 1600
	#define DEFAULT_NUM_POLE_PAIRS 2
	#define DEFAULT_MAX_MECHANICAL_RPM 6000
	#define DEFAULT_THROTTLE_TYPE 0 // 0 means either hall effect or MAX Ohms to 0 Ohm for zero throttle to max throttle.
	#define DEFAULT_DATA_TO_DISPLAY_SET1 0b0000000000000000
	#define DEFAULT_DATA_TO_DISPLAY_SET2 0b0000000000000000
#endif

#define ROTOR_TIME_CONSTANT_ARRAY_SIZE 50

typedef struct {
	int motorType;					// 0.  0 means AC induction motor.  1 means permanent magnet AC.
	int Kp;							// 1.  PI loop proportional gain
	int Ki;							// 2.  PI loop integreal gain
	int currentSensorAmpsPerVolt;	// 3.  				
	int maxRegenPosition;			// 4.  
	int minRegenPosition;			// 5.  
	int minThrottlePosition;		// 6.  
	int maxThrottlePosition;		// 7.  
	int throttleFaultPosition;		// 8.  
	int maxBatteryAmps;				// 9.  
	int maxBatteryAmpsRegen;		// 10. 
	int maxMotorAmps;				// 11. 
	int maxMotorAmpsRegen;			// 12. 
	int prechargeTime;				// 13. precharge time in 0.1 second increments
	int spares[1];					// 14. spare
	unsigned crc;					// 15. crc
} SavedValuesStruct;

typedef struct {
	unsigned int angleOffset;	// the offset between the permanent magnet's "north" and the angle that the encoder index pulse happens.  in [0, 511].
	int rotorTimeConstantIndex;  	// 0. 31 and 32 is the best for my motor. 0 corresponds to rotor time constant of 0.005 seconds.  145 corresponds to 0.150 seconds.
	int numberOfPolePairs;			// number of pole pairs.  If the nameplate says around 1700RPM on a 60Hz 3 phase, then it's 2 pole pairs.  3400RPM (or so) means 1 pole pair.
	int maxRPM;
	int throttleType;				// 0 = hall effect or "Max Ohms to Min Ohms" if using Pot.  1 = MIN OHMS to MAX OHMS.
	int encoderTicks;				// The number of ticks per revolution from the encoder.  MAXCNT will actually be 4 times this value.
	unsigned int dataToDisplaySet1;	
	// 0b0000 0000 0000 0000
	// bit 15 set: display counter1ms
	// Bit 14 set: display Id
	// bit 13 set: display Iq
	// Bit 12 set: display IdRef
	// bit 11 set: display IqRef
	// Bit 10 set: display Vd
	// bit 9 set: display Vq
	// Bit 8 set: display Ia
	// bit 7 set: display Ib
	// bit 6 set: display Ic
	// Bit 5 set: display Va
	// bit 4 set: display Vb
	// bit 3 set: display Vc
	// bit 2 set: display % of voltage being used.  Ratio of radius of <Vd,Vq> to total voltage.
	// bit 1-0 set: future use

	unsigned int dataToDisplaySet2;
	// Bit 15 set: display rawThrottle
	// bit 14 set: display normalizedThrottle
	// Bit 13 set: display temperature
	// bit 12 set: display slipSpeedRPM
	// Bit 11 set: display electricalSpeedRPM
	// bit 10 set: display mechanicalSpeedRPM
	// Bit  9 set: display batteryCurrent
	// Bit  8-0 set: future use. 

	int spares[7];
	unsigned crc;
} SavedValuesStruct2;

typedef struct {
	long Kp;
	long Ki;
	long error_d;
	long errorSum_d;
	long pwm_d;
	long error_q;
	long errorSum_q;
	long pwm_q;
	int testFinished;
	int testFailed;
	int testRunning;
	int ratioKpKi;
	int zeroCrossingIndex; // initialize to -1.
	int iteration; // how many times have you run the PI loop with the same Kp and Ki?  This is used in the PI auto loop tuning.
	int maxIterationsBeforeZeroCrossing; // the default is 20.  The PI loop test converges in like 12 iterations on my tests.
	int previousTestCompletionTime;		// You don't want to run another PI test with new Kp and Ki values too soon.  Make sure the current has gone back to zero before restarting a test.
	int clampErrorVd;
	int clampErrorVq;
} piType;

typedef struct {
	unsigned int startTime;
	int maxTestSpeed;
	int bestTimeConstantIndex;
	int timeConstantIndex;
	int testRunning;
	int testFinished;
} rotorTestType;

typedef struct {
	unsigned int startTime;
	int maxTestSpeed;
	int bestAngleOffset;
	int currentAngleOffset;
	int testRunning;
	int testFinished;
} angleOffsetTestType;

#endif
