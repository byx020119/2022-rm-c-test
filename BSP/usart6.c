#include "main.h"
#include "usart6.h"

#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
//int fputc(int ch, FILE *f)
//{ 	
//	while((USART6->SR&0X40)==0);//ѭ������,ֱ���������   
//	USART6->DR = (u8) ch;      
//	return ch;
//}
#endif

uint8_t USART6_DMA1_RX_BUF[2][IMU_DMA_RX_BUF_LEN]; //2��LEN��

uint8_t CRC8_Ref_Value;
uint8_t CRC8_Solve_Value;
uint16_t CRC16_Ref_Value;    //�յ���CRC16У��ֵ
uint16_t CRC16_Solve_Value;  //����õ���CRC16У��ֵ

uint8_t Save_Element_Array[30];
uint16_t data_Length;
uint16_t Tail_Over_Zero_Value =0;   //βָ��ͨ�����
uint16_t Head_Over_Zero_Value =0;   //ͷָ��ͨ�����

uint32_t usart6_this_time_rx_len = 0;

void usart6_Init(u32 bound)
{
	/* -------------- Enable Module Clock Source ----------------------------*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
	/* -------------- Configure GPIO & USART6 -------------------------------*/
	{
		GPIO_InitTypeDef gpio;
		USART_InitTypeDef usart;
		DMA_InitTypeDef dma;
		
		GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_USART6);
		GPIO_PinAFConfig(GPIOG, GPIO_PinSource9, GPIO_AF_USART6);
		
		GPIO_StructInit(&gpio);
		gpio.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_9;
		gpio.GPIO_Mode = GPIO_Mode_AF;
		gpio.GPIO_Speed = GPIO_Speed_100MHz;
		gpio.GPIO_PuPd = GPIO_OType_PP;
		GPIO_Init(GPIOG, &gpio);
		
		USART_DeInit(USART6);
		USART_StructInit(&usart);
		usart.USART_BaudRate =bound;
		usart.USART_WordLength = USART_WordLength_8b;
		usart.USART_StopBits = USART_StopBits_1;
		usart.USART_Parity = USART_Parity_No;//USART_Parity_Even;
		usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_Init(USART6, &usart);
		
		USART_DMACmd(USART6, USART_DMAReq_Rx, ENABLE);
		USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);        //usart rx idle interrupt  enabled;IDLE�Ǵ��ڿ����ж�
//		USART_ITConfig(USART2, USART_IT_TC, ENABLE);
		USART_Cmd(USART6, ENABLE);
	}
	/* -------------- Configure DMA1_Stream5_ch4 --------------------------------*/
	{
		DMA_InitTypeDef dma;
		
		DMA_DeInit(DMA2_Stream1);
		DMA_StructInit(&dma);
		dma.DMA_Channel = DMA_Channel_5;
		dma.DMA_PeripheralBaseAddr = (uint32_t)(&USART6->DR);
		dma.DMA_Memory0BaseAddr = (uint32_t)&USART6_DMA1_RX_BUF[0][0];
		dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
		dma.DMA_BufferSize = sizeof(USART6_DMA1_RX_BUF)/2;  //sizeof(USART2_DMA1_RX_BUF)��ʾ��������ܵĴ�С������2��ʾÿ�δ������������������С��һ�룬��USART2_DMA1_RX_BUF[0]����USART2_DMA1_RX_BUF[1]�Ĵ�С
		                                                    //�൱������NDTR(ÿ�δ����������)��ֵ
		dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
		dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		dma.DMA_Mode = DMA_Mode_Circular;                //ѭ��ģʽ��DMA����һ�����ݺ�DMA_BufferSize(ÿ�δ����������)�����¼���Ϊ��ʼ��ʱ��ֵ�����µ�������ʱ���ɼ�������
		                                                 //��ͨģʽ��DMA����һ�����ݺ�DMA_BufferSize(ÿ�δ����������)�������¼���Ϊ��ʼ��ʱ��ֵ����Ҫ���´����������³�ʼ��
		                                                 //DMA����һ������ʱ��ÿ����һ���е�һ�����ݣ�DMA_BufferSize���Զ���1��ֱ����Ϊ0���ð��������
		dma.DMA_Priority = DMA_Priority_Medium;
		dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
		dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
		dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_Init(DMA2_Stream1, &dma);
		
		//����Memory1,Memory0�ǵ�һ��ʹ�õ�Memory
		//����������ǿ���˫����ģʽ
		DMA_DoubleBufferModeConfig(DMA2_Stream1, (uint32_t)&USART6_DMA1_RX_BUF[1][0], DMA_Memory_0);   //first used memory configuration
		DMA_DoubleBufferModeCmd(DMA2_Stream1, ENABLE);
		
		DMA_Cmd(DMA2_Stream1, ENABLE);
	}
	/* -------------- Configure NVIC ----------------------------------------*/
	{
		NVIC_InitTypeDef nvic;
		nvic.NVIC_IRQChannel = USART6_IRQn;                          
		nvic.NVIC_IRQChannelPreemptionPriority = 0;   //pre-emption priority 
		nvic.NVIC_IRQChannelSubPriority = 3;		    //subpriority 
		nvic.NVIC_IRQChannelCmd = ENABLE;			
		NVIC_Init(&nvic);	
	}
}
//void USART6_SendChar(char b)
//{
//    while( USART_GetFlagStatus(USART6,USART_FLAG_TC) == RESET);
//	  USART_SendData(USART6,b);
//}

	int i,j;
void USART6_IRQHandler(void)
{
	if(USART_GetITStatus(USART6, USART_IT_IDLE) != RESET)       //USART��IDLE�ж�;IDLE�ǿ����ж�
	{
		//clear the idle pending flag                             //���IDLE��־λ
		(void)USART6->SR;
		(void)USART6->DR;

		//Target is Memory0
		if(DMA_GetCurrentMemoryTarget(DMA2_Stream1) == 0)         //DMA_GetCurrentMemoryTarget():�õ���ǰDMA���ڴ��ַ��0����1
		{
			DMA_Cmd(DMA2_Stream1, DISABLE);
			usart6_this_time_rx_len = IMU_DMA_RX_BUF_LEN - DMA_GetCurrDataCounter(DMA2_Stream1);//�ж�0����������Ѵ�������
			DMA2_Stream1->NDTR = (uint16_t)IMU_DMA_RX_BUF_LEN;         //�趨1������
			DMA2_Stream1->CR |= (uint32_t)(DMA_SxCR_CT);                  //ʹ��1����
			DMA_Cmd(DMA2_Stream1, ENABLE);
			
			if(usart6_this_time_rx_len == IMU_FRAME_LENGTH)
			{
				IMUDataProcess(USART6_DMA1_RX_BUF[0]);//IMU���Բ�����Ԫ
			}
			DMA_Cmd(DMA2_Stream1, ENABLE);

		}
		else //Target is Memory1
		{
			DMA_Cmd(DMA2_Stream1, DISABLE);
			usart6_this_time_rx_len = IMU_DMA_RX_BUF_LEN - DMA_GetCurrDataCounter(DMA2_Stream1);
			DMA2_Stream1->NDTR = (uint16_t)IMU_DMA_RX_BUF_LEN;      //relocate the dma memory pointer to the beginning position
			DMA2_Stream1->CR &= ~(uint32_t)(DMA_SxCR_CT);                  //enable the current selected memory is Memory 0
			DMA_Cmd(DMA2_Stream1, ENABLE);
			
      if(usart6_this_time_rx_len == IMU_FRAME_LENGTH)
			{
				IMUDataProcess(USART6_DMA1_RX_BUF[1]);
			}
			DMA_Cmd(DMA2_Stream1, ENABLE);
		}
	}
}

float Angles;
int16_t Gyro[3];//���ٶ�
float Eular[3]; //ŷ����

void IMUDataProcess(uint8_t *pData)
{
	static  int   yawCount       = 0;
	static  int   pitchCount       = 0;
	static  float LastAngle      = 0;
	static  float NowAngle       = 0;
	static  float lastPitchAngle = 0;
	static  float nowPitchAngle  = 0;
	
	mpu6050_micrsecond.time_last = Get_Time_Micros();
	
	if(pData == NULL)
	{
		return;
	}
	Gyro[0] = ((float)(int16_t)(pData[16] + (pData[17]<<8)));//roll
	Gyro[1] = ((float)(int16_t)(pData[18] + (pData[19]<<8)));//pitch
	Gyro[2] = ((float)(int16_t)(pData[20] + (pData[21]<<8)));//yaw

	Eular[0] = ((float)(int16_t)(pData[23] + (pData[24]<<8)))/100;//pitch
	Eular[1] = -((float)(int16_t)(pData[25] + (pData[26]<<8)))/100;//roll	
	
	Angles = ((float)(int16_t)(pData[27] + (pData[28]<<8)))/10;//yaw
	
	lastPitchAngle = nowPitchAngle;
	nowPitchAngle = Eular[1];

	LastAngle=NowAngle;
	NowAngle=Angles;
	
	if((NowAngle-LastAngle)>300)
		yawCount--;
	if((NowAngle-LastAngle)<-300)
		yawCount++;
	Eular[2]  = NowAngle+yawCount*360;              //�Ѿ����ǵ��˼�Ȧ��
	
	if((nowPitchAngle-lastPitchAngle)>300)
		pitchCount--;
	if((nowPitchAngle-lastPitchAngle)<-300)
		pitchCount++;
	Eular[1]  = nowPitchAngle+pitchCount*360;
}
