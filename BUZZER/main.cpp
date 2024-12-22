#include <Arduino.h>
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

void setup() {
  Serial.begin(115200);

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
  Serial.println("Timer finished");
}

void loop() {
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
}

