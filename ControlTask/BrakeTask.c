#include "BrakeTask.h"
#include "Phototube.h"
#include "SpeedTask.h"
#include "main.h"



/***
���ܣ�ɲ�������ֵ��������״̬�����̵��״̬���޸�ɲ�����λ�ñ�־
��ע����
***/
void Brake_Handle()
{
	//���ܼ��ɲ��
	if(Phototube_brake==1)//����⻻��
	{
		Brake_flag=-1;
		Phototube_brake=0;//���ܼ��ɲ����־����
		brake_limt.time_now = Get_Time_Micros();//TIM2->CNT
	  brake_limt.time_error = brake_limt.time_now - brake_limt.time_last;
		if(brake_limt.time_error>1000000)
		{
			Brake_flag=0;
			brake_limt.time_last=Get_Time_Micros();
		}
		
	}
	if(Phototube_brake==1)//�Ҳ��⻻��
	{
		Brake_flag=1;
		Phototube_brake=0;
		brake_limt.time_now = Get_Time_Micros();//TIM2->CNT
	  brake_limt.time_error = brake_limt.time_now - brake_limt.time_last;
		if(brake_limt.time_error>1000000)
		{
			Brake_flag=0;
			brake_limt.time_last=Get_Time_Micros();
		}
		
	}
}
