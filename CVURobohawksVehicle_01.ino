//Cailean Sorce   8/27/20
//Arduino vehicle example for CVU Robohawks
//this runs on nodeMCU esp8266 dev board for wifi access


#include <Servo.h> //used to easily control the wheel servos
#include "CVURobohawksVehicleWifi_01.h" //contains the html + js and funcitons used to set up and interface with the 
//wifi client that will be used to control the vehicle
#include "CaileanSorce_nodemcuESP8266_pinDefinitions.h" //contains the const ints used to convert the GPIO port names silkscreened on the
//dev board to the esp8266 GPIO numbers


char *wifiName = "CSorceWiFi", //the name of the wifi network that will be created
      *wifiPassword = "CaileanSorce"; //the password to join the wifi network. MUST BE >= 8 characters

int joyStickX = 0, joyStickY = 0; //used to store the joyStick positions. updated with the wifi joystick

Servo rightServo, leftServo; //create two servo objects
int rightServoSpeed, leftServoSpeed; //used to store the speeds for the left and right servos

const int flashButton = 0; //the flash button on the nodemcu is on GPIO 0 (thats esp8266 GPIO 0, not D0)


//this funciton is called when the user whats to run autonomous
void runAutonomous()
{
  unsigned int runTime = 2000; //the number of milliseconds that the bot should drive forward for

  //set both wheels to rotate at full speed forward
  leftServo.write(180); 
  rightServo.write(0);

  //delay the program without calling delay. delay can't be called because it can mess up the web server running in the backgroud
  unsigned long startTime = millis(); //record the current time as the start time
  while (runTime > (millis() - startTime)) //run a while loop for runTime number of milliseconds
  {
    yield(); //yield to the rest of the program  (the web server part)
  }

  //stop both wheels
  leftServo.write(90);
  rightServo.write(90);
}

void setup() {

  Serial.begin(115200); //begin serial communication at 115200 baude rate

  setUpWiFi(wifiName, wifiPassword, 6, joyStickX, joyStickY, runAutonomous); //set up the wifi stuff

  rightServo.attach(D5); //attach rightservo to pin D5
  leftServo.attach(D6); //atach leftServo to pin D6

  pinMode(flashButton, INPUT); //set the flash button pin to an input

 
  ESP.wdtDisable(); //disable the software watchdog timer. the hardware timer is still active (for 6 seconds)
}

void loop() {
  server.handleClient(); //get any new info from the wifi client

  rightServoSpeed = map(-(joyStickY - joyStickX)/2, -100, 100, 0, 180); //calculate the value the right servo should spin at (0 = backwards, 90 = stop, 180 = forwards)
  leftServoSpeed = map((joyStickY + joyStickX)/2, -100, 100, 0, 180); //calulate the value the left servo should spin at (0 = backwards, 90 = stop, 180 = forwards)

  //print out the follwoing info to the serial monitor for debugging
  Serial.print("Xjoy = "); Serial.println(joyStickX);
  Serial.print("Yjoy = "); Serial.println(joyStickY);
  Serial.print("rightServo = "); Serial.println(rightServoSpeed);
  Serial.print("leftServo = "); Serial.println(leftServoSpeed);

  //write the updated speeds to the servos
  rightServo.write(rightServoSpeed);
  leftServo.write(leftServoSpeed);

  //if the "flash" button is pressed, run autonomous
  if (digitalRead(flashButton) == LOW)
  {
    runAutonomous();
  }
}
