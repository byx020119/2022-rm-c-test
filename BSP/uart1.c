#include "main.h"
#include "ShootingTask.h"
#include "WorkState.h"
#include "uart1.h"
#include "RemoteTask.h"
#include "Freedom_Status.h"
#include "JudgingSystemTask.h"
#include "usart6.h"

/***
���г�������UART1��ӡ���ݣ��Ӿ���ӡ��UART2
***/
//#if 1
//#pragma import(__use_no_semihosting)             
////��׼����Ҫ��֧�ֺ���                 
//struct __FILE 
//{ 
//	int handle; 
//}; 

//FILE __stdout;       
////����_sys_exit()�Ա���ʹ�ð�����ģʽ    
//void _sys_exit(int x) 
//{ 
//	x = x; 
//} 
////�ض���fputc���� 
//int fputc(int ch, FILE *f)
//{ 	
//	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
//	USART3->DR = (u8) ch;      
//	return ch;
//}
//#endif


__align(8) u8 USART1_TX_BUF[USART1_MAX_SEND_LEN]; 	//���ͻ���,���USART1_MAX_SEND_LEN(400)�ֽ�
u8 USART1_RX_BUF[USART1_MAX_RECV_LEN];//���ջ���,���USART_REC_LEN(400)���ֽ�.
u16 USART1_RX_STA=0;						 //����״̬���	



/***
���յ�8���ֽڣ�ǰ����'R','M';֮��Y���8λ��Y���8λ��֮��P���8λ��P���8λ����������ֽڿ���
��Y�ᣬ��P��
***/
uint32_t usart1_this_time_rx_len = 0;                                    //USART3�յ������ݸ���
uint8_t ChariotRecognition_data[2][ChariotRecognition_data_dma_buf_len]; //DMA�����������ݵ�˫��������
int16_t ChariotRecognitionTemp[3];                                       //�����Ƕ����ݵ�����
int16_t ChariotRecognitionDirection[2];                                  //��������ͷ�����Ĵ�װ������
int16_t Chariot_Rec_Dir_rotate[2];                                       //��������ͷ�����Ĵ�װ�׺�Сװ������
float last_ChariotRecognition_pitch = 0.0f;
float ChariotRecognition_pitch = 0.0f;                                   //pitch�Ƕ�ֵ
float last_ChariotRecognition_yaw = 0.0f;
float ChariotRecognition_yaw = 0.0f;                                     //yaw�Ƕ�ֵ
uint16_t last_Target_Distance = 0;
uint16_t Target_Distance = 150;                                       //����ͷ��Ŀ��ľ���
uint16_t Distance_buf[10];
uint8_t Dis_Buf_Index = 0;
uint8_t Pitch_Add_Angle = 0;
int Armor_R_Flag_Before=0;
int Armor_R_Flag_Behind=0;
int GM_Rotete_flag_Before=0;    //ǰ�̶�����ͷʶ��Ŀ��
int GM_Rotete_flag_Behind=0;    //��̶�����ͷʶ��Ŀ��
int Time_count=0;

CRringBuffer_t CR_ringBuffer;

float CR_yaw_Angle[20];
u8 CR_yaw_Angle_Index = 0;
u8 CR_yaw_Angle_CNT   = 0;
int8_t loop_j;

char Sendtosight[12];///���͸��Ӿ�
static int count_Sendtosight=0;
void USART1_DMA_TX_config();
void DMA_TX_cmd(DMA_Stream_TypeDef *DMAx_Streamx,u16 datasize);
int friction_wheel_count = 0;
float kalman_yaw = 0;
float kalman_pitch = 0;
float kalman_yaw_feedforward = 0;
uint8_t update_flag = 1;
uint8_t enter_CNT = 0;
int Last_CameraDetectTarget_Flag=0;
float E_TEST=0;
float E_TEST1=0;
float E_TEST2=0;
float E_TEST3=0;
int camere_count=0;
int colorflag = 0;

double Yawangle = 0.0f;
int Yawsent = 0;
double t =0.0f;
double z =0.0f;
/***
������usart1_Init(bound)
���ܣ�����USART1��������������
��ע��USART1_TX--->PA9
      USART1_RX--->PB7
      �������ӡ����
***/
void usart1_Init(u32 bound)
{
	/* -------------- Enable Module Clock Source ----------------------------*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);	//ʹ��GPIOA\Bʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);//ʹ��DMA2ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1ʱ��
 /* -------------- Configure GPIO & USART3 -------------------------------*/
	{
		GPIO_InitTypeDef gpio;
		USART_InitTypeDef usart;
		//����1��Ӧ���Ÿ���ӳ��
		GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9����ΪUSART1
		GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_USART1); //GPIOB7����ΪUSART1 
		
		//USART1�˿�����
		gpio.GPIO_Pin = GPIO_Pin_9; //GPIOA9
		gpio.GPIO_Mode = GPIO_Mode_AF;//���ù���
		gpio.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
		gpio.GPIO_OType = GPIO_OType_PP; //���츴�����
		gpio.GPIO_PuPd = GPIO_PuPd_UP; //����
		GPIO_Init(GPIOA,&gpio); //��ʼ����PA9
		
		gpio.GPIO_Pin = GPIO_Pin_7;
		gpio.GPIO_Mode = GPIO_Mode_AF;//���ù���
		gpio.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
		gpio.GPIO_OType = GPIO_OType_PP; //���츴�����
		gpio.GPIO_PuPd = GPIO_PuPd_UP; //����
		GPIO_Init(GPIOB,&gpio); //��ʼ��PB7


		//USART1 ��ʼ������
		USART_DeInit(USART1);
		USART_StructInit(&usart);
		usart.USART_BaudRate = bound;//����������
		usart.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
		usart.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
		usart.USART_Parity = USART_Parity_No;//����żУ��λ
		usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
		usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//�շ�ģʽ
		USART_Init(USART1, &usart); //��ʼ������1
		
		USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
		//USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//��������ж�
		USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1
	}
	/* -------------- Configure DMA1_Stream1 --------------------------------*/
	{
		DMA_InitTypeDef dma;
		
		DMA_DeInit(DMA2_Stream5);
		DMA_StructInit(&dma);
		dma.DMA_Channel = DMA_Channel_4;
		dma.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);
		dma.DMA_Memory0BaseAddr = (uint32_t)&ChariotRecognition_data[0][0];        //����DMA���ڴ��Ŀ��λ�ã���DMA����Ҫ��ȡ����д���λ��
		dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
		dma.DMA_BufferSize = sizeof(ChariotRecognition_data)/2;                    //���鳤��
		dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
		dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		dma.DMA_Mode = DMA_Mode_Circular;
		dma.DMA_Priority = DMA_Priority_Medium;
		dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
		dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
		dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_Init(DMA2_Stream5, &dma);
		
		//����Memory1,Memory0�ǵ�һ��ʹ�õ�Memory
		DMA_DoubleBufferModeConfig(DMA2_Stream5, (uint32_t)&ChariotRecognition_data[1][0], DMA_Memory_0);   //first used memory configuration
		DMA_DoubleBufferModeCmd(DMA2_Stream5, ENABLE);
		
		DMA_Cmd(DMA2_Stream5, ENABLE);
	}
	/* -------------- Configure NVIC ----------------------------------------*/
	{
		NVIC_InitTypeDef nvic;
		//����1 NVIC ����
		nvic.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
		nvic.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�0
		nvic.NVIC_IRQChannelSubPriority =2;		//�����ȼ�0
		nvic.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
		NVIC_Init(&nvic);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����
	}
}

////////////////////////usart2 DMA send,
/*1���ͽ���ʹ���˲�ͬ��DMAͨ����������Ҫ�����䣬�ο������ֲᣬ�ҵ������Ӧӳ�����������ͨ��
2�� 
*/

void USART1_DMA_TX_config()
{


}
///////��ͬ������ĳ�ʼ���������������ֻ������Ҫ������ʱ��ŵ���//
void DMA_TX_cmd(DMA_Stream_TypeDef *DMAx_Streamx,u16 datasize)
{

}


void USART1_IRQHandler(void)  	//����1�жϷ������
{
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)      //���յ�����
	{
//		//clear the idle pending flag ������ô�����־
		(void)USART1->SR;
		(void)USART1->DR;

		//Target is Memory0Ŀ�����ڴ�0
		if(DMA_GetCurrentMemoryTarget(DMA2_Stream5) == 0)
		{
			DMA_Cmd(DMA2_Stream5, DISABLE);
			usart1_this_time_rx_len = ChariotRecognition_data_dma_buf_len - DMA_GetCurrDataCounter(DMA2_Stream5);
			DMA2_Stream5->NDTR = (uint16_t)ChariotRecognition_data_dma_buf_len;     //relocate the dma memory pointer to the beginning position��dma�ڴ�ָ���Ƶ�ʼ��λ��
			DMA2_Stream5->CR |= (uint32_t)(DMA_SxCR_CT);                  //enable the current selected memory is Memory 1���õ�ǰѡ���ڴ����ڴ�1
			DMA_Cmd(DMA2_Stream5, ENABLE);
      if(usart1_this_time_rx_len == ChariotRecognition_data_len)
			{
				ChariotRecognition_Mes_Process(ChariotRecognition_data[0]);
			}
		}
		else //Target is Memory1Ŀ�����ڴ�1
		{
			DMA_Cmd(DMA2_Stream5, DISABLE);
			usart1_this_time_rx_len = ChariotRecognition_data_dma_buf_len - DMA_GetCurrDataCounter(DMA2_Stream5);
			DMA2_Stream5->NDTR = (uint16_t)ChariotRecognition_data_dma_buf_len;      //relocate the dma memory pointer to the beginning position��dma�ڴ�ָ���Ƶ�ʼ��λ
			DMA2_Stream5->CR &= ~(uint32_t)(DMA_SxCR_CT);                  //enable the current selected memory is Memory 0���õ�ǰѡ���ڴ����ڴ�1
			DMA_Cmd(DMA2_Stream5, ENABLE);
      if(usart1_this_time_rx_len == ChariotRecognition_data_len)
			{
				ChariotRecognition_Mes_Process(ChariotRecognition_data[1]);
			}
		}
	}
} 


/////////////////////////////////////////
State_distance state_distacne=closedistance;


void Sendtosightway(int value)//���Ӿ��̶�
{
	//����������ֵ��ȡ��С�����һλ����̧��ʮ��������λ�����0-9����һ���ַ���ʽ���ͣ�������ֵ������+48��0��ASCII�룩
	
//	t= (  ( (int)GMYawEncoder.ecd_angle % 360 )+360 ) % 360; //ÿһȦ���������ֵ����֤������
//	int Yawsent =  (int)t % 360 ;
//	
//  int GMYawtemp_Eular=(Yawsent)*10;    //-180����ȷ�ȵ�С�����һλ����10��������λ
//	int GMPitchtemp_Encoder=(GMPitchEncoder.ecd_angle+90)*10;	
	
//  if(robotState.robot_id>=1&&robotState.robot_id<=9)//�жϵз��ҷ� ��1-9Ϊred��101-109Ϊblue
//	{
//	Sendtosight[0]='B';	
//	}
//	if(robotState.robot_id>=101&&robotState.robot_id<=109)
//	{
//	Sendtosight[0]='R';	
//	}
	
//	colorflag=PCin(6);
////û�е�ʱ�����ÿ���
//	if( PCin(6) == 1 )
//	{
//		Sendtosight[0] ='B';	
//	}
//	if( PCin(6) == 0)
//	{
//		Sendtosight[0]='R';	
//	}
	int GMYawtemp_Eular=(Angles+180)*10;
	int GMPitchtemp_Encoder=(Eular[0]+90)*10;

	Sendtosight[0]='R';	
	
	Sendtosight[1]='M';//��ʼ����
	Sendtosight[2]='N';
	Sendtosight[3]='L';
	
	Sendtosight[4]=(uint8_t)(GMYawtemp_Eular/1000+48);			 //ǧλ
	Sendtosight[5]=(uint8_t)(GMYawtemp_Eular%1000/100+48);	 //��λ
	Sendtosight[6]=(uint8_t)(GMYawtemp_Eular%100/10+48);		 //ʮλ
	Sendtosight[7]=(uint8_t)(GMYawtemp_Eular%10+48);				 //��λ
	
	Sendtosight[8]=(uint8_t)(GMPitchtemp_Encoder/1000+48);		//ǧλ
	Sendtosight[9]=(uint8_t)(GMPitchtemp_Encoder%1000/100+48);//��λ
	Sendtosight[10]=(uint8_t)(GMPitchtemp_Encoder%100/10+48);	//ʮλ
	Sendtosight[11]=(uint8_t)(GMPitchtemp_Encoder%10+48);			//��λ
	
  while(count_Sendtosight<value)
  {
	  USART_SendData(USART1, Sendtosight[count_Sendtosight]);
	  count_Sendtosight++;
	  delay_us(150);
  }
 
  {
		count_Sendtosight=0;
  }
}

/***
������ChariotRecognition_Mes_Process(p)
���ܣ��������ݴ���
��ע����
***/
void ChariotRecognition_Mes_Process(uint8_t *p)
{
	  ChariotRecognitionTemp[0] = ((p[3]<<8) | p[2]);
		ChariotRecognitionTemp[1] = ((p[5]<<8) | p[4]);
		ChariotRecognitionTemp[2] =  p[6];
	  ChariotRecognitionDirection[0]= p[7];
	  ChariotRecognitionDirection[1]= p[8];
	  Chariot_Rec_Dir_rotate[0]= p[9];
	  Chariot_Rec_Dir_rotate[1]= p[10];
	
	Last_CameraDetectTarget_Flag=CameraDetectTarget_Flag;
	
	if(p[0]=='R'&&p[1]=='M'&&p[2]!='A')    //��ʶ��
	{
	 E_TEST3++;
	 E_TEST=ChariotRecognitionTemp[0];
	 E_TEST1 =ChariotRecognitionTemp[1];	
		
  	//UART1�յ�����ʱ������ͷ����Ŀ���־��Ϊ1��
	  CameraDetectTarget_Flag =1;	
		CR_ringBuffer.lost_COUNT =0;	
		
	  if(RC_CtrlData.rc.s1==3)//  if(RC_CtrlData.rc.s1==3 && gameState.game_progress == 4��Ϸ��ʼ����ʶ�𵽣�������
	  {
		  TempShootingFlag=1;//������־λ
	  }
		
		usart1_microsecond.time_now = Get_Time_Micros();//�����յ����ݵ�ʱ��
		usart1_microsecond.time_error = usart1_microsecond.time_now - usart1_microsecond.time_last;//�����������ε�ʱ��
		
		enter_CNT++;
		if(enter_CNT == 1)
		{
			YawCurrentPositionSave = GMYawEncoder.ecd_angle;
			PitchCurrentPositionSave = GMPitchEncoder.ecd_angle;
		}
		
		if(enter_CNT >30) enter_CNT = 30;
		
		//ֹͣ��׼�������ԺͶ��״̬��ʶ�����ʶ��֮���Ϊ���ģʽ���������Ӿ����������ݣ�����̨����ʶ������ݣ�Ҫ��״̬���ֶ��ر�Ħ����
		if(GetWorkState() == STOP_STATE || GetWorkState() == PREPARE_STATE || GetWorkState() == Test_STATE|| GetWorkState() == Dodeg_STATE)
		{
			ChariotRecognition_yaw = GMYawEncoder.ecd_angle;
			ChariotRecognition_pitch= GMPitchEncoder.ecd_angle;
		}
		else  //���ɡ�ʶ�𡢾�Ѳ�ߺͶ��״̬
		{
			ChariotRecognition_yaw  = ChariotRecognitionTemp[0]/100.0 ;//���ո�����  // GMYawEncoder.ecd_angle + ChariotRecognitionTemp[0]/100.0 

//			if(ChariotRecognition_yaw==0)//�˵��Ӿ�����������0��0���������Ϳ��Ա���ƽ��
//			{
//				ChariotRecognition_yaw = last_ChariotRecognition_yaw;
//			}
			
			if(CR_ringBuffer.lost_COUNT<35)  
			{
				CR_ringBuffer.ringBuf[CR_ringBuffer.tailPosition++] = ChariotRecognition_yaw;//yaw�Ƕ�����
				if(CR_ringBuffer.tailPosition >= BUFFER_SIZE) CR_ringBuffer.tailPosition = 0;//�γɻ��ζ���
				CR_ringBuffer.lost_COUNT = 0;//��ʧĿ���������
			}
			else //����ܶ��ûʶ�𵽣�ͻȻʶ��һ�Σ��������ζ����д洢��ֵ���������ζ������㣬βָ������
			{
				for(u8 loop_i = 0;loop_i<5;loop_i++)
				{
				  CR_ringBuffer.ringBuf[loop_i] = 0;
				}
				CR_ringBuffer.tailPosition = 0;
				CR_ringBuffer.lost_COUNT = 0;
			}
			
			Distance_buf[Dis_Buf_Index++] = ChariotRecognitionTemp[2];
			if(Dis_Buf_Index >= Dis_buf_Size) Dis_Buf_Index = 0;  //����ѭ��
			Target_Distance = filter(Distance_buf);

			// Eular[0]+
			ChariotRecognition_pitch = ChariotRecognitionTemp[1]/100.0;//(0.0088*Target_Distance+1.9604);//GMPitchEncoder.ecd_angle + ChariotRecognitionTemp[1]/100.0;

			
			last_ChariotRecognition_yaw   = ChariotRecognition_yaw;
			last_ChariotRecognition_pitch = ChariotRecognition_pitch;
		}
		usart1_microsecond.time_last = Get_Time_Micros();//��¼��һ���յ����ݵ�ʱ��
	}
	
	if(p[0]=='R'&&p[1]=='M'&&p[2]=='A'&&p[3]=='A'&&p[4]=='A')//ʶ�𲻵�
	{
		
			E_TEST=0;
      E_TEST1=0;
		
		CR_ringBuffer.lost_COUNT++;
		
		if(CR_ringBuffer.lost_COUNT>=35)//������������һ��Ҫȫ�ģ������׿���ʶ��ģʽ��
		{
			CR_ringBuffer.lost_COUNT = 35;
		}
		
		if(CR_ringBuffer.lost_COUNT<=35)//������ʧĿ��Ĵ���С��3�Σ�����Ԥ��
		{

			ChariotRecognition_yaw = GMYawEncoder.ecd_angle;
			ChariotRecognition_pitch = GMPitchEncoder.ecd_angle;			
   	}	
	}
	
	if(CR_ringBuffer.lost_COUNT>=35)
	{
		enter_CNT = 0;
		ChariotRecognition_yaw = GMYawEncoder.ecd_angle;
		ChariotRecognition_pitch = GMPitchEncoder.ecd_angle;
		TempShootingFlag = 0;
		CameraDetectTarget_Flag = 0;//�������?֡ûʶ�𵽣���״̬
	}
	
		if(  RC_CtrlData.rc.s1== 3) //&& gameState.game_progress == 4��Ϸ��ʼ
	{
		friction_wheel_state_flag = 1; //��Ϸ��ʼ��s1 �ŵ��м䣬Ħ���־Ϳ�
	}

	if(GetWorkState()== ChariotRecognition_STATE && RC_CtrlData.rc.s1==2)//ʶ��״̬�£����󲦸˷ŵ��������޷��ر�Ħ����  //5/2
	{
		TempShootingFlag = 0;//�ز���
		friction_wheel_state_flag = 0;//��Ħ����
	}

}


uint16_t filter(uint16_t *distance_buf)//�˲���ʵ�Ӿ�����
{
	int i,j,t;
	for(i=0;i<Dis_buf_Size-1;i++)
	{
		for(j=0;j<Dis_buf_Size-i-1;j++)
		{
			if(distance_buf[j]>distance_buf[j+1])
			{
				t = distance_buf[j+1];
				distance_buf[j+1] = distance_buf[j];
				distance_buf[j] = t;
			}
		}
	}
	
	return (int)(distance_buf[Dis_buf_Size/2]);
}
