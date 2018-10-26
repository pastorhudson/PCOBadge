/*  PCOBadge is a fun project by Ron Hudson.
    PCOBadge pulls an individual's checkin info from the Planning Center Check-ins api and displays it on an e-paper display.
    It lives at https://github.com/pastorhudson/pcobadge
    It is Liscenced under the:

  MIT License

  Copyright (c) 2018 Ron Hudson

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

/* Libraries You Need */

/* e-paper display lib Version 3.x
   https://github.com/ZinggJM/GxEPD/releases */
#include <GxEPD.h>
#include <GxGDEH029A1/GxGDEH029A1.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

/* include any other fonts you want to use https://github.com/adafruit/Adafruit-GFX-Library */
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

/* lable.h is a byte encoded version of label.bmp Label Image */
#include "label.h"
#include "labelMenuBeta.h"

/* WiFi  libs*/
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>

/* Misc Libs */
#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include <base64.h>

/* Always include the update server, or else you won't be able to do OTA updates! */
/**/const int port = 8888;
/**/ESP8266WebServer httpServer(port);
/**/ESP8266HTTPUpdateServer httpUpdater;
/*                                                                                */

/* Configure pins for display */
GxIO_Class io(SPI, SS, 0, 2);
GxEPD_Class display(io); // default selection of D4, D2

/* A single byte is used to store the button states for debouncing */
byte buttonState = 0;
byte lastButtonState = 0;   //the previous reading from the input pin
unsigned long lastDebounceTime = 0;  //the last time the output pin was toggled
unsigned long debounceDelay = 80;    //the debounce time

/* Font Styles */
const GFXfont* h1 = &FreeSansBold24pt7b;
const GFXfont* h2 = &FreeSansBold18pt7b;
const GFXfont* h3 = &FreeSansBold12pt7b;
const GFXfont* h4 = &FreeSansBold9pt7b;

/* EDIT THIS! API Keys And URL's */
String userID = "xxxxxx"; // Your User ID Number from check-ins. Use Just the Numerical Portion.

/* Generate your Application ID and Secret personal access token pair at https://api.planningcenteronline.com/personal_access_tokens/new
   Be sure to allow access to Check-ins Version 2018-08-01 */
String applicationID = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxAPPLICATION IDxxxxxxxxxxxxxxxxxxxxxx"; // Application ID
String secret = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxSecretxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // Secret

/* Maybe edit this ¯\_(ツ)_/¯ SHA1 fingerprint of the certificate for api.planningcenteronline.com Expires Nov 24, 2019 */
String fingerPRINT = "95 54 E1 9C 51 F1 9E 0A 2E 2F 41 51 9D A4 E4 83 26 80 71 7C‎";


/* checkinAPIURL is the Check-ins api url using the userID from above.
   b64auth is the Base64 Encoded applicationID:secret
   You shouldn't need to edit these.*/
String apiStatusURL = "http://status.planningcenteronline.com/index.json";
String checkinAPIURL = "https://api.planningcenteronline.com/check_ins/v2/people/" + userID + "/check_ins?per_page=1/";
String b64auth = base64::encode(applicationID + ":" + secret, false); // Base64 Encoded auth string

bool OTA = false; //OTA mode disabled by default

void setup()
{
  display.init();

  pinMode(1, INPUT_PULLUP); //down
  pinMode(3, INPUT_PULLUP); //left
  pinMode(5, INPUT_PULLUP); //center
  pinMode(12, INPUT_PULLUP); //right
  pinMode(10, INPUT_PULLUP); //up
  Serial.begin(115200);
  Serial.println("Connecting To Wifi");
  /* WiFi Manager automatically connects using the saved credentials, if that fails it will go into AP mode */
  WiFiManager wifiManager;
  wifiManager.autoConnect("PCOBadge");

  /* If the center button is pressed on boot, enable OTA upload */
  if (digitalRead(5) == 0) {
    Serial.println("Detected OTA Trigger");
    httpUpdater.setup(&httpServer);
    httpServer.begin();
    menu();
    /* Otherwsie, Display PCO Badge from Last Check-in */
  } else {
    menu();
  }
}

void loop()
{
  httpServer.handleClient();

  byte reading =  (digitalRead(1)  == 0 ? 0 : (1 << 0)) | //down
                  (digitalRead(3)  == 0 ? 0 : (1 << 1)) | //left
                  (digitalRead(5)  == 0 ? 0 : (1 << 2)) | //center
                  (digitalRead(12) == 0 ? 0 : (1 << 3)) | //right
                  (digitalRead(10) == 0 ? 0 : (1 << 4)); //up

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      for (int i = 0; i < 5; i++) {
        if (bitRead(buttonState, i) == 0) {
          switch (i) {
            case 0:
              //do something when the user presses down
              Serial.println("Down button pressed.");
              break;
            case 1:
              //do something when the user presses left
              Serial.println("Left Button Pressed");
              break;
            case 2:
              //do something when the user presses center
              Serial.println("Center button pressed.");
              menu();
              break;
            case 3:
              //do something when the user presses right
              Serial.println("Right button pressed.");
              pcoAPI();
              break;
            case 4:
              //do something when the user presses up
              Serial.println("Up Button Pressed");
              break;
            default:
              break;
          }
        }
      }
    }
  }
  lastButtonState = reading;
}

/* Gets the last checkin info for userID.  */
void pcoAPI()
{
  Serial.println("pcoAPI() called");
  display.setRotation(3); //even = portrait, odd = landscape
  display.fillScreen(GxEPD_WHITE); // Clear Screen
  display.drawBitmap(label, 0, 0, 296, 128, GxEPD_WHITE); // Draw the label.h background image
//  display.update();

  /* Parse JSON from HTTP GET request */
  HTTPClient http;
  http.begin(checkinAPIURL , fingerPRINT);
  http.addHeader("Authorization", "Basic " + b64auth);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  if (httpCode = 200) { //Check for the returning code
    String payload = http.getString();
    DynamicJsonBuffer jsonBuffer; //buffer for PCO Check-in JSON
    JsonObject& root = jsonBuffer.parseObject(payload);
    String first_name = root["data"][0]["attributes"]["first_name"];
    String last_name = root["data"][0]["attributes"]["last_name"];
    String security_code = root["data"][0]["attributes"]["security_code"];
    const char* created_at = root["data"][0]["attributes"]["created_at"];
    /* Name */
    display.setTextColor(GxEPD_BLACK);
    display.setFont(h1);
    display.setCursor(20, 60);
    display.println(first_name);
    display.setFont(h2);
    display.setCursor(20, 90);
    display.println(last_name);
    /* date info */
    display.setFont(h4);
    display.setCursor(20, 120);
    display.println(formatTimeStamp(created_at));
    /* Security Code
      /* #TODO Compile suitible fixed width font to use here. */
    display.setFont(h3);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(263, 25);
    display.println(security_code[0]);
    display.setCursor(263, 47);
    display.println(security_code[1]);
    display.setCursor(263, 69);
    display.println(security_code[2]);
    display.setCursor(263, 91);
    display.println(security_code[0]);

    display.update();
  }
  else {
    display.println("Error on HTTPS request: \n" + String(httpCode));
    display.update();
  }

  http.end();
}

/* Simple demo of printing something on screen. */
void myMSG() {
  Serial.println("myMSG() called");
  display.setRotation(3); //even = portrait, odd = landscape
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(h4);
  display.setCursor(10, 20);
  display.println("IT WORKS WOOHOO!");
  display.update();

}

void menu() {
  Serial.println("menu() called");
  display.setRotation(3); //even = portrait, odd = landscape
  display.fillScreen(GxEPD_WHITE); // Clear Screen
  display.drawBitmap(labelMenuBeta, 0, 0, 296, 128, GxEPD_WHITE); // Draw the statusLabel.h background image
  display.update();
}



void showIP() {
  display.setRotation(3); //even = portrait, odd = landscape
  display.fillScreen(GxEPD_WHITE);
  const GFXfont* f = &FreeSansBold9pt7b ;
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(0, 10);

  String ip = WiFi.localIP().toString();
  String url = WiFi.localIP().toString() + ":" + String(port) + "/update";
  byte charArraySize = url.length() + 1;
  char urlCharArray[charArraySize];
  url.toCharArray(urlCharArray, charArraySize);

  display.println("You are now connected!");
  display.println("");
  display.println("Go to:");
  display.println(urlCharArray);
  display.println("to upload a new sketch.");
  display.update();
}

/* Formats the time_stamp from PCO API
   Expects format const char* buff = "2018-10-25T06:08:25Z"
   Returns a String*/
String formatTimeStamp(const char* buff)
{

  struct tm tm;
  time_t t;

  if (strptime(buff, "%y-%m-%dT%H:%M:%SZ", &tm) == NULL);
  else {
    time_t t = mktime(&tm);
    String(tm.tm_mday) + " " + String(tm.tm_mon);
    String time_stamp = String(dayShortStr(tm.tm_wday)) + " " + String(monthShortStr(tm.tm_mon + 1)) + " " + String(tm.tm_mday) + " " + String(tm.tm_year);
    return time_stamp;
  }
}
