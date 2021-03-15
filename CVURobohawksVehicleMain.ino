
/**
 * @author Cailean Sorce
 * @date 1/10/2021
 * @brief Arduino vehicle example for CVU Robohawks. this runs on nodeMCU esp8266 dev board for wifi access
 */

// include the necessary files
#include <Servo.h> //used to easily control the wheel servos
#include <EEPROM.h> //used to store the servo errors in EEPROM (flash) memoory, so that it can be saved through power down
#include "CVURobohawksVehicleWifi.h" //contains the html + js and funcitons used to set up and interface with the
//wifi client that will be used to control the vehicle
#include "CaileanSorce_nodemcuESP8266_pinDefinitions.h" //contains the const ints used to convert the GPIO port names silkscreened on the
//dev board to the esp8266 GPIO numbers


// declare the variables/ objects
char wifiName[] = "CSorceWiFi",      //the name of the wifi network that will be created
    wifiPassword[] = "CaileanSorce"; //the password to join the wifi network. MUST BE >= 8 characters

int joyStickX = 0, joyStickY = 0,    //used to store the joyStick positions. updated with the wifi joystick
    rightServoSpeed, leftServoSpeed, //used to store the speeds for the servos
    rightServoError, leftServoError; //used to store the errors for the servos

int EEPROMServoErrorPosition = 0; //the position that the servo errors will be stored in EEPROM

Servo rightServo, leftServo; //create two servo objects

CVURobohawksVehicleWiFi WiFiControl;

const int flashButton = 0; //the flash button on the nodemcu is on GPIO 0 (thats esp8266 GPIO 0, not D0)



// delare the functions

/**
 * @brief Contains the code to execute the robot movements for autonomous
*/
void runAutonomous();

/**
 * @brief Saves the wheel errors to EEPROM
 * @param left The error for the left wheel to be saved
 * @param right The error for the right wheel to be saved
*/
void saveWheelError(int left, int right);

/**
 * @brief retrieves the wheel errors from EEPROM
 * @param left will be set to the saved left wheel error
 * @param right will be set to the saved right wheel error
*/
void getWheelError(int &left, int &right);

/**
 * @brief Delays the program while still allowing background functions to run. 
 * uses while loop and yeild() to delay
 * @param runTime The amount of time to delay for, in millis
*/
void myDelay(unsigned int runTime);

/**
 * @brief Provides an easy way to set the wheel servos to a speed.
 * @param servo The servo object to set the speed
 * @param speed The speed to set the servo to, from -100 to 100, with 0 being stopped
 * @param error The error for the wheel; the value to adjust the speed to to achieve no rotation at a speed of 0
 * @param reversed Whether the positive dirrection of the servo wheel is reversed relative to the bot 
*/
void setServoSpeed(Servo &servo, int speed, int error, bool reversed);

void setup()
{

  Serial.begin(115200); //begin serial communication at 115200 baud rate

  EEPROM.begin(512); //begin EEPROM access

  getWheelError(leftServoError, rightServoError); //get the saved wheel errors

  WiFiControl.setUpWiFi(wifiName, wifiPassword, 8, joyStickX, joyStickY, leftServoError, rightServoError, saveWheelError, runAutonomous); //set up the wifi stuff

  rightServo.attach(D5); //attach rightservo to pin D5
  leftServo.attach(D6);  //attach leftServo to pin D6

  pinMode(flashButton, INPUT); //set the flash button pin to an input

  ESP.wdtDisable(); //disable the software watchdog timer. the hardware timer is still active (for 6 seconds)
}

void loop()
{
  ESP.wdtFeed(); //feed the watch dog timer. this should be done automatically, but call the funcion to avoid errors

  WiFiControl.refreshWiFi(); //get any new info from the wifi client

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

void saveWheelError(int left, int right)
{

  //write the left error to EEPROM
  byte *l = (byte *)&left;
  for (int i = 0; i < 4; i++)
  {
    EEPROM.write(EEPROMServoErrorPosition + i, *(l + i));
  }

  //write the right error to EEPROM
  byte *r = (byte *)&right;
  for (int i = 0; i < 4; i++)
  {
    EEPROM.write(EEPROMServoErrorPosition + i + 4, *(r + i));
  }

  //save the values
  EEPROM.commit();
}

void getWheelError(int &left, int &right)
{

  //read the left error from EEPROM
  byte *l = (byte *)&left;
  for (int i = 0; i < 4; i++)
  {
    *(l + i) = EEPROM.read(EEPROMServoErrorPosition + i);
  }

  //read the right error from EEPROM
  byte *r = (byte *)&right;
  for (int i = 0; i < 4; i++)
  {
    *(r + i) = EEPROM.read(EEPROMServoErrorPosition + i + 4);
  }
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

  //reverse the speed if reversed is true
  if (reversed)
  {
    speed = -speed;
  }

  //map the speed power to a value that can be given to the servo
  int power = map(speed, -100, 100, 1000, 2000) + error;

  //write the power to the servo
  servo.writeMicroseconds(power);
}
