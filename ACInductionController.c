// COMPUTE ROTOR FLUX ANGLE Code
#include "ACController.h"

/*****************Config bit settings****************/
_FOSC(0xFFFF & XT_PLL8);//XT_PLL4); // Use XT with external crystal from 4MHz to 10MHz.  FRM Pg. 178
// nominal clock is 128kHz.  The counter is 1 byte. 
//#define STANDARD_THROTTLE
#ifdef DEBUG
	_FWDT(WDT_OFF);
#else
	_FWDT(WDT_ON & WDTPSA_64 & WDTPSB_8); // See Pg. 709 in F.R.M.  Timeout in 1 second or so.  128000 / 64 / 8 / 256
#endif

_FBORPOR(0xFFFF & BORV_27 & PWRT_64 & MCLR_EN & PWMxL_ACT_HI & PWMxH_ACT_HI); // Brown Out voltage set to 2v.  Power up time of 64 ms. MCLR is enabled.
// PWMxL_ACT_HI means PDC1 = 0 would mean PWM1L is 0 volts.  Just the opposite of how I always thought it worked.  haha.
// PWMxH_ACT_HI means, in complementary mode, that PDC1 = 0 would mean PWM1H is 5v, because it would be "active" (the complement of PWM1L), and thus HIGH. 
_FGS(CODE_PROT_OFF);

const SavedValuesStruct savedValuesDefault = {
	1875,30,1875,30,DEFAULT_CURRENT_SENSOR_AMPS_PER_VOLT,25,400,624,972,5,10,10,8,8,30,0
};

SavedValuesStruct savedValues = {
	1875,30,1875,30,DEFAULT_CURRENT_SENSOR_AMPS_PER_VOLT,25,400,624,972,5,10,10,8,8,30,0
};

const SavedValuesStruct2 savedValuesDefault2 = {
	29,2,12000,0, {0,0,0,0,0,0,0,0,0,0,0}, 0
};
SavedValuesStruct2 savedValues2 = {
	29,2,12000,0, {0,0,0,0,0,0,0,0,0,0,0}, 0
};

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

// = loopPeriod / rotorTimeConstant * 2^18.  loopPeriod is 0.0001 seconds, because it's being run at 10KHz. Rotor time constants range from 0.040 to 0.140 seconds.
// After using an element from this array, you must eventually divide the result by 2^18!!!
// rotorTimeConstantArray1[0] corresponds to a rotorTimeConstant of 0.005 seconds.  rotorTimeConstantArray1[145] <=> rotor time constant of 0.150.
const unsigned int rotorTimeConstantArray1[] = {
//    0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56   57   58   59   60   61   62   63   64   65   66   67   68   69   70   71   72   73   74   75   76   77   78   79   80   81   82   83   84   85   86   87   88   89   90   91   92   93   94   95   96   97   98   99  100  101  102  103  104  105  106  107  108  109  110  111  112  113  114  115  116  117  118  119  120  121  122  123  124  125  126  127  128  129  130  131  132  133  134  135  136  137  138  139  140  141  142  143  144  145 
// .005,.006,.007,.008,.009,.010,.011,.012,.013,.014,.015,.016,.017,.018,.019,.020,.021,.022,.023,.024,.025,.026,.027,.028,.029,.030,.031,.032,.033,.034,.035,.036,.037,.038,.039,.040,.041,.042,.043,.044,.045,.046,.047,.048,.049,.050,.051,.052,.053,.054,.055,.056,.057,.058,.059,.060,.061,.062,.063,.064,.065,.066,.067,.068,.069,.070,.071,.072,.073,.074,.075,.076,.077,.078,.079,.080,.081,.082,.083,.084,.085,.086,.087,.088,.089,.090,.091,.092,.093,.094,.095,.096,.097,.098,.099,.100,.101,.102,.103,.104,.105,.106,.107,.108,.109,.110,.111,.112,.113,.114,.115,.116,.117,.118,.119,.120,.121,.122,.123,.124,.125,.126,.127,.128,.129,.130,.131,.132,.133,.134,.135,.136,.137,.138,.139,.140,.141,.142,.143,.144,.145,.146,.147,.148,.149,.150
   5243,4369,3745,3277,2913,2621,2383,2185,2016,1872,1748,1638,1542,1456,1380,1311,1248,1192,1140,1092,1049,1008, 971, 936, 904, 874, 846, 819, 794, 771, 749, 728, 708, 690, 672, 655, 639, 624, 610, 596, 583, 570, 558, 546, 535, 524, 514, 504, 495, 485, 477, 468, 460, 452, 444, 437, 430, 423, 416, 410, 403, 397, 391, 386, 380, 374, 369, 364, 359, 354, 350, 345, 340, 336, 332, 328, 324, 320, 316, 312, 308, 305, 301, 298, 295, 291, 288, 285, 282, 279, 276, 273, 270, 267, 265, 262, 260, 257, 255, 252, 250, 247, 245, 243, 240, 238, 236, 234, 232, 230, 228, 226,224,222,220,218,217,215,213,211,210,208,206,205,203,202,200,199,197,196,194,193,191,190,189,187,186,185,183,182,181,180,178,177,176,175
};
// (1/rotorTimeConstant) * 1/(2*pi) * 2^11.  I'm trying to keep all of them in integer range.
// rotorTimeConstantArray2[0] corresponds to a rotorTimeConstant of 0.005 seconds.  rotorTimeConstantArray2[145] = 0.150.
const unsigned int rotorTimeConstantArray2[] = {
//    0     1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55
// .005, .006, .007, .008, .009, .010, .011, .012, .013, .014, .015, .016, .017, .018, .019, .020, .021, .022, .023, .024, .025, .026, .027, .028, .029, .030, .031, .032,.033,.034,.035,.036,.037,.038,.039,.040,.041,.042,.043,.044,.045,.046,.047,.048,.049,.050,.051,.052,.053,.054,.055,.056,.057,.058,.059,.060,.061,.062,.063,.064,.065,.066,.067,.068,.069,.070,.071,.072,.073,.074,.075,.076,.077,.078,.079,.080,.081,.082,.083,.084,.085,.086,.087,.088,.089,.090,.091,.092,.093,.094,.095,.096,.097,.098,.099,.100,.101,.102,.103,.104,.105,.106,.107,.108,.109,.110,.111,.112,.113,.114,.115,.116,.117,.118,.119,.120,.121,.122,.123,.124,.125,.126,.127,.128,.129,.130,.131,.132,.133,.134,.135,.136,.137,.138,.139,.140,.141,.142,.143,.144,.145,.146,.147,.148,.149,.150
  65190,54325,46564,40744,36217,32595,29632,27162,25073,23282,21730,20372,19173,18108,17155,16297,15521,14816,14172,13581,13038,12537,12072,11641,11240,10865,10514,10186,9877,9587,9313,9054,8809,8578,8358,8149,7950,7761,7580,7408,7243,7086,6935,6791,6652,6519,6391,6268,6150,6036,5926,5821,5718,5620,5525,5432,5343,5257,5174,5093,5015,4939,4865,4793,4724,4656,4591,4527,4465,4405,4346,4289,4233,4179,4126,4074,4024,3975,3927,3880,3835,3790,3747,3704,3662,3622,3582,3543,3505,3468,3431,3395,3360,3326,3292,3259,3227,3196,3165,3134,3104,3075,3046,3018,2990,2963,2936,2910,2885,2859,2834,2810,2786,2762,2739,2716,2694,2672,2650,2629,2608,2587,2567,2546,2527,2507,2488,2469,2451,2432,2414,2397,2379,2362,2345,2328,2312,2295,2279,2264,2248,2233,2217,2202,2188,2173
};

// This one is hard to explain.  It's to quickly find distance from (Vd, Vq) to (0,0).  (Vd,Vq) must be clamped to some maximum radius.  The fast distance I use isn't very accurate, so I add this correction.
// Let's say Vd <= Vq.  You find the index for this array by doing index = 1024 * Vd / Vq.  This works because the %error from fastDistance(Vd,Vq) is the same as long as Vd/Vq is the same.
// In other words, if Vd1/Vq1 = Vd2/Vq2, then fastDistance(Vd1,Vq1)/RealDistance(Vd1,Vq1) = fastDistance(Vd2,Vq2)/RealDistance(Vd2,Vq2).
// All this just to avoid doing a square root.  haha.
// It has been scaled up by 2^15.  So, you must shift down by 15 after multiplying by distCorrection[].
// array size is 1025.  distCorrection[0] up to distCorrection[1024].
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

extern unsigned int timeOfLastDatastreamTransmission;
extern char RTDataString[];
extern volatile char newChar;
extern volatile int echoNewChar;


volatile realtime_data_type RTData = {0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile unsigned int showDatastreamJustOnce = 0;
volatile unsigned int faultBits = 0;
volatile int vRef1 = 0, vRef2 = 0;
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
volatile unsigned int dataStreamPeriod = 0;  //  the real time data string will transmit every "dataStreamPeriod" milliseconds.
volatile int temperatureBasePlate = 0;
volatile int temperatureMotor = 0;
volatile long temperatureSum = 0;
volatile unsigned int counter1ms = 0;
volatile int ADCurrent1 = 0, ADCurrent2 = 0, ADThrottle = 0, ADTemperature = 0;
volatile unsigned int rotorFluxAngle = 1; 		// This is the rotor flux angle. In [0, 511]
volatile int RPS_times16 = 0; // range [-3200, 3200], where 3200 corresponds to 200rev/sec = 12,000RPM, and 0 means 0RPM.
volatile int oldRPS = 0;	// previous RPS_times16.
volatile int veryOldRPS = 0; // previous oldRPS.
volatile int rRate = 1;
volatile piType pi_Iq, pi_Id;

volatile int i_alpha = 0;
volatile int i_beta = 0;
volatile int Id = 0;
volatile int Iq = 0;
volatile int Ia = 0, Ib = 0;
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

volatile int minClampErrorVd = 0;
volatile int maxClampErrorVd = 0;
volatile int minClampErrorVq = 0;
volatile int maxClampErrorVq = 0;

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
volatile int maxLineToLineADTicks = 0;

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
void DelayTenthsSecond(unsigned int time);
void DelaySeconds(unsigned int time);
void ReadADInputs();
void GrabADResults();
void InitADAndPWM();
void InitDiscreteADConversions();
void GetVRefs();
void DelaySeconds(unsigned int seconds);
void DecimalToString(int number, int length);
char IntToCharHex(unsigned int i);
void InitCNModule();
void InitPIStruct();
void ClearAllFaults();  // clear the flip flop fault and the desat fault.
void ClearDesatFault();
void ClearFlipFlop();
void ComputeRotorFluxAngle();
void SpaceVectorModulation();
void ClampVdVq();
void Delay1uS();
void __attribute__((__interrupt__, auto_psv)) _CNInterrupt(void);
void __attribute__ ((__interrupt__,auto_psv)) _ADCInterrupt(void);
void __attribute__ ((__interrupt__,auto_psv)) _T4Interrupt(void);
void MoveDataFromEEPromToRAM();
void EESaveValues();
void InitializeThrottleAndCurrentVariables();

int main() {

	InitIORegisters();

	MoveDataFromEEPromToRAM();
//	EESaveValues();
	
	InitTimers();  // Now timer1 is running at around 59KHz.
	DelayTenthsSecond(5); // delay 0.5 sec to let voltages settle.

	InitCNModule();
	InitDiscreteADConversions();
	GetVRefs();
	InitializeThrottleAndCurrentVariables();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	
	InitUART2();  // Now the UART is running.
	InitPIStruct();

	// High pedal lockout. Wait until they aren't touching the throttle before starting the precharge sequence.
	do {
		ReadADInputs();
	} while (ADThrottle < savedValues.minRegenPosition || ADThrottle > savedValues.minThrottlePosition);  // I'm using a wig wag hall effect throttle for testing.

	O_LAT_PRECHARGE_RELAY = 1;  // 1 means close the relay.  Now the cap is filling up.
	DelayTenthsSecond(10); // savedValues.prechargeTime);
	// High pedal lockout. Wait until they aren't touching the throttle before closing the main contactor.

	do {
		ReadADInputs();
	} while (ADThrottle < savedValues.minRegenPosition || ADThrottle > savedValues.minThrottlePosition);  // I'm using a wig wag hall effect throttle.

	O_LAT_CONTACTOR = 1;	// close main contactor.
	DelayTenthsSecond(2);   // delay 0.2 seconds, to give the contactor a chance to close.  Then, there will be no current going through the precharge relay.
	O_LAT_PRECHARGE_RELAY = 0;  // open precharge relay once main contactor is closed.

	InitADAndPWM();		// Now the A/D is triggered by the pwm period, and the PWM interrupt is enabled.

	ClearAllFaults();	// The flip flop and desaturation detection faults start up in an unknown state. clear them.
	U2STAbits.OERR = 0; // ClearReceiveBuffer();
	ShowMenu(); 	// serial show menu.
	while(1) {
		ProcessCommand();  // If there's a command to be processed, this does it.  haha.
		if (TMR2 >= 59) {  // TMR3:TMR2 is a 32 bit timer, running at 59KHz.  So, 59 ticks of TMR2 (the low 16 bits) is about 1ms.
			TMR2 = 0;
			counter1ms++;
		}
		// if datastreamPeriod not zero display rt data at specified interval

		if (dataStreamPeriod) {
			if ((counter1ms - timeOfLastDatastreamTransmission) >= dataStreamPeriod) {
				FetchRTData();
				if (showDatastreamJustOnce) {
					dataStreamPeriod = 0;  // You showed it once, now stop the stream.
				}
				// 0         1         2         3         4         5         6         7         8         9
				// 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
				// xxxxx xxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxx   // That's the excel version.
				timeOfLastDatastreamTransmission = counter1ms;
//				int16_to_str(&RTDataString[0], RTData.throttle, 4);
//				u16_to_str(&RTDataString[6], RTData.temperatureBasePlate, 3); 
//				int16_to_str(&RTDataString[10], RTData.IqRef, 4);  // 
//				int16_to_str(&RTDataString[16], RTData.Iq, 4);  // 
//				int16_to_str(&RTDataString[22], RTData.IdRef, 4);  // 
//				int16_to_str(&RTDataString[28], RTData.Id, 4);	// 
//				int16_to_str(&RTDataString[34], RTData.RPS_times16, 4);	// in [-3200, 3200] if RPM is in [-12000,12000]
//				int16_to_str(&RTDataString[40], RTData.batteryCurrentNormalized, 4);
//				u16x_to_str(&RTDataString[46], faultBits, 4);
//				TransmitString(RTDataString);
				// xxxxx
				TransmitString((char *)"123\r\n");
			}
		}

		if (I_PORT_GLOBAL_FAULT == 0) faultBits |= GLOBAL_FAULT;

		// let the interrupt take care of the rest...
		#ifndef DEBUG
		ClrWdt();  // kick the watchdog.  haha.  That's a Fran original.
		#endif
 	}
}

//---------------------------------------------------------------------
// The ADC sample and conversion is triggered by the PWM period.
//---------------------------------------------------------------------
// This runs at 10kHz.
void __attribute__ ((__interrupt__,auto_psv)) _ADCInterrupt(void) {
	static int tempVd = 0;
	static int tempVq = 0;
	static int temp = 0;
	static int rampRate = 1;

	static unsigned int counter10k = 0;
	static unsigned int startTimeInterrupt = 0, elapsedTimeInterrupt = 0;
	static long batteryCurrentLong = 0;
	static long vBetaSqrt3_times32768 = 0;
	static long vAlpha_times32768 = 0;
	static int revCounter = 0;	// revCounter increments at 10kHz.  When it gets to 78, the number of ticks in POSCNT is extremely close to the revolutions per seoond * 16.
								// So, the motor mechanical speed will be computed every 1/128 seconds, and will have a range of [0, 3200], where 3200 corresponds to 12000rpm.	

    IFS0bits.ADIF = 0;  	// Interrupt Flag Status Register. Pg. 142 in F.R.M.

	counter10k++;	
	// ADIF = A/D Conversion Complete Interrupt Flag Status bit.  
	// ADIF = 0 means we are resetting it so that an interrupt request has not occurred.

	startTimeInterrupt = TMR4;

	// so dense... so glorious.  Covers positive and negative RPM.
	// 
	revCounter++;
	if (revCounter >= 78) {  // Compute rpm at 128 times a second.
//		veryOldRPS = oldRPS;
//		oldRPS = RPS_times16;
		RPS_times16 = POSCNT;	// if POSCNT is 0x0FFFF due to THE MOTOR GOING BACKWARDS, RPS_times16 would be -1, since it's of type signed short.  So, it's all good.  Negative RPM is covered.
		POSCNT = 0;
		revCounter = 0;
//		if (RPS_times16 == 0) {  // possibly unplugged encoder.
//			if (veryOldRPS > 50 || veryOldRPS < -50) {  // if there was a collapse of the speed in a matter of 1/64 sec, assume the encoder was unplugged?  What is the behavior near zero rpm?
//														// will the PI loop cause the error to grow out of control and trigger a fault if the rpm "stays at zero" due to it being unplugged?
//				faultBits |= ENCODER_CABLE_UNPLUGGED_FAULT;
//			}
//		}
		if (maxSpeed < RPS_times16) maxSpeed = RPS_times16;
	}
///////////////////////////////////////////////////////////////////
//	Alternative way to find RPM:  velocity += (POSCNT - 2*oldPOSCNT + reallyOldPOSCNT) * 10000); // 2nd derivative difference quotient.  units are ticks per 0.0001 second.  That's a problem.	
// low pass filter this?  it will be nonzero and 0 a lot.  Sort of jumpy and grainy due to the extremely short time interval.  a running average?
// velocity is in ticks/0.0001sec.  Convert to rev/sec * 16:  (ticks / 0.0001sec) * rev/2048tick * 0.0001 * 16 ---> rev/sec*16
// So.... RPS_times16 = 16 * velocity / (2048 * 10000) = 
//	RPS_times16 = 

	// CH0 corresponds to ADCBUF0. etc...
	// CH0=AN7, CH1=AN0, CH2=AN1, CH3=AN2. 
	// AN0 = CH1 = ADThrottle
	// AN1 = CH2 = ADCurrent1
	// AN2 = CH3 = ADCurrent2
	// AN7 = CH0 = ADTemperature

	ADCurrent1 = ADCBUF2;
	ADCurrent2 = ADCBUF3;
	Ia = ADCurrent1;	// CH2 = ADCurrent1
	Ib = ADCurrent2;		// CH3 = ADCurrent2

	Ia -= vRef1;  // vRef1 is just a constant found at the beginning of the program, approximately = 512, that changes the current feedback from being centered at 512 to centered at 0.  It's specific to current sensor #1.
	Ib -= vRef2;  // vRef2 is just a constant found at the beginning of the program, approximately = 512, that changes the current feedback from being centered at 512 to centered at 0.  It's specific to current sensor #2.

	// So, you must change the interval to [-4096, 4096], so as to match the throttle range below to make feedback comparable with commanded current. So...

	Ia <<= 4;	// Ia is now in [-4096, 4096] if it was in  [-256, 256].  In other words, if it was in [-2*LEM Rating, 2*LEM Rating]. 
	Ib <<= 4;

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
			throttleFaultCounter++;
			if (throttleFaultCounter >= THROTTLE_FAULT_COUNTS) {  // don't worry about clamping it.  If there's a throttle fault, you must turn off the controller to clear it.
				faultBits |= THROTTLE_FAULT;
			}
		}
		else {
			if (throttleFaultCounter > 0) {  // Hurray, no fault, so decrement the fault counter if necessary.
				throttleFaultCounter--;
			}
		}
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
	/////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// Clarke transform:
//  	First, take the 3 vectors, 120 degrees apart, and add them to
// 		get a new vector, and project that vector onto the x and y axis.  The x-axis component is called i_alpha.  y-axis component is called i_beta.
// clarke transform, scaled down by 2/3 to simplify it:
// i_alpha = i_a
// 1/sqrt(3) * 2^16 = 37837
	i_alpha = Ia;
	i_beta = __builtin_mulsu(Ib*2 + Ia, 37837u) >> 16;  // 1/sqrt(3) * (i_a + 2 * Ib).  
// End of clarke transform.
////////////////////////////////////////////////////////////////////////////////////////////////
// "ComputeRotorFluxAngle()" uses Id and Iq, found below.  So, initialize them to something.  I'll have Id start as 0, and Iq start as 0.
	ComputeRotorFluxAngle();
//		rotorFluxAngle = 0;	 // set rotorFluxAngle to zero while tuning the PI loop with a locked rotor.
////////////////////////////////////////////////////////////////////////////////////////////////
// Park transform:
	// rotorFluxAngle is in [0, 511].
	// sin(theta + 90 degrees) = cos(theta).
	// I want the 2 angles to be in [0, 511] so I can use the lookup table.
	rotorFluxAnglePlus90 = ((rotorFluxAngle + 128) & 511);  // To advance 90 degrees on a scale of 0 to 511, you add 128, 
															// and then do "& 511" to make it wrap around if overflow occurred.
	cos_theta_times32768 = _sin_times32768[rotorFluxAnglePlus90];  // 
	sin_theta_times32768 = _sin_times32768[rotorFluxAngle];  // 
	// Park Transform:
	// Id = Ialpha*cos(theta) + Ibeta*sin(theta)
	// Iq = -Ialpha*sin(theta) + Ibeta*cos(theta)
	Id = (__builtin_mulss(i_alpha,cos_theta_times32768) + __builtin_mulss(i_beta,sin_theta_times32768)) >> 15; 	
	Iq = (__builtin_mulss(-i_alpha,sin_theta_times32768) + __builtin_mulss(i_beta,cos_theta_times32768)) >> 15; 
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

	if (pi_Id.pwm > MAX_VD_VQ*1024L) {
		pi_Id.pwm = MAX_VD_VQ*1024L;
		faultBits |= PI_OVERFLOW_FAULT;
	}
	else if (pi_Id.pwm < -MAX_VD_VQ*1024L) {
		pi_Id.pwm = -MAX_VD_VQ*1024L;		
		faultBits |= PI_OVERFLOW_FAULT;
	}
	if (pi_Iq.pwm > MAX_VD_VQ*1024L) {
		pi_Iq.pwm = MAX_VD_VQ*1024L;
		faultBits |= PI_OVERFLOW_FAULT;
	}
	else if (pi_Iq.pwm < -MAX_VD_VQ*1024L) {
		pi_Iq.pwm = -MAX_VD_VQ*1024L;		
		faultBits |= PI_OVERFLOW_FAULT;
	}

	Vd = pi_Id.pwm >> 10;
	Vq = pi_Iq.pwm >> 10;
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
			if (IqRef > IqRefRef) IqRef = IqRefRef;
	}
	else if (IqRef > IqRefRef) {
		IqRef -= rampRate;
			if (IqRef < IqRefRef) IqRef = IqRefRef;
	}

// should I have it go instantly to IdRefRef if IdRefRef is smaller in magnitude?
	if (IdRef < IdRefRef) {
		IdRef += rampRate;
			if (IdRef > IdRefRef) IdRef = IdRefRef;
	}
	else if (IdRef > IdRefRef) {
		IdRef -= rampRate;
			if (IdRef < IdRefRef) IdRef = IdRefRef;
	}	

//////////////////////////////////////////////////////////////////////////////////
// Inverse Park Transform:
//	vAlpha_times32768 = (Vd*cos_theta_times32768 - Vq*sin_theta_times32768);	// shift right 15 because sin and cos have been shifted left by 15.
//	vBeta =  (Vd*sin_theta_times32768 + Vq*cos_theta_times32768) >> 15;	//
	vAlpha_times32768 = __builtin_mulss(Vd, cos_theta_times32768) - __builtin_mulss(Vq, sin_theta_times32768);
	v_beta = (__builtin_mulss(Vd, sin_theta_times32768) + __builtin_mulss(Vq, cos_theta_times32768)) >> 15;
//////////////////////////////////////////////////////////////////////////////////
// Now do the inverse Clarke transform, with a scaling factor of 3/2 to simplify it.
//  Va = v_alpha
//	Vb = 1/2*(-v_alpha + sqrt(3)*vBeta)
//  Vc = 1/2*(-v_alpha - sqrt(3)*vBeta);
	v_alpha = vAlpha_times32768 >> 15;
	Va = v_alpha; 
	//vBetaSqrt3_times32768 = (56756*vBeta);  // 56756 = sqrt(3)*(2^15).
	vBetaSqrt3_times32768 = __builtin_mulus(56756u, v_beta);  // 56756 = sqrt(3)*(2^15).

	Vb = (-vAlpha_times32768 + vBetaSqrt3_times32768) >> 16;  // scaled up by 2^15, so shift down by 15.  But you also must divide by 2 at the end as part of the inverse clarke.  So, shift down by a total of 16.
	Vc = (-vAlpha_times32768 - vBetaSqrt3_times32768) >> 16;
///////////////////////////////////////////////////////////////////////////////////
// SpaceVectorModulation:
// You now turn Va, Vb, Vc into duties for PDC1, PDC2, PDC3.  

	if (faultBits == 0) {
		SpaceVectorModulation();
	}
	else {
		PDC1 = 0;
		PDC2 = 0;
		PDC3 = 0;
	}

//	if ((counter10k & 7) == 7) { // 158*2 Hz.  2.2 seconds of data storage.
//		bigArrayCounter++;
//		if (bigArrayCounter > 700) {
//			bigArrayCounter = 0;
//			Nop();
//			Nop();
//			Nop();
//			Nop();
//		}
//		bigArray[bigArrayCounter] = Iq;
//	}
	elapsedTimeInterrupt = TMR4 - startTimeInterrupt;
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
	static unsigned int rotorFluxAngle_times128 = 0;  // For fine control.
	static long magCurrChange = 0;
	static long slipSpeedNumerator = 0;
	static int slipSpeedRPS_times16 = 0;
	static int rotorFluxRPS_times16 = 0;
	static int angleChange_times128 = 0;
	static long magnetizingCurrentFine = 0;
	static int magnetizingCurrent = 0;

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
//	magnetizingCurrent += ((rotorTimeConstantArray1[savedValues2.rotorTimeConstantIndex] * (Id - magnetizingCurrent))) >> 18;
///////////////////////////////////////////////////////////////////////////
	magCurrChange = __builtin_mulus(rotorTimeConstantArray1[savedValues2.rotorTimeConstantIndex], (Id - magnetizingCurrent));
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
		slipSpeedNumerator = __builtin_mulus(rotorTimeConstantArray2[savedValues2.rotorTimeConstantIndex], Iq) >> 7; // Must scale down by 2^11 if you want units to be rev/sec.  But that's too grainy.  So, let's only scale down by 2^7 so you get slip speed in rev/sec * 16, rather than just rev/sec
		if (magnetizingCurrent > 0) {
			if (slipSpeedNumerator > 0L) {
				if (slipSpeedNumerator > __builtin_mulss(magnetizingCurrent,MAX_SLIP_SPEED_RPS_TIMES16)) {
					slipSpeedRPS_times16 = MAX_SLIP_SPEED_RPS_TIMES16;  // must be positive slip speed
				}
				else {
					slipSpeedRPS_times16 = __builtin_divsd(slipSpeedNumerator, magnetizingCurrent);  // must be positive slip speed.
				}
			}
			else { // if (slipSpeedNumerator < 0).  It can't be zero, since that case was already accounted for.
				if (-slipSpeedNumerator > __builtin_mulss(magnetizingCurrent,MAX_SLIP_SPEED_RPS_TIMES16)) {
					slipSpeedRPS_times16 = -MAX_SLIP_SPEED_RPS_TIMES16;  // must end with negative slip speed.
				}
				else {
					slipSpeedRPS_times16 = __builtin_divsd(slipSpeedNumerator, magnetizingCurrent);  // this is negative result.
				}
			}
		}
		else {  // magnetizingCurrent < 0
			if (slipSpeedNumerator > 0) {  // POS / NEG = NEG.
				if (slipSpeedNumerator > __builtin_mulss(-magnetizingCurrent,MAX_SLIP_SPEED_RPS_TIMES16)) {  //
					slipSpeedRPS_times16 = -MAX_SLIP_SPEED_RPS_TIMES16;  // must be negative slip speed.  pos/neg = neg.
				}
				else {
					slipSpeedRPS_times16 = __builtin_divsd(slipSpeedNumerator, magnetizingCurrent);  // must be negative slip speed.
				}
			}
			else {  // slipSpeedNumerator < 0, magnetizingCurrent < 0.  So, slipSpeed will be positive below.
				if (slipSpeedNumerator < __builtin_mulss(magnetizingCurrent,MAX_SLIP_SPEED_RPS_TIMES16)) {  // if it's more negative than the right hand side...
					slipSpeedRPS_times16 = MAX_SLIP_SPEED_RPS_TIMES16;  // must end with positive slip speed.  neg / neg = pos.
				}
				else {
					slipSpeedRPS_times16 = __builtin_divsd(slipSpeedNumerator, magnetizingCurrent);  // this is positive result.
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
		if (RPS_times16 > MAX_RPS_TIMES16) {  // You are about to friggen grenade your motor.  Slow down.
			faultBits |= RPS_FAULT;
		}
		else if (RPS_times16 < -MAX_RPS_TIMES16) {
			faultBits |= RPS_FAULT;
		}
		rotorFluxRPS_times16 = savedValues2.numberOfPolePairs*RPS_times16 + slipSpeedRPS_times16;  // There's no danger of integer overflow, so just do normal multiply.  Larger number of pole pairs means lower rpm, so it all evens out.

//;  Rotor flux angle (radians):
//;     AngFlux = AngFlux + fLoopPeriod * 2 * pi * VelFluxRPS
//;  Rotor flux angle (0 to 511 ticks):
//      AngFlux = AngFlux + fLoopPeriod * 512 * VelFluxRPS.  
//   Now, I don't have VelFluxRPS.  Too darn grainy.  I found rotorFluxRPS_times16 above.  So.....
//      AngFlux = AngFlux + fLoopPeriod * 32 * rotorFluxRPS_times16, because 32 * 16 = 512.
//   OK, fLoopPeriod = 0.0001 sec.  I don't want to divide here, so let's do a trick that is much faster.
//   0.0001sec * 32 * 2^24 = 53687.09.  So, I'll just multiply by 53687, and shift down by 2^24 afterwards.  Actually, let's keep 2^7 worth of extra resolution, because angleChange was too grainy before.
//	angleChange_times128 = (53687u * rotorFluxRPS_times16) >> 17;  // must shift down by 24 eventually, but let's keep a higher resolution here.  So, only shift down by 17.  Keeping 7 bits.
	angleChange_times128 = __builtin_mulus(53687u, rotorFluxRPS_times16) >> 17;  // must shift down by 24 eventually, but let's keep a higher resolution here.  So, only shift down by 17.  Keeping 7 bits.
	// angleChange_times128 must be in [-5242, 5242] assuming all the clamping I'm doing above.
	rotorFluxAngle_times128 += (unsigned)angleChange_times128;  // if it overflows, so what.  it will wrap back around.  To go from [0,65536] --> [0,512], divide by by 128.  higher resolution rotor flux angle saved here.
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
	i = __builtin_divsd(((long)small) << 10, big);   // small * 1024 / big gives an index from 0 to 1024 inclusive.
	fastDist = (unsigned int)((small + big - (small >> 1) - (small >> 2)) + (small >> 4));
	// fastDist must be in [0, 19687]
	r = (__builtin_muluu(distCorrection[i], fastDist) >> 15);
	if (r > R_MAX) {
		scale = __builtin_divud(R_MAX_TIMES_65536, r);
		small = (int)(__builtin_mulsu(small, scale) >> 16);
		big = (int)(__builtin_mulsu(big, scale) >> 16);
		IqRefNew = (int)(__builtin_mulsu(IqRef, scale) >> 16);
		IdRefNew = (int)(__builtin_mulsu(IdRef, scale) >> 16);
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
void Delay1uS() {  // Assuming 15MIPs.
	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop(); Nop();	Nop(); Nop(); Nop();
}


// Make sure this is only called after you know the previous conversion is done.  Right now, the conversion is taking ?? uS, so call this
// when ?? us has gone by after starting "StartADConversion()".
// Run this just before starting the next ADConversion.  It will be a lag, but I'll know what's what.  haha.
void GrabADResults() {
	 ADTemperature = ADCBUF0;
	 ADThrottle = ADCBUF1;
	 ADCurrent1 = ADCBUF2;
	 ADCurrent2 = ADCBUF3;
	// AN0 = CH1 = ADThrottle
	// AN1 = CH2 = ADCurrent1
	// AN2 = CH3 = ADCurrent2
	// AN6 = CH0 = ADTemperature
}
// This is the slow one, when you just want to start a conversion, and wait for the results.
void ReadADInputs() {
	ADCON1bits.SAMP = 1; // start sampling ...
	Delay(256); // for 4mS
	ADCON1bits.SAMP = 0; // Stop sampling, and start converting.
	while (!ADCON1bits.DONE) {
	#ifndef DEBUG
		ClrWdt(); // conversion done?
	#endif
	}
	GrabADResults();
}

void GetVRefs() {
	int i, sum1 = 0, sum2 = 0; //, sum3 = 0;

	for (i = 0; i < 32; i++) {
		ReadADInputs();
		sum1 += ADCurrent1;
		sum2 += ADCurrent2;
	}
	vRef1 = (sum1 >> 5);
	vRef2 = (sum2 >> 5);

	if (vRef1 < 512 - 50 || vRef1 > 512 + 50 || 
		vRef2 < 512 - 50 || vRef2 > 512 + 50) {
		faultBits |= VREF_FAULT;
	}
}

void InitTimers() {
	T1CON = 0;  // Make sure it starts out as 0.
	T1CONbits.TCKPS = 0b11;  // prescale of 256.  So, timer1 will run at 58593.75Hz if Fcy is 15.000MHz.
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

// Assuming a 15.000MHz clock, one tick is 1/58594 seconds.
void Delay(unsigned int time) {
	unsigned int temp;
	temp = TMR1;	
	while (TMR1 - temp < time) {
	#ifndef DEBUG
		ClrWdt();
	#endif
	}
}
void DelaySeconds(unsigned int time) {
	int i;
	for (i = 0; i < time; i++) { 
		Delay(58594u);  // 1 second.
	}
}
void DelayTenthsSecond(unsigned int time) {
	int i;
	for (i = 0; i < time; i++) { 
		Delay(5859);  // 58594 ticks in Delay is 1 second.  So, 1/10 of that.
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

void InitDiscreteADConversions() {
	// ============= ADC - Measure 
	// ADC setup for simultanous sampling
	// AN0 = CH1 = Throttle
	// AN1 = CH2 = current1
	// AN2 = CH3 = current2
	// AN7 = CH0 = temperatureBasePlate

	// Note:  F.R.M. means Family Resource Manual for the dsPIC30F family of microcontrollers.
   
    ADCON1bits.FORM = 0;  // integer in the range 0-1023
    ADCON1bits.SSRC = 0;  // Clearing ADCON1bits.SAMP bit ends sampling and starts conversion.

    // Simultaneous Sample Select bit (only applicable when CHPS = 01 or 1x)
    // Samples CH0, CH1, CH2, CH3 simultaneously (when CHPS = 1x)
    // Samples CH0 and CH1 simultaneously (when CHPS = 01)
    ADCON1bits.SIMSAM = 1; 

    // Sampling begins immediately after last conversion completes. 
    // SAMP bit is auto set.
	// ADCON1bits.ASAM = 1;  
    ADCON1bits.ASAM = 0;

/*
ADPCFG = 0xFFFB; // all PORTB = Digital; RB2 = analog
ADCON1 = 0x0000; // SAMP bit = 0 ends sampling ...
// and starts converting
ADCHS = 0x0002; // Connect RB2/AN2 as CH0 input ..
// in this example RB2/AN2 is the input
ADCSSL = 0;
ADCON3 = 0x0002; // Manual Sample, Tad = internal 2 Tcy
ADCON2 = 0;
ADCON1bits.ADON = 1; // turn ADC ON
while (1) // repeat continuously
{
ADCON1bits.SAMP = 1; // start sampling ...
DelayNmSec(100); // for 100 mS
ADCON1bits.SAMP = 0; // start Converting
while (!ADCON1bits.DONE); // conversion done?
ADCValue = ADCBUF0; // yes then get ADC value
}
*/
    // Pg. 407 in F.R.M.
    // Samples CH0, CH1, CH2, CH3 simultaneously when CHPS = 1x
    ADCON2bits.CHPS = 0b10; // VCFG = 000; This selects the A/D High voltage as AVdd, and A/D Low voltage as AVss.
						 // SMPI = 0000; This makes an interrupt happen every time the A/D conversion process is done (for all 4 I guess, since they happen at the same time.)
						 // ALTS = 0; Always use MUX A input multiplexer settings
						 // BUFM = 0; Buffer configured as one 16-word buffer ADCBUF(15...0)

    // Pg. 408 in F.R.M.
    // A/D Conversion Clock Select bits = 4 * Tcy.  (7+1) * Tcy/2 = 4*Tcy.
	// The A/D conversion of 4 simultaneous conversions takes 4*12*A/D clock cycles.  The A/D clock is selected to be 32*Tcy.
    // So, it takes about 96us to complete if ADCS = 63.
	ADCON3bits.ADCS = 31;  // 

    // ADCHS: ADC Input Channel Select Register 
    // Pg. 409 in F.R.M.
    // CH0 positive input is AN7, which is temperatureBasePlate.
    ADCHSbits.CH0SA = 7;
    	
	// CH1 positive input is AN0, CH2 positive input is AN1, CH3 positive input is AN2.
	ADCHSbits.CH123SA = 0;

	// CH0 negative input is Vref-.
	ADCHSbits.CH0NA = 0;

	// CH1, CH2, CH3 negative inputs are Vref-, which is AVss, which is Vss.  haha.
    ADCHSbits.CH123NA = 0;

    // Turn on A/D module
    ADCON1bits.ADON = 1; // Pg. 416 in F.R.M.  Under "Enabling the Module"
						 // ** It's important to set all the bits above before turning on the A/D module. **
						 // Now the A/D conversions start happening once ADON == 1.
	IEC0bits.ADIE = 0;	 // DISABLE interrupts for when an A/D conversion is complete. Pg. 148 of F.R.M.  
	return;
}

void InitADAndPWM() {
    ADCON1bits.ADON = 0; // Pg. 416 in F.R.M.  Under "Enabling the Module".  Turn it off for a moment.

	// PWM Initialization

	PTPER = 749;	// 15,000,000/((PTPER + 1)*2) = 10KHz.
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
	DTCON1bits.DTAPS = 0b10;  // CLOCK Period is 4*Tcy.
	DTCON1bits.DTA = 15;  	// 15 CLOCK periods.  That is 4*15*Tcy = 4uS of dead time assuming 15MHz.  Seems to work OK.

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
    // So, it takes about 4*12*16*Tcy to complete 4 A/D conversions. That's 51.2uS. The pwm period is 100uS, since it's 10kHz nominal.
	ADCON3bits.ADCS = 15;  // 16Tcy.


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
								// Fcy = 15MHz.  3*16Tcy is the minimum pulse width required for QEA or QEB to change from high to low or low to high.
								// You can do at most 15,000,000/(3*16) = 312,500 of those pulses per second.  So, you can have at most
								// 312,500 * 4 clock counts per second.  That means the maximum detectable RPM is:
								// x rev/sec * 2096 clockCounts/Rev = 312,500 * 4 clockCounts/sec.
								// So, x = 596 rev/sec = 35782 RPM.
								// 
	DFLTCONbits.IMV = 0b00;  	// INDEX pulse happens when QEA is low.  Irrelevant.  I'm not using the index pulse for the AC induction motor.

	MAXCNT = 0xFFFF; 	// reset the counter each time it reaches maxcnt.  It will reset anyway won't it?  I mean, after 0xFFFF is 0x0000!  haha.
						// Use this for the AC controller.  It's easier to measure speed of rotor with this setting, if there's no INDEX signal on the encoder.
	POSCNT=0;  // How many ticks have gone by so far?  Starts out as zero anyway.  It's safe to write to it though.

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

	EEDataInRam2[0] = savedValues2.rotorTimeConstantIndex;
	EEDataInRam2[1] = savedValues2.numberOfPolePairs;
	EEDataInRam2[2] = savedValues2.maxRPM;
	EEDataInRam2[3] = savedValues2.throttleType;
	EEDataInRam2[4] = 0;
	EEDataInRam2[5] = 0;
	EEDataInRam2[6] = 0;
	EEDataInRam2[7] = 0;
	EEDataInRam2[8] = 0;
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
	savedValues2.spares[4] = 0;
	savedValues2.spares[5] = 0;
	savedValues2.spares[6] = 0;
	savedValues2.spares[7] = 0;
	savedValues2.spares[8] = 0;
	savedValues2.spares[9] = 0;
	savedValues2.spares[10] = 0;

	if (EEDataInRam3[15] == CRC3) {
		savedValues2.rotorTimeConstantIndex = EEDataInRam3[0];		// 
		savedValues2.numberOfPolePairs = EEDataInRam3[1];						// 
		savedValues2.maxRPM = EEDataInRam3[2];		// 
		savedValues2.throttleType = EEDataInRam3[3];
		savedValues2.crc = EEDataInRam3[15];					// 
	}
	else if (EEDataInRam4[15] == CRC4) {
		savedValues2.rotorTimeConstantIndex = EEDataInRam4[0];		// 
		savedValues2.numberOfPolePairs = EEDataInRam4[1];						// 
		savedValues2.maxRPM = EEDataInRam4[2];		// 
		savedValues2.throttleType = EEDataInRam4[3];		
		savedValues2.crc = EEDataInRam4[15];					// 
	}
	else {	// There wasn't a single good copy.  Load the default configuration.
		savedValues2 = savedValuesDefault2;
		savedValues2.crc =    savedValues2.rotorTimeConstantIndex + 
							savedValues2.numberOfPolePairs + 
							savedValues2.maxRPM + 
							savedValues2.throttleType; // only go to 4 because at the moment, I only have 4 useful values in savedValues2 struct.
	}
}

void InitializeThrottleAndCurrentVariables() {
	// Input:  currentSensorVoltsPerAmp
	// Output:  maxRefNormalized initialied.
	//			minRefNormalized initialized
	// Ex:  MaxMotorAmps = 300
	// 	
	int ADTicksPerAmp_times128 = 0;
	long totalADTicks_times128 = 0;

	//ADTicksPerAmp_times128 = 128 * (1024/5) / currentSensorAmpsPerVolt; // this number will usually be less than 1!  1024 A/D ticks per 5 volts.
	//ADTicksPerAmp_times128 = 26214 / currentSensorAmpsPerVolt;
	ADTicksPerAmp_times128 = __builtin_divsd(26214L,savedValues.currentSensorAmpsPerVolt);

	//totalADTicks_times128 = (ADTicksPerAmp_times128 * savedValues.maxMotorRegenAmps);
	totalADTicks_times128 = __builtin_mulss(ADTicksPerAmp_times128, savedValues.maxMotorAmpsRegen);
	maxMotorCurrentNormalizedRegen = totalADTicks_times128 >> 3;  // to go from [-256*128, 256*128] to [-4096, 4096], divide by 8. I'm assuming a current sensor acceptable range from [1.25v, 3.75v].
	if (maxMotorCurrentNormalizedRegen > 4096) maxMotorCurrentNormalizedRegen = 4096;

	//totalADTicksRegen_times128 = (ADTicksPerAmp_times128 * savedValues.maxMotorRegenAmps);
	totalADTicks_times128 = __builtin_mulss(ADTicksPerAmp_times128, savedValues.maxMotorAmps);
	maxMotorCurrentNormalized = totalADTicks_times128 >> 3;  // to go from [-256*128, 256*128] to [-4096, 4096], divide by 8. I'm assuming a current sensor acceptable range from [1.25v, 3.75v].
	if (maxMotorCurrentNormalized > 4096) maxMotorCurrentNormalized = 4096;

	
	totalADTicks_times128 = __builtin_mulss(ADTicksPerAmp_times128, savedValues.maxBatteryAmps);	; // go from amps to normalized.  Normalized is following the above process.
	maxBatteryCurrentNormalized = totalADTicks_times128 >> 3; // // to go from [-256*128, 256*128] to [-4096, 4096], divide by 8. I'm assuming a current sensor acceptable range from [1.25v, 3.75v].
	totalADTicks_times128 = __builtin_mulss(ADTicksPerAmp_times128, savedValues.maxBatteryAmpsRegen);	; // go from amps to normalized.  Normalized is following the above process.
	maxBatteryCurrentNormalizedRegen = totalADTicks_times128 >> 3; // // to go from [-256*128, 256*128] to [-4096, 4096], divide by 8. I'm assuming a current sensor acceptable range from [1.25v, 3.75v].

//	normalizedToAmpsMultiplier = 4096NormalizedTicks/256ADTicks * 1024ADTicks/5volt * volt/currentSensorAmpsPerVolt amp down to 20, or 4096 down to 1200 with LEM 600.
	
}
