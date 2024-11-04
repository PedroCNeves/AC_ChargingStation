#include <Arduino.h>
#include "PWM.h"

#define CP_OUT 10 // Setting Arduino pin 10 as output - Control Pilot signal generator
#define CP_IN A0 // Setting Arduino pin A0 as input - Control Pilot signal reader

int counter = 0; // Initialize counter for debug messages over the serial port
int counter2 = 0; // Initialize counter for LED blinking when charging

float cp_pwm = 255; // Initialize the PWM value for the Control Pilot signal generator (255 = flat positive signal)
int frequency = 1000; // Initialize the frequency of the PWM signal (1 kHz)
// Initialize the PWM value for the Control Pilot signal generator when charging 
// Example: Set to 10 A -> Duty Cycle = 10/0.6 = 16.67% -> 255 * 0.1667 = 42.5
// Example: Set t0 16 A -> Duty Cycle = 16/0.6 = 26.66% -> 255 * 0.266 = 68.3
float cp_pwm_charging = 42; // (42 = 10 A)

int findPeakVoltage();  // Function prototype for finding the peak voltage read by pin A0 and determine the charging state (A, B, C, F)
int peak_voltage = 0; // Initialize the peak voltage read by pin A0
int current_voltage = 0; // Initialize the current voltage read by pin A0

char state = 'a'; // Initialize the charging state (A, B, C, F)
int charging = 0; // Initialize the charging state (0 = not charging, 1 = charging)

void setup() {
  // Set up the serial port. This is useful for debugging
  Serial.begin(9600); // Set the baud rate to 9600 bps

  // Set up the PWM
  InitTimersSafe();
  bool success = SetPinFrequencySafe(CP_OUT, frequency); // Set the frequency of the PWM signal to 1 kHz
  if(success) {
    pinMode(CP_OUT, OUTPUT); // Set the CP_OUT pin as output
    pwmWrite(CP_OUT, 255); // Set the CP_OUT pin to +12V
  }

  // Set up the LED pins as output
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  // Set the Relay pin as output
  pinMode(16, OUTPUT);

  // Initialize Relay and LEDs states
  digitalWrite(16, LOW); // Relays open
  digitalWrite(6, HIGH); // Top LED on
  digitalWrite(2, LOW); // Bottom LED off
}

void loop() {
  counter++;
  if (counter > 10) {
    counter = 0;
    Serial.println("Counter = 10");
    Serial.print("Peak Voltage: ");
    Serial.println(peak_voltage);
    Serial.println(state);
  }
  if (charging == 1) {
    counter2++;
    if (counter2 > 10) {
      counter2 = 0;
      // Blink the middle LEDs when charging
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
    pwmWrite(CP_OUT, cp_pwm_charging); // Exanple 10 A -> Duty Cycle = 10/0.6 = 16.67% -> 255 * 0.1667 = 42.5
    digitalWrite(16, LOW); // Relays open
    digitalWrite(2, HIGH); // Bottom LED on
    digitalWrite(3, LOW); // Middle d LED off
    digitalWrite(4, LOW); // Middle u LED off
  }
  else if (peak_voltage > 780) {
    //state C
    state = 'c';
    charging = 1;
    pwmWrite(CP_OUT, cp_pwm_charging); // Exanple 10 A -> Duty Cycle = 10/0.6 = 16.67% -> 255 * 0.1667 = 42.5
    digitalWrite(16, HIGH); // Relays closed
    digitalWrite(2, HIGH); // Bottom LED on
  }
  else {
    //state F
    state = 'f'; // CP set to fixed -12V
    charging = 0;
    pwmWrite(CP_OUT, 0);
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
