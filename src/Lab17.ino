#include "MQTT.h"

SYSTEM_THREAD(ENABLED);

void callback(char *topic, byte *payload, unsigned int length);

MQTT client("lab.thewcl.com", 1883, callback);

int currentState = 0;
bool northLEDOn = true; // We can use a single variable to track the state of both lanes.

struct State
{
  String output;
  unsigned long waitTime;
  int nextState[2];
};
typedef struct State State_t;
State_t fsm[4] = {
    // North LED pins, East LED pins, Wait time, Next state
    // I had to convert the 2D array to a string to publish it to MQTT
    {"[[1, 0, 0], [0, 0, 1]]", 100, {1, 0}},  // NORTH ON, EAST OFF
    {"[[0, 1, 0], [0, 0, 1]]", 5000, {2, 2}}, // NORTH YELLOW, EAST OFF
    {"[[0, 0, 1], [1, 0, 0]]", 100, {2, 3}},  // NORTH OFF, EAST ON
    {"[[0, 0, 1], [0, 1, 0]]", 5000, {0, 0}}  // NORTH OFF, EAST YELLOW
};
void setup()
{
}

void loop()
{
  if (client.isConnected())
  {
    client.loop();
    // FINITE STATE MACHINE!!!
    client.publish("VinsonLab17/GetData", fsm[currentState].output);
    delay(fsm[currentState].waitTime);
    currentState = fsm[currentState].nextState[northLEDOn];
  }
  else
  {
    client.connect(System.deviceID());
    client.subscribe("VinsonLab17/PostData");
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  // Toggles between North and East LED's
  if (String(p).equals("North"))
  {
    northLEDOn = true;
  }
  else if (String(p).equals("East"))
  {
    northLEDOn = false;
  }
}