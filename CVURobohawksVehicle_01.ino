//Cailean Sorce   8/27/20
//Arduino vehicle example for CVU Robohawks
//this runs on nodeMCU esp8266 dev board for wifi access


char *wifiName = "CSorceWiFi", //the name of the wifi network that will be created
      *wifiPassword = "CaileanSorce"; //the password to join the wifi network. MUST BE >= 8 characters

int joyStickX, joyStickY; //used to store the joyStick positions. updated with the wifi joystick


#include <Servo.h>
#include "CaileanSorce_esp8266_wifiJoyStick.h"
#include "CaileanSorce_nodemcuESP826_pinDefinitions.h"


Servo rightServo, leftServo; //create two servo objects
int rightServoSpeed, leftServoSpeed; //used to store the speeds for the left and right servos

void setup() {

  Serial.begin(115200); //begin serial communication at 115200 baude rate

  setUpWiFi(); //set up the wifi stuff

  rightServo.attach(D5); //attach rightservo to pin D5
  leftServo.attach(D6); //atach leftServo to pin D6

  ESP.wdtDisable(); //disable the software watchdog timer. the hardware timer is still active for 6 seconds
}

void loop() {
  server.handleClient(); //get any new info from the virtual joy stick

  rightServoSpeed = map(-(joyStickY - joyStickX)/2, -1000, 1000, 0, 180); //calculate the value the right servo should spin at (0 = backwards, 90 = stop, 180 = forwards)
  leftServoSpeed = map((joyStickY + joyStickX)/2, -1000, 1000, 0, 180); //calulate the value the left servo should spin at (0 = backwards, 90 = stop, 180 = forwards)

  //print out the follwoing info to the serial monitor for debugging
  Serial.print("Xjoy = "); Serial.println(joyStickX);
  Serial.print("Yjoy = "); Serial.println(joyStickY);
  Serial.print("rightServo = "); Serial.println(rightServoSpeed);
  Serial.print("leftServo = "); Serial.println(leftServoSpeed);

  //write the updated speeds to the servos
  rightServo.write(rightServoSpeed);
  leftServo.write(leftServoSpeed);
}
