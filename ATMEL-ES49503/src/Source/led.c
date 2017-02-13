/*
 * led.c
 *
 * Created: 2017/2/6 10:30:41
 *  Author: chenjiawei
 */ 

#include "led.h"

#define LED_DISPLAY_2S 8
uint8_t led_250ms_tick;

/**
  * @brief  This function is Initialization LED0 and LED1. default state of led is turn off.
  * @param  None
  * @retval None
  */

void Configure_Led(void)
{
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);

	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(LED0_PIN, &pin_conf);
	port_pin_set_output_level(LED0_PIN, LED0_INACTIVE);
	
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(LED1_PIN, &pin_conf);
	port_pin_set_output_level(LED1_PIN, LED1_INACTIVE);

}

/****************************************************************************
FUNCTION		: SysLED_Display
DESCRIPTION		: LED显示函数
INPUT			: None
OUTPUT			: None
NOTICE			: LED显示逻辑判断
DATE			: 2016/07/21
*****************************************************************************/
void SysLED_Display(void)
{
	if(sys_states.val.sys_sw_nconnect_flag == 1)
	{
		Bsp_LED0_Off();//red
		Bsp_LED1_Off();//green
	}
	else
	{
		if(sys_err_flags.VAL >0)
		{
		Bsp_LED0_On();
		Bsp_LED1_Off();
		}
		else
		{
			if((afe_flags.val.afe_dfrv_autoprotect_flag == 1)||(sys_states.val.soft_dch_protect ==1))
			{
				SysLED_RunProtect();
			}
			else
			{
				if(g_sys_cap.val.re_cap_rate>95)//满电
				{
					Bsp_LED0_Off();
					Bsp_LED1_On();
				}
				else
				{
					if(sys_states.val.sys_chg_state == 1)
					{
						SysLED_ChgNormal();
					}
					else
					{
						if(sys_states.val.sys_dch_state == 1)
						{
							Bsp_LED0_On();//red
							Bsp_LED1_On();//green
						}
						else
						{
							SysLED_RunNormal();
						}
					}
				}
			}
		}
	}
}

/****************************************************************************
FUNCTION		: SysLED_RunProtect
DESCRIPTION		: 输出打开,系统保护
INPUT			: None
OUTPUT			: None
NOTICE			: 红灯2S灭250ms亮,绿灯灭
DATE			: 2016/07/21
*****************************************************************************/
void SysLED_RunProtect(void)
{
	led_250ms_tick++;
	if(led_250ms_tick > LED_DISPLAY_2S)
	{
		if(led_250ms_tick >(LED_DISPLAY_2S+1))
		{
			led_250ms_tick =0;
			Bsp_LED0_Off();
		}
		else
		{
			Bsp_LED0_On();
		}
	}
	else
	{
		Bsp_LED0_Off();
	}
	Bsp_LED1_Off();
}

/****************************************************************************
FUNCTION		: SysLED_ChgNormal
DESCRIPTION		: 输出打开,系统充电
INPUT			: None
OUTPUT			: None
NOTICE			: 绿灯1S灭250ms亮,红灯灭
DATE			: 2016/07/21
*****************************************************************************/
void SysLED_ChgNormal(void)
{
	led_250ms_tick++;
	if(led_250ms_tick > (LED_DISPLAY_2S>>1))
	{
		if(led_250ms_tick >((LED_DISPLAY_2S>>1)+1))
		{
			led_250ms_tick =0;
			Bsp_LED1_Off();
		}
		else
		{
			Bsp_LED1_On();
		}
	}
	else
	{
		Bsp_LED1_Off();
	}
	Bsp_LED0_Off();
}

/****************************************************************************
FUNCTION		: SysLED_RunNormal
DESCRIPTION		: 输出打开,系统正常
INPUT			: None
OUTPUT			: None
NOTICE			: 绿灯2S灭250ms亮,红灯灭
DATE			: 2016/07/21
*****************************************************************************/
void SysLED_RunNormal(void)
{
	led_250ms_tick++;
	if(led_250ms_tick > LED_DISPLAY_2S)
	{
		if(led_250ms_tick >(LED_DISPLAY_2S+1))
		{
			led_250ms_tick =0;
			Bsp_LED1_Off();
		}
		else
		{
			Bsp_LED1_On();
		}
	}
	else
	{
		Bsp_LED1_Off();
	}
	Bsp_LED0_Off();
}