
#include "ACMain.h"

/*****************Config bit settings****************/
_FOSC(0xFFFF & XT_PLL16);//XT_PLL4); // Use XT with external crystal from 4MHz to 10MHz.  FRM Pg. 178
// nominal clock is 128kHz.  The counter is 1 byte. 
//#define STANDARD_THROTTLE
//#ifdef DEBUG
	_FWDT(WDT_OFF);
//#else
//	_FWDT(WDT_ON & WDTPSA_64 & WDTPSB_8); // See Pg. 709 in F.R.M.  Timeout in 1 second or so.  128000 / 64 / 8 / 256
//#endif

_FBORPOR(0xFFFF & BORV_27 & PWRT_64 & MCLR_EN & PWMxL_ACT_HI & PWMxH_ACT_HI); // Brown Out voltage set to 2v.  Power up time of 64 ms. MCLR is enabled.
// PWMxL_ACT_HI means PDC1 = 0 would mean PWM1L is 0 volts.  Just the opposite of how I always thought it worked.  haha.
// PWMxH_ACT_HI means, in complementary mode, that PDC1 = 0 would mean PWM1H is 0v, and PWM1L is 5v. 
_FGS(CODE_PROT_OFF);

// 4688, 75
const SavedValuesStruct savedValuesDefault = {
	DEFAULT_KP,DEFAULT_KI,DEFAULT_KP,DEFAULT_KI,DEFAULT_CURRENT_SENSOR_AMPS_PER_VOLT,25,400,624,972,5,MAX_BATTERY_AMPS,MAX_BATTERY_AMPS_REGEN,MAX_MOTOR_AMPS,MAX_MOTOR_AMPS,DEFAULT_PRECHARGE_TIME,0
};

volatile SavedValuesStruct savedValues = {
	DEFAULT_KP,DEFAULT_KI,DEFAULT_KP,DEFAULT_KI,DEFAULT_CURRENT_SENSOR_AMPS_PER_VOLT,25,400,624,972,5,MAX_BATTERY_AMPS,MAX_BATTERY_AMPS_REGEN,MAX_MOTOR_AMPS,MAX_MOTOR_AMPS,DEFAULT_PRECHARGE_TIME,0
};

const SavedValuesStruct2 savedValuesDefault2 = {
	DEFAULT_ROTOR_TIME_CONSTANT_ARRAY_INDEX,2,6000,0, DEFAULT_ENCODER_TICKS,DEFAULT_STATOR_RESISTANCE_TIMES1024,DEFAULT_STATOR_INDUCTANCE_TIMES1024,DEFAULT_PACK_VOLTAGE,1,{0,0,0,0,0,0}, 0
};
volatile SavedValuesStruct2 savedValues2 = {
	DEFAULT_ROTOR_TIME_CONSTANT_ARRAY_INDEX,2,6000,0, DEFAULT_ENCODER_TICKS,DEFAULT_STATOR_RESISTANCE_TIMES1024,DEFAULT_STATOR_INDUCTANCE_TIMES1024,DEFAULT_PACK_VOLTAGE,1,{0,0,0,0,0,0}, 0
};
unsigned int revCounterMax = 160000L/(4*DEFAULT_ENCODER_TICKS);  // revCounterMax = 16000000L / (4*savedValues2.encoderTicks);  // 4* because I'm doing 4 times resolution for the encoder. 16,000,000 because revolutions per 16 seconds is computed as:  16*10,000*poscnt * rev/(maxPosCnt*revcounter*(16sec))

// This is always a copy of the data that's in the EE PROM.
// To change EE Prom, modify this, and then copy it to EE Prom.

int EEDataInRam1[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int EEDataInRam2[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int EEDataInRam3[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int EEDataInRam4[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

_prog_addressT EE_addr1 = 0x7ffc00;
_prog_addressT EE_addr2 = 0x7ffc20;
_prog_addressT EE_addr3 = 0x7ffc40;
_prog_addressT EE_addr4 = 0x7ffc60;

// = loopPeriod / rotorTimeConstant * 2^18.  loopPeriod is 0.0001 seconds, because it's being run at 10KHz. Rotor time constants range from 0.005 to 0.150 seconds.
// After using an element from this array, you must eventually divide the result by 2^18!!!
// rotorTimeConstantArray1[0] corresponds to a rotorTimeConstant of 0.005 seconds.  rotorTimeConstantArray1[145] <=> rotor time constant of 0.150 sec.
const unsigned int rotorTimeConstantArray1[] = {
//    0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56   57   58   59   60   61   62   63   64   65   66   67   68   69   70   71   72   73   74   75   76   77   78   79   80   81   82   83   84   85   86   87   88   89   90   91   92   93   94   95   96   97   98   99  100  101  102  103  104  105  106  107  108  109  110  111  112  113  114  115  116  117  118  119  120  121  122  123  124  125  126  127  128  129  130  131  132  133  134  135  136  137  138  139  140  141  142  143  144  145 
// .005,.006,.007,.008,.009,.010,.011,.012,.013,.014,.015,.016,.017,.018,.019,.020,.021,.022,.023,.024,.025,.026,.027,.028,.029,.030,.031,.032,.033,.034,.035,.036,.037,.038,.039,.040,.041,.042,.043,.044,.045,.046,.047,.048,.049,.050,.051,.052,.053,.054,.055,.056,.057,.058,.059,.060,.061,.062,.063,.064,.065,.066,.067,.068,.069,.070,.071,.072,.073,.074,.075,.076,.077,.078,.079,.080,.081,.082,.083,.084,.085,.086,.087,.088,.089,.090,.091,.092,.093,.094,.095,.096,.097,.098,.099,.100,.101,.102,.103,.104,.105,.106,.107,.108,.109,.110,.111,.112,.113,.114,.115,.116,.117,.118,.119,.120,.121,.122,.123,.124,.125,.126,.127,.128,.129,.130,.131,.132,.133,.134,.135,.136,.137,.138,.139,.140,.141,.142,.143,.144,.145,.146,.147,.148,.149,.150
   5243,4369,3745,3277,2913,2621,2383,2185,2016,1872,1748,1638,1542,1456,1380,1311,1248,1192,1140,1092,1049,1008, 971, 936, 904, 874, 846, 819, 794, 771, 749, 728, 708, 690, 672, 655, 639, 624, 610, 596, 583, 570, 558, 546, 535, 524, 514, 504, 495, 485, 477, 468, 460, 452, 444, 437, 430, 423, 416, 410, 403, 397, 391, 386, 380, 374, 369, 364, 359, 354, 350, 345, 340, 336, 332, 328, 324, 320, 316, 312, 308, 305, 301, 298, 295, 291, 288, 285, 282, 279, 276, 273, 270, 267, 265, 262, 260, 257, 255, 252, 250, 247, 245, 243, 240, 238, 236, 234, 232, 230, 228, 226, 224, 222, 220, 218, 217, 215, 213, 211, 210, 208, 206, 205, 203, 202, 200, 199, 197, 196, 194, 193, 191, 190, 189, 187, 186, 185, 183, 182, 181, 180, 178, 177, 176, 175
};
// (1/rotorTimeConstant) * 1/(2*pi) * 2^11.  I'm trying to keep all of them in integer range.
// rotorTimeConstantArray2[0] corresponds to a rotorTimeConstant of 0.005 seconds.  rotorTimeConstantArray2[145] = 0.150.
const unsigned int rotorTimeConstantArray2[] = {
//    0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56   57   58   59   60   61   62   63   64   65   66   67   68   69   70   71   72   73   74   75   76   77   78   79   80   81   82   83   84   85   86   87   88   89   90   91   92   93   94   95   96   97   98   99  100  101  102  103  104  105  106  107  108  109  110  111  112  113  114  115  116  117  118  119  120  121  122  123  124  125  126  127  128  129  130  131  132  133  134  135  136  137  138  139  140  141  142  143  144  145 
// .005, .006, .007, .008, .009, .010, .011, .012, .013, .014, .015, .016, .017, .018, .019, .020, .021, .022, .023, .024, .025, .026, .027, .028, .029, .030, .031, .032,.033,.034,.035,.036,.037,.038,.039,.040,.041,.042,.043,.044,.045,.046,.047,.048,.049,.050,.051,.052,.053,.054,.055,.056,.057,.058,.059,.060,.061,.062,.063,.064,.065,.066,.067,.068,.069,.070,.071,.072,.073,.074,.075,.076,.077,.078,.079,.080,.081,.082,.083,.084,.085,.086,.087,.088,.089,.090,.091,.092,.093,.094,.095,.096,.097,.098,.099,.100,.101,.102,.103,.104,.105,.106,.107,.108,.109,.110,.111,.112,.113,.114,.115,.116,.117,.118,.119,.120,.121,.122,.123,.124,.125,.126,.127,.128,.129,.130,.131,.132,.133,.134,.135,.136,.137,.138,.139,.140,.141,.142,.143,.144,.145,.146,.147,.148,.149,.150
  65190,54325,46564,40744,36217,32595,29632,27162,25073,23282,21730,20372,19173,18108,17155,16297,15521,14816,14172,13581,13038,12537,12072,11641,11240,10865,10514,10186,9877,9587,9313,9054,8809,8578,8358,8149,7950,7761,7580,7408,7243,7086,6935,6791,6652,6519,6391,6268,6150,6036,5926,5821,5718,5620,5525,5432,5343,5257,5174,5093,5015,4939,4865,4793,4724,4656,4591,4527,4465,4405,4346,4289,4233,4179,4126,4074,4024,3975,3927,3880,3835,3790,3747,3704,3662,3622,3582,3543,3505,3468,3431,3395,3360,3326,3292,3259,3227,3196,3165,3134,3104,3075,3046,3018,2990,2963,2936,2910,2885,2859,2834,2810,2786,2762,2739,2716,2694,2672,2650,2629,2608,2587,2567,2546,2527,2507,2488,2469,2451,2432,2414,2397,2379,2362,2345,2328,2312,2295,2279,2264,2248,2233,2217,2202,2188,2173
};

// This one is hard to explain.  It's to quickly find distance from (Vd, Vq) to (0,0).  (Vd,Vq) must be clamped to some maximum radius.  The fast distance I use isn't very accurate, so I add this correction.
// Let's say Vd <= Vq.  You find the index for this array by doing index = 1024 * Vd / Vq.  This works because the %error from fastDistance(Vd,Vq) is the same as long as Vd/Vq is the same.
// In other words, if Vd1/Vq1 = Vd2/Vq2, then fastDistance(Vd1,Vq1)/RealDistance(Vd1,Vq1) = fastDistance(Vd2,Vq2)/RealDistance(Vd2,Vq2).
// All this just to avoid doing a square root.  haha.
// It has been scaled up by 2^15.  So, you must shift down by 15 after multiplying by distCorrection[].
// array size is 1025.  distCorrection[0] up to distCorrection[1024] inclusive.
const unsigned int distCorrection[] = {
32768,32758,32748,32738,32728,32718,32709,32699,32689,32680,32670,32660,32651,32641,32632,32622,32613,32603,32594,32585,32575,32566,32557,32548,32539,32530,32521,32512,32503,32494,32485,32476,32467,32458,32449,32441,32432,32423,32415,32406,32398,32389,32381,32372,32364,32355,32347,32339,32330,32322,32314,32306,32298,32290,32282,32274,32266,32258,32250,32242,32234,32226,32218,32211,32203,32195,32188,32180,32173,32165,32158,32150,32143,32135,32128,32121,32113,32106,32099,32092,32085,32077,32070,32063,32056,32049,32042,32036,32029,32022,32015,32008,32002,31995,31988,31982,31975,31968,31962,31955,31949,31942,31936,31930,31923,
31917,31911,31905,31898,31892,31886,31880,31874,31868,31862,31856,31850,31844,31838,31832,31827,31821,31815,31810,31804,31798,31793,31787,31782,31776,31771,31765,31760,31754,31749,31744,31738,31733,31728,31723,31718,31713,31708,31702,31697,31692,31688,31683,31678,31673,31668,31663,31658,31654,31649,31644,31640,31635,31631,31626,31622,31617,31613,31608,31604,31600,31595,31591,31587,31582,31578,31574,31570,31566,31562,31558,31554,31550,31546,31542,31538,31534,31530,31526,31523,31519,31515,31512,31508,31504,31501,31497,31494,31490,31487,31483,31480,31477,31473,31470,31467,31463,31460,31457,31454,31451,31448,31444,31441,31438,
31435,31432,31429,31427,31424,31421,31418,31415,31413,31410,31407,31404,31402,31399,31397,31394,31392,31389,31387,31384,31382,31379,31377,31375,31372,31370,31368,31366,31363,31361,31359,31357,31355,31353,31351,31349,31347,31345,31343,31341,31339,31338,31336,31334,31332,31331,31329,31327,31326,31324,31322,31321,31319,31318,31316,31315,31314,31312,31311,31310,31308,31307,31306,31304,31303,31302,31301,31300,31299,31298,31297,31296,31295,31294,31293,31292,31291,31290,31289,31289,31288,31287,31286,31286,31285,31284,31284,31283,31282,31282,31281,31281,31280,31280,31280,31279,31279,31279,31278,31278,31278,31277,31277,31277,31277,
31277,31277,31277,31276,31276,31276,31276,31276,31277,31277,31277,31277,31277,31277,31277,31278,31278,31278,31278,31279,31279,31280,31280,31280,31281,31281,31282,31282,31283,31283,31284,31285,31285,31286,31287,31287,31288,31289,31290,31290,31291,31292,31293,31294,31295,31296,31297,31298,31299,31300,31301,31302,31303,31304,31305,31306,31308,31309,31310,31311,31313,31314,31315,31317,31318,31319,31321,31322,31324,31325,31327,31328,31330,31331,31333,31335,31336,31338,31340,31341,31343,31345,31347,31348,31350,31352,31354,31356,31358,31360,31362,31364,31366,31368,31370,31372,31374,31376,31378,31380,31382,31384,31387,31389,31391,
31393,31396,31398,31400,31403,31405,31407,31410,31412,31415,31417,31420,31422,31425,31427,31430,31432,31435,31438,31440,31443,31446,31448,31451,31454,31457,31459,31462,31465,31468,31471,31474,31476,31479,31482,31485,31488,31491,31494,31497,31500,31503,31507,31510,31513,31516,31519,31522,31526,31529,31532,31535,31539,31542,31545,31549,31552,31555,31559,31562,31566,31569,31572,31576,31579,31583,31587,31590,31594,31597,31601,31605,31608,31612,31616,31619,31623,31627,31630,31634,31638,31642,31646,31650,31653,31657,31661,31665,31669,31673,31677,31681,31685,31689,31693,31697,31701,31705,31709,31713,31718,31722,31726,31730,31734,
31739,31743,31747,31751,31756,31760,31764,31769,31773,31777,31782,31786,31791,31795,31800,31804,31808,31813,31817,31822,31827,31831,31836,31840,31845,31850,31854,31859,31864,31868,31873,31878,31882,31887,31892,31897,31902,31906,31911,31916,31921,31926,31931,31936,31941,31946,31951,31956,31961,31966,31971,31976,31981,31986,31991,31996,32001,32006,32011,32016,32022,32027,32032,32037,32042,32048,32053,32058,32063,32069,32074,32079,32085,32090,32095,32101,32106,32112,32117,32123,32128,32133,32139,32144,32150,32155,32161,32167,32172,32178,32183,32189,32195,32200,32206,32212,32217,32223,32229,32234,32240,32246,32252,32257,32263,
32269,32275,32281,32286,32292,32298,32304,32310,32316,32322,32328,32334,32339,32345,32351,32357,32363,32369,32375,32382,32388,32394,32400,32406,32412,32418,32424,32430,32436,32443,32449,32455,32461,32467,32474,32480,32486,32492,32499,32505,32511,32518,32524,32530,32537,32543,32549,32556,32562,32569,32575,32581,32588,32594,32601,32607,32614,32620,32627,32633,32640,32646,32653,32660,32666,32673,32679,32686,32693,32699,32706,32713,32719,32726,32733,32739,32746,32753,32759,32766,32773,32780,32787,32793,32800,32807,32814,32821,32827,32834,32841,32848,32855,32862,32869,32876,32883,32890,32896,32903,32910,32917,32924,32931,32938,
32945,32952,32960,32967,32974,32981,32988,32995,33002,33009,33016,33023,33031,33038,33045,33052,33059,33066,33074,33081,33088,33095,33102,33110,33117,33124,33132,33139,33146,33153,33161,33168,33175,33183,33190,33198,33205,33212,33220,33227,33234,33242,33249,33257,33264,33272,33279,33287,33294,33302,33309,33317,33324,33332,33339,33347,33354,33362,33369,33377,33385,33392,33400,33407,33415,33423,33430,33438,33446,33453,33461,33469,33476,33484,33492,33499,33507,33515,33523,33530,33538,33546,33554,33561,33569,33577,33585,33593,33600,33608,33616,33624,33632,33640,33648,33655,33663,33671,33679,33687,33695,33703,33711,33719,33727,
33735,33743,33751,33759,33767,33775,33783,33791,33799,33807,33815,33823,33831,33839,33847,33855,33863,33871,33879,33887,33895,33903,33912,33920,33928,33936,33944,33952,33960,33969,33977,33985,33993,34001,34010,34018,34026,34034,34042,34051,34059,34067,34075,34084,34092,34100,34109,34117,34125,34133,34142,34150,34158,34167,34175,34183,34192,34200,34208,34217,34225,34234,34242,34250,34259,34267,34276,34284,34292,34301,34309,34318,34326,34335,34343,34352,34360,34369,34377,34386,34394,34403,34411,34420,34428,34437,34445,34454,34462,34471,34479,34488,34497,34505,34514,34522,34531,34539,34548,34557,34565,34574,34583,34591,34600,
34608,34617,34626,34634,34643,34652,34660,34669,34678,34687,34695,34704,34713,34721,34730,34739,34748,34756,34765,34774,34782,34791,34800,34809,34818,34826,34835,34844,34853,34861,34870,34879,34888,34897,34906,34914,34923,34932,34941,34950,34959,34967,34976,34985,34994,35003,35012,35021,35030,35038,35047,35056,35065,35074,35083,35092,35101,35110,35119,35128,35137,35146,35154,35163,35172,35181,35190,35199,35208,35217,35226,35235,35244,35253,35262,35271,35280,35289,35298,35307,35316,35325,35334,35343,
};

// 512 possible values for sin.  for x in [-1, 1] = [-32767, 32767].  So the scale is about 2^15 - 1.  Let's pretend it's 2^15.  haha.
const int _sin_times32768[] =  
{0, 	402,	804,	1206,	1608,	2009,	2410,	2811,	3212,	3612,	4011,	4410,	4808,	5205,	5602,	5998,	6393,	6786,	7179,	7571,	7962,
8351,	8739,	9126,	9512,	9896,	10278,	10659,	11039,	11417,	11793,	12167,	12539,	12910,	13279,	13645,	14010,	14372,	14732,	15090,	15446,
15800,	16151,	16499,	16846,	17189,	17530,	17869,	18204,	18537,	18868,	19195,	19519,	19841,	20159,	20475,	20787,	21096,	21403,	21705,	22005,
22301,	22594,	22884,	23170,	23452,	23731,	24007,	24279,	24547,	24811,	25072,	25329,	25582,	25832,	26077,	26319,	26556,	26790,	27019,	27245,
27466,	27683,	27896,	28105,	28310,	28510,	28706,	28898,	29085,	29268,	29447,	29621,	29791,	29956,	30117,	30273,	30424,	30571,	30714,	30852,
30985,	31113,	31237,	31356,	31470,	31580,	31685,	31785,	31880,	31971,	32057,	32137,	32213,	32285,	32351,	32412,	32469,	32521,	32567,	32609,
32646,	32678,	32705,	32728,	32745,	32757,	32765,	32767,	32765,	32757,	32745,	32728,	32705,	32678,	32646,	32609,	32567,	32521,	32469,	32412,
32351,	32285,	32213,	32137,	32057,	31971,	31880,	31785,	31685,	31580,	31470,	31356,	31237,	31113,	30985,	30852,	30714,	30571,	30424,	30273,
30117,	29956,	29791,	29621,	29447,	29268,	29085,	28898,	28706,	28510,	28310,	28105,	27896,	27683,	27466,	27245,	27019,	26790,	26556,	26319,
26077,	25832,	25582,	25329,	25072,	24811,	24547,	24279,	24007,	23731,	23452,	23170,	22884,	22594,	22301,	22005,	21705,	21403,	21096,	20787,
20475,	20159,	19841,	19519,	19195,	18868,	18537,	18204,	17869,	17530,	17189,	16846,	16499,	16151,	15800,	15446,	15090,	14732,	14372,	14010,
13645,	13279,	12910,	12539,	12167,	11793,	11417,	11039,	10659,	10278,	9896,	9512,	9126,	8739,	8351,	7962,	7571,	7179,	6786,	6393,
5998,	5602,	5205,	4808,	4410,	4011,	3612,	3212,	2811,	2410,	2009,	1608,	1206,	804,	402,	0,  	-402,	-804,	-1206,	-1608,
-2009,	-2410,	-2811,	-3212,	-3612,	-4011,	-4410,	-4808,	-5205,	-5602,	-5998,	-6393,	-6786,	-7179,	-7571,	-7962,	-8351,	-8739,	-9126,	-9512,
-9896,	-10278,	-10659,	-11039,	-11417,	-11793,	-12167,	-12539,	-12910,	-13279,	-13645,	-14010,	-14372,	-14732,	-15090,	-15446,	-15800,	-16151,	-16499,	-16846,
-17189,	-17530,	-17869,	-18204,	-18537,	-18868,	-19195,	-19519,	-19841,	-20159,	-20475,	-20787,	-21096,	-21403,	-21705,	-22005,	-22301,	-22594,	-22884,	-23170,
-23452,	-23731,	-24007,	-24279,	-24547,	-24811,	-25072,	-25329,	-25582,	-25832,	-26077,	-26319,	-26556,	-26790,	-27019,	-27245,	-27466,	-27683,	-27896,	-28105,
-28310,	-28510,	-28706,	-28898,	-29085,	-29268,	-29447,	-29621,	-29791,	-29956,	-30117,	-30273,	-30424,	-30571,	-30714,	-30852,	-30985,	-31113,	-31237,	-31356,
-31470,	-31580,	-31685,	-31785,	-31880,	-31971,	-32057,	-32137,	-32213,	-32285,	-32351,	-32412,	-32469,	-32521,	-32567,	-32609,	-32646,	-32678,	-32705,	-32728,
-32745,	-32757,	-32765,	-32767,	-32765,	-32757,	-32745,	-32728,	-32705,	-32678,	-32646,	-32609,	-32567,	-32521,	-32469,	-32412,	-32351,	-32285,	-32213,	-32137,
-32057,	-31971,	-31880,	-31785,	-31685,	-31580,	-31470,	-31356,	-31237,	-31113,	-30985,	-30852,	-30714,	-30571,	-30424,	-30273,	-30117,	-29956,	-29791,	-29621,
-29447,	-29268,	-29085,	-28898,	-28706,	-28510,	-28310,	-28105,	-27896,	-27683,	-27466,	-27245,	-27019,	-26790,	-26556,	-26319,	-26077,	-25832,	-25582,	-25329,
-25072,	-24811,	-24547,	-24279,	-24007,	-23731,	-23452,	-23170,	-22884,	-22594,	-22301,	-22005,	-21705,	-21403,	-21096,	-20787,	-20475,	-20159,	-19841,	-19519,
-19195,	-18868,	-18537,	-18204,	-17869,	-17530,	-17189,	-16846,	-16499,	-16151,	-15800,	-15446,	-15090,	-14732,	-14372,	-14010,	-13645,	-13279,	-12910,	-12539,
-12167,	-11793,	-11417,	-11039,	-10659,	-10278,	-9896,	-9512,	-9126,	-8739,	-8351,	-7962,	-7571,	-7179,	-6786,	-6393,	-5998,	-5602,	-5205,	-4808,
-4410,	-4011,	-3612,	-3212,	-2811,	-2410,	-2009,	-1608,	-1206,	-804,	-402, 		0,		0,		0,		0,		0,		0,		0,		0,};
////////////////////////////////////////////////////////////////

extern char string[];

//volatile int dataDumping = 0;

volatile int largeArrayLoaded = 0;
volatile int captureVariable = 0;
volatile int dataCounter = 0;


// in void ComputeRotorFluxAngle() {
volatile unsigned int rotorFluxAngle_times128 = 0;  // For fine control.
volatile int rotorFluxRPS_times16 = 0;
volatile int rotorFluxRPS_times16_sensorless = 0;

volatile int angleChange_times128 = 0;
volatile int angleChange_times128_sensorless = 0;
volatile unsigned int oldRotorFluxAngle = 1;
volatile unsigned int rotorFluxAngle = 1; 		// This is the rotor flux angle. In [0, 511]

volatile int RPS_times16 = 0; // range [-3200, 3200], where 3200 corresponds to 200rev/sec = 12,000RPM, and 0 means 0RPM.
volatile int RPS_times16_sensorless = 0;
volatile int currentSensorAmpsPerVoltTimes5 = DEFAULT_CURRENT_SENSOR_AMPS_PER_VOLT*5;


volatile int maxRPS_times16 = DEFAULT_MAX_RPS_TIMES16; // 3200 CORRESPONDS TO 12000RPM.
volatile int oldRPS = 0;	// previous RPS_times16.
volatile int veryOldRPS = 0; // previous oldRPS.

// ComputeRotorFluxSpeedSensorless();
/*
volatile int v_alpha_volts_times8 = 0;
volatile int v_beta_volts_times8 = 0;
volatile int di_alpha_dt_times8 = 0;
volatile int di_beta_dt_times8 = 0;


volatile int i_alpha_amps_times8_old10 = 0;//i_alpha_amps_times8_old9;
volatile int i_alpha_amps_times8_old9 = 0;//i_alpha_amps_times8_old8;
volatile int i_alpha_amps_times8_old8 = 0;//i_alpha_amps_times8_old7;
volatile int i_alpha_amps_times8_old7 = 0;//i_alpha_amps_times8_old6;
volatile int i_alpha_amps_times8_old6 = 0;//i_alpha_amps_times8_old5;
volatile int i_alpha_amps_times8_old5 = 0; // i_alpha_amps_times8_old4;
volatile int i_alpha_amps_times8_old4 = 0; // i_alpha_amps_times8_old3;
volatile int i_alpha_amps_times8_old3 = 0; // i_alpha_amps_times8_old2;
volatile int i_alpha_amps_times8_old2 = 0; // i_alpha_amps_times8_old1;
volatile int i_alpha_amps_times8_old1 = 0; // i_alpha_amps_times8;
volatile int i_alpha_amps_times8 = 0; // __builtin_mulss((int)currentSensorAmpsPerVoltTimes5, (int)i_alpha) >> 11;

volatile int temp = 0; // 0;

volatile int i_beta_amps_times8_old10 = 0; // i_beta_amps_times8_old9;
volatile int i_beta_amps_times8_old9 = 0; // i_beta_amps_times8_old8;
volatile int i_beta_amps_times8_old8 = 0; // i_beta_amps_times8_old7;
volatile int i_beta_amps_times8_old7 = 0; // i_beta_amps_times8_old6;
volatile int i_beta_amps_times8_old6 = 0; // i_beta_amps_times8_old5;
volatile int i_beta_amps_times8_old5 = 0; // i_beta_amps_times8_old4;
volatile int i_beta_amps_times8_old4 = 0; // i_beta_amps_times8_old3;
volatile int i_beta_amps_times8_old3 = 0; // i_beta_amps_times8_old2;
volatile int i_beta_amps_times8_old2 = 0; // i_beta_amps_times8_old1;
volatile int i_beta_amps_times8_old1 = 0; // i_beta_amps_times8;
volatile int i_beta_amps_times8 = 0; // __builtin_mulss((int)currentSensorAmpsPerVoltTimes5, (int)i_beta) >> 11;



volatile int BEMFAngleSensorless = 1;
volatile int rotorFluxAngleSensorless = 1;
volatile int oldRotorFluxAngleSensorless = 1; 		//

volatile int positiveRotation = 0;

volatile long Ed_times8_times65536 = 0L;
volatile int Ed_times8_filtered = 0L;
volatile long Eq_times8_times65536 = 0L;
volatile int Eq_times8_filtered = 0L;

volatile long tempLongAlpha = 0L;
volatile long tempLongBeta = 0L;
volatile long tempLongEdEq = 0L;

volatile long temp1 = 0;
volatile long temp2 = 0;
volatile long temp3 = 0;
volatile long temp4 = 0;

volatile int magnitudeIAlpha_times8 = 0;
volatile int magnitudeIBeta_times8 = 0;
volatile int magnitudeVAlpha_times8 = 0;
volatile int magnitudeVBeta_times8 = 0;
volatile int E_alpha_times8 = 0;
volatile int E_beta_times8 = 0;
volatile int Ed_times8 = 0;
volatile int Eq_times8 = 0;
volatile int theta = 0;
volatile int thetaPlus90 = 0;
volatile int i_alpha_amps_times8_filtered = 0;
volatile long i_alpha_amps_times8_filtered_times65536 = 0L;
volatile int i_alpha_amps_times8_filtered_corrected = 0;
volatile int i_beta_amps_times8_filtered = 0;
volatile long i_beta_amps_times8_filtered_times65536 = 0L;
volatile int i_beta_amps_times8_filtered_corrected = 0;
volatile int v_alpha_volts_times8_filtered = 0;
volatile long v_alpha_volts_times8_filtered_times65536 = 0L;

volatile int v_alpha_volts_times8_filtered_corrected = 0;
volatile int v_beta_volts_times8_filtered = 0;
volatile long v_beta_volts_times8_filtered_times65536 = 0L;

volatile int v_beta_volts_times8_filtered_corrected = 0;
volatile long int di_alpha_dt_times8_filtered_corrected = 0;
volatile long int di_beta_dt_times8_filtered_corrected = 0;
volatile int periodIBeta_div_2 = 10000;  // just a guess for the startup period.
volatile int periodIAlpha_div_2 = 10000;
volatile int periodVBeta_div_2 = 10000;
volatile int periodVAlpha_div_2 = 10000;
volatile int localMaxCandidateIAlpha = 0;
volatile int localMinCandidateIAlpha = 0;
volatile int localMaxCandidateIBeta = 0;
volatile int localMinCandidateIBeta = 0;
volatile int localMaxCandidateVBeta = 0;
volatile int localMinCandidateVBeta = 0;
volatile int localMaxCandidateVAlpha = 0;
volatile int localMinCandidateVAlpha = 0;
volatile int tIAlpha = 0;
volatile int tIBeta = 0;
volatile int tVAlpha = 0;
volatile int tVBeta = 0;
volatile int shiftCorrection = 0;
volatile int correctionIndex = 0;
volatile char stateIAlpha = 0;
volatile char stateIBeta = 0;
volatile char stateVAlpha = 0;
volatile char stateVBeta = 0;
volatile int derivativeScale = 0;

volatile int magnitudeCorrection_times1024 = 0;
volatile int tempIAlpha = 0;
volatile int tempIBeta = 0;
volatile int tempVAlpha = 0;
volatile int tempVBeta = 0;
*/
// END OF ComputeRotorFluxSpeedSensorless(); VARIABLES /////////////////////////////

volatile unsigned int rGlobal = 0;
volatile long rGlobal_filtered_times65536 = 0L;//   
volatile int rGlobal_filtered = 0;//


volatile long magCurrChange = 0;
volatile long slipSpeedNumerator = 0;
volatile int slipSpeedRPS_times16 = 0;
volatile long magnetizingCurrentFine = 0;
volatile int magnetizingCurrent = 0;
volatile int magnetizingCurrentAmps_times8 = 0;
volatile unsigned int elapsedTimeInterrupt = 0;

//volatile unsigned int LrLmSquared_times128;  // Lr / Lm^2.  rotor inductance / (mutual inductance^2).  One data point:  Leeson Motor rotor inductance = 0.12117H.  Mutual Inductance = 0.11215.  Stator Inductance = 0.129930H
									// Lm / Ls = 0.866.  Is that related to the motor efficiency?  I'm going to assume yes.  So, I"m going to use Lr / (sqrt(3)/2 * sqrt(3)/2 * Lr*Lr) = 4/3 * 1/Lr as a guess.  A correction for voltage, current, and radian units is required.

	volatile long temp_filtered_times65536 = 0;
	volatile int temp_filtered = 0;
	volatile int minTemp = 32767;
	volatile int maxTemp = 0;
	volatile int minLs = 0;
	volatile int maxLs = 0;
	volatile int minLr = 0;
	volatile int maxLr = 0;
	volatile int minRs = 0;
	volatile int maxRs = 0;

volatile int piRatio = 62;
volatile int piZeroCrossingIndex = -1;
volatile int piGoodValuesIndex = 0;
volatile int piGoodValuesArrayLoaded = 0;
volatile int huntingForGoodPIValues = 0;
volatile int piIterationIndex = 0;
volatile int startNewRotorTest = 0;	
volatile int rotorArrayLoaded = 0;

volatile unsigned int counter10k = 0;
volatile unsigned int period10k = 0;
volatile unsigned int periodStart = 0;
volatile unsigned int delayBetweenPITests = 0;
volatile unsigned int rotorStartTime = 0;

volatile largeStorageType largeStorage;
volatile realtime_data_type RTData = {0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile unsigned int showDatastreamJustOnce = 0;
volatile unsigned int faultBits = STARTUP_FAULT;
volatile unsigned int oldFaultBits = 0;
volatile int vRef1 = 512, vRef2 = 512;  // these are temporary values.  The real values will be computed later, but this is close, since zero current corresponds to 2.5v on the current sensor.
volatile long throttleSum = 0;
volatile int throttle = 0;
volatile int maxMotorCurrentNormalizedRegen = 0;
volatile int maxMotorCurrentNormalized = 0;
volatile int batteryCurrentNormalized = 0;
volatile int batteryAmps = 0;
volatile int maxBatteryCurrentNormalized = 0;
volatile int maxBatteryCurrentNormalizedRegen = 0;
volatile int normalizedToAmpsMultiplier = 0;
volatile long batteryCurrentSum = 0;

volatile int temperatureMultiplier = 8;

volatile int throttleCounter = 0;
volatile int throttleFaultCounter = 0;
volatile unsigned int datastreamPeriod = 0;  //  the real time data string will transmit every "dataStreamPeriod" milliseconds.
volatile int temperatureBasePlate = 0;
volatile long temperatureSum = 0;
volatile unsigned int counter1ms = 0;
volatile int ADCurrent1 = 0, ADCurrent2 = 0;
volatile piType pi_Iq, pi_Id;

// _ADInterrupt() variables
volatile int i_alpha = 0;
volatile int i_beta = 0;
volatile int Id = 0;
volatile int Iq = 0;
volatile int Ia = 0, Ib_times2 = 0;
volatile int Vd = 0, Vq = 0;
volatile int v_alpha = 0;
volatile int v_beta = 0;

volatile int cos_theta_times32768 = 0;
volatile int sin_theta_times32768 = 0;
volatile unsigned int rotorFluxAnglePlus90 = 0;

volatile int pdc1 = 0;
volatile int pdc2 = 0;
volatile int pdc3 = 0;
volatile int averageDuty = 0;

volatile int maxSpeed = 0;
volatile int clampErrorVd = 0;
volatile int clampErrorVdPOS = 0;
volatile int clampErrorVq = 0;
volatile int clampErrorVqPOS = 0;
volatile int Va = 0, Vb = 0, Vc = 0;
volatile int IdRef = 0;	// in the range [0, 4096]
volatile int IdRefRef = 0;

volatile int IqRefRef = 0;
volatile int IqRef = 0; // in the range [-4096, 4096]

extern int TransmitString(char* str);
extern void u16x_to_str(char *str, unsigned val, unsigned char digits);
extern void u16_to_str(char *str, unsigned val, unsigned char digits);
extern void int16_to_str(char *str, int val, unsigned char digits);
extern void ShowMenu(void);
extern void ProcessCommand(void);
extern void InitUART2(void);

void FetchRTData();
void InitTimers();
void InitIORegisters(void);
void Delay(unsigned int);
void DelayTenthsSecond(int time);
void DelaySeconds(int time);
void ReadADInputs();
void GrabADResults();
void InitADAndPWM();
void InitDiscreteADConversions();
void GetVRefs();
void DelaySeconds(int seconds);
void DecimalToString(int number, int length);
char IntToCharHex(unsigned int i);
void InitCNModule();
void InitPIStruct();
void ClearAllFaults();  // clear the flip flop fault and the desat fault.
void ClearDesatFault();
void ClearFlipFlop();
void ComputeRotorFluxAngle();
void ComputeRotorFluxAngleSensorless();
void SpaceVectorModulation();
void ClampVdVq();
void Delay1uS();
int __arctan(int numerator, int denominator);


void __attribute__((__interrupt__, auto_psv)) _CNInterrupt(void);
void __attribute__ ((__interrupt__,auto_psv)) _ADCInterrupt(void);
void __attribute__ ((__interrupt__,auto_psv)) _MathError(void);
void __attribute__ ((__interrupt__,auto_psv)) _StackError(void);
void __attribute__ ((__interrupt__,auto_psv)) _AddressError(void);
void __attribute__ ((__interrupt__,auto_psv)) _OscillatorFail(void);


void MoveDataFromEEPromToRAM();
void EESaveValues();
void InitializeThrottleAndCurrentVariables();
void MoveToNextPIValues();

int main() {
	int i = 0, j = 0;
	unsigned int now = 0;
	long int vRef1Sum = 0;
	long int vRef2Sum = 0;
	int localCurrent1 = 0;
	int localCurrent2 = 0;
	int bestRotorIndex = 0;
	int bestRotorSpeed = 0;
	unsigned int notShownFaultYet = 0x0FFFF;	


	InitIORegisters();
	InitTimers();  // Now timer1 is running at around 59KHz.
	DelayTenthsSecond(10); // delay 0.3 sec to let voltages settle.  The dang precharge relay just turned on, and may cause a sag in the +5v supply?
	// make sure its' up to 5v first.
	O_LAT_PRECHARGE_RELAY = 1;  // turn on precharge relay.

	MoveDataFromEEPromToRAM();

	InitCNModule();
	InitializeThrottleAndCurrentVariables();
	InitPIStruct();
	InitADAndPWM();		// Now the A/D is triggered by the pwm period, and the PWM interrupt is enabled.
	InitUART2();  // Now the UART is running.
	
	for (i = 0; i < 256; i++) {
		now = counter10k;
		while (counter10k == now) {
			ClrWdt();
		} // a new A/D value will be available.
		localCurrent1 = ADCurrent1;
		localCurrent2 = ADCurrent2;
		vRef1Sum += (long)localCurrent1;
		vRef2Sum += (long)localCurrent2;
	}
	vRef1 = (vRef1Sum >> 8);
	vRef2 = (vRef2Sum >> 8);
	if (vRef1 > 512 + 50 || vRef1 < 512 - 50 || vRef2 > 512 + 50 || vRef2 < 512 - 50) {
 		faultBits |= VREF_FAULT;
	}
	DelayTenthsSecond(savedValues.prechargeTime + 1);  // Make sure at least 5 time constants for precharge time!!
	O_LAT_CONTACTOR = 1;  // close main contactor.
	DelayTenthsSecond(5);  // delete later??
	O_LAT_PRECHARGE_RELAY = 0;  // open precharge relay once main contactor is closed.

	ClearAllFaults();	// The flip flop and desaturation detection faults start up in an unknown state. clear them.
	U2STAbits.OERR = 0; // ClearReceiveBuffer();
	ShowMenu(); 	// serial show menu.
	while(1) {
		ProcessCommand();  // If there's a command to be processed, this does it.  haha.
		if (TMR2 >= 59) {  // TMR3:TMR2 is a 32 bit timer, running at 59KHz.  So, 59 ticks of TMR2 (the low 16 bits) is about 1ms.
			TMR2 = 0;
			counter1ms++;
		}
		if (largeArrayLoaded) {
			largeArrayLoaded = 0;
			captureVariable = 0;
			dataCounter = 0;
			for (j = 0; j < NUMBER_OF_DATA_TYPES; j++) {
				for (i = 0; i < PI_GOOD_VALUES_ARRAY_SIZE/NUMBER_OF_DATA_TYPES; i++) {
					int16_to_str(&string[0], (int)largeStorage.data[j][i], 5);
					TransmitString(string);
				}
				TransmitString("\r\n");
			}
			TransmitString("rps times 16 = ");
			int16_to_str(&string[0], (int)RPS_times16,5);// (int)((((long)rotorFluxRPS_times16) * 15L) >> 2), 5);
			TransmitString(string);
		}
		if (piGoodValuesArrayLoaded) {
			piGoodValuesArrayLoaded = 0;
			for (i = 0; i < piGoodValuesIndex; i++) {
				int16_to_str(&string[0], (int)largeStorage.PI_data[i], 5);
				TransmitString(string);
			}
			if (piGoodValuesIndex == 0) {
				TransmitString("No values passed the test.\r\n");
			}
		}
		if (rotorArrayLoaded) {
			rotorArrayLoaded = 0;
			bestRotorIndex = 0;
			bestRotorSpeed = 0;
			for (i = 0; i < ROTOR_TIME_CONSTANT_ARRAY_SIZE; i++) {
				if (bestRotorSpeed < largeStorage.rotor_data[i]) {
					bestRotorIndex = i;
				}
				int16_to_str(&string[0], (int)((((long)largeStorage.rotor_data[i]) * 15L) >> 2), 5);
				TransmitString(string);
			}
			savedValues2.rotorTimeConstantArrayIndex = bestRotorIndex;
			// saveToEE()
		}
		if (I_PORT_GLOBAL_FAULT == 0) {
			faultBits |= GLOBAL_FAULT;
		}
		if (faultBits & STARTUP_FAULT) {
//			if (throttle == 0) { // Make sure throttle is zero before allowing things to start.
				faultBits &= ~STARTUP_FAULT;
//			}
		}
		if (faultBits != 0) {
			if ((faultBits & 1) && (notShownFaultYet & 1)) {
				TransmitString("Throttle out of range! Is it unplugged?\r\n");
				notShownFaultYet &= ~1;
			}
			if ((faultBits & 2) && (notShownFaultYet & 2)) {
				TransmitString("Desaturation Detection Fault!  That's actually pretty bad.\r\n");
				notShownFaultYet &= ~2;
			}
			if ((faultBits & 4) && (notShownFaultYet & 4)) {
				TransmitString("UART fault.  Is the serial cable unplugged?\r\n");
				notShownFaultYet &= ~4;
			}
			if ((faultBits & 8) && (notShownFaultYet & 8)) {
				TransmitString("Undervoltage Fault.  Either your 5v or 24v supply dropped too low.\r\n");
				notShownFaultYet &= ~8;
			}
			if ((faultBits & 16) && (notShownFaultYet & 16)) {
				TransmitString("Overcurrent fault.  The hardware overcurrent protection just came on and saved the day.\r\n");
				notShownFaultYet &= ~16;
			}
			if ((faultBits & 32) && (notShownFaultYet & 32)) {
				TransmitString("Current sensor fault.  Is a current sensor unplugged?\r\n");
				notShownFaultYet &= ~32;
			}
			if ((faultBits & 64) && (notShownFaultYet & 64)) {
				TransmitString("High pedal lockout fault.  Ignore this for now.  It's not really a fault.  haha.\r\n");
				notShownFaultYet &= ~64;
			}
			if ((faultBits & 128) && (notShownFaultYet & 128)) {
				TransmitString("Rotor flux angle fault.  something weird happened with the rotor flux angle.\r\n");
				notShownFaultYet &= ~128;
			}
			if ((faultBits & 256) && (notShownFaultYet & 256)) {
				TransmitString("There was some hardware caused fault, not originating on the microcontroller (not set by me!\r\n");
				notShownFaultYet &= ~256;
			}
			if ((faultBits & 512) && (notShownFaultYet & 512)) {
				TransmitString("P I Overflow fault.  It's really not tracking IdRef and IqRef well at all.\r\n");
				notShownFaultYet &= ~512;
			}
			if ((faultBits & 1024) && (notShownFaultYet & 1024)) {
				TransmitString("PDC Fault.  I think I got rid of this one.\r\n");
				notShownFaultYet &= ~1024;
			}
			if ((faultBits & 2048) && (notShownFaultYet & 2048)) {
				TransmitString("Motor Overspeed Fault.  The motor RPM got too high!\r\n");
				notShownFaultYet &= ~2048;
			}
			if ((faultBits & 4096) && (notShownFaultYet & 4096)) {
				TransmitString("Magnetizing Current fault.  It got crazy high for some reason.\r\n");
				notShownFaultYet &= ~4096;
			}
			if ((faultBits & 8192) && (notShownFaultYet & 8192)) {
				TransmitString("Encoder cable unplugged.  I'm not using this one at the moment.\r\n");
				notShownFaultYet &= ~8192;
			}
			if ((faultBits & 16384) && (notShownFaultYet & 16384)) {
				TransmitString("\r\nI bet you just typed 'off'  haha..\r\n");
				notShownFaultYet &= ~16384;
			}
		}
		// let the interrupts take care of the rest...
		ClrWdt();  // kick the watchdog.  haha.  That's a Fran original.
 	}
}

//---------------------------------------------------------------------
// The ADC sample and conversion is triggered by the PWM period.
//---------------------------------------------------------------------
// This runs at 9.997kHz.
void __attribute__ ((__interrupt__,auto_psv)) _ADCInterrupt(void) {
//	static int startUp = 1;
	static int tempVd = 0;
	static int tempVq = 0;
	static int temp = 0;
	static int rampRate = 1;
//	static int minClampErrorVd = 0;
//	static int maxClampErrorVd = 0;
//	static int minClampErrorVq = 0;
//	static int maxClampErrorVq = 0;
	static int newPeriodStarted = 0;
	static int ratio_times1250 = 0;
	static int periodOffset = 0;
	static int periodOffsetStart = 0;
	static unsigned int startTimeInterrupt = 0;
	static long batteryCurrentLong = 0;
	static long vBetaSqrt3_times32768 = 0;
	static long v_alpha_times32768 = 0;
	static int revCounter = 0;	// revCounter increments at 10kHz.  When it gets to 78, the number of ticks in POSCNT is extremely close to the revolutions per seoond * 16.
								// So, the motor mechanical speed will be computed every 1/128 seconds, and will have a range of [0, 3200], where 3200 corresponds to 12000rpm.	

	startTimeInterrupt = TMR4;

    IFS0bits.ADIF = 0;  	// Interrupt Flag Status Register. Pg. 142 in F.R.M.

	counter10k++;	
	// ADIF = A/D Conversion Complete Interrupt Flag Status bit.  
	// ADIF = 0 means we are resetting it so that an interrupt request has not occurred.


	// so dense... so glorious.  Covers positive and negative RPM.
	// 
//#ifndef SENSORLESS
	revCounter++;
	if (revCounter >= revCounterMax) { // 512 ticks per revolution for encoder.
		RPS_times16 = POSCNT;	// if POSCNT is 0x0FFFF due to THE MOTOR GOING BACKWARDS, RPS_times16 would be -1, since it's of type signed short.  So, it's all good.  Negative RPM is covered.
		POSCNT = 0;
		revCounter = 0;
	}
//#endif

	// CH0 corresponds to ADCBUF0. etc...
	// CH0=AN7, CH1=AN0, CH2=AN1, CH3=AN2. 
	// AN0 = CH1 = ADThrottle
	// AN1 = CH2 = ADCurrent1
	// AN2 = CH3 = ADCurrent2
	// AN7 = CH0 = ADTemperature

	ADCurrent1 = ADCBUF2;
	ADCurrent2 = ADCBUF3;
	Ia = ADCurrent1;	// CH2 = ADCurrent1
	Ib_times2 = ADCurrent2;		// CH3 = ADCurrent2

	Ia -= vRef1;  // vRef1 is just a constant found at the beginning of the program, approximately = 512, that changes the current feedback from being centered at 512 to centered at 0.  It's specific to current sensor #1.
	Ib_times2 -= vRef2;  // vRef2 is just a constant found at the beginning of the program, approximately = 512, that changes the current feedback from being centered at 512 to centered at 0.  It's specific to current sensor #2.

	// So, you must change the interval to [-4096, 4096], so as to match the throttle range below to make feedback comparable with commanded current. So...

	Ia <<= 4;	// Ia is now in [-4096, 4096] if it was in  [-256, 256].  In other words, if it was in [-2*LEM Rating, 2*LEM Rating]. 
	Ib_times2 <<= 5;

//	if (!huntingForGoodPIValues && !startNewRotorTest) { 
/*
	throttleSum += ADCBUF1;
	temperatureSum += ADCBUF0;
	batteryCurrentLong = __builtin_mulss(Ia,pdc1) + __builtin_mulss(Ib,pdc2) + __builtin_mulss(-Ia-Ib,pdc3);  // batteryCurrent is in [-4096*sqrt(3)/2, 4096*sqrt(3)/2] = [-3547, 3547].
	batteryCurrentSum += batteryCurrentLong;
	throttleCounter++;
	if (throttleCounter >= 128) {
		throttleCounter = 0;
		throttle = (throttleSum >> 7);  // in [0, 1023].
		temperatureBasePlate = (temperatureSum >> 7); // in [0,1023]
		batteryCurrentNormalized = __builtin_divsd(batteryCurrentSum >> 7,MAX_DUTY);
		throttleSum = 0;
		temperatureSum = 0;
		batteryCurrentSum = 0;
	
		// Is there a throttle fault?
		if (throttle <= savedValues.throttleFaultPosition) {
//			throttleFaultCounter++;
//			if (throttleFaultCounter >= THROTTLE_FAULT_COUNTS) {  // don't worry about clamping it.  If there's a throttle fault, you must turn off the controller to clear it.
			faultBits |= THROTTLE_FAULT;
//			}
		}
//			else {
//				if (throttleFaultCounter > 0) {  // Hurray, no fault, so decrement the fault counter if necessary.
//					throttleFaultCounter--;
//				}
//			}
		// I think I'll work my way left to right.  ThrottleFaultVoltage < ThrottleMaxRegen < ThrottleMinRegen < ThrottleMin < ThrottleMax 
		if (throttle < savedValues.maxRegenPosition) {  //First clamp it below.
			throttle = savedValues.maxRegenPosition;	
		}
		else if (throttle > savedValues.maxThrottlePosition) {	// And clamp it above!  
			throttle = savedValues.maxThrottlePosition;
		}
		if (throttle < savedValues.minRegenPosition) {  // It's in regen territory.  Map it from [savedValues.maxThrottleRegen, savedValues.minThrottleRegen) to [-maxMotorCurrentNormalizedRegen, 0)
			throttle -= savedValues.minRegenPosition;  // now it's in the range [maxThrottleRegen - minThrottleRegen, 0)
			throttle =	-__builtin_divsd(((long)throttle) * maxMotorCurrentNormalizedRegen, savedValues.maxRegenPosition - savedValues.minRegenPosition);
		}
		else if (throttle <= savedValues.minThrottlePosition) { // in the dead zone!
			throttle = 0;
		}
		else { // <= throttle max is the only other option!  Map the throttle from (savedValues.minThrottlePosition, savedValues.maxThrottlePosition] to (0,maxMotorCurrentNormalized]
			throttle -= savedValues.minThrottlePosition;
			throttle = __builtin_divsd(__builtin_mulss(throttle, maxMotorCurrentNormalized), savedValues.maxThrottlePosition - savedValues.minThrottlePosition);
		}
		if (temperatureBasePlate > THERMAL_CUTBACK_START) {  // Force the throttle to cut back.
			temperatureMultiplier = (temperatureBasePlate - THERMAL_CUTBACK_START) >> 3;  // 0 THROUGH 7.
			if (temperatureMultiplier >= 7)
				temperatureMultiplier = 0;
			else {
				// temperatureMultiplier is now 6 to 0 (for 1/8 to 7/8 current)
				temperatureMultiplier = 7 - temperatureMultiplier;
				// temperatureMultiplier is now 1 for 1/8, 2 for 2/8, ..., 7 for 7/8, etc.
			}
		}
		else {
			temperatureMultiplier = 8;	// Allow full throttle.
		}
		IqRefRef = __builtin_mulss(throttle,temperatureMultiplier) >> 3;
		IdRefRef = IqRefRef;
		if (IdRefRef < 0) IdRefRef = -IdRefRef;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Keep battery amps in [-savedValues.maxBatteryAmpsRegen, savedValues.maxBatteryAmps] //////////	
	if (batteryCurrentNormalized > maxBatteryCurrentNormalized) { // maxBatteryCurrentNormalized is positive.  Computed in "ProcessCommand", each time a new savedValues.maxBatteryAmps is found.
		// averageDuty = (pdc1+pdc2+pdc3)/3.  But division is slow.  So, do this instead:
		// averageDuty = 65536 * ((pdc1+pdc2+pdc3)/3) / 65536
		// averageDuty = 21845 * (pdc1+pdc2+pdc3) / 65536
		// averageDuty = (21845 * (pdc1+pdc2+pdc3)) >> 16
		averageDuty = __builtin_mulss(21845, pdc1+pdc2+pdc3) >> 16;	// avoiding divide by 3.  HAHA.  
		if (averageDuty > 0) {
			temp = __builtin_divsd(maxBatteryCurrentNormalized,averageDuty); // 
			if (IqRefRef > temp) {
				IqRefRef = temp;
			}
		}
	}
	else if (batteryCurrentNormalized < -maxBatteryCurrentNormalizedRegen) { // maxRegenCurrent is negative.  Computed in "InitializeThrottleAndCurrentVariables()".
		// averageDuty = (pdc1+pdc2+pdc3)/3.  But division is slow.  So, do this instead:
		// averageDuty = 65536 * ((pdc1+pdc2+pdc3)/3) / 65536
		// averageDuty = 21845 * (pdc1+pdc2+pdc3) / 65536
		// averageDuty = (21845 * (pdc1+pdc2+pdc3)) >> 16
		averageDuty = __builtin_mulss(21845, pdc1+pdc2+pdc3) >> 16;	// avoiding divide by 3.  HAHA.  
		if (averageDuty > 0) {
			temp = -__builtin_divsd(maxBatteryCurrentNormalizedRegen,averageDuty); // This is the first try at IqRefRef.  Later, other values will be calculated based on throttle.  Keep the one closest to zero.
			if (IqRefRef < temp) {
				IqRefRef = temp;  // if it was more negative, then make it smaller in magnitude, 'cuz there's too much friggen current going into the batteries!!!
			}
		}
	}
//	}
//	else {
//		throttleSum = 0;
//		temperatureSum = 0;
//		batteryCurrentSum = 0;
//		throttleCounter = 0;
//	}
	/////////////////////////////////////////////////////////////////////////////////////////////
*/
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Clarke transform:
	//  	First, take the 3 vectors, 120 degrees apart, and add them to
	// 		get a new vector, and project that vector onto the x and y axis.  The x-axis component is called i_alpha.  y-axis component is called i_beta.
	// clarke transform, scaled down by 2/3 to simplify it:
	// i_alpha = i_a
	// 1/sqrt(3) * 2^16 = 37837
	
	i_alpha = Ia;
	if (captureVariable) {
		IdRefRef = IqRefRef = 300;
//		if (startUp == 0) {
		if ((counter10k & 511) == 0) {
			if (NUMBER_OF_DATA_TYPES == 1) {
				largeStorage.data[0][dataCounter] = rGlobal;
			}
			else if (NUMBER_OF_DATA_TYPES == 2) {
				largeStorage.data[0][dataCounter] = rGlobal_filtered;
				largeStorage.data[1][dataCounter] = rotorFluxRPS_times16;//E_beta_times8;//v_beta_volts_times8_filtered_corrected;			
			}
			else if (NUMBER_OF_DATA_TYPES == 3) {
				largeStorage.data[0][dataCounter] = rGlobal;
				largeStorage.data[1][dataCounter] = Vd;//E_beta_times8;//v_beta_volts_times8_filtered_corrected;
				largeStorage.data[2][dataCounter] = Vq;//rotorFluxRPS_times16;//rotorFluxAngle;//temp2;
			}
			dataCounter += 1;
			if (dataCounter >= PI_GOOD_VALUES_ARRAY_SIZE/NUMBER_OF_DATA_TYPES) {
				largeArrayLoaded = 1;
				captureVariable = 0;
				dataCounter = 0;
			}
//		}
		}
	}
	i_beta = __builtin_mulsu((int)(Ib_times2 + Ia), 37837u) >> 16;  // 1/sqrt(3) * (i_a + 2 * Ib).  

	// End of clarke transform.
	////////////////////////////////////////////////////////////////////////////////////////////////
	// "ComputeRotorFluxAngle()" uses Id and Iq, found below.  So, initialize them to something.  I'll have Id start as 0, and Iq start as 0.
	//	ComputeRotorFluxAngle();
	if (huntingForGoodPIValues) {
		rotorFluxAngle = 0;	 // set rotorFluxAngle to zero while tuning the PI loop with a locked rotor.
//		rotorFluxAngleSensorless = 0;	 // set rotorFluxAngle to zero while tuning the PI loop with a locked rotor.
	}
	else {
		ComputeRotorFluxAngle();
//		ComputeRotorFluxAngleSensorless();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Park transform:
	// rotorFluxAngle is in [0, 511].
	// sin(theta + 90 degrees) = cos(theta).
	// I want the 2 angles to be in [0, 511] so I can use the lookup table.
/*	if (savedValues2.sensorless == 1) {
		if (startUp == 1) {
			if (RPS_times16 < 300) {
				rotorFluxAnglePlus90 = ((rotorFluxAngle + 128) & 511);  // To advance 90 degrees on a scale of 0 to 511, you add 128, 
																// and then do "& 511" to make it wrap around if overflow occurred.
				cos_theta_times32768 = _sin_times32768[rotorFluxAnglePlus90];  // 
				sin_theta_times32768 = _sin_times32768[rotorFluxAngle];  // 
			}
			else {
				startUp = 0;
			}
		}
		else {

//			rotorFluxAnglePlus90 = ((rotorFluxAngle + 128) & 511);  // To advance 90 degrees on a scale of 0 to 511, you add 128, 
															// and then do "& 511" to make it wrap around if overflow occurred.
//			cos_theta_times32768 = _sin_times32768[rotorFluxAnglePlus90];  // 
//			sin_theta_times32768 = _sin_times32768[rotorFluxAngle];  // 			
			rotorFluxAnglePlus90 = ((rotorFluxAngleSensorless + 128) & 511);  // To advance 90 degrees on a scale of 0 to 511, you add 128, 
															// and then do "& 511" to make it wrap around if overflow occurred.
			cos_theta_times32768 = _sin_times32768[rotorFluxAnglePlus90];  // 
			sin_theta_times32768 = _sin_times32768[rotorFluxAngleSensorless];  // 			
		}
	}
	else {
*/
		rotorFluxAnglePlus90 = ((rotorFluxAngle + 128) & 511);  // To advance 90 degrees on a scale of 0 to 511, you add 128, 
															// and then do "& 511" to make it wrap around if overflow occurred.
		cos_theta_times32768 = _sin_times32768[rotorFluxAnglePlus90];  // 
		sin_theta_times32768 = _sin_times32768[rotorFluxAngle];  // 
//	}
	// Park Transform:
	// Id = Ialpha*cos(theta) + Ibeta*sin(theta)
	// Iq = -Ialpha*sin(theta) + Ibeta*cos(theta)
	Id = (__builtin_mulss((int)i_alpha, (int)cos_theta_times32768) + __builtin_mulss((int)i_beta, (int)sin_theta_times32768)) >> 15; 	
	Iq = (__builtin_mulss((int)(-i_alpha), (int)sin_theta_times32768) + __builtin_mulss((int)i_beta,(int)cos_theta_times32768)) >> 15; 

	if (faultBits == 0) {
		if (huntingForGoodPIValues) {
			if ((counter10k - delayBetweenPITests) < 1000) {
				IdRef = 0;
				IqRef = 0;
			}
			else {
				IdRef = 0;  // 512 on a scale of 0 to 4096 corresponds to 75 amps for a LEM Hass 300-s.  Because 4096 means 600amps.
				IqRef = 511;//*ampToNormalizedMultiplier;
				//if (IqRef > 4095) IqRef = 4095;
		
				if (piIterationIndex == 0) {
					piIterationIndex++;
				}
				else {
					if (pi_Iq.error < -80) {
						MoveToNextPIValues();
					}
					else if (pi_Iq.error > IqRef + 200) {  //  IqRef is a constant 511.  If it oscillated up to +711, that's bad.  move on.
						MoveToNextPIValues();
					}
					else if (piZeroCrossingIndex == -1 && piIterationIndex > 20) {  // PIZeroCrossingIndex == -1 means it hasn't crossed zero yet.
						MoveToNextPIValues();  // CONVERGENCE TOO SLOW!!!  Move on!
					}
					else if (pi_Iq.error > 80 && piZeroCrossingIndex >= 0) {  // it already crossed zero, but now is way back up again.  This is oscillation.  move on!
						MoveToNextPIValues();
					}
					else {
						if (pi_Iq.error <= 0 && piZeroCrossingIndex == -1) {  // if it's crossing zero for the first time, record the location.
							piZeroCrossingIndex = piIterationIndex;
						}
						piIterationIndex++;
						if (piIterationIndex >= MAX_PI_ITERATIONS) {  // The PI values made it through the rigorous testing.  haha.  Save P in the PI_data array.  Q can be obtained by dividing P by 62.
							largeStorage.PI_data[piGoodValuesIndex] = (int)pi_Iq.P;
							piGoodValuesIndex++;
							if (piGoodValuesIndex >= PI_GOOD_VALUES_ARRAY_SIZE) {
								piGoodValuesArrayLoaded = 1;
								huntingForGoodPIValues = 0;
							}
							else {
								// Now, start over again.  MOve to next PI values.
								MoveToNextPIValues();
							}
						}
					}
					if (pi_Iq.P > MAX_PI_P) {
						piGoodValuesArrayLoaded = 1;  // stop here.  You have gone far enough.
						huntingForGoodPIValues = 0;
					}
				}
			}
		}
		else if (startNewRotorTest) {  // I need this to run for say, 1 second, and then record the RPM.
			IdRefRef = 200;//20*ampToNormalizedMultiplier;
			IqRefRef = 200;//*ampToNormalizedMultiplier;
			//if (IdRef > 4095) IdRef = 4095;
			//if (IqRef > 4095) IqRef = 4095;
			if (counter10k - rotorStartTime > 50000) {  // 5 seconds.
				rotorStartTime = counter10k;
				largeStorage.rotor_data[savedValues2.rotorTimeConstantArrayIndex] = RPS_times16; // record RPM.
				savedValues2.rotorTimeConstantArrayIndex++;
				if (savedValues2.rotorTimeConstantArrayIndex >= ROTOR_TIME_CONSTANT_ARRAY_SIZE) {
					rotorArrayLoaded = 1;
					startNewRotorTest = 0;
					savedValues2.rotorTimeConstantArrayIndex = 0;
					IdRefRef = 0;
					IqRefRef = 0;
				}
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// PI Loop:
	pi_Iq.error = IqRef - Iq;
	pi_Id.error = IdRef - Id;
	
	pi_Iq.errorSum += pi_Iq.error - clampErrorVq;
	pi_Id.errorSum += pi_Id.error - clampErrorVd;
	
	if (IdRef == 0) {
		pi_Id.pwm = 0;
		pi_Id.errorSum = 0;
		pi_Id.error = 0;
		clampErrorVd = 0;
	}
	else {
		pi_Id.pwm = (pi_Id.P * pi_Id.error) + (pi_Id.I*pi_Id.errorSum);
	}
	if (IqRef == 0) {
		pi_Iq.pwm = 0;
		pi_Iq.errorSum = 0;
		pi_Iq.error = 0;
		clampErrorVq = 0;
	}
	else {
		pi_Iq.pwm = (pi_Iq.P * pi_Iq.error) + (pi_Iq.I * pi_Iq.errorSum);
	}

	if (pi_Id.pwm > (MAX_VD_VQ << 13)) {
		pi_Id.pwm = (MAX_VD_VQ << 13);
		faultBits |= PI_OVERFLOW_FAULT;
	}
	else if (pi_Id.pwm < ((-MAX_VD_VQ) << 13)) {
		pi_Id.pwm = ((-MAX_VD_VQ) << 13);		
		faultBits |= PI_OVERFLOW_FAULT;
	}
	if (pi_Iq.pwm > (MAX_VD_VQ << 13)) {
		pi_Iq.pwm = (MAX_VD_VQ << 13);
		faultBits |= PI_OVERFLOW_FAULT;
	}
	else if (pi_Iq.pwm < ((-MAX_VD_VQ) << 13)) {
		pi_Iq.pwm = ((-MAX_VD_VQ) << 13);		
		faultBits |= PI_OVERFLOW_FAULT;
	}

	Vd = pi_Id.pwm >> 13;
	Vq = pi_Iq.pwm >> 13;
	tempVd = Vd;
	tempVq = Vq;

	ClampVdVq();
	clampErrorVd = tempVd - Vd;
	clampErrorVq = tempVq - Vq;
	//	if (maxClampErrorVd < clampErrorVd) maxClampErrorVd = clampErrorVd;
	//	if (minClampErrorVd > clampErrorVd) minClampErrorVd = clampErrorVd;
	//	if (maxClampErrorVq < clampErrorVq) maxClampErrorVq = clampErrorVq;
	//	if (minClampErrorVq > clampErrorVq) minClampErrorVq = clampErrorVq;

	// should I have it go instantly to IqRefRef if IqRefRef is smaller in magnitude?
	if (IqRef < IqRefRef) {
		IqRef += rampRate;  // this is too fast.  I want ot to be a laxidaisical drift back so as to avoid harsh PI loop clamping.
		if (IqRef > IqRefRef) {
			IqRef = IqRefRef;
		}
	}
	else if (IqRef > IqRefRef) {
		IqRef -= rampRate;
		if (IqRef < IqRefRef) {
			IqRef = IqRefRef;
		}
	}

	// should I have it go instantly to IdRefRef if IdRefRef is smaller in magnitude?
	if (IdRef < IdRefRef) {
		IdRef += rampRate;
		if (IdRef > IdRefRef) {
			IdRef = IdRefRef;
		}
	}
	else if (IdRef > IdRefRef) {
		IdRef -= rampRate;
		if (IdRef < IdRefRef) {
			IdRef = IdRefRef;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Inverse Park Transform:
	//	v_alpha_times32768 = (Vd*cos_theta_times32768 - Vq*sin_theta_times32768);	// shift right 15 because sin and cos have been shifted left by 15.
	//	vBeta =  (Vd*sin_theta_times32768 + Vq*cos_theta_times32768) >> 15;	//
	v_alpha_times32768 = __builtin_mulss((int)Vd, (int)cos_theta_times32768) - __builtin_mulss((int)Vq, (int)sin_theta_times32768);
	v_beta = (__builtin_mulss((int)Vd, (int)sin_theta_times32768) + __builtin_mulss((int)Vq, (int)cos_theta_times32768)) >> 15;
	//////////////////////////////////////////////////////////////////////////////////
	// Now do the inverse Clarke transform, with a scaling factor of 3/2 to simplify it.
	//  Va = v_alpha
	//	Vb = 1/2*(-v_alpha + sqrt(3)*vBeta)
	//  Vc = 1/2*(-v_alpha - sqrt(3)*vBeta);
	v_alpha = v_alpha_times32768 >> 15;
	Va = v_alpha; 
	//vBetaSqrt3_times32768 = (56756*vBeta);  // 56756 = sqrt(3)*(2^15).
	vBetaSqrt3_times32768 = __builtin_mulus(56756u, (int)v_beta);  // 56756 = sqrt(3)*(2^15).

	Vb = (-v_alpha_times32768 + vBetaSqrt3_times32768) >> 16;  // scaled up by 2^15, so shift down by 15.  But you also must divide by 2 at the end as part of the inverse clarke.  So, shift down by a total of 16.
	Vc = (-v_alpha_times32768 - vBetaSqrt3_times32768) >> 16;
	///////////////////////////////////////////////////////////////////////////////////
	// SpaceVectorModulation:
	// You now turn Va, Vb, Vc into duties for PDC1, PDC2, PDC3.  

	if (faultBits == 0) {  // check again before poceeding, since there could have been a fault just above.
		SpaceVectorModulation();
	}
	else {
		PDC1 = 0;
		PDC2 = 0;
		PDC3 = 0;
		pi_Id.error = 0l;
		pi_Id.pwm = 0l;  
		pi_Id.errorSum = 0l;
		pi_Iq.error = 0l;
		pi_Iq.pwm = 0l;  
		pi_Iq.errorSum = 0l;		
	}
	elapsedTimeInterrupt = TMR4 - startTimeInterrupt;
}

void MoveToNextPIValues() {
	pi_Id.P += piRatio;
	pi_Id.I += 1;
	pi_Id.error = 0l;
	pi_Id.pwm = 0l;  
	pi_Id.errorSum = 0l;

	pi_Iq.P += piRatio;
	pi_Iq.I += 1;
	pi_Iq.error = 0l;
	pi_Iq.pwm = 0l;  
	pi_Iq.errorSum = 0l;	

	piIterationIndex = 0;
	piZeroCrossingIndex = -1;
	IdRef = 0;
	IqRef = 0;
	delayBetweenPITests = counter10k;	
}

void InitPIStruct() {
	// **3125, 50**
	// 120v:  1875, 30
	// 

	pi_Id.P = (long)savedValues.Kp_Id;
	pi_Id.I = (long)savedValues.Ki_Id;
	pi_Id.error = 0l;
	pi_Id.pwm = 0l;  
	pi_Id.errorSum = 0l;

	pi_Iq.P = (long)savedValues.Kp_Iq;
	pi_Iq.I = (long)savedValues.Ki_Iq;
	pi_Iq.error = 0l;
	pi_Iq.pwm = 0l;  
	pi_Iq.errorSum = 0l;
}

void ComputeRotorFluxAngle() {
//	static unsigned int rotorFluxAngle_times128 = 0;  // For fine control.
//	static long magCurrChange = 0;
//	static long slipSpeedNumerator = 0;
//	static int slipSpeedRPS_times16 = 0;
//	static int rotorFluxRPS_times16 = 0;
//	static int angleChange_times128 = 0;
//	static long magnetizingCurrentFine = 0;
//	static int magnetizingCurrent = 0;

//;	 Physical form of equations:
//;  Magnetizing current (amps):
//;     Imag = Imag + (fLoopPeriod/fRotorTmConst)*(Id - Imag)
//;
//;  Slip speed in RPS:
//;     VelSlipRPS = (1/fRotorTmConst) * (Iq/Imag) / (2*pi)
//;
//;  Rotor flux speed in RPS:
//;     VelFluxRPS = iPoles * VelMechRPS + VelSlipRPS
//;
//;  Rotor flux angle (radians):
//;     AngFlux = AngFlux + fLoopPeriod * 2 * pi * VelFluxRPS

// ****For divide, use this:  	int quot =	 __builtin_divsd(long numerator, int denominator);****
// ****For multiply, use this:  long prod =   __builtin_mulus(unsigned left,  int right);****  or muluu, or mulsu, or mulss.

//; 1.  Magnetizing current (amps):
//;     Imag = Imag + (fLoopPeriod/fRotorTmConst)*(Id - Imag)
//      rotorTimeConstantArray[]'s entries have been scaled up by 2^18 to give more resolution for the rotor time constant.  I scaled by 18, because that allowed incrementing rotor time constant by 0.01 seconds.
//	magnetizingCurrent += ((rotorTimeConstantArray1[savedValues2.rotorTimeConstantArrayIndex] * (Id - magnetizingCurrent))) >> 18;
///////////////////////////////////////////////////////////////////////////
	magCurrChange = __builtin_mulus((unsigned int)rotorTimeConstantArray1[savedValues2.rotorTimeConstantArrayIndex], (int)(Id - magnetizingCurrent));
	if (magCurrChange > 0) {
		if (magnetizingCurrentFine < MAX_LONG_INT - magCurrChange) {
			magnetizingCurrentFine += magCurrChange;
		}
		else {
			magnetizingCurrentFine = MAX_LONG_INT;
			faultBits |= MC_OVERFLOW_FAULT;
		}
	}
	else {
		if (magnetizingCurrentFine > -MAX_LONG_INT - magCurrChange) {
			magnetizingCurrentFine += magCurrChange;
		}
		else {
			magnetizingCurrentFine = -MAX_LONG_INT;
			faultBits |= MC_OVERFLOW_FAULT;
		}
	}
	magnetizingCurrent = (int)(magnetizingCurrentFine >> 18);
	magnetizingCurrentAmps_times8 = __builtin_mulss((int)magnetizingCurrent, (int)currentSensorAmpsPerVoltTimes5) >> 11;  // 5/4*currentSensorAmpsPerVolt / 4096 * #ticks = amps.  So amps*8 cancels it down to >>11.

////////////////////////////////////////////////////////////////////////////
//; 2. To Compute Slip speed in RPS:
//;    VelSlipRPS = (1/fRotorTmConst) * (Iq/Imag) / (2*pi)
//     rotorTimeConstantArray2[] entries are 1/fRotorTmConst * 1/(2*pi) * 2^11.  I couldn't scale any higher than 2^11 so as to keep them integers. (this is not 100% true now.  haha.)
//	   VelSlipRPS = (ARRAY[] * Iq) / Imag.
//	   Let slipSpeedNumerator = (ARRAY[] * Iq).  Then, do the bit shift down first, and the divide by magnetizing current afterwards to prevent the loss of resolution.
/////////////////////////////////////////////////////////////////////////////////////////////////////	
	if (magnetizingCurrent == 0) {
		return;  // there is no rotor flux angle, since there is no field.  So, keep the angle the same as it was before??
	}
	else if (Iq == 0) {
		slipSpeedRPS_times16 = 0;  // If the numerator is 0, the whole thing is zero.
	}
	else {  // magnetizingCurrent != 0, slipSpeedNumerator != 0
		slipSpeedNumerator = __builtin_mulus((unsigned int)rotorTimeConstantArray2[savedValues2.rotorTimeConstantArrayIndex], (int)Iq) >> 7; // Must scale down by 2^11 if you want units to be rev/sec.  But that's too grainy.  So, let's only scale down by 2^7 so you get slip speed in rev/sec * 16, rather than just rev/sec
		if (magnetizingCurrent > 0) {
			if (slipSpeedNumerator > 0L) {
				if (slipSpeedNumerator > __builtin_mulss((int)magnetizingCurrent,(int)MAX_SLIP_SPEED_RPS_TIMES16)) {
					slipSpeedRPS_times16 = MAX_SLIP_SPEED_RPS_TIMES16;  // must be positive slip speed
				}
				else {
					slipSpeedRPS_times16 = __builtin_divsd((long)slipSpeedNumerator, (int)magnetizingCurrent);  // must be positive slip speed.
				}
			}
			else { // if (slipSpeedNumerator < 0).  It can't be zero, since that case was already accounted for.
				if (-slipSpeedNumerator > __builtin_mulss((int)magnetizingCurrent,(int)MAX_SLIP_SPEED_RPS_TIMES16)) {
					slipSpeedRPS_times16 = -MAX_SLIP_SPEED_RPS_TIMES16;  // must end with negative slip speed.
				}
				else {
					slipSpeedRPS_times16 = __builtin_divsd((long)slipSpeedNumerator, (int)magnetizingCurrent);  // this is negative result.
				}
			}
		}
		else {  // magnetizingCurrent < 0
			if (slipSpeedNumerator > 0) {  // POS / NEG = NEG.
				if (slipSpeedNumerator > __builtin_mulss((int)(-magnetizingCurrent),(int)MAX_SLIP_SPEED_RPS_TIMES16)) {  //
					slipSpeedRPS_times16 = -MAX_SLIP_SPEED_RPS_TIMES16;  // must be negative slip speed.  pos/neg = neg.
				}
				else {
					slipSpeedRPS_times16 = __builtin_divsd((long)slipSpeedNumerator, (int)magnetizingCurrent);  // must be negative slip speed.
				}
			}
			else {  // slipSpeedNumerator < 0, magnetizingCurrent < 0.  So, slipSpeed will be positive below.
				if (slipSpeedNumerator < __builtin_mulss((int)magnetizingCurrent,(int)MAX_SLIP_SPEED_RPS_TIMES16)) {  // if it's more negative than the right hand side...
					slipSpeedRPS_times16 = MAX_SLIP_SPEED_RPS_TIMES16;  // must end with positive slip speed.  neg / neg = pos.
				}
				else {
					slipSpeedRPS_times16 = __builtin_divsd((long)slipSpeedNumerator, (int)magnetizingCurrent);  // this is positive result.
				}
			}
		}
	}
///////////////////////////////////////////////////////////////////////////////////

//; 3. Rotor flux speed in RPS:
//;    VelFluxRPS = numPolePairs * VelMechRPS + VelSlipRPS
//     RPS_times16 was gloriously found in the main interrupt. It is in [-3200, 3200], which means [-12000RPM, 12000RPM].  I will make sure that normal driving is positive rpm.
//     savedValues2.numberOfPolePairs is 2 in the case of my motor, because the RPM is listed as 1700 with 60 Hz 3 phase input.  1 pole pair would list the rpm as around 3400 with 60 Hz 3 phase input.
//	rotorFluxRPS_times16 = savedValues2.numberOfPolePairs * RPS_times16 + slipSpeedRPS_times16;  // There's no danger of integer overflow for 1 or 2 pole pairs, so just do normal multiply.

		rotorFluxRPS_times16 = savedValues2.numberOfPolePairs*RPS_times16 + slipSpeedRPS_times16;  // There's no danger of integer overflow, so just do normal multiply.  Larger number of pole pairs means lower rpm, so it all evens out.

		if (RPS_times16 > maxRPS_times16) {  // You are about to friggen grenade your motor.  Slow down.
			faultBits |= OVERSPEED_FAULT;
		}
		else if (RPS_times16 < -maxRPS_times16) {
			faultBits |= OVERSPEED_FAULT;
		}

//;  Rotor flux angle (radians):
//;     AngFlux = AngFlux + fLoopPeriod * 2 * pi * VelFluxRPS
//;  Rotor flux angle (0 to 511 ticks):
//      AngFlux = AngFlux + fLoopPeriod * 512 * VelFluxRPS.  
//   Now, I don't have VelFluxRPS.  Too darn grainy.  I found rotorFluxRPS_times16 above.  So.....
//      AngFlux = AngFlux + fLoopPeriod * 32 * rotorFluxRPS_times16, because 32 * 16 = 512.
//   OK, fLoopPeriod = 0.0001 sec.  I don't want to divide here, so let's do a trick that is much faster.
//   0.0001sec * 32 * 2^24 = 53687.09.  So, I'll just multiply by 53687, and shift down by 2^24 afterwards.  Actually, let's keep 2^7 worth of extra resolution, because angleChange was too grainy before.
//	angleChange_times128 = (53687u * rotorFluxRPS_times16) >> 17;  // must shift down by 24 eventually, but let's keep a higher resolution here.  So, only shift down by 17.  Keeping 7 bits.
	angleChange_times128 = __builtin_mulus(53687u, (int)rotorFluxRPS_times16) >> 17;  // must shift down by 24 eventually, but let's keep a higher resolution here.  So, only shift down by 17.  Keeping 7 bits.
	// angleChange_times128 must be in [-5242, 5242] assuming all the clamping I'm doing above.
	rotorFluxAngle_times128 += (unsigned)angleChange_times128;  // if it overflows, so what.  it will wrap back around.  To go from [0,65536] --> [0,512], divide by by 128.  higher resolution rotor flux angle saved here.
	oldRotorFluxAngle = rotorFluxAngle;
	rotorFluxAngle = (rotorFluxAngle_times128 >> 7);
}

void ClampVdVq() {

	static int VdPos, VqPos, small, big;
	static int i;
	static unsigned int fastDist;
	static unsigned int r;
	static unsigned int scale;
	static int IdRefNew = 0;
	static int IqRefNew = 0;

	if (Vd == 0 && Vq == 0) return;  // Forgetting to do this caused me a lot of annoyances.  Divide by zero danger below.

	if (Vd < 0) VdPos = -Vd;
	else VdPos = Vd;
	if (Vq < 0) VqPos = -Vq;
	else VqPos = Vq;

	if (VqPos < VdPos) {
		small = VqPos;
		big = VdPos;
	}
	else {
		small = VdPos;
		big = VqPos;
	}
	i = __builtin_divsd(((long)small) << 10, (int)big);   // small * 1024 / big gives an index from 0 to 1024 inclusive.
	fastDist = (unsigned int)((small + big - (small >> 1) - (small >> 2)) + (small >> 4));
	// fastDist must be in [0, 19687*2??]
	r = (__builtin_muluu((unsigned)distCorrection[i], (unsigned)fastDist) >> 15);  // scale down by 2^15 since distCorrection was scaled up by 2^15.
	rGlobal = r;

	rGlobal_filtered_times65536   	//= (15*rGlobal_filtered_times65536/65536 + 1*rGlobal) / 16 * 65536;
							 		//= (16*rGlobal_filtered_times65536/65536 - 1*rGlobal_filtered + 1*rGlobal) / 16 * 65536
									  = ((rGlobal_filtered_times65536 >> 12) - ((long)rGlobal_filtered) + ((long)rGlobal)) << 12;
	rGlobal_filtered = rGlobal_filtered_times65536 >> 16;

	if (r > R_MAX) {
		scale = __builtin_divud((unsigned long)R_MAX_TIMES_65536, (unsigned)r);
		small = (int)(__builtin_mulsu((int)small, (unsigned)scale) >> 16);
		big = (int)(__builtin_mulsu((int)big, (unsigned)scale) >> 16);
		IqRefNew = (int)(__builtin_mulsu((int)IqRef, (unsigned)scale) >> 16);
		IdRefNew = (int)(__builtin_mulsu((int)IdRef, (unsigned)scale) >> 16);  // what if I only did this to IdRef instead of both of them?  Then it would be field weakening.  I'll compare later.
		IqRef = IqRefNew + (__builtin_mulus(3,IqRef - IqRefNew) >> 2);
		IdRef = IdRefNew + (__builtin_mulus(3,IdRef - IdRefNew) >> 2);
	}
	else return;
	
	if (VqPos < VdPos) {
		if (Vq < 0) {
			Vq = -small;
		}
		else {
			Vq = small;
		}
		if (Vd < 0) {
			Vd = -big;
		}
		else {
			Vd = big;
		}
	}
	else {  // VdPos <= VqPos.  Now, Vd goes with small, and Vq goes with big.  watch the sign though.
		if (Vd < 0) {
			Vd = -small;
		}
		else {
			Vd = small;
		}
		if (Vq < 0) {
			Vq = -big;
		}
		else {
			Vq = big;
		}
	}
}

void SpaceVectorModulation() { 
	// faster way:
	if (Va <= Vb) {
		// 3 possibilities:
		// Va <= Vb <= Vc
		// Vc <= Va <= Vb
		// Va <= Vc <= Vb
		if (Va <= Vc) {  // Va is smallest.
			pdc1 = 0;
			pdc2 = Vb - Va;
			pdc3 = Vc - Va;
		}
		else { // Vc is smallest
			pdc1 = Va-Vc;
			pdc2 = Vb-Vc;
			pdc3 = 0;
		}
	}
	else {
		// 3 possibilities:
		// Vb <= Va <= Vc
		// Vb <= Vc <= Va
		// Vc <= Vb <= Va
		if (Vb <= Vc) { // Vb is smallest.
			pdc1 = Va-Vb;
			pdc2 = 0;
			pdc3 = Vc-Vb;
		}
		else {  // Vc is smallest.
			pdc1 = Va-Vc;
			pdc2 = Vb-Vc;
			pdc3 = 0;			
		}
	}

// Equivalent slower code below //////////////////
/*
	if (Vc <= Va && Vc <= Vb) {  // Vc is smallest
		// Q1, Q2
		pdc1 = Va-Vc;
		pdc2 = Vb-Vc;
		pdc3 = 0;
	}
	else if (Va <= Vb && Va >= Vc) { // Va is smallest 
		// Q3, Q4
		pdc1 = 0;
		pdc2 = Vb - Va;
		pdc3 = Vc - Va;
	}
	else { // Vb is smallest.
		// Q5, Q6
		pdc1 = Va-Vb;
		pdc2 = 0;
		pdc3 = Vc-Vb;
	}
*/
	if (pdc1 > MAX_DUTY) {
		pdc1 = MAX_DUTY;
	}
	if (pdc2 > MAX_DUTY) {
		pdc2 = MAX_DUTY;
	}
	if (pdc3 > MAX_DUTY) {
		pdc3 = MAX_DUTY;
	}
	// pdc1, 2, and 3 can never be negative because of above.  So, don't bother to test that.
	PDC1 = pdc1;
	PDC2 = pdc2;
	PDC3 = pdc3;
}

void ClearAllFaults() {
	ClearDesatFault();
	ClearFlipFlop();
}

void ClearFlipFlop() {
	O_LAT_CLEAR_FLIP_FLOP = 0;  // Really like 100nS would be enough.
	Delay1uS();
	O_LAT_CLEAR_FLIP_FLOP = 1;	
}

void ClearDesatFault() {	// reset must be pulled low for at least 1.2uS.
	int i = 0;
	O_LAT_CLEAR_DESAT = 0; 	// FOD8316 Datasheet says low for at least 1.2uS.  But then the stupid fault signal may not be cleared for 20 whole uS!
	Delay1uS(); Delay1uS(); Delay1uS();
	O_LAT_CLEAR_DESAT = 1;

	for (i = 0; i < 30; i++) {  // Now, let's waste more than 30uS waiting around for the desat fault signal to be cleared.
		Delay1uS();
	} // 30uS better be long enough!
}
void Delay1uS() {  // Assuming 30MIPs.
	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop();
	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop();
}

void InitTimers() {
	T1CON = 0;  // Make sure it starts out as 0.
	T1CONbits.TCKPS = 0b11;  // prescale of 256.  So, timer1 will run at 58593.75 * 2Hz if Fcy is 30.000MHz.
	PR1 = 0xFFFF;  // 
	T1CONbits.TON = 1; // Start the timer.

	T2CONbits.T32 = 1;  // 32 bit mode.
	T2CONbits.TCKPS = 0b11;  // 1:1 prescaler.
	T2CONbits.TCS = 0;  	// use internal 15MHz Fcy for the clock.
	PR3 = 0x0FFFF;		// HIGH 16 BITS.
	PR2 = 0x0FFFF;		// low 16 bits.
	// Now, TMR3:TMR2 makes up the 32 bit timer, running at 58.6KHz.

	T4CONbits.T32 = 1;  // 32 bit mode.
	T4CONbits.TCKPS = 0b00;  // 1:1 prescaler.
	T4CONbits.TCS = 0;  	// use internal 15MHz Fcy for the clock.
	PR5 = 0x0FFFF;		// HIGH 16 BITS.
	PR4 = 0x0FFFF;		// low 16 bits.

	T2CONbits.TON = 1;	// Start the timer.
	T4CONbits.TON = 1;	// Start the timer.

	TMR3 = 0;  	// Timer3:Timer2 high word
	TMR5 = 0;	// Timer5:Timer4 high word

	TMR2 = 0;  	// Timer3:Timer2 low word
	TMR4 = 0;	// Timer5:Timer4 low word
}	

// Assuming a 30MHz clock, one tick is 1/(58594*2) seconds.
void Delay(unsigned int time) {
	unsigned int temp;
	temp = TMR1;	
	while (TMR1 - temp < time) {
		ClrWdt();
	}
}
void DelaySeconds(int time) {
	int i;
	for (i = 0; i < time; i++) { 
		Delay(58594u);  // 0.5 second.
		Delay(58594u);  // 0.5 second.
	}
}
void DelayTenthsSecond(int time) {
	int i;
	for (i = 0; i < time; i++) { 
		Delay(5859*2);  // 58594*2 ticks in Delay is 1 second.  So, 1/10 of that.
	}
}

void InitCNModule() {
	CNEN1bits.CN0IE = 1;  // overcurrent fault.
	CNEN1bits.CN1IE = 1;  // desat fault.

	CNPU1bits.CN0PUE = 0; // Make sure internal pull-up is turned OFF on CN0.
	CNPU1bits.CN1PUE = 0; // Make sure internal pull-up is turned OFF on CN1.

	_CNIF = 0;  // Clear change notification interrupt flag just to make sure it starts cleared.
	_CNIP = 3;  // Set the priority level for interrupts to 3.
	_CNIE = 1;  // Make sure interrupts are enabled.
}

void __attribute__((__interrupt__, auto_psv)) _CNInterrupt(void) {
	IFS0bits.CNIF = 0;  // clear the interrupt flag.

	if (I_PORT_DESAT_FAULT == 0) {  // It just became 0 like 2Tcy's ago.
		faultBits |= DESAT_FAULT;
	}
	if (I_PORT_OVERCURRENT_FAULT == 0) {
		faultBits |= OVERCURRENT_FAULT;
	}
}

void InitIORegisters() {
	I_TRIS_THROTTLE = 1;		// 1 means configure as input.  A/D throttle.
	I_TRIS_CURRENT1	= 1;		// 1 means configure as input.  A/D current 1.
	I_TRIS_CURRENT2	= 1;		// 1 means configure as input.  A/D current 2.
	I_TRIS_INDEX	= 1;		// 1 means configure as input.  encoder index.  1 pulse per revolution.
	I_TRIS_QEA		= 1;		// 1 means configure as input.  encoder QEA
	I_TRIS_QEB		= 1;		// 1 means configure as input.  encoder QEB
	O_TRIS_CLEAR_FLIP_FLOP = 0;	// 0 means configure as output. clear flip flop.
	O_LAT_CLEAR_FLIP_FLOP = 1;  // bring LOW, then HIGH to clear the flip flop.

	_TRISF0 = 0;				// DEBUGGING PIN.  configure as output.
	_LATF0 = 0;					// initialize it as low.

	I_TRIS_TEMPERATURE = 1;	// 1 means configure as input.  A/D temperatureBasePlate.
	I_TRIS_REGEN_THROTTLE = 1;	// 1 means configure as input.  A/D regen throttle.
	I_TRIS_DESAT_FAULT = 1;		// 1 means configure as input.  desat fault.
	I_TRIS_OVERCURRENT_FAULT = 1;	// 1 means configure as input. overcurrent fault.
	I_TRIS_UNDERVOLTAGE_FAULT = 1;	// 1 means configure as input.  undervoltage fault.
	O_TRIS_LED = 0; 			// 0 means configure as output.  Status LED.
	O_LAT_LED = 1; 				 // high means turn ON the LED.
	O_TRIS_PRECHARGE_RELAY = 0;	// 0 means configure as output.  precharge relay control.
	O_LAT_PRECHARGE_RELAY = 0;	// HIGH means turn ON the precharge relay.
	I_TRIS_GLOBAL_FAULT	= 1;	
	O_TRIS_CONTACTOR = 0;		// 
	O_LAT_CONTACTOR = 0;	
	
	O_TRIS_CLEAR_DESAT = 0;	
	O_LAT_CLEAR_DESAT = 1;		// Bring low then high to clear desat.
	
	O_TRIS_PWM_3H = 0;		// 0 means configure as output.
	O_LAT_PWM_3H = 0;		// Low means OFF.
	
	O_TRIS_PWM_3L = 0;		// 0 means configure as output.
	O_LAT_PWM_3L = 0;		// Low means OFF.
			
	O_TRIS_PWM_2H = 0;		// 0 means configure as output.
	O_LAT_PWM_2H = 0;		// Low means OFF.
		
	O_TRIS_PWM_2L = 0;		// 0 means configure as output.
	O_LAT_PWM_2L = 0;		// Low means OFF.
	
	O_TRIS_PWM_1H = 0;		// 0 means configure as output.
	O_LAT_PWM_1H = 0;		// Low means OFF.
	
	O_TRIS_PWM_1L = 0;		// 0 means configure as output.
	O_LAT_PWM_1L = 0;		// Low means OFF.
	
	ADPCFG = 0b1111111001111000;  // 0 is analog.  1 is digital.
}

void InitADAndPWM() {
    ADCON1bits.ADON = 0; // Pg. 416 in F.R.M.  Under "Enabling the Module".  Turn it off for a moment.

	// PWM Initialization

	PTPER = 1474;	// 29,491,200/((PTPER + 1)*2) = 9997Hz. 
	PDC1 = 0;
	PDC2 = 0;
	PDC3 = 0;

	PWMCON1 = 0b0000000000000000; 			// Pg. 339 in FRM.
	PWMCON1bits.PEN1H = 1; // enabled. Enables pwm1h.
	PWMCON1bits.PEN1L = 1; // enabled. Enables pwmll.
	PWMCON1bits.PEN2H = 1; // enabled.
	PWMCON1bits.PEN2L = 1; // enabled.
	PWMCON1bits.PEN3H = 1; // enabled.
	PWMCON1bits.PEN3L = 1; // enabled. 
	
	PWMCON2 = 0;

    DTCON1 = 0;     		// Pg. 341 in Family Reference Manual. 
	DTCON1bits.DTAPS = 0b10;  // deadtime unit A prescale is 4*Tcy.
	DTCON1bits.DTA = 15;  	// 15 CLOCK periods.  That is 4*15*Tcy = 2uS of dead time assuming 30MHz.

//    FLTACON = 0b0000000010000001; // Pg. 343 in Family Reference Manual. The Fault A input pin functions in the cycle-by-cycle mode.

	PTCON = 0x8002;         // Pg. 337 in FRM.  Enable PWM for center aligned operation.
							// PTCON = 0b1000000000000010;  Pg. 337 in Family Reference Manual.
							// PTEN = 1; PWM time base is ON
							// PTSIDL = 0; PWM time base runs in CPU Idle mode
							// PTOPS = 0000; 1:1 Postscale
							// PTCKPS = 00; PWM time base input clock period is TCY (1:1 prescale)
							// PTMOD = 10; PWM time base operates in a continuous up/down counting mode

	// SEVTCMP: Special Event Compare Count Register 
    // Phase of ADC capture set relative to PWM cycle: 0 offset and counting up
    SEVTCMP = 2;        // Cannot be 0 -> turns off trigger (Missing from doc)
						// SEVTCMP = 0b0000000000000010;  Pg. 339 in Family Reference Manual.
						// SEVTDIR = 0; A special event trigger will occur when the PWM time base is counting upwards and...
						// SEVTCMP = 000000000000010; If SEVTCMP == PTMR<14:0>, then a special event trigger happens.


	// ============= ADC - Measure 
	// ADC setup for simultanous sampling
	// AN0 = CH1 = Throttle;
	// AN1 = CH2 = Current1;
	// AN2 = CH3 = Current2;
	// AN8 = CH0 = regen throttle;	// trade between these 2
	// AN7 = CH0 = Temperature;		// trade between these 2.

	ADCON1 = 0;  // Starts this way anyway.  But just to be sure.   

    ADCON1bits.FORM = 0;  // unsigned integer in the range 0-1023
    ADCON1bits.SSRC = 0b011;  // Motor Control PWM interval ends sampling and starts conversion

    // Simultaneous Sample Select bit (only applicable when CHPS = 01 or 1x)
    // Samples CH0, CH1, CH2, CH3 simultaneously (when CHPS = 1x)
    // Samples CH0 and CH1 simultaneously (when CHPS = 01)
    ADCON1bits.SIMSAM = 1; 
 
    // Sampling begins immediately after last conversion completes. 
    // SAMP bit is auto set.
    ADCON1bits.ASAM = 1;  

	ADCON2 = 0; // Pg. 407 in F.R.M.
    // Pg. 407 in F.R.M.
    // Samples CH0, CH1, CH2, CH3 simultaneously when CHPS = 1x
    ADCON2bits.CHPS = 0b10; // VCFG = 000; This selects the A/D High voltage as AVdd, and A/D Low voltage as AVss.
						 // SMPI = 0000; This makes an interrupt happen every time the A/D conversion process is done (for all 4 I guess, since they happen at the same time.)
						 // ALTS = 0; Always use MUX A input multiplexer settings
						 // BUFM = 0; Buffer configured as one 16-word buffer ADCBUF(15...0)


 	ADCON3 = 0; // Pg. 408 in F.R.M.
    // Pg. 408 in F.R.M.
    // A/D Conversion Clock Select bits = 16 * Tcy.  (31+1) * Tcy/2 = 16*Tcy.
	// The A/D conversion of 4 simultaneous conversions takes 4*12*A/D clock cycles.  The A/D clock is selected to be 4*Tcy.
    // So, it takes about 4*12*16*Tcy to complete 4 A/D conversions. That's 51.2uS. The pwm period is 100uS, since it's 10kHz.
	ADCON3bits.ADCS = 31;  // 16Tcy.


    // ADCHS: ADC Input Channel Select Register 
    ADCHS = 0; // Pg. 409 in F.R.M.

    // ADCHS: ADC Input Channel Select Register 
    // Pg. 409 in F.R.M.
    // CH0 positive input is AN7, temperatureBasePlate.
    ADCHSbits.CH0SA = 7;
    	
	// CH1 positive input is AN0, CH2 positive input is AN1, CH3 positive input is AN2.
	ADCHSbits.CH123SA = 0;

	// CH0 negative input is Vref-.
	ADCHSbits.CH0NA = 0;

	// CH1, CH2, CH3 negative inputs are Vref-, which is AVss, which is Vss.  haha.
    ADCHSbits.CH123NA = 0;

    // ADCSSL: ADC Input Scan Select Register 
    ADCSSL = 0; // Pg. 410 F.R.M.
				// I think it sets the order that the A/D inputs are done.  But I'm doing 4 all at the same time, so set it to 0?


    // Turn on A/D module
    ADCON1bits.ADON = 1; // Pg. 416 in F.R.M.  Under "Enabling the Module"
						 // ** It's important to set all the bits above before turning on the A/D module. **
						 // Now the A/D conversions start happening once ADON == 1.
	_ADIP = 4;			 // A/D interrupt priority set to 4.  Default is 4.
	IEC0bits.ADIE = 1;	 // Enable interrupts to happen when a A/D conversion is complete. Pg. 148 of F.R.M.  	

	// QEICON starts as all zeros.
	// The Quadrature Encoder Interface.  This is for enabling the encoder.
	QEICONbits.QEIM = 0b111; 	// enable QEI x4 mode with position counter reset by MAXCNT.  
	QEICONbits.PCDOUT = 1; 	 	// Position Counter Direction Status Output Enable (QEI logic controls state of I/O pin)
	QEICONbits.POSRES = 0;		// Position counter is not reset by index pulse. But there's no index pulse in my case.  haha. This is ignored in this situation.
	QEICONbits.SWPAB = 0;		// don't swap QEA and QEB inputs.

	DFLTCONbits.CEID = 1; 		// Interrupts due to position count errors disabled
	DFLTCONbits.QEOUT = 1; 		// Digital filter outputs enabled.  QEA, QEB. 0 means normal pin operation.
	DFLTCONbits.QECK = 0b011; 	// clock prescaler of 16. So, QEA or QEB must be high or low for a time of 16*3 Tcy's. 
								// Fcy = 30MHz.  3*16Tcy is the minimum pulse width required for QEA or QEB to change from high to low or low to high.
								// You can do at most 30,000,000/(3*16) = 625,000 of those pulses per second.  So, you can have at most
								// 625,000 * 4 clock counts per second, since resolution has been multiplied by 4 (QEA and QEB up and down transitions all cause a pulse to be seen by the microcontroller..)  
								//  That means the maximum detectable RPM is:
								// x rev/sec * 2096 clockCounts/Rev = 625,000 * 4 clockCounts/sec.  ASSUMING A 512 tick per revolution encoder like I"m using...
								// So, x = 1220 rev/sec.
								// 
	DFLTCONbits.IMV = 0b00;  	// INDEX pulse happens when QEA is low.  Irrelevant.  I'm not using the index pulse for the AC induction motor.

	MAXCNT = 0xFFFF; 	// reset the counter each time it reaches maxcnt.  It will reset anyway won't it?  I mean, after 0xFFFF is 0x0000!  haha.
						// Use this for the AC controller.  It's easier to measure speed of rotor with this setting, if there's no INDEX signal on the encoder.
	POSCNT = 0;  // How many ticks have gone by so far?  Starts out as zero anyway.  It's safe to write to it though.

	PDC1 = 0;
	PDC2 = 0;
	PDC3 = 0;
}

// I know I should disable interrupts here, but I don't want to affect interrupt timing.
void FetchRTData() {
	RTData.throttle = throttle;  // in [-4096, 4096]
	RTData.temperatureBasePlate = temperatureBasePlate;  // in [0, 726] or something like that.  726 means about 85degC.

	RTData.IdRef = IdRef;	// range is [-4096, 4096]
	RTData.Id = Id;			// range is [-4096, 4096]
	RTData.IqRef = IqRef;  	// range is [-4096, 4096]
	RTData.Iq = Iq;			// range is [-4096, 4096]
	RTData.pdc1 = pdc1;
	RTData.pdc2 = pdc2;
	RTData.pdc3 = pdc3;
	RTData.RPS_times16 = RPS_times16;
	RTData.batteryCurrentNormalized = batteryCurrentNormalized;
	RTData.faultBits = faultBits;	
}

void EESaveValues() {  // save the new stuff.
	int i = 0;
	EEDataInRam1[0] = savedValues.Kp_Id;					// Id proportional gain
	EEDataInRam1[1] = savedValues.Ki_Id;						// Id integreal gain
	EEDataInRam1[2] = savedValues.Kp_Iq;		// throttle low voltage (foot off pedal)
	EEDataInRam1[3] = savedValues.Ki_Iq;		// throttle high voltage (full throttle)
	EEDataInRam1[4] = savedValues.currentSensorAmpsPerVolt;		// throttle fault voltage.  Too low of voltage (to protect from disconnected throttle.
	EEDataInRam1[5] = savedValues.maxRegenPosition;		// gain for actual throttle position
	EEDataInRam1[6] = savedValues.minRegenPosition;			// gain for pwm (voltage)
	EEDataInRam1[7] = savedValues.minThrottlePosition;				// 0-8.  8 means zero seconds to max current. 7 means 1 second to max current ... 0 means 8 seconds to max current.
	EEDataInRam1[8] = savedValues.maxThrottlePosition;			// real time data period
	EEDataInRam1[9] = savedValues.throttleFaultPosition;	// motor overspeed threshold
	EEDataInRam1[10] = savedValues.maxBatteryAmps;	// motor overspeed fault time, in units of about 1/128 sec.
	EEDataInRam1[11] = savedValues.maxBatteryAmpsRegen;		// battery amps limit.  Unit is amperes. Must be <= maxMotorAmps 
	EEDataInRam1[12] = savedValues.maxMotorAmps;			// precharge time in 0.1 second increments
	EEDataInRam1[13] = savedValues.maxMotorAmpsRegen;	// motor current must be > motor_sc_amps to calculate motor speed.  Units are amperes.
	EEDataInRam1[14] = savedValues.prechargeTime;		// motor amps limit.  Unit is amperes.  Must be >= maxBatteryAmps
	EEDataInRam1[15] = 0;

	EEDataInRam2[0] = savedValues2.rotorTimeConstantArrayIndex;
	EEDataInRam2[1] = savedValues2.numberOfPolePairs;
	EEDataInRam2[2] = savedValues2.maxRPM;
	EEDataInRam2[3] = savedValues2.throttleType;
	EEDataInRam2[4] = savedValues2.encoderTicks;
	EEDataInRam2[5] = savedValues2.statorResistance_times1024;
	EEDataInRam2[6] = savedValues2.statorInductance_times1024;
	EEDataInRam2[7] = savedValues2.packVoltage;
	EEDataInRam2[8] = savedValues2.sensorless;
	EEDataInRam2[9] = 0;
	EEDataInRam2[10] = 0;
	EEDataInRam2[11] = 0;
	EEDataInRam2[12] = 0;
	EEDataInRam2[13] = 0;
	EEDataInRam2[14] = 0;
	EEDataInRam2[15] = 0;
	for (i = 0; i < 15; i++) {
		EEDataInRam1[15] += EEDataInRam1[i];	// compute checksum.
		EEDataInRam2[15] += EEDataInRam2[i];
	}

    _erase_eedata(EE_addr1, _EE_ROW);  // 
    _wait_eedata();  // #define _wait_eedata() { while (NVMCONbits.WR); }
	ClrWdt();
    _erase_eedata(EE_addr2, _EE_ROW);  // 
    _wait_eedata();  // #define _wait_eedata() { while (NVMCONbits.WR); }
	ClrWdt();
    _erase_eedata(EE_addr3, _EE_ROW);  // 
    _wait_eedata();  // #define _wait_eedata() { while (NVMCONbits.WR); }
	ClrWdt();
    _erase_eedata(EE_addr4, _EE_ROW);  // 
    _wait_eedata();  // #define _wait_eedata() { while (NVMCONbits.WR); }
	ClrWdt();
    // Write a row to EEPROM from array "EEDataInRam1"
    _write_eedata_row(EE_addr1, EEDataInRam1); // first copy of savedValues.
    _wait_eedata();  // 
	ClrWdt();
    // Write a row to EEPROM from array "EEDataInRam1".  
    _write_eedata_row(EE_addr2, EEDataInRam1); // 2nd copy of savedValues.
    _wait_eedata();  // Hopefully this takes less than 3 seconds.  haha.
	ClrWdt();
    // Write a row to EEPROM from array "EEDataInRam2"
    _write_eedata_row(EE_addr3, EEDataInRam2); // 1st copy of savedValues2.
    _wait_eedata();  // Hopefully this takes less than 3 seconds.  haha.
	ClrWdt();
    // Write a row to EEPROM from array "EEDataInRam2"
    _write_eedata_row(EE_addr4, EEDataInRam2); // 2nd copy of savedValues2.
    _wait_eedata();  // Hopefully this takes less than 3 seconds.  haha.
	ClrWdt();
	// 2 copies of the same thing, to be more robust.
}

void MoveDataFromEEPromToRAM() {
	int i = 0;
	unsigned int CRC1 = 0, CRC2 = 0, CRC3 = 0, CRC4 = 0;

	_memcpy_p2d16(EEDataInRam1, EE_addr1, _EE_ROW);
	_memcpy_p2d16(EEDataInRam2, EE_addr2, _EE_ROW);
	_memcpy_p2d16(EEDataInRam3, EE_addr3, _EE_ROW);
	_memcpy_p2d16(EEDataInRam4, EE_addr4, _EE_ROW);
	for (i = 0; i < 15; i++) { // Skip the last one, which is CRC.
		CRC1 += EEDataInRam1[i];
		CRC2 += EEDataInRam2[i];
		CRC3 += EEDataInRam3[i];
		CRC4 += EEDataInRam4[i];		
	}

	if (EEDataInRam1[15] == CRC1) {  // crc from EEProm is OK for copy 1.  There has been a previously saved configuration.  
		savedValues.Kp_Id = EEDataInRam1[0];		// 
		savedValues.Ki_Id = EEDataInRam1[1];						// 
		savedValues.Kp_Iq = EEDataInRam1[2];		// 
		savedValues.Ki_Iq = EEDataInRam1[3];						// 
		savedValues.currentSensorAmpsPerVolt = EEDataInRam1[4];		// 
		savedValues.maxRegenPosition = EEDataInRam1[5];		// 
		savedValues.minRegenPosition = EEDataInRam1[6];		// 
		savedValues.minThrottlePosition = EEDataInRam1[7];	// 
		savedValues.maxThrottlePosition = EEDataInRam1[8];	// 
		savedValues.throttleFaultPosition = EEDataInRam1[9];	// 
		savedValues.maxBatteryAmps = EEDataInRam1[10];		// 
		savedValues.maxBatteryAmpsRegen = EEDataInRam1[11];	// 
		savedValues.maxMotorAmps = EEDataInRam1[12];		// 
		savedValues.maxMotorAmpsRegen = EEDataInRam1[13];	//  
		savedValues.prechargeTime = EEDataInRam1[14];		//
		savedValues.crc = EEDataInRam1[15];					// 
	}
	else if (EEDataInRam2[15] == CRC2) {
		savedValues.Kp_Id = EEDataInRam2[0];		// 
		savedValues.Ki_Id = EEDataInRam2[1];						// 
		savedValues.Kp_Iq = EEDataInRam2[2];		// 
		savedValues.Ki_Iq = EEDataInRam2[3];						// 
		savedValues.currentSensorAmpsPerVolt = EEDataInRam2[4];		// 
		savedValues.maxRegenPosition = EEDataInRam2[5];		// 
		savedValues.minRegenPosition = EEDataInRam2[6];		// 
		savedValues.minThrottlePosition = EEDataInRam2[7];	// 
		savedValues.maxThrottlePosition = EEDataInRam2[8];	// 
		savedValues.throttleFaultPosition = EEDataInRam2[9];	// 
		savedValues.maxBatteryAmps = EEDataInRam2[10];		// 
		savedValues.maxBatteryAmpsRegen = EEDataInRam2[11];	// 
		savedValues.maxMotorAmps = EEDataInRam2[12];		// 
		savedValues.maxMotorAmpsRegen = EEDataInRam2[13];	//  
		savedValues.prechargeTime = EEDataInRam2[14];		//
		savedValues.crc = EEDataInRam2[15];					// 
	}
	else {
		savedValues = savedValuesDefault;
		savedValues.crc = 	  savedValues.Kp_Id + 
							savedValues.Ki_Id + 
							savedValues.Kp_Iq + 
							savedValues.Ki_Iq + 
							savedValues.currentSensorAmpsPerVolt + 
							savedValues.maxRegenPosition + 
							savedValues.minRegenPosition + 
							savedValues.minThrottlePosition + 
							savedValues.maxThrottlePosition + 
							savedValues.throttleFaultPosition + 
							savedValues.maxBatteryAmps + 
							savedValues.maxBatteryAmpsRegen + 
							savedValues.maxMotorAmps + 
							savedValues.maxMotorAmpsRegen + 
							savedValues.prechargeTime;
	}

	savedValues2.spares[0] = 0;
	savedValues2.spares[1] = 0;
	savedValues2.spares[2] = 0;
	savedValues2.spares[3] = 0;

	if (EEDataInRam3[15] == CRC3) {
		savedValues2.rotorTimeConstantArrayIndex = EEDataInRam3[0];		// 
		savedValues2.numberOfPolePairs = EEDataInRam3[1];						// 
		savedValues2.maxRPM = EEDataInRam3[2];		// 
		savedValues2.throttleType = EEDataInRam3[3];
		savedValues2.encoderTicks = EEDataInRam3[4];
		savedValues2.statorResistance_times1024 = EEDataInRam3[5];
		savedValues2.statorInductance_times1024 = EEDataInRam3[6];
		savedValues2.packVoltage = EEDataInRam3[7];
		savedValues2.sensorless = EEDataInRam3[8];
		savedValues2.crc = EEDataInRam3[15];					// 
	}
	else if (EEDataInRam4[15] == CRC4) {
		savedValues2.rotorTimeConstantArrayIndex = EEDataInRam4[0];		// 
		savedValues2.numberOfPolePairs = EEDataInRam4[1];						// 
		savedValues2.maxRPM = EEDataInRam4[2];		// 
		savedValues2.throttleType = EEDataInRam4[3];
		savedValues2.encoderTicks = EEDataInRam4[4];
		savedValues2.statorResistance_times1024 = EEDataInRam4[5];
		savedValues2.statorInductance_times1024 = EEDataInRam4[6];
		savedValues2.packVoltage = EEDataInRam4[7];
		savedValues2.sensorless = EEDataInRam4[8];
		savedValues2.crc = EEDataInRam4[15];					// 
	}
	else {	// There wasn't a single good copy.  Load the default configuration.
		savedValues2 = savedValuesDefault2;
		savedValues2.crc =  savedValues2.rotorTimeConstantArrayIndex + 
							savedValues2.numberOfPolePairs + 
							savedValues2.maxRPM + 
							savedValues2.throttleType +
							savedValues2.encoderTicks +
							savedValues2.statorResistance_times1024+
							savedValues2.statorInductance_times1024 + 
							savedValues2.packVoltage + 
							savedValues2.sensorless;
	}
}

void InitializeThrottleAndCurrentVariables() {
	// run this at startup, when there's a new savedValues.currentSensorAmpsPerVolt
	// Ex:  MaxMotorAmps = 300
	// 		currentSensorAmpsPerVolt = 480, which is for a LEM Hass 300-s
	// 		ticksPerAmp_times128 = 26214/480 = 54.6 = 54
	//		totalADTicks_times128 = 54 * maxMotorAmps = 54 * 300 = 16200
	//		maxMotorCurrentNormalized = 16200 >> 3 = 2025
	//
	//
	//
	//
	//
	// Ex:  MaxMotorAmps = 8
	// 		currentSensorAmpsPerVolt = 16, which is for a lem hass 50-s with 
	//		ticksPerAmp_times128 = 27214/16 = 1700.8 = 1700
	// 		totalADTicks_times128 = 1700 * maxMotorAmps = 1700 * 8 = 13600
	//		maxMotorCurrentNormalized = 1700
	//
	//
	//
	//
	//
	int ADTicksPerAmp_times128 = 0;
	long totalADTicks_times128 = 0;

	//ADTicksPerAmp_times128 = 128 * (1024/5) / currentSensorAmpsPerVolt; // 1024 A/D ticks per 5v.
	//ADTicksPerAmp_times128 = 26214 / currentSensorAmpsPerVolt;
	ADTicksPerAmp_times128 = (int)__builtin_divsd(26214L,(int)savedValues.currentSensorAmpsPerVolt);
//	ampToNormalizedMultiplier = ADTicksPerAmp_times128 >> 5;
	//totalADTicks_times128 = (ADTicksPerAmp_times128 * savedValues.maxMotorRegenAmps);
	totalADTicks_times128 = __builtin_mulss((int)ADTicksPerAmp_times128, (int)savedValues.maxMotorAmpsRegen);
	if ((totalADTicks_times128 >> 3) > 4096) {
		maxMotorCurrentNormalizedRegen = 4096;
	}
	else {
		maxMotorCurrentNormalizedRegen = totalADTicks_times128 >> 3;  // to go from [-256*128, 256*128] to [-4096, 4096], divide by 8.  I'm assuming a current sensor acceptable range from [1.25v, 3.75v].
	}

	//totalADTicksRegen_times128 = (ADTicksPerAmp_times128 * savedValues.maxMotorRegenAmps);
	totalADTicks_times128 = __builtin_mulss((int)ADTicksPerAmp_times128, (int)savedValues.maxMotorAmps);
	if ((totalADTicks_times128 >> 3) > 4096) {
		maxMotorCurrentNormalized = 4096;
	}
	else {
		maxMotorCurrentNormalized = totalADTicks_times128 >> 3;  // to go from [-256*128, 256*128] to [-4096, 4096], divide by 8.  I'm assuming a current sensor acceptable range from [1.25v, 3.75v].
	}

	totalADTicks_times128 = __builtin_mulss((int)ADTicksPerAmp_times128, (int)savedValues.maxBatteryAmps);	; // go from amps to normalized.  Normalized is following the above process.
	if ((totalADTicks_times128 >> 3) > 4096) {
		maxBatteryCurrentNormalized = 4096;
	}
	else {
		maxBatteryCurrentNormalized = totalADTicks_times128 >> 3;  // to go from [-256*128, 256*128] to [-4096, 4096], divide by 8.  I'm assuming a current sensor acceptable range from [1.25v, 3.75v].
	}

	totalADTicks_times128 = __builtin_mulss((int)ADTicksPerAmp_times128, (int)savedValues.maxBatteryAmpsRegen);	; // go from amps to normalized.  Normalized is following the above process.
	if ((totalADTicks_times128 >> 3) > 4096) {
		maxBatteryCurrentNormalizedRegen = 4096;
	}
	else {
		maxBatteryCurrentNormalizedRegen = totalADTicks_times128 >> 3;  // to go from [-256*128, 256*128] to [-4096, 4096], divide by 8.  I'm assuming a current sensor acceptable range from [1.25v, 3.75v].
	}
	currentSensorAmpsPerVoltTimes5 = (int)__builtin_mulss((int)savedValues.currentSensorAmpsPerVolt, 5);  // when converting from the normalized [0, 4096] to real life amps, the conversion factor is currentSensorAmpsPerVolt*5/16384.
	// I'm usually converting to amps*8 to increase resolution a bit, so the scale factor is actually currentSensorAmpsPerVolt * 5/2048.  So, do the multiply here, and save the shift for later.
}


//RCON
void __attribute__ ((__interrupt__,auto_psv)) _MathError(void) {
	while (1);
}
void __attribute__ ((__interrupt__,auto_psv)) _StackError(void) {
	while (1);
}
void __attribute__ ((__interrupt__,auto_psv)) _AddressError(void) {
	while (1);
}
void __attribute__ ((__interrupt__,auto_psv)) _OscillatorFail(void) {
	while (1);
}


/*
// The index into this array is a number from 0 to 1792, inclusive.  Ex:  the arctan of 0.172 is:  __arcTan_theta_0_122[round(0.172*128)] = __arctan_theta_0_122[22] = 14 "degrees".  It will tell you the angle from 0 to 122, where there are 512 "degrees" in the circle.
// 1792/128 = 14, which was already covered in the lookup table __arctan_theta_122_127[14], but a little overlap won't hurt anything.
// 122, 123, ..., 128 are covered as a separate special case.

/*
const char __arctan_theta_0_122[] = 
{
0,1,1,2,3,3,4,4,5,6,6,7,8,8,9,10,10,11,11,12,13,13,14,14,15,16,16,17,18,18,19,19,20,21,21,22,22,23,24,24,25,25,26,26,27,28,28,29,29,30,30,31,31,32,33,33,34,34,35,35,36,36,37,37,38,38,39,39,40,40,41,41,42,42,43,43,44,44,45,45,46,46,46,47,47,48,48,49,49,50,50,50,51,51,52,52,52,53,53,54,54,54,55,55,56,56,56,57,57,57,58,58,59,59,59,60,60,60,61,61,61,62,62,62,63,63,63,64,64,64,65,65,65,66,66,66,66,67,67,67,68,68,68,69,69,69,69,70,70,70,70,71,71,71,71,72,72,72,73,73,73,73,74,74,74,74,74,75,75,75,75,76,76,76,76,77,77,77,77,77,78,78,78,78,78,79,79,79,79,79,80,80,80,80,80,81,81,81,81,81,82,82,82,82,82,83,83,83,83,83,83,84,84,84,84,84,84,85,85,85,85,85,85,86,86,86,86,86,86,86,87,87,87,87,87,87,88,88,88,88,88,88,88,88,89,89,89,89,89,89,89,90,90,90,90,90,90,90,90,91,91,91,91,91,91,91,91,92,92,92,92,92,92,92,92,93,93,93,93,93,93,93,93,93,93,94,94,94,94,94,94,94,94,94,95,95,95,95,95,95,95,95,95,95,96,96,96,
96,96,96,96,96,96,96,96,97,97,97,97,97,97,97,97,97,97,97,98,98,98,98,98,98,98,98,98,98,98,98,99,99,99,99,99,99,99,99,99,99,99,99,99,100,100,100,100,100,100,100,100,100,100,100,100,100,100,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,102,103,103,103,103,103,103,103,103,103,103,103,103,103,103,103,103,103,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,104,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,107,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,108,109,109,109,109,109,109,109,109,109,109,
109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,113,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,
114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,115,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,
117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,117,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,118,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,
119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,121,121,121,121,121,121,121,121,121,121,121,121,
121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,121,122,122,122,122,122,122,122,122,122,122,122,122,122,
122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,122,
};
*/
// This covers arctan(14) to arctan(infinity).
// if ((long)y >= (long)14*x), offset = y/x;  So, artcan(y/x) = __arctan_theta_122_127[y/x].
// char is OK since it only goes up to 127.  phew!
/*
const char __arctan_theta_122_127[] = 
{ // index 0 through index 13 will never happen.  Just trying to avoid subtracting 14 each time you do arctan.
0,0,0,0,0,0,0,0,0,0,0,0,0,0,122,123,123,123,123,124,124,124,124,124,125,125,125,125,125,125,125,125,125,126,126,126,126,126,126,126,126,126,126,126,126,126,126,126,126,126,126,126,126,126,126,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
};
*/
// 
//
// ex:  correction[0][0..31] covers the half-periods 32/10000, (33)/10000, ..., (63)/10000.
// ex:  correction[1][0..31] covers the half-periods 64/10000, (64+2)/10000, ..., (64+2*31)/10000.
// ex:  correction[2][0..31] covers the half-periods 128/10000, (128+4)/10000, ..., (128+4*31)/10000.
// ex:  correction[3][0..31] covers the half-periods 256/10000, (256+8)/10000, ..., (256+8*31)/10000.
// ex:  correction[4][0..31] covers the half-periods 512/10000, (512+16)/10000, ..., (512+16*31)/10000.
// ex:  correction[5][0..31] covers the half-periods 1024/10000, (1024+32)/10000, ..., (1024+32*31)/10000.
// ex:  correction[6][0..31] covers the half-periods 2048/10000, (2048+64)/10000, ..., (2048+64*31)/10000.
// ex:  correction[7][0..31] covers the half-periods 4096/10000, (4096+128)/10000, ..., (4096+128*31)/10000.
// ex:  correction[8][0..31] covers the half-periods 8192/10000, (8192+256)/10000, ..., (8192+256*31)/10000.
// ex:  correction[9][0..31] covers the half-periods 16384/10000, (16384+512)/10000, ..., (16384+512*31)/10000.

// So, the goal is to figure out which [][] you are in, given a half-period.  Ex:  half-period is 785.  find [][]:
// Well, 0-9 is found through the series of if..then statements at the beginning.  done.  haha.
/*
const int sensorlessShiftCorrection[10][32] = {{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},{3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},{7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7},{15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15},{30,30,30,30,30,30,30,30,30,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31},{60,61,61,61,61,61,61,61,61,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,62,63,63,63,63},{121,122,122,122,122,123,123,123,123,124,124,124,124,124,124,124,125,125,125,125,125,125,125,125,125,125,125,125,126,126,126,126},{243,244,244,245,245,246,246,247,247,248,248,248,249,249,249,249,250,250,250,250,250,251,251,251,251,251,251,251,252,252,252,252},{487,488,489,490,
491,492,493,494,495,496,497,497,498,498,499,499,500,500,501,501,501,502,502,502,503,503,503,503,504,504,504,504}, {957,958,959,960,961,962,963,964,965,966,967,968,969,970,971,972,973,974,975,976,977,978,979,980,981,982,983,984,985,986,987,988}
};

// the filtered IAlpha and Ibeta has been shifted down by must be scaled up by this / 1024.  between 1/0.93 and 1/0.98.  multiply filtered i by this to obtain correct magnitude of true i(t). 
const int sensorlessMagnitudeCorrection_times1024[] = {
1098,1094,1090,1086,1083,1080,1077,1074,1072,1070,1067,1066,1064,1062,1060,1059,1057,1056,1055,1054,1053,1052,1051,1050,1049,1048,1047,1046,1046,1045,1044,1044,
};

// to get the index into this array from your inductance, you do 
// index = (YourInductance - 0.010) * 1000.
// ex:  YourInductance = 30mH.  So, index = (0.030 - 0.010) * 100 = 0.020 * 1000 = 20
// Valid indices are 0 through 290.  So, valid inductances are 0.010 through 0.300Henry, in increments of 0.001Henry.
// these actual numbers are Lr/(0.866*Lr)^2 * 128 * 16 / (2*pi), because the constant is used in a formula that ends up with radians per sec, but I'm using revolutions / 16 seconds, so it must be scaled by 16/(2*pi).
//const unsigned int LrLmSquared_times128Array[] = {  // this covers 0.006 to 0.300 Henries, with a resolution of 0.001.
//43460,39509,36217,33431,31043,28973,27162,25565,24144,22874,21730,20695,19755,18896,18108,17384,16715,16096,15521,14986,14487,14019,13581,13170,12782,12417,12072,11746,11437,11144,10865,10600,10348,10107,9877,9658,9448,9247,9054,8869,8692,8522,8358,8200,8048,7902,7761,7625,7493,7366,7243,7125,7010,6898,6791,6686,6585,6487,6391,6299,6209,6121,6036,5953,5873,5795,5718,5644,5572,5501,5432,5365,5300,5236,5174,5113,5053,4995,4939,4883,4829,4776,4724,4673,4623,4575,4527,4480,4435,4390,4346,4303,4261,4219,4179,4139,4100,4062,4024,3987,3951,3915,3880,3846,
//3812,3779,3747,3715,3683,3652,3622,3592,3562,3533,3505,3477,3449,3422,3395,3369,3343,3318,3292,3268,3243,3219,3196,3172,3149,3127,3104,3082,3061,3039,3018,2997,2977,2956,2936,2917,2897,2878,2859,2841,2822,2804,2786,2768,2751,2733,2716,2699,2683,2666,2650,2634,2618,2602,2587,2572,2556,2542,2527,2512,2498,2483,2469,2455,2442,2428,2414,2401,2388,2375,2362,2349,2337,2324,2312,2299,2287,2275,2264,2252,2240,2229,2217,2206,2195,2184,2173,2162,2151,2141,2130,2120,2110,
//2100,2089,2079,2070,2060,2050,2040,2031,2021,2012,2003,1994,1984,1975,1967,1958,1949,1940,1932,1923,1915,1906,1898,1890,1881,1873,1865,1857,1849,1842,1834,1826,1818,1811,1803,1796,1788,1781,1774,1767,1760,1752,1745,1738,1731,1725,1718,1711,1704,1698,1691,1684,1678,1672,1665,1659,1652,1646,1640,1634,1628,1622,1616,1610,1604,1598,1592,1586,1580,1575,1569,1563,1558,1552,1547,1541,1536,1530,1525,1520,1514,1509,1504,1499,1493,1488,1483,1478,1473,1468,1463,1458,1454,1449
//};

// To get the index into this array from your inductance, you do
// index = (YourInductance - 0.0001) * 10000.
// valid indices are 0 through 199.  So, valid inductances are 0.0001 through 0.0200 Henry, with increments of 0.0001 Henry.
const unsigned int LrLmSquared_times8FineArray[] = {  // This covers 0.0001 to 0.0200 with a resolution of 0.0001.
33953,16977,11318,8488,6791,5659,4850,4244,3773,3395,3087,2829,2612,2425,2264,2122,1997,1886,1787,1698,1617,1543,1476,1415,1358,1306,1258,1213,1171,1132,1095,1061,1029,999,970,943,918,894,871,849,828,808,790,772,755,738,722,707,693,679,666,653,641,629,617,606,596,585,575,566,557,548,539,531,522,514,507,499,492,485,478,472,465,459,453,447,441,435,430,424,419,414,409,404,399,395,390,386,381,377,373,369,365,361,357,354,350,346,343,340,336,333,330,326,323,320,317,314,311,
309,306,303,300,298,295,293,290,288,285,283,281,278,276,274,272,269,267,265,263,261,259,257,255,253,252,250,248,246,244,243,241,239,237,236,234,233,231,229,228,226,225,223,222,220,219,218,216,215,214,212,211,210,208,207,206,205,203,202,201,200,199,197,196,195,194,193,192,191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,175,174,173,172,171,171,170,
};

int __arctan(int numerator, int denominator) {
	static int negative = 0;
	static int angle = 0;

	if (denominator == 0) {	// it's a 90 degree angle, unless the top is zero too...  oh well.  let's just call arctan(0/0) 90 degrees as well.
		return 128;
	}
	if (numerator < 0) {
		if (denominator < 0) {  // numerator < 0 and denominator < 0.
			negative = 0;
			numerator = -numerator;
			denominator = -denominator;
		}
		else {  // numerator < 0 and denominator >= 0
			negative = 1;
			numerator = -numerator;
		}
	}
	else { // numerator >= 0.
		if (denominator < 0) {  // numerator >= 0 and denominator < 0
			negative = 1;  // numerator and denominator have opposite signs, so the final answer is negative.
			denominator = -denominator;
		}
		else {	// numerator >= 0 and denominator >= 0
			negative = 0;	// numerator >= 0 and denominator >= 0.
		}
	}
	if (((long)numerator) >= __builtin_mulss(163,denominator)) { // y/x >= 163.  arctan(163) = 127.5001 "degrees", which rounds up to 128.
		return 128;  // 90 degree angle.
	}
	if (((long)numerator) >= __builtin_mulss(14,denominator)) { // y/x >= 14.  So, the angle is in 122, ..., 127.  use a lookup table with index going from 0 to 162 to find out the correct angle.
		// if (y >= 14*x), offset = y/x;  So, artcan(y/x) = __arctan_theta_122_127[y/x].
		// char is OK since it only goes up to 127.  phew!
		angle = (int)__arctan_theta_122_127[__builtin_divsd((long)numerator, (int)denominator)];
		if (negative) {
			angle = -angle;		// 
		}
		return angle;
	}
	// If it made it here, the angle is in [0, 122], and x is in 0, 13.9999.
	// The index into this array is a number from 0 to 1792, inclusive.  Ex:  the arctan of 0.3671875 is:  __arcTan_theta_0_122[round(0.3671875*128)] = __arctan_theta_0_122[47] = 25 "degrees".  It will tell you the angle from 0 to 122, where there are 512 "degrees" in the circle.
	// 1792/128 = 14, which was already covered in the lookup table __arctan_theta_122_127[14], but a little overlap won't hurt anything.
	// 122, 123, ..., 128 are covered as a separate special case.
	// __arctan_theta_0_122[] = 

	// y is in [0, 13.999999*x].  So, give it more resolution.
	// yBig = y * 128.
	// ratio = yBig / x;
	angle = __arctan_theta_0_122[__builtin_divsd(__builtin_mulss(numerator, 128), (int)denominator)];
	if (negative) {
		angle = -angle;
	}
	return angle;
}
*/
/*			if (rotorFluxAngle < oldRotorFluxAngle) {  // only when a new period starts.
				newPeriodStarted = 1;
				periodOffsetStart = counter10k;
			}
		
			if (BEMFAngleSensorless < OldBEMFAngleSensorless && newPeriodStarted) {  // new period started.
				newPeriodStarted = 0;
				periodOffset = counter10k - periodOffsetStart;
				ratio_times1250 = __builtin_mulss((int)rotorFluxRPS_times16, (int)periodOffset) >> 7;
				largeStorage.data[0][dataCounter] = ratio_times1250;//ratio_times1250;
				dataCounter++;
				if (dataCounter < 400) { // savedValues2.statorInductance_times1024 < 250) {
//					savedValues2.statorInductance_times1024++;
//					savedValues2.statorInductance_times1024++;
//					LrLmSquared_times128 = LrLmSquared_times128Array[savedValues2.rotorInductance_times1024 - MIN_ROTOR_INDUCTANCE];
				}
				else {
//					savedValues2.rotorInductance_times1024 = 20;
//					savedValues2.statorInductance_times1024 = 20;
//
//					if (savedValues2.statorResistance_times1024 < 1500) {
//						savedValues2.statorResistance_times1024+=50;
//					}
//					else {
//						savedValues2.statorInductance_times1024 = 20;
//						if (savedValues2.statorResistance_times1024 < 1500) {
//							savedValues2.statorResistance_times1024 += 100;
//						}
//						else {
							largeArrayLoaded = 1;
							captureVariable = 0;
							dataCounter = 0;
//						}
//					}
				}
//			}
*/
//	Ed_times8_times65536   	//= (3*Ed_times8_times65536/65536 + 1*Ed_times8) / 4 * 65536;
								 					//= (4*Ed_times8_times65536/65536 - 1*Ed_times8_filtered + 1*Ed_times8) / 4 * 65536
//								= ((Ed_times8_times65536 >> 14) - ((long)Ed_times8_filtered) + ((long)Ed_times8)) << 14;
//	Ed_times8_filtered = Ed_times8_times65536 >> 16;
//	Eq_times8_times65536   	//= (3*Eq_times8_times65536/65536 + 1*Eq_times8) / 4 * 65536;
								 					//= (4*Eq_times8_times65536/65536 - 1*Eq_times8_filtered + 1*Eq_times8) / 4 * 65536
//								= ((Eq_times8_times65536 >> 14) - ((long)Eq_times8_filtered) + ((long)Eq_times8)) << 14;
//	Eq_times8_filtered = Eq_times8_times65536 >> 16;

//	Eq_times8_filtered = Eq_times8;
//	Ed_times8_filtered = Ed_times8;
	// Now, low pass filter Ed and Eq.  1st order low pass filter is recommended.  I'll do a running average.
	// 
	
//	Ed_times8 = (__builtin_mulss(3,Ed_times8) + Ed) >> 2;
//	Eq_times8 = (__builtin_mulss(3,Eq_times8) + Eq) >> 2;
//	Ed_times8 = (Ed_times8 + Ed) >> 1;
//	Eq_times8 = (Eq_times8 + Eq) >> 1;
//	EqAverageOld = Eq_times8;

//	Ed_times8 = Ed;
//	Eq_times8 = Eq;
	// magnetizingCurrent MUST be nonzero, since this is run after the test was already done in ComputeRotorFluxAngle().
//	if (EqAverageOld >= 0 && Eq_times8 < 0) {
//		period_div2 = counter10k - start; 
//		if (period_div2 >= 10) {
//			start = counter10k;		
//			BEMF_RPS_times16_sensorless = __builtin_divsd(320000L, (int)period_div2);
//		}
//	}
//	else if (EqAverageOld < 0 && Eq_times8 >= 0) {
//		period_div2 = counter10k - start; 
//		if (period_div2 >= 10) {
//			start = counter10k;		
//			BEMF_RPS_times16_sensorless = __builtin_divsd(320000L, (int)period_div2);
//		}
//	}
//	if (magnetizingCurrentAmps_times8 >= 8) {
//	if (magnetizingCurrentAmps_times8 != 0) {
//		if (Eq_times8_filtered < 0) {
			// rotorFluxSpeedRadiansPerSec = ((1 + sigmaR) / Psi_mR) * (Eq - sign(Eq) * Ed).  Note:  sigmaR = (Lr / Lm) - 1, where Lr = rotor inductance and Lm is mutual inductance.
			//  Also, Psi_mR = Lm * i_mR, i_mR is magnetizing Current.  Plugging it in and simplifying, you get:
			// rotorFluxSpeedRadiansPerSec = (Lr / Lm^2) * 1/magnetizingCurrentAmps * (Eq_volts - sign(Eq_volts)*Ed_volts);
			// BEMF_RPS_times16_sensorless = LrLmSquared_times128 * (1/(magnetizingCurrentAmps)*(Eq_times8 - sign(Eq_times8)*Ed_times8)
			// 
			// 
			//	BEMF_RPS_times16_sensorless = __builtin_divsd(Eq_times8 + Ed_times8, magnetizingCurrentAmps_times8);
			// WARNING HERE!!!!!  MAY NOT BE INTEGER.
//			tempLongEdEq = __builtin_mulus((unsigned int)LrLmSquared_times128, (int)(Eq_times8_filtered + Ed_times8_filtered));
//			if (Ed_times8_filtered >= 0) {  // decrease of negative speed. aka slowing down.
//				decreaseNeg++;

//				if (BEMFToRotorFluxAngleCorrection > 60) {
//					BEMFToRotorFluxAngleCorrection--;
//				}
//			}
//			else {  // speed up in negative direction.
//				increaseNeg++;
////				if (BEMFToRotorFluxAngleCorrection < 80) {
//					BEMFToRotorFluxAngleCorrection++;
//				}				
//			}
//		}
//		else {
//			tempLongEdEq = __builtin_mulus((unsigned int)(LrLmSquared_times128), (int)(Eq_times8_filtered - Ed_times8_filtered));
//			if (Ed_times8_filtered >= 0) {  // decrease of positive speed.  slow down.
//				increasePos--;
//				if (LrLmSquared_times128 > 0) LrLmSquared_times128--;
//				if (BEMFToRotorFluxAngleCorrection >  80) {
//					BEMFToRotorFluxAngleCorrection--;
//				}
//			}
//			else { // increase of positive speed.  speed up.
//				if (LrLmSquared_times128 < 32767) LrLmSquared_times128++;
//				increasePos++;
//				if (BEMFToRotorFluxAngleCorrection < 100) {
//					BEMFToRotorFluxAngleCorrection++;
//				}				
//			}
//		}
		
//	savedValues2.rotorInductance_times1024 = 82;
//	LrLmSquared_times128 = 5500;
// < >= advances more and more!!!
// >= < lags more and more
// >= >= corrects right both ways. sort of.
//
//		tempLongEdEq >>= 7;  // it was scaled up by 128, so scale down by 128
		// now it SHOULD be safe to do this division, as the RPS times 16 should be way less than 32000.

//		BEMF_RPS_times16_sensorless = __builtin_divsd((long)tempLongEdEq, (int)magnetizingCurrentAmps_times8);
//		BEMF_RPS_times16_sensorless_filtered_times65536 = ((BEMF_RPS_times16_sensorless_filtered_times65536 >> 14) - ((long)BEMF_RPS_times16_sensorless_filtered) + ((long)BEMF_RPS_times16_sensorless)) << 14;  // see above for derivation.
//		BEMF_RPS_times16_sensorless_filtered = BEMF_RPS_times16_sensorless_filtered_times65536 >> 16;

//	}

/*
	if (stateIAlpha == TOP_HALF_SINE_WAVE) {
		if (i_alpha_amps_times8_filtered < 0) { // just went below the x-axis.  is it truly the end of a half-period for this filtered waveform?
			if (tIAlpha >= (periodIAlpha_div_2 >> 1) && (tIAlpha >= 20)) {  // it's a valid change of state.  The period was long enough.  phew.
//			if (tIAlpha >= 20) {  // it's a valid change of state.  The period was long enough.  phew.
				stateIAlpha = BOTTOM_HALF_SINE_WAVE;
				magnitudeIAlpha_times8 = -localMaxCandidateIAlpha;  // negative since it's the bottom half of a sine wave and you are going to compute the derivative below.
				magnitudeIAlpha_times8 = __builtin_mulss((int)magnitudeIAlpha_times8, (int)magnitudeCorrection_times1024) >> 10;  // the magnitude of the filtered iAlpha is somwhere between 0.93 and 0.98 of the true iAlpha.  So, correct it.
				periodIAlpha_div_2 = tIAlpha;
				tIAlpha = 0;
//				localMaxCandidateIAlpha = 0;  // This will be reset at the end of the bottom half period, so don't bother.  just be sure to initialize it to zero at program startup.
				localMinCandidateIAlpha = i_alpha_amps_times8_filtered;
			}
			else { // not a valid change of state.  Just compute what the derivative ought to be based on where you OUGHT to be along the corrected sine wave.
				if (tIAlpha < 31000) {
					tIAlpha++;
					if (periodIAlpha_div_2 < tIAlpha) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
						periodIAlpha_div_2 = tIAlpha;
					}
				}
				
			}
		}
		else {  // still traveling along top half of sine wave. Do you have a new local maximum?  work on finding the magnitude of this half cycle.  Useful for the next half-cycle.
			if (i_alpha_amps_times8_filtered > localMaxCandidateIAlpha) {
				localMaxCandidateIAlpha = i_alpha_amps_times8_filtered;
//				if (localMaxCandidateIAlpha > magnitudeIAlpha_times8) {
//					magnitudeIAlpha_times8 = localMaxCandidateIAlpha; // this half cycle is already higher than the previous one, so update it with the most recent info.
//				}
			}
			if (tIAlpha < 31000) {
				tIAlpha++;
				if (periodIAlpha_div_2 < tIAlpha) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
					periodIAlpha_div_2 = tIAlpha;
				}
			}
		}
	}
	else {  // state == BOTTOM_HALF_SINE_WAVE
		if (i_alpha_amps_times8_filtered >= 0) {  // just went above the x-axis.  is it truly the end of a half-period for this filtered waveform?
			if (tIAlpha >= (periodIAlpha_div_2 >> 1) && (tIAlpha >= 20)) {  // it's a valid change of state.
//			if (tIAlpha >= 20) {  // it's a valid change of state.
				stateIAlpha = TOP_HALF_SINE_WAVE;
				magnitudeIAlpha_times8 = -localMinCandidateIAlpha;  // negative since it's the bottom half of a sine wave and you are going to compute the derivative below.
				magnitudeIAlpha_times8 = __builtin_mulss((int)magnitudeIAlpha_times8, (int)magnitudeCorrection_times1024) >> 10;  // the magnitude of the filtered iAlpha is somwhere between 0.93 and 0.98 of the true iAlpha.  So, correct it.
				periodIAlpha_div_2 = tIAlpha;
				tIAlpha = 0;
				localMaxCandidateIAlpha = i_alpha_amps_times8_filtered;  // The first time the waveform pokes its head above the x-axis is the new local maximum so far.
//				localMinCandidateIAlpha = 0; // This will be reset at the end of the top half-period, so don't bother.  just be sure to initialize it to zero at program startup.
			}
			else { // not a valid change of state.  Just compute what the derivative ought to be based on where you OUGHT to be along the corrected sine wave.
				if (tIAlpha < 31000) {
					tIAlpha++;
					if (periodIAlpha_div_2 < tIAlpha) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
						periodIAlpha_div_2 = tIAlpha;
					}
				}
			}
		}
		else {  // still traveling along bottom half of sine wave. Do you have a new local minimum?  work on finding the magnitude of this half cycle.  Useful for the next half-cycle.
			if (i_alpha_amps_times8_filtered < localMinCandidateIAlpha) {
				localMinCandidateIAlpha = i_alpha_amps_times8_filtered;
//				if (localMinCandidateIAlpha < magnitudeIAlpha_times8) {
//					magnitudeIAlpha_times8 = localMinCandidateIAlpha; // this half cycle is already more negative than previous half cycle was positive, so update it with the most recent info.
//				}
			}
			if (tIAlpha < 31000) {
				tIAlpha++;
				if (periodIAlpha_div_2 < tIAlpha) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
					periodIAlpha_div_2 = tIAlpha;
				}
			}
		}
	}
	theta = __builtin_divsd( ((long)(tIAlpha + shiftCorrection)) << 8, (int)periodIAlpha_div_2);
	if (theta > 511) {
		theta = 511;
	}
	thetaPlus90 = (theta + 128) & 511;
//	di_alpha_dt_times8_old = di_alpha_dt_times8;  // save old value for some reason.  haha. You can use it as a test to see if di/dt is changing too much.
	// magnitudeIAlpha_times8 must be in [-32768, 32767] to avoid overflow.  That corresponds to -4096 to 4095 amps.

//	i(t) = magnitude * sin(2*pi/period * t), t in seconds.
//  period = 2*periodIAlpha_div_2/10000, so...
//  i(t) = magnitude * sin(2*pi/(2*periodIAlpha_div_2/10000))
//  i(t) = magnitude * sin(10000 * pi / periodIAlpha_div_2 * t), t in seconds.
//  i(t) = magnitude * sin(theta)
//  di/dt = magnitude * 10000 * pi / periodIAlpha_div_2 * cos(10000 * pi / periodIAlpha_div_2 * t), t in seconds.
//  di/dt = magnitude * 10000 * pi / periodIAlpha_div_2 * cos(theta).
//  di/dt = magnitude * 31416 / periodIAlpha_div_2 * cos(theta).
//  di/dt = magnitude * (31416 * cos(theta)) / periodIAlpha_div_2.


	i_alpha_amps_times8_filtered_corrected = __builtin_mulss((int)magnitudeIAlpha_times8, (int)_sin_times32768[theta]) >> 15;

	di_alpha_dt_times8_filtered_corrected = __builtin_mulss((int)magnitudeIAlpha_times8, (int)(__builtin_mulss(31416, (int)_sin_times32768[thetaPlus90]) >> 15));

	tempLongAlpha = ((long)periodIAlpha_div_2) << derivativeScale;  // this is the denominator.  Is it too big or too small, so as to mess up the inputs or output of __builtin_divsd?

	// this is now for sure safe to do.  It is a guarantee that you have a long divided by an int.  And the result will fit inside an int.
	if (tempLongAlpha < 6500L) tempLongAlpha = 6500L;
	else if (tempLongAlpha > 32767L) tempLongAlpha = 32767;
		di_alpha_dt_times8_filtered_corrected = (long)__builtin_divsd((long)di_alpha_dt_times8_filtered_corrected, (int)tempLongAlpha);		

	if (stateIBeta == TOP_HALF_SINE_WAVE) {
		if (i_beta_amps_times8_filtered < 0) { // just went below the x-axis.  is it truly the end of a half-period for this filtered waveform?
			if (tIBeta >= (periodIBeta_div_2 >> 1) && (tIBeta >= 20)) {  // it's a valid change of state.  The period was long enough.  phew.
//			if (tIBeta >= 20) {  // it's a valid change of state.  The period was long enough.  phew.
				stateIBeta = BOTTOM_HALF_SINE_WAVE;
				magnitudeIBeta_times8 = -localMaxCandidateIBeta;  // negative since it's the bottom half of a sine wave and you are going to compute the derivative below.
				magnitudeIBeta_times8 = __builtin_mulss((int)magnitudeIBeta_times8, (int)magnitudeCorrection_times1024) >> 10;  // the magnitude of the filtered iBeta is somwhere between 0.93 and 0.98 of the true iBeta.  So, correct it.
				periodIBeta_div_2 = tIBeta;
				tIBeta = 0;
//				localMaxCandidateIBeta = 0;  // This will be reset at the end of the bottom half period, so don't bother.  just be sure to initialize it to zero at program startup.
				localMinCandidateIBeta = i_beta_amps_times8_filtered;
			}
			else { // not a valid change of state.  Later just compute what it ought to be based on where you OUGHT to be along the corrected sine wave.
				if (tIBeta < 31000) {
					tIBeta++;
					if (periodIBeta_div_2 < tIBeta) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
						periodIBeta_div_2 = tIBeta;
					}
				}
			}
		}
		else {  // still traveling along top half of sine wave. Do you have a new local maximum?  work on finding the magnitude of this half cycle.  Useful for the next half-cycle.
			if (i_beta_amps_times8_filtered > localMaxCandidateIBeta) {
				localMaxCandidateIBeta = i_beta_amps_times8_filtered;
//				if (localMaxCandidateIBeta > magnitudeIBeta_times8) {
//					magnitudeIBeta_times8 = localMaxCandidateIBeta; // this half cycle is already higher than the previous one, so update it with the most recent info.
//				}
			}
			if (tIBeta < 31000) {
				tIBeta++;
				if (periodIBeta_div_2 < tIBeta) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
					periodIBeta_div_2 = tIBeta;
				}
			}
		}
	}
	else {  // state == BOTTOM_HALF_SINE_WAVE
		if (i_beta_amps_times8_filtered >= 0) {  // just went above the x-axis.  is it truly the end of a half-period for this filtered waveform?
			if (tIBeta >= (periodIBeta_div_2 >> 1) && (tIBeta >= 20)) {  // it's a valid change of state.
//			if (tIBeta >= 20) {  // it's a valid change of state.
				stateIBeta = TOP_HALF_SINE_WAVE;
				magnitudeIBeta_times8 = -localMinCandidateIBeta;  //  negative since it's the bottom half of a sine wave and you are going to compute the derivative below.
				magnitudeIBeta_times8 = __builtin_mulss((int)magnitudeIBeta_times8, (int)magnitudeCorrection_times1024) >> 10;  // the magnitude of the filtered iBeta is somwhere between 0.93 and 0.98 of the true iBeta.  So, correct it.
				periodIBeta_div_2 = tIBeta;
				tIBeta = 0;
				localMaxCandidateIBeta = i_beta_amps_times8_filtered;  // The first time the waveform pokes its head above the x-axis is the new local maximum so far.
//				localMinCandidateIBeta = 0; // This will be reset at the end of the top half-period, so don't bother.  just be sure to initialize it to zero at program startup.
			}
			else { // not a valid change of state.  Just compute what the derivative ought to be based on where you OUGHT to be along the corrected sine wave.
				if (tIBeta < 31000) {
					tIBeta++;
					if (periodIBeta_div_2 < tIBeta) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
						periodIBeta_div_2 = tIBeta;
					}
				}
			}
		}
		else {  // still traveling along bottom half of sine wave. Do you have a new local minimum?  work on finding the magnitude of this half cycle.  Useful for the next half-cycle.
			if (i_beta_amps_times8_filtered < localMinCandidateIBeta) {
				localMinCandidateIBeta = i_beta_amps_times8_filtered;
//				if (localMinCandidateIBeta < magnitudeIBeta_times8) {
//					magnitudeIBeta_times8 = localMinCandidateIBeta; // this half cycle is already more negative than previous half cycle was positive, so update it with the most recent info.
//				}
			}
			if (tIBeta < 31000) {
				tIBeta++;
				if (periodIBeta_div_2 < tIBeta) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
					periodIBeta_div_2 = tIBeta;
				}
			}
		}
	}

	theta = __builtin_divsd( ((long)(tIBeta + shiftCorrection)) << 8, (int)periodIBeta_div_2);
	if (theta > 511) {
		theta = 511;
	}
	thetaPlus90 = (theta + 128) & 511;

// if periodIBeta is sufficiently different from periodIAlpha, you could get overflow.  test this...
	i_beta_amps_times8_filtered_corrected = __builtin_mulss((int)magnitudeIBeta_times8, (int)_sin_times32768[theta]) >> 15;  // this is the corrected, ideal i_beta.  the filtered version was shifted over, and squashed down a bit.

	di_beta_dt_times8_filtered_corrected = __builtin_mulss((int)magnitudeIBeta_times8, (int)(__builtin_mulss(31416, (int)_sin_times32768[thetaPlus90]) >> 15));  // do the derivative numerator first.
	tempLongBeta = ((long)periodIBeta_div_2) << derivativeScale;  // this is the denominator.  Is it too big or too small, so as to mess up the inputs or output of __builtin_divsd?

	if (tempLongBeta < 6500L) tempLongBeta = 6500L;
	else if (tempLongBeta > 32767L) tempLongBeta = 32767L;
		di_beta_dt_times8_filtered_corrected = (long)__builtin_divsd((long)di_beta_dt_times8_filtered_corrected, (int)tempLongBeta);

// Now find the filtered, corrected sine wave values for vAlpha and vBeta.  
	if (stateVAlpha == TOP_HALF_SINE_WAVE) {
		if (v_alpha_volts_times8_filtered < 0) { // just went below the x-axis.  is it truly the end of a half-period for this filtered waveform?
			if (tVAlpha >= (periodVAlpha_div_2 >> 1) && (tVAlpha >= 20)) {  // it's a valid change of state.  The period was long enough.  phew.
//			if (tVAlpha >= 20) {  // it's a valid change of state.  The period was long enough.  phew.
				stateVAlpha = BOTTOM_HALF_SINE_WAVE;
				magnitudeVAlpha_times8 = -localMaxCandidateVAlpha;  // negative since it's the bottom half of a sine wave and you are going to compute the derivative below.
				magnitudeVAlpha_times8 = __builtin_mulss((int)magnitudeVAlpha_times8, (int)magnitudeCorrection_times1024) >> 10;  // the magnitude of the filtered vAlpha is somwhere between 0.93 and 0.98 of the true vAlpha.  So, correct it.
				periodVAlpha_div_2 = tVAlpha;
				tVAlpha = 0;
//				localMaxCandidateVAlpha = 0;  // This will be reset at the end of the bottom half period, so don't bother.  just be sure to initialize it to zero at program startup.
				localMinCandidateVAlpha = v_alpha_volts_times8_filtered;
			}
			else { // not a valid change of state.  Period was too short.  Just compute what the derivative ought to be based on where you OUGHT to be along the corrected sine wave.
				if (tVAlpha < 31000) {
					tVAlpha++;
					if (periodVAlpha_div_2 < tVAlpha) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
						periodVAlpha_div_2 = tVAlpha;
					}
				}
			}
		}
		else {  // still traveling along top half of sine wave. Do you have a new local maximum?  work on finding the magnitude of this half cycle.  Useful for the next half-cycle.
			if (v_alpha_volts_times8_filtered > localMaxCandidateVAlpha) {
				localMaxCandidateVAlpha = v_alpha_volts_times8_filtered;
//				if (localMaxCandidateVAlpha > magnitudeVAlpha_times8) {
//					magnitudeVAlpha_times8 = localMaxCandidateVAlpha; // this half cycle is already higher than the previous one, so update it with the most recent info.
//				}
			}
			if (tVAlpha < 31000) {
				tVAlpha++;
				if (periodVAlpha_div_2 < tVAlpha) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
					periodVAlpha_div_2 = tVAlpha;
				}
			}
		}
	}
	else {  // state == BOTTOM_HALF_SINE_WAVE
		if (v_alpha_volts_times8_filtered >= 0) {  // just went above the x-axis.  is it truly the end of a half-period for this filtered waveform?
			if (tVAlpha >= (periodVAlpha_div_2 >> 1) && (tVAlpha >= 20)) {  // it's a valid change of state.
//			if (tVAlpha >= 20) {  // it's a valid change of state.
				stateVAlpha = TOP_HALF_SINE_WAVE;
				magnitudeVAlpha_times8 = -localMinCandidateVAlpha;  // 
				magnitudeVAlpha_times8 = __builtin_mulss((int)magnitudeVAlpha_times8, (int)magnitudeCorrection_times1024) >> 10;  // the magnitude of the filtered vAlpha is somwhere between 0.93 and 0.98 of the true vAlpha.  So, correct it.
				periodVAlpha_div_2 = tVAlpha;
				tVAlpha = 0;
				localMaxCandidateVAlpha = v_alpha_volts_times8_filtered;  // The first time the waveform pokes its head above the x-axis is the new local maximum so far.
//				localMinCandidateVAlpha = 0; // This will be reset at the end of the top half-period, so don't bother.  just be sure to initialize it to zero at program startup.
			}
			else { // not a valid change of state.  Just compute what the derivative ought to be based on where you OUGHT to be along the corrected sine wave.
				if (tVAlpha < 31000) {
					tVAlpha++;
					if (periodVAlpha_div_2 < tVAlpha) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
						periodVAlpha_div_2 = tVAlpha;
					}
				}
			}
		}
		else {  // still traveling along bottom half of sine wave. Do you have a new local minimum?  work on finding the magnitude of this half cycle.  Useful for the next half-cycle.
			if (v_alpha_volts_times8_filtered < localMinCandidateVAlpha) {
				localMinCandidateVAlpha = v_alpha_volts_times8_filtered;
//				if (localMinCandidateVAlpha < magnitudeVAlpha_times8) {
//					magnitudeVAlpha_times8 = localMinCandidateVAlpha; // this half cycle is already more negative than previous half cycle was positive, so update it with the most recent info.
//				}
			}
			if (tVAlpha < 31000) {
				tVAlpha++;
				if (periodVAlpha_div_2 < tVAlpha) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
					periodVAlpha_div_2 = tVAlpha;
				}
			}
		}
	}
	theta = __builtin_divsd( ((long)(tVAlpha + shiftCorrection)) << 8, (int)periodVAlpha_div_2);
	if (theta > 511) {
		theta = 511;
	}
//	thetaPlus90 = (theta + 128) & 511;
	
//	di_alpha_dt_times8_old = di_alpha_dt_times8;  // save old value for some reason.  haha. You can use it as a test to see if di/dt is changing too much.
	// magnitudeVAlpha_times8 must be in [-32768, 32767] to avoid overflow.  That corresponds to -4096 to 4095 amps.

//	i(t) = magnitude * sin(2*pi/period * t), t in seconds.
//  period = 2*periodVAlpha_div_2/10000, so...
//  i(t) = magnitude * sin(2*pi/(2*periodVAlpha_div_2/10000))
//  i(t) = magnitude * sin(10000 * pi / periodVAlpha_div_2 * t), t in seconds.
//  i(t) = magnitude * sin(theta)
//  di/dt = magnitude * 10000 * pi / periodVAlpha_div_2 * cos(10000 * pi / periodVAlpha_div_2 * t), t in seconds.
//  di/dt = magnitude * 10000 * pi / periodVAlpha_div_2 * cos(theta).
//  di/dt = magnitude * 31416 / periodVAlpha_div_2 * cos(theta).
//  di/dt = magnitude * (31416 * cos(theta)) / periodVAlpha_div_2.
	v_alpha_volts_times8_filtered_corrected = __builtin_mulss((int)magnitudeVAlpha_times8, (int)_sin_times32768[theta]) >> 15;
//	dv_alpha_dt_times8 = __builtin_mulss(31416, _sin_times32768[thetaPlus90]) >> 15;
//	dv_alpha_dt_times8 = __builtin_mulss(magnitude, (int)di_alpha_dt_times8);
//	dv_alpha_dt_times8 = __builtin_divsd(di_alpha_dt_times8, periodVAlpha_div_2);

	if (stateVBeta == TOP_HALF_SINE_WAVE) {
		if (v_beta_volts_times8_filtered < 0) { // just went below the x-axis.  is it truly the end of a half-period for this filtered waveform?
			if (tVBeta >= (periodVBeta_div_2 >> 1) && (tVBeta >= 20)) {  // it's a valid change of state.  The period was long enough.  phew.
//			if (tVBeta >= 20) {  // it's a valid change of state.  The period was long enough.  phew.
				stateVBeta = BOTTOM_HALF_SINE_WAVE;
				magnitudeVBeta_times8 = -localMaxCandidateVBeta;  // negative since it's the bottom half of a sine wave and you are going to compute the derivative below.
				magnitudeVBeta_times8 = __builtin_mulss((int)magnitudeVBeta_times8, (int)magnitudeCorrection_times1024) >> 10;  // the magnitude of the filtered vBeta is somwhere between 0.93 and 0.98 of the true vBeta.  So, correct it.
				periodVBeta_div_2 = tVBeta;
				tVBeta = 0;
//				localMaxCandidateVBeta = 0;  // This will be reset at the end of the bottom half period, so don't bother.  just be sure to initialize it to zero at program startup.
				localMinCandidateVBeta = v_beta_volts_times8_filtered;
			}
			else { // not a valid change of state.  Just compute what the derivative ought to be based on where you OUGHT to be along the corrected sine wave.
				if (tVBeta < 31000) {
					tVBeta++;
					if (periodVBeta_div_2 < tVBeta) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
						periodVBeta_div_2 = tVBeta;
					}
				}
			}
		}
		else {  // still traveling along top half of sine wave. Do you have a new local maximum?  work on finding the magnitude of this half cycle.  Useful for the next half-cycle.
			if (v_beta_volts_times8_filtered > localMaxCandidateVBeta) {
				localMaxCandidateVBeta = v_beta_volts_times8_filtered;
//				if (localMaxCandidateVBeta > magnitudeVBeta_times8) {
//					magnitudeVBeta_times8 = localMaxCandidateVBeta; // this half cycle is already higher than the previous one, so update it with the most recent info.
//				}
			}
			if (tVBeta < 31000) {
				tVBeta++;
				if (periodVBeta_div_2 < tVBeta) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
					periodVBeta_div_2 = tVBeta;
				}
			}
		}
	}
	else {  // state == BOTTOM_HALF_SINE_WAVE
		if (v_beta_volts_times8_filtered >= 0) {  // just went above the x-axis.  is it truly the end of a half-period for this filtered waveform?
			if (tVBeta >= (periodVBeta_div_2 >> 1) && (tVBeta >= 20)) {  // it's a valid change of state.
//			if (tVBeta >= 20) {  // it's a valid change of state.
				stateVBeta = TOP_HALF_SINE_WAVE;
				magnitudeVBeta_times8 = -localMinCandidateVBeta;  //  negative since it's the bottom half of a sine wave and you are going to compute the derivative below.
				magnitudeVBeta_times8 = __builtin_mulss((int)magnitudeVBeta_times8, (int)magnitudeCorrection_times1024) >> 10;  // the magnitude of the filtered iBeta is somwhere between 0.93 and 0.98 of the true iBeta.  So, correct it.
				periodVBeta_div_2 = tVBeta;
				tVBeta = 0;
				localMaxCandidateVBeta = v_beta_volts_times8_filtered;  // The first time the waveform pokes its head above the x-axis is the new local maximum so far.
//				localMinCandidate = 0; // This will be reset at the end of the top half-period, so don't bother.  just be sure to initialize it to zero at program startup.
			}
			else { // not a valid change of state.  Just compute what the derivative ought to be based on where you OUGHT to be along the corrected sine wave.
				if (tVBeta < 31000) {
					tVBeta++;
					if (periodVBeta_div_2 < tVBeta) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
						periodVBeta_div_2 = tVBeta;
					}
				}
			}
		}
		else {  // still traveling along bottom half of sine wave. Do you have a new local minimum?  work on finding the magnitude of this half cycle.  Useful for the next half-cycle.
			if (v_beta_volts_times8_filtered < localMinCandidateVBeta) {
				localMinCandidateVBeta = v_beta_volts_times8_filtered;
//				if (localMinCandidateVBeta < magnitudeVBeta_times8) {
//					magnitudeVBeta_times8 = localMinCandidateVBeta; // this half cycle is already more negative than previous half cycle was positive, so update it with the most recent info.
//				}
			}
			if (tVBeta < 31000) {
				tVBeta++;
				if (periodVBeta_div_2 < tVBeta) {  // if the current half-period has gotten bigger than the last one, and still isn't done yet, change the old period so it sort of comes along.
					periodVBeta_div_2 = tVBeta;
				}
			}
		}
	}
	theta = __builtin_divsd( ((long)(tVBeta + shiftCorrection)) << 8, (int)periodVBeta_div_2);
	if (theta > 511) {
		theta = 511;
	}
*/
/*
// 
// make sure the half-period is clamped to around 31700 to allow for an advance of around 1024. 
// this sequence of if..thens is just to extract the magnitude correction and shift correction for i_alpha(t), i_beta(t), v_alpha(t), and v_beta(t).
	if (periodIAlpha_div_2 >= 16384) {
		derivativeScale = 0;  // later when computing the derivatives of Ialpha and Ibeta, the numbers get a little crazy, and I need to stay with int*int = long, and long / int = int for speed's sake.
		// So, since you divide by the period_div_2 to finish computing the derivative, as long as the denominator is between about 6000 and 32767, everything works out OK.
		// But the period can go all the way down to like 20, so you introduce derivativeScale, which multiplies the denominator by the necessary amount to force it to be in the acceptable range of 6000 (or so) up to 32767.
		// Then, later, when you are actually using the derivative, you have to scale it UP by the same factor that it was scaled down, to even things out.  But at that point, you have already passed the dreaded division,
		// and have successfully avoided the uncertainty of long / (int or long) = int or long.  It now must be long / int = int.  So you can use the much faster divide.
		correctionIndex = (periodIAlpha_div_2 - 16384) >> 9;  // ex:  half-period = 18233.  To find the offset, subtract off 16384, then divide by 512 because the list of half-periods was 16384 + 512*0, 16384 + 512*1, 16384 + 512*2, .., 16384 + 512*31.
		shiftCorrection = sensorlessShiftCorrection[9][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];

		i_alpha_amps_times8_filtered_times65536   	//= (1023*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 1024 * 65536;
								 					//= (1024*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 1024 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 6) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 6;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;


		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 6) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 6;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 6) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 6;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 6) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 6;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else if (periodIAlpha_div_2 >= 8192) {
		derivativeScale = 1;
		correctionIndex = (periodIAlpha_div_2 - 8192) >> 8;  // ex:  half-period = 18233.  To find the offset, subtract off 8192, then divide by 256 because the list of half-periods was 8192 + 256*0, 8192 + 256*1, 8192 + 256*2, .., 8192 + 256*31.
		shiftCorrection = sensorlessShiftCorrection[8][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];

		i_alpha_amps_times8_filtered_times65536   	//= (511*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 512 * 65536;
								 					//= (512*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 512 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 7) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 7;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;

		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 7) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 7;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 7) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 7;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 7) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 7;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else if (periodIAlpha_div_2 >= 4096) {
		derivativeScale = 2;
		correctionIndex = (periodIAlpha_div_2 - 4096) >> 7;  // ex:  half-period = 7631.  To find the offset, subtract off 4096, then divide by 128 because the list of half-periods was 4096 + 128*0, 4096 + 128*1, 4096 + 128*2, .., 4096 + 128*31.
		shiftCorrection = sensorlessShiftCorrection[7][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];

		i_alpha_amps_times8_filtered_times65536   	//= (255*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 256 * 65536;
								 					//= (256*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 256 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 8) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 8;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;

		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 8) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 8;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 8) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 8;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 8) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 8;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else if (periodIAlpha_div_2 >= 2048) {
		derivativeScale = 3;
		correctionIndex = (periodIAlpha_div_2 - 2048) >> 6;  // ex:  half-period = 3411.  To find the offset, subtract off 2048, then divide by 64 because the list of half-periods was 2048 + 64*0, 2048 + 64*1, 2048 + 64*2, .., 2048 + 64*31.
		shiftCorrection = sensorlessShiftCorrection[6][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];
		i_alpha_amps_times8_filtered_times65536   	//= (127*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 128 * 65536;
								 					//= (128*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 128 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 9) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 9;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;

		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 9) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 9;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 9) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 9;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 9) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 9;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else if (periodIAlpha_div_2 >= 1024) {		
		derivativeScale = 4;
		correctionIndex = (periodIAlpha_div_2 - 1024) >> 5;  // ex:  half-period = 1879.  To find the offset, subtract off 1024, then divide by 32 because the list of half-periods was 1024 + 32*0, 1024 + 32*1, 1024 + 32*2, .., 1024 + 32*31.
		shiftCorrection = sensorlessShiftCorrection[5][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];
		i_alpha_amps_times8_filtered_times65536   	//= (63*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 64 * 65536;
								 					//= (64*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 64 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 10) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 10;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;

		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 10) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 10;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 10) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 10;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 10) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 10;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else if (periodIAlpha_div_2 >= 512) {
		derivativeScale = 5;
		correctionIndex = (periodIAlpha_div_2 - 512) >> 4;  // ex:  half-period = 777.  To find the offset, subtract off 512, then divide by 16 because the list of half-periods was 512 + 16*0, 512 + 16*1, 512 + 16*2, .., 512 + 16*31.
		shiftCorrection = sensorlessShiftCorrection[4][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];
		i_alpha_amps_times8_filtered_times65536   	//= (31*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 32 * 65536;
								 					//= (32*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 32 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 11) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 11;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;

		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 11) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 11;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 11) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 11;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 11) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 11;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else if (periodIAlpha_div_2 >= 256) {
		derivativeScale = 6;
		correctionIndex = (periodIAlpha_div_2 - 256) >> 3;  // ex:  half-period = 357.  To find the offset, subtract off 256, then divide by 8 because the list of half-periods was 256 + 8*0, 256 + 8*1, 256 + 8*2, .., 256 + 8*31.
		shiftCorrection = sensorlessShiftCorrection[3][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];
		i_alpha_amps_times8_filtered_times65536   	//= (15*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 16 * 65536;
								 					//= (16*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 16 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 12) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 12;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;

		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 12) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 12;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 12) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 12;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 12) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 12;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else if (periodIAlpha_div_2 >= 128) {
		derivativeScale = 7;
		correctionIndex = (periodIAlpha_div_2 - 128) >> 2;  // ex:  half-period = 185.  To find the offset, subtract off 128, then divide by 4 because the list of half-periods was 128 + 4*0, 128 + 4*1, 128 + 4*2, .., 128 + 4*31.
		shiftCorrection = sensorlessShiftCorrection[2][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];
		i_alpha_amps_times8_filtered_times65536   	//= (7*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 8 * 65536;
								 					//= (8*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 8 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 13) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 13;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;

		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 13) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 13;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 13) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 13;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 13) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 13;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else if (periodIAlpha_div_2 >= 64) {
		derivativeScale = 8;
		correctionIndex = (periodIAlpha_div_2 - 64) >> 1;  // ex:  half-period = 93.  To find the offset, subtract off 64, then divide by 2 because the list of half-periods was 64 + 2*0, 64 + 2*1, 64 + 2*2, .., 64 + 2*31.
		// So, for 93, 93-64 = 29.  29 / 2 = 14.  correctionIndex would then be 14.
		shiftCorrection = sensorlessShiftCorrection[1][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];
		i_alpha_amps_times8_filtered_times65536   	//= (3*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 4 * 65536;
								 					//= (4*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 4 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 14) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 14;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;

		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 14) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 14;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 14) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 14;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 14) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 14;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else if (periodIAlpha_div_2 >= 32) {
		derivativeScale = 9;
		correctionIndex = (periodIAlpha_div_2 - 32) >> 0;  // ex:  half-period = 59.  To find the offset, subtract off 32, then divide by 1 because the list of half-periods was 32 + 1*0, 32 + 1*1, 32 + 1*2, .., 32 + 1*31.
		// So, for 59, 59-32 = 27.  27 / 1 = 27.  correctionIndex would then be 27.
		shiftCorrection = sensorlessShiftCorrection[0][correctionIndex];
		magnitudeCorrection_times1024 = sensorlessMagnitudeCorrection_times1024[correctionIndex];
		i_alpha_amps_times8_filtered_times65536   	//= (1*i_alpha_amps_times8_filtered_times65536/65536 + 1*tempIAlpha) / 2 * 65536;
								 					//= (2*i_alpha_amps_times8_filtered_times65536/65536 - 1*i_alpha_amps_times8_filtered + 1*tempIAlpha) / 2 * 65536
								= ((i_alpha_amps_times8_filtered_times65536 >> 15) - ((long)i_alpha_amps_times8_filtered) + ((long)tempIAlpha)) << 15;
		i_alpha_amps_times8_filtered = i_alpha_amps_times8_filtered_times65536 >> 16;

		i_beta_amps_times8_filtered_times65536 = ((i_beta_amps_times8_filtered_times65536 >> 15) - ((long)i_beta_amps_times8_filtered) + ((long)tempIBeta)) << 15;  // see above for derivation.
		i_beta_amps_times8_filtered = i_beta_amps_times8_filtered_times65536 >> 16;

		v_alpha_volts_times8_filtered_times65536 = ((v_alpha_volts_times8_filtered_times65536 >> 15) - ((long)v_alpha_volts_times8_filtered) + ((long)tempVAlpha)) << 15;  // see above for derivation.
		v_alpha_volts_times8_filtered = v_alpha_volts_times8_filtered_times65536 >> 16;

		v_beta_volts_times8_filtered_times65536 = ((v_beta_volts_times8_filtered_times65536 >> 15) - ((long)v_beta_volts_times8_filtered) + ((long)tempVBeta)) << 15;  // see above for derivation.
		v_beta_volts_times8_filtered = v_beta_volts_times8_filtered_times65536 >> 16;
	}
	else {
		derivativeScale = 10;
		shiftCorrection = 0;  // no filtering, so no shifting needed.
		magnitudeCorrection_times1024 = 1024;  // magnitude correction multiplier is 1.000
		i_alpha_amps_times8_filtered = tempIAlpha;
		i_beta_amps_times8_filtered = tempIAlpha;
		v_alpha_volts_times8_filtered = tempVAlpha;
		v_beta_volts_times8_filtered = tempVBeta;
	}
*/
//void ComputeRotorFluxAngleSensorless() {
/*	

// ****For divide, use this:  	int quot =	 __builtin_divsd(long numerator, int denominator);****
// ****For multiply, use this:  long prod =   __builtin_mulus(unsigned left,  int right);****  or muluu, or mulsu, or mulss.

//	else if (theta < 0) theta = 0;
//	v_alpha_volts_times8 = v_alpha * pack_voltage * 8/MAX_DUTY = v_alpha*pack_voltage*8/2948 = (v_alpha * pack_voltage * 699) >> 17 real life volts.  2/737 is approximately 178 >> 16
// MAX_DUTY = 2948.  To avoid division, let's find 8/MAX_DUTY by multply and shift down.  8/2948 is... 2/737 = 178 >> 16.
	v_alpha_volts_times8 = (__builtin_mulss((int)savedValues2.packVoltage, (int)v_alpha) * 178L) >> 16;  
	v_beta_volts_times8 = (__builtin_mulss((int)savedValues2.packVoltage, (int)v_beta) * 178L) >> 16;

	// i_alpha_amps_times8 = 8 * i_alpha ticks * currentSensorAmpsPerVolt / (4096/1.25);
	// i_alpha_amps_times8 = 8 * i_alpha ticks * currentSensorAmpsPerVolt * 1.25volts / 4096ticks;
	// i_alpha_amps_times8 = i_alpha * currentSensorAmpsPerVolt * 4 * 1.25 * 2 / 4096;
	// i_alpha_amps_times8 = currentSensorAmpsPerVolt * 5 * i_alpha / 2048;
	// i_alpha_amps_times8 = currentSensorAmpsPerVolt*5 * i_alpha >> 11;
//	i_alpha_amps_times8 = __builtin_mulss((int)(__builtin_mulss((int)savedValues.currentSensorAmpsPerVolt, 5)), i_alpha) >> 11;
//	i_beta_amps_times8 = __builtin_mulss((int)(__builtin_mulss((int)savedValues.currentSensorAmpsPerVolt, 5)), i_beta) >> 11;

	i_alpha_amps_times8_old10 = i_alpha_amps_times8_old9;
	i_alpha_amps_times8_old9 = i_alpha_amps_times8_old8;
	i_alpha_amps_times8_old8 = i_alpha_amps_times8_old7;
	i_alpha_amps_times8_old7 = i_alpha_amps_times8_old6;
	i_alpha_amps_times8_old6 = i_alpha_amps_times8_old5;
	i_alpha_amps_times8_old5 = i_alpha_amps_times8_old4;
	i_alpha_amps_times8_old4 = i_alpha_amps_times8_old3;
	i_alpha_amps_times8_old3 = i_alpha_amps_times8_old2;
	i_alpha_amps_times8_old2 = i_alpha_amps_times8_old1;
	i_alpha_amps_times8_old1 = i_alpha_amps_times8;
	i_alpha_amps_times8 = __builtin_mulss((int)currentSensorAmpsPerVoltTimes5, (int)i_alpha) >> 11;

	temp = i_alpha_amps_times8 - i_alpha_amps_times8_old10;
	
	if (temp > 32) {
		di_alpha_dt_times8 = 32000;
	}
	else if (temp < -32) {
		di_alpha_dt_times8 = -32000;
	}
	else {
		di_alpha_dt_times8 = __builtin_mulss(temp,1000); 
	}

	i_beta_amps_times8_old10 = i_beta_amps_times8_old9;
	i_beta_amps_times8_old9 = i_beta_amps_times8_old8;
	i_beta_amps_times8_old8 = i_beta_amps_times8_old7;
	i_beta_amps_times8_old7 = i_beta_amps_times8_old6;
	i_beta_amps_times8_old6 = i_beta_amps_times8_old5;
	i_beta_amps_times8_old5 = i_beta_amps_times8_old4;
	i_beta_amps_times8_old4 = i_beta_amps_times8_old3;
	i_beta_amps_times8_old3 = i_beta_amps_times8_old2;
	i_beta_amps_times8_old2 = i_beta_amps_times8_old1;
	i_beta_amps_times8_old1 = i_beta_amps_times8;
	i_beta_ave = 
	i_beta_amps_times8 = __builtin_mulss((int)currentSensorAmpsPerVoltTimes5, (int)i_beta) >> 11;

	temp = i_beta_amps_times8 - i_beta_amps_times8_old10;
	
	if (temp > 32) {
		di_beta_dt_times8 = 32000;
	}
	else if (temp < -32) {
		di_beta_dt_times8 = -32000;
	}
	else {
		di_beta_dt_times8 = __builtin_mulss(temp,1000); 
	}
	
//	v_beta_volts_times8_filtered_corrected = __builtin_mulss((int)magnitudeVBeta_times8, (int)_sin_times32768[theta]) >> 15;
	temp1 = __builtin_mulss((int)savedValues2.statorResistance_times1024, (int)i_alpha_amps_times8) >> 10;
	temp2 = __builtin_mulss((int)savedValues2.statorInductance_times1024, (int)di_alpha_dt_times8) >> 10;
	temp3 = __builtin_mulss((int)savedValues2.statorResistance_times1024,  (int)i_beta_amps_times8) >> 10;
	temp4 = __builtin_mulss((int)savedValues2.statorInductance_times1024, (int)di_beta_dt_times8)  >> 10;

	E_alpha_times8 = (((long)v_alpha_volts_times8) - temp1 - temp2);
	E_beta_times8 =  (((long)v_beta_volts_times8)  - temp3 - temp4);

	BEMFAngleSensorless = __arctan(E_beta_times8, E_alpha_times8);
	// BEMFAngleSensorless is in [-127, 128]. 
	if (E_beta_times8 >= 0) { 
		if (BEMFAngleSensorless < 0) { 	
			BEMFAngleSensorless = 256 + BEMFAngleSensorless;
		}
	}
	else {
		if (BEMFAngleSensorless >= 0) {
			BEMFAngleSensorless += 256;
		}
		else {
			BEMFAngleSensorless += 512;			
		}
	}
	BEMFAngleSensorless &= 511;  // may not be needed.
	// now BEMFAngleSensorless is in [0, 256]
	cos_theta_times32768 = _sin_times32768[(BEMFAngleSensorless + 128) & 511];  // 
	sin_theta_times32768 = _sin_times32768[BEMFAngleSensorless];  // 
	// Park type of Transform:
	// Ed = Ealpha*cos(BEMFAngleSensorless) + Ebeta*sin(BEMFAngleSensorless)
	// Eq = -Ealpha*sin(BEMFAngleSensorless) + Ebeta*cos(BEMFAngleSensorless)
	Ed_times8 = (__builtin_mulss((int)E_alpha_times8,(int)cos_theta_times32768) + __builtin_mulss((int)E_beta_times8,(int)sin_theta_times32768)) >> 15; 	
	Eq_times8 = (__builtin_mulss((int)(-E_alpha_times8),(int)sin_theta_times32768) + __builtin_mulss((int)E_beta_times8,(int)cos_theta_times32768)) >> 15; 

	// I think that if Ed is nonzero, maybe the stator resistance and stator inductance need tweaking?
	if (Eq_times8 > 0) {
		positiveRotation = 1;
		oldRotorFluxAngleSensorless = rotorFluxAngleSensorless;
		rotorFluxAngleSensorless = BEMFAngleSensorless - 128;//15;//128;  // rotate 90 degrees clockwise, then make sure it's still in [0,511].
		rotorFluxAngleSensorless = (rotorFluxAngleSensorless + 512) & 511;
		if (rotorFluxAngleSensorless < 128 && oldRotorFluxAngleSensorless > 384) { // assume you just crossed zero
			period10k = counter10k - periodStart;
			if (period10k < 10) {
				rotorFluxRPS_times16_sensorless = 16000;  // biggest possible 
			}
			else {
				rotorFluxRPS_times16_sensorless = __builtin_divsd(160000L, period10k);
			}
			periodStart = counter10k;
		}
	}
	else {
		positiveRotation = 0;
		oldRotorFluxAngleSensorless = rotorFluxAngleSensorless;
		rotorFluxAngleSensorless = BEMFAngleSensorless + 128;//128;  // rotate 90 degrees counter-clockwise, then make sure it's still in [0,511].
		rotorFluxAngleSensorless = rotorFluxAngleSensorless & 511;
		if (oldRotorFluxAngleSensorless < 128 && rotorFluxAngleSensorless > 384) { // assume you just crossed zero
			period10k = counter10k - periodStart;  // still positive period even if rotating other way.
			if (period10k < 10) {
				rotorFluxRPS_times16_sensorless = -16000;  // biggest possible 
			}
			else {
				rotorFluxRPS_times16_sensorless = __builtin_divsd(-160000L, period10k);
			}
			periodStart = counter10k;
		}
	}
	// velMechRPS = (VelFluxRPS - VelSlipRPS) / numPolePairs;
	RPS_times16_sensorless = __builtin_divsd(rotorFluxRPS_times16_sensorless - slipSpeedRPS_times16, savedValues2.numberOfPolePairs);
*/
//}

