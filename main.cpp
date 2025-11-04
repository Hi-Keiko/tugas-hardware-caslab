#include <Arduino.h>
void setup() {
    // Initialize serial communication at a baud rate of 115200
    Serial.begin(9600);
}

void loop() {
    // Print "Hello World!" to the serial monitor, followed by a new line
    Serial.println("Hello World!");
    // Pause for 2000 milliseconds (2 seconds) before repeating
    delay(2000);
}
