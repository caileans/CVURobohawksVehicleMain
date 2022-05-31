
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

  #include "CVURobohawksVehicleWiFi.h"

  // const char CVURobohawksVehicleWiFi::vehicleControlHTML[] PROGMEM = "hello"; 
  
  //the main html code. this is sent when a new client requests the main web page
   const char CVURobohawksVehicleWiFi::vehicleControlHTML[] PROGMEM =
  #include "CVURobohawksVehicleControlHTML.h"
       ;

   //java script that creates a virtual joystick:
   const char CVURobohawksVehicleWiFi::virtualJoyStickJS[] PROGMEM =
  #include "CVURobohawksVehicleJoyStickJS.h"
       ;

   //the html for fine tuning the servo mid positions
   const char CVURobohawksVehicleWiFi::wheelTuneHTML[] PROGMEM =
  #include "CVURobohawksVehicleWheelTuneHTML.h"
   ;



  //this function is called when a wifi client requests the main web page ("[IPaddress]/")
  void handleSendVehicleControlHTML(CVURobohawksVehicleWiFi* WiFi)
  {
    WiFi->server.send(200, "text/html", WiFi->vehicleControlHTML); //send the main html web page to the wifi clinet
  }

  void handleSendWheelTuneHTML(CVURobohawksVehicleWiFi* WiFi)
  {
    WiFi->server.send(200, "text/html", WiFi->wheelTuneHTML);
  }

  //this function is called when the virtual joystick js code is requested (usually done by the main
  //html web page)
  void handleSendVirtualJoyStick(CVURobohawksVehicleWiFi* WiFi)
  {
    WiFi->server.send(200, "application/javascript", WiFi->virtualJoyStickJS); //send the virtual joystick js code to the wifi client
  }

  //the handleJoyStickData function is called when the wifi client sends a new set of joy stick XY values
  void handleJoyStickData(CVURobohawksVehicleWiFi* WiFi)
  {
    DEBUG_SERIAL("updating joyStick possitions\n");

    *WiFi->joyStickXPtr = WiFi->server.arg("x").toInt(); //assign the first argument received to joyStickXPtr
    *WiFi->joyStickYPtr = WiFi->server.arg("y").toInt(); //assign the second argument received to joyStickYPtr

    WiFi->server.send(200, "text/plain", ""); //respond to the server with 200 (okay) code
  }

  //this function is called when the main controller page requests autonomous to be run
  void handleRunAutonomous(CVURobohawksVehicleWiFi* WiFi)
  {
    DEBUG_SERIAL("running autonomous\n");
    //run the autonomous function (via the pointer to it)
    (*WiFi->autoFunctionptr)();

    //reply to the wifi client with an "okay " message
    WiFi->server.send(200, "text/plain", "");
  }

  //this function is called when the wheel tune page requests the offsets be saved
  void handleSaveWheelErrors(CVURobohawksVehicleWiFi* WiFi)
  {
    DEBUG_SERIAL("saving servo mid positions: left=");
    DEBUG_SERIAL(*WiFi->leftWheelErrorPtr);
    DEBUG_SERIAL(" right=");
    DEBUG_SERIAL(*WiFi->rightWheelErrorPtr);
    DEBUG_SERIAL("\n");

    (*WiFi->saveWheelErrorFunctionPtr)(*WiFi->leftWheelErrorPtr, *WiFi->rightWheelErrorPtr);

    //reply to the wifi client with an "okay " message
    WiFi->server.send(200, "text/plain", "");
  }

  //this function is called when the wheel tune page requests the current wheel offsets
  void handleSendWheelErrors(CVURobohawksVehicleWiFi* WiFi)
  {
    DEBUG_SERIAL("sending ServoPositions:  ");

    //build the string to send
    char info[15] = "l=";
    itoa((*WiFi->leftWheelErrorPtr), info + strlen(info), 10);
    strcat(info, "&r=");
    itoa((*WiFi->rightWheelErrorPtr), info + strlen(info), 10);

    DEBUG_SERIAL(info);
    DEBUG_SERIAL("\n");
    //send the current servo mid positions to the page
    WiFi->server.send(200, "text/plain", info);
  }

  //this is called when the wheel tune page sends new offset values for the wheels
  void handleUpdateWheelErrors(CVURobohawksVehicleWiFi* WiFi)
  {
    DEBUG_SERIAL("updating servo errors\n");

    *WiFi->joyStickXPtr = *WiFi->joyStickYPtr = 0;

    *WiFi->leftWheelErrorPtr = WiFi->server.arg(0).toInt();  //assign the first argument received
    *WiFi->rightWheelErrorPtr = WiFi->server.arg(1).toInt(); //assign the second argument received

    WiFi->server.send(200, "text/plain", "");
  }


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
  void CVURobohawksVehicleWiFi::setUpWiFi(char *wifiName, char *wifiPass, int chanel, int &joyStickXVar, int &joyStickYVar,
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
    Serial.println(WiFi.softAP(wifiName, wifiPass, chanel, false, 1) ? "WiFi is Ready" : "WiFi Failed");

    //output what the IP address is
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    //WiFi.printDiag(Serial);

//    ESP8266WebServer server(80);

    //define what function should be called by each request made by the wifi client
    server.on("/", std::bind(handleSendVehicleControlHTML, this));
    server.on("/CVURobohawksVehicleControlHTML.html", std::bind(handleSendVehicleControlHTML, this));
    server.on("/CVURobohawksVehicleServoTuneHTML.html", std::bind(handleSendWheelTuneHTML, this));
    server.on("/CVURobohawksVehicleJoyStick.js", std::bind(handleSendVirtualJoyStick, this));
    server.on("/runAutonomous", std::bind(handleRunAutonomous, this));
    server.on("/jsData.html", std::bind(handleJoyStickData, this));
    server.on("/saveWheelErrors", std::bind(handleSaveWheelErrors, this));
    server.on("/wheelErrors", std::bind(handleSendWheelErrors, this));
    server.on("/wheelErrors.html", std::bind(handleUpdateWheelErrors, this));

    server.begin(); //start the server object

    Serial.println("\nServer Setup Complete.\n");
  }

/**
 * @brief checks for any updates from the wifi client (the controller)
 */
  void CVURobohawksVehicleWiFi::refreshWiFi()
  {
    server.handleClient();
  }
