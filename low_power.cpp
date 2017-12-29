
#include "avr/sleep.h"
// Use pin 2 as wake up pin
const int wakeUpPin = 2;
const int ledPin = 13;
void wakeUp()
{
    // Just a handler for the pin interrupt.
    // Disable external pin interrupt on wake up pin.
    sleep_disable();
    detachInterrupt(0);
    //Serial.println("in the interrupt handler");
}

void setup()
{
    Serial.begin(9600);
    // Configure wake up pin as input.
    // This will consumes few uA of current.
    pinMode(wakeUpPin, INPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    digitalWrite(wakeUpPin, HIGH);
}

void loop()
{
    delay(1000);
    sleepSetup();
}

void sleepSetup()
{
    sleep_enable();
    attachInterrupt(0, wakeUp, LOW);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    digitalWrite(13, LOW);
    sleep_cpu();
    Serial.println("just wake up!");
    digitalWrite(13, HIGH);
}
