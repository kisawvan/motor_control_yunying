#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "MyCAN.h"
#include <string.h>
#include <math.h>
#include "freertos_demo.h"

// CAN 单位映射参数（需根据实际驱动器说明书修正，此处提供示例基准）
#define MAX_TORQUE_NM 5.0f
#define MAX_SPEED_RPM 6000.0f
// 重力等物理常数
#define G_CONST 9.81f
#define ROD_LENGTH_M 1.00f		// 假设重心在0.5m处，整体长度为1m
#define ROD_MASS_KG 0.540f		// 杆件质量 540g = 0.54kg
#define PAYLOAD_MASS_KG 0.2788f // 末端工件质量 278.8g = 0.2788kg
#define COG_DISTANCE_M 1.00f	// 工件重心到电机中心距离 1m

static uint16_t torque_to_can_units(float torque_nm)
{
	if (torque_nm < 0.0f)
		torque_nm = -torque_nm;
	if (torque_nm > MAX_TORQUE_NM)
		torque_nm = MAX_TORQUE_NM;
	return (uint16_t)((torque_nm / MAX_TORQUE_NM) * 65535.0f + 0.5f);
}

static uint16_t speed_to_can_units(float speed_rpm)
{
	if (speed_rpm < 0.0f)
		speed_rpm = 0.0f;
	if (speed_rpm > MAX_SPEED_RPM)
		speed_rpm = MAX_SPEED_RPM;
	return (uint16_t)((speed_rpm / MAX_SPEED_RPM) * 65535.0f + 0.5f);
}

static uint16_t angle_to_position_units16(float angle_deg)
{
	float a = fmodf(angle_deg, 360.0f);
	if (a < 0.0f)
		a += 360.0f;
	// 假设360度对应65535
	return (uint16_t)((a / 360.0f) * 65535.0f + 0.5f);
}

// 重力补偿计算函数
float calculate_gravity_compensation(float angle_rad)
{
	// 杆件作用力矩：杆重 * g * 杆中心到转轴距离(rod_length/2) * cos(theta)
	float torque_rod = ROD_MASS_KG * G_CONST * (ROD_LENGTH_M / 2.0f) * cosf(angle_rad);
	// 末端工件力矩：payload * g * 重心距离 * cos(theta)
	float torque_payload = PAYLOAD_MASS_KG * G_CONST * COG_DISTANCE_M * cosf(angle_rad);
	float total_torque = torque_rod + torque_payload;
	if (total_torque < 0.0f)
		total_torque = -total_torque;
	return total_torque;
}

// CAN报文生成函数（使用传递的template8作为参考模板）
void generate_motor_command(float target_angle_deg, float target_speed_rpm, const uint8_t *template8, uint8_t *can_data)
{
	// 基础报文
	memcpy(can_data, template8, 8);

	// 计算重力补偿力矩
	float angle_rad = target_angle_deg * 3.1415926535f / 180.0f;
	float compensation_torque = calculate_gravity_compensation(angle_rad);

	uint16_t torque_val = torque_to_can_units(compensation_torque);
	uint16_t speed_val = speed_to_can_units(target_speed_rpm);
	uint16_t pos_val = angle_to_position_units16(target_angle_deg);

	// 按字节写入（大端模式）：3,4位为速度(下标2,3), 5,6位为力矩(下标4,5), 7,8位为位置(下标6,7)
	can_data[2] = (speed_val >> 8) & 0xFF;
	can_data[3] = (speed_val >> 0) & 0xFF;

	can_data[4] = (torque_val >> 8) & 0xFF;
	can_data[5] = (torque_val >> 0) & 0xFF;

	can_data[6] = (pos_val >> 8) & 0xFF;
	can_data[7] = (pos_val >> 0) & 0xFF;
}

uint8_t KeyNum;
uint32_t TxID = 0x01;
uint8_t TxLength = 8;
// 3、4位是速度大小，5、6位是最大力矩、7、8位是位置
uint8_t motor_P_30d[8] = {0x07, 0x35, 0x20, 0x03, 0xB8, 0x0B, 0xe8, 0x03};
uint8_t motor_P_180d[8] = {0x07, 0x35, 0x20, 0x03, 0xB8, 0x0B, 0x88, 0x13};
uint8_t motor_p_0[8] = {0x07, 0x35, 0x20, 0x03, 0xB8, 0x0B, 0x00, 0x00};
uint32_t RxID;
uint8_t RxLength;
uint8_t RxData[8];
uint8_t CmdData[8]; // 用于存放生成的带有重力补偿的报文
int main(void)
{
	MyCAN_Init();
	Key_Init();
	FreeRTOS_Start();
	can_send_data(CAN1, TxID, motor_P_180d, TxLength);
	Delay_ms(1000);
	//	can_send_data(CAN1, TxID, motor0, TxLength);
	//	Delay_ms(1000);
	while (1)
	{
		KeyNum = Key_GetNum();
		// 如果按下了PA15按键(即返回值为3)
		if (KeyNum == 3)
		{
			// 生成目标为0度时的带重力补偿控制报文（速度暂设1000rpm, 使用motor_P_180d为控制项模板）
			generate_motor_command(0.0f, 1000.0f, motor_P_180d, CmdData);
//			can_send_data(CAN1, TxID, CmdData, TxLength);
//			Delay_ms(100);
			can_send_data(CAN1, TxID, motor_p_0, TxLength);
		}
	}
}
