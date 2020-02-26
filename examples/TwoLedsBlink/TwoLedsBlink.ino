#include "ptt.h"

#define LED0_PORT 2
#define LED1_PORT 3

PTTasker tasker;

PT_THREAD(led0(pt_t *p)){
    PT_BEGIN(p);
    digitalWrite(LED0_PORT, HIGH);
    PT_DELAY(p, 500);
    digitalWrite(LED0_PORT, LOW);
    PT_DELAY(p, 500);
    PT_END(p);
}

PT_THREAD(led1(pt_t *p)){
    PT_BEGIN(p);
    digitalWrite(LED1_PORT, HIGH);
    PT_DELAY(p, 1000);
    digitalWrite(LED1_PORT, LOW);
    PT_DELAY(p, 1000);
    PT_END(p);
}

void setup() {
    pinMode(LED0_PORT, OUTPUT);
    pinMode(LED1_PORT, OUTPUT);
    tasker.loop(&led0);
    tasker.perform(&led1, 10);
}

void loop() {
  tasker.handle();
}
