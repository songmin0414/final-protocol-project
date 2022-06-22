#include "L3_FSMevent.h"
#include "L3_msg.h"
#include "L3_timer.h"
#include "L3_LLinterface.h"
#include "protocol_parameters.h"
#include "mbed.h"


//FSM state -------------------------------------------------
#define L3STATE_IDLE                0


//state variables
static uint8_t main_state = L3STATE_IDLE; //protocol state
static uint8_t prev_state = main_state;

//SDU (input)
static uint8_t originalWord[200];
static uint8_t wordLen=0;

static uint8_t sdu[200];

//serial port interface
static Serial pc(USBTX, USBRX);


//application event handler : generating SDU from keyboard input
static void L3service_processInputWord(void)
{
    char c = pc.getc();
    if (!L3_event_checkEventFlag(L3_event_dataToSend))
    {
        if (c == '\n' || c == '\r')
        {
            originalWord[wordLen++] = '\0';
            L3_event_setEventFlag(L3_event_dataToSend);
            debug_if(DBGMSG_L3,"word is ready! ::: %s\n", originalWord);
        }
        else
        {
            originalWord[wordLen++] = c;
            if (wordLen >= L3_MAXDATASIZE-1)
            {
                originalWord[wordLen++] = '\0';
                L3_event_setEventFlag(L3_event_dataToSend);
                pc.printf("\n max reached! word forced to be ready :::: %s\n", originalWord);
            }
        }
    }
}



void L3_initFSM()
{
    //initialize service layer
    pc.attach(&L3service_processInputWord, Serial::RxIrq);

    pc.printf("Give a word to send : ");
}

void L3_FSMrun(void)
{   
    if (prev_state != main_state)
    {
        debug_if(DBGMSG_L3, "[L3] State transition from %i to %i\n", prev_state, main_state);
        prev_state = main_state;
    }

    //FSM should be implemented here! ---->>>>
    switch (main_state)
    {
        case L3STATE_IDLE:	//IDLE state description
            if (L3_event_checkEventFlag(L3_event_dataToSend)) //time out  + send broadcast msg (data_req)
            {
                if(L3_event_timer() == 0)           
            		{
            		
            			L3_msg_encode(broadMsg);
            			L3_LLI_dataReqFunc(broadMsg, wordLen);
            		}

				// wordLen = 0;
				L3_event_clearEventFlag(L3_event_dataToSend);
            	main_state = L3STATE_IDLE;
            	}
			else if (L3_event_checkEventFlag(L3_event_msgRcvd)) // if indicate req_msg, send resp msg + print
				{
                    //get req_msg
				uint8_t* dataPtr = L3_LLI_getMsgPtr();
                uint8_t size = L3_LLI_getSize();
                }
                else if (L3_event_checkEventFlag(L3_event_dataTosend)) //send resp msg
                {
                     
				    L3_LLI_dataReqFunc(RespMsg, wordLen);
           		    pc.printf("broadcast msg send succes \n");
				    main_state = L3STATE_IDLE; 
				}
            break;
    }
}