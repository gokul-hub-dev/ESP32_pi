#include <Arduino.h>

/***********SERVER OTA CODE***********/
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

const char* host = "esp32";
const char* ssid = "Test";
const char* password = "876543218";

WebServer server(80);

/*
 * Login page
 */
const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";
 
/*
 * Server Index Page
 */
 
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

/************************************/

TaskHandle_t Task1;
TaskHandle_t Task2;

void SERVER_OTA_CORE( void * pvParameters ){
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  while(1){
    static int count = 0;	
    server.handleClient();
    delay(1);
    count++;
    if(count >= 1000){
       Serial.println("OTA core started.......!!!\n");
       Serial.print("IP address: ");
       Serial.println(WiFi.localIP());
       count=0;
    }
  }
}

/*************************RELAY_CORE1**********************************/
#include <EEPROM.h>
#define EEPROM_SIZE 1
const int RelayPin = 25;    // the number of the pushbutton pin
void RELAY_CORE1( void * pvParameters ){
  EEPROM.begin(EEPROM_SIZE);
  pinMode(RelayPin, INPUT_PULLUP);

  for(;;){
    if (0 == EEPROM.read(0))
    {
      Serial.println("Waiting for relay triggering......");
      while(digitalRead(RelayPin));
      EEPROM.write(0,1);
      EEPROM.commit();
      Serial.println("EEPROM set successfully");
    }
    else if(1 == EEPROM.read(0))
    {
      delay(2000);
      EEPROM.write(0,0);
      EEPROM.commit(); 
      Serial.println("EEPROM reset Successfully");
    }
  }
}
/*********************************************************************/
/*********************************BUZZER_CORE*************************/
#include "pitches.h"
#include "esp32-hal-timer.h"

#define BUZZZER_PIN  26
#define ISR_STOP_BUZZER_PIN  27

volatile bool TimerStopBuzzer;
volatile bool isrStopBuzzer;

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

/* Interrupt Service Routine */
void IRAM_ATTR isr() {
  isrStopBuzzer = false;
}

/* Timer Interrupt */
hw_timer_t *timer = NULL;
void IRAM_ATTR onTimer() {
  TimerStopBuzzer = false;
}
/*********************************************************************/

void setup() {
  Serial.begin(115200); 
  delay(5000);
/*********************************BUZZER_CORE*************************/  
  pinMode(ISR_STOP_BUZZER_PIN, INPUT_PULLUP);  // Set ISR pin as input
  attachInterrupt(ISR_STOP_BUZZER_PIN, isr, FALLING); // Attach ISR to the pin
  Serial.println("Interupt finished");
  TimerStopBuzzer = true;
  isrStopBuzzer = true;
  // Create hardware timer
  timer = timerBegin(1000000);                           // Timer frequency
  timerAttachInterrupt(timer, &onTimer);                // Attach interrupt
  /*
  1000000 = 1sec
  10000000 = 10sec
  300000000 = 60sec
  */
  timerAlarm(timer, 300000000, true, 0);                  // Match value, auto-reload, continuous  
/*********************************************************************/
    // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //create a task that will be executed in the SERVER_OTA_CORE() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    SERVER_OTA_CORE,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the RELAY_CORE1() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    RELAY_CORE1,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 

}

void loop() {
/*************BUZZER_CORE1************/
  if (TimerStopBuzzer == true && isrStopBuzzer == true) {
    Serial.println("Starting buzzer...........");
    for (int thisNote = 0; thisNote < 8; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(BUZZZER_PIN, melody[thisNote], noteDuration);

      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZZER_PIN);
    }
  }
/************************************/
}
