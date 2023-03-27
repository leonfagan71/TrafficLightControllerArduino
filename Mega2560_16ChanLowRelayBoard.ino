// C++ code
//
#include <TM1637Display.h>

// Define the pins for the relays
int relays[] = {
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  32,
  33
};

#define PIN_BTN_L0 49 //Button for button change trigger
#define PIN_BTN_L1 50 //Button for button change trigger
#define PIN_BTN_L2 51 //Button for button change trigger
#define PIN_BTN_L3 52 //Button for button change trigger

#define PIN_BTN_LA 53; //Button for button change trigger

const int PIN_P0_R = 22;
const int PIN_P0_A = 23;
const int PIN_P0_G = 24;

const int PIN_P1_R = 25;
const int PIN_P1_A = 26;
const int PIN_P1_G = 27;

const int PIN_P2_R = 28;
const int PIN_P2_A = 29;
const int PIN_P2_G = 30;

const int PIN_P3_R = 31;
const int PIN_P3_A = 32;
const int PIN_P3_G = 33;

const unsigned int MAX_MESSAGE_LENGTH = 12;

#define LoopStateAuto 0
#define LoopStateManual 1

#define LoopPhaseRed 0
#define LoopPhaseAmber1 1
#define LoopPhaseAmber2 2
#define LoopPhaseGreen 3

//Current loop state, auto or manual
static int L_0 = 1; //Default into auto (0)
static int L_1 = 0;
static int L_2 = 0;
static int L_3 = 0;

//Current Loop Phase
static int P_0 = 1; //1=red
static int P_1 = 1;
static int P_2 = 1;
static int P_3 = 1;

//time until display dim.
static int OPT_DISP_DIM = 0;

//time until next phase change
static uint64_t OPT_L0_PCHANGE = 0; //Lane 0 phase ms time for change
static uint64_t OPT_L1_PCHANGE = 0; //Lane 1 phase ms time for change
static uint64_t OPT_L2_PCHANGE = 0; //Lane 2 phase ms time for change
static uint64_t OPT_L3_PCHANGE = 0; //Lane 3 phase ms time for change

unsigned int OPT_L0_PTimeRed = 5000;
unsigned int OPT_L0_PTimeAmber1 = 2500;
unsigned int OPT_L0_PTimeAmber2 = 7500;
unsigned int OPT_L0_PTimeGreen = 5000;

unsigned int OPT_L1_PTimeRed = 5000;
unsigned int OPT_L1_PTimeAmber1 = 500;
unsigned int OPT_L1_PTimeAmber2 = 7500;
unsigned int OPT_L1_PTimeGreen = 5000;

unsigned int OPT_L2_PTimeRed = 5000;
unsigned int OPT_L2_PTimeAmber1 = 4525;
unsigned int OPT_L2_PTimeAmber2 = 6000;
unsigned int OPT_L2_PTimeGreen = 5000;

unsigned int OPT_L3_PTimeRed = 2500;
unsigned int OPT_L3_PTimeAmber1 = 2500;
unsigned int OPT_L3_PTimeAmber2 = 2500;
unsigned int OPT_L3_PTimeGreen = 2500;

//display
#define CLK 3
#define DIO 4
// Create a display object of type TM1637Display
TM1637Display display = TM1637Display(CLK, DIO);
const uint8_t allON[] = {
  0xff,
  0xff,
  0xff,
  0xff
};
const uint8_t allOFF[] = {
  0x00,
  0x00,
  0x00,
  0x00
};
const uint8_t dsp_done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G, // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
  SEG_C | SEG_E | SEG_G, // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G // E
};
const uint8_t dsp_hold[] = {
  SEG_F | SEG_E | SEG_B | SEG_C | SEG_G, // H
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
  SEG_F | SEG_E | SEG_D, // L
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G // d
};
const uint8_t dsp_auto[] = {
  SEG_A | SEG_F | SEG_B | SEG_G | SEG_E | SEG_C, // A
  SEG_C | SEG_D | SEG_E, // u
  SEG_G | SEG_E | SEG_D | SEG_F, // t
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F // o
};
const uint8_t dsp_comp[] = {
  SEG_A | SEG_F | SEG_B | SEG_G | SEG_E | SEG_C, // C
  SEG_C | SEG_D | SEG_E, // O
  SEG_G | SEG_E | SEG_D | SEG_F, // t
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F // o
};
static uint8_t dsp_current_phase[4] = {};
const uint8_t dsp_red = {
  SEG_A | SEG_E | SEG_F
};
const uint8_t dsp_amb = {
  SEG_A | SEG_F | SEG_B | SEG_G | SEG_E | SEG_C
};
const uint8_t dsp_gre = {
  SEG_G | SEG_F | SEG_A | SEG_B | SEG_C | SEG_D
};
//r e,f,a
//A a,f,b,g,e,c
//G g,f,a,b,c,d
static char message[MAX_MESSAGE_LENGTH];
static unsigned int message_pos = 0;
// Define the serial port

static bool L0_BTN_STATE = 0;
void setup() {
  display.setBrightness(10);
  display.setSegments(dsp_hold); //dsp_hold

  //delay(3000);
  //dsp_current_phase[2]=dsp_gre;
  //display.setSegments(dsp_current_phase);

  //delay(30000);
  for (int i = 0; i < 12; i++) {
    pinMode(relays[i], OUTPUT);
    digitalWrite(relays[i], HIGH);
  }

  //configure buttons
  pinMode(PIN_BTN_L0, INPUT);

  // Initialize the serial port
  Serial.begin(9600); // Any baud rate should work
  Serial.println("STARTUP");
  delay(6000);//Delay startup so traffic lights don't turn on straight away.
  digitalWrite(PIN_P0_R, LOW);
  delay(400);
  digitalWrite(PIN_P1_R, LOW);
  delay(400);
  digitalWrite(PIN_P2_R, LOW);
  delay(400);
  digitalWrite(PIN_P3_R, LOW);
  display.setSegments(dsp_done);
  delay(3000);
  display.setSegments(dsp_auto);
  Serial.println(millis());
  //Set display dim in 60 seconds.
  OPT_DISP_DIM = millis() + 60000;
  dsp_current_phase[0] = dsp_red;
  dsp_current_phase[1] = dsp_red;
  dsp_current_phase[2] = dsp_red;
  dsp_current_phase[3] = dsp_red;
  display.setSegments(dsp_current_phase);
  OPT_L0_PCHANGE = millis() + 5000;
  OPT_L1_PCHANGE = millis() + 5000;
  OPT_L2_PCHANGE = millis() + 5000;
  OPT_L3_PCHANGE = millis() + 5000;
}

void loop() {
  bool disp_change = false;
  if (L_0 == LoopStateManual) {
    if (digitalRead(PIN_BTN_L0) == LOW) {
      //Button is pressed riht now
      if (!L0_BTN_STATE) {
        L0_BTN_STATE = true;
        //Button trigger press now...
        Loop0PhaseChange(disp_change);
      }
    } else {
      L0_BTN_STATE = false;
    }
  }

  // Check if the serial port is available
  if (Serial.available() > 0) {

    byte data = Serial.read();
    if (data != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1)) {
      //Add the incoming byte to our message
      message[message_pos] = data;
      message_pos++;
    } else {
      //Add null character to string
      message[message_pos] = '\0';
      //Print the message (or do other things)
      Serial.println(message);
      //Reset for the next message
      message_pos = 0;
      String input;
      input = message;

      // If the input is "MANUAL", switch to manual control
      // Switch on the relay according to the argument
     
      if (input.startsWith("L0_")) {
        input.replace("L0_", "");
        if (input == "AUTO_OFF") {
          L_0 = LoopStateManual;
        } else if (input == "AUTO_ON") {
          L_0 = LoopStateAuto;
        } else if (input.indexOf("TRIG") != -1 && L_0 == LoopStateManual) {
          if (input!="TRIG"){
            //Could be phase changes.
            //We need to set the phase ton
            if (input=="TRIGR"){
              P_0=LoopPhaseAmber2;
               Loop0PhaseChange(disp_change);
            }
            if (input=="TRIGA"){
              P_0=LoopPhaseGreen;
               Loop0PhaseChange(disp_change);
            }
            if (input=="TRIGG"){
              P_0=LoopPhaseAmber1;
               Loop0PhaseChange(disp_change);
            }
          }else{
             Loop0PhaseChange(disp_change);
          }
        } else if (input.startsWith("P")) { //setting phase times.
          String phase = input.substring(1, 2);
          input.replace("P" + phase, "");
          /*L1_PA7500//Amber1 phase 7500ms
          L1_PR5000//Red phase 5000ms
          L1_PG5000//green phase 5000ms
          L1_PF2500//Amber2 phase time 2500ms*/
          if (phase == "A") {
            OPT_L0_PTimeAmber1 = input.toInt();
          } else if (phase == "R") {
            OPT_L0_PTimeRed = input.toInt();
          } else if (phase == "G") {
            OPT_L0_PTimeGreen = input.toInt();
          } else if (phase == "F") {
            OPT_L0_PTimeAmber2 = input.toInt();
          }
        } else if (input.startsWith("G")) { //Getting phase times.
          String phase = input.substring(1, 2);
          input.replace("G" + phase, "");
          if (phase == "A") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L0_PTimeAmber1);
          } else if (phase == "R") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L0_PTimeRed);
          } else if (phase == "G") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L0_PTimeGreen);
          } else if (phase == "F") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L0_PTimeAmber2);
          } else if (phase == "S") {//State 
            Serial.print(message);
            Serial.print("=");
            if (L_0 == LoopStateManual){
              Serial.println("Manual");
            }else{
              Serial.println("Auto");
            }
          }
        }
      }
      if (input.startsWith("L1_")) {
        input.replace("L1_", "");
        if (input == "AUTO_OFF") {
          L_1 = LoopStateManual;
        } else if (input == "AUTO_ON") {
          L_1 = LoopStateAuto;
        } else if (input.indexOf("TRIG") != -1 && L_1 == LoopStateManual) {
          if (input!="TRIG"){
            //Could be phase changes.
            //We need to set the phase ton
            if (input=="TRIGR"){
              P_1=LoopPhaseAmber2;
              Loop1PhaseChange(disp_change);
            }
            if (input=="TRIGA"){
              P_1=LoopPhaseGreen;
              Loop1PhaseChange(disp_change);
            }
            if (input=="TRIGG"){
              P_1=LoopPhaseAmber1;
              Loop1PhaseChange(disp_change);
            }
          }else{
            Loop1PhaseChange(disp_change);
          }
        } else if (input.startsWith("P")) { //setting phase times.
          String phase = input.substring(1, 2);
          input.replace("P" + phase, "");
          /*L1_PA7500//Amber1 phase 7500ms
          L1_PR5000//Red phase 5000ms
          L1_PG5000//green phase 5000ms
          L1_PF2500//Amber2 phase time 2500ms*/
          if (phase == "A") {
            OPT_L1_PTimeAmber1 = input.toInt();
          } else if (phase == "R") {
            OPT_L1_PTimeRed = input.toInt();
          } else if (phase == "G") {
            OPT_L1_PTimeGreen = input.toInt();
          } else if (phase == "F") {
            OPT_L1_PTimeAmber2 = input.toInt();
          }
        }else if (input.startsWith("G")) { //Getting phase times.
          String phase = input.substring(1, 2);
          input.replace("G" + phase, "");
          if (phase == "A") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L1_PTimeAmber1);
          } else if (phase == "R") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L1_PTimeRed);
          } else if (phase == "G") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L1_PTimeGreen);
          } else if (phase == "F") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L1_PTimeAmber2);
          }else if (phase == "S") {//State 
            Serial.print(message);
            Serial.print("=");
            if (L_1 == LoopStateManual){
              Serial.println("Manual");
            }else{
              Serial.println("Auto");
            }
          }
        }

      }
      if (input.startsWith("L2_")) {
        input.replace("L2_", "");
        if (input == "AUTO_OFF") {
          L_2 = LoopStateManual;
        } else if (input == "AUTO_ON") {
          L_2 = LoopStateAuto;
        } else if (input.indexOf("TRIG") != -1 && L_2 == LoopStateManual) {
          if (input!="TRIG"){
            //Could be phase changes.
            //We need to set the phase ton
            if (input=="TRIGR"){
              P_2=LoopPhaseAmber2;
              Loop2PhaseChange(disp_change);
            }
            if (input=="TRIGA"){
              P_2=LoopPhaseGreen;
              Loop2PhaseChange(disp_change);
            }
            if (input=="TRIGG"){
              P_2=LoopPhaseAmber1;
              Loop2PhaseChange(disp_change);
            }
          }else{
            Loop2PhaseChange(disp_change);
          }
        } else if (input.startsWith("P")) { //setting phase times.
          String phase = input.substring(1, 2);
          input.replace("P" + phase, "");
          /*L2_PA7500//Amber1 phase 7500ms
          L2_PR5000//Red phase 5000ms
          L2_PG5000//green phase 5000ms
          L2_PF2500//Amber2 phase time 2500ms*/
          if (phase == "A") {
            OPT_L2_PTimeAmber1 = input.toInt();
          } else if (phase == "R") {
            OPT_L2_PTimeRed = input.toInt();
          } else if (phase == "G") {
            OPT_L2_PTimeGreen = input.toInt();
          } else if (phase == "F") {
            OPT_L2_PTimeAmber2 = input.toInt();
          }
        }else if (input.startsWith("G")) { //Getting phase times.
          String phase = input.substring(1, 2);
          input.replace("G" + phase, "");
          if (phase == "A") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L2_PTimeAmber1);
          } else if (phase == "R") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L2_PTimeRed);
          } else if (phase == "G") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L2_PTimeGreen);
          } else if (phase == "F") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L2_PTimeAmber2);
          }else if (phase == "S") {//State 
            Serial.print(message);
            Serial.print("=");
            if (L_2 == LoopStateManual){
              Serial.println("Manual");
            }else{
              Serial.println("Auto");
            }
          }
        }
      }
      if (input.startsWith("L3_")) {
        input.replace("L3_", "");
        if (input == "AUTO_OFF") {
          L_3 = LoopStateManual;
        } else if (input == "AUTO_ON") {
          L_3 = LoopStateAuto;
        } else if (input.indexOf("TRIG") != -1 && L_3 == LoopStateManual) {
          if (input!="TRIG"){
            //Could be phase changes.
            //We need to set the phase ton
            if (input=="TRIGR"){
              P_3=LoopPhaseAmber2;
              Loop3PhaseChange(disp_change);
            }
            if (input=="TRIGA"){
              P_3=LoopPhaseGreen;
              Loop3PhaseChange(disp_change);
            }
            if (input=="TRIGG"){
              P_3=LoopPhaseAmber1;
              Loop3PhaseChange(disp_change);
            }
          }else{
            Loop3PhaseChange(disp_change);
          }
        } else if (input.startsWith("P")) { //setting phase times.
          String phase = input.substring(1, 2);
          input.replace("P" + phase, "");
          /*L3_PA7500//Amber1 phase 7500ms
          L3_PR5000//Red phase 5000ms
          L3_PG5000//green phase 5000ms
          L3_PF2500//Amber2 phase time 2500ms*/
          if (phase == "A") {
            OPT_L3_PTimeAmber1 = input.toInt();
          } else if (phase == "R") {
            OPT_L3_PTimeRed = input.toInt();
          } else if (phase == "G") {
            OPT_L3_PTimeGreen = input.toInt();
          } else if (phase == "F") {
            OPT_L3_PTimeAmber2 = input.toInt();
          }
        }else if (input.startsWith("G")) { //Getting phase times.
          String phase = input.substring(1, 2);
          input.replace("G" + phase, "");
          if (phase == "A") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L3_PTimeAmber1);
          } else if (phase == "R") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L3_PTimeRed);
          } else if (phase == "G") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L3_PTimeGreen);
          } else if (phase == "F") {
            Serial.print(message);
            Serial.print("=");
            Serial.println(OPT_L3_PTimeAmber2);
          }else if (phase == "S") {//State 
            Serial.print(message);
            Serial.print("=");
            if (L_3 == LoopStateManual){
              Serial.println("Manual");
            }else{
              Serial.println("Auto");
            }
          }
        }
      }
    }
  }
  unsigned long mi = millis();
  if (OPT_DISP_DIM != 0 && OPT_DISP_DIM < mi) {
    display.setBrightness(3);
    OPT_DISP_DIM = 0;
  }
  if (L_0 == LoopStateAuto) {
    if (OPT_L0_PCHANGE == 0) {
      //Phase is initial red. Set to initial red time.
      OPT_L0_PCHANGE = millis() + OPT_L0_PTimeRed;
      Serial.println("L0PR");
    }
    if (OPT_L0_PCHANGE <= mi) {
      //Occurs when phase needs to change
      Loop0PhaseChange(disp_change);
    }
  }
  if (L_1 == LoopStateAuto) {
    if (OPT_L1_PCHANGE == 0) {
      //Phase is initial red. Set to initial red time.
      OPT_L1_PCHANGE = millis() + OPT_L1_PTimeRed;
      Serial.println("L1PR");
    }
    if (OPT_L1_PCHANGE <= mi) {
      //Occurs when phase needs to change
      Loop1PhaseChange(disp_change);
    }
  }
  if (L_2 == LoopStateAuto) {
    if (OPT_L2_PCHANGE == 0) {
      //Phase is initial red. Set to initial red time.
      OPT_L2_PCHANGE = millis() + OPT_L2_PTimeRed;
      Serial.println("L2PR");
    }
    if (OPT_L2_PCHANGE <= mi) {
      //Occurs when phase needs to change
      Loop2PhaseChange(disp_change);
    }
  }
  if (L_3 == LoopStateAuto) {
    if (OPT_L3_PCHANGE == 0) {
      //Phase is initial red. Set to initial red time.
      OPT_L3_PCHANGE = millis() + OPT_L3_PTimeRed;
      Serial.println("L3PR");
    }
    if (OPT_L3_PCHANGE <= mi) {
      Loop3PhaseChange(disp_change);
    }
  }
  if (disp_change) {
    display.setSegments(dsp_current_phase);
  }
}

size_t print64(Print * pr, uint64_t n) {
  char buf[21];
  char * str = & buf[sizeof(buf) - 1];
  * str = '\0';
  do {
    uint64_t m = n;
    n /= 10;
    *--str = m - 10 * n + '0';
  } while (n);
  pr -> print(str);
}
void Loop0PhaseChange(bool & disp_change) {
  if (P_0 == LoopPhaseRed) {
    //Set phase amber1
    P_0 = LoopPhaseAmber1;
    OPT_L0_PCHANGE = millis() + OPT_L0_PTimeAmber1;
    digitalWrite(PIN_P0_R, LOW);
    digitalWrite(PIN_P0_A, LOW);
    digitalWrite(PIN_P0_G, HIGH);
    Serial.println("L0PA1");
    dsp_current_phase[0] = dsp_amb;
    disp_change = true;
  } else if (P_0 == LoopPhaseAmber1) {
    //Set phase green
    P_0 = LoopPhaseGreen;
    OPT_L0_PCHANGE = millis() + OPT_L0_PTimeGreen;
    digitalWrite(PIN_P0_R, HIGH);
    digitalWrite(PIN_P0_A, HIGH);
    digitalWrite(PIN_P0_G, LOW);
    Serial.println("L0PG");
    dsp_current_phase[0] = dsp_gre;
    disp_change = true;
  } else if (P_0 == LoopPhaseGreen) {
    //Set phase amber2
    P_0 = LoopPhaseAmber2;
    OPT_L0_PCHANGE = millis() + OPT_L0_PTimeAmber2;
    digitalWrite(PIN_P0_R, HIGH);
    digitalWrite(PIN_P0_A, LOW);
    digitalWrite(PIN_P0_G, HIGH);
    Serial.println("L0PA2");
    dsp_current_phase[0] = dsp_amb;
    disp_change = true;
  } else if (P_0 == LoopPhaseAmber2) {
    //Set phase Red
    P_0 = LoopPhaseRed;
    OPT_L0_PCHANGE = millis() + OPT_L0_PTimeRed;
    digitalWrite(PIN_P0_R, LOW);
    digitalWrite(PIN_P0_A, HIGH);
    digitalWrite(PIN_P0_G, HIGH);
    Serial.println("L0PR");
    dsp_current_phase[0] = dsp_red;
    disp_change = true;
  } else {
    P_0 = LoopPhaseRed;
    OPT_L0_PCHANGE = millis() + OPT_L0_PTimeRed;
    digitalWrite(PIN_P0_R, LOW);
    digitalWrite(PIN_P0_A, HIGH);
    digitalWrite(PIN_P0_G, HIGH);
    Serial.println("L0PR");
    dsp_current_phase[0] = dsp_red;
    disp_change = true;
  }
}

void Loop1PhaseChange(bool & disp_change) {
  if (P_1 == LoopPhaseRed) {
    //Set phase amber1
    P_1 = LoopPhaseAmber1;
    OPT_L1_PCHANGE = millis() + OPT_L1_PTimeAmber1;
    digitalWrite(PIN_P1_R, LOW);
    digitalWrite(PIN_P1_A, LOW);
    digitalWrite(PIN_P1_G, HIGH);
    Serial.println("L1PA1");
    dsp_current_phase[1] = dsp_amb;
    disp_change = true;
  } else if (P_1 == LoopPhaseAmber1) {
    //Set phase green
    P_1 = LoopPhaseGreen;
    OPT_L1_PCHANGE = millis() + OPT_L1_PTimeGreen;
    digitalWrite(PIN_P1_R, HIGH);
    digitalWrite(PIN_P1_A, HIGH);
    digitalWrite(PIN_P1_G, LOW);
    Serial.println("L1PG");
    dsp_current_phase[1] = dsp_gre;
    disp_change = true;
  } else if (P_1 == LoopPhaseGreen) {
    //Set phase amber2
    P_1 = LoopPhaseAmber2;
    OPT_L1_PCHANGE = millis() + OPT_L1_PTimeAmber2;
    digitalWrite(PIN_P1_R, HIGH);
    digitalWrite(PIN_P1_A, LOW);
    digitalWrite(PIN_P1_G, HIGH);
    Serial.println("L1PA2");
    dsp_current_phase[1] = dsp_amb;
    disp_change = true;
  } else if (P_1 == LoopPhaseAmber2) {
    //Set phase Red
    P_1 = LoopPhaseRed;
    OPT_L1_PCHANGE = millis() + OPT_L1_PTimeRed;
    digitalWrite(PIN_P1_R, LOW);
    digitalWrite(PIN_P1_A, HIGH);
    digitalWrite(PIN_P1_G, HIGH);
    Serial.println("L1PR");
    dsp_current_phase[1] = dsp_red;
    disp_change = true;
  } else {
    P_1 = LoopPhaseRed;
    OPT_L1_PCHANGE = millis() + OPT_L1_PTimeRed;
    digitalWrite(PIN_P1_R, LOW);
    digitalWrite(PIN_P1_A, HIGH);
    digitalWrite(PIN_P1_G, HIGH);
    Serial.println("L1PR");
    dsp_current_phase[1] = dsp_red;
    disp_change = true;
  }
}

void Loop2PhaseChange(bool & disp_change) {
  if (P_2 == LoopPhaseRed) {
    //Set phase amber2
    P_2 = LoopPhaseAmber1;
    OPT_L2_PCHANGE = millis() + OPT_L2_PTimeAmber1;
    digitalWrite(PIN_P2_R, LOW);
    digitalWrite(PIN_P2_A, LOW);
    digitalWrite(PIN_P2_G, HIGH);
    Serial.println("L2PA1");
    dsp_current_phase[2] = dsp_amb;
    disp_change = true;
  } else if (P_2 == LoopPhaseAmber1) {
    //Set phase green
    P_2 = LoopPhaseGreen;
    OPT_L2_PCHANGE = millis() + OPT_L2_PTimeGreen;
    digitalWrite(PIN_P2_R, HIGH);
    digitalWrite(PIN_P2_A, HIGH);
    digitalWrite(PIN_P2_G, LOW);
    Serial.println("L2PG");
    dsp_current_phase[2] = dsp_gre;
    disp_change = true;
  } else if (P_2 == LoopPhaseGreen) {
    //Set phase amber2
    P_2 = LoopPhaseAmber2;
    OPT_L2_PCHANGE = millis() + OPT_L2_PTimeAmber2;
    digitalWrite(PIN_P2_R, HIGH);
    digitalWrite(PIN_P2_A, LOW);
    digitalWrite(PIN_P2_G, HIGH);
    Serial.println("L2PA2");
    dsp_current_phase[2] = dsp_amb;
    disp_change = true;
  } else if (P_2 == LoopPhaseAmber2) {
    //Set phase Red
    P_2 = LoopPhaseRed;
    OPT_L2_PCHANGE = millis() + OPT_L2_PTimeRed;
    digitalWrite(PIN_P2_R, LOW);
    digitalWrite(PIN_P2_A, HIGH);
    digitalWrite(PIN_P2_G, HIGH);
    Serial.println("L2PR");
    dsp_current_phase[2] = dsp_red;
    disp_change = true;
  } else {
    P_2 = LoopPhaseRed;
    OPT_L2_PCHANGE = millis() + OPT_L2_PTimeRed;
    digitalWrite(PIN_P2_R, LOW);
    digitalWrite(PIN_P2_A, HIGH);
    digitalWrite(PIN_P2_G, HIGH);
    Serial.println("L2PR");
    dsp_current_phase[2] = dsp_red;
    disp_change = true;
  }
}
void Loop3PhaseChange(bool & disp_change) {
  //Occurs when phase needs to change
  if (P_3 == LoopPhaseRed) {
    //Set phase amber3
    P_3 = LoopPhaseAmber1;
    OPT_L3_PCHANGE = millis() + OPT_L3_PTimeAmber1;
    digitalWrite(PIN_P3_R, LOW);
    digitalWrite(PIN_P3_A, LOW);
    digitalWrite(PIN_P3_G, HIGH);
    Serial.println("L3PA1");
    dsp_current_phase[3] = dsp_amb;
    disp_change = true;
  } else if (P_3 == LoopPhaseAmber1) {
    //Set phase green
    P_3 = LoopPhaseGreen;
    OPT_L3_PCHANGE = millis() + OPT_L3_PTimeGreen;
    digitalWrite(PIN_P3_R, HIGH);
    digitalWrite(PIN_P3_A, HIGH);
    digitalWrite(PIN_P3_G, LOW);
    Serial.println("L3PG");
    dsp_current_phase[3] = dsp_gre;
    disp_change = true;
  } else if (P_3 == LoopPhaseGreen) {
    //Set phase amber3
    P_3 = LoopPhaseAmber2;
    OPT_L3_PCHANGE = millis() + OPT_L3_PTimeAmber2;
    digitalWrite(PIN_P3_R, HIGH);
    digitalWrite(PIN_P3_A, LOW);
    digitalWrite(PIN_P3_G, HIGH);
    Serial.println("L3PA2");
    dsp_current_phase[3] = dsp_amb;
    disp_change = true;
  } else if (P_3 == LoopPhaseAmber2) {
    //Set phase Red
    P_3 = LoopPhaseRed;
    OPT_L3_PCHANGE = millis() + OPT_L3_PTimeRed;
    digitalWrite(PIN_P3_R, LOW);
    digitalWrite(PIN_P3_A, HIGH);
    digitalWrite(PIN_P3_G, HIGH);
    Serial.println("L3PR");
    dsp_current_phase[3] = dsp_red;
    disp_change = true;
  } else {
    P_3 = LoopPhaseRed;
    OPT_L3_PCHANGE = millis() + OPT_L3_PTimeRed;
    digitalWrite(PIN_P3_R, LOW);
    digitalWrite(PIN_P3_A, HIGH);
    digitalWrite(PIN_P3_G, HIGH);
    Serial.println("L3PR");
    dsp_current_phase[3] = dsp_red;
    disp_change = true;
  }
}
