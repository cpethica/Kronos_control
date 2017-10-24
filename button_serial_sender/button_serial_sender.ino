//Planet_Kronos_x8 The Kazimier August 2016
//The MIT License (MIT)
#include <SLIPEncodedSerial.h>
#include <OSCBundle.h>
#include <TimerOne.h>


SLIPEncodedSerial SLIPSerial(Serial);

#define NUM_MOTORS 8

// BAUD RATE
const long BAUD_RATE = 115200;

//switch Pins
int switchForward[NUM_MOTORS] = {39, 37, 35, 33, 31, 29, 27, 25};
int switchBack[NUM_MOTORS] = {38, 36, 34, 32, 30, 28, 26, 24};

int switchForwardAll = 23;
int switchBackAll = 22;

typedef enum motorStates { // enum with motor States, just a list of Names constants
  FORWARD,
  BACKWARD,
  STOP
};

motorStates actualState[NUM_MOTORS];

void setup() {

  // radio setup
  delay(10);
  Serial.begin(BAUD_RATE);
  delay(10);

  SLIPSerial.begin(115200);

  //two pins to control the motor
  for (int i = 0; i < NUM_MOTORS; i++) {
    //two switchs INPUT,
    pinMode(switchForward[i], INPUT);
    pinMode(switchBack[i], INPUT);

    actualState[i] = STOP;  // start with STOP state
  }

 // Timer1.initialize(1500000);
 // Timer1.attachInterrupt(checkSwitches);
}

void loop()
{
  //process received messages
  checkSwitches();
  delay(500);
  //constantly check the keys independent of the OSC messages
  //now check all motors and turn the motor off in case off the switchs are on

}

// OSC sender

void sendOSC(String msg, int motorID) {

  // message contains /F, /B, /S string to correctly route OSC to brain
  String msgText = msg;
  unsigned int data = motorID;
  OSCMessage msgOUT(msgText.c_str());
  // add integer data - motor number
  msgOUT.add(data);
  SLIPSerial.beginPacket();
  msgOUT.send(SLIPSerial);
  SLIPSerial.endPacket();
}

boolean sendMotorCommand(enum motorStates state, int motorID) {

  //only send motor messages if it's a new OSC message a new state
  if (actualState[motorID] != state) {
    actualState[motorID] = state; // update actual state

    switch (state) {
      case FORWARD:
        sendOSC("/F", motorID);
        break;
      case BACKWARD:
        sendOSC("/B", motorID);
        break;
      case STOP:
        sendOSC("/S", motorID);
        break;
    }
  }
  //read switches and return error in case
  //if it gets here I hope everything is ok
  return 1; //return 1 for OK
}

void checkSwitches() {

  for (int motorID = 0; motorID < NUM_MOTORS; motorID++) {

    boolean switchF = digitalRead(switchForward[motorID]); // if switch is ON for both it stop and send back erro
    boolean switchB = digitalRead(switchBack[motorID]);

    // send switch states
    if (switchF == 0) {
      sendMotorCommand(FORWARD, motorID); // motorID will be the index
    }
    else if (switchB == 0) {
      sendMotorCommand(BACKWARD, motorID); // motorID will be the index
    }
    else {
      sendMotorCommand(STOP, motorID); // motorID will be the index
    }
  }
}
