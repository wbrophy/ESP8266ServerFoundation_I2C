/* (Some) credits - those I missed I apologize
 * https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system
 * https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer
 * https://links2004.github.io/Arduino/d4/dd2/class_esp_class.html
 * https://github.com/esp8266/Arduino/tree/master/doc/esp8266wifi
 * https://github.com/esp8266/Arduino/blob/master/doc/esp8266wifi/generic-class.rst#mode
 * https://codebender.cc/library/Adafruit_Sensor#Adafruit_Sensor.h
 * https://github.com/adafruit/Adafruit_MPU6050
 * https://www.arduino.cc/reference/en/libraries/websockets/
 * https://github.com/gilmaimon/ArduinoWebsockets
 * https://protosupplies.com/product/mpu-6050-gy-521-3-axis-accel-gryo-sensor-module/
 * http://playground.arduino.cc/Main/MPU-6050
 * https://lastminuteengineers.com/mpu6050-accel-gyro-arduino-tutorial/
 * https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050
 * http://www-robotics.cs.umass.edu/~grupen/503/Projects/ArduinoMPU6050-Tutorial.pdf
 * https://github.com/arduino-libraries/Arduino_JSON/tree/master
 * https://www.youtube.com/watch?v=fREqfdCphRA
 * https://gitlab.com/MrDIYca/code-samples/-/blob/master/mrdiy_websocket_project_example.ino
 * https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system
 * https://www.arduino.cc/reference/en/language/functions/communication/wire/
 * https://github.com/arduino/reference-en/tree/master
 * https://github.com/esp8266/Arduino/tree/master/cores/esp8266
 * https://github.com/esp8266/Arduino/blob/master/libraries/DNSServer/examples/CaptivePortal/CaptivePortal.ino
 * https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer
 * https://www.w3schools.com/js/js_numbers.asp ****JavaScript Numbers are Always 64-bit Floating Point**** 
 * https://www.w3schools.com/js/js_bitwise.asp **** (WORD << 48); (WORD >> 48); to get signed WORD. 
 * 
*/
#include <Arduino_JSON.h>
#include <EEPROM.h>
#include <FS.h> // SPIFFS - included in <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h> // included in <ESP8266WebServer.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "DefaultPages.h"
#include <Wire.h> /*MODIFICATION*/

// Global Variable(s) / Object(s)
DNSServer dnsServer;
ESP8266WebServer server(80); //**** make the port configurable
WebSocketsServer webSocket = WebSocketsServer(81); //**** make the port configurable
JSONVar objI2CBus; /*MODIFICATION*/

void setup(void)
{
  JSONVar objWiFiConfiguration;
  
  // Start Serial Communication interface
  Serial.begin(115200);
  delay(1000);
  Serial.print(strBanner);
  
  Serial.println("\nBegin Setup......");

  ///////////////////////////////////////////////////////
  // Initialize I2C data devices - Start /*MODIFICATION*/
  ///////////////////////////////////////////////////////
  Wire.begin();
  Serial.println("Scan I2C Bus.");
  Serial.println(ScanI2C());
  JSONVar objScanI2C = JSON.parse(ScanI2C());
  objI2CBus["i2cbus"] = objScanI2C;
  for (int i = 0; i < objI2CBus["i2cbus"]["device"].length(); i++)
    {
      JSONVar objFile = JSON.parse(readFile("/i2c/" + String(objI2CBus["i2cbus"]["device"][i]["address"]) + ".cfg"));
      if (!objFile["exists"]) continue;
      JSONVar objDevice = JSON.parse(String(objFile["content"]));
      objI2CBus["i2cbus"]["device"][i] = objDevice;
    }
  Serial.println(objI2CBus);

  for (int i = 0; i < objI2CBus["i2cbus"]["device"].length(); i++)
    {
      if (!(bool)objI2CBus["i2cbus"]["device"][i]["enable"]) continue;
      Serial.println("Initializing device " + String(objI2CBus["i2cbus"]["device"][i]["address"]));
      int x_DeviceId = int(strtoul(objI2CBus["i2cbus"]["device"][i]["address"], null, 0));
      for (int j = 0; j < objI2CBus["i2cbus"]["device"][i]["initialize"].length(); j++)
        {
          int x_Register = int(strtoul(objI2CBus["i2cbus"]["device"][i]["initialize"][j]["register"], null, 0));
          int x_Value = int(objI2CBus["i2cbus"]["device"][i]["initialize"][j]["value"]);
          objI2CBus["i2cbus"]["device"][i]["initialize"][j]["result"] = I2CwriteRegister(x_DeviceId, x_Register, x_Value, true);
          delay(int(objI2CBus["i2cbus"]["device"][i]["initialize"][j]["sleep"]) > 0 ? int(objI2CBus["i2cbus"]["device"][i]["initialize"][j]["sleep"]) : 0);
          Serial.println(JSON.stringify(objI2CBus["i2cbus"]["device"][i]["initialize"][j]));
        }
    }
  ///////////////////////////////////////////////////////
  // Initialize I2C data devices - End   /*MODIFICATION*/
  ///////////////////////////////////////////////////////

  Serial.println("\nExample wifi.cfg:");
  Serial.println(strWiFiCfgExample);
  Serial.println("\nLoad wifi.cfg.");
  Serial.println(readFile("wifi.cfg"));
  objWiFiConfiguration = JSON.parse(String((JSON.parse(readFile("wifi.cfg")))["content"]));
  Serial.println(objWiFiConfiguration);

  Serial.println("\nSet WiFi mode and disconnect any existing connections.");
  WiFi.mode(WIFI_AP_STA); /* WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3 */
  WiFi.disconnect();
  delay(1000);

  Serial.println(((bool)objWiFiConfiguration.hasOwnProperty("enableStation") == false || (bool)objWiFiConfiguration["enableStation"] == true) ? "WiFi Station Mode enabled." : "WiFi Station Mode disabled.");
  if ((bool)objWiFiConfiguration.hasOwnProperty("enableStation") == false || (bool)objWiFiConfiguration["enableStation"] == true )
    {
      if (objWiFiConfiguration["WiFiConnection"].length() > 0)
        {
          int x_WiFiConnection = 0;
          while (x_WiFiConnection < objWiFiConfiguration["WiFiConnection"].length())
            {
              if ((bool)objWiFiConfiguration["WiFiConnection"][x_WiFiConnection].hasOwnProperty("connect") == false || (bool)objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["connect"] == true)
                {
                  Serial.println(WNconnect
                    (
                      (objWiFiConfiguration["WiFiConnection"][x_WiFiConnection].hasOwnProperty("hostname") && String(objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["hostname"]) != "") ? String(objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["hostname"]) : DefaultID(),
                      String(objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["ssid"]),
                      objWiFiConfiguration["WiFiConnection"][x_WiFiConnection].hasOwnProperty("password") ? String(objWiFiConfiguration["WiFiConnection"][x_WiFiConnection]["password"]) : ""
                    ));
                  if (WiFi.isConnected()) { break; }
                }
              x_WiFiConnection++;
            }
            Serial.println(WiFi.isConnected() ? "WiFi Station connected." : "WiFi Station NOT connected.");
        }
      else { Serial.println("No WiFiConnection defined"); }
    }

  Serial.println(((bool)objWiFiConfiguration.hasOwnProperty("enableAP") == false || (bool)objWiFiConfiguration["enableAP"] == true) ? "WiFi AP Mode enabled." : "WiFi AP Mode disabled.");
  if ((bool)objWiFiConfiguration.hasOwnProperty("enableAP") == false || (bool)objWiFiConfiguration["enableAP"] == true )
    {
      Serial.println(APconnect
        (
          (objWiFiConfiguration["softAP"].hasOwnProperty("ssid") == true) && (String(objWiFiConfiguration["softAP"]["ssid"]) != "") ? String(objWiFiConfiguration["softAP"]["ssid"]) : DefaultID(),
          (objWiFiConfiguration["softAP"].hasOwnProperty("password") == true) && (String(objWiFiConfiguration["softAP"]["password"]) != "") ? String(objWiFiConfiguration["softAP"]["password"]) : "",
          (objWiFiConfiguration["softAP"].hasOwnProperty("hidden") == true) && ((bool)objWiFiConfiguration["softAP"]["hidden"] == true) ? true : false,
          (objWiFiConfiguration["softAP"].hasOwnProperty("maxconnection") == true) && ((int)objWiFiConfiguration["softAP"]["maxconnection"] >= 1) && ((int)objWiFiConfiguration["softAP"]["maxconnection"] <= 8) ? (int)objWiFiConfiguration["softAP"]["maxconnection"] : 4
        ));
    }

  Serial.println("\nStart DNS Server for SoftAP Captive Portal.");
  Serial.println(dnsServer.start(53, "*", WiFi.softAPIP()) ? "DNS Server Started." : "DNS Server NOT Started.");

  Serial.println("\nBegin webSocket server.");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("\nMAP HTTP Requests to handlers.");
  server.onNotFound(handleNotFound);
  server.on("/", handleRoot);
  server.on("/api/scannetworks", handleScannetworks);
  server.on("/api/scani2cbus", handleScani2cbus); /*MODIFICATION*/
  server.on("/api/i2creadregister", handleI2CreadRegister); /*MODIFICATION*/
  server.on("/api/readeeprom", handleReadEEPROM);
  server.on("/api/formatspiffs", handleFormatSPIFFS);
  server.on("/api/listspiffs", handleListSPIFFS);
  server.on("/applications/editspiffsfile", handleEditSPIFFSfile);
  server.on("/api/savespiffsfile", handleSaveSPIFFSfile);
  server.on("/api/readspiffsfile", handleReadSPIFFSfile);
  server.on("/api/deletespiffsfile", handleDeleteSPIFFSfile);
  server.on("/api/getdata", handleGetData);
  server.on("/api/getnetwork", handleGetNetwork);
  server.on("/applications/websocket", handleWebSocket);
  server.on("/api/restartesp", handleRestart);

  Serial.println("\nBegin HTTP server.");
  server.begin();
  Serial.println("HTTP server started.");
  Serial.println("\nSetup Complete.");
}

void loop(void)
{
  // Get data from data provider
  String sData = getData();

  // Process Serial Input
  if (Serial.available())
    {
      String strSerialInput = Serial.readString();
      strSerialInput.trim();
      String strCommand = strSerialInput.substring(0,strSerialInput.indexOf(" "));
      strCommand.toLowerCase();
      String strParameters = strSerialInput.substring(strCommand.length()+1);
      strParameters.trim();
      
      Serial.println("Serial Input-> " + strSerialInput);
      if (strCommand == "restart")
        { ESP.restart(); }
      
      if (strCommand == "delete")
        { Serial.println(deleteFile(strParameters)); }

      if (strCommand == "write")
        { Serial.println(writeFile(strParameters)); }
    }
  
  // Process DNS Requests
  dnsServer.processNextRequest();
  
  // Process webSocket
  webSocket.loop();
  webSocket.broadcastTXT(sData);

  // Process webserver
  server.handleClient();
}

/////////////////////////////////////////////////
void handleRoot() 
  { server.send(200, "text/html", strRootHTML); }
/////////////////////////////////////////////////

//////////////////////////////////////////////////////
void handleWebSocket() 
  { server.send(200, "text/html", strWebSocketHTML); }
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
void handleGetData()
  { server.send(200, "application/json", String(getData())); }
//////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
void handleListSPIFFS()
  { server.send(200, "application/json", String(ListSPIFFS("/"))); }
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
void handleReadEEPROM() 
  { server.send(200, "application/json", String(ReadEEPROM())); }
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
void handleGetNetwork() 
  { server.send(200, "application/json", String(GetNetwork())); }
/////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////
void handleRestart() 
  { 
    server.send(200, "text/html", strRestartESP);
    delay(1000);
    ESP.restart();
  }
/////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
void handleFormatSPIFFS() 
  {
    String response = strHttpResponseHead;
    SPIFFS.begin();
    if (SPIFFS.format()) { response += "SPIFFS Format Complete."; }
    else { response += "SPIFFS Format Failed."; }
    SPIFFS.end();
    response += "</body></html>";
    server.send(200, "text/html", response);
  }
////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handleEditSPIFFSfile()
  {
    String response = strEditSPIFFSfile;
    server.send(200, "text/html", response);
  }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////
void handleReadSPIFFSfile()
  {
    String x_file = "";
    if (server.args() > 0 ) 
      {
        if (server.arg("file") != "")
          { x_file = readFile(server.arg("file")); }
      }
    server.send(200, "application/json", x_file);
  }
/////////////////////////////////////////////////////

///////////////////////////////////////////////////////
void handleDeleteSPIFFSfile()
  {
    String x_file = "";
    if (server.args() > 0 ) 
      {
        if (server.arg("file") != "")
          { x_file = deleteFile(server.arg("file")); }
      }
    server.send(200, "application/json", x_file);
  }
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
void handleSaveSPIFFSfile()
  {
    JSONVar x_file = JSON.parse(server.arg("plain"));
    server.send(200, "application/json", writeFile(server.arg("plain")));
    
    /*
    if (server.arg("plain") != "") 
      {
        Serial.println(server.arg("plain"));
        JSONVar x_file = JSON.parse(server.arg("plain"));
        String strPath = x_file["fileName"];
        if (!strPath.startsWith("/")) { strPath = "/" + strPath; }
        SPIFFS.begin();
        File x_f = SPIFFS.open(strPath, "w");
        x_f.print(String(x_file["content"]));
        x_f.close();
        SPIFFS.end();
        server.send(200, "application/json", server.arg("plain"));
      }
    */
  }
///////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// /*MODIFICATION*/
/////////////////////////////////////////////////////////
void handleScani2cbus() 
  {
    server.send(200, "application/json", ScanI2C());
  }
/////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
void handleScannetworks() 
  { server.send(200, "application/json", ScanNetworks()); }
///////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// /*MODIFICATION*/
/////////////////////////////////////////////////////////
void handleI2CreadRegister() 
  {
    JSONVar objRead;
    int x_DeviceId = 0;
    int x_Register = 0;
    int x_Bytes = 0;
    objRead["DeviceId"] = server.arg("i2cdevice");
    objRead["Register"] = server.arg("i2cregister");
    objRead["Bytes"] = server.arg("i2cbytes");

    if (server.arg("i2cdevice") != "" && server.arg("i2cregister") != "" && server.arg("i2cbytes") != "")
      {
        x_DeviceId = int(strtoul(objRead["DeviceId"], null, 0));
        x_Register = int(strtoul(objRead["Register"], null, 0));
        x_Bytes = int(strtoul(objRead["Bytes"], null, 0));
      }
    server.send(200, "application/json", I2CreadRegister(x_DeviceId, x_Register, x_Bytes));
  }
/////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handleNotFound() 
  {
    String strPath = server.uri();
    JSONVar objFile = JSON.parse(readFile(String(server.uri())));
    // ****** hack DIY option only serves .html from SPIFFS/filesystem - Revisit *******
    if (objFile["exists"] && ( strPath.endsWith(".css") || strPath.endsWith(".html") || strPath.endsWith(".js") ))
      {
        String strEncoding = "text/html";
        if (strPath.endsWith(".css")) strEncoding = "text/css";
        if (strPath.endsWith(".js")) strEncoding = "text/javascript";
        String strFileContent = String(objFile["content"]);
        server.send(200, strEncoding, strFileContent);
      }
    else
      {
        server.send(200, "text/html", strRedirectToRootHTML);
      }
  }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
String deleteFile(String strPath)
  {
    JSONVar x_file;
    if (!strPath.startsWith("/")) strPath = "/" + strPath;
    SPIFFS.begin();
    if (SPIFFS.exists(strPath)) { SPIFFS.remove(strPath); }
    x_file["status"] = SPIFFS.exists(strPath) ? false:true;
    SPIFFS.end();
    return JSON.stringify(x_file);
  }
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
String writeFile(String strFile)
  {
    JSONVar x_file = JSON.parse(strFile);
    String strPath = x_file["fileName"];
    if (!strPath.startsWith("/")) { strPath = "/" + strPath; }
    SPIFFS.begin();
    File x_f = SPIFFS.open(strPath, "w");
    x_f.print(String(x_file["content"]));
    x_file["status"] = SPIFFS.exists(strPath) ? true:false;
    x_f.close();
    SPIFFS.end();
    return JSON.stringify(x_file);
  }
////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
String GetNetwork()
  {
    JSONVar x_return;
    x_return["ConnectionResult"] = WiFi.waitForConnectResult();
    x_return["network"] = WiFi.SSID();
    x_return["localIP"] = WiFi.localIP().toString();
    x_return["macAddress"] = WiFi.macAddress();
    x_return["hostname"] = WiFi.hostname();
    x_return["gateway"] = WiFi.gatewayIP().toString();
    x_return["dns"] = WiFi.dnsIP().toString();
    return JSON.stringify(x_return);
  }
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
String getData() /*MODIFICATION*/
  {
    /*MODIFICATION*/
    //JSONVar x_data;

    for (int i = 0; i < objI2CBus["i2cbus"]["device"].length(); i++)
      {
        if (!(bool)objI2CBus["i2cbus"]["device"][i]["enable"]) continue;
        int x_DeviceId = int(strtoul(objI2CBus["i2cbus"]["device"][i]["address"], null, 0));
        int x_Register = int(strtoul(objI2CBus["i2cbus"]["device"][i]["getdata"][0]["register"], null, 0));
        int x_Bytes = int(objI2CBus["i2cbus"]["device"][i]["getdata"][0]["bytes"]);
        JSONVar x_Buffer = JSON.parse(I2CreadRegister(x_DeviceId, x_Register, x_Bytes));
        
        for (int j = 0; j < x_Buffer.length(); j++)
          {
            byte x_Byte = x_Buffer[j];
            objI2CBus["i2cbus"]["device"][i]["getdata"][0]["data"][j] = x_Byte;
          }
      }
    //return JSON.stringify(x_data);
    return JSON.stringify(objI2CBus);
  }
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
String readFile(String strPath)
  {
    JSONVar x_file;
    String x_content = "";
    if (!strPath.startsWith("/")) strPath = "/" + strPath;
    SPIFFS.begin();
    x_file["path"] = strPath;
    if (SPIFFS.exists(strPath))
      {
        File x_f = SPIFFS.open(strPath, "r");
        x_file["fullName"] = String(x_f.fullName());
        x_file["size"] = String(x_f.size());
        while(x_f.available()) 
          { x_content += String(x_f.readStringUntil('\n')) + String("\n"); }
        x_file["exists"] = true;
        x_file["content"] = x_content;
        x_f.close();
      }
    else { x_file["exists"] = false; }
    SPIFFS.end();
    x_content = JSON.stringify(x_file);
    //Serial.println(x_content);
    x_file = null;
    return x_content;
  }
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////
String ReadEEPROM()
  {
    JSONVar x_return;
    String x_str = "";
    EEPROM.begin(512);
    delay(10);
    for (int x_i = 0; x_i < 512; x_i++)
      { x_str += String(char(EEPROM.read(x_i))); }
    EEPROM.end();
    x_return["EEPROM"] = x_str;
    return JSON.stringify(x_return);
  }
///////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
String WNconnect(String x_hostname, String x_ssid, String x_pwd)
  {
    JSONVar x_return;
    if (x_hostname != "") { WiFi.hostname(x_hostname); }
    WiFi.begin(x_ssid, x_pwd);
    x_return["ConnectionResult"] = WiFi.waitForConnectResult();
    x_return["network"] = WiFi.SSID();
    x_return["localIP"] = WiFi.localIP().toString();
    x_return["macAddress"] = WiFi.macAddress();
    x_return["hostname"] = WiFi.hostname();
    x_return["gateway"] = WiFi.gatewayIP().toString();
    x_return["dns"] = WiFi.dnsIP().toString();
    return JSON.stringify(x_return);
  }
////////////////////////////////////////////////////////////////////

  
////////////////////////////////////////////////////////////
String APconnect(String x_ssid, String x_pwd, bool x_hidden, int x_maxconnection)
  {
    JSONVar x_return;
    x_return["ssid"] = x_ssid;
    x_return["active"] = false;
    x_return["hidden"] = x_hidden;
    x_return["maxconnection"] = x_maxconnection;
    x_return["active"] = WiFi.softAP
      (
        (x_ssid != "") ? x_ssid : DefaultID(),
        (x_pwd != "") ? x_pwd : "",
        1,
        x_hidden,
        (x_maxconnection >= 1 && x_maxconnection <= 8) ? x_maxconnection : 4
      );
    x_return["ip"] = WiFi.softAPIP().toString();
    x_return["mac"] = WiFi.softAPmacAddress();
    return JSON.stringify(x_return);
  }
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
String ListSPIFFS(String x_path)
  {
    int x_count = 0;
    JSONVar x_files;
    x_path = x_path == "" ? "/" : x_path;
    SPIFFS.begin();
    Dir x_dir = SPIFFS.openDir(x_path);
    while (x_dir.next()) 
      {
        JSONVar x_file;
        x_file["fileName"] = String(x_dir.fileName());
        x_file["fileSize"] = String(x_dir.fileSize());
        x_file["fileTime"] = String(x_dir.fileTime());
        x_file["fileCreationTime"] = String(x_dir.fileCreationTime());
        x_files["file"][x_count] = x_file;
        x_count++;
      }
    SPIFFS.end();
    return JSON.stringify(x_files);
  }
///////////////////////////////////////////////////////////////////////


/////////////////////////////////////
String DefaultID()
  {
    String x_mac = WiFi.macAddress();
    x_mac.replace(":", "");
    return "esp" + x_mac;
  }
/////////////////////////////////////


///////////////////////////////////////////////////
String ScanNetworks()
  {
    JSONVar x_networks;
    int x_n = WiFi.scanNetworks();
    x_networks["error"] = "";
    
    if (x_n == 0) { x_networks["error"] = "No Networks Found."; }
    else
    {
      for (int x_i = 0; x_i < x_n; ++x_i)
      {
        JSONVar x_network;
        String strEncType = "";
        switch (WiFi.encryptionType(x_i))
          {
            case ENC_TYPE_WEP:
              strEncType = "WEP";
              break;
            case ENC_TYPE_TKIP:
              strEncType = "WPA";
              break;
            case ENC_TYPE_CCMP:
              strEncType = "WPA2";
              break;
            case ENC_TYPE_NONE:
              strEncType = "None";
              break;
            case ENC_TYPE_AUTO:
              strEncType = "Auto";
              break;
          }
        x_network["ssid"] = String(WiFi.SSID(x_i));
        x_network["rssi"] = String(WiFi.RSSI(x_i));
        x_network["encription"] = strEncType;
        x_networks["network"][x_i] = x_network;
      }
    }
    return JSON.stringify(x_networks);
  }
///////////////////////////////////////////////////

///////////////////////////////////////////////////
// /*MODIFICATION*/
String ScanI2C()
  {
    JSONVar x_i2cDevices;
    byte x_error, x_address;
    int x_devices = 0;
    x_i2cDevices["error"] = false;
    for (x_address = 1; x_address < 127; x_address++ )
      {
        JSONVar x_device;
        String x_deviceHEX;
        Wire.beginTransmission(x_address);
        x_error = Wire.endTransmission();
        if (x_error == 0)
          {
            x_deviceHEX = "0x" + String(x_address<16 ? "0" : "") + String(x_address, HEX);
            x_device["address"] = x_deviceHEX;
            x_device["error"] = false;
            x_i2cDevices["device"][x_devices] = x_device;
            x_devices++;
          }
      }
    return JSON.stringify(x_i2cDevices);
  }
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
// /*MODIFICATION*/
int I2CwriteRegister(int i2c_DeviceId, int i2c_Register, int i2c_Value, bool i2c_EndTransmission)
  {
          Wire.beginTransmission(i2c_DeviceId);
          Wire.write(i2c_Register);
          Wire.write(i2c_Value);
          return Wire.endTransmission(i2c_EndTransmission);
  }
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
// /*MODIFICATION*/
String I2CreadRegister(int I2C_DeviceId, int I2C_Register, int I2C_Bytes)
  {
    JSONVar objI2Cdata;
    
    Wire.beginTransmission(I2C_DeviceId);
    Wire.write(I2C_Register);
    Wire.endTransmission(false);
    Wire.requestFrom(I2C_DeviceId, I2C_Bytes, true);
    
    for (int j = 0; j < I2C_Bytes; j++)
      {
        byte I2C_Byte = Wire.read();
        objI2Cdata[j] = I2C_Byte;
      }
    return JSON.stringify(objI2Cdata);
  }
/////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
  {
    switch (type)
      {
        case WStype_DISCONNECTED:
          Serial.printf("[%u] WebSocket Disconnected!\n", num);
          break;

        case WStype_CONNECTED: 
          {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] WebSocket Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            // send message to client
            webSocket.sendTXT(num, "0");
          }
          break;

        case WStype_TEXT:
          Serial.printf("[%u] get Text: %s\n", num, payload);
          // send message to client
          // webSocket.sendTXT(num, "message here");
          // send data to all connected clients
          // webSocket.broadcastTXT("message here");
          break;
      
        case WStype_BIN:
          Serial.printf("[%u] get binary length: %u\n", num, length);
          hexdump(payload, length);
          // send message to client
          // webSocket.sendBIN(num, payload, length);
          break;
      }
  }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
