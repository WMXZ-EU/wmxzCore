
#include "kinetis.h"
#include "core_pins.h"

#include "hibernate.h"

void flashLed(int del)
{
   pinMode(LED_BUILTIN, OUTPUT);
   digitalWrite(LED_BUILTIN, HIGH);
   delay(del);
   digitalWrite(LED_BUILTIN, LOW);
   pinMode(LED_BUILTIN, INPUT);
   delay(del);
}

void setup() {
   //
   // put your setup code here, to run once:
   while(millis()<3000)
   flashLed(100);
   //
   hibernate(5);   
}

void loop() {
   // put your main code here, to run repeatedly:
   //
}

