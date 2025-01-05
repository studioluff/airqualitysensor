//___________ NETWORKING___________
const char HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Air Quality Sensor</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Nunito:wght@400;600&display=swap');

        body {
            font-family: 'Nunito', sans-serif;
            background-color: #f2f2f2;
            color: #333;
            margin: 0;
            padding: 10px;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            height: 100vh;

        }

        h1 {
            text-align: center;
            margin-bottom: 5px;
        }
        h3{
          margin-bottom: 50px;
          margin-top: 5px;
        }
          

      .parameter-container{
        display:inline;
         padding: 20px;
            background-color: #ffffff;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
      }
        .parameters-boxes {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 30px;
            padding: 20px;
            background-color: #ffffff;
            border-radius: 8px;
            
        }

        .parameter {
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 20px;
            background-color: #ffffff;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
        }

        .parameter-label {
            font-size: 16px;
            font-weight: bold;
            color: #9a9a9a;
            margin-bottom: 10px;
        }

        .parameter-value {
            font-size: 24px;
            font-weight: bold;
            color: #777;
        }
    </style>
</head>
<body>
    <h1>Air Quality Sensor</h1>
   <h3> by <a href='https://www.studioluff.com' target="_blank">studio LUFF</a></h3>
    <div class="parameter-container">
      <div class="parameters-boxes">
          <div class="parameter">
              <div class="parameter-label">PM 1.0</div>
              <div class="parameter-value" id="pm1">--</div>
          </div>
          <div class="parameter">
              <div class="parameter-label">PM 2.5</div>
              <div class="parameter-value" id="pm2.5">--</div>
          </div>
          <div class="parameter">
              <div class="parameter-label">PM 10.0</div>
              <div class="parameter-value" id="pm10">--</div>
          </div>
        </div>
        <p style="font-size: 0.6em;">Last updated: <span id="updated">--</span></p>
    </div><br>
    <p>This is a minimal web interface for your Air Quality Sensor. To get the full web interface follow <a href='https://www.studioluff.com/setup' target="_blank">these instructions</a>.</p>


    <script>
        var ws = new WebSocket('ws://' + window.location.host + '/ws');
        ws.onopen = function(event) { console.log('WebSocket connected.'); };
        ws.onmessage = function(event) {
         
            var data = JSON.parse(event.data);
             console.log(data);
             var data;
            try {
                data = JSON.parse(event.data);
            } catch (e) {
                console.log("received", event.data);
                console.warn(e);
                return;
            }
            if (!data.hasOwnProperty('m')) {
                console.warn("JSON format not recognized");
                return;
            }
            if (data.m === 'currentData') {
                //UPDATE INTERFACE
                
                window.ppm1_0 = data.v[0];
                window.ppm2_5 = data.v[1];
                window.ppm10 = data.v[2];
                const utcDate = new Date(0); // The 0 there is the key, which sets the date to the epoch
                utcDate.setUTCSeconds(data.v[5]);
                // Convert the UTC date to the local time zone
                let keyDateTmp = new Date(utcDate.toLocaleString());
                window.timestamp = keyDateTmp;

                document.getElementById('pm1').innerHTML = window.ppm1_0;
                document.getElementById('pm2.5').innerHTML = window.ppm2_5;
                document.getElementById('pm10').innerHTML = window.ppm10;
                document.getElementById('updated').innerHTML = window.timestamp;
                
            }
            
        };
    </script>
</body>
</html>
)rawliteral";


void initWiFi() {

  if (ssid.isEmpty()) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

  } else {

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);


    localname.trim();

    WiFi.setHostname(localname.c_str());  //define hostname

    WiFi.begin(ssid.c_str(), password.c_str());

    if (WiFi.status() != WL_CONNECTED) {

      serialPrintln("Attempt to connect to wifi failed \t trying again later.");

    } else {
      initLocalNet();

      if (checkIfConnectedToTheInternet()) {
        initInternetServices();
      }

      wasPrevConnectedToLocalNet = true;
    }
  }
}
bool checkIfConnectedToTheInternet() {
  int pingResult;

  bool success = Ping.ping("www.pierdr.com", 1);

  if (!success) {
    serialPrintln("Unable to reach the internet");
    wasPrevConnectedToInternet = false;
    return false;
  } else {
    wasPrevConnectedToInternet = true;
    return true;
  }
}
void initLocalNet() {

  serialPrint("Connected to WiFi - IP address:");
  serialPrintln(WiFi.localIP());
  if (localname == "") {
    localname = "airqualitysensor";
  }

  if (!MDNS.begin(localname.c_str())) {
    serialPrintln("Error setting up MDNS responder!");
  }

  initWebServer();

  reconnecting_wifi_interval = 5000;
}
void initInternetServices() {
  initNTPClient();
}

void initWebServer() {
  server.end();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    /** CHECK IF THERE ARE STATIC FILES **/
    // Check if "index.html" file exists inside "template" folder
    if (SD.exists("/webinterface/index.html")) {
      //Stream index.html
      request->send(SD, "/webinterface/index.html", "text/html");
    } else {
      serialPrintln("template not found - writing standard page");
      /** OTHERWISE SERVE THE STANDARD PAGE **/
      request->send(200, "text/html", HTML);
    }
  });
  server.onNotFound([](AsyncWebServerRequest *request) {
    path = "/webinterface" + request->url();
    serialPrint("serving file:");
    serialPrintln(path);
    if (SD.exists(path)) {
      workingFile = SD.open(path);
      if (workingFile) {
        request->send(SD, path, getContentType(path));
        workingFile.close();
        return;
      }
      else{
        serialPrintln("some rror occured trying to open a file");
      }
    }
    else{
      serialPrintln("some error occured whil trying to open the path");
    }
    request->send(404, "text/plain", "Not found");
  });

  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      serialPrintln("WebSocket client connected");

      //SEND TO THE CLIENT
      sendDataToClient();
      sendHistoricalToClients = true;
      indexSendHistoricalDataToClient = 0;


    } else if (type == WS_EVT_DISCONNECT) {
      serialPrintln("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
      serialPrint("WebSocket client message received: ");
      Serial.write(data, len);
      serialPrintln("");
      // client->text("Echo: " + String((char *)data));
      // Parse the JSON message
      StaticJsonDocument<64> doc;
      deserializeJson(doc, data, len);

      if (doc.containsKey("m") && doc["m"] == "playAnimation" && doc.containsKey("v")) {
        String v = doc["v"];
        if (v == "start") {
          initLEDAnimation();
        }
      } else if (doc.containsKey("m") && doc["m"] == "settingsUpdate" && doc.containsKey("v")) {

        String settingsStr = doc["v"];

        int comma1 = settingsStr.indexOf(",");
        custom_sleep_time = settingsStr.substring(0, comma1).toInt();  // milliseconds

        int comma2 = settingsStr.indexOf(",", comma1 + 1);
        custom_brightness_factor = max(0.1f, settingsStr.substring(comma1 + 1, comma2).toFloat());

        int comma3 = settingsStr.indexOf(",", comma2 + 1);
        custom_time_on = settingsStr.substring(comma2 + 1, comma3).toInt();  // milliseconds

        int comma4 = settingsStr.indexOf(",", comma3 + 1);
        custom_threshold_indicator = settingsStr.substring(comma3 + 1, comma4).toInt();

        serialPrint("custom_sleep_time: ");
        serialPrintln((custom_sleep_time));

        serialPrint("custom_brightness_factor: ");
        serialPrintln((custom_brightness_factor));

        serialPrint("custom_time_on: ");
        serialPrintln((custom_time_on));

        serialPrint("custom_threshold_indicator: ");
        serialPrintln((custom_threshold_indicator));

        saveSettings();

        sendSettings();

      } else if (doc.containsKey("m") && doc["m"] == "resetDefaults") {

        custom_sleep_time = 360000;
        custom_brightness_factor = 1.0;
        custom_autobrightness = 1;
        custom_does_use_NTP_client = 1;
        custom_time_on = 3000;
        custom_threshold_indicator = 50;

        sendSettings();

      } else if (doc.containsKey("m") && doc["m"] == "querySettings" && doc.containsKey("v")) {
        sendSettings();
      } else if (doc.containsKey("m") && doc["m"] == "overrideValue" && doc.containsKey("v")) {
        currentAQIvalue = doc["v"].as<int>();
        updateLightIndicatorHeight();
      } else if (doc.containsKey("m") && doc["m"] == "debugSerial" && doc.containsKey("v")) {
        int debugSerial = doc["v"].as<int>();
        if (debugSerial == 1) {
          enableSerial();
        } else if (debugSerial == 0) {
          disableSerial();
        }
      }
    }
  });
  server.addHandler(&ws);

  server.begin();
  serialPrintln("Web server started");
}

void sendDataToClient() {

  if (didReadForFirstTime && (lastTimeSensedTimeStampt != 0)) {

    String message = "{\"m\":\"currentData\",\"v\":[" + String(pmsValues[0]) + "," + String(pmsValues[1]) + "," + String(pmsValues[2]) + "," + String(ldr1Value) + "," + String(ldr2Value) + "," + lastTimeSensedTimeStampt + "]}";

    // Send the message to all connected WebSocket clients
    ws.textAll(message);
  } else {
    String message = "{\"m\":\"message\",\"v\":\"waiting for data\"}";

    // Send the message to all connected WebSocket clients
    ws.textAll(message);
  }
}
void sendSettings() {

  char payload[256];
  snprintf(payload, sizeof(payload), "{\"m\":\"settings\",\"v\":[%lu,%.2f,%d,%d]}",
           custom_sleep_time,
           custom_brightness_factor,
           custom_time_on,
           custom_threshold_indicator);

  String message = payload;
  ws.textAll(message);
}
void sendTouch() {
    // First validate WebSocket object - use try/catch to prevent any crash
    try {
        // Use a larger buffer to be safe
        const size_t BUFFER_SIZE = 512;
        char payload[BUFFER_SIZE];
        
        // Get epoch time and validate
        uint32_t epoch = rtc.getEpoch();
        if (epoch == 0) {
            return;
        }
        
        // Use snprintf return value to check for truncation
        int written = snprintf(payload, BUFFER_SIZE, 
                             "{\"m\":\"debugTouchEvent\",\"v\":%lu}", 
                             (unsigned long)epoch);
        
        // Check if message was truncated or there was an error
        if (written < 0 || written >= BUFFER_SIZE) {
            return;
        }
        
        // Create message only if buffer operations succeeded
        String message = payload;
        
        // Verify message conversion and WebSocket before sending
        if (message.length() > 0) {
            // Access ws through try/catch to prevent crash if it's null or invalid
            ws.textAll(message);
        }
    } catch (...) {
        
        return;
    }
}
void updateWifi() {
  if ((WiFi.status() != WL_CONNECTED) && (millis() - lastTimeCheckedForWifiConnection >= reconnecting_wifi_interval)) {  //CHECK EVERY MINUTE
    //if an animation is running this might cause some delays, so doing so it's going to check for wifi when animations are not running.
    if (animState != ANIM_NONE) {
      lastTimeCheckedForWifiConnection = millis();
      reconnecting_wifi_interval = 5000;
      return;
    }
    lastTimeCheckedForWifiConnection = millis();
    reconnecting_wifi_interval = min(60000, reconnecting_wifi_interval * 2);
    if (ssid.isEmpty()) {
      openWifiCredentials();
    }
    if (ssid.isEmpty()) {
      return;
    }
    if (!wasPrevConnectedToLocalNet) {
      initWiFi();
    }
    wasPrevConnectedToLocalNet = false;

    WiFi.disconnect();
    WiFi.reconnect();

  } else if (WiFi.status() == WL_CONNECTED && !wasPrevConnectedToLocalNet) {

    initLocalNet();


    if (checkIfConnectedToTheInternet()) {
      initInternetServices();
    }

    wasPrevConnectedToLocalNet = true;
  } else if (WiFi.status() == WL_CONNECTED && !wasPrevConnectedToInternet) {

    if (animState == ANIM_NONE) {
      if (millis() - lastTimeCheckedForInternetConnection >= 10000)  //check every 5 minutes
      {

        serialPrintln("Reconnecting internet services - ntp not working");
        if (checkIfConnectedToTheInternet()) {
          initInternetServices();
        }
        lastTimeCheckedForInternetConnection = millis();
      }
    }
  }
}
void updateServer() {
  if ((WiFi.status() != WL_CONNECTED)) {
    return;
  }
  if (millis() - lastTimeSentToClient >= 100) {
    if (sendHistoricalToClients) {
      
      //get current day
      getDateNow();
      if (working_epoch <= 0) {
        lastTimeSentToClient = millis();
        return;
      }
      if (indexSendHistoricalDataToClient < NUM_OF_DAYS_HISTORICAL) {

        historicalDayToSend = mday - indexSendHistoricalDataToClient;
        historicalYearToSend = year;
        historicalMonthToSend = mon;

        if (historicalDayToSend < 1) {
          historicalMonthToSend = historicalMonthToSend - 1;

          if (historicalMonthToSend < 1) {
            historicalMonthToSend = 12;
            historicalYearToSend = year - 1;
          }

          historicalDayToSend = (isLeapYear(historicalYearToSend)) ? daysInMonthLeapYear[historicalMonthToSend - 1] - abs(mday - indexSendHistoricalDataToClient) : daysInMonth[historicalMonthToSend - 1] - abs(mday - indexSendHistoricalDataToClient);
        }
        
        sendHistoricalDataOfDay(historicalYearToSend, historicalMonthToSend, historicalDayToSend);

        //increment index
        indexSendHistoricalDataToClient++;
      } else {
        sendHistoricalToClients = false;
        indexSendHistoricalDataToClient = 0;
      }
    }
    lastTimeSentToClient = millis();
  }
}



void sendHistoricalDataOfDay(int year, int month, int day) {
  // Construct the file path for the given date
  String filePath = "/data/" + String(year) + "/" + String(month) + "/" + String(day) + ".csv";
  // Check if the file exists
  if (!SD.exists(filePath)) {
    return;
  }

  // Open the file
  File file = SD.open(filePath);
  if (!file) {
    serialPrintln("Failed to open file");
    return;
  }

  // Read the file contents
  String fileContent;
  while (file.available()) {
    fileContent += file.readStringUntil('\n');
  }
  file.close();
  char targetChar = ',';


  if (fileContent.charAt(fileContent.length() - 2) == targetChar) {
    fileContent.remove(fileContent.length() - 2);
  }
  // Construct the message with the historical data
  String message = "{\"m\":\"historical\",\"v\":[" + fileContent + "]}";

  // Send the message to all connected WebSocket clients
  ws.textAll(message);
}
//This functions returns a String of content type
String getContentType(String filename) {
  if (filename.endsWith(".htm")) {  //check if the string filename ends with ".htm"
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  }
  return "text/plain";
}
