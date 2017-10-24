//Planet_Kronos_x8 The Kazimier August 2016
//The MIT License (MIT)
#include <SLIPEncodedSerial.h>
#include <OSCBundle.h>
#include <TimerOne.h>


SLIPEncodedSerial SLIPSerial(Serial);

#define NUM_MOTORS 8

// RADIO PIN AND BAUD RATE
const long BAUD_RATE = 115200;
const short RADIO_PIN = 8;

// motor driver Pins
int MOTOR_PINS[NUM_MOTORS] = {13, 12, 11, 10, 5, 6, 7, 9};
int SLEEP_PINS[NUM_MOTORS] = {22, 24, 26, 28, 30, 32, 34, 36};
int DIR_PINS[NUM_MOTORS] = {4, 38, 40, 42, 44, 46, 2, 3};
//int CUR_PINS[NUM_MOTORS] = {A12, A11, A10, A9, A5, A6, A7, A8};

//switch Pins
int switchPinA[NUM_MOTORS] = {39, 43, 47, 52, 35, 31, 27, 23};
int switchPinB[NUM_MOTORS] = {41, 45, 49, 50, 37, 33, 29, 25};

//Speed Value
int Speed = 255;

typedef enum motorStates { // enum with motor States, just a list of Names constants
  FORWARD,
  BACKWARD,
  STOP
};

motorStates actualState[NUM_MOTORS];


void setup() {

  // radio setup
  delay(10);
  pinMode(RADIO_PIN, OUTPUT);
  digitalWrite(RADIO_PIN, HIGH);
  Serial.begin(BAUD_RATE);
  delay(10);

  SLIPSerial.begin(115200);
  // start the Ethernet connection


  //two pins to control the motor
  for (int i = 0; i < NUM_MOTORS; i++) {
    //pinMode(MOTOR_PINS[i], OUTPUT);
    pinMode(SLEEP_PINS[i], OUTPUT);
    analogWrite(MOTOR_PINS[i], 0);
    digitalWrite(SLEEP_PINS[i], HIGH);
    //two switchs INPUT,

    pinMode(switchPinA[i], INPUT);
    pinMode(switchPinB[i], INPUT);

    //  pinMode(CUR_PINS[i], INPUT);

    actualState[i] = STOP;  // start with STOP state

    sendOSC("go", i);
  }

  Timer1.initialize(150000);
  Timer1.attachInterrupt(checkSensors);
}
void loop()
{
  //process received messages
  OSCMsgReceive();

  //constantly check the keys independent of the OSC messages
  //now check all motors and turn the motor off in case off the switchs are on

}

void checkSensors() {

  for (int motorID = 0; motorID < NUM_MOTORS; motorID++) {

    boolean switchF = digitalRead(switchPinA[motorID]); // if switch is ON for both it stop and send back erro
    boolean switchB = digitalRead(switchPinB[motorID]);
    //if either switchs are on throw error

    if (switchF == 1 && actualState[motorID] == FORWARD) {
      sendOSC("STOP FORWARD", motorID);
      sendMotorCommand(STOP, motorID); // motorID will be the index
    }
    if (switchB == 1  && actualState[motorID] == BACKWARD) {
      sendOSC("STOP BACKWARD", motorID);
      sendMotorCommand(STOP, motorID); // motorID will be the index
    }

  }


  //currentsense();
}

void sendOSC(String msg, int motorID) {

  String msgText = "C/" +  String(motorID) + "/";
  OSCMessage msgOUT(msgText.c_str());
  msgOUT.add(msg);
  SLIPSerial.beginPacket();
  msgOUT.send(SLIPSerial);
  SLIPSerial.endPacket();
}

// CURRENT SENSE OUTPUT

//void currentsense() {
//  for (int motorID = 0; motorID < NUM_MOTORS; motorID++) {
//    Current[motorID] = analogRead(CUR_PINS[motorID]);
//// message out
//    String msgText = "C" + String(motorID) + "/" + String(Current[motorID]);
//    OSCMessage msgOUT(msgText.c_str());
//    SLIPSerial.beginPacket();
//    msgOUT.send(SLIPSerial);
//    SLIPSerial.endPacket();
//// message out end
//  }
//  delay(500);
//}

void OSCMsgReceive() {


  OSCMessage bndl;
  int size;
  //receive a bundle

  while (!SLIPSerial.endofPacket())
    if ( (size = SLIPSerial.available()) > 0)
    {
      while (size--)
        bndl.fill(SLIPSerial.read());
    }

  if (!bndl.hasError())
  {

    bndl.route("/F", motorForward);
    bndl.route("/B", motorBackward);
    bndl.route("/S", motorStop);
  }
}

void motorForward(OSCMessage &msg, int addrOffset) {

  //receive the motor ID from the OSC message
  int motorID;
  int error;

  if (msg.isInt(0)) //only if theres a number
  {
    motorID = msg.getInt(0); //get an integer from the OSC message
    //receive FORWARD update generalState and send back TRUE OSC message
    error = sendMotorCommand(FORWARD, motorID); //motor return 1 for OK 0 for error
  } else {

    error = 0; //trow an error
  }

}

void motorBackward(OSCMessage &msg, int addrOffset ) {


  //receive the motor ID from the OSC message
  int motorID;
  int error;

  if (msg.isInt(0)) //only if theres a number
  {

    motorID = msg.getInt(0); //get an integer from the OSC message
    //receive BACKWARD update generalState and send back TRUE OSC message
    error = sendMotorCommand(BACKWARD, motorID); //motor return 1 for OK 0 for error

  } else {
    error = 0; //trow an error
  }

}

void motorStop(OSCMessage &msg, int addrOffset ) {
  //receive STOP update generalState and send back TRUE OSC message

  int motorID;
  int error;

  if (msg.isInt(0)) //only if theres a number
  {

    motorID = msg.getInt(0); //get an integer from the OSC message
    //receive BACKWARD update generalState and send back TRUE OSC message
    error = sendMotorCommand(STOP, motorID); //motor return 1 for OK 0 for error

  } else {
    error = 0; //trow an error
  }
}


boolean sendMotorCommand(enum motorStates state, int motorID) {

  //first thig to check before sending new motor commands is.
  //the switches are pressed/
  //if any of then are pressed, just turnOFF the motor and return

  boolean switchF = digitalRead(switchPinA[motorID]); // if switch is ON for both it stop and send back erro
  boolean switchB = digitalRead(switchPinB[motorID]);

  //if either switchs are on throw erro
  if (switchF == 1 && state == FORWARD) {
    actualState[motorID] = STOP; // should stop

    //send motors to off (pwm and sleep)
    analogWrite(MOTOR_PINS[motorID], 0);

    return 0; //return 0 //error message
  }

  //if either switchs are on throw erro
  if (switchB == 1 && state == BACKWARD) {
    actualState[motorID] = STOP; // should stop

    //send motors to off
    analogWrite(MOTOR_PINS[motorID], 0);

    return 0; //return 0 //error message
  }
  //only send motor messages if it' a new OSC message a new state

  if (actualState[motorID] != state) {
    actualState[motorID] = state; // update actual state

    switch (state) {
      case FORWARD:
        digitalWrite(DIR_PINS[motorID], HIGH);
        analogWrite(MOTOR_PINS[motorID], Speed);

        break;
      case BACKWARD:

        //check this, not sure how to make your motor go Backward
        digitalWrite(DIR_PINS[motorID], LOW);
        analogWrite(MOTOR_PINS[motorID], Speed);
        break;
      case STOP:
        //dont need to check switchs to make it stop..
        //check how to make the motor STOP
        analogWrite(MOTOR_PINS[motorID], 0);
        break;

    }

  }

  //read switchs and return error in case


  //if it gets here I hope everything is ok

  return 1; //return 1 for OK


}
