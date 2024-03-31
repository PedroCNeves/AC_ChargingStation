#include <Arduino.h>
#include "PWM.h"

#define CP_OUT 10
#define CP_IN A0

int counter = 0;
int counter2 = 0;

float cp_pwm = 255;
int frequency = 1000;

int findPeakVoltage();
int peak_voltage = 0;
int current_voltage = 0;

char state = 'z';

int charging = 0;

void setup() {
  // Set up the serial port
  Serial.begin(9600);

  // Set up the PWM
  InitTimersSafe();
  bool success = SetPinFrequencySafe(CP_OUT, frequency);
  if(success) {
    pinMode(CP_OUT, OUTPUT);  
  }

  // Set up the LEDs
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  //Relay
  pinMode(16, OUTPUT);

  digitalWrite(16, LOW); // Relays open
  digitalWrite(6, HIGH); // Top LED on
  digitalWrite(2, LOW); // Bottom LED off
}

void loop() {
  counter++;
  if (counter > 10) {
    counter = 0;
    Serial.println("contador = 10");
    Serial.print("Peak Voltage: ");
    Serial.println(peak_voltage);
    Serial.println(state);
  }
  if (charging == 1) {
    counter2++;
    if (counter2 > 10) {
      counter2 = 0;
      //if digitalwrite 3 is high, turn it off
      if (digitalRead(3) == HIGH) {
        digitalWrite(3, LOW);
        digitalWrite(4, HIGH);
      }
      else {
        digitalWrite(3, HIGH);
        digitalWrite(4, LOW);
      }
    }
  }
  
  // Read the CP voltage peaks
  findPeakVoltage();

  // Set the charging state based on the peak voltage
  if (peak_voltage > 970) {
    //state A
    state = 'a';
    charging = 0;
    pwmWrite(CP_OUT, 255); // CP set to fixed +12V
    digitalWrite(16, LOW); // Relays open
    digitalWrite(2, LOW); // Bottom LED off
    digitalWrite(3, LOW); // Middle d LED off
    digitalWrite(4, LOW); // Middle u LED off
  }
  else if (peak_voltage > 870) {
    //state B
    state = 'b';
    charging = 0;
    pwmWrite(CP_OUT, 42); // 10 A -> Duty Cycle = 10/0.6 = 16.67% -> 255 * 0.1667 = 42.5
    digitalWrite(16, LOW); // Relays open
    digitalWrite(2, HIGH); // Bottom LED on
    digitalWrite(3, LOW); // Middle d LED off
    digitalWrite(4, LOW); // Middle u LED off
  }
  else if (peak_voltage > 780) {
    //state C
    state = 'c';
    charging = 1;
    pwmWrite(CP_OUT, 42);
    digitalWrite(16, HIGH); // Relays closed
    digitalWrite(2, HIGH); // Bottom LED on
  }
  else {
    //state F
    state = 'f';
    charging = 0;
    pwmWrite(CP_OUT, 255);
    digitalWrite(16, LOW); // Relays open
    digitalWrite(2, LOW); // Bottom LED off
    digitalWrite(3, LOW); // Middle d LED off
    digitalWrite(4, LOW); // Middle u LED off
  }



}

// function that finds the peak voltage read by pin A0
  int findPeakVoltage() {
    int i = 0;
    current_voltage = 0;
    peak_voltage = 0;
    while (i < 1000) {
      current_voltage = analogRead(CP_IN);
      if (current_voltage > peak_voltage) {
        peak_voltage = current_voltage;
      }
      i++;
    }
    return peak_voltage;
  }
