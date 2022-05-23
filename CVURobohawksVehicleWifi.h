
/**
 * @file CVURobohawksVehicleWiFi
 * @class CVURobohawksVehicleWiFi
 * @brief Contains CVURobohawksVehicleWiFi class, used to set up wifi on ESP8266 to control a tankdrive type vehicle
 * 
 * conatains a set up function, which should be called in setup(), and a refreshWifi function, which should be called in loop()
 * 
 * @author Cailean Sorce
 * @date 1/11/2021
 */

//define a debug macro
#ifndef NO_DEBUG_SERIAL
#define DEBUG_SERIAL(message) (Serial.print(message))
#else
#define DEBUG_SERIAL(message) ((void)0)
#endif

//include the library's used for the wifi and web page
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


class CVURobohawksVehicleWiFi
{

private:
  //the main html code. this is sent when a new client requests the main web page
  const char vehicleControlHTML[] PROGMEM =
#include "CVURobohawksVehicleControlHTML.html"
      ;

  //java script that creates a virtual joystick:
  const char virtualJoyStickJS[] PROGMEM =
#include "CVURobohawksVehicleJoyStickJS.js"
      ;

  //the html for fine tuning the servo mid positions
  const char wheelTuneHTML[] PROGMEM =
#include "CVURobohawksVehicleWheelTuneHTML.html"
      ;

  //create a ESP8266WebServer object server, using port 80
  ESP8266WebServer server(80);

  int *joyStickXPtr, *joyStickYPtr; //create two pointer vars that will point to two vars passed into
  //the setUpWiFi function. these pointers are used to assign the joystick X and Y values sent from a wifi client
  //to the variables used in the main program

  void (*autoFunctionptr)(); //a pointer used to call a autonomous function
  void (*saveWheelErrorFunctionPtr)(int, int);

  int *leftWheelErrorPtr, *rightWheelErrorPtr;

  //this function is called when a wifi client requests the main web page ("[IPaddress]/")
  void handleSendVehicleControlHTML()
  {
    server.send(200, "text/html", vehicleControlHTML); //send the main html web page to the wifi clinet
  }

  void handleSendWheelTuneHTML()
  {
    server.send(200, "text/html", wheelTuneHTML);
  }

  //this function is called when the virtual joystick js code is requested (usually done by the main
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

  //this function is called when the wheel tune page requests the offsets be saved
  void handleSaveWheelErrors()
  {
    DEBUG_SERIAL("saving servo mid positions: left=");
    DEBUG_SERIAL(*leftWheelErrorPtr);
    DEBUG_SERIAL(" right=");
    DEBUG_SERIAL(*rightWheelErrorPtr);
    DEBUG_SERIAL("\n");

    (*saveWheelErrorFunctionPtr)(*leftWheelErrorPtr, *rightWheelErrorPtr);

    //reply to the wifi client with an "okay " message
    server.send(200, "text/plain", "");
  }

  //this function is called when the wheel tune page requests the current wheel offsets
  void handleSendWheelErrors()
  {
    DEBUG_SERIAL("sending ServoPositions:  ");

    //build the string to send
    char info[15] = "l=";
    itoa((*leftWheelErrorPtr), info + strlen(info), 10);
    strcat(info, "&r=");
    itoa((*rightWheelErrorPtr), info + strlen(info), 10);

    DEBUG_SERIAL(info);
    DEBUG_SERIAL("\n");
    //send the current servo mid positions to the page
    server.send(200, "text/plain", info);
  }

  //this is called when the wheel tune page sends new offset values for the wheels
  void handleUpdateWheelErrors()
  {
    DEBUG_SERIAL("updating servo errors\n");

    *joyStickXPtr = *joyStickYPtr = 0;

    *leftWheelErrorPtr = server.arg(0).toInt();  //assign the first argument received
    *rightWheelErrorPtr = server.arg(1).toInt(); //assign the second argument received

    server.send(200, "text/plain", "");
  }

public:
  /**
 * @brief Creates a WiFi network and initializes an interface allowing control of a tankdrive vehicle with servo wheels over the WiFi network
 * @param wifiName A cstring containing the name of the wifi network to be created 
 * @param wifiPass A cstring containing the password for the wifi network (MUST BE MORE THEN 8 CHARS)
 * @param channel An integer [0, 14]. This is the wifi channel that will be used
 * @param joyStickXVar A integer variable used to store the X value of the virtual joyStick
 * @param joyStickYVar A integer variable used to store the Y value of the virtual joyStick
 * @param leftWheelError A integer variable used to store the error for the left servo
 * @param rightWheelError A interger variable used to store the error for the right servo
 * @param saveWheelErrorFunction A void type function with to integer paramenters. This function will be called 
 * with the left and right servo errors as parameters when the user wishes to "save the servo errors"
 * @param autoFunction A void type function with no parameters. This function will be called when the user wishes to "run autonomous"
 */
  void setUpWiFi(char *wifiName, char *wifiPass, int channel, int &joyStickXVar, int &joyStickYVar,
                 int &leftWheelError, int &rightWheelError, void saveWheelErrorFunction(int, int), void autoFunction())
  {
    //assign the joyStick x and y ptrs to the two var passed into the function
    joyStickXPtr = &joyStickXVar;
    joyStickYPtr = &joyStickYVar;

    leftWheelErrorPtr = &leftWheelError;
    rightWheelErrorPtr = &rightWheelError;

    //assign the autofunction passed in to the autoFunctionptr
    autoFunctionptr = autoFunction;

    saveWheelErrorFunctionPtr = saveWheelErrorFunction;

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
    server.on("/CVURobohawksVehicleServoTuneHTML.html", handleSendWheelTuneHTML);
    server.on("/CVURobohawksVehicleJoyStick.js", handleSendVirtualJoyStick);
    server.on("/runAutonomous", handleRunAutonomous);
    server.on("/jsData.html", handleJoyStickData);
    server.on("/saveWheelErrors", handleSaveWheelErrors);
    server.on("/wheelErrors", handleSendWheelErrors);
    server.on("/wheelErrors.html", handleUpdateWheelErrors);

    server.begin(); //start the server object

    Serial.println("\nServer Setup Complete.\n");
  }

/**
 * @brief checks for any updates from the wifi client (the controller)
 */
  void refreshWiFi()
  {
    server.handleClient();
  }
}