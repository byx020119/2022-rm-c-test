#include "BrakeTask.h"
#include "Phototube.h"
#include "SpeedTask.h"
#include "main.h"

int change_brake=0;

/***
���ܣ�ɲ�������ֵ�������̵��״̬���޸�ɲ�����λ�ñ�־
��ע����
***/
void Brake_Handle(void)
{
	//ͨ����һ�ε�refֵ������refֵ�ж��Ƿ�ɲ��
	if(RC_CtrlData.rc.s2!=1&&RC_CtrlData.rc.s2!=2)//�����Լ�ֹͣ״̬�²����˺���
		{
			if(((last_Chassis_Temp_Speed<0&&Chassis_Temp_Speed >0)||(last_Chassis_Temp_Speed<0&&Chassis_Temp_Speed ==0))&&change_brake==0)//����⻻��
			{
				Brake_flag=-1;

				change_brake=1;
				
				if(brake_limt.time_error==0)brake_limt.time_last=Get_Time_Micros();		 
		
			}
			if(((last_Chassis_Temp_Speed>0&&Chassis_Temp_Speed <0)||(last_Chassis_Temp_Speed>0&&Chassis_Temp_Speed ==0))&&change_brake==0)//�Ҳ��⻻��
			{
				Brake_flag=1;
			
				change_brake=1;
				
				if(brake_limt.time_error==0)brake_limt.time_last=Get_Time_Micros();
		
			}
			if(change_brake==1)
			{
			 brake_limt.time_now = Get_Time_Micros();//TIM2->CNT
		 	 brake_limt.time_error = brake_limt.time_now - brake_limt.time_last;
			 if(brake_limt.time_error>500000)//����ɲ��ʱ�� ��λus
	   		{
					Brake_flag=0;
					brake_limt.time_error=0;
					change_brake=0;
		  	}
			}
		}

	
}
