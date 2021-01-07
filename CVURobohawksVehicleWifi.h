//this file contains all of set up code needed to control an esp8266 tank drive vehicle over wifi
//last modified by Cailean Sorce on 9/5/20

//HTML and java script code  (in CVURobohawksVehicleControlHTML.html and CVURobohawksVehicleJoyStick.js)
//based on code from https://automatedhome.party/html-and-js-code-for-wifi-car/
#ifndef NO_DEBUG_SERIAL
#define DEBUG_SERIAL(message) (Serial.print(message))
#else
#define DEBUG_SERIAL(message) ((void)0)
#endif

//the main html code. this is sent when a new client requests the main web page
const char vehicleControlHTML[] PROGMEM =
#include "C:\\Users\Cailean\\Documents\\Arduino\\CVURobohawksVehicleMain\\CVURobohawksVehicleControlHTML.html"

//java script that creates a virtual joystick:
const char virtualJoyStickJS[] PROGMEM =
#include "C:\\Users\\Cailean\Documents\\Arduino\\CVURobohawksVehicleMain\\CVURobohawksVehicleJoyStick.js"


//the html for fine tuning the servo mid positions
const char servoTuneHTML[] PROGMEM =
#include "C:\\Users\\Cailean\\Documents\\Arduino\\CVURobohawksVehicleMain\\CVURobohawksVehicleServoTuneHTML.html"


//include the library's used for the wifi and web page
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


//create a ESP8266WebServer object server, using port 80
ESP8266WebServer server(80);

int *joyStickXPtr, *joyStickYPtr; //create two pointer vars that will point to two vars passed into
//the setUpWifi function. these pointers are used to assign the joystick X and Y values sent from a wifi client
//to the variables used in the main program

void (*autoFunctionptr)(); //a pointer used to call a autonomous function
void (*saveServoErrorFunctionPtr)(int, int);

int *leftServoErrorPtr, *rightServoErrorPtr;

//the handleSendVehicleControlHTML function is called when a wifi client requests the main web page ("(IPaddress)/"
void handleSendVehicleControlHTML()
{
  server.send(200, "text/html", vehicleControlHTML); //send the main html web page to the wifi clinet
}

void handleSendServoTuneHTML()
{
  server.send(200, "text/html", servoTuneHTML);
}

//the handleSendVirtualJoyStick function is called when the virtual joystick js code is requested (usually done by the main
//html web page)
void handleSendVirtualJoyStick()
{
  server.send(200, "application/javascript", virtualJoyStickJS); //send the virtual joystick js code to the wifi client
}

//the handleJoyStickData function is called when the wifi client sends a new set of joy stick XY values
void handleJoyStickData()
{
  DEBUG_SERIAL("updating joyStick possitions\n");

  *joyStickXPtr = server.arg("x").toInt(); //assign the first argument received to joyStickXPtr
  *joyStickYPtr = server.arg("y").toInt(); //assign the second argument received to joyStickYPtr

  server.send(200, "text/plain", ""); //respond to the server with 200 (okay) code
}

//this function is called when the main controller page requests autonomous to be run
void handleRunAutonomous()
{
  DEBUG_SERIAL("running autonomous\n");
  //run the autonomous function (via the pointer to it)
  (*autoFunctionptr)();

  //reply to the wifi client with an "okay " message
  server.send(200, "text/plain", "");
}

//this function is called when the servo mid position adjust page requests the positions be saved
void handleSaveServoErrors()
{
  DEBUG_SERIAL("saving servo mid positions: left=");
  DEBUG_SERIAL(*leftServoErrorPtr);
  DEBUG_SERIAL(" right=");
  DEBUG_SERIAL(*rightServoErrorPtr);
  DEBUG_SERIAL("\n");

  (*saveServoErrorFunctionPtr)(*leftServoErrorPtr, *rightServoErrorPtr);

  //reply to the wifi client with an "okay " message
  server.send(200, "text/plain", "");
}

//this function is called when the servo mid position adjust page requests the current servo mid positions
void handleSendServoErrors()
{
  DEBUG_SERIAL("sending ServoPositions:  ");

  //build the string to send
  char info[15] = "l=";
  itoa((*leftServoErrorPtr), info + strlen(info), 10);
  strcat(info, "&r=");
  itoa((*rightServoErrorPtr), info + strlen(info), 10);

  DEBUG_SERIAL(info); DEBUG_SERIAL("\n");
  //send the current servo mid positions to the page
  server.send(200, "text/plain", info);
}

void handleUpdateServoErrors()
{
  DEBUG_SERIAL("updating servo errors\n");

  *joyStickXPtr = *joyStickYPtr = 0;

  *leftServoErrorPtr = server.arg(0).toInt();  //assign the first argument received
  *rightServoErrorPtr = server.arg(1).toInt(); //assign the second argument received

  server.send(200, "text/plain", "");
}



//this function should be called in the main setup function. the following variables should be passed into the function:
//a cstring containing the name the the wifi network should have, the password that should be required to join the wifi
//network (MUST BE MORE THEN 8 CHARS), two vars of type int (must be int, can't be int8_t....), these two int vars will be
//updated with new XY joystick values sent from the wifi client every time server.handleClient() is called. a void function
//with no parameters should also be passed in. this function will be called when a client requests to run autonomous.
void setUpWiFi(char *wifiName, char *wifiPass, int channel, int &joyStickXVar, int &joyStickYVar,
               int &leftServoError, int &rightServoError, void saveServoErrorFunction(int, int), void autoFunction())
{
  //assign the joyStick x and y ptrs to the two var passed into the function
  joyStickXPtr = &joyStickXVar;
  joyStickYPtr = &joyStickYVar;

  leftServoErrorPtr = &leftServoError;
  rightServoErrorPtr = &rightServoError;

  //assign the autofunction passed in to the autoFunctionptr
  autoFunctionptr = autoFunction;

  saveServoErrorFunctionPtr = saveServoErrorFunction;

  ESP.eraseConfig();

  //set up the wifi
  Serial.println("Setting up access point...");
  Serial.println(WiFi.softAP(wifiName, wifiPass, channel, false, 1) ? "WiFi is Ready" : "WiFi Failed");

  //output what the IP address is
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  //WiFi.printDiag(Serial);

  //define what function should be called by each request made by the wifi client
  server.on("/", handleSendVehicleControlHTML);
  server.on("/CVURobohawksVehicleControlHTML.html", handleSendVehicleControlHTML);
  server.on("/CVURobohawksVehicleServoTuneHTML.html", handleSendServoTuneHTML);
  server.on("/CVURobohawksVehicleJoyStick.js", handleSendVirtualJoyStick);
  server.on("/runAutonomous", handleRunAutonomous);
  server.on("/jsData.html", handleJoyStickData);
  server.on("/saveServoErrors", handleSaveServoErrors);
  server.on("/servoErrors", handleSendServoErrors);
  server.on("/servoErrors.html", handleUpdateServoErrors);

  server.begin(); //start the server object

  Serial.println("\nServer Setup Complete.\n");
}
