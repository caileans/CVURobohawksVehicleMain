//this file contains all of the html and java script that is used to control an esp8266 tank drive vehicle over wifi
//last modified by Cailean Sorce on 9/5/20

//HTML and java script code based on code from https://automatedhome.party/html-and-js-code-for-wifi-car/

//the main html code. this is sent when a new client requests the main web page
const char mainHTML[] PROGMEM = R"====(
<html>
  <head>
    <meta charset="utf-8"/>
    <meta name="viewport" content="width=device-width, user-scalable=no, minimum-scale=1.0, maximum-scale=1.0"/>

    <style>
        body {
          overflow  : hidden;
          padding   : 0;
          margin    : 0;
          background-color: #BBB;
        }
        #info {
          position  : absolute;
          top   : 0px;
          width   : 100%;
          padding   : 5px;
          text-align  : center;
        }
        #info a {
          color   : #66F;
          text-decoration : none;
        }
        #info a:hover {
          text-decoration : underline;
        }
        #container {
          width   : 100%;
          height    : 100%;
          overflow  : hidden;
          padding   : 0;
          margin    : 0;
          -webkit-user-select : none;
          -moz-user-select  : none;
        }

        button#run-autonomous {
            border: solid grey 1px;
            background-color: cyan;
            cursor: pointer;
            transition: 0.2s ease-in-out;
        }

        button#run-autonomous:hover {
            box-shadow: 0 0px 8px 0 rgba(0,0,0,0.2);
        }

        span#autonomous-progress {
            color: grey;
            font-weight: bold;
        }

    </style>

  </head>
  <body>
    <div id="container"></div>
    <div id="info">
        <span id="button">
            <button id="run-autonomous" onclick="runAutonomous()">Run autonomous</button>
            <span id="autonomous-progress" style="display:none">Running autonomous...</span>
        </span>
        or
      Touch the screen to move
      <br/>
      <span id="result"></span>
    </div>
    <script src="./virtualjoystick.js"></script>
    <script>
      console.log("touchscreen is", VirtualJoystick.touchScreenAvailable() ? "available" : "not available");

      var joystick  = new VirtualJoystick({
        container : document.getElementById('container'),
        mouseSupport  : true,
        limitStickTravel  : true,
      });
      joystick.addEventListener('touchStart', function(){
        console.log('down')
      })
      joystick.addEventListener('touchEnd', function(){
        console.log('up')
      })
      var prevX = 0;
      var prevY = 0;
      var newX = 0;
      var newY = 0;

      setInterval(function(){
        var outputEl  = document.getElementById('result');
        newX = Math.round(joystick.deltaX());
        newY = Math.round(joystick.deltaY()) * -1;
        outputEl.innerHTML  = '<b>Position:</b> '
          + ' X:'+newX
          + ' Y:'+newY;
        if ( newX != prevX || newY != prevY ){
          var xhr = new XMLHttpRequest();
          xhr.open('PUT', "./jsData.html?x="+newX+"&y="+newY)
          xhr.send();
        }
        prevX = newX;
        prevY = newY;
      }, 1/10 * 1000);

      function setDisplay(id, display){
          document.getElementById(id).style.display = display;
      }

      function runAutonomous(){
          setDisplay('run-autonomous', 'none');
          setDisplay('autonomous-progress', 'inline');

          var xhr = new XMLHttpRequest();

          xhr.onreadystatechange = function(){
              if(this.readyState == 4 && this.status == 200){
                  setDisplay('run-autonomous', 'inline');
                  setDisplay('autonomous-progress', 'none');
              }
          }
          xhr.open('GET', '/runAutonomous');
          xhr.send();
      }
    </script>
  </body>
</html>
)====";




//java script that creats a virtual joystick:
const char virtualJoyStickJS[] PROGMEM = R"====(
var VirtualJoystick  = function(opts)
{
  opts      = opts      || {};
  this._container   = opts.container  || document.body;
  this._strokeStyle = opts.strokeStyle  || 'cyan';
  this._stickEl   = opts.stickElement || this._buildJoystickStick();
  this._baseEl    = opts.baseElement  || this._buildJoystickBase();
  this._mouseSupport  = opts.mouseSupport !== undefined ? opts.mouseSupport : false;
  this._stationaryBase  = opts.stationaryBase || false;
  this._baseX   = this._stickX = opts.baseX || 0
  this._baseY   = this._stickY = opts.baseY || 0
  this._limitStickTravel  = opts.limitStickTravel || false
  this._stickRadius = opts.stickRadius !== undefined ? opts.stickRadius : 100
  this._useCssTransform = opts.useCssTransform !== undefined ? opts.useCssTransform : false

  this._container.style.position  = "relative"

  this._container.appendChild(this._baseEl)
  this._baseEl.style.position = "absolute"
  this._baseEl.style.display  = "none"
  this._container.appendChild(this._stickEl)
  this._stickEl.style.position  = "absolute"
  this._stickEl.style.display = "none"

  this._pressed = false;
  this._touchIdx  = null;

  if(this._stationaryBase === true){
    this._baseEl.style.display  = "";
    this._baseEl.style.left   = (this._baseX - this._baseEl.width /2)+"px";
    this._baseEl.style.top    = (this._baseY - this._baseEl.height/2)+"px";
  }

  this._transform = this._useCssTransform ? this._getTransformProperty() : false;
  this._has3d = this._check3D();

  var __bind  = function(fn, me){ return function(){ return fn.apply(me, arguments); }; };
  this._$onTouchStart = __bind(this._onTouchStart , this);
  this._$onTouchEnd = __bind(this._onTouchEnd , this);
  this._$onTouchMove  = __bind(this._onTouchMove  , this);
  this._container.addEventListener( 'touchstart'  , this._$onTouchStart , false );
  this._container.addEventListener( 'touchend'  , this._$onTouchEnd , false );
  this._container.addEventListener( 'touchmove' , this._$onTouchMove  , false );
  if( this._mouseSupport ){
    this._$onMouseDown  = __bind(this._onMouseDown  , this);
    this._$onMouseUp  = __bind(this._onMouseUp  , this);
    this._$onMouseMove  = __bind(this._onMouseMove  , this);
    this._container.addEventListener( 'mousedown' , this._$onMouseDown  , false );
    this._container.addEventListener( 'mouseup' , this._$onMouseUp  , false );
    this._container.addEventListener( 'mousemove' , this._$onMouseMove  , false );
  }
}

VirtualJoystick.prototype.destroy = function()
{
  this._container.removeChild(this._baseEl);
  this._container.removeChild(this._stickEl);

  this._container.removeEventListener( 'touchstart' , this._$onTouchStart , false );
  this._container.removeEventListener( 'touchend'   , this._$onTouchEnd , false );
  this._container.removeEventListener( 'touchmove'  , this._$onTouchMove  , false );
  if( this._mouseSupport ){
    this._container.removeEventListener( 'mouseup'    , this._$onMouseUp  , false );
    this._container.removeEventListener( 'mousedown'  , this._$onMouseDown  , false );
    this._container.removeEventListener( 'mousemove'  , this._$onMouseMove  , false );
  }
}

/**
 * @returns {Boolean} true if touchscreen is currently available, false otherwise
*/
VirtualJoystick.touchScreenAvailable  = function()
{
  return 'createTouch' in document ? true : false;
}

/**
 * microevents.js - https://github.com/jeromeetienne/microevent.js
*/
;(function(destObj){
  destObj.addEventListener  = function(event, fct){
    if(this._events === undefined)  this._events  = {};
    this._events[event] = this._events[event] || [];
    this._events[event].push(fct);
    return fct;
  };
  destObj.removeEventListener = function(event, fct){
    if(this._events === undefined)  this._events  = {};
    if( event in this._events === false  )  return;
    this._events[event].splice(this._events[event].indexOf(fct), 1);
  };
  destObj.dispatchEvent   = function(event /* , args... */){
    if(this._events === undefined)  this._events  = {};
    if( this._events[event] === undefined ) return;
    var tmpArray  = this._events[event].slice();
    for(var i = 0; i < tmpArray.length; i++){
      var result  = tmpArray[i].apply(this, Array.prototype.slice.call(arguments, 1))
      if( result !== undefined )  return result;
    }
    return undefined
  };
})(VirtualJoystick.prototype);

//////////////////////////////////////////////////////////////////////////////////
//                    //
//////////////////////////////////////////////////////////////////////////////////

VirtualJoystick.prototype.deltaX  = function(){ return this._stickX - this._baseX;  }
VirtualJoystick.prototype.deltaY  = function(){ return this._stickY - this._baseY;  }

VirtualJoystick.prototype.up  = function(){
  if( this._pressed === false ) return false;
  var deltaX  = this.deltaX();
  var deltaY  = this.deltaY();
  if( deltaY >= 0 )       return false;
  if( Math.abs(deltaX) > 2*Math.abs(deltaY) ) return false;
  return true;
}
VirtualJoystick.prototype.down  = function(){
  if( this._pressed === false ) return false;
  var deltaX  = this.deltaX();
  var deltaY  = this.deltaY();
  if( deltaY <= 0 )       return false;
  if( Math.abs(deltaX) > 2*Math.abs(deltaY) ) return false;
  return true;
}
VirtualJoystick.prototype.right = function(){
  if( this._pressed === false ) return false;
  var deltaX  = this.deltaX();
  var deltaY  = this.deltaY();
  if( deltaX <= 0 )       return false;
  if( Math.abs(deltaY) > 2*Math.abs(deltaX) ) return false;
  return true;
}
VirtualJoystick.prototype.left  = function(){
  if( this._pressed === false ) return false;
  var deltaX  = this.deltaX();
  var deltaY  = this.deltaY();
  if( deltaX >= 0 )       return false;
  if( Math.abs(deltaY) > 2*Math.abs(deltaX) ) return false;
  return true;
}

//////////////////////////////////////////////////////////////////////////////////
//                    //
//////////////////////////////////////////////////////////////////////////////////

VirtualJoystick.prototype._onUp = function()
{
  this._pressed = false;
  this._stickEl.style.display = "none";

  if(this._stationaryBase == false){
    this._baseEl.style.display  = "none";

    this._baseX = this._baseY = 0;
    this._stickX  = this._stickY  = 0;
  }
}

VirtualJoystick.prototype._onDown = function(x, y)
{
  this._pressed = true;
  if(this._stationaryBase == false){
    this._baseX = x;
    this._baseY = y;
    this._baseEl.style.display  = "";
    this._move(this._baseEl.style, (this._baseX - this._baseEl.width /2), (this._baseY - this._baseEl.height/2));
  }

  this._stickX  = x;
  this._stickY  = y;

  if(this._limitStickTravel === true){
    var deltaX  = this.deltaX();
    var deltaY  = this.deltaY();
    var stickDistance = Math.sqrt( (deltaX * deltaX) + (deltaY * deltaY) );
    if(stickDistance > this._stickRadius){
      var stickNormalizedX = deltaX / stickDistance;
      var stickNormalizedY = deltaY / stickDistance;

      this._stickX = stickNormalizedX * this._stickRadius + this._baseX;
      this._stickY = stickNormalizedY * this._stickRadius + this._baseY;
    }
  }

  this._stickEl.style.display = "";
  this._move(this._stickEl.style, (this._stickX - this._stickEl.width /2), (this._stickY - this._stickEl.height/2));
}

VirtualJoystick.prototype._onMove = function(x, y)
{
  if( this._pressed === true ){
    this._stickX  = x;
    this._stickY  = y;

    if(this._limitStickTravel === true){
      var deltaX  = this.deltaX();
      var deltaY  = this.deltaY();
      var stickDistance = Math.sqrt( (deltaX * deltaX) + (deltaY * deltaY) );
      if(stickDistance > this._stickRadius){
        var stickNormalizedX = deltaX / stickDistance;
        var stickNormalizedY = deltaY / stickDistance;

        this._stickX = stickNormalizedX * this._stickRadius + this._baseX;
        this._stickY = stickNormalizedY * this._stickRadius + this._baseY;
      }
    }

          this._move(this._stickEl.style, (this._stickX - this._stickEl.width /2), (this._stickY - this._stickEl.height/2));
  }
}


//////////////////////////////////////////////////////////////////////////////////
//    bind touch events (and mouse events for debug)      //
//////////////////////////////////////////////////////////////////////////////////

VirtualJoystick.prototype._onMouseUp  = function(event)
{
  return this._onUp();
}

VirtualJoystick.prototype._onMouseDown  = function(event)
{
  event.preventDefault();
  var x = event.clientX;
  var y = event.clientY;
  return this._onDown(x, y);
}

VirtualJoystick.prototype._onMouseMove  = function(event)
{
  var x = event.clientX;
  var y = event.clientY;
  return this._onMove(x, y);
}

//////////////////////////////////////////////////////////////////////////////////
//    comment               //
//////////////////////////////////////////////////////////////////////////////////

VirtualJoystick.prototype._onTouchStart = function(event)
{
  // if there is already a touch inprogress do nothing
  if( this._touchIdx !== null ) return;

  // notify event for validation
  var isValid = this.dispatchEvent('touchStartValidation', event);
  if( isValid === false ) return;

  // dispatch touchStart
  this.dispatchEvent('touchStart', event);

  event.preventDefault();
  // get the first who changed
  var touch = event.changedTouches[0];
  // set the touchIdx of this joystick
  this._touchIdx  = touch.identifier;

  // forward the action
  var x   = touch.pageX;
  var y   = touch.pageY;
  return this._onDown(x, y)
}

VirtualJoystick.prototype._onTouchEnd = function(event)
{
  // if there is no touch in progress, do nothing
  if( this._touchIdx === null ) return;

  // dispatch touchEnd
  this.dispatchEvent('touchEnd', event);

  // try to find our touch event
  var touchList = event.changedTouches;
  for(var i = 0; i < touchList.length && touchList[i].identifier !== this._touchIdx; i++);
  // if touch event isnt found,
  if( i === touchList.length) return;

  // reset touchIdx - mark it as no-touch-in-progress
  this._touchIdx  = null;

//??????
// no preventDefault to get click event on ios
event.preventDefault();

  return this._onUp()
}

VirtualJoystick.prototype._onTouchMove  = function(event)
{
  // if there is no touch in progress, do nothing
  if( this._touchIdx === null ) return;

  // try to find our touch event
  var touchList = event.changedTouches;
  for(var i = 0; i < touchList.length && touchList[i].identifier !== this._touchIdx; i++ );
  // if touch event with the proper identifier isnt found, do nothing
  if( i === touchList.length) return;
  var touch = touchList[i];

  event.preventDefault();

  var x   = touch.pageX;
  var y   = touch.pageY;
  return this._onMove(x, y)
}


//////////////////////////////////////////////////////////////////////////////////
//    build default stickEl and baseEl        //
//////////////////////////////////////////////////////////////////////////////////

/**
 * build the canvas for joystick base
 */
VirtualJoystick.prototype._buildJoystickBase  = function()
{
  var canvas  = document.createElement( 'canvas' );
  canvas.width  = 126;
  canvas.height = 126;

  var ctx   = canvas.getContext('2d');
  ctx.beginPath();
  ctx.strokeStyle = this._strokeStyle;
  ctx.lineWidth = 6;
  ctx.arc( canvas.width/2, canvas.width/2, 40, 0, Math.PI*2, true);
  ctx.stroke();

  ctx.beginPath();
  ctx.strokeStyle = this._strokeStyle;
  ctx.lineWidth = 2;
  ctx.arc( canvas.width/2, canvas.width/2, 60, 0, Math.PI*2, true);
  ctx.stroke();

  return canvas;
}

/**
 * build the canvas for joystick stick
 */
VirtualJoystick.prototype._buildJoystickStick = function()
{
  var canvas  = document.createElement( 'canvas' );
  canvas.width  = 86;
  canvas.height = 86;
  var ctx   = canvas.getContext('2d');
  ctx.beginPath();
  ctx.strokeStyle = this._strokeStyle;
  ctx.lineWidth = 6;
  ctx.arc( canvas.width/2, canvas.width/2, 40, 0, Math.PI*2, true);
  ctx.stroke();
  return canvas;
}

//////////////////////////////////////////////////////////////////////////////////
//    move using translate3d method with fallback to translate > 'top' and 'left'
//      modified from https://github.com/component/translate and dependents
//////////////////////////////////////////////////////////////////////////////////

VirtualJoystick.prototype._move = function(style, x, y)
{
  if (this._transform) {
    if (this._has3d) {
      style[this._transform] = 'translate3d(' + x + 'px,' + y + 'px, 0)';
    } else {
      style[this._transform] = 'translate(' + x + 'px,' + y + 'px)';
    }
  } else {
    style.left = x + 'px';
    style.top = y + 'px';
  }
}

VirtualJoystick.prototype._getTransformProperty = function()
{
  var styles = [
    'webkitTransform',
    'MozTransform',
    'msTransform',
    'OTransform',
    'transform'
  ];

  var el = document.createElement('p');
  var style;

  for (var i = 0; i < styles.length; i++) {
    style = styles[i];
    if (null != el.style[style]) {
      return style;
    }
  }
}

VirtualJoystick.prototype._check3D = function()
{
  var prop = this._getTransformProperty();
  // IE8<= doesn't have `getComputedStyle`
  if (!prop || !window.getComputedStyle) return module.exports = false;

  var map = {
    webkitTransform: '-webkit-transform',
    OTransform: '-o-transform',
    msTransform: '-ms-transform',
    MozTransform: '-moz-transform',
    transform: 'transform'
  };

  // from: https://gist.github.com/lorenzopolidori/3794226
  var el = document.createElement('div');
  el.style[prop] = 'translate3d(1px,1px,1px)';
  document.body.insertBefore(el, null);
  var val = getComputedStyle(el).getPropertyValue(map[prop]);
  document.body.removeChild(el);
  var exports = null != val && val.length && 'none' != val;
  return exports;
}
)====";


//include the librarys used for the wifi and web page
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

//create a ESP8266WebServer object server, using port 80
ESP8266WebServer server(80);


int *joyStickXptr, *joyStickYptr; //create two pointer vars that will point to two vars passed into
//the setUpWifi function. these pointers are used to assign the joystick X and Y values sent from a wifi client
//to the varaibles used in the main program

void (*autoFunctionptr)();

//the handleRoot function is called when a wifi client requests the main web page ("(IPaddress)/"
void handleRoot()
{
  server.send(200, "text/html", mainHTML); //send the main html web page to the wifi clinet
}

//the handleSendVirtualJoyStick funciton is called when the virtual joystick js code is requested (usually done by the main
//html web page)
void handleSendVirtualJoyStick()
{
    server.send(200, "application/javascript", virtualJoyStickJS); //send the virtual joystick js code to the wifi client
}

//the handleJoyStickData funciton is called when the wifi client sends a new set of joy stick XY values
void handleJoyStickData()
{
  *joyStickXptr = server.arg(0).toInt(); //assign the first argument recienved to joyStickXptr
  *joyStickYptr = server.arg(1).toInt(); //assign the second argument recieved to joyStickYptr

  server.send(200, "text/plain", ""); //respond to the server with 200 (okay) code
}

void handleRunAutonomous()
{
  //run the autonomous function (via the pointer to it)
  (*autoFunctionptr)();

  //reply to the wifi client with an "okay " message
  server.send(200, "text/plain", "");
}

//this function should be called in the main setup function. the following variables should be passed into the function:
//a cstring containing the name the the wifi network should have, the password that should be required to join the wifi
//network (MUST BE MORE THEN 8 CHARS), two vars of type int (must be int, can't be int8_t....), these two int vars will be
//updated with new XY joystick values sent from the wifi client every time server.handleClient() is called. a void funciton
//with no parameters should also be passed in. this funciton will be called when a client requests to run autonomous.
void setUpWiFi(char *wifiName, char *wifiPass, int& joyStickXvar, int& joyStickYvar, void autoFunction())
{
  //asign the joyStick x and y ptrs to the two var passed into the function
  joyStickXptr = &joyStickXvar;
  joyStickYptr = &joyStickYvar;

  //assign the autofunction passed in to the autoFunctionptr
  autoFunctionptr = autoFunction;

  //set up the wifi
  Serial.println("Setting up access point...");
  Serial.println(WiFi.softAP(wifiName, wifiPass) ? "WiFi is Ready" : "WiFi Failed");

  //output what the IP address is
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  //define what funciton should be called by each request made by the wifi client
  server.on("/", handleRoot);
  server.on("/virtualjoystick.js", handleSendVirtualJoyStick);
  server.on("/runAutonomous", handleRunAutonomous);
  server.on("/jsData.html", handleJoyStickData);


  server.begin(); //start the server object

  Serial.println("\nServer Setup Complete.\n");
}
