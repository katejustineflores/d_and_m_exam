#include "variant.h"
#include <due_can.h>
#include <SD.h>
#include <Wire.h>
//#include <Adafruit_INA219.h>
#include <avr/pgmspace.h>
//#include <XBee.h>
#include <stdbool.h>
//#include <RTCDue.h>

#include <DueFlashStorage.h>
DueFlashStorage dueFlashStorage;


#define ATCMD     "AT"
#define ATECMDTRUE  "ATE"
#define ATECMDFALSE "ATE0"
#define ATRCVCAN    "ATRCV"
#define ATSNDCAN    "ATSND"
#define ATGETSENSORDATA    "ATGSDT"
#define ATSNIFFCAN  "ATSNIFF"
#define ATDUMP    "ATDUMP"
#define OKSTR     "OK"
#define ERRORSTR  "ERROR"
#define ATSD      "ATSD"
#define DATALOGGER Serial1
#define powerM Serial2


#define VERBOSE 0
#define RELAYPIN 44
#define LED1 48
#define POLL_TIMEOUT 5000
#define BAUDRATE 9600
#define ARQTIMEOUT 30000


#define ENABLE_RTC 0
#define CAN_ARRAY_BUFFER_SIZE 100

// #define comm_mode "ARQ" // 1 for ARQ, 2 XBEE
char comm_mode[5] = "";

const char base64[64] PROGMEM = {'A','B','C','D','E','F','G','H','I','J','K','L','M',
'N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h',
'i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2',
'3','4','5','6','7','8','9','+','/'};

//XBee xbee = XBee();
long timestart = 0;
long timenow = 0;
long arq_start_time = 0;
// Group: Global Variables
// These are the variables declared globally.

/* 
  Variable: b64
  global variable that turns on the b64 data representation mode. 0 by default.

  An AT Command can be used to toggle this. *AT+SWITCHB64* command toggles on and off
  the b64 data representation. 

  Variable declaration:
  --- Code
  uint8_t b64 = 0;
  --- 
*/
uint8_t b64 = 0;

/* 
  Variable: payload
  global variable uint8_t array that will hold the data from xbee

  Variable declaration:
  --- Code
  uint8_t payload[200];
  --- 
*/
uint8_t payload[200];

//XBeeAddress64 addr64 = XBeeAddress64(0x00, 0x00); //sets the data that it will only send to the coordinator
//ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
//ZBTxStatusResponse txStatus = ZBTxStatusResponse();
//XBeeResponse response = XBeeResponse();
//ZBRxResponse rx = ZBRxResponse();

int g_chip_select = SS3;
uint8_t datalogger_flag = 0;


/* 
  Variable: xbee_response
  global variable char array that will hold the command frame received from the xbee

  Variable declaration:
  --- Code
  char xbee_response[200];
  --- 
*/
char xbee_response[200];

/*
  Variable: g_gids
  2 dimensional integer array that stores the links the unique id with the geographic id
  
  Variable declaration:
  --- Code
  int g_gids[40][2];
  ---
*/
int g_gids[40][2];

/*
  Variable: g_num_of_nodes
  integer that stores the number of nodes. This variable is overwritten by *<process_config_line>*. This variable is also used by *<init_char_arrays>*.

  Variable declaration:
  --- Code
  uint8_t g_num_of_nodes = 40;
  --- 
*/
uint8_t g_num_of_nodes = 40;

/*
  Variable: g_mastername
  global char array that holds the 5 char mastername. This variable is overwrittern by *<process_config_line>*. This variable defaults to "XXXXX".

  *SD Card Config Line* usage:
  --- Code
  mastername = INATA
  ---
*/
char g_mastername[6] = "XXXXX";

/*
  Variable: g_turn_on_delay
  integer that determines the delay in centi seconds ( ie. 100 centiseconds = 1 sec ) introduced by the firmware after the column switch turns on. 
  This ensures the voltage stability for the column.

  *SD Card Config Line* usage:
  --- Code
  turn_on_delay = 100
  ---
*/
uint8_t g_turn_on_delay = 10;

/*
  Variable: g_sensor_version
  integer that determines the sensor version ( 1, 2, or 3 ). This variable is overwrittern by *<process_config_line>*. 

  *SD Card Config Line* usage:
  --- Code
  sensorVersion = 3
  ---*/
//uint8_t g_sensor_version = dueFlashStorage.read(0);
uint8_t g_sensor_version = 4;

/*
  Variable: g_datalogger_version
  integer that determines the datalogger version ( 2 ARQ or 3 Regular,V3 ). This variable is overwrittern by *<process_config_line>*. 
  
  Variable declaration:
  --- Code
  uint8_t g_datalogger_version = 3;
  ---
*/
uint8_t g_datalogger_version = 2;

/* 
  Variable: broad_timeout
  integer that determines the timeout duration of broadcast (in milliseconds ). This variable is overwrittern by *<process_config_line>*. 

  *SD Card Config Line* usage:
  --- Code
  brodcast_timeout = 3000  ---
*/
int broad_timeout = 3000;

/* 
  Variable: has_piezo
  boolean variable that decides the sampling of the piezometer readout board

  *SD Card Config Line* usage:
  --- Code
  PIEZO = 1
  ---
*/
bool has_piezo = false;

/* 
  Variable: g_sampling_max_retry
  integer that determines the number of column sampling retry
  
  *SD Card Config Line* usage:
  --- Code
  sampling_max_retry = 3
  ---
*/
int g_sampling_max_retry = 1;

/* 
  Variable: g_delim
  global char array that holds the delimiter used for separating data from different commands
  
  Variable declaration:
  --- Code
  char g_delim[2] = "~";
  ---
*/
char g_delim[2] = "~";
// CAN-related
char g_temp_dump[2500];
char g_final_dump[5000];
char g_no_gids_dump[2500];

/* 
  Variable: text_message
  char array that holds the formatted messages to be sent

  Intial Variable declaration:
  --- Code
  char text_message[5000];
  ---
  
  See Also:

  <build_txt_msgs>
*/
char text_message[10000];

String g_string;
String g_string_proc;

CAN_FRAME g_can_buffer[CAN_ARRAY_BUFFER_SIZE];

/*
  Variable: g_timestamp
  global String that holds the timestamp.This variable is overwritten by the timestamp from the string sent by the ARQ. This variable defaults to "TIMESTAMP000".

  Intial Variable declaration:
  --- Code
 String g_timestamp = "TIMESTAMP000";
  ---
*/
// String g_timestamp = "180607142000";
String g_timestamp = "TIMESTAMP000";

//current sensor
//Adafruit_INA219 ina219;
bool ate=true;

//--------------------------------------------------------------------------------------------------------


//Function: turn_on_column
// Assert GPIO ( defined by RELAYPIN ) high to turn on sensor column.
void turn_on_column(){
  digitalWrite(RELAYPIN, HIGH);
  digitalWrite(LED1,HIGH);
  delay(g_turn_on_delay);
}

//Function: turn_off_column
// Assert GPIO ( defined by RELAYPIN ) low to turn off sensor column.
void turn_off_column(){
  digitalWrite(RELAYPIN, LOW);
  digitalWrite(LED1, LOW);
  delay(1000);
}


//=============================================================================================================
void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUDRATE);
  DATALOGGER.begin(9600);
  // powerM.begin(9600);
  pinMode(RELAYPIN, OUTPUT);
  pinMode(LED1, OUTPUT);
  init_can();
  init_char_arrays();
  //init_gids();
  //init_sd(); 
  //open_config();
  //print_stored_config();
  //Serial.println("commands: a - accel 1 \t b - accel 2 \t c - change sensor version");
}

void loop() {
  // put your main code here, to run repeatedly:
//    delay(1000);

  //char x[2] = {};

if (Serial.available() > 0){
  //Serial.println("commands: a - accel 1 \t b - accel 2");
  //char serbuffer[2] = {};
  char  x = Serial.read();
  //Serial.readBytesUntil('\n', x, 3);

  //if (strstr(x, "a")) {
  if (x == 'a'){
    //Serial.print("Sensor Version:");Serial.println(g_sensor_version);
    Serial.print("Accel 1:");
    switch (g_sensor_version){
      case 2:
        get_data(32,1,g_final_dump);
        break;
      case 3:
        get_data(11,1,g_final_dump);
        break;
      case 4:
        get_data(41,1,g_final_dump);
        break;
      case 5:
        get_data(51,1,g_final_dump);
        break;      
    }
    //Serial.println("commands: a - accel 1 \t b - accel 2 \t c - change sensor version");
    }
  
  //else if (strstr(x, "b")) {
  else if(x == 'b'){
    //Serial.print("Sensor Version:");Serial.println(g_sensor_version);
    Serial.print("Accel 2:");
    switch (g_sensor_version){
      case 2:
        get_data(33,1,g_final_dump);
        break;
      case 3:
        get_data(12,1,g_final_dump);
        break;
      case 4:
        get_data(42,1,g_final_dump);
        break;
      case 5:
        get_data(52,1,g_final_dump);
        break;
    }
    //Serial.println("commands: a - accel 1 \t b - accel 2 \t c - change sensor version");
    }
  
  //else if (strstr(x, "c")) {
 /* else if(x == 'c'){
    String serbuffer;
    Serial.setTimeout(2000);
    Serial.print("Current version:");Serial.println(g_sensor_version);
    Serial.println("change version");
    Serial.print("Enter version:");
    
    serbuffer = Serial.readStringUntil('\n');
    
    Serial.println(serbuffer.toInt());
    if (serbuffer.toInt()!=0){
      dueFlashStorage.write(0,serbuffer.toInt());
    }
    g_sensor_version = dueFlashStorage.read(0);

    //Serial.println("commands: a - accel 1 \t b - accel 2 \t c - change sensor version");
    }*/

  }

}
