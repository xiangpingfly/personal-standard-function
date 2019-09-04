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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* Main program */
int main(void)
{
	float light_data = 0;
    float pressure = 0, pres_temperature = 0;
    float temperature = 0, humidity = 0;
    float co2 = 0;
    float tvoc = 0, tovcco2 = 0;
    uint32_t len = 0;
    uint8_t buf1[128] = {0};
	/* STM32 Configure the MPU attributes as Write Through */
//	MPU_Config();

	/* STM32 Enable the CPU Cache */
//	CPU_CACHE_Enable();
	
	/* STM32F7xx HAL library initialization */
//	HAL_Init();

	/* Configure the system clock to 16 MHz */
	CLK_SYSCLK_Config();  //内部高速时钟，HSI和CPU时钟0分频，fcpu = 16MHz
	
	Delay_Init(16);  //延时函数基准配置
//	Led_GPIO_Init();  //led引脚配置
//	Key_GPIO_Init();  //按键引脚配置
    IIC_Master_Init(1);
    IIC_Master_Init(2);
    IIC_Master_Init(3);
	// TIMx_Init(TIM2_Select, 4, 1000-1);  //配置通用定时器，周期1000us，1ms
	
	Uart_Init(UART1_Select, 9600, uart1_read_deal);  //UART1波特率9600
    
    init_Co2();
    Init_Timer4();
    I2C_MASTERMODE_Init(STANDARDSPEED);
    
    // uart1_send_ontime_int();  //UART1定时发送初始化，注册定时器
	
    asm("rim");//开全局中断，sim为关中断
    
	/* Infinite loop */
	while (1)
	{
		delay_ms(1000);
//		key_scan(key_task);
        // uart1_send_ontime();  //UART1定时发送
		// uart_read(UART1_Select, uart1_read_deal);
		if(light_read(&light_data))
			light_data = 0;
        if(pres_temp_read(&pressure, &pres_temperature))
		{
			pressure = 0;
			pres_temperature = 0;
		}
        if(temp_humi_read(&temperature, &humidity))
		{
			temperature = 0;
			humidity = 0;
		}
        Get_CO2(&co2);
        TVOC_ReadDat(&tvoc, &tovcco2);
        len = sprintf((char*)buf1, "light = %.2f, pres = %.2f, pres_temp = %.2f, temp = %.2f, humi = %.2f, co2 = %.2f, tvoc = %.2f, tovcco2 = %.2f", 
                      light_data, pressure, pres_temperature, temperature, humidity, co2, tvoc, tovcco2);
        uart_write(UART1_Select, buf1, len);
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
