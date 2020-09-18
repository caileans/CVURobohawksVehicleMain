//Cailean Sorce   9/12/20
//ESP8266 example: how to use the CVURobohawksVehicleWifi.h library


//first you need to include the wifi library, which contains the html + js and funcitons used to set up and interface with the wifi client that will be used to 
//control the vehicle
#include "CVURobohawksVehicleWifi_01.h" 


//next create two c string variables. one needs to be the name of the wifi network that will be created, the other should be the password you will 
//need to join the wifi network
char wifiName[] = "CSorceWiFi", //the name of the wifi network that will be created
      wifiPassword[] = "CaileanSorce"; //the password to join the wifi network. MUST BE >= 8 characters!!!


//create two variables to that will hold the X and Y position of the virtual joy stick
int joyStickX, joyStickY; 



//create a void type function with no arguments. this will be called when you tell the robot to run autonomous
void runAutonomous() 
{

}

//in your setup function two funcitons need to be called....
void setup() {

    //first you need to begin serial communication. this is so that the esp8266 can comunicate with your computer via USB
    Serial.begin(115200); //begin serial communication at 115200 baude rate

    //then you need to call the setUpWiFi funcion and pass it 6 arguments in the following order: the two cstring variable previously created that hold the 
    //name and password of the wifi network that will be created.       a integer between 1 and 11 (this can be a variable). this  is the the WiFi channel that will be used. 
    //this has to be set to 8 to work at CVU.       Two variables of type int that the WiFi librarys will set eaqual to the virtual joysticks X and Y positions every time 
    //server.handle client is called.      A void type function that takes no arguments. The WiFi librarys will call this function when the "run autonomous" button is pressed.
    setUpWiFi(wifiName, wifiPassword, 8, joyStickX, joyStickY, runAutonomous);
}

void loop() {
    //call server.handleclient() at the begining of the loop(). this will check to see is any new info has been sent by the wifi client(s)
    server.handleClient(); 


}
