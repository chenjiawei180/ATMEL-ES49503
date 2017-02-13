/*
 * soc.c
 *
 * Created: 2017/2/8 8:27:02
 *  Author: chenjiawei
 */ 

#include "soc.h"
#include "history.h"
#include "protect.h"
#include "flash.h"

uint8_t cap_update_reload_cnt =0;
uint16_t vbat_1min_delay =0;
uint16_t vbat_1min_last_val =0;
uint16_t deta_time_val;
uint8_t stop_cap_update_val=0;
uint8_t dch_delay=0;
uint16_t vbat_10s_last_val =0;
uint8_t chg_delay=0;

uint8_t temp_soc_err_cnt=0;
uint8_t soc_fcc_save = 0;
uint8_t soc_fcc_reload = 0;
uint8_t fullcap_reload_delay =0;
uint32_t uv_cap_val=0;           //修正soc值20161009zzysoc2
uint8_t uv_re_cap_rate=0;        //修正soc值20161009zzysoc2

uint8_t cap_save_delay_cnt =0;
uint16_t chg_record_cnt =0;

const uint16_t Cell_volt[101] =
{
	3307,3324,3342,3359,3376,3394,3411,3428,3445,3463,
	3480,3489,3498,3506,3515,3524,3533,3542,3550,3559,
	3568,3573,3579,3584,3589,3595,3600,3605,3610,3616,
	3621,3624,3627,3630,3633,3636,3639,3642,3645,3648,
	3651,3655,3658,3662,3665,3669,3672,3676,3679,3683,
	3686,3694,3702,3710,3718,3727,3735,3743,3751,3759,
	3767,3776,3784,3793,3801,3810,3818,3827,3835,3844,
	3852,3862,3871,3881,3890,3900,3909,3919,3928,3938,
	3947,3958,3968,3979,3990,4001,4011,4022,4033,4043,
	4054,4066,4078,4090,4102,4115,4127,4139,4151,4163,
	4175
};

/****************************************************************************
FUNCTION		: NormalCapacityProc
DESCRIPTION		: 容量更新以及电压补偿
INPUT			: None
OUTPUT			: None
NOTICE			: 未完成
DATE			: 2016/06/24
*****************************************************************************/
void NormalCapacityProc(void)
{
	static bool  power_first_flag = false;

	uint32_t capacity_volt;
	uint32_t temp = 0;
	//开机输入容量值
	if(Total_VBAT > 6553)
	{
		if((power_first_flag == false) && (afe_flags.val.afe_uv_flag == 0))
		{
			if(flash_flags.val.re_cap_update_flag == 0)
			{
				// 6553 = 2.0V   8519 = 2.6V  13762 = 4.2V  SUB = 73400/100 = 52
				//电压查表
				capacity_volt = (uint32_t)Total_VBAT * 5000 / 16384;
				g_sys_cap.val.re_cap_rate = VbatToSoc((uint16_t)capacity_volt);

				EEPROM_Write_DATA(EEPROM_INDEX_CAP_VAL, g_sys_cap.val.re_cap_rate, 0);
				g_sys_cap.val.re_cap_rate_old = g_sys_cap.val.re_cap_rate;
				g_sys_cap.val.re_cap_rate_old2 = g_sys_cap.val.re_cap_rate;   //修正soc值20161010zzysoc3
			}
			temp = g_sys_cap.val.re_cap_rate;
			temp = temp * g_sys_cap.val.full_cap / 100;
			//	        	temp = ~temp;  // 取反
			//	        	temp+=1;

			g_sys_cap.val.cap_val = temp;
			g_sys_cap.val.cap_val3 = 0 ;        //修正soc值20161010zzysoc4
			power_first_flag = true;
		}
	}

	Cap_Update_Check();//检测是否需要容量补偿容量更新

	FullCap_Update();//判断是否需要满电更新

	// 计算剩余容量
	if(Total_VBAT > 6553)
	{
		if(g_sys_cap.val.cap_val > g_sys_cap.val.full_cap)
		{
			g_sys_cap.val.cap_val = g_sys_cap.val.full_cap;
		}
		if(afe_flags.val.afe_uv_flag == true)
		{
			//修正soc值20161009zzysoc2
			uv_cap_val = g_sys_cap.val.cap_val ;
			g_sys_cap.val.cap_val = 0;//修正soc值20161009zzysoc2原有，其他为添加项
			g_sys_cap.val.re_cap_rate_old2 = g_sys_cap.val.re_cap_rate;//欠压清零//修正soc值20161010zzysoc3
			uv_re_cap_rate = g_sys_cap.val.re_cap_rate ;
			//   temp =  uv_cap_val  ;
			//   temp *= 100;
			//   temp = (UCHAR)(temp/ g_sys_cap.val.full_cap);
			//    uv_re_cap_rate  = g_sys_cap.val.re_cap_rate + temp - g_sys_cap.val.re_cap_rate_old2;
		}
		temp = g_sys_cap.val.cap_val;
		temp *= 100;
		temp   = (uint8_t)(temp / g_sys_cap.val.full_cap);
		g_sys_cap.val.re_cap_rate  = g_sys_cap.val.re_cap_rate + temp - g_sys_cap.val.re_cap_rate_old2;
		g_sys_cap.val.re_cap_rate_old2 = temp;                              //修正soc值20161010zzysoc3
		if( (g_sys_cap.val.re_cap_rate > 200) || (g_sys_cap.val.cap_val == 0 ) ) //防止负值溢出
		{
			g_sys_cap.val.re_cap_rate = 0;
		}
		if( g_sys_cap.val.re_cap_rate > 100 )//防止超出100
		{
			g_sys_cap.val.re_cap_rate = 100;
		}
	}

	//清除循环次数标记方法1
	if(g_sys_cap.val.cycle_record_flag != 0)
	{
		if((g_sys_cap.val.cycle_record_flag < g_sys_cap.val.re_cap_rate) && ((g_sys_cap.val.re_cap_rate -  g_sys_cap.val.cycle_record_flag) > 7))
		{
			g_sys_cap.val.cycle_record_flag = 0;
		}
	}

	SOC_FLASH_Save();
	BatCycleProc();
}

/****************************************************************************
FUNCTION		: VbatToSoc
DESCRIPTION		: 电压查表
INPUT			: vbat_val 电压值
OUTPUT			: 容量值
NOTICE			: 折半查表
DATE			: 2016/06/27
*****************************************************************************/
uint8_t VbatToSoc(uint16_t vbat_val)
{

	uint8_t low = 0;
	uint8_t high = 101;    //修改
	uint8_t mid;
	while(low < high)
	{
		mid = (low + high)/2;    // 先执行除法，再加法。防止low + high > 256 而溢出

		if(vbat_val == Cell_volt[mid])
		{break;}    // 索引到刚好相等的值，则马上返回
		if(high - low == 1)
		{
			mid = low;                 // 由于不是精确查找，若在小区间内，取小值
			break;
		}
		if(vbat_val < Cell_volt[mid])
		{ high = mid;}
		else
		{low = mid;}
	}

	return mid;
}

/****************************************************************************
FUNCTION		: Cap_Update_Check
DESCRIPTION		: 容量补偿更新程序
INPUT			: None
OUTPUT			: None
NOTICE			: 不在充电放电超过一定时间进行电压补偿
DATE			: 2016/06/24
*****************************************************************************/
void Cap_Update_Check(void)
{
	uint16_t vbat_sub =0;
	uint8_t new_cap_rate;
	uint32_t deta_cap_rate,new_cap_rate2;
	uint32_t capacity_volt;
	//    ULONG temp2 = 0;                //同时修正soc值20161009zzysoc1
	if((nADC_CURRENT <CUR_CHG_01C)&&(nADC_CURRENT >CUR_DCH_01C))
	{
		sys_flags.val.cap_update_end_flag =0;
		cap_update_reload_cnt =0;
		if(vbat_1min_delay ==0)
		{
			vbat_1min_last_val = Total_VBAT;
			vbat_1min_delay++;
			//电压查表
			capacity_volt = (uint32_t)Total_VBAT*5000/16384;
			new_cap_rate = VbatToSoc((uint16_t)capacity_volt);
			//电压查表一次,估计误差值,10分钟/误差值等于更新时间
			if(new_cap_rate>g_sys_cap.val.re_cap_rate)
			{
				deta_time_val = new_cap_rate - g_sys_cap.val.re_cap_rate;
			}
			else
			{
				deta_time_val = g_sys_cap.val.re_cap_rate - new_cap_rate;
			}
			if(deta_time_val != 0)
			{
				deta_time_val *= deta_time_val;
				deta_time_val = 2400/deta_time_val;
			}
			else
			{
				deta_time_val = 120;
			}
		}
		else
		{
			vbat_1min_delay++;
			if(vbat_1min_delay > deta_time_val)
			{
				if(Total_VBAT > vbat_1min_last_val)
				{
					vbat_sub = Total_VBAT - vbat_1min_last_val;
				}
				else
				{
					vbat_sub = vbat_1min_last_val - Total_VBAT;
				}
				if(vbat_sub < VBAT_SOC_UPDATE)
				{
					sys_flags.val.re_cap_update_flag = true;
					stop_cap_update_val =1;
				}
				vbat_1min_delay =0;
			}
		}
	}
	else
	{
		cap_update_reload_cnt++; //超过3次重新计时
		if(cap_update_reload_cnt >3)
		{
			cap_update_reload_cnt =0;
			vbat_1min_delay =0;
			sys_flags.val.cap_update_end_flag =1;
		}
	}
	
	
	// 20160722新增 充放电补偿
	//3.2V末端校准,5% 3.8V 10%
	if(nADC_TMONI_BAT_MIN >10)
	{
		if((nADC_CURRENT <CUR_DCH_01C)&&(nADC_CURRENT>CUR_DCH_02C))
		{
			if(dch_delay ==0)
			{
				vbat_10s_last_val = Total_VBAT;
			}
			dch_delay++;
			if(dch_delay >CAP_10S_DELAY)
			{
				if(vbat_10s_last_val > Total_VBAT)
				{
					vbat_sub = vbat_10s_last_val - Total_VBAT;
					if(vbat_sub<VBAT_SOC_UPDATE)
					{
						//电压查表
						capacity_volt = (uint32_t)Total_VBAT*5000/16384;
						new_cap_rate2 = VbatToSoc((uint16_t)capacity_volt);
						if(capacity_volt<3200)
						{
							if((new_cap_rate2 <(g_sys_cap.val.re_cap_rate-5))&&(g_sys_cap.val.re_cap_rate>5))
							{
								sys_flags.val.re_cap_update_flag = true;
								stop_cap_update_val =5;
							}
						}
						else
						{
							if((new_cap_rate2 <(g_sys_cap.val.re_cap_rate-15))&&(g_sys_cap.val.re_cap_rate>15))
							{
								sys_flags.val.re_cap_update_flag = true;
								stop_cap_update_val =15;
							}
						}
					}
				}
				dch_delay =0;
			}
		}
		else
		{
			dch_delay =0;
		}
		
		
		if((nADC_CURRENT >CUR_CHG_01C)&&(nADC_CURRENT<CUR_CHG_02C))
		{
			if(chg_delay ==0)
			{
				vbat_10s_last_val = Total_VBAT;
			}
			chg_delay++;
			if(chg_delay >CAP_10S_DELAY)
			{
				if(vbat_10s_last_val< Total_VBAT)
				{
					vbat_sub = Total_VBAT - vbat_10s_last_val;
					if(vbat_sub<VBAT_SOC_UPDATE)
					{
						//电压查表
						capacity_volt = (uint32_t)Total_VBAT*5000/16384;
						new_cap_rate2 = VbatToSoc((uint16_t)capacity_volt);
						if((new_cap_rate2 <(g_sys_cap.val.re_cap_rate-15))&&(g_sys_cap.val.re_cap_rate>15))
						{
							sys_flags.val.re_cap_update_flag = true;
							stop_cap_update_val =15;
						}
					}
				}
				chg_delay =0;
			}
		}
		else
		{
			chg_delay =0;
		}
	}
	
	//假如出现电压补偿,那么进行补偿
	//最短1分钟一次电压补偿,补偿值是电量差/4，但是不低于1%
	if(sys_flags.val.re_cap_update_flag == true)
	{
		//电压查表
		capacity_volt = (uint32_t)Total_VBAT*5000/16384;
		new_cap_rate = VbatToSoc((uint16_t)capacity_volt);

		sys_flags.val.cap_update_end_flag = 1; //如果不清0,说明容量补偿完毕

		if((new_cap_rate -g_sys_cap.val.re_cap_rate>stop_cap_update_val)&&(new_cap_rate >g_sys_cap.val.re_cap_rate))
		{
			//差值除以4作为每次更新的容量值
			deta_cap_rate = new_cap_rate - g_sys_cap.val.re_cap_rate;
			deta_cap_rate>>=2;
			sys_flags.val.cap_update_end_flag = 0;
			if(deta_cap_rate ==0)
			{
				deta_cap_rate =1;
				sys_flags.val.cap_update_end_flag = 1;//低于4%的误差,即便是在补偿,认为补偿完了,可以做满电容量计算了
			}
			g_sys_cap.val.re_cap_rate_sum -= deta_cap_rate;//放电容量校准,容量回升,池子减少
			//deta_cap_rate = g_sys_cap.val.full_cap*deta_cap_rate/100;
			//g_sys_cap.val.cap_val = g_sys_cap.val.cap_val+deta_cap_rate;
			g_sys_cap.val.re_cap_rate = g_sys_cap.val.re_cap_rate + deta_cap_rate;//修正soc值20161010zzysoc3
		}
		if((g_sys_cap.val.re_cap_rate -new_cap_rate>stop_cap_update_val)&&(g_sys_cap.val.re_cap_rate >new_cap_rate))
		{
			deta_cap_rate = g_sys_cap.val.re_cap_rate -new_cap_rate;
			deta_cap_rate>>=2;
			sys_flags.val.cap_update_end_flag = 0;
			if(deta_cap_rate ==0)
			{
				deta_cap_rate =1;
				sys_flags.val.cap_update_end_flag = 1;
			}
			g_sys_cap.val.re_cap_rate_sum += deta_cap_rate;//放电容量校准,容量下降,池子增加
			//deta_cap_rate = g_sys_cap.val.full_cap*deta_cap_rate/100;
			//g_sys_cap.val.cap_val = g_sys_cap.val.cap_val-deta_cap_rate;
			g_sys_cap.val.re_cap_rate = g_sys_cap.val.re_cap_rate-deta_cap_rate;//修正soc值20161010zzysoc3
		}
		sys_flags.val.re_cap_update_flag = false;
	}
}

/****************************************************************************
FUNCTION		: FullCap_Update
DESCRIPTION		: 满电容量更新
INPUT			: None
OUTPUT			: None
NOTICE			: 跟在电压补偿之后,根据cap_update_end_flag的状态进行判断是否进入更新
DATE			: 2016/06/24
*****************************************************************************/
void FullCap_Update(void)
{
	uint32_t full_cap_temp;
	//假如允许最大容量更新,那么更新最大容量
	if(nADC_CURRENT <CUR_DCH_01C) //182 = 1A
	{
		if((nADC_TMONI_BAT_MAX <40)&&(nADC_TMONI_BAT_MIN >10))
		{
			temp_soc_err_cnt =0;
			if(sys_flags.val.cap_update_end_flag == 1)
			{
				
				if(soc_fcc_save ==0)
				{
					soc_fcc_save = 1;
					soc_fcc_reload = 0;
					fullcap_reload_delay =0;
					g_sys_cap.val.cap_val2 = g_sys_cap.val.cap_val;
					g_sys_cap.val.re_cap_rate2 = g_sys_cap.val.re_cap_rate;
				}
				else
				{
					if(soc_fcc_reload  == 1)
					{
						soc_fcc_save = 0;
					}
				}
			}
		}
		else
		{
			temp_soc_err_cnt++;
			if(temp_soc_err_cnt >3)
			{
				temp_soc_err_cnt =0;
				soc_fcc_save = 0;
			}
		}
	}
	else
	{
		if(nADC_CURRENT < CUR_CHG_01C)
		{
			if(soc_fcc_save == 1)
			{
				soc_fcc_reload = 1;
			}

			if((soc_fcc_save == 1)&&(sys_flags.val.cap_update_end_flag == 1))
			{
				full_cap_temp = g_sys_cap.val.full_cap >>3;
				if((g_sys_cap.val.cap_val2 - g_sys_cap.val.cap_val) > full_cap_temp)
				{
					soc_fcc_save = 0;//不管成功与否,都需要重新开始记录
					if(afe_flags.val.afe_uv_flag== 0) //如果没有发生欠压保护就按原数据进行，发生了就按发生前的数据//修正soc值20161009zzysoc2
					{
						uv_cap_val = g_sys_cap.val.cap_val;
						uv_re_cap_rate = g_sys_cap.val.re_cap_rate;
					}
					full_cap_temp = g_sys_cap.val.cap_val2 - uv_cap_val- g_sys_cap.val.cap_val3;//修正soc值20161010zzysoc4 增加cap_val3应为负值。
					full_cap_temp = full_cap_temp*100;
					full_cap_temp = full_cap_temp/(g_sys_cap.val.re_cap_rate2 - uv_re_cap_rate);
					
					//                    soc_fcc_save = 0;//不管成功与否,都需要重新开始记录
					//                    full_cap_temp = g_sys_cap.val.cap_val2 - g_sys_cap.val.cap_val- g_sys_cap.val.cap_val3;//修正soc值20161010zzysoc4 增加cap_val3应为负值。
					//                    full_cap_temp = full_cap_temp*100;
					//                    full_cap_temp = full_cap_temp/(g_sys_cap.val.re_cap_rate2 - g_sys_cap.val.re_cap_rate);

					if(full_cap_temp > g_sys_cap.val.full_cap)
					{
						if((full_cap_temp - g_sys_cap.val.full_cap) < BAT_CAP_3PS)
						{
							g_sys_cap.val.full_cap = full_cap_temp;
							
							EEPROM_Write_DATA(EEPROM_INDEX_FULL_CAP,g_sys_cap.val.full_cap,1);
						}
						else
						{
							if((full_cap_temp - g_sys_cap.val.full_cap) >BAT_CAP_30PS)
							{
								//                                sys_err_flags.val.fcc_update_err_flag =1;//XXY
							}
						}
					}
					else
					{
						if((g_sys_cap.val.full_cap - full_cap_temp) < BAT_CAP_3PS)
						{
							g_sys_cap.val.full_cap = full_cap_temp;
							EEPROM_Write_DATA(EEPROM_INDEX_FULL_CAP,g_sys_cap.val.full_cap,1);
						}
						else
						{
							if((full_cap_temp - g_sys_cap.val.full_cap) >BAT_CAP_30PS)
							{
								//                                sys_err_flags.val.fcc_update_err_flag =1;//XXY
							}
						}
					}
				}
				fullcap_reload_delay++;
				if(fullcap_reload_delay >20)
				{
					fullcap_reload_delay =0;
					soc_fcc_save = 0;//不管成功与否,都需要重新开始记录
				}

			}
		}
		else
		{
			soc_fcc_save = 0;
			soc_fcc_reload =0;
		}
	}
}

/****************************************************************************
FUNCTION		: SOC_FLASH_Save
DESCRIPTION		: 容量存储，在一个BLOCK,32WORD里面循环存储
INPUT			: None
OUTPUT			: None
NOTICE			:
DATE			: 2016/06/24
*****************************************************************************/
void SOC_FLASH_Save(void)
{
	if((g_sys_cap.val.re_cap_rate > g_sys_cap.val.re_cap_rate_old)&&((g_sys_cap.val.re_cap_rate - g_sys_cap.val.re_cap_rate_old)>0))
	{
		cap_save_delay_cnt++;
		if(cap_save_delay_cnt >10)
		{
			cap_save_delay_cnt =0;

			g_sys_cap.val.re_cap_rate_old = g_sys_cap.val.re_cap_rate;
			EEPROM_Write_DATA(EEPROM_INDEX_CAP_VAL,g_sys_cap.val.re_cap_rate,0);
		}
	}
	if((g_sys_cap.val.re_cap_rate < g_sys_cap.val.re_cap_rate_old)&&((g_sys_cap.val.re_cap_rate_old - g_sys_cap.val.re_cap_rate)>0))
	{
		cap_save_delay_cnt ++;
		if(cap_save_delay_cnt >10)
		{
			cap_save_delay_cnt =0;
			g_sys_cap.val.re_cap_rate_old = g_sys_cap.val.re_cap_rate;
			EEPROM_Write_DATA(EEPROM_INDEX_CAP_VAL,g_sys_cap.val.re_cap_rate,0);
		}
	}
}

/****************************************************************************
FUNCTION		: BatCycleProc
DESCRIPTION		: 循环次数记录
INPUT			: None
OUTPUT			: None
NOTICE			: 循环次数分为3类,常规循环:累计超过7%的放电容量,记录1次.
NOTICE          : 深度放电:电压低于3V|连续放电超过50%,记录一次.深度充电:电压高于4.1V|连续充电超过50%记录一次.
DATE			: 2016/06/27
*****************************************************************************/
void BatCycleProc(void)
{
	uint8_t soc_rate;
	//常规循环
	if((nADC_CURRENT <CURRENT_DCH_05A)&&(g_sys_cap.val.cycle_record_flag ==0))
	{
		if(g_sys_cap.val.re_cap_rate_record <g_sys_cap.val.re_cap_rate)
		{
			g_sys_cap.val.re_cap_rate_record = g_sys_cap.val.re_cap_rate;
		}
		else
		{
			//在放电状态,进行上次放电容量 - 当前放电容量 ,大于等于1%自然会存入
			soc_rate =g_sys_cap.val.re_cap_rate_record - g_sys_cap.val.re_cap_rate;
			g_sys_cap.val.re_cap_rate_sum += soc_rate;
			g_sys_cap.val.re_cap_rate_record = g_sys_cap.val.re_cap_rate;
			//一般来说,电压校准不会超过这个值,如果超过了7%误差值,只能当做是一次放电.除非电压补偿超过了14%
			//如果为负,那么需要等到转正了才能减少--电压补偿原因
			if(g_sys_cap.val.re_cap_rate_sum >6)
			{
				if(g_sys_cap.val.re_cap_rate_sum >100)//数据异常,不应该记录,软件防死
				{
					g_sys_cap.val.re_cap_rate_sum =0;
				}
				//                soc_rate= g_sys_cap.val.re_cap_rate_sum/7;
				g_sys_cap.val.bat_cycle_cnt+= 1;
				g_sys_cap.val.re_cap_rate_sum =0;//除了上电不进行清0
				EEPROM_Write_DATA(EEPROM_INDEX_CYCLE,g_sys_cap.val.bat_cycle_cnt,1);
				g_sys_cap.val.cycle_record_flag = g_sys_cap.val.re_cap_rate;//记录一次以后,不再记录,等待下次记录时机，为0也不担心
			}
		}
	}
	else
	{
		g_sys_cap.val.re_cap_rate_record =0;//不在放电归0,保证放电可以正常记录
	}
	
	
	if(nADC_CURRENT >CUR_CHG_01C)
	{
		if(g_sys_cap.val.cycle_record_flag >0)
		{
			//在记录循环次数以后,累计充电时间超过10分钟,清除记录标志,方法2.
			chg_record_cnt++;
			if(chg_record_cnt >2400)  //240 = 1min
			{
				g_sys_cap.val.cycle_record_flag =0;
				chg_record_cnt =0;
			}
		}
	}
	
	//深度放电
	if(nADC_CURRENT <CURRENT_DCH_05A)
	{
		if(g_sys_cap.val.deep_cycle_rate_record <g_sys_cap.val.re_cap_rate)
		{
			g_sys_cap.val.deep_cycle_rate_record = g_sys_cap.val.re_cap_rate;
		}
		soc_rate = g_sys_cap.val.deep_cycle_rate_record -g_sys_cap.val.re_cap_rate;
		g_sys_cap.val.deep_rate_sum += soc_rate;
		if(g_sys_cap.val.deep_rate_sum >50)
		{
			g_sys_cap.val.deep_dch_cycle_cnt++;
			g_sys_cap.val.deep_rate_sum =0;//20160815  AID 增加清0
			EEPROM_Write_DATA(EEPROM_INDEX_DEEP_DCH_CYCLE,g_sys_cap.val.deep_dch_cycle_cnt,1);
		}
	}
	else
	{
		g_sys_cap.val.deep_cycle_rate_record =0;
		if(nADC_CURRENT >CUR_CHG_01C)
		{
			g_sys_cap.val.deep_rate_sum =0;
		}
	}
	if(nADC_CELL_MIN <VCELL_DEEP_DCH)
	{
		if(g_sys_cap.val.deep_dch_volt_delay <200)
		{
			g_sys_cap.val.deep_dch_volt_delay++;
		}
		if(g_sys_cap.val.deep_dch_volt_delay>50)
		{
			if(g_sys_cap.val.deep_dch_volt_delay <100)
			{
				g_sys_cap.val.deep_dch_volt_delay = 200;
				g_sys_cap.val.deep_dch_cycle_cnt++;
				EEPROM_Write_DATA(EEPROM_INDEX_DEEP_DCH_CYCLE,g_sys_cap.val.deep_dch_cycle_cnt,1);
			}
		}
	}
	else
	{
		if(nADC_CELL_MIN >VCELL_DEEP_DCH_CLEAR)
		{
			g_sys_cap.val.deep_dch_volt_delay =0;
		}
		else
		{
			if(g_sys_cap.val.deep_dch_volt_delay<200)
			{
				g_sys_cap.val.deep_dch_volt_delay =0;
			}
		}
	}
	//深度充电
	if(nADC_CURRENT >CUR_CHG_01C)
	{
		if(g_sys_cap.val.deep_cycle_chg_record >g_sys_cap.val.re_cap_rate)
		{
			g_sys_cap.val.deep_cycle_chg_record = g_sys_cap.val.re_cap_rate;
		}
		soc_rate = g_sys_cap.val.re_cap_rate -g_sys_cap.val.deep_cycle_chg_record;
		g_sys_cap.val.deep_rate_chgsum += soc_rate;
		if(g_sys_cap.val.deep_rate_chgsum >50)
		{
			g_sys_cap.val.deep_chg_cycle_cnt++;
			g_sys_cap.val.deep_rate_chgsum =0;//20160815  AID 增加清0
			EEPROM_Write_DATA(EEPROM_INDEX_DEEP_CHG_CYCLE,g_sys_cap.val.deep_chg_cycle_cnt,1);
		}
	}
	else
	{
		g_sys_cap.val.deep_cycle_chg_record =100;
		if(nADC_CURRENT <CURRENT_DCH_05A)
		{
			g_sys_cap.val.deep_rate_chgsum =0;
		}
	}
	
	if(nADC_CELL_MAX >VCELL_DEEP_HIGH_CHG)
	{
		if(g_sys_cap.val.deep_chg_volt_delay <200)
		{
			g_sys_cap.val.deep_chg_volt_delay++;
		}
		if(g_sys_cap.val.deep_chg_volt_delay>50)
		{
			if(g_sys_cap.val.deep_chg_volt_delay <100)
			{
				g_sys_cap.val.deep_chg_volt_delay = 200;
				g_sys_cap.val.deep_chg_cycle_cnt++;
				EEPROM_Write_DATA(EEPROM_INDEX_DEEP_CHG_CYCLE,g_sys_cap.val.deep_chg_cycle_cnt,1);
			}
		}
	}
	else
	{
		if(nADC_CELL_MAX <VCELL_DEEP_CHG_CLEAR)
		{
			g_sys_cap.val.deep_chg_volt_delay =0;
		}
		else
		{
			if(g_sys_cap.val.deep_chg_volt_delay<200)
			{
				g_sys_cap.val.deep_chg_volt_delay =0;
			}
		}
	}
}