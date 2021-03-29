#include <stdio.h>

#define EPSILON 0.00001

typedef double electric_value_type; //电压, 电流, 功率等值, 使用的数类型, 可以定义为double或者float
typedef double percent_type; //百分比使用的数据类型, 可以定义为double或者float

/*
 * 用于计算视在功率的电压, 电流值对
 */
typedef struct voltAmp
{
    electric_value_type U; //电压测量值
    electric_value_type I; //电流测量值
} voltAmp_s;

/**************************************************
* 功能描述: 计算单相视在功率
* ------------------------------------------------
* 输入参数: phaseU, 电压的测量值
* 输入参数: phaseI, 电流的测量值
* ------------------------------------------------
* 返回值: 无
**************************************************/
electric_value_type get_phase_apparent_power(electric_value_type phaseU, electric_value_type phaseI)
{
    return (phaseU * phaseI);
}

/**************************************************
* 功能描述: 计算总视在功率
* ------------------------------------------------
* 输入参数: pVA, 电压-电流的测量值数组
* 输入参数: len, 电压-电流的测量值数组长度
* ------------------------------------------------
* 返回值: 总视在功率
**************************************************/
electric_value_type get_total_apparent_power(voltAmp_s *pVA, int len)
{
    electric_value_type apPower = 0.0;

    if ((NULL == pVA) || (0 == len)) {
        return 0.0;
    }

    for (int i = 0; i < len; i++) {
        apPower += get_phase_apparent_power(pVA->U, pVA->I);
    }

    return apPower;
}

/**************************************************
* 功能描述: 计算配变负载率, 计算方式使用
* 负载率=(输出视在功率)/(配变容量)x100%
* ------------------------------------------------
* 输入参数: pVA, 电压-电流的测量值数组
* 输入参数: len, 电压-电流的测量值数组长度
* 输入参数: capacity, 配变的容量
* ------------------------------------------------
* 返回值: 配变负载率, 已乘以100%
**************************************************/
percent_type get_load_rate(voltAmp_s *pVA, int len, electric_value_type capacity)
{
    if (capacity < EPSILON) {
        return 0.0;
    }

    return ((get_total_apparent_power(pVA, len)/capacity)*100.0);
}
