/**
  ******************************************************************************
  * @file    main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-April-2016
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

//uint16_t time_data_deal = 0;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Main program */
int main(void)
{
#if defined STM32_HAL
	/* STM32 Configure the MPU attributes as Write Through */
	MPU_Config();

	/* STM32 Enable the CPU Cache */
	CPU_CACHE_Enable();
	
	/* STM32F7xx HAL library initialization */
	HAL_Init();
#endif

	/* Configure the system clock */
	CLK_SYSCLK_Config();
	
	Delay_Init(72);  //��ʱ������׼����
    Led_GPIO_Init();
    
//	/* Infinite loop */
	while(1)
	{
        Led_GPIO_Write(LED0, LED_OFF);
		delay_ms(1000);
        Led_GPIO_Write(LED2, LED_ON);
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
