#ifndef ptt_h
#define ptt_h

#include <iostream>
#include <unordered_map>
using namespace std;

#include <functional>


#define PT_INCLUDE_DEEP 10

struct pt {
	uint16_t lc[PT_INCLUDE_DEEP];
	uint8_t sr[PT_INCLUDE_DEEP];	//num_repeats for PT_SUBTHREAD
	uint8_t lc_p;					//current deep level for PT_SUBTHREAD
	uint32_t st;					//call time of PT_DELAY
   
	void reset(){ lc_p = 0; lc[0] = 0; };
};
typedef pt pt_t;

#define PT_THREAD(name_args)  char name_args

#define PT_BEGIN(pt) switch(pt->lc[pt->lc_p]) { case 0:
#define PT_WAIT_UNTIL(pt, c) pt->lc[pt->lc_p] = __LINE__; case __LINE__: \
							               if(!(c)) return 0
#define PT_END(pt) } pt->lc[pt->lc_p] = 0; return 2

#define PT_WAIT(pt, c, timeout) pt->st = millis(); PT_WAIT_UNTIL(pt, c || (millis() - pt->st >= timeout))
#define PT_DELAY(pt, ms) PT_WAIT(pt, 0, ms)

#define PT_SUBTHREAD_R(pt, pt_thread, r, ...) pt->lc[pt->lc_p + 1] = 0; pt->sr[pt->lc_p + 1] = r; \
										                          pt->lc[pt ->lc_p] = __LINE__; case __LINE__: \
									                            pt->lc_p++; if (pt_thread(pt, ##__VA_ARGS__)) { if(--pt->sr[pt->lc_p--]) return 0; } else { pt->lc_p--; return 0; }
#define PT_SUBTHREAD(pt, pt_thread, ...) PT_SUBTHREAD_R(pt, pt_thread, 1, ##__VA_ARGS__)


#define PT_LOOP -1
#define PT_ONCE 1

#define PT_DEFAULT_GROUP_ID 0

typedef PT_THREAD((*pt_f)(pt_t*));
typedef std::function<uint8_t()> pt_c;
typedef std::function<uint8_t()> pt_s;

struct task {
	uint8_t group_id;
	
	pt_t pt;
	int8_t state;
  
	pt_c start_condition_f;
	pt_c stop_f;
 
	task(){ reset(); }
	void reset(){ state = 0; start_condition_f = NULL; stop_f = NULL;}
};
typedef task task_t;

class PTTasker{
private:
	unordered_map<void*, task_t*> tmap;
 
	task_t* getTask(pt_f f);
	uint8_t stopTask(pt_f f, uint8_t use_default_stop = true);
public:
	uint8_t isCompleted(pt_f f);
	uint8_t isPerforming(pt_f f);
	
	uint8_t perform(uint8_t group_id, pt_f f, int8_t num_repeats, pt_s stop_f = NULL, pt_c start_condition_f = NULL);
	uint8_t perform(pt_f f, int8_t num_repeats, pt_s stop_f = NULL, pt_c start_condition_f = NULL);
	
	uint8_t perform_after(uint8_t group_id, pt_f f, int8_t num_repeats, pt_f after_f, pt_s stop_f = NULL);
	uint8_t perform_after(pt_f f, int8_t num_repeats, pt_f after_f, pt_s stop_f = NULL);

	uint8_t once(uint8_t group_id, pt_f f, pt_s stop_f = NULL, pt_c start_condition_f = NULL);
	uint8_t once(pt_f f, pt_s stop_f = NULL, pt_c start_condition_f = NULL);
	
	uint8_t loop(uint8_t group_id, pt_f f, pt_s stop_f = NULL, pt_c start_condition_f = NULL);
	uint8_t loop(pt_f f, pt_s stop_f = NULL, pt_c start_condition_f = NULL);

	uint8_t stop(pt_f f, pt_f stop_f = NULL);
	uint8_t stop(pt_f f, pt_s stop_f);

	void stopAll(uint8_t group_id = PT_DEFAULT_GROUP_ID);
	
	void handle();
};

#endif
