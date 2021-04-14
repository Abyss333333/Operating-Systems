#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"
#include <stdbool.h>

/* This is the wait queue structure */
struct wait_queue {
	/* ... Fill this in Lab 3 ... */

	struct thread *first; // first in wait queue
	struct thread *last; // last in wait queue 

	
};

Tid
thread_kill_q(Tid tid);

/* This is the thread control block */
struct thread {
	/* ... Fill this in ... */
	ucontext_t t_context ;
	void *stack_pointer;// pointer to stack pointer so easy to delete stack 
	Tid thread_id;
	int state; // 0 running, 1 ready, 2 waiting, 3 waiting to be killed, 4 needs to be killed after wait
	// state isnt really used much as of now
	bool kernel; // isnt used much now tbh
	struct thread *next_ready; // next in line in readyq
	struct thread *prev_ready; // prev in line in readyq
	struct thread *next_kill; // next to be killed
	struct thread *next_wait; // next in line waiting
	struct thread *prev_wait; // previous in line waiting 
	int wait_check; //used in thread_sleep
	int needs_to_exit; // set in thread_kill
	int thread_yielded_to;// used in thread sleep
	
	struct thread *blocking_q_first;
	struct thread *blocking_q_last;
	struct thread *next_block;
	int block_check;
	int got_blocked_by;
	Tid block_list[THREAD_MAX_THREADS];

	int has_lock;
	
	
	

	
	
	
	

	



};


struct thread_table {
	Tid thread_list[THREAD_MAX_THREADS];
	Tid current_Tid;
	struct thread *t_ptrs[THREAD_MAX_THREADS];
	int num_of_threads;
	volatile int check; // to avoid loops of setcontext and get_context
	volatile int wait_check; // to avoid loops of wait
	int exit_check; // checks that the thread yield is coming from thread_exit. Allows not to set us current threads context.
	int suicide_ret; // when a thread is comming exit on itself
	int exit_normal;// = 1 when thread_exit comes from stub funtion (meaning program is done running). =0 if thread is killing itself
	struct thread *ready_q_first;// readyq first
	struct thread *ready_q_last;// readyq last
	struct thread *kill_q_first;// killq first
	struct thread *kill_q_last; // killq last
	int coming_from_exit; // coming from exit. purpose of this is so the "running thread" which is actually the next thread wont
	//get put in readyq, however as of now it makes very little difference for some reason
	struct wait_queue *wait_q;

	int lock_with_thread;
	
};

// thread_table
struct thread_table *t_table  ;
 

// used for printing and checking
void check_readyq(){
	struct thread *ptr = t_table->ready_q_first;
	int count = 0;
	while (ptr!= NULL){
	
		unintr_printf("%d check ready\n", ptr->thread_id);
		ptr = ptr->next_ready;
		count++;
	}
	unintr_printf("%d HERE\n", count);
}
// used for printing and checking
void check_killq(){
	struct thread *ptr =t_table->kill_q_first;
	
	while (ptr!= NULL){
	
		unintr_printf("%d kill ready\n", ptr->thread_id);
		ptr = ptr->next_kill;
	}
	//printf("HERE\n");
}

void clean_killq() {

	
	struct thread *temp = t_table->kill_q_first;
	t_table->kill_q_first = NULL;
	t_table->kill_q_last = NULL;

	while (temp!= NULL){
		
		struct thread *tempx = temp;


		temp = temp->next_kill;

		free(tempx->stack_pointer);
		tempx->stack_pointer = NULL;
		free(tempx);
		tempx = NULL;
		
	}
}

void xxx(int x){
	unintr_printf("Heh %d\n", x);
}

void yyy();

void stub(void(*thread_main)(void*), void*arg) {

	//Tid ret;
	interrupts_set(1);// cause thread contexts are created with interrupts down in thread_create

	if (t_table->t_ptrs[t_table->current_Tid]->needs_to_exit == 0){
		if (!interrupts_enabled()){
			yyy();
		}
		thread_main(arg);
	}

	interrupts_off();

	
	
	


	//check_killq();
	 t_table->exit_normal =1;

	

	//clean_killq();
	 
	
	thread_exit();

	//clean_killq();
	
	
	
	
	exit(0);
	//

	

	
	
	
	

}

void (*stub_fcn)(void(*)(void*), void*) =&stub;







void
thread_init(void)
{
	
	/* your optional code here */

	
	// Cconstruct thread table
	t_table = (struct thread_table *)malloc (sizeof(struct thread_table));
	
	

	// initialize thread structure for initial thread and label it as Kernel
	struct thread *t_init;
	t_init = (struct thread *)malloc (sizeof(struct thread));
	int err = getcontext(&t_init->t_context);
	assert(!err);
	t_init->kernel = true;
	t_init->thread_id = 0;
	t_init->state = 0;
	//t_init->stack_pointer = t_init->t_context.uc_mcontext.gregs[REG_RSP];
	t_init->next_ready = NULL;
	t_init->prev_ready = NULL;

	
	t_init->t_context.uc_mcontext.gregs[REG_RIP] = (long int) stub_fcn;//1

	//set t_table values
	t_table->thread_list[0] = 1;
	t_table->current_Tid = 0;
	t_table->t_ptrs[0] = t_init;
	t_table->num_of_threads = 1;
	t_table->lock_with_thread = -1;

	


}

Tid
thread_id()
{
	
	int ret_id = t_table->current_Tid;
	
	
	return ret_id ;
}

Tid
thread_create(void (*fn) (void *), void *parg)
{	

	interrupts_set(0);
	
	if (t_table->num_of_threads == THREAD_MAX_THREADS){

		interrupts_set(1);
		return THREAD_NOMORE;
	}

	

	else {

		//make thread state table for new thread
		struct thread *t_created;
		t_created = (struct thread *)malloc (sizeof(struct thread));

		if (t_created == NULL){
			interrupts_set(1);
			return THREAD_NOMEMORY;
		}
		
		
		//allocate stack
		void *stack = malloc(THREAD_MIN_STACK);

		if (stack == NULL){
			free(t_created);
			interrupts_set(1);
			return THREAD_NOMEMORY;
		}

		//set thread stack pointer to stack
		t_created->stack_pointer = stack;

		


		//get current context
		ucontext_t current_context ;
		int err = getcontext(&(current_context));
		assert(!err);

		//changes to context
		//1st RIP, 2nd stack point, 3rd paramaeters
		current_context.uc_mcontext.gregs[REG_RIP] = (long int) stub_fcn;//1
		current_context.uc_mcontext.gregs[REG_RSP] = (long int) stack + THREAD_MIN_STACK - 8;//2  aligning the stack 
		current_context.uc_mcontext.gregs[REG_RDI] = (long long int) fn;//3
		current_context.uc_mcontext.gregs[REG_RSI] = (long long int) parg;//3
		current_context.uc_stack.ss_sp = stack;
		current_context.uc_stack.ss_size = THREAD_MIN_STACK -8;
		current_context.uc_mcontext.gregs[REG_RBP] = (long int) stack - 8;//2  aligning the stack 

		//make the created threads context = the changed context
		t_created->t_context = current_context;



		//initialize Thread state block values
		t_created->state = 1; //1 for ready
		t_created->next_ready = NULL;
		t_created->prev_ready = NULL;
		t_created->thread_id = t_table->num_of_threads;
		t_created->kernel = false;
		t_created->next_kill = NULL;
		t_created->next_wait = NULL;
		t_created->prev_wait = NULL;
		t_created->got_blocked_by = -1;

		//update thread table
		t_table->thread_list[t_created->thread_id] = 1;
		t_table->t_ptrs[t_created->thread_id] = t_created;
		t_table->num_of_threads ++;

		//add to ready queue
			
		if (t_table->ready_q_first == NULL && t_table->ready_q_last == NULL){
			t_table->ready_q_first = t_created;
			t_table->ready_q_last = t_created;
		}
		else {
			
			t_created->prev_ready =t_table->ready_q_last ;
			
			t_table->ready_q_last->next_ready = t_created;
			t_table->ready_q_last = t_created;
		}

		//check_killq();
		//printf("%d IX \n", t_created->thread_id);
		 int return_v= t_created->thread_id;
		interrupts_on();
		return return_v;
		
	}


	
}


int ret_val;
Tid
thread_yield(Tid want_tid)
{
	if (interrupts_enabled()){
		interrupts_off();	
	}
	
	if (want_tid == THREAD_SELF ){
		int j = t_table->current_Tid;
		interrupts_set(1);
		return j;
	}

	else if (want_tid == THREAD_ANY && t_table->ready_q_first == NULL){
		interrupts_set(1);
		return THREAD_NONE;
	}

	

	else if (want_tid > THREAD_MAX_THREADS -1 ){
		interrupts_set(1);
		return THREAD_INVALID;
	}

	else if (want_tid < -7){
		interrupts_set(1);
		return THREAD_INVALID;
	}
	else if (t_table->thread_list[want_tid] == 0 && want_tid > 0) {
		interrupts_set(1);
		return THREAD_INVALID;
	}

	else if (t_table->num_of_threads == 1 && want_tid != t_table->current_Tid){
		interrupts_set(1);
		return THREAD_NONE;
	}
	
	else if (t_table->thread_list[want_tid] == 1 || want_tid == THREAD_ANY){

		interrupts_off();
		//volatile int check = 0;
		ucontext_t current_context ;
		ret_val = t_table->current_Tid;
		// the suicide ret will be returned as we are yielding from it and it technically the current running thread
		if (t_table->suicide_ret!=0){
			ret_val = t_table->suicide_ret;
			
		}
		
		
	
		//if current thread is exiting, running thread is going to be the next thread that will be running 
		//Hence why in the upcoming if statement, running_threads t_context is its own t_context and not current_context got from
		//the above getcontext
		int running_Tid = t_table->current_Tid;
		struct thread *running_thread = t_table->t_ptrs[running_Tid];

	

		
		if (t_table->exit_check == 1 ){
			running_thread->t_context = running_thread->t_context;
			//running_thread->t_context.uc_mcontext.gregs[REG_RIP] = (long int) stub_fcn;//1
		}
		else{
			
			
			int err = getcontext(&(t_table->t_ptrs[t_table->current_Tid]->t_context));
			assert(!err);
		}

		running_thread = t_table->t_ptrs[t_table->current_Tid];// SUPER IMPORTANT DONT DELETE.

		
		t_table->exit_check = 0;
		
		
		if (t_table->check == 0){

			// put current thread on ready queue, unless coming from exit, cause then you need to take out current tid from ready q
		// as t_table->running thread now points to the thread thats going to run next
		// if current_thread needs to exit, dont put in running q
			
			if (t_table->coming_from_exit == 0)
			{ 
				if (t_table->ready_q_first == NULL) {
					t_table->ready_q_first = running_thread;
					t_table->ready_q_last = running_thread;
					running_thread->state = 1;
				}
				else{
				
					running_thread->prev_ready = t_table->ready_q_last;
					t_table->ready_q_last->next_ready = running_thread;
					running_thread->next_ready = NULL;
					//printf("GAP; \n");
					running_thread->state = 1;
					t_table->ready_q_last = running_thread;
				
					}
			}

		
			
			
			
			
			// done with putting currrent thread on ready queue
			//now time to switch to thread inputted
			if (want_tid == THREAD_ANY){
					if(t_table->current_Tid == 0 || t_table->t_ptrs[0]->state == 2 ){
						running_thread = t_table->ready_q_first;
					}
				else{	
					running_thread = t_table->t_ptrs[0];
				}
			
			}
			else {
				running_thread = t_table->t_ptrs[want_tid];
			}

			
				
			
			current_context = running_thread->t_context;
			running_thread->state = 0;
			


			// before setcontext, update t_table
			t_table->current_Tid = running_thread->thread_id;
			
			
			
			//take it out of ready queue
			
			if (t_table->ready_q_first == running_thread){ ;// if running thread is first in queue

				if (t_table->ready_q_last == running_thread){ // if running thread is first and last in queue (only one)
					t_table->ready_q_first = NULL;
					t_table->ready_q_last = NULL;
					running_thread->next_ready = NULL;
				}

				else{ // its first but not last in the queue

					running_thread->next_ready->prev_ready = NULL;
					t_table->ready_q_first = running_thread->next_ready;
					running_thread->next_ready = NULL;

				}

			}

			else if (t_table->ready_q_last == running_thread){ // running thread is last in the ready queue but not the first

					running_thread->prev_ready->next_ready = NULL;
					t_table->ready_q_last = running_thread->prev_ready;
					running_thread->prev_ready = NULL;
			}
			
			else {
				running_thread->next_ready->prev_ready = running_thread->prev_ready;
				running_thread->prev_ready->next_ready = running_thread->next_ready;
				running_thread->next_ready= NULL;
				running_thread->prev_ready = NULL;
			}


			
			
			
			//set check to 1 so this doesnt happen 
			
			
			
			
			t_table->check = 1;
			//interrupts_on();
			int err1 = setcontext(&current_context);
			assert(!err1);
				
			
			
			



			
		}
		t_table->check =0;

		if (t_table->coming_from_exit){
			t_table->coming_from_exit = 0;
			t_table->suicide_ret = 0;
			interrupts_on();
			return ret_val;
		}
		
		//t_table->coming_from_exit = 0;
		//t_table->suicide_ret = 0;
		interrupts_on();
		return ret_val;
		
	}
	else {
		interrupts_on();
		return THREAD_INVALID;
	}
	interrupts_on();
	return 100;
}

void breakq(){
	int x = 8;
	assert(x == 8);
}

void
thread_exit()
{
	
		interrupts_off();
	
	// cleans killq, destryoys threads in the kill queue
	clean_killq();
	

	int x = t_table->current_Tid;

	//make all blocking threads of x, ready
	struct thread *blocking_thread = t_table->t_ptrs[x];
	while (blocking_thread->blocking_q_first != NULL){
		// put the blocked thread in ready queue
		struct thread *temp = blocking_thread->blocking_q_first;
		temp->state = 1; // ready state
		//temp->got_blocked_by = -1; //not blocked by anyone

		if (t_table->thread_list[temp->thread_id] != 0){
			
		
		if (t_table->ready_q_first == NULL){
			t_table->ready_q_first = temp;
			t_table->ready_q_last = temp;

		}
		else {
			t_table->ready_q_last->next_ready = temp;
			temp->prev_ready = t_table->ready_q_last;
			t_table->ready_q_last = temp;
		}
		}
		blocking_thread->block_list[temp->thread_id] =0;
		blocking_thread->blocking_q_first = blocking_thread->blocking_q_first->next_block;
		temp->next_block = NULL;

	}

	blocking_thread->blocking_q_last = NULL;
	blocking_thread = NULL;


	//exit normal == 0 means thread has called exit itself. and suicide ret is the thread_id that would be returned after yielding
	//from this thread. 
	//exit normal == 1 means the program has finished running and is exiting the normal way through the stub function
	if (t_table->exit_normal == 0){
		t_table->suicide_ret = x;
		//t_table->exit_normal = 1;
	}
	// set current_Tid to 0, will be changed later if there is a thread in readyq
	t_table->current_Tid = 0;
	//set check to zero so context can be changed
	t_table->check = 0;


	
	//checking that thread is coming from exit. doesnt use this threads context 
	t_table->exit_check = 1;
	// make exit_normal zero so we can check for more future suicidal threads
	t_table->exit_normal = 0;
	
	thread_kill_q(x); // add to kill q
	
	

	

	if (t_table->ready_q_first!= NULL) {
		t_table->current_Tid = t_table->ready_q_first->thread_id;
	}

	
	t_table->coming_from_exit = 1;
	interrupts_off();
	thread_yield(t_table->current_Tid);
	//clean_killq();

	interrupts_on();
	

	

	


}




Tid thread_kill(Tid tid){

	
	
		interrupts_off();
	
	
	if ( tid < 0 || tid > THREAD_MAX_THREADS-1){
		interrupts_on();
		return THREAD_INVALID;

	}
	else if (t_table->thread_list[tid] == 0 ) {
		interrupts_on();
		return THREAD_INVALID;
	}
	else if (t_table->current_Tid == tid ){
		interrupts_on();
		return THREAD_INVALID;
	}



	else {

		


		if (t_table->t_ptrs[tid]->state == 2)// if its sleeping
		{	

			if (t_table->t_ptrs[tid]->got_blocked_by != t_table->current_Tid){

			
			t_table->t_ptrs[tid]->needs_to_exit = 1; // exit when run again
			interrupts_on();
			return tid;
			}
		}

		//  if (t_table->t_ptrs[tid]->block_list[0] == 1){
		// 	yyy();
		//  	interrupts_on();
		//  	return tid; // child cant kill parent
		//  }

		

			thread_kill_q(tid);
		
		//set the list value to 0 to indicate its not available
		

		struct thread *temp = t_table->kill_q_first;
		struct thread *prev;
		
		while (temp->thread_id != tid) {
			prev = temp;
			temp = temp->next_kill;

		}

		if (temp == t_table->kill_q_first){
			t_table->kill_q_first = temp->next_kill;
			temp->next_kill = NULL;
			
		}
		else if (temp == t_table->kill_q_last){
			t_table->kill_q_last = prev;
			prev->next_kill = NULL;
			temp->next_kill = NULL;
		}

		else {
			prev->next_kill = temp->next_kill;
			temp->next_kill = NULL;
		}

		
		free(temp->stack_pointer);
		temp->stack_pointer = NULL;
		free(temp);
		temp= NULL;

		interrupts_on();
		return tid;


	}

}


Tid
thread_kill_q(Tid tid)
{

	interrupts_off();
	
	if ( tid < 0 || tid > THREAD_MAX_THREADS-1){
		interrupts_on();
		return THREAD_INVALID;

	}
	
	
	else {
		
		//set the list value to 0 to indicate its not available
		t_table->thread_list[tid] = 0;
		//decrement total thread count
		t_table->num_of_threads -- ;
		
		// take it out of ready queue
		struct thread *killed_thread = t_table->t_ptrs[tid];
		// take it out t_Table
		t_table->t_ptrs[tid] = NULL;
		
		
		//take it out of ready queue

		if (t_table->ready_q_first == killed_thread){ ;// if killed thread is first in queue

				if (t_table->ready_q_last == killed_thread){ // if killed thread is first and last in queue (only one)
					t_table->ready_q_first = NULL;
					t_table->ready_q_last = NULL;
					killed_thread->next_ready = NULL;
				}

				else{ // its first but not last in the queue

					killed_thread->next_ready->prev_ready = NULL;
					t_table->ready_q_first = killed_thread->next_ready;
					killed_thread->next_ready = NULL;

				}

			}

			else if (t_table->ready_q_last == killed_thread){ // killed is last in the ready queue but not the first

					killed_thread->prev_ready->next_ready = NULL;
					t_table->ready_q_last = killed_thread->prev_ready;
					killed_thread->prev_ready = NULL;
			}
			
			else if (killed_thread->next_ready != NULL){ // coming from exit
				killed_thread->next_ready->prev_ready = killed_thread->prev_ready;
				killed_thread->prev_ready->next_ready = killed_thread->next_ready;
				killed_thread->next_ready= NULL;
				killed_thread->prev_ready = NULL;
			}














		



			
		int ret_id = killed_thread->thread_id;
		int kill_id = killed_thread->thread_id ;
		killed_thread->thread_id = kill_id;

		// put it in Kill queue
		if (t_table->kill_q_first == NULL) {
			t_table->kill_q_first = killed_thread;
			t_table->kill_q_last = killed_thread;
			killed_thread->state = 3; // waiting to be killed
			killed_thread->next_kill = NULL;
		}
		else{
			//killed_thread->prev_ready = kill_q->last;
			t_table->kill_q_last->next_kill = killed_thread;
			killed_thread->state = 3;
			t_table->kill_q_last = killed_thread;
				
		}

		//printf("KILLLL %d\n",killed_thread->thread_id);
		
		
		return ret_id;

		


	}
	
	return THREAD_FAILED;
}

/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* make sure to fill the wait_queue structure defined above */
struct wait_queue *
wait_queue_create()
{
	interrupts_off();
	struct wait_queue *wq;

	wq = malloc(sizeof(struct wait_queue));
	assert(wq);

	//TBD();
	wq->first = NULL;
	wq->last = NULL;

	t_table->wait_q = wq;

	interrupts_on();
	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	//TBD();
	if (interrupts_enabled()){
		interrupts_off();
	}
	if ( wq->first == NULL && wq->last == NULL) {
		free(wq);
	}
	interrupts_on();
}
void check(){
	int x = 1;
	assert(x);
}
void check_waitq(){

	interrupts_off();
	
	struct thread *ptr =t_table->wait_q->first;
	
	while (ptr!= t_table->wait_q->last){
	
		printf("%d wait ready\n", ptr->thread_id);
		ptr = ptr->next_wait;
	}
	if (ptr != NULL){
		printf("%d wait ready\n", ptr->thread_id);
	}


	//interrupts_on();

	

}
Tid thread_sleep_(struct wait_queue *queue){
	return 0;
}

void yyy(){
	int x =0;
	x++;
	x--;
}
Tid
thread_sleep(struct wait_queue *queue)
{
	//TBD();
	interrupts_off();
	if (queue == NULL){ // wait_queue structure isnt initialized
		interrupts_on();
		return THREAD_INVALID;
	}
	else if (t_table->ready_q_first == NULL){ // no other thread is ready
		interrupts_on();
		return THREAD_NONE;
	}

	
	//else if (t_table->current_Tid == 0){
		//unintr_printf("HERERERER \n");
	//	interrupts_on();
	//	return 0;
	//}

	else {

		interrupts_off();
		
		

		//ucontext_t current_context ; // create context variable 

		int running_Tid = t_table->current_Tid;
		//int ret_val = running_Tid;
		struct thread *running_thread = t_table->t_ptrs[running_Tid];
		struct thread *original_thread = running_thread;
		{
			/* data */
		};
		

		// put into waiting thread 

			
			
			


		if (queue->first == NULL){
				queue->first = running_thread;
				queue->last  = running_thread;
		}
		else {
			queue->last->next_wait = running_thread;
			running_thread->prev_wait = queue->last;
			queue->last = running_thread;
		}
		running_thread = t_table->t_ptrs[t_table->current_Tid];
		running_thread->state = 2;
		//t_table->current_Tid = 0;

		// wait check is at one now, so when it wakes up it will have to return a value
		t_table->current_Tid = t_table->ready_q_first->thread_id;
		running_Tid = t_table->current_Tid;
			
		running_thread->thread_yielded_to = running_Tid;

		//swtiching to top ready thread
			
			 // thread that is being put to sleep gets the info on the thread it is 
			//yield to 
			// now yield to thread 
		struct thread *yielded_thread = t_table->t_ptrs[t_table->ready_q_first->thread_id]; // running thread now equals the thread that is being yielded to
		//current_context = running_thread->t_context;
			
			//taking it out of ready queue from the top
		if (t_table->ready_q_first == yielded_thread && t_table->ready_q_last == yielded_thread){ // it its the only element in
			//ready queue
				t_table->ready_q_first = NULL;
				t_table->ready_q_last = NULL;
		}
		else{ // not the only element
			yielded_thread->next_ready->prev_ready = NULL;
			t_table->ready_q_first = yielded_thread->next_ready;
			yielded_thread->next_ready = NULL;
		}

		yielded_thread->state = 0;
			
				
		yielded_thread->next_ready = NULL;
		yielded_thread->prev_ready = NULL;

		

		int err = getcontext(&(queue->last->t_context));
		assert(!err);

		// if (running_thread == NULL){
		// 	check();
		// }
		
		//check_waitq();
		

		//running_thread->t_context = current_context; //save context

		if(original_thread->wait_check == 0 ){
		

			original_thread->wait_check = 1;

			original_thread->thread_yielded_to = yielded_thread->thread_id;
			
			
			// changing context
			
			
			interrupts_off();
			
			int err1 = setcontext(&(yielded_thread->t_context));
			assert(!err1);
		
		}



		original_thread->wait_check = 0;
	
		int retx = original_thread->thread_yielded_to;
		interrupts_on();
		return retx;
	}



	return THREAD_FAILED;
}

// pop waiting queue and put into end of running queue

void pop_wait (struct wait_queue * queue){

	interrupts_off();
	// take first thread from waiting into running
	struct thread *w_r = queue->first;
	
	
	if (queue->first == w_r && queue->last == w_r){
		queue->first = NULL;
		queue->last = NULL;
	}
	else {
		w_r->next_wait->prev_wait = NULL;
		queue->first = w_r->next_wait;
		w_r->next_wait = NULL;
	}

	
	
	
	
	 w_r->state = 1;
	// putting w_r into bottom of running queue
	if (t_table->ready_q_first == NULL){
		t_table->ready_q_first = w_r;
		t_table->ready_q_last = w_r;
	}
	else{
		w_r->prev_ready = t_table->ready_q_last;
		t_table->ready_q_last->next_ready = w_r;
		t_table->ready_q_last = w_r;
		w_r->next_ready = NULL;
	}

	





}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup_(struct wait_queue *queue, int all){
	return 0;
}
int
thread_wakeup(struct wait_queue *queue, int all)
{
	//TBD();
	
	interrupts_off();
	
	if (queue == NULL){
		interrupts_on();
		return 0;
	}

	else if (queue->first == NULL){
		interrupts_on();
		return 0;
	}
	else if (all == 0){
	// take first thread from waiting into running
		interrupts_off();
		pop_wait(queue);
		interrupts_on();
		return 1;
	}
	else if (all == 1){
		interrupts_off();
		int count = 0;
		

		while (queue->first != NULL){
			pop_wait(queue);
			count++;
			
		}

		
		
		interrupts_on();
		return count ;


	}

	interrupts_on();
	return 0;
}

/* suspend current thread until Thread tid exits */
Tid
thread_wait(Tid tid)
{
	//TBD();

	// 1) put in tid's blocking queue
	// 2) switch context to top ready queue
	interrupts_off();

	if (tid == t_table->current_Tid || tid == THREAD_SELF){ // current thread cant block itself
		interrupts_on();
		return THREAD_INVALID; // 
	}
	else if (t_table->thread_list[tid] == 0){ // blocking thread doesnt exist
		interrupts_on();
		return THREAD_INVALID;
	}

	else {
		struct thread *blocking_thread = t_table->t_ptrs[tid];
		struct thread *current_running_thread = t_table->t_ptrs[t_table->current_Tid];
		struct thread *yielded_thread = t_table->t_ptrs[t_table->ready_q_first->thread_id];

		//1 put in tids blocking queue

		if (blocking_thread->blocking_q_first == NULL) // empty blocking queue
		{
			blocking_thread->blocking_q_first = current_running_thread;
			blocking_thread->blocking_q_last = current_running_thread;
		}
		else{ // non empty blocking queue

			blocking_thread->blocking_q_last->next_block = current_running_thread;
			blocking_thread->blocking_q_last = current_running_thread;

		}

		current_running_thread->state = 2; // basically waiting

		//2 switch context to top ready queue
		// take it out of ready queue
		if (t_table->ready_q_first == yielded_thread && t_table->ready_q_last == yielded_thread){ // it its the only element in
			//ready queue
				t_table->ready_q_first = NULL;
				t_table->ready_q_last = NULL;
		}
		else{ // not the only element
			yielded_thread->next_ready->prev_ready = NULL;
			t_table->ready_q_first = yielded_thread->next_ready;
			yielded_thread->next_ready = NULL;
		}

		yielded_thread->state = 0; // now running
		current_running_thread->got_blocked_by = blocking_thread->thread_id;
		t_table->current_Tid = yielded_thread->thread_id;
		blocking_thread->block_list[current_running_thread->thread_id] = 1;
		int err = getcontext(&(current_running_thread->t_context));
		assert(!err);

		

		if (current_running_thread->block_check == 0){
			
			current_running_thread->block_check = 1;

			interrupts_off();
			int err1 = setcontext(&(yielded_thread->t_context));
			assert(!err1);
		}

		int ret_x = current_running_thread->got_blocked_by;
		current_running_thread->got_blocked_by = -1; //not blocked by anyone
		current_running_thread->block_check = 0;

		interrupts_on();
		return ret_x;





	}





	return 0;
}

struct lock {
	/* ... Fill this in ... */
	struct wait_queue *lock_q;
	int is_locked;
};

struct lock *
lock_create()
{	
	interrupts_off();
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);

	lock->lock_q = wait_queue_create();
	lock->is_locked = 0;

	//TBD();
	interrupts_on();
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	interrupts_off();
	assert(lock != NULL);

	if( lock ->is_locked == 0){
		wait_queue_destroy(lock->lock_q);
		free(lock);
	}

	//TBD();

	//free(lock);
	interrupts_on();
}

void
lock_acquire(struct lock *lock)
{	interrupts_off();
	assert(lock != NULL);

	while (lock->is_locked == 1){
		thread_sleep(lock->lock_q);
		interrupts_off();
	}

	lock->is_locked = 1;
	t_table->t_ptrs[t_table->current_Tid]->has_lock = 1;
	t_table->lock_with_thread = t_table->current_Tid;
	//TBD();
	interrupts_on();
}

void
lock_release(struct lock *lock)
{
	assert(lock != NULL);
	interrupts_off();

	if (lock->is_locked == 1 && t_table->t_ptrs[t_table->current_Tid]->has_lock == 1){
		lock->is_locked = 0;
		t_table->t_ptrs[t_table->current_Tid]->has_lock = 0;
		t_table->lock_with_thread = -1;
		thread_wakeup(lock->lock_q, 1);
		interrupts_off();
	}

	interrupts_on();

	//TBD();
}

struct cv {
	/* ... Fill this in ... */
	struct wait_queue *cv_q;
};

struct cv *
cv_create()
{
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);

	cv->cv_q = wait_queue_create();

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);
	interrupts_off();

	//TBD();
	wait_queue_destroy(cv->cv_q);
	free(cv);
	interrupts_on();
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);
	interrupts_off();
	if (t_table->t_ptrs[t_table->current_Tid]-> has_lock == 1){
		lock_release(lock);
		thread_sleep(cv->cv_q);

	}

	lock_acquire(lock);
	
	interrupts_on();

	//TBD();
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);
	interrupts_off();

	if(t_table->t_ptrs[t_table->current_Tid]-> has_lock == 1){
		thread_wakeup(cv->cv_q, 0);
	}

	interrupts_on();

	//TBD();
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);
	interrupts_off();

	if(t_table->t_ptrs[t_table->current_Tid]-> has_lock == 1){
		thread_wakeup(cv->cv_q, 1);
	}

	interrupts_on();


	//TBD();
}
