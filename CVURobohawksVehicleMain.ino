//Cailean Sorce   8/27/20
//Arduino vehicle example for CVU Robohawks
//this runs on nodeMCU esp8266 dev board for wifi access

#include <Servo.h> //used to easily control the wheel servos
#include <EEPROM.h>
#include "CVURobohawksVehicleWifi.h" //contains the html + js and funcitons used to set up and interface with the
//wifi client that will be used to control the vehicle
#include "CaileanSorce_nodemcuESP8266_pinDefinitions.h" //contains the const ints used to convert the GPIO port names silkscreened on the
//dev board to the esp8266 GPIO numbers

char wifiName[] = "CSorceWiFi",      //the name of the wifi network that will be created
    wifiPassword[] = "CaileanSorce"; //the password to join the wifi network. MUST BE >= 8 characters

int joyStickX = 0, joyStickY = 0,    //used to store the joyStick positions. updated with the wifi joystick
    rightServoSpeed, leftServoSpeed, //used to store the speeds for the left and right servos
    rightServoError, leftServoError;

int EEPROMServoErrorPosition = 0;

Servo rightServo, leftServo; //create two servo objects

const int flashButton = 0; //the flash button on the nodemcu is on GPIO 0 (thats esp8266 GPIO 0, not D0)

//this function is called when the user whats to run autonomous
void runAutonomous();

void saveServoError(int left, int right);

void getServoError(int &left, int &right);

//this function delays the program while still allowing the necessary background functions to run. it takes the delay time in ms as an argument
void myDelay(unsigned int runTime);

void setServoSpeed(Servo &servo, int speed, int error, bool reversed);

void setup()
{

  Serial.begin(115200); //begin serial communication at 115200 baud rate

  EEPROM.begin(512);

  getServoError(leftServoError, rightServoError);

  DEBUG_SERIAL(leftServoError);
  DEBUG_SERIAL(rightServoError);
  DEBUG_SERIAL("\n");

  setUpWiFi(wifiName, wifiPassword, 8, joyStickX, joyStickY, leftServoError, rightServoError, saveServoError, runAutonomous); //set up the wifi stuff

  rightServo.attach(D5); //attach rightservo to pin D5
  leftServo.attach(D6);  //attach leftServo to pin D6

  pinMode(flashButton, INPUT); //set the flash button pin to an input

  ESP.wdtDisable(); //disable the software watchdog timer. the hardware timer is still active (for 6 seconds)
}

void loop()
{
  ESP.wdtFeed();

  server.handleClient(); //get any new info from the wifi client

  leftServoSpeed = map((joyStickY + joyStickX) / 2, -71, 71, -100, 100);  //calculate the value the left servo should spin at
  rightServoSpeed = map((joyStickY - joyStickX) / 2, -71, 71, -100, 100); //calculate the value the right servo should spin at

  //write the updated speeds to the servos
  setServoSpeed(leftServo, leftServoSpeed, leftServoError, false);
  setServoSpeed(rightServo, rightServoSpeed, rightServoError, true);

  //if the "flash" button is pressed, run autonomous
  if (digitalRead(flashButton) == LOW)
  {
    runAutonomous();
  }
}

void runAutonomous()
{
  unsigned int runTime = 2000; //the number of milliseconds that the bot should drive forward for

  //set both wheels to rotate at full speed forward
  setServoSpeed(leftServo, 100, leftServoError, false);
  setServoSpeed(rightServo, 100, rightServoError, true);

  //delay the program 2sec using my delay to avoid errors
  myDelay(2000);

  //stop both wheels
  setServoSpeed(leftServo, 0, leftServoError, false);
  setServoSpeed(rightServo, 0, rightServoError, true);
}

void saveServoError(int left, int right)
{

  DEBUG_SERIAL("\nsaving servo error\n");

  byte *l = (byte *)&left;
  for (int i = 0; i < 4; i++)
  {
    EEPROM.write(EEPROMServoErrorPosition + i, *(l + i));
  }

  byte *r = (byte *)&right;
  for (int i = 0; i < 4; i++)
  {
    EEPROM.write(EEPROMServoErrorPosition + i + 4, *(r + i));
  }

  EEPROM.commit();
}

void getServoError(int &left, int &right)
{

  DEBUG_SERIAL("\ngetting servo error");

  byte *l = (byte *)&left;
  for (int i = 0; i < 4; i++)
  {
    *(l + i) = EEPROM.read(EEPROMServoErrorPosition + i);
  }

  byte *r = (byte *)&right;
  for (int i = 0; i < 4; i++)
  {
    *(r + i) = EEPROM.read(EEPROMServoErrorPosition + i + 4);
  }

  DEBUG_SERIAL(left);
  DEBUG_SERIAL(right);
  DEBUG_SERIAL("\n");
}

//this function delays the program while still allowing the necessary background functions to run. it takes the delay time in ms as an argument
void myDelay(unsigned int runTime)
{
  //delay the program without calling delay. delay can't be called because it can mess up the web server running in the background
  unsigned long startTime = millis();      //record the current time as the start time
  while (runTime > (millis() - startTime)) //run a while loop for runTime number of milliseconds
  {
    yield(); //yield to the rest of the program  (the web server part)
  }
}

void setServoSpeed(Servo &servo, int speed, int error, bool reversed)
{
  if (reversed)
  {
    speed = -speed;
  }

  // DEBUG_SERIAL("error"); DEBUG_SERIAL(error);
  // DEBUG_SERIAL("speed"); DEBUG_SERIAL(speed);

  int power = map(speed, -100, 100, 1000, 2000) + error;

  // DEBUG_SERIAL("power"); DEBUG_SERIAL(power);
  // DEBUG_SERIAL("\n");

  servo.writeMicroseconds(power);
}
