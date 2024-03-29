/*
 * protect.c
 *
 * Created: 2017/2/7 16:21:14
 *  Author: chenjiawei
 */ 

#include "protect.h"
#include "afe_wr.h"
#include "ad_dat.h"
#include "led.h"
#include "soc.h"
#include "history.h"
#include "gpio.h"


uint16_t OCC_TIMEOUT = 0;
uint16_t OCD_TIMEOUT = 0;

uint8_t soft_cp_cnt = 0;
uint8_t soft_cp_dis_cnt = 0;
uint8_t soft_occ_cnt = 0;
uint8_t soft_dp_cnt = 0;
uint8_t soft_dp_dis_cnt = 0;
uint8_t soft_ocd_cnt=0;

volatile uint16_t sys_250ms_cnt = 0;
uint16_t cell_err_cnt=0;
uint16_t cell_err2_cnt=0;
uint16_t cell_err3_cnt=0;            //zzy?
uint16_t cell_shdn_cnt=0;            //zzy?
uint16_t cell_alarm_cnt=0;

volatile uint8_t Balanc_index = 0;
uint8_t soft_pcb_ot_cnt =0;

uint8_t advance_delay=0;

//添加异常flag计数变量
//过充电
uint16_t OCHG_P3_cnt;
uint16_t OCHG_P3_on_cnt;
uint16_t OCHG_P3_off_cnt;
uint16_t OCHG_P4_cnt;
uint16_t OCHG_P4_on_cnt;
uint16_t OCHG_P4_off_cnt;
uint16_t OCHG_P5_cnt;
uint16_t OCHG_P5_on_cnt;
uint16_t OCHG_P5_off_cnt;
//过放电
uint16_t ODCH_P3_cnt;
uint16_t ODCH_P3_on_cnt;
uint16_t ODCH_P3_off_cnt;
uint16_t ODCH_P4_cnt;
uint16_t ODCH_P4_on_cnt;
uint16_t ODCH_P4_off_cnt;
uint16_t ODCH_P5_cnt;
uint16_t ODCH_P5_on_cnt;
uint16_t ODCH_P5_off_cnt;
//充电过电流
uint16_t OCC_P3_cnt;
uint16_t OCC_P3_on_cnt;
uint16_t OCC_P3_off_cnt;
uint16_t OCC_P4_cnt;
uint16_t OCC_P4_on_cnt;
uint16_t OCC_P4_off_cnt;
uint16_t OCC_P5_cnt;
uint16_t OCC_P5_on_cnt;
uint16_t OCC_P5_off_cnt;
//放电过电流
uint16_t ODC_P3_cnt;
uint16_t ODC_P3_on_cnt;
uint16_t ODC_P3_off_cnt;
uint16_t ODC_P4_cnt;
uint16_t ODC_P4_on_cnt;
uint16_t ODC_P4_off_cnt;
uint16_t ODC_P5_cnt;
uint16_t ODC_P5_on_cnt;
uint16_t ODC_P5_off_cnt;
//过温
uint16_t OTEMP_P3_cnt;
uint16_t OTEMP_P3_on_cnt;
uint16_t OTEMP_P3_off_cnt;
uint16_t OTEMP_P4_cnt;
uint16_t OTEMP_P4_on_cnt;
uint16_t OTEMP_P4_off_cnt;
uint16_t OTEMP_P5_cnt;
uint16_t OTEMP_P5_on_cnt;
uint16_t OTEMP_P5_off_cnt;

uint16_t DCH_Inhibit_Temp_cnt;
uint16_t DCH_Inhibit_Temp_on_cnt;
uint16_t DCH_Inhibit_Temp_off_cnt;
uint16_t CHG_Inhibit_Temp_cnt;
uint16_t CHG_Inhibit_Temp_on_cnt;
uint16_t CHG_Inhibit_Temp_off_cnt;

/****************************************************************************
FUNCTION		: AFE_Control
DESCRIPTION		: AFE信息处理控制
INPUT			: None
OUTPUT			: None
UPDATE			:
DATE			: 2016/06/24
*****************************************************************************/
void AFE_Control(void)
{
	HardwareProtection();//硬件保护
	SoftwareProtection();//软件保护
	SoftMeansureControl(); //软件采集保护

	//Cell_Balance(); //均衡
	//一切都正常,但是PCB过温了,说明可能出现了反接,充放电管都关闭,直到温度降低回85℃
	PCB_Protect();
}

/****************************************************************************
FUNCTION		: HardwareProtection
DESCRIPTION		: 硬件保护函数
INPUT			: None
OUTPUT			: None
UPDATE			:
DATE			: 2016/06/24
*****************************************************************************/
void HardwareProtection(void)
{
	//开机设置电压保护值,未成功持续设置,直到成功
	if(sys_flags.val.afe_set_hard_protect_end == 0)
	{
		if(AFE_HardwareProtection_Write() ==0)
		{
			sys_flags.val.afe_set_hard_protect_end =1;
			afe_flags.val.afe_set_volt_protect_err_flag = 0;
		}
		else
		{
			afe_flags.val.afe_set_volt_protect_err_flag = 1;
		}
	}
	AFE_HardwareProtection_Read(); //读取SPI保护信息
	
	//AFE读取的信息处理  OCC OCD SCD
	if((afe_flags.val.afe_occ_flag == 1)||(afe_flags.val.afe_ov_flag==1))
	{
		sys_states.val.sys_chg_on =0;
	}
	else
	{
		sys_states.val.sys_chg_on =1;
	}
	if((afe_flags.val.afe_ocd_flag == 1)||(afe_flags.val.afe_scd_flag == 1)||(afe_flags.val.afe_uv_flag == 1))
	{
		sys_states.val.sys_dch_on =0;
		afe_flags.val.afe_dfrv_autoprotect_flag = 1;
		if(afe_flags.val.afe_uv_flag ==1)               //zzy?
		{
			afe_flags.val.afe_uv_lock_flag = 1;
		}
	}
	else
	{
		if((afe_flags.val.afe_ocd_flag == 0)&&(afe_flags.val.afe_scd_flag == 0)&&(afe_flags.val.afe_uv_flag == 0))
		{
			afe_flags.val.afe_dfrv_autoprotect_flag = 0;
		}
		sys_states.val.sys_dch_on =1;
	}
}

/****************************************************************************
FUNCTION		: SoftwareProtection
DESCRIPTION		: 软件保护
INPUT			: None
OUTPUT			: None
NOTICE			: 先执行硬件保护,再进行软件保护
DATE			: 2016/06/24
*****************************************************************************/
void SoftwareProtection(void)
{
	//CHG PROTECT
	if(sys_states.val.soft_chg_protect ==0)
	{
		if(sys_states.val.sys_chg_on == 1)
		{
			if((nADC_TMONI_BAT_MAX > TEMP_CHG_HIGH_PROTECT)||(nADC_TMONI_BAT_MIN < TEMP_CHG_LOW_PROTECT))
			{
				soft_cp_cnt++;
				if(soft_cp_cnt >PROTECT_DELAY_3S)  //循环8次,250*8 = 2S
				{
					soft_cp_cnt =0;
					sys_states.val.soft_chg_protect =1;
					sys_states.val.chg_temp_protect = 1;
					//BatteryState.val.CHG_Inhibit_Temp = 1; //设置 充电禁止温度 
				}
			}
			else
			{
				soft_cp_cnt =0;
				if(nADC_CURRENT >0) //充电
				{
					if(nADC_CURRENT >CURRENT_CHG_STATE)
					{
						if(nADC_CURRENT >CURRENT_CHGOC_PROTECT)
						{
							soft_occ_cnt++;
							if(soft_occ_cnt >PROTECT_DELAY_1S) // 4*250ms = 1S//已取消zzy沿用云海PROTECT_DELAY_1S改1
							{
								soft_occ_cnt =0;
								OCC_TIMEOUT =0;
								sys_states.val.sys_software_occ = 1;
								sys_states.val.soft_chg_protect =1;
							}
						}
						else
						{
							soft_occ_cnt =0;
						}
						sys_states.val.sys_chg_state =1;
						sys_states.val.sys_dch_state =0;
					}
					else
					{
						soft_occ_cnt =0;
						sys_states.val.sys_chg_state =0;
					}
				}
				else
				{
					sys_states.val.sys_chg_state =0;
				}
			}
		}
	}
	else
	{
		if(sys_states.val.sys_software_occ == 1)
		{
			soft_cp_dis_cnt = 0;
			if(OCC_TIMEOUT >PROTECT_DELAY_5S) // 250ms * 4 = 1S 5S = 20
			{
				OCC_TIMEOUT =0;
				sys_states.val.sys_software_occ =0;
				sys_states.val.soft_chg_protect =0;
			}
		}
		else
		{
			if((nADC_TMONI_BAT_MAX < TEMP_CHG_HIGH_ALARM)&&(nADC_TMONI_BAT_MIN > TEMP_CHG_LOW_ALARM))
			{
				sys_states.val.soft_chg_protect =0;
				sys_states.val.chg_temp_protect = 0;
				soft_cp_dis_cnt++;
				if (soft_cp_dis_cnt > PROTECT_DELAY_3S)
				{
					soft_cp_dis_cnt = 0;
					//BatteryState.val.CHG_Inhibit_Temp = 0; //解除 充电禁止温度 
				}
			}
		}
	}
	
	//DCH PROTECT
	if(sys_states.val.soft_dch_protect == 0)
	{
		if(sys_states.val.sys_dch_on == 1)
		{
			if((nADC_TMONI_BAT_MAX > TEMP_DCH_HIGH_PROTECT)||(nADC_TMONI_BAT_MIN < TEMP_DCH_LOW_PROTECT))
			{
				soft_dp_cnt++;
				if(soft_dp_cnt >PROTECT_DELAY_3S) //防抖延迟2S
				{
					soft_dp_cnt =0;
					sys_states.val.soft_dch_protect =1;
					sys_states.val.dch_temp_protect = 1;
					//BatteryState.val.DCH_Inhibit_Temp = 1; //设置 放电禁止温度 
				}
			}
			else
			{
				soft_dp_cnt =0;
				if(nADC_CURRENT <0)
				{
					if(nADC_CURRENT <CURRENT_DCH_STATE)
					{
						sys_states.val.sys_dch_state =1;
						sys_states.val.sys_chg_state =0;
						if(nADC_CURRENT <CURRENT_DCHOC_PROTECT)
						{
							soft_ocd_cnt++;
							if(soft_ocd_cnt >PROTECT_DELAY_1S)  //防抖延迟1S  //zzy20161025 PROTECT_DELAY_1S 改为1 沿用神州云海方案已取消
							{
								soft_ocd_cnt =0;
								OCD_TIMEOUT =0;
								sys_states.val.sys_software_odc = 1;
								sys_states.val.soft_dch_protect =1;
							}
						}
						else
						{
							soft_ocd_cnt =0;
						}
					}
					else
					{
						soft_ocd_cnt =0;
						sys_states.val.sys_dch_state =0;
					}
				}
				else
				{
					sys_states.val.sys_dch_state =0;
				}
			}
		}
	}
	else
	{
		if(sys_states.val.sys_software_odc == 1)
		{
			soft_dp_dis_cnt = 0;
			if(OCD_TIMEOUT >PROTECT_DELAY_5S)// 250ms * 4 = 1S 5S = 20
			{
				OCD_TIMEOUT =0;
				sys_states.val.soft_dch_protect =0;
				sys_states.val.sys_software_odc = 0;
			}
		}
		else
		{
			if((nADC_TMONI_BAT_MAX < TEMP_DCH_HIGH_ALARM)&&(nADC_TMONI_BAT_MIN > TEMP_DCH_LOW_ALARM))
			{
				sys_states.val.soft_dch_protect =0;
				sys_states.val.dch_temp_protect = 0;
				soft_dp_dis_cnt++ ;
				if (soft_dp_dis_cnt > PROTECT_DELAY_3S)
				{
					soft_dp_dis_cnt = 0;
					//BatteryState.val.DCH_Inhibit_Temp = 0; //解除 放电禁止温度 
				}
			}
		}
	}
}

/****************************************************************************
FUNCTION		: SoftMeansureControl
DESCRIPTION		: 软件电芯电压采集处理
INPUT			: None
OUTPUT			: None
NOTICE			:
DATE			: 2016/06/24
*****************************************************************************/
void SoftMeansureControl(void)
{
	uint16_t shdn_on_cnt;
	uint16_t cell_alarm_on_cnt;
	uint16_t cell_err_on_cnt;
	uint16_t cell_err2_on_cnt;
	uint16_t cell_err3_on_cnt;
		
	// 压差超过500mV提示电芯故障
	if((nADC_CELL_MAX - nADC_CELL_MIN)>VCELL_SUB_0V5)
	{
		cell_err_on_cnt = sys_250ms_cnt - cell_err_cnt;
		if(cell_err_on_cnt >PROTECT_DELAY_30S)
		{
			//            sys_err_flags.val.cell_err_flag = 1; //XXY
		}
	}
	else
	{
		cell_err_cnt =sys_250ms_cnt;
	}
	// 最低电压低于1.5V提示电芯故障
	if(nADC_CELL_MIN<VCELL_ERR)
	{
		cell_err2_on_cnt =sys_250ms_cnt - cell_err2_cnt;
		if(cell_err2_on_cnt >PROTECT_DELAY_30S)
		{
			sys_err_flags.val.cell_err_flag = 1;
		}
	}
	else
	{
		cell_err2_cnt =sys_250ms_cnt;
	}
	// 最高电压高于4.3V提示电芯故障改为
	if(nADC_CELL_MAX>VCELL_HIGH_ERR)    //zzy if(nADC_CELL_MIN>VCELL_HIGH_ERR)
	{
		cell_err3_on_cnt =sys_250ms_cnt - cell_err3_cnt;
		if(cell_err3_on_cnt >PROTECT_DELAY_30S)
		{
			sys_err_flags.val.cell_err_flag = 1;  //XXY zzy
		}
	}
	else
	{
		cell_err3_cnt =sys_250ms_cnt;
	}
	// 低于2.5V进入SHDN
	if(nADC_CELL_MIN < VCELL_SHDN)
	{
		shdn_on_cnt = sys_250ms_cnt - cell_shdn_cnt;  // 250ms
		if(shdn_on_cnt >PROTECT_DELAY_60S)
		{
			//            SHDN_SetHigh();
		}
	}
	else
	{
		cell_shdn_cnt = sys_250ms_cnt;
	}
	
	if(nADC_CELL_MIN < VCELL_LOW_ALARM)
	{
		cell_alarm_on_cnt = sys_250ms_cnt - cell_alarm_cnt;  // 250ms
		if(cell_alarm_on_cnt>PROTECT_DELAY_2S)
		{
			sys_flags.val.cell_low_alarm_flag = 1;
		}
	}
	else
	{
		cell_alarm_cnt = sys_250ms_cnt;
		sys_flags.val.cell_low_alarm_flag = 0;
	}
	if(nADC_CELL_MAX > VCELL_HIGH_ALARM)
	{
		cell_alarm_on_cnt = sys_250ms_cnt - cell_alarm_cnt;  // 250ms
		if(cell_alarm_on_cnt>PROTECT_DELAY_2S)
		{
			sys_flags.val.cell_high_alarm_flag = 1;//原本写错了吧zzy sys_flags.val.cell_low_alarm_flag = 1;
		}
	}
	else
	{
		sys_flags.val.cell_high_alarm_flag = 0;
		
	}

	//添加异常flag计数函数
	Abnormal_Flag();
	
}

/****************************************************************************
FUNCTION		: Cell_Balance
DESCRIPTION		: 均衡控制
INPUT			: None
OUTPUT			: None
NOTICE			:   关闭OV/UV功能---开启均衡--选择需要均衡节数---开启均衡
 * 
 * 
 * 
DATE			: 2016/06/24
*****************************************************************************/
void Cell_Balance(void)
{
    if((nADC_CELL_MAX > VCELL_BALANCE_Open) && (nADC_CELL_MAX-nADC_CELL_MIN > VCELL_BALANCE_START))
    {
        afe_flags.val.afe_CellBalance = 1;  //开启均衡标志
		BatteryState.val.BalanceState = 1 ; // 电池状态设定为均衡模式开
    }
    //关闭均衡状态：最高电压小于关闭电压4.0V、压差小于结束压差、处于放电状态
    if((nADC_CELL_MAX < VCELL_BALANCE_Close) 
        || ((nADC_CELL_MAX-nADC_CELL_MIN) < VCELL_BALANCE_END)
        ||(sys_states.val.sys_dch_state == 1) )
    {
        afe_flags.val.afe_CellBalance = 0;  //关闭均衡标志
		BatteryState.val.BalanceState = 0 ; // 电池状态设定为均衡模式关
    }
    if(afe_flags.val.afe_CellBalance == 1)
    {
        Balanc_index ++;
        if(Balanc_index < 25) //关均衡。判断电压
        {
            Cells_Bal_Close();
            if(Balanc_index >20)
            {
                Cells_Bal_Judge();// CELL BALANCE
            }
        }                
        else if ((Balanc_index < 75) )
        {
            g_bal_state.VAL = g_bal_need.VAL & 0x5555; //关闭偶数位
            Cells_Bal_Open();
        }
        else if(Balanc_index > 80)
        {
            g_bal_state.VAL = g_bal_need.VAL & 0xAAAA; //关闭奇数位
            Cells_Bal_Open();
            if(Balanc_index >130)
            {
                Balanc_index = 0;
            }
        }
        else
        {
            Cells_Bal_Close();
        }
                
    }
    else
    {
        Cells_Bal_Close();
    }
}

/****************************************************************************
FUNCTION		: PCB_Protect
DESCRIPTION		: PCB过温保护
INPUT			: None
OUTPUT			: None
NOTICE			: 独立的强制关闭类保护,只负责关闭,不负责打开,并置位相应保护标志位,这样不会容易逻辑混乱
DATE			: 2016/06/24
*****************************************************************************/
void PCB_Protect(void)
{
	if(nADC_TMONI_PCB_MAX > TEMP_PCB_PROTECT)
	{
		soft_pcb_ot_cnt++;
		if(soft_pcb_ot_cnt >PROTECT_DELAY_2S) //防抖延迟2S
		{
			soft_pcb_ot_cnt =9; //保持2S位置,这样2S的--会刚好解除
			sys_states.val.sys_pcb_ot_flag =1;
		}
	}
	else
	{
		if(nADC_TMONI_PCB_MAX <TEMP_PCB_PROTECT_CLEAR)
		{
			if(soft_pcb_ot_cnt >0)
			{
				soft_pcb_ot_cnt--;
			}
			else
			{
				sys_states.val.sys_pcb_ot_flag =0;
			}
		}
	}
}

/****************************************************************************
FUNCTION		: Sys_250ms_tick
DESCRIPTION		: 系统250ms计数
INPUT			: None
OUTPUT			: None
NOTICE			:
DATE			: 2016/06/24
*****************************************************************************/
void Sys_250ms_tick(void)
{
	sys_250ms_cnt++;
	advance_delay++;

	SOC();
	if(afe_flags.val.afe_ocd_flag == 1)
	{
		AFE_OC_DELAY_CNT++;
	}
	else
	{
		if(AFE_OC_DELAY_CNT!=45)//zzy20161026 仅仅只有当插入充电器才会达到45，保存不变，进入恢复
		{
			AFE_OC_DELAY_CNT =0;
		}
	}
	if(afe_flags.val.afe_scd_flag == 1)
	{
		AFE_SCD_DELAY_CNT++;
	}
	else
	{
		if(AFE_SCD_DELAY_CNT!=45)//仅仅只有当插入充电器才会达到45，保存不变，进入恢复
		{
			AFE_SCD_DELAY_CNT =0;
		}
	}
	if(afe_flags.val.afe_occ_flag == 1)
	{
		AFE_OCC_DELAY_CNT++;
	}
	else
	{
		if(AFE_OCC_DELAY_CNT!=45)//仅仅只有当拔插充电器与短按键时才会达到45，保存不变，进入恢复
		{
			AFE_OCC_DELAY_CNT =0;
		}
	}
	if(sys_states.val.sys_software_odc == 1)
	{
		OCD_TIMEOUT++;
	}
	else
	{
		OCD_TIMEOUT =0;
	}
	if(sys_states.val.sys_software_occ == 1)
	{
		OCC_TIMEOUT++;
	}
	else
	{
		OCC_TIMEOUT =0;
	}
	
	//SysLED_Display();
}

/****************************************************************************
FUNCTION		: SOC
DESCRIPTION		: 容量计量（库伦法）
INPUT			: None
OUTPUT			: None
NOTICE			: TMIER里面记录
DATE			: 2016/06/24
*****************************************************************************/
void SOC(void)
{

	int tmp_cap=0;
	int32_t cur;
	cur = nADC_CURRENT*180000;
	cur >>= 15;
	g_sys_cap.val.cap_cnt+=cur;
	tmp_cap=(int)(g_sys_cap.val.cap_cnt/CAP_CNT_VAL);
	g_sys_cap.val.cap_cnt=g_sys_cap.val.cap_cnt-(long)(tmp_cap)*CAP_CNT_VAL;
	
    //添加计算累积充电量和累积放电量
	if (tmp_cap >= 0)
	{
		CHG_Val = CHG_Val + tmp_cap;
		if (CHG_Val - CHG_Val_Bak >= 1000 )
		{
			//Write_Time_or_mAh(CHG_Val,CHG_FLAG);
			CHG_Val_Bak = CHG_Val;
		}
		//BatteryState.val.ActionState = 2; // 电池状态设定为充电
	}
	else
	{
		DCH_Val = DCH_Val - tmp_cap;
		if (DCH_Val - DCH_Val_Bak >=1000 )
		{
			//Write_Time_or_mAh(DCH_Val,DCH_FLAG);
			DCH_Val_Bak = DCH_Val;
		}
		//BatteryState.val.ActionState = 3; // 电池状态设定为放电
	}
	
	if(tmp_cap>-30)
	{
		g_sys_cap.val.cap_val+=tmp_cap;
		
	}
	//    if(g_sys_cap.val.cap_val <0)
	//    {
	//        g_sys_cap.val.cap_val =0;
	//    }
	if(g_sys_cap.val.cap_val >0)//修正soc值20161010zzysoc4 如果容量值小于等于零，就使用 g_sys_cap.val.cap_val3储存负容量值
	{
		g_sys_cap.val.cap_val3 = 0;
	}
	else
	{
		g_sys_cap.val.cap_val3 +=  g_sys_cap.val.cap_val ;
		g_sys_cap.val.cap_val =0;
		if(g_sys_cap.val.cap_val3  <-1200 )
		{
			g_sys_cap.val.cap_val3  = -1200;
		}
	}
}


void Flag_Process(void)
{
	SOC_Flag();    //更新SOC标志位
		
}

void SOC_Flag(void)
{
	//更新SOC状态
	if ( g_sys_cap.val.re_cap_rate == 100 )
	{
		BatteryState.val.SocState = 1 ; //SOC状态设置为满充电
	}
	else if ( g_sys_cap.val.re_cap_rate == 0 )
	{
		BatteryState.val.SocState = 2 ; //SOC状态设置放电终止
	}
	else 
	{
		if ( BatteryState.val.SocState == 1 )
		{
			if (g_sys_cap.val.re_cap_rate < (100-CHG_Release_Soc))
			{
				BatteryState.val.SocState = 0 ; //SOC状态设置为其他
			} 
		}
		else if (BatteryState.val.SocState == 2)
		{
			if (g_sys_cap.val.re_cap_rate > 5)
			{
				BatteryState.val.SocState = 0 ; //SOC状态设置为其他
			}
		}
		else
		{
			BatteryState.val.SocState = 0 ; //SOC状态设置为其他
		}
	}
}


void Abnormal_Flag(void)
{
	OCHG_Flag();
	ODCH_Flag();
	OCC_Flag();
	ODC_Flag();
	OTEMP_Flag();	
	Stop_Flag();
	Inhibit_Flag();
	Action_Flag();
}

void OCHG_Flag(void)
{
	if (nADC_CELL_MAX > OCHG_P3_VOL)    //过充电保护3
	{
		OCHG_P3_cnt = sys_250ms_cnt - OCHG_P3_on_cnt;
		if (OCHG_P3_cnt > PROTECT_DELAY_2S)
		{
			AbnormalState.val.OCHG_Protect3 = 1;
		}
	}
	else
	{
		OCHG_P3_on_cnt = sys_250ms_cnt;
		
	}
	
	if (nADC_CELL_MAX < OCHG_D3_VOL)    //解除过充电保护3
	{
		OCHG_P3_cnt = sys_250ms_cnt - OCHG_P3_off_cnt;
		if (OCHG_P3_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.OCHG_Protect3 = 0;
		}
	}
	else
	{
		OCHG_P3_off_cnt = sys_250ms_cnt;
	}
	
	
	if (nADC_CELL_MAX > OCHG_P4_VOL)    //过充电保护4
	{
		OCHG_P4_cnt = sys_250ms_cnt - OCHG_P4_on_cnt;
		if (OCHG_P4_cnt > PROTECT_DELAY_2S)
		{
			AbnormalState.val.OCHG_Protect4 = 1;
		}
	}
	else
	{
		OCHG_P4_on_cnt = sys_250ms_cnt;

	}
	
	if (nADC_CELL_MAX < OCHG_D4_VOL)    //解除过充电保护4
	{
		OCHG_P4_cnt = sys_250ms_cnt - OCHG_P4_off_cnt;
		if (OCHG_P4_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.OCHG_Protect4 = 0;
		}
	}
	else
	{
		OCHG_P4_off_cnt = sys_250ms_cnt;
	}
	
	if (nADC_CELL_MAX > OCHG_P5_VOL)    //过充电保护5
	{
		//OCHG_P5_off_cnt = sys_250ms_cnt;
		OCHG_P5_cnt = sys_250ms_cnt - OCHG_P5_on_cnt;
		if (OCHG_P5_cnt > PROTECT_DELAY_2S)
		{
			AbnormalState.val.OCHG_Protect5 = 1;
		}
	}
	else
	{
		OCHG_P5_on_cnt = sys_250ms_cnt;
		//OCHG_P5_cnt = sys_250ms_cnt - OCHG_P5_off_cnt;
		//if (OCHG_P5_cnt > PROTECT_DELAY_3S)
		//{
			////AbnormalState.val.OCHG_Protect5 = 0;
		//}
	}
}

void ODCH_Flag(void)
{
	if (nADC_CELL_MIN < ODCH_P3_VOL)    //过放电保护3
	{
		ODCH_P3_cnt = sys_250ms_cnt - ODCH_P3_on_cnt;
		if (ODCH_P3_cnt > PROTECT_DELAY_2S)
		{
			AbnormalState.val.ODCH_Protect3 = 1;
		}
	}
	else
	{
		ODCH_P3_on_cnt = sys_250ms_cnt;
	}
	
	if (nADC_CELL_MIN > ODCH_D3_VOL)    //解除过放电保护3
	{
		ODCH_P3_cnt = sys_250ms_cnt - ODCH_P3_off_cnt;
		if (ODCH_P3_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.ODCH_Protect3 = 0;
		}
	}
	else
	{
		ODCH_P3_off_cnt = sys_250ms_cnt;
	}
	
	if (nADC_CELL_MIN < ODCH_P4_VOL)    //过放电保护4
	{
		ODCH_P4_cnt = sys_250ms_cnt - ODCH_P4_on_cnt;
		if (ODCH_P4_cnt > PROTECT_DELAY_2S)
		{
			AbnormalState.val.ODCH_Protect4 = 1;
		}
	}
	else
	{
		ODCH_P4_on_cnt = sys_250ms_cnt;
	}
	
	if (nADC_CELL_MIN > ODCH_D4_VOL)    //解除过放电保护4
	{
		ODCH_P4_cnt = sys_250ms_cnt - ODCH_P4_off_cnt;
		if (ODCH_P4_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.ODCH_Protect4 = 0;
		}
	}
	else
	{
		ODCH_P4_off_cnt = sys_250ms_cnt;
	}
	
	if (nADC_CELL_MIN < ODCH_P5_VOL)    //过放电保护5
	{
		//ODCH_P5_off_cnt = sys_250ms_cnt;
		ODCH_P5_cnt = sys_250ms_cnt - ODCH_P5_on_cnt;
		if (ODCH_P5_cnt > PROTECT_DELAY_2S)
		{
			AbnormalState.val.ODCH_Protect5 = 1;
		}
	}
	else
	{
		ODCH_P5_on_cnt = sys_250ms_cnt;
		//ODCH_P5_cnt = sys_250ms_cnt - ODCH_P5_off_cnt;
		//if (ODCH_P5_cnt > PROTECT_DELAY_3S)
		//{
			////AbnormalState.val.ODCH_Protect5 = 0;
		//}
	}
}

void OCC_Flag(void)
{
	//int8_t temp = ( TEMP_3_BAT + TEMP_4_BAT + TEMP_5_BAT )/3 ;
	int8_t temp = ( nADC_TMONI_BAT_MAX + nADC_TMONI_BAT_MIN )/2;
	int16_t Limit_Current = 0;
	if (temp > 10)
	{
		Limit_Current = CURRENT_CHG1A * 30;
	}
	else if (temp > 0)
	{
		Limit_Current = CURRENT_CHG1A * 15;
	}
	else if (temp > -10)
	{
		Limit_Current = CURRENT_CHG1A * 10;
	}
	else
	{
		Limit_Current = CURRENT_CHG1A * 10;
	}
	
	
	if (nADC_CURRENT > ( Limit_Current *12 / 10 ) )    //充电过电流保护3
	{
		OCC_P3_cnt = sys_250ms_cnt - OCC_P3_on_cnt;
		if (OCC_P3_cnt > PROTECT_DELAY_2S)
		{
			AbnormalState.val.OCC_Protect3 = 1;
		}
	}
	else
	{
		OCC_P3_on_cnt = sys_250ms_cnt;

	}
	
	if (nADC_CURRENT <= ( Limit_Current *11 / 10 ) )    //解除充电过电流保护3
	{
		OCC_P3_cnt = sys_250ms_cnt - OCC_P3_off_cnt;
		if (OCC_P3_cnt > PROTECT_DELAY_3S )
		{
			AbnormalState.val.OCC_Protect3 = 0;
		}
	}
	else
	{
		OCC_P3_off_cnt = sys_250ms_cnt;
	}
	
	if (nADC_CURRENT > ( Limit_Current *13 / 10 ) )    //充电过电流保护4
	{
		OCC_P4_cnt = sys_250ms_cnt - OCC_P4_on_cnt;
		if (OCC_P4_cnt > PROTECT_DELAY_2S)
		{
			AbnormalState.val.OCC_Protect4 = 1;
		}
	}
	else
	{
		OCC_P4_on_cnt = sys_250ms_cnt;

	}
	
	if (nADC_CURRENT <= ( Limit_Current *12 / 10 ))    //解除充电过电流保护4
	{
		OCC_P4_cnt = sys_250ms_cnt - OCC_P4_off_cnt;
		if ( OCC_P4_cnt > PROTECT_DELAY_3S )
		{
			AbnormalState.val.OCC_Protect4 = 0;
		}
	}
	else
	{
		OCC_P4_off_cnt = sys_250ms_cnt;
	}

	if (nADC_CURRENT > ( Limit_Current *15 / 10 ) )    //充电过电流保护5
	{
		OCC_P5_cnt = sys_250ms_cnt - OCC_P5_on_cnt;
		if (OCC_P5_cnt > PROTECT_DELAY_2S)
		{
			AbnormalState.val.OCC_Protect5 = 1;
		}
	}
	else
	{
		OCC_P5_on_cnt = sys_250ms_cnt;
	}

	//if (nADC_CURRENT <= 0)    //解除充电过电流保护5
	//{
		//OCC_P5_cnt = sys_250ms_cnt - OCC_P5_off_cnt;
		//if (OCC_P5_cnt > PROTECT_DELAY_3S )
		//{
			//AbnormalState.val.OCC_Protect5 = 0;
		//}
	//}
	//else
	//{
		//OCC_P5_off_cnt = sys_250ms_cnt;
	//}
	
}

void ODC_Flag(void)
{
	if (nADC_CURRENT < ODC_P3_AM )    //放电过电流保护3
	{
		ODC_P3_cnt = sys_250ms_cnt - ODC_P3_on_cnt;
		if (ODC_P3_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.ODC_Protect3 = 1;
		}
	}
	else
	{
		ODC_P3_on_cnt = sys_250ms_cnt;
	}
	
	if (nADC_CURRENT > ODC_D3_AM )    //解除放电过电流保护3
	{
		ODC_P3_cnt = sys_250ms_cnt - ODC_P3_off_cnt;
		if (ODC_P3_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.ODC_Protect3 = 0;
		}
	}
	else
	{
		ODC_P3_off_cnt = sys_250ms_cnt;
	}

	if (nADC_CURRENT < ODC_P4_AM )    //放电过电流保护4
	{
		ODC_P4_cnt = sys_250ms_cnt - ODC_P4_on_cnt;
		if (ODC_P4_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.ODC_Protect4 = 1;
		}
	}
	else
	{
		ODC_P4_on_cnt = sys_250ms_cnt;
	}	
	
	if (nADC_CURRENT > ODC_D4_AM )    //解除放电过电流保护4
	{
		ODC_P4_cnt = sys_250ms_cnt - ODC_P4_off_cnt;
		if (ODC_P4_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.ODC_Protect4 = 0;
		}
	}
	else
	{
		ODC_P4_off_cnt = sys_250ms_cnt;
	}

	if (nADC_CURRENT < ODC_P5_AM )    //放电过电流保护5
	{
		ODC_P5_cnt = sys_250ms_cnt - ODC_P5_on_cnt;
		if (ODC_P5_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.ODC_Protect5 = 1;
		}
	}
	else
	{
		ODC_P5_on_cnt = sys_250ms_cnt;
	}
	
	//if (nADC_CURRENT > ODC_D5_AM )    //解除放电过电流保护5
	//{
		//ODC_P5_cnt = sys_250ms_cnt - ODC_P5_off_cnt;
		//if (ODC_P5_cnt > PROTECT_DELAY_3S)
		//{
			//AbnormalState.val.ODC_Protect5 = 0;
		//}
	//}
	//else
	//{
		//ODC_P5_off_cnt = sys_250ms_cnt;
	//}
	
}

void OTEMP_Flag(void)
{
	if (nADC_TMONI_BAT_MAX > OVER_TEMP_P3 )    //过温保护3
	{
		OTEMP_P3_cnt = sys_250ms_cnt - OTEMP_P3_on_cnt;
		if (OTEMP_P3_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.Over_Temp3 = 1;
		}
	}
	else
	{
		OTEMP_P3_on_cnt = sys_250ms_cnt;
	}

	if (nADC_TMONI_BAT_MAX < OVER_TEMP_D3 )    //解除过温保护3
	{
		OTEMP_P3_cnt = sys_250ms_cnt - OTEMP_P3_off_cnt;
		if (OTEMP_P3_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.Over_Temp3 = 0;
		}
	}
	else
	{
		OTEMP_P3_off_cnt = sys_250ms_cnt;
	}
	
	if (nADC_TMONI_BAT_MAX > OVER_TEMP_P4 )    //过温保护4
	{
		OTEMP_P4_cnt = sys_250ms_cnt - OTEMP_P4_on_cnt;
		if (OTEMP_P4_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.Over_Temp4 = 1;
		}
	}
	else
	{
		OTEMP_P4_on_cnt = sys_250ms_cnt;
	}	
	
	if (nADC_TMONI_BAT_MAX < OVER_TEMP_D4 )    //解除过温保护4
	{
		OTEMP_P4_cnt = sys_250ms_cnt - OTEMP_P4_off_cnt;
		if (OTEMP_P4_cnt > PROTECT_DELAY_3S)
		{
			AbnormalState.val.Over_Temp4 = 0;
		}
	}
	else
	{
		OTEMP_P4_off_cnt = sys_250ms_cnt;
	}

	if (nADC_TMONI_BAT_MAX > OVER_TEMP_P5 )    //过温保护5
	{
		OTEMP_P5_cnt = sys_250ms_cnt - OTEMP_P5_on_cnt;
		if (OTEMP_P5_cnt > PROTECT_DELAY_5S)
		{
			AbnormalState.val.Over_Temp5 = 1;
		}
	}
	else
	{
		OTEMP_P5_on_cnt = sys_250ms_cnt;
	}	

	//if (nADC_TMONI_BAT_MAX < OVER_TEMP_D5 )    //解除过温保护3
	//{
		//OTEMP_P5_cnt = sys_250ms_cnt - OTEMP_P5_off_cnt;
		//if (OTEMP_P5_cnt > PROTECT_DELAY_3S)
		//{
			//AbnormalState.val.Over_Temp5 = 0;
		//}
	//}
	//else
	//{
		//OTEMP_P5_off_cnt = sys_250ms_cnt;
	//}

}

void Stop_Flag(void)
{
	if (AbnormalState.val.OCHG_Protect4 ||
		AbnormalState.val.OCHG_Protect5 ||
		AbnormalState.val.ODCH_Protect4 ||
		AbnormalState.val.ODCH_Protect5 ||
		AbnormalState.val.OCC_Protect4  ||
		AbnormalState.val.OCC_Protect5  ||
		AbnormalState.val.ODC_Protect4  ||
		AbnormalState.val.ODC_Protect5  ||
		AbnormalState.val.Over_Temp4    ||
		AbnormalState.val.Over_Temp5    
	)
	{
		MCU_STOP_High();
		AbnormalState.val.Stop_Bit = 1;
	}
	else if ( (nADC_CURRENT > 0)  && 
			(	
				AbnormalState.val.OCHG_Protect3 || 
				AbnormalState.val.OCC_Protect3  || 
				AbnormalState.val.Over_Temp3    || 
				BatteryState.val.CHG_Inhibit_Temp
			)
	)
	{
		MCU_STOP_High();
		AbnormalState.val.Stop_Bit = 1;
	}
	else if ( nADC_CURRENT < 0  && AbnormalState.val.ODCH_Protect3	)
	{
		MCU_STOP_High();
		AbnormalState.val.Stop_Bit = 1;
	}
	else if ( (nADC_CURRENT < CURRENT_DCH_05A) && 
			(
				AbnormalState.val.ODCH_Protect3  ||
				AbnormalState.val.Over_Temp3     || 
				BatteryState.val.DCH_Inhibit_Temp
			)
	)
	{
		MCU_STOP_High();
		AbnormalState.val.Stop_Bit = 1;
	}
	else
	{
		MCU_STOP_Low();
		AbnormalState.val.Stop_Bit = 0;
	}
}

void Inhibit_Flag(void)
{
	if (nADC_TMONI_BAT_MAX > CHG_Inhibit_Temp_H || nADC_TMONI_BAT_MIN < CHG_Inhibit_Temp_L)    //充电温度异常
	{
		CHG_Inhibit_Temp_cnt = sys_250ms_cnt - CHG_Inhibit_Temp_on_cnt;
		if (CHG_Inhibit_Temp_cnt > PROTECT_DELAY_3S)
		{
			BatteryState.val.CHG_Inhibit_Temp = 1; //设置 充电禁止温度
		}
	}
	else
	{
		CHG_Inhibit_Temp_on_cnt = sys_250ms_cnt;
	}

	if (nADC_TMONI_BAT_MAX < CHG_Inhibit_Temp_HR && nADC_TMONI_BAT_MIN > CHG_Inhibit_Temp_LR )    //充电温度正常
	{
		CHG_Inhibit_Temp_cnt = sys_250ms_cnt - CHG_Inhibit_Temp_off_cnt;
		if (CHG_Inhibit_Temp_cnt > PROTECT_DELAY_3S)
		{
			BatteryState.val.CHG_Inhibit_Temp = 0; //解除 充电禁止温度
		}
	}
	else
	{
		CHG_Inhibit_Temp_off_cnt = sys_250ms_cnt;
	}
	
	if (nADC_TMONI_BAT_MAX > DCH_Inhibit_Temp_H || nADC_TMONI_BAT_MIN < DCH_Inhibit_Temp_L)    //放电温度异常
	{
		DCH_Inhibit_Temp_cnt = sys_250ms_cnt - DCH_Inhibit_Temp_on_cnt;
		if (DCH_Inhibit_Temp_cnt > PROTECT_DELAY_3S)
		{
			BatteryState.val.DCH_Inhibit_Temp = 1; //设置 充电禁止温度
		}
	}
	else
	{
		DCH_Inhibit_Temp_on_cnt = sys_250ms_cnt;
	}

	if (nADC_TMONI_BAT_MAX < DCH_Inhibit_Temp_HR && nADC_TMONI_BAT_MIN > DCH_Inhibit_Temp_LR )    //充电温度正常
	{
		DCH_Inhibit_Temp_cnt = sys_250ms_cnt - DCH_Inhibit_Temp_off_cnt;
		if (DCH_Inhibit_Temp_cnt > PROTECT_DELAY_3S)
		{
			BatteryState.val.DCH_Inhibit_Temp = 0; //解除 充电禁止温度
		}
	}
	else
	{
		CHG_Inhibit_Temp_off_cnt = sys_250ms_cnt;
	}
	
}

void Action_Flag(void)
{
	if(nADC_CURRENT > CURRENT_CHG_STATE)
	{
		BatteryState.val.ActionState = 2; // 电池状态设定为充电
	}
	else if (nADC_CURRENT < CURRENT_DCH_STATE)
	{
		BatteryState.val.ActionState = 3; // 电池状态设定为放电
	}
	else
	{
		BatteryState.val.ActionState = 1; // 电池状态设定为停止
	}
}