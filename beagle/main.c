#include <stdio.h>
#include <stdint.h>

#include "servo.h"

int main() {

   servo servo;
   servo.ctrl_pin = "P8_13";
   servo.prunum = 0;

   if(init_servo(&servo) == 0) {
      servo_zero(&servo);
      float f;
      while (1) {
         scanf("%f", &f);
         servo_rotate(&servo, f);
         servo_position(&servo);
      }
   }
}