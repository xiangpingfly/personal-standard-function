#include "mid_light.h"

//光照度读取函数，成功返回0，失败返回1
uint8_t light_read(float * light_data)
{
	uint8_t buf[4] = {0};  //读取数据缓存数组
	uint32_t data = 0;  //读取数据缓存
	IIC_Master_ReadReg_Info_TypeDef iic_info =
	{
		.wr_info.port = LIGHT_PORT,
		.wr_info.device_adr = LIGHT_DEVICE_ADR,
        .wr_info.dev_adr_tenbit_flag = 0,
		.wr_info.data = buf,
		.wr_info.len = 4,
        .wr_info.error_resend_times = 3,
		.register_adr = 0x00,
		.reg_adr_twobyte_flag = 0,
		.readreg_delay_flag = Delay_Back,
		.readreg_delay_nms = 15,
	};  //IIC读取信息
	
	if(IIC_Simulation_Master_ReadRegister(&iic_info))  //IIC读取数据并保存结果到缓存数组，并判断是否成功
		return 1;
	
	data = buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);  //缓存数据
	*light_data = (float)data * 1.4 / 1000;  //计算结果
	
	return 0;
}
