#include <PWMaa.h>  //download https://code.google.com/archive/p/arduino-pwm-frequency-library/downloads
// edit ATimerDefs.h and add '(0,0,0,0,0), // TIMER1C'  after Line 107
// if applied to Arduino Mega 2560
#include <PZEM004Tv30.h>  // Power meter library
#include <math.h>
/*
  This code runs on mega and uno.
*/
PZEM004Tv30 pzemOT1(68, 69);  // power meter

float power1;
float voltage1;
float current1;
float factor1;

float powerx;
int relayPin = 8;

// initializ the SSRs
//use pin 11 on the Mega instead, otherwise there is a frequency cap at 31 Hz
//  ledTimer0  Pins 13, 4  might influence the internal clock
int ledTimer1 = 5;      // Pins 11, 12 16-bits timer
int32_t frequency = 1;  //PWM frequency (in Hz)

int mark = 0;
// String comdata = "";       // initialize input string
int numdata[2] = { 0 };
float DutyCircle;
const byte numChars = 32;
char comdata[numChars];

boolean newData = false;

void setup() {
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe();
  digitalWrite(relayPin, LOW);
  //sets the frequency for the specified pin
  bool success1 = SetPinFrequencySafe(ledTimer1, frequency);
  Serial.begin(115200);  // set up communication
  Serial.println("Power box");
}

void loop() {
  int j = 0;
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB
  }
  recvWithStartEndMarkers();
  if (newData == true) {
    if (comdata[0] == 'F') {
      pwmWrite(ledTimer1, 0);  // change the duty circle
      // digitalWrite(relayPin,LOW);
      delay(10);
      mark = 0;
      // comdata = String("");
    }
    if (comdata[0] == 'P' && comdata[1] == 'W') {
      Serial.println("power reading");
      powerx = pzemOT1.power();
      //    voltage1 = pzemOT1.voltage();
      //    current1 = pzemOT1.current();
      //    power1 = voltage1 * current1;
      power1 = pzemOT1.power();
      //    factor1 = pzemOT1.powerfactor();
      if (!isnan(power1)) {
        //        Serial.print("1: ");
        Serial.print(power1);
        Serial.println();
      } else {

        Serial.println("1: OFF");
        // Serial.print(voltage1); Serial.println();
        // Serial.print(current1); Serial.println();
      }
      mark = 0;
    }

    if (mark == 1) {
      int condit = strlen(comdata);  //.length() - 1;
      // convert input characters
      for (int i = 0; i < condit; i++) {
        if (comdata[i] == ',') {
          j++;
        } else {
          numdata[j] = numdata[j] * 10 + (int(comdata[i]) - '0');
        }
      }
      pwmWrite(numdata[0], numdata[1]);  // change the duty circle
      DutyCircle = float(numdata[1]) / 255 * 100;
      //Serial.println(DutyCircle);
      //Serial.println(numdata[1]);
      for (int i = 0; i < 2; i++) {
        // Serial.println(numdata[i], DEC);
        numdata[i] = 0;
      }
      // Serial.println(DutyCircle,DEC);
      //     comdata = String("");
      mark = 0;
    }

    if (comdata[0] == 'S' && comdata[1] == 'O') {
      Serial.println("Relay On");
      digitalWrite(relayPin, HIGH);
      mark = 0;
    }

    if (comdata[0] == 'S' && comdata[1] == 'F') {
      Serial.println("Relay Off");
      digitalWrite(relayPin, LOW);
      mark = 0;
    }
    newData = false;
  }
}



void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  // if (Serial.available() > 0) {
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    delay(2);

    if (recvInProgress == true) {
      if (rc != endMarker) {
        comdata[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        comdata[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
  mark = 1;
}

//
