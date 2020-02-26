#include "ptt.h"

#define LED0_PORT 2
#define LED1_PORT 3

PTTasker tasker;

PT_THREAD(led(pt_t *p, int port, int delay)){
    PT_BEGIN(p);
    digitalWrite(port, HIGH);
    PT_DELAY(p, delay);
    digitalWrite(port, LOW);
    PT_DELAY(p, delay);
    PT_END(p);
}

PT_THREAD(led0(pt_t *p)){
    PT_BEGIN(p);
    PT_SUBTHREAD(p, led, LED0_PORT, 500);
    PT_END(p);
}

PT_THREAD(led1_v1(pt_t *p)){
    PT_BEGIN(p);
    PT_SUBTHREAD(p, led, LED1_PORT, 1000);
    PT_END(p);
}

PT_THREAD(led1_v2(pt_t *p)){
    PT_BEGIN(p);
    PT_SUBTHREAD_R(p, led, 10, LED1_PORT, 1000);
    PT_END(p);
}

void setup() {
    pinMode(LED0_PORT, OUTPUT);
    pinMode(LED1_PORT, OUTPUT);
    //loop blinking of LED0
    tasker.loop(&led0);

    //10 times LED1 blinking
    tasker.perform(&led1_v1, 10);
    //task led1_v2 waits a finish of task led1_v1
    //and perform 10 times LED1 blinking (PT_SUBTHREAD_R)
    tasker.perform_after(&led1_v2, 1, &led1_v1);
}

void loop() {
  tasker.handle();
}
