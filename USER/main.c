#include "main.h"

//����git/////////
int main()
{	

 	delay_ms(2000);//��ʱ�ȴ��ϵ����
	BSP_Init();//���ֳ�ʼ��
  engineerpower_Init();//GPIO��ʼ��
	ControtLoopTaskInit();
	TIM6_Start();//ʱ��
	while(1)
	{
//	Sendtosightway(12);	//�Ӿ�
	delay_us(200);
	}
}
