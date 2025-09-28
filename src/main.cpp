#include <Arduino.h>

#define LED_PIN LED_BUILTIN   // On NodeMCU, LED_BUILTIN = GPIO2 (D4)

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, LOW);   // Turn LED on (active LOW)
  delay(500);
  digitalWrite(LED_PIN, HIGH);  // Turn LED off
  delay(500);
}
