#include "bsp.h"
#include "timer.h"
#include "can1.h"
#include "can2.h"
#include "usart3.h"
#include "Phototube.h"
#include "uart1.h"
#include "usart2.h"
void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�ж����ȼ�����Ϊ2��
	TIM2_Configuration();           //1us��һ��������¼��������ʱ��
	TIM6_Configuration();           //2ms�ж�һ�Σ�ˢ��һ�ο��ƻ�
	CAN1_Configuration();           //���͵�����������������̷���������
	CAN2_Configuration();           //���͵�����������������̷���������
	USART3_Configuration();         //ң������ʼ��
	usart1_Init(115200);            //�������㴫��������19200
//  USART6_Configuration();         //���վ��ᴫ��������
	usart2_Init(115200);            //�������ش���������
//	PWM_Configuration();            //Ħ����
	Phototube_Configuration();
}

