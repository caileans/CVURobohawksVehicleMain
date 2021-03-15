R"====(

<html>

<head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, user-scalable=no, minimum-scale=1.0, maximum-scale=1.0" />

    <style>
        body {
            overflow: hidden;
            padding: 0;
            margin: 0;
            background-color: #BBB;
            font-size: 14pt;
        }

        #title {
            position: absolute;
            /* top: 8px; */
            width: 100%;
            /* padding-top: 10px; */
            text-align: center;
            font-size: 18pt;
        }

        #info {
            position: absolute;
            top: 70px;
            width: 100%;
            /* padding: 5px; */
            text-align: center;
        }

        #joy-container {
            position: absolute;
            width: 100%;
            top: 150px;
            height: calc(100vh - 200px);
            overflow: hidden;
            padding: 0;
            margin: 0;
            -webkit-user-select: none;
            -moz-user-select: none;
            /* border: solid 2px red; */
        }

        #wheel-adjust {
            position: absolute;
            bottom: 0px;
            width: 100%;
            padding: 5px;
            text-align: center;
        }

        button#run-autonomous {
            border: solid grey 1px;
            background-color: red;
            cursor: pointer;
            transition: 0.2s ease-in-out;
            font-size: 14pt;
        }

        button#run-autonomous:hover {
            box-shadow: 0 0px 8px 0 rgba(0, 0, 0, 0.2);
        }

        span#autonomous-progress {
            color: grey;
            font-weight: bold;
        }
    </style>

</head>

<body>
    <h3 id="title">ESP8266 Vehicle Control</h3>
    <div id="joy-container"></div>

    <div id="info">
        <span id="button">
            <button id="run-autonomous" onclick="runAutonomous()">Run autonomous</button>
            <span id="autonomous-progress" style="display:none">Running autonomous...</span>
        </span>
        </br>
        <span id="touch-message">or Touch the screen to move</span>
        <br />
        <span id="result"></span>

    </div>

    <div id="wheel-adjust">
        <a href="./CVURobohawksVehicleWheelTuneHTML.html" id="wheel-adjust-link">Adjust wheel error</a>
    </div>
    <script src="./CVURobohawksVehicleJoyStick.js"></script>
    <script>
        console.log("touchscreen is", VirtualJoystick.touchScreenAvailable() ? "available" : "not available");

        const messageA = 'or Touch the screen to move';
        const messageB = 'Touch screen disabled';

        var joystick = new VirtualJoystick({
            container: document.getElementById('joy-container'),
            mouseSupport: true,
            limitStickTravel: true,
            stationaryBase: true,
            centerBase: true,
            sprungStick: false,
            baseOutlineColor: 'black',
            baseCenterColor: 'black',
            stickColor: "rgba(255, 0, 0, 0.75)",

        });
        joystick.addEventListener('touchStart', function () {
            console.log('down')
        })
        joystick.addEventListener('touchEnd', function () {
            console.log('up')
        })
        var prevX = 0;
        var prevY = 0;
        var newX = 0;
        var newY = 0;

        setInterval(function () {
            var outputEl = document.getElementById('result');
            newX = Math.round(joystick.deltaX());
            newY = Math.round(joystick.deltaY()) * -1;
            outputEl.innerHTML = '<b>Position:</b> '
                + ' X:' + newX
                + ' Y:' + newY;
            if (newX != prevX || newY != prevY) {
                var xhr = new XMLHttpRequest();
                xhr.open('PUT', "./jsData.html?x=" + newX + "&y=" + newY)
                xhr.send();
            }
            prevX = newX;
            prevY = newY;
        }, 1 / 10 * 1000);

        function setDisplay(id, display) {
            document.getElementById(id).style.display = display;
        }

        function runAutonomous() {
            setDisplay('run-autonomous', 'none');
            setDisplay('autonomous-progress', 'inline');

            document.getElementById('joy-container').style.zIndex = -100;
            document.getElementById('touch-message').innerHTML = messageB;

            var xhr = new XMLHttpRequest();

            xhr.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    setDisplay('run-autonomous', 'inline');
                    setDisplay('autonomous-progress', 'none');

                    document.getElementById('touch-message').innerHTML = messageA;
                    document.getElementById('joy-container').style.zIndex = '';
                }
            }
            xhr.open('GET', '/runAutonomous');
            xhr.send();
        }
    </script>
</body>

</html>

)===="