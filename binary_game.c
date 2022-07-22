
#include <stdio.h>
#include "includes.h"
#include "os_cfg.h"
#include "stdio.h"
#include "stdlib.h"
#include "globals.h"
#include "pointer.h"
#include "priority.h"

#define TASK_STK_SIZE   1024



OS_STK  mainTask_stk[TASK_STK_SIZE];

OS_STK  countTask_stk[TASK_STK_SIZE];

OS_STK  machineStateTask_stk[TASK_STK_SIZE];

OS_STK  display_task_HEX_task_stk[TASK_STK_SIZE];

OS_STK  display_LEDR_task_stk[TASK_STK_SIZE];

OS_STK  button_press_task_stk[TASK_STK_SIZE];


// define pointer




void mainTask(void *pdata);

void countTask(void *pdata);

void machine_state_task(void* pdata);

void display_task_HEX_task(void *pdata);

void display_LEDR_task(void *pdata);

void button_press_task(void* pdata);

// small function
int display_hex(int number);
void check_power_flag(void);
void power_off(void);
void check_answer_resume_task1(void);
void check_answer_add_point(void);
void check_end_game(void);
void count_elapse(void);


// define priority

OS_EVENT *dispSem;  //semaphore

//commbox

OS_EVENT *BPMBox;
INT8U commRxBuf[100];

INT8U err;
INT8U mailBoxErr;
int * statemsg;
//global variables
int count = COUNT_NUMBER;
int question;
int number_of_question;
int answer;
int point = INIT_SCORE;
int state = OFF;
int elapse_time = ELAPSE_TIME;
int power_flag = 0;
int hex_display[]=   {0x3F, 0x6, 0x5B, 0x4F, 0x66, 0x6D, 0x7C, 0x7, 0x7F, 0x67}; // 0-9

int display_hex(int number){
	return hex_display[number];
}

void power_off(){
	    if(power_flag){
	    	state = IDLE;
	    }
}
void check_power_flag(){

	if ( SW_pointer >= 0x20000){
		power_flag = 1;
	}
	else {
		power_flag = 0;
		state = OFF;
	}
}

void check_answer_resume_task1(){

	if ((question == answer) || (count == 0)){
		count = COUNT_NUMBER;
		number_of_question = number_of_question + 1;
		OSTaskResume(1);
	}


}

void reset(){
	count = COUNT_NUMBER;
	elapse_time = ELAPSE_TIME;
	answer = 0;
	point =INIT_SCORE;
	number_of_question = 0;

}

void check_answer_add_point(){
	if (question == answer)
		point = point +1;
}

void check_end_game(){
	if (number_of_question == 10){
		state = FINISH;
	}
}

void submit_answer(){
	answer = (SW_pointer & 0xFF);
}
void count_elapse(){
	count--;
	elapse_time++;
	if (elapse_time == 6000){
		elapse_time = ELAPSE_TIME;
	}
}

int main(void)

{

	dispSem = OSSemCreate(1);			 // create semaphore
	BPMBox = OSMboxCreate((void *)0);  // create mailbox
    OSTaskCreate(mainTask, (void *)0, &mainTask_stk[TASK_STK_SIZE-1], 1);

    OSStart();
    return 0;

}

void mainTask(void *pdata)
{
    OSTaskCreate(countTask, (void *)0, &countTask_stk[TASK_STK_SIZE-1], Prio2);

    OSTaskCreate(machine_state_task, (void *)0, &machineStateTask_stk[TASK_STK_SIZE-1], Prio3);

    OSTaskCreate(display_LEDR_task, (void *)0, &display_LEDR_task_stk[TASK_STK_SIZE-1], Prio11);

    OSTaskCreate(display_task_HEX_task, (void *)0, &display_task_HEX_task_stk[TASK_STK_SIZE-1], Prio12);

    OSTaskCreate(button_press_task, (void *)0, &button_press_task_stk[TASK_STK_SIZE-1], Prio13);

    while (1)
    	{
			printf("Main Task run\n");


			OSSemPend(dispSem, 0 , &err);
			question = rand() & 0xFF;
			OSSemPost(dispSem);
			OSTaskSuspend(OS_PRIO_SELF);

    }

}

/**
 * count Task
 */
void countTask(void *pdata)
{

	while(1)
	{
	      //test message
							   //test pend mailbox
		statemsg = (int *)OSMboxPend(BPMBox,10 ,&mailBoxErr);
		if (mailBoxErr == OS_NO_ERR){
			OSSemPend(dispSem, 0 , &err);
			state = *statemsg;
			OSSemPost(dispSem);
		}
		else{
			printf("there is some err in count task \n");
		}

			switch(state){
				case IDLE:
					OSSemPend(dispSem, 0 , &err);
					reset();
					OSSemPost(dispSem);
					break;
				case COUNT:
					OSSemPend(dispSem, 0 , &err);
					count_elapse();
					check_answer_add_point();
					check_end_game();
					check_answer_resume_task1();
					OSSemPost(dispSem);
					break;

				default:
					break;
				}

			// uncommend this one to do testing
//				OSTimeDly(1); // debug mode make it run faster

		 // uncommend this one to run on real environment
				OSTimeDlyHMSM(0, 0, 1,0);
	}

}

/**
 * State machine task . This task run state machine
 */
void machine_state_task(void* pdata)
{

	while(1)
	{
		      //test message

		statemsg = (int *)OSMboxPend(BPMBox,10 ,&mailBoxErr);
		if (mailBoxErr == OS_NO_ERR){
			OSSemPend(dispSem, 0 , &err);
			state = *statemsg;
			OSSemPost(dispSem);
		}
		else{
			printf("there is some err in machine state task \n");
		}
		switch(state){
			case OFF:
				OSSemPend(dispSem, 0 , &err);
				check_power_flag();
				power_off();
				OSSemPost(dispSem);
				break;
			default:
				OSSemPend(dispSem, 0 , &err);
				check_power_flag();
				OSSemPost(dispSem);
				break;
		}
		OSTimeDly(1);

	}
}




void display_LEDR_task(void *pdata)
{

	while(1)
	{
	      //test message

		statemsg = (int *)OSMboxPend(BPMBox,10 ,&mailBoxErr);
		if (mailBoxErr == OS_NO_ERR){

			OSSemPend(dispSem, 0 , &err);
			state = *statemsg;
			OSSemPost(dispSem);
		}
		else{
			printf("there is some err in display LEDR task \n");
		}
        switch (state){
        	case OFF:
        		OSSemPend(dispSem, 0 , &err);
        		SW_pointer &= 0x00;
        		LEDR_pointer &= 0x00;
        		OSSemPost(dispSem);
        		break;
        	case IDLE:
        		OSSemPend(dispSem, 0 , &err);
        		SW_pointer &= 0x00;
        		LEDR_pointer &= 0x00;
        		LEDR_pointer |= (SW_pointer & 0x20000);
        		OSSemPost(dispSem);
        		break;
        	case COUNT:
        		OSSemPend(dispSem, 0 , &err);
        		SW_pointer &= (0x20000 & SW_pointer); // clear the switch
        		LEDR_pointer &= (0x20000 & SW_pointer); // clear the led
        		if(SW_pointer & (1 << 16)){
        			LEDR_pointer |= (question & 0xFF ) | (SW_pointer & 0x30000);
        		}
        		OSSemPost(dispSem);
        		break;
        	case PAUSE:
        		OSSemPend(dispSem, 0 , &err);
        		SW_pointer &= 0x00;
        		LEDR_pointer &= 0x00;
        		LEDR_pointer |= (0x30000 & SW_pointer); // clear the led
        		OSSemPost(dispSem);
        		break;
        	default:
        		SW_pointer &= 0x00;
        		LEDR_pointer &= 0x00;
        		LEDR_pointer = (SW_pointer & 0x20000);
        		OSSemPost(dispSem);
        		break;
        }


        OSTimeDlyHMSM(0, 0, 0, 100);
    }

}



void display_task_HEX_task(void *pdata)
{

	while(1)
	{
	      //test message
							   //test pend mailbox
		statemsg = (int *)OSMboxPend(BPMBox,10 ,&mailBoxErr);
		if (mailBoxErr == OS_NO_ERR){

			OSSemPend(dispSem, 0 , &err);
			state = *statemsg;
			OSSemPost(dispSem);
		}
		else{
			printf("there is some err hex task display\n");
		}




		switch(state){
						case OFF:
							OSSemPend(dispSem, 0 , &err);
							HEX3_HEX0_pointer &= 0x00;
							HEX7_HEX4_pointer &= 0x00;
							OSSemPost(dispSem);
							break;
						case IDLE:
							OSSemPend(dispSem, 0 , &err);
							HEX3_HEX0_pointer &= 0x00;
							HEX7_HEX4_pointer &= 0x00;
							HEX3_HEX0_pointer |= (display_hex(question%10) & 0xFF) | ((display_hex((question/10)%10) << 8) & 0xFF00) | ((display_hex(question/100) << 16) & 0xFF0000);
							HEX7_HEX4_pointer = display_hex(count%10) | (display_hex(count/10) << 8)| (display_hex(point%10)<< 16 )| (display_hex(point/10) << 24);
							OSSemPost(dispSem);
							break;
						case COUNT:
							OSSemPend(dispSem, 0 , &err);
							HEX3_HEX0_pointer &= 0x00;
							HEX7_HEX4_pointer &= 0x00;
							HEX3_HEX0_pointer |= (display_hex(question%10) & 0xFF) | ((display_hex((question/10)%10) << 8) & 0xFF00) | ((display_hex(question/100) << 16) & 0xFF0000);
							HEX7_HEX4_pointer = display_hex(count%10) | (display_hex(count/10) << 8)| (display_hex(point%10)<< 16 )| (display_hex(point/10) << 24);
							OSSemPost(dispSem);
							break;
						case PAUSE:
							OSSemPend(dispSem, 0 , &err);
							HEX3_HEX0_pointer &= 0x00;
							HEX7_HEX4_pointer &= 0x00;
							HEX7_HEX4_pointer = display_hex(count%10) | (display_hex(count/10) << 8)| (display_hex(point%10)<< 16 )| (display_hex(point/10) << 24);
							OSSemPost(dispSem);
							break;
						case FINISH:
							OSSemPend(dispSem, 0 , &err);
							HEX3_HEX0_pointer &= 0x00;
							HEX7_HEX4_pointer &= 0x00;
							HEX3_HEX0_pointer = (display_hex(elapse_time%10) & (0xFF)) | ((display_hex((elapse_time%60)/10)<< 8)& 0xFF00) | ((display_hex((elapse_time/60)%10)<< 16) & 0xFF0000) | ((display_hex(elapse_time/600)<< 24)& 0xFF000000);
							HEX7_HEX4_pointer = (display_hex(point%10)<< 16 )| (display_hex(point/10) << 24);
							OSSemPost(dispSem);
							break;
						default:
							OSSemPend(dispSem, 0 , &err);
							HEX3_HEX0_pointer &= 0x00;
							HEX7_HEX4_pointer &= 0x00;
							HEX3_HEX0_pointer |= (display_hex(question%10) & 0xFF) | ((display_hex((question/10)%10) << 8) & 0xFF00) | ((display_hex(question/100) << 16) & 0xFF0000);
							HEX7_HEX4_pointer = display_hex(count%10) | (display_hex(count/10) << 8);
							OSSemPost(dispSem);
							break;
		}
		OSTaskSuspend(OS_PRIO_SELF); // 12
	}

}



void button_press_task(void* pdata)
{

	while (1){
//		printf("button press task running\n");
		LEDG_pointer &= 0x00; // clear LEDG
		OSSemPend(dispSem, 0 , &err);
		switch(state){
			case IDLE:

				if(KEY_pointer & 0x02){

					LEDG_pointer = KEY_pointer;
					state = COUNT;
				}
//				OSSemPost(dispSem);
				break;
			case COUNT:

				if(KEY_pointer & 0x02){

					LEDG_pointer = KEY_pointer;
					state = PAUSE;
				}

				if (KEY_pointer & 0x04){

					LEDG_pointer = KEY_pointer;
					submit_answer();
				}
//				OSSemPost(dispSem);
				break;
			case PAUSE:

				if(KEY_pointer & 0x02){

					LEDG_pointer = KEY_pointer;
					state = COUNT;
				}
				if (KEY_pointer & 0x01){
					LEDG_pointer = KEY_pointer;
					state = IDLE;
				}

				break;
			default:
				break;

			}
		mailBoxErr = OSMboxPostOpt(BPMBox, (void*)&state,OS_POST_OPT_BROADCAST);
		switch (mailBoxErr){
			case OS_NO_ERR:
				// release key and send message
				OSSemPost(dispSem);
				break;
			case OS_MBOX_FULL:
				printf("mail box is full full\n");
				break;
			}
		OSTaskResume(12); // resume display_task_HEX_task
	}
}





