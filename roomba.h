/*
 * roomba commands, data from
 * http://www.irobot.lv/uploaded_files/File/iRobot_Roomba_500_Open_Interface_Spec.pdf
 */

/* Initialisation commands */
#define ROOMBA_RESET	7	/* undocumented */
#define ROOMBA_START	128
#define ROOMBA_BAUD	129

/* Mode commands */
#define ROOMBA_SAFE	131
#define ROOMBA_FULL	132
#define ROOMBA_POWER	133

/* Cleaning commands */
#define ROOMBA_SPOT	134
#define ROOMBA_CLEAN	135
#define ROOMBA_MAX	136
#define ROOMBA_DOCK	143

/* Schedule commands */
#define ROOMBA_SCHEDULE	167
#define ROOMBA_SETTIME	168

/* Binary LEDs */
#define ROOMBA_LED	139
#define ROOMBA_LED_DEBRIS	0x1
#define ROOMBA_LED_SPOT		0x2
#define ROOMBA_LED_DOCK		0x4
#define ROOMBA_LED_CHECK	0x8
