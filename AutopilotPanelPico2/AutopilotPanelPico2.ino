/*To Do:
Make Encoder make different steps depending on which mode is active (HDG, Alt...) (Done)
Make Debounce of encoder Switch better, similar to debounce of normal switches (Done)
Max and Min values for the 7 segment display depending on mode (Done)
instead of the delay after the debounce, maybe try to solve with millis (only if debounce problem with encoder switch)
add that if over 360 degrees in heading mode it starts back at 0 (Needs Testing line 301)
add that it saves the altitude or speed or such you set, so when you get back to that mode it still has that displayed (Done)
when vs switches from positive to negative it stops at 0 and then continues (Needs Testing line 297)
when encoder button gets pressed it should reset to small steps (Needs Testing line 274)
*/
 
#define NUMButtons 14 //Number of buttons for array
#define NUMDigits 6
 
#define EncoderPinA 2
#define EncoderPinB 1
#define EncoderButton 0
 
#define NumModes 4
 
int CLKPin = 28;
int DataPin7S = 27;
int DataPinLED = 17;
 
int ButtonPins[NUMButtons] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}; //Pins, which have Buttons
 
int LatchPins[NUMDigits] = {26, 22, 21, 20, 19, 18}; //Latch Pins starting with ones, then tens, then hundreds...
 
int CurrentPinA;
int LastPinA;
int EncoderPos = 0;
 
bool EncoderPress = false;
bool EncoderCurrentState;
bool EncoderLastState;
 
int Data[NumModes] = {0, 1, 2, 3};
 
int Speed;
int Heading;
int Altitude;
int VerticalSpeed;
 
int DisplayNumber = 0;
int DisplaySpecial = 2;
int DisplayMin = 0;
int DisplayMax = 99900;
int EncoderSteps = 2;
int LastEncoderSteps = 0;
int EncoderStepsSmall = 100;
int EncoderStepsBig = 1000;
 
int LastVS = 0;
int CurrentVS;
 
unsigned long CurrentTime;
unsigned long PreviousTime;
int Interval = 1;
 
bool ButtonCurrentState[NUMButtons];
bool ButtonLastState[NUMButtons];
 
bool LEDState[NUMButtons];
 
byte LEDState1 = 0b00000000; //Maybe put 0b00000100 here to activate Altitude LED from the start
byte LEDState2 = 0b00000000;
 
bool LatchState = HIGH;
 
byte digits[] = {
  0b00111111, //0
  0b00000110, //1
  0b01011011, //2
  0b01001111, //3
  0b01100110, //4
  0b01101101, //5
  0b11111101, //6
  0b00000111, //7
  0b01111111, //8
  0b11101111, //9
  0b01101101, //S
  0b01110011, //P
  0b01011110, //d
  0b01110110, //H
  0b01111101, //G
  0b01000000, //-
  0b00000000  //Nothing
};
 
byte LEDs[] = {
  0b00000001, //SW1 Speed
  0b00000010, //SW2 Heading
  0b00000100, //SW3 Altitude
  0b00001000, //SW4 Vertical Speed
  0b00010000, //SW5
  0b00100000, //SW6
  0b01000000, //SW7
  0b10000000, //SW8
  0b00000001, //SW9
  0b00000010, //SW10
  0b00000100, //SW11
  0b00001000, //SW12
  0b00010000, //SW13
  0b00100000, //SW14
  0b00000000, //filler
  0b11110000  //To clean the first 4 digits
};
 
void ButtonLED() {
  CurrentTime = millis();
  if (CurrentTime - PreviousTime < Interval) {
    return;
  }
  else {
    PreviousTime = CurrentTime;
   
    for (int i = 0; i < NUMButtons; i++) {
      ButtonCurrentState[i] = digitalRead(ButtonPins[i]);
 
      if (ButtonCurrentState[i] == LOW && ButtonLastState[i] == HIGH) {
 
        if (i == 0 || i == 1 || i == 2 || i == 3) {
          LEDState1 = LEDState1 & LEDs[15];
          LEDState1 = LEDState1 | LEDs[i]; //Makes sure only 1 of the first 4 leds is on
 
          for (int n = 7; n >= 0; n--) {
            int bitValue = (LEDState2 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
            digitalWrite(DataPinLED, bitValue);
 
            digitalWrite(CLKPin, HIGH);
            digitalWrite(CLKPin, LOW);
          }
 
          for (int n = 7; n >= 0; n--) {
            int bitValue = (LEDState1 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
            digitalWrite(DataPinLED, bitValue);
 
            digitalWrite(CLKPin, HIGH);
            digitalWrite(CLKPin, LOW);
          }
 
          EncoderSteps = i;
        }
     
        LEDState[i] = !LEDState[i];
 
        if (LEDState[i] == false) {
          if (i < 8 && i > 3) {
            LEDState1 = LEDState1 | LEDs[i];
            for (int n = 7; n >= 0; n--) {
              int bitValue = (LEDState2 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
              digitalWrite(DataPinLED, bitValue);
 
              digitalWrite(CLKPin, HIGH);
              digitalWrite(CLKPin, LOW);
          }
 
          for (int n = 7; n >= 0; n--) {
            int bitValue = (LEDState1 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
            digitalWrite(DataPinLED, bitValue);
 
            digitalWrite(CLKPin, HIGH);
            digitalWrite(CLKPin, LOW);
          }
          }
          else if (i > 7) {
            LEDState2 = LEDState2 | LEDs[i];
            for (int n = 7; n >= 0; n--) {
              int bitValue = (LEDState2 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
              digitalWrite(DataPinLED, bitValue);
 
              digitalWrite(CLKPin, HIGH);
              digitalWrite(CLKPin, LOW);
          }
 
            for (int n = 7; n >= 0; n--) {
              int bitValue = (LEDState1 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
              digitalWrite(DataPinLED, bitValue);
 
              digitalWrite(CLKPin, HIGH);
              digitalWrite(CLKPin, LOW);
            }
          }
        }
        else if (LEDState[i] == true) {
          if (i < 8 && i > 3) {
            LEDState1 = LEDState1 & ~LEDs[i];
            for (int n = 7; n >= 0; n--) {
              int bitValue = (LEDState2 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
              digitalWrite(DataPinLED, bitValue);
 
              digitalWrite(CLKPin, HIGH);
              digitalWrite(CLKPin, LOW);
          }
 
            for (int n = 7; n >= 0; n--) {
              int bitValue = (LEDState1 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
              digitalWrite(DataPinLED, bitValue);
 
              digitalWrite(CLKPin, HIGH);
              digitalWrite(CLKPin, LOW);
            }
          }
          else if (i > 7) {
            LEDState2 = LEDState2 & ~LEDs[i];
            for (int n = 7; n >= 0; n--) {
              int bitValue = (LEDState2 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
              digitalWrite(DataPinLED, bitValue);
 
              digitalWrite(CLKPin, HIGH);
              digitalWrite(CLKPin, LOW);
          }
 
            for (int n = 7; n >= 0; n--) {
              int bitValue = (LEDState1 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
              digitalWrite(DataPinLED, bitValue);
 
              digitalWrite(CLKPin, HIGH);
              digitalWrite(CLKPin, LOW);
            }
          }
        }
      }
      ButtonLastState[i] = ButtonCurrentState[i];
    }
  }
}
 
void Encoder() {
  CurrentPinA = digitalRead(EncoderPinA);
 
  if (EncoderSteps == 0) { //Speed
    EncoderStepsSmall = 1;
    EncoderStepsBig = 10;
 
    DisplayMin = 0;
    DisplayMax = 999;
  }
  else if (EncoderSteps == 1) { //Heading
    EncoderStepsSmall = 1;
    EncoderStepsBig = 10;
 
    DisplayMin = 1;
    DisplayMax = 360; //add that if over 360 degrees in heading mode it starts back at 0
  }
  else if (EncoderSteps == 2) { //Altitude
    EncoderStepsSmall = 100;
    EncoderStepsBig = 1000;
 
    DisplayMin = 0;
    DisplayMax = 99900;
 
//    digitalWrite(LatchPins[5], LOW);
  }
  else if (EncoderSteps == 3) { //Vertical Speed
    EncoderStepsSmall = 100;
    EncoderStepsBig = 1000;
 
    DisplayMin = -99900;
    DisplayMax = 99900;
  }
 
  if (EncoderSteps != LastEncoderSteps) {
    EncoderPress = false; //Resets to small encoder steps, when mode is changed
  }
 
  if (CurrentPinA != LastPinA) {
    if (CurrentPinA == digitalRead(EncoderPinB)) {
      if (EncoderPress == false) {
        Data[EncoderSteps] = Data[EncoderSteps] - EncoderStepsSmall;
      }
      if (EncoderPress == true) {
        Data[EncoderSteps] = Data[EncoderSteps] - EncoderStepsBig;
      }
    }
    else {
      if (EncoderPress == false) {
        Data[EncoderSteps] = Data[EncoderSteps] + EncoderStepsSmall;
      }
      if (EncoderPress == true) {
        Data[EncoderSteps] = Data[EncoderSteps] + EncoderStepsBig;
      }
    }
 
    CurrentVS = Data[3];
 
    if (CurrentVS < 0 && LastVS > 0 || CurrentVS > 0 && LastVS < 0) { //VS snaps to 0 when switching from - to + or vice versa
      Data[3] = 0;
    }
 
    if (Data[1] == 361) { //Makes Heading a loop
      Data[1] = 0;
    }
    else if (Data[1] == -1) {
      Data[1] = 360;
    }
    else {
      Data[EncoderSteps] = constrain(Data[EncoderSteps], DisplayMin, DisplayMax);
    }
 
    Serial.print("Speed: ");
    Serial.println(Data[0]);
 
    Serial.print("Heading: ");
    Serial.println(Data[1]);
 
    Serial.print("Altitude: ");
    Serial.println(Data[2]);
 
    Serial.print("VS: ");
    Serial.println(Data[3]);
  }
 
  LastPinA = CurrentPinA;
 
  EncoderCurrentState = digitalRead(EncoderButton);
 
  if (EncoderCurrentState == LOW && EncoderLastState == HIGH) {
    EncoderPress = !EncoderPress;
    Serial.println(EncoderPress);
  }
  EncoderLastState = EncoderCurrentState;
 
  LastVS = CurrentVS;
 
  // delay(1); //Maybe add if EncoderButton needs extra debounce (like the normal buttons)
}
 
void setup() {
  Serial.begin(115200);
 
  for (int i = 0; i < NUMButtons; i++) {
    pinMode(ButtonPins[i], INPUT_PULLUP);
    ButtonLastState[i] = HIGH;
    LEDState[i] = false;
  }
 
  for (int i = 0; i < NUMButtons; i++) {
    LEDState[i] = true;
  }
 
  for (int i = 0; i < NUMDigits; i++) {
    pinMode(LatchPins[i], OUTPUT);
    digitalWrite(LatchPins[i], LOW);
  }
 
  for (int i = 0; i < NumModes; i++) {
    if (i == 1) {
      Data[i] = 1;
    }
    else {
      Data[i] = 0;
    }
  }
 
  pinMode(DataPinLED, OUTPUT);
  pinMode(DataPin7S, OUTPUT);
  pinMode(CLKPin, OUTPUT);
 
  pinMode(EncoderPinA, INPUT_PULLUP);
  pinMode(EncoderPinB, INPUT_PULLUP);
  pinMode(EncoderButton, INPUT_PULLUP);
 
  PreviousTime = millis();
 
  EncoderLastState = HIGH;
 
  LastPinA = digitalRead(EncoderPinA);
 
  digitalWrite(CLKPin, LOW);
 
  for (int n = 7; n >= 0; n--) {
    int bitValue = (LEDState2 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
    digitalWrite(DataPinLED, bitValue);
 
    digitalWrite(CLKPin, HIGH);
    digitalWrite(CLKPin, LOW);
  }
 
  for (int n = 7; n >= 0; n--) {
    int bitValue = (LEDState1 >> n) & 1; //makes a bitvalue variable, that only stores the las bit due to & 1
 
    digitalWrite(DataPinLED, bitValue);
 
    digitalWrite(CLKPin, HIGH);
    digitalWrite(CLKPin, LOW);
  }
 
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}
 
void loop() {
  ButtonLED();
  Encoder();
}