#include "DogetState.h"
#include "Freedom_Status.h"
#include "WorkState.h"
#include "SpeedTask.h"
#include "GunHeartAndBleed.h"
#include "JudgingSystemTask.h"

int Doget_Change_Flag=0;             // λ�ÿɱ���״̬��־
int Destination_Flag = 0;            //�Ƿ񵽴����˱�־
int16_t last_Random_MotionRange = 0; //������һ�������λ��
int16_t Random_MotionRange = 0;      //���������λ��
int Doget_twice=0;                   //���ζ��
int Random_change_flag=0;            //��������������仯��־
int Random_change__flag_2=0;            //��������������仯��־
int Doget_rego_flag=0;
int test_ecd=0;


void YawFreeRoation_Doget(void)
{
  if( GetWorkState()==Dodeg_STATE  )
	{
		
		if(Last_CameraDetectTarget_Flag==1&&CameraDetectTarget_Flag==0)
		{
		   YawCurrentPositionSave = -GMYawEncoder.ecd_angle;           //���浱ǰyaw����ֵ
		   GimbalRef.yaw_angle_dynamic_ref = YawCurrentPositionSave;
		}
		
		if(GMPitchEncoder.ecd_angle-20>0)
		{
		pitch_rotate_flag=1;
		}
		if(GMPitchEncoder.ecd_angle+20<0)
		{
		pitch_rotate_flag=0;
		}
		if(pitch_rotate_flag==1)
		{
	  GimbalRef.pitch_angle_dynamic_ref=GimbalRef.pitch_angle_dynamic_ref + 0.05f;
		}
		if(pitch_rotate_flag==0)
		{
	  GimbalRef.pitch_angle_dynamic_ref=GimbalRef.pitch_angle_dynamic_ref - 0.05f;
		}
	
		//�̶�����ͷδʶ��ʱ����̨����ʽ��ת
		if(Freedom_Rotation_flag==0)
		{
			if(-GMYawEncoder.ecd_angle-0>0)
			{
			Yaw_rotate_flag=1;
			}
			if(-GMYawEncoder.ecd_angle+200<0)
			{
			Yaw_rotate_flag=0;
			}
			if(Yaw_rotate_flag==1)
			{
			GimbalRef.yaw_angle_dynamic_ref=GimbalRef.yaw_angle_dynamic_ref - 0.5f;
			}
			if(Yaw_rotate_flag==0)
			{
			GimbalRef.yaw_angle_dynamic_ref=GimbalRef.yaw_angle_dynamic_ref + 0.5f;
			}
   	}
		
		
		if(GM_Rotete_flag_Before=='X'||GM_Rotete_flag_Behind=='X')
		{
			Recognition_Delay_count++;
			if(Recognition_Delay_count>3000)
			{
			   Freedom_Rotation_flag=1; //ʶ��һ��ʱ��󣬹̶�����ͷʶ��
			}
		}
		
   
	 //�̶�����ͷʶ�𵽵з�װ�װ壬����̨��ת��ȥ
	if(Freedom_Rotation_flag==1)
	{
		Yaw_encoder = -GMYawEncoder.ecd_angle-Yaw_encoder_s;
		YAW_Round_Cnt   = (int)(Yaw_encoder)/1852;
		YAW_Angle_YuShu = (int)(Yaw_encoder)%1852;
		if(YAW_Angle_YuShu>=0)
		{
			if(GM_Rotete_flag_Before=='X') //ǰ�̶�����ͷʶ��Ŀ��
			{
				if(YAW_Angle_YuShu<=703)        //926+223=1249   926-223=703  1852-223=1629  1852-446=1406 926-446=480 1852+446=2298 1249+223=1472
					{
						Recognition_YAW_Pos_Ref = YAW_Round_Cnt*1852 -446;
					}
				if(YAW_Angle_YuShu>703)
				  {
				    Recognition_YAW_Pos_Ref = YAW_Round_Cnt*1852 +1852;
				  }

			}
			else if(GM_Rotete_flag_Behind=='X') //��̶�����ͷʶ��Ŀ��
			{
	  		if(YAW_Angle_YuShu<=703)       
					{
						Recognition_YAW_Pos_Ref = YAW_Round_Cnt*1852 +926;
					}
				if(YAW_Angle_YuShu>703)
				  {
				    Recognition_YAW_Pos_Ref = YAW_Round_Cnt*1852 +480;
				  }
			}
		}
		else
		{
			if(GM_Rotete_flag_Before=='X') //ǰ�̶�����ͷʶ��Ŀ��
			{
	  		if(YAW_Angle_YuShu>=-1249)
					{
						Recognition_YAW_Pos_Ref = YAW_Round_Cnt*1852 +0;
					}
				if(YAW_Angle_YuShu<-1249)
				 {
				   Recognition_YAW_Pos_Ref = YAW_Round_Cnt*1852 -2298;
				 }

	  	}
			else if(GM_Rotete_flag_Behind=='X') //��̶�����ͷʶ��Ŀ��
				{
					if(YAW_Angle_YuShu>=-1249)
					{
						Recognition_YAW_Pos_Ref = YAW_Round_Cnt*1852 -1472;
					}
				if(YAW_Angle_YuShu<-1249)
				 {
				   Recognition_YAW_Pos_Ref = YAW_Round_Cnt*1852 -926;
				 }
				}
		}
		
//������
		if(Recognition_YAW_Pos_Ref+Yaw_encoder_s - (-GMYawEncoder.ecd_angle)>=0)
			{
				Recognition_YAW_Rotation_Dir = 1;   //��λ�û��ο�ֵ����ķ���ת
			}
		else
			{
				Recognition_YAW_Rotation_Dir = 2;  //��λ�û��ο�ֵ��С�ķ���ת
			}
			
			
		if(Recognition_YAW_Rotation_Dir == 1)
			{
				GimbalRef.yaw_angle_dynamic_ref=GimbalRef.yaw_angle_dynamic_ref+0.8f;//0.4f
				if(GimbalRef.yaw_angle_dynamic_ref - Recognition_YAW_Pos_Ref>0)
				{
					GimbalRef.yaw_angle_dynamic_ref = Recognition_YAW_Pos_Ref;
					Recognition_YAW_Rotation_Dir = 0;
					Freedom_Rotation_flag=0;
					Recognition_Delay_count=0;
				}
			}
		else if(Recognition_YAW_Rotation_Dir == 2)
			{
				GimbalRef.yaw_angle_dynamic_ref=GimbalRef.yaw_angle_dynamic_ref-0.8f;//0.1f
				if(GimbalRef.yaw_angle_dynamic_ref - Recognition_YAW_Pos_Ref<0)
				{
					GimbalRef.yaw_angle_dynamic_ref = Recognition_YAW_Pos_Ref;
					Recognition_YAW_Rotation_Dir = 0;
					Freedom_Rotation_flag=0;
					Recognition_Delay_count=0;
				}
			}
//		else if(Recognition_YAW_Rotation_Dir == 0)  //�ο�ֵ����Ŀ��λ�ú󣬵ȴ������λ
//			{
//				if(abs(GimbalRef.yaw_angle_dynamic_ref - (-GMYawEncoder.ecd_angle))<1)
//				{
//					Freedom_Flag = 1;
//				}
//			}
	  
		//�����̨�ڰ���̶�����ͷ�Ĺ�����ʶ��Ŀ�꣬��ô���̽����ù��̣��ȴ�����ʶ��ģʽ
		if(CameraDetectTarget_Flag ==1)
		{
			Recognition_YAW_Rotation_Dir = 0;
			Freedom_Rotation_flag=0;
		}
	}
	
	}
}



void Chassis_Motion_Switch_Doget(void)
{
	//������������ʱ�Ķ��
if(GetWorkState()== Dodeg_STATE && Aerocraft_attack_flag==1)
  {
		if(Aerocraft_Phototube_flag==1)
		  {
	    	Chassis_Position_Ref = -10000; 
	  	}
		if(Aerocraft_Phototube_flag==0)
		{
			//�������ܽ��л���
				if(Dir_Change_Flag==1)
			{
			   switch(Chassis_Freedom_i)
				{
					case 0:
					{
						Chassis_Position_Ref = 5000;
						Dir_Change_Flag=0;
					};break;
					case 1:
					{
						Chassis_Position_Ref = -10000; 
						Dir_Change_Flag=0;
					};break;
				}
			}
			//�������̽��л���
			test_ecd=abs(Chassis_Position_Ref - CM1Encoder.ecd_angle);
				if(abs(Chassis_Position_Ref - CM1Encoder.ecd_angle)<5)
			{
				switch(Chassis_Freedom_i)
				{
					case 0:
					{
						Chassis_Position_Ref = 5000;
					};break;
					case 1:
					{
						Chassis_Position_Ref = -10000; 
					};break;
				}
				Chassis_Freedom_i++;
				if(Chassis_Freedom_i>=2) Chassis_Freedom_i = 0;
			}
		}
			
	  	if(Chassis_Change_Dir_Flag==0)    //�����˶��ٶ�
			{
			last_Chassis_Temp_Speed = Chassis_Temp_Speed;
		  if(Aerocraft_Phototube_flag==1)
	      	{
		       Chassis_Temp_Speed=-800;
	      	}
			if(Aerocraft_Phototube_flag==0)
	     {
				if(Chassis_Position_Ref <CM1Encoder.ecd_angle)
				{
					Chassis_Temp_Speed = -600;
				}
				else if(Chassis_Position_Ref > CM1Encoder.ecd_angle)
				{	
					Chassis_Temp_Speed = 600;
				}
		 	}
				
				if(last_Chassis_Temp_Speed*Chassis_Temp_Speed<0)    //���̻���
				{
				  Chassis_Change_Dir_Flag = 1;
				}
		  }
  }
	
	//δ������������ʱ�Ķ��
if(GetWorkState()== Dodeg_STATE && Aerocraft_attack_flag==0)   
	{
		//��Ӣ�۴��蹥����������״̬
		
		if(Amor_ID==0&&Doget_Change_Flag==1)
		{
		  Chassis_Position_Ref = -10000;     //�����Ҳ����
			Doget_Change_Flag = 0;
			Destination_Flag = 0;
//			Attacked_Flag=0;
		}
		
		if(Amor_ID==1&&Doget_Change_Flag==1) 
		{
		  Chassis_Position_Ref = 10000;       //����������
			Doget_Change_Flag = 0;
			Destination_Flag = 0;
//			Attacked_Flag=0;
		}
	 
			
		//�̶�����ͷ�������
		if(Doget_rego_flag==1)
		{
		  Chassis_Position_Ref = -10000;     //�����Ҳ����
			Doget_Change_Flag = 0;
			Destination_Flag = 0;
			Doget_rego_flag=0;
//			Attacked_Flag=0;
		}
		
		if(Doget_rego_flag==2)
		{
		  Chassis_Position_Ref = 10000;       //����������
			Doget_Change_Flag = 0;
			Destination_Flag = 0;
			Doget_rego_flag=0;
//			Attacked_Flag=0;
		}
		
		
		if(Chassis_Power_On_Flag == 1)  //�����Ѿ��ϵ�
		{
		 if(Destination_Flag==1)         //�Ѿ��ִ�������
		 {
			   //�������ܽ��л���
			 
			if(Dir_Change_Flag==1)                         
			{
			  getRandom_MotionRange();
					switch(Chassis_Freedom_i)
				{
					case 0:
					{
						Chassis_Position_Ref = fabs(Random_MotionRange);
						Dir_Change_Flag=0;
					};break;
					case 1:
					{
						Chassis_Position_Ref = -fabs(Random_MotionRange); 
						Dir_Change_Flag=0;
					};break;
				}		
			}
			//�̶�����ͷ�������ж��
			if(RobotHP>=120&&Big_armor_recognition_flag==1)
				{
				  if(Dodeg_Delay_Count<5000)
			    	{
					   if((Armor_R_Flag_Before=='R'||Armor_R_Flag_Before=='L')&&Destination_Flag==1)   
						 { 
							 switch(Chassis_Freedom_i)
				       {
					       case 0:
				       	{
						      Doget_rego_flag=2;
				       	};break;
					       case 1:
					      {
						      Doget_rego_flag=1;
					      };break;
				       }		
							 
							 Dodeg_Delay_Count=0;
						 }
						 if((Armor_R_Flag_Behind=='R'||Armor_R_Flag_Behind=='L')&&Destination_Flag==1)
						 {
							  switch(Chassis_Freedom_i)
				       {
					       case 0:
				       	{
						      Doget_rego_flag=1;
				       	};break;
					       case 1:
					      {
						      Doget_rego_flag=2;
					      };break;
				       }
							 Dodeg_Delay_Count=0;
						 }
			    	}
				 if(Dodeg_Delay_Count>5000&Dodeg_Delay_Count<16000)
				 {
           if(Armor_R_Flag_Before=='R')         //�൱�ڴ������湥��
						 {
						   Doget_rego_flag=2;
						 }
					if(Armor_R_Flag_Before=='L')         //�൱�ڴ��豳�湥��
						 {
						   Doget_rego_flag=1;             
						 }
					if(Armor_R_Flag_Behind=='R')         //�൱�ڴ��豳�湥��
						 {
						   Doget_rego_flag=1;
						 }
					if(Armor_R_Flag_Behind=='L')         //�൱�ڴ������湥��
						 {
						   Doget_rego_flag=2;             
						 }
				 }
				}
			if(RobotHP<120)//&&Big_armor_recognition_flag==1)
				{
				  if(Dodeg_Delay__Count_2<=100)
			    	{
					   if((Armor_R_Flag_Before=='R'||Armor_R_Flag_Before=='L')&&Destination_Flag==1)   
						 { 
							 switch(Chassis_Freedom_i)
				       {
					       case 0:
				       	{
						      Doget_rego_flag=2;
				       	};break;
					       case 1:
					      {
						      Doget_rego_flag=1;
					      };break;
				       }		
							 
							 Dodeg_Delay_Count=0;
						 }
						 if((Armor_R_Flag_Behind=='R'||Armor_R_Flag_Behind=='L')&&Destination_Flag==1)
						 {
							  switch(Chassis_Freedom_i)
				       {
					       case 0:
				       	{
						      Doget_rego_flag=1;
				       	};break;
					       case 1:
					      {
						      Doget_rego_flag=2;
					      };break;
				       }
							 Dodeg_Delay_Count=0;
						 }
			    	}
				 if(Dodeg_Delay__Count_2>100)
				 {
           if(Armor_R_Flag_Before=='R')         //�൱�ڴ������湥��
						 {
						   Doget_rego_flag=2;
						 }
					if(Armor_R_Flag_Before=='L')         //�൱�ڴ��豳�湥��
						 {
						   Doget_rego_flag=1;             
						 }
					if(Armor_R_Flag_Behind=='R')         //�൱�ڴ��豳�湥��
						 {
						   Doget_rego_flag=1;
						 }
					if(Armor_R_Flag_Behind=='L')         //�൱�ڴ������湥��
						 {
						   Doget_rego_flag=2;             
						 }
				 }
				}
				
			if(abs(Chassis_Position_Ref - CM1Encoder.ecd_angle)<5)    //����Ŀ��λ��
			{
			
				if(RobotHP>=120)
				{
				if(Dodeg_Delay_Count<5000)
				{	
			  	if(Random_change_flag==0)                              //��Ӣ�۹�����������״̬
					{
					getRandom_MotionRange();
		      Chassis_Position_Ref = Random_MotionRange;
					}
					if(Random_change_flag==1)                             //��Ӣ�۵ڶ��ι�����������ζ��״̬
					{
					getRandom_MotionRange();
				  switch(Chassis_Freedom_i)
				   {
				  	case 0:
					  {
					  	Chassis_Position_Ref = fabs(Random_MotionRange);
					  };break;
					  case 1:
				  	{
				  		Chassis_Position_Ref = -fabs(Random_MotionRange); 
				   	};break;
			     }		
					}
				}
				
				
				
//				if(Dodeg_Delay_Count>5000&&Dodeg_Delay_Count<16000)
//				{
//				  switch(Chassis_Freedom_i)
//				   {
//				  	case 0:
//					  {
//					  	Chassis_Position_Ref = 10000;
//					  };break;
//					  case 1:
//				  	{
//				  		Chassis_Position_Ref = -10000; 
//				   	};break;
//			     }		
//				}
				}
				if(RobotHP<120)
				{
					if(Random_change__flag_2==1)                    //������ʱ����ܵ����߶�10s
					{
					getRandom_MotionRange();
		      Chassis_Position_Ref = Random_MotionRange;
					}
					
					if(Random_change__flag_2==0)                    //����ȫ���������20s
					{
			     getRandom_MotionRange();
				   switch(Chassis_Freedom_i)
				   {
				  	case 0:
					  {
					  	Chassis_Position_Ref = fabs(Random_MotionRange);
					  };break;
					  case 1:
				  	{
				  		Chassis_Position_Ref = -fabs(Random_MotionRange); 
				   	};break;
			     }		
          }
				}	
			}
		 }
			if(Chassis_Change_Dir_Flag==0)    //�����˶��ٶ�
			{
				
				last_Chassis_Temp_Speed = Chassis_Temp_Speed;
				
				if(Chassis_Position_Ref <CM1Encoder.ecd_angle)
				{
					if(abs(Chassis_Position_Ref - CM1Encoder.ecd_angle)>500)
					{
						if(RobotHP>=120)
						{
							if(Dodeg_Delay_Count<=5000)
			      	{
				          Chassis_Temp_Speed = -750;//-700;
			      	}
//						  if(Dodeg_Delay_Count>5000&&Dodeg_Delay_Count<16000)
//			      	{
//				          Chassis_Temp_Speed = -600;//-700;
//			      	}
						}
						if(RobotHP<120)
						{
						  Chassis_Temp_Speed = -700;//-700;
						}
					}
					else
					{
						Chassis_Temp_Speed = -600;
					}
				}
				else if(Chassis_Position_Ref > CM1Encoder.ecd_angle)
				{
					if(abs(Chassis_Position_Ref - CM1Encoder.ecd_angle)>500)
					{
						if(RobotHP>=120)
						{
						 if(Dodeg_Delay_Count<=5000)
			      	{
				          Chassis_Temp_Speed = 750;//-700;
			      	}
//						  if(Dodeg_Delay_Count>5000&&Dodeg_Delay_Count<16000)
//			      	{
//				          Chassis_Temp_Speed = 600;//-700;
//			      	}
						}
						if(RobotHP<120)
						{
						  Chassis_Temp_Speed = 700;//-700;
						}
					}
					else
					{
						Chassis_Temp_Speed = 600;
					}
				}
				
				if(fabs(Chassis_Temp_Speed - last_Chassis_Temp_Speed)>150 && last_Chassis_Temp_Speed*Chassis_Temp_Speed>0) //ͬ�ţ�ͬ��仯
				{
					last_Chassis_Temp_Speed_Save = last_Chassis_Temp_Speed;
					Chassis_Temp_Speed_Save      = Chassis_Temp_Speed;
					if(fabs(Chassis_Temp_Speed)>fabs(last_Chassis_Temp_Speed))  //����
					{
						Chassis_Speed_UP_Flag = 1;
						CMRamp.ResetCounter(&CMRamp);  //б������
					}
					else if(fabs(Chassis_Temp_Speed)<fabs(last_Chassis_Temp_Speed))
					{
						Chassis_Speed_DOWN_Flag = 1;
						CMRamp.ResetCounter(&CMRamp);  //б������
					}
				}
			
				if(Chassis_Speed_UP_Flag == 1)
				{
					Chassis_Temp_Speed = last_Chassis_Temp_Speed_Save +(Chassis_Temp_Speed_Save - last_Chassis_Temp_Speed_Save)*CMRamp.Calc(&CMRamp);
					if(CMRamp.Calc(&CMRamp) == 1)  Chassis_Speed_UP_Flag = 0;
				}
				else if(Chassis_Speed_DOWN_Flag == 1)
				{
					Chassis_Temp_Speed = last_Chassis_Temp_Speed_Save + (Chassis_Temp_Speed_Save - last_Chassis_Temp_Speed_Save)*CMRamp.Calc(&CMRamp);
					if(CMRamp.Calc(&CMRamp) == 1)  Chassis_Speed_DOWN_Flag = 0;
				}
				
				if(last_Chassis_Temp_Speed*Chassis_Temp_Speed<0)    //���̻���
				{
				  Chassis_Change_Dir_Flag = 1;
				}
		  }
	  }
	}
}


void getRandom_MotionRange(void)
{
	if(RobotHP>=120)
	{
	  last_Random_MotionRange = Random_MotionRange;//�����ϴβ����������
		
	  if(Random_change_flag==0)                //��Ӣ�۹�����������״̬
	    {
	      Random_MotionRange = rand()%2500;//1000~2500
	
	      if(fabs(Random_MotionRange)<1000)
	        {
	          Random_MotionRange = Random_MotionRange+Random_MotionRange/fabs(Random_MotionRange)*1000;//>1000
	        }
       }
	
	   if(Random_change_flag==1)              //��Ӣ�۵ڶ��ι�����������ζ��״̬
    	{
	      Random_MotionRange = rand()%3000+3500; //3500~6500//past-1000~past+1000֮��
	      if(last_Random_MotionRange-Random_MotionRange<1000)
	        {
	          Random_MotionRange = Random_MotionRange-1000;
	        }
		
		    if(Random_MotionRange-last_Random_MotionRange<1000)
	        {
	          Random_MotionRange = Random_MotionRange+1000;
	        }
      }
  }
	if(RobotHP<120)
	{
		
		if(Random_change__flag_2==0)            //����ȫ���������20s
	  	{
	      Random_MotionRange = rand()%8000;//2000~8000
	      if(fabs(Random_MotionRange)<2000)
	        {
	          Random_MotionRange = Random_MotionRange+Random_MotionRange/fabs(Random_MotionRange)*2000;
	        }
	    }
		if(Random_change__flag_2==1)          //������ʱ����ܵ����߶�10s 
		  {
	      Random_MotionRange = rand()%2500;//1000~2500
	      if(fabs(Random_MotionRange)<1000)
	      {
	          Random_MotionRange = Random_MotionRange+Random_MotionRange/fabs(Random_MotionRange)*1000;
	      }
	    }
  }
}
