#include "mid_pressure.h"

typedef enum
{
	Pressure_Select = 0,
	Temperature_Select
}Pres_Temp_Sele_TypeDef;

static IIC_Master_WRInfo_TypeDef iic_info =
{
	.port = PRES_PORT,
	.device_adr = PRES_DEVICE_ADDRESS,
	.dev_adr_tenbit_flag = 0,
	.len = 1,
	.error_resend_times = 3,
};  //IIC读取信息

static uint16_t c[6];

static uint8_t pres_dev_reset(void);  //设备复位，成功返回0，失败返回1
static uint8_t calibration_data_read(void);  //读取校准数据，成功返回0，失败返回1
static uint8_t digital_data_read(uint32_t * data, Pres_Temp_Sele_TypeDef sele);  //读取气压或温度数据，成功返回0，失败返回1
static uint8_t pres_temp_convert(float * pres_data, float * temp_data);  //气压及其温度数据读取并计算函数，成功返回0，失败返回1

//气压及其温度读取函数，成功返回0，失败返回1
uint8_t pres_temp_read(float * pres_data, float * temp_data)
{
	static uint8_t reset_flag = 1;  //复位标志
	static uint8_t cali_get_flag = 1;  //获取校准数据标志
	
	if(reset_flag)
	{
		if(pres_dev_reset())  //复位，并判断是否成功
			return 1;
		reset_flag = 0;
		cali_get_flag = 1;
		delay_ms(1000);
	}
	
	if(cali_get_flag)
	{
		if(calibration_data_read())  //读取校准数据，并判断是否成功
		{
			reset_flag = 1;
			cali_get_flag = 1;
			return 1;
		}
		cali_get_flag = 0;
	}
	
	if(pres_temp_convert(pres_data, temp_data))  //气压及其温度数据读取并计算，并判断是否成功
	{
		reset_flag = 1;
		cali_get_flag = 1;
		return 1;
	}
	
	return 0;
}

//设备复位，成功返回0，失败返回1
static uint8_t pres_dev_reset(void)
{
	uint8_t result = 1;
	uint8_t command = 0x1E;
	
	iic_info.data = &command;
	iic_info.len = 1;
	
	result = IIC_Simulation_Master_Write(&iic_info);
	
    return result;
}

//读取校准数据，成功返回0，失败返回1
static uint8_t calibration_data_read(void)
{
	uint8_t buf[2] = {0};
    uint8_t command = 0xA2;
    uint8_t i;
	
    for(i = 0; i < 6; i++)
    {
        iic_info.data = &command;
        iic_info.len = 1;
        if(IIC_Simulation_Master_Write(&iic_info))  //发送数据
            return 1;  //出现错误返回1
        iic_info.data = buf;
        iic_info.len = 2;
        if(IIC_Simulation_Master_ReadDirect(&iic_info))  //读取数据
            return 1;
        c[i] = ((uint16_t)buf[0] << 8) | buf[1];
        command += 2;
    }
	
	return 0;  //没有出现过错误，返回0
}

//读取气压或温度数据，成功返回0，失败返回1
static uint8_t digital_data_read(uint32_t * data, Pres_Temp_Sele_TypeDef sele)
{
    uint8_t buf[3] = {0};
    uint8_t command = 0x48;  //气压转换启动命令
	
	if(sele == Temperature_Select)
		command = 0x58;  //温度转换启动命令
	
	iic_info.data = &command;
	iic_info.len = 1;
	if(IIC_Simulation_Master_Write(&iic_info))
		return 1;  //出现错误返回1
	delay_ms(DELAY_CONVERSION);  //延时等待ADC数据转换完成
	
	command = 0x00;
	iic_info.data = &command;
	iic_info.len = 1;
	if(IIC_Simulation_Master_Write(&iic_info))
		return 1;
	
	iic_info.data = buf;
	iic_info.len = 3;
	if(IIC_Simulation_Master_ReadDirect(&iic_info))
		return 1;
	*data = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
	
	return 0;  //没有出现过错误，返回0
}

//气压及其温度数据读取并计算函数，成功返回0，失败返回1
static uint8_t pres_temp_convert(float * pres_data, float * temp_data)
{
	uint32_t d1 = 0, d2 = 0;  //气压、温度数据量缓存
	int32_t dt = 0, temp = 0;
	int64_t off = 0, sens = 0, p = 0;
	int32_t t2 = 0;
	int64_t off2 = 0, sens2 = 0;
	
	if(digital_data_read(&d1, Pressure_Select))
		return 1;
	
	if(digital_data_read(&d2, Temperature_Select))
		return 1;
	
	dt = d2 - ((uint32_t)c[4] << 8);  //d2 - c5 * 2^8，优先级+高于<<
	temp = 2000 + ((dt >> 8) * (c[5] >> 8) >> 7);  //2000 + dt * c6 / 2^23
	// temp = 2000 + (dt * c[5] >> 23);  //2000 + dt * c6 / 2^23
	if(temp < 2000)
	{
		t2 = dt * dt >> 31;
		off2 = 5 * (temp - 2000) * (temp - 2000) >> 1;
		sens2 = 5 * (temp - 2000) * (temp - 2000) >> 2;
		
		if(temp < -1500)
		{
			off2 = off2 + 7 * (temp + 1500) * (temp + 1500);
			sens2 = sens2 + (11 * (temp + 1500) * (temp + 1500) >> 1);
		}
	}
	temp = temp - t2;
	
	off = ((uint64_t)c[1] << 16) + ((c[3]>>3) * (dt>>4));
	sens = ((uint64_t)c[0] << 15) + ((c[2]>>4) * (dt>>4));
	// off = ((uint64_t)c[1] << 16) + (c[3] * dt >> 7);
	// sens = ((uint64_t)c[0] << 15) + (c[2] * dt >> 8);
	off = off - off2;
	sens = sens - sens2;
	p = (((d1 >> 11) * (sens >> 10)) - off) >> 15;
	// p = ((d1 * sens >> 21) - off) >> 15;
	
	*pres_data = p / 1000.0;  //mbar->KPa，1mbar = 100Pa = 0.1KPa
	*temp_data = temp / 100.0;
	
	return 0;
}
