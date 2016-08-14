
#include "kinetis.h"
#include "core_pins.h"

#include hibernate.c

void flashLed(int del)
{
   pinMode(LED_BUILTIN, OUTPUT);
   digitalWrite(LED_BUILTIN, HIGH);
   delay(del);
   digitalWrite(LED_BUILTIN, LOW);
   pinMode(LED_BUILTIN, INPUT);
}

void setup() {
   //
   // put your setup code here, to run once:
   flashLed(100);
   //
   hibernate(5);   
}

void loop() {
   // put your main code here, to run repeatedly:
   //
}
