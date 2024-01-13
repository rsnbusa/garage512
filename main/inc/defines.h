#ifndef DEFINES_H
#define DEFINES_H

#define DEBB

#define DISPLAY                 // if HW hast OLED... if ENABLE but no OLED connected will run ut of heap... very weird
// #define FASTL                   //if HW has a LED STrip

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define DELTASK(a) if(a) { vTaskDelete(a);a=NULL;}
#define START                   true
#define STOP                    false
#define NODELAY                 0
#define NUM_CMDS                16

#define NEWTASKDELAY            200         // wait this amount before working in own task features
#define NODENAME                "MaryL"
#define WORKTIME                1000        // used to change during development from seconds to whatever less or more

// Heap sizes for diffeerent tasks 
#define FLEDHEAP                4096
#define STATEHEAP               8192
#define MINORHEAP               1024

#define RELAY                   (33 )         // Door Rleay
#define SYSLED                  (19)          //Sytem activity led
#define HALOP                   (27)          // Open Position Door sensor
#define HALCL                   (14)          // Closed position Door sensor
#define CENTINEL                0x12345678
#define DLED                    (18)          // Light for door open/close
#define EXTP                    (21)          //External sensor like Camera control etc
#define RESETP                  0
#define LASER                   (22)          // Turn On LINE BREAK sensor
#define IFR                     (23)          // BreakLine sensor
#define SDA                     (16)
#define SCL                     (17)
#define I2CADD                  (0x3C)
#define PWRPIN                  (04)         //power pin 

#define IFRTEST                 (32)
#define IFRTESTSAMPLE           (25)
#define SSR                     (21)

#define ON                      1
#define OFF                     0
#define RELAYTRIGGER            1000
#define POWER_ON                0
#define POWER_OFF               1
#define POWER_RESET             2
#define POWERDELAY               (10000)
#define RESTARTMOTOR            (2000)              // 3 secs for the Controller Motor to get ready
#define POWERGUARD              (360000)            // 6 minutes guard time
// Lqueue cmds
#define OPENINGTIMER            TOPEN       //from the enumeration in typedef -
#define CLOSETIMER              TCLOSE
#define REPEATTIMER             TREPEAT
#define RELAYTIMER              TRELAY
#define RELAYWAIT               TREPEATW
#define STRIPOWER               TDISPLAY
#define BREAKTIMER              TBREAK
#define OPENHAL                 7
#define CLOSEHAL                8
#define UNK                     9
#define LASERBREAK              10
#define EXTSENSOR               11


#define BLINKOPENING            150
#define BLINKOPEN               100
#define BLINKCLOSING            50

// TIMERS
#define CLOSETIME               20000        //1
#define CANCELTIME              20000        //2
#define REPEATTIME              20000        //3
#define WAITTIME                20000        //4


#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_HIGH_SPEED_MODE
#define LEDC_OUTPUT_IO          LASER // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           (LEDC_TIMER_10_BIT) // Set duty resolution to 11 bits
#define LEDC_FREQUENCY          (38000) // Frequency in Hertz. Set frequency at 5 kHz
#define DEFAULT_CYCLE           (0.50)  // 50% cycle

#define LOCALTIME               "GMT+5"

// Defaults for Erase Config and KBD
#define KCYCLE                  (25)
#define KFADEBR                 (64)
#define KFADEW                  (50)
#define KFADESTEPS              (2)
#define KARROWBR                (128)
#define KARROWPACE              (40)


//Defaults Timers...good defaults espcially for PWM and IFR sensors
#define KOPENINGT               (60)
#define KESPERA                 (30)
#define KCLOSET                 (60)
#define KREPEATT                (60)
#define KDISPLAYTIMER           (120)
#define KREPEATW                (30)
#define KBREAKTIMER             (180)
#define KPWMTIMER               (600)
#define KIFRBREAK               (1000)
#define KWAITHAL                (300)
#define KPWMCOOL                (100)
#define KPWMBREATH              (60)
#define KFREQUENCY              (38000)
#define KPWMON                  (600)
#define MAXBLKCOUNT             (4)


//Rainmaker Device Sensor names
#define CMD0                    "Remote"
#define CMD1                    "DoorT"
#define CMD2                    "Reset"
#define CMD3                    "Guard"
#define CMD4                    "CloseBreak"
#define CMD5                    "KeepOpen"
#define CMD6                    "DutyT"
#define CMD7                    "DoorState"
#define CMD8                    "name"
#define CMD9                    "Align"
#define CMD10                   "ON"
#define CMD11                   "PowerOff"
#define CMD12                   "PowerOn"
#define CMD13                   "Samp"
#define CMD14                   "Breath"

// DoorBits
const static int OPENBREAK_BIT   		    = BIT0;
const static int OPENINGBREAK_BIT   		= BIT1;
const static int KOPEN_BIT 				    = BIT2;
const static int CLOSEBREAK_BIT 		    = BIT3;

//30	Black
//31	Red
//32	Green
//33	Yellow
//34	Blue
//35	Magenta
//36	Cyan
//37	White

#define CLOSEDSTATE						"\e[36m[CLOSED]\e[0m"               //blue
#define OPENINGSTATE					"\e[35m[OPENING]\e[0m"              // Magenta
#define OPENEDSTATE						"\e[32m[OPENED]\e[0m]"              //Green
#define CLOSINGSTATE					"\e[33m[CLOSING]\e[0m"              //Yellow
#define ONOFFSTATE						"\e[37m[ONOFF]\e[0m"                //White
#define UNKNOWSTATE						"\e[31m[UNK]\e[0m"                  //Red
#define OFFLINESTATE    				"\e[37m[OFFLINE]\e[0m"              //White
#define SYSTEM          				"\e[90;3;4m\t[SYSTEM]\e[0m"         //Gray
#define KBD          				    "\e[31m[KBD]\e[0m"                  //Red
#define WEB          				    "\e[33;1m[APP]\e[0m"                  //Yellow


// #define BLACKC							"\e[30m"
// #define BLUE							"\e[34m"
// #define CYAN							"\e[36m"
// #define GRAY							"\e[90m"
// #define GREEN							"\e[32m"
// #define LGREEN							"\e[92m"
// #define LRED							"\e[91m"
// #define LYELLOW							"\e[93m"
// #define MAGENTA							"\e[35m"
// #define RED								"\e[31m"
// #define RESETC							"\e[0m"
// #define WHITEC							"\e[37m"
// #define YELLOW							"\e[33m"

#endif


