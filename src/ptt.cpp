#include "ptt.h"

task_t* PTTasker::getTask(pt_f f){
	unordered_map<void*, task_t*>::iterator it = tmap.find((void*)f);
	if (it != tmap.end()){
		return it->second;
	}
	return NULL;
}

uint8_t PTTasker::isPerforming(pt_f f){
	task_t *t = getTask(f);
	return t && t->state != 0 && !t->start_condition_f;
}

uint8_t PTTasker::isCompleted(pt_f f){
	task_t *t = getTask(f);
	return t && t->state == 0;
}

uint8_t PTTasker::perform(uint8_t group_id, pt_f f, int8_t num_repeats, pt_s stop_f, pt_c start_condition_f){
	//Serial.printf("perform: num_repeats=%d; for=%u\n", num_repeats, (void*)f);
	task_t *t = getTask(f);
	if (!t){
		t = new task;
		tmap[(void*)f] = t;
	}
	  
	if (t->state == 0){
		t->group_id = group_id;
		t->state = num_repeats;
		t->start_condition_f = start_condition_f;
		t->pt.reset(); 
		return 1;
	}
 
	//Serial.println("task in progress");
	return 0;
}

uint8_t PTTasker::perform(pt_f f, int8_t num_repeats, pt_s stop_f, pt_c start_condition_f){
	return perform(PT_DEFAULT_GROUP_ID, f, num_repeats, stop_f, start_condition_f);
}

uint8_t PTTasker::perform_after(uint8_t group_id, pt_f f, int8_t num_repeats, pt_f after_f, pt_s stop_f){
	return perform(group_id, f, num_repeats, stop_f, std::bind(&PTTasker::isCompleted, this, after_f));	
}

uint8_t PTTasker::perform_after(pt_f f, int8_t num_repeats, pt_f after_f, pt_s stop_f){
	return perform_after(PT_DEFAULT_GROUP_ID, f, num_repeats, after_f, stop_f);
}

uint8_t PTTasker::once(uint8_t group_id, pt_f f, pt_s stop_f, pt_c start_condition_f){
	return perform(group_id, f, PT_ONCE, stop_f, start_condition_f);
}

uint8_t PTTasker::once(pt_f f, pt_s stop_f, pt_c start_condition_f){
	return once(PT_DEFAULT_GROUP_ID, f, stop_f, start_condition_f);
}

uint8_t PTTasker::loop(uint8_t group_id, pt_f f, pt_s stop_f, pt_c start_condition_f){
	return perform(group_id, f, PT_LOOP, stop_f, start_condition_f);
}

uint8_t PTTasker::loop(pt_f f, pt_s stop_f, pt_c start_condition_f){
	return loop(PT_DEFAULT_GROUP_ID, f, stop_f, start_condition_f);
}

uint8_t PTTasker::stopTask(pt_f f, uint8_t use_default_stop){
	task_t *t = getTask(f);
	if (!t || t->state == 0){
		return 0;
	}
	
	if (use_default_stop && t->stop_f){
		t->stop_f();
	}

	t->reset();
	return 1;
}

uint8_t PTTasker::stop(pt_f f, pt_f stop_f){
	if (stopTask(f, !stop_f)){
		if (stop_f){
			perform(stop_f, PT_ONCE);
		}
		return 1;
	}
	return 0;
}

uint8_t PTTasker::stop(pt_f f, pt_s stop_f){
	if (stopTask(f, !stop_f)){
		if (stop_f){
			stop_f();
		}
		return 1;
	}
	return 0;
}

void PTTasker::stopAll(uint8_t group_id){
	for (auto it : tmap) {
		task_t *t = (task_t *)it.second;
		if (t->group_id == group_id){
			stopTask((pt_f)it.first);
		}
	}
}

void PTTasker::handle(){
	for (auto it : tmap) {
		task_t *t = (task_t *)it.second;
		
		uint8_t start;
		if (t->start_condition_f){
			start = t->start_condition_f();
			if (start){
				t->start_condition_f = NULL;
			}
		}else{
			start = t->state != 0;
		}
		
		if (start){
			if (((pt_f)it.first)(&t->pt)){
				if (t->state > 0){
					--t->state;
				}
				//Serial.printf("Task completed: %u; state=%d\n", (void*)it.first, t->state);
			}
		}
	}
}
