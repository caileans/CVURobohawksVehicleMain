
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
#include <avr/pgmspace.h>
#include <ESP8266WebServer.h>



class CVURobohawksVehicleWiFi
{

private:
 //the main html code. this is sent when a new client requests the main web page
 static const char vehicleControlHTML[];

 //java script that creates a virtual joystick:
 static const char virtualJoyStickJS[];

 //the html for fine tuning the servo mid positions
 static const char wheelTuneHTML[];

  // const static char vehicleControlHTML[];
  // static const char[] virtualJoyStickJS = "hello2";
  // static const char[] wheelTuneHTML = "hello3";

  //create a ESP8266WebServer object server, using port 80
  ESP8266WebServer server;

  int *joyStickXPtr, *joyStickYPtr; //create two pointer vars that will point to two vars passed into
  //the setUpWiFi function. these pointers are used to assign the joystick X and Y values sent from a wifi client
  //to the variables used in the main program

  void (*autoFunctionptr)(); //a pointer used to call a autonomous function
  void (*saveWheelErrorFunctionPtr)(int, int);

  int *leftWheelErrorPtr, *rightWheelErrorPtr;

  
  //this function is called when a wifi client requests the main web page ("[IPaddress]/")
  friend void handleSendVehicleControlHTML(CVURobohawksVehicleWiFi*);

  friend void handleSendWheelTuneHTML(CVURobohawksVehicleWiFi*);

  //this function is called when the virtual joystick js code is requested (usually done by the main
  //html web page)
  friend void handleSendVirtualJoyStick(CVURobohawksVehicleWiFi*);

  //the handleJoyStickData function is called when the wifi client sends a new set of joy stick XY values
  friend void handleJoyStickData(CVURobohawksVehicleWiFi*);

  //this function is called when the main controller page requests autonomous to be run
  friend void handleRunAutonomous(CVURobohawksVehicleWiFi*);

  //this function is called when the wheel tune page requests the offsets be saved
  friend void handleSaveWheelErrors(CVURobohawksVehicleWiFi*);

  //this function is called when the wheel tune page requests the current wheel offsets
  friend void handleSendWheelErrors(CVURobohawksVehicleWiFi*);

  //this is called when the wheel tune page sends new offset values for the wheels
  friend void handleUpdateWheelErrors(CVURobohawksVehicleWiFi*);

public:

 CVURobohawksVehicleWiFi():server(80){}


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
  void setUpWiFi(char *wifiName, char *wifiPass, int chanel, int &joyStickXVar, int &joyStickYVar,
                 int &leftWheelError, int &rightWheelError, void saveWheelErrorFunction(int, int), void autoFunction());

/**
 * @brief checks for any updates from the wifi client (the controller)
 */
  void refreshWiFi();
};
