#include <Arduino.h>

uint32_t count;

void setup() {
    Serial.begin(115200);
    Serial.println("Hello World");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    Serial.println(++count);

    digitalWrite(LED_BUILTIN, count % 2);

    delay(1000);
}