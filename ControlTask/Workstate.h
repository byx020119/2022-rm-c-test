//�Ժ�

#ifndef WorkState_H
#define WorkState_H
#include <stm32f4xx.h>

typedef enum
{
    PREPARE_STATE,     		      //�ϵ���ʼ��״̬ 4s������
    Freedom_STATE,					    //�����˶�״̬����Ѳ��״̬
	  Waiting_STATE,              //�ȴ�״̬
	  Patrol_STATE,               //��Ѳ��״̬
		ChariotRecognition_STATE,   //ս��ʶ��״̬
	  Dodeg_STATE,                //���״̬
	  Test_STATE,                 //ң��������״̬
	  STOP_STATE,        		      //ֹͣ״̬
	  Attacked_STATE,             //������״̬
}WorkState_e;


void WorkStateFSM(void);
void WorkStateSwitchProcess(void);
void SetWorkState(WorkState_e state);
WorkState_e GetWorkState(void);
WorkState_e GetlastWorkState(void);

extern uint8_t CameraDetectTarget_Flag;
extern uint8_t DodgeTarget_Flag;
extern uint8_t RemoteTest_Flag;
extern uint8_t Waiting_Flag;
extern uint8_t Chassis_Power_On_Flag;
extern uint8_t Attacked_Flag;
extern uint8_t Freedom_Flag;
extern uint32_t Dodge_time_count;

#endif
