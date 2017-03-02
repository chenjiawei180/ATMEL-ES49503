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


uint16_t OCC_TIMEOUT = 0;
uint16_t OCD_TIMEOUT = 0;

uint8_t soft_cp_cnt = 0;
uint8_t soft_occ_cnt = 0;
uint8_t soft_dp_cnt = 0;
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

	Cell_Balance(); //均衡
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
				if(soft_cp_cnt >PROTECT_DELAY_2S)  //循环8次,250*8 = 2S
				{
					soft_cp_cnt =0;
					sys_states.val.soft_chg_protect =1;
					sys_states.val.chg_temp_protect = 1;
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
				if(soft_dp_cnt >PROTECT_DELAY_2S) //防抖延迟2S
				{
					soft_dp_cnt =0;
					sys_states.val.soft_dch_protect =1;
					sys_states.val.dch_temp_protect = 1;
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
		cell_alarm_cnt = sys_250ms_cnt;
	}

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
    }
    //关闭均衡状态：最高电压小于关闭电压4.0V、压差小于结束压差、处于放电状态
    if((nADC_CELL_MAX < VCELL_BALANCE_Close) 
        || ((nADC_CELL_MAX-nADC_CELL_MIN) < VCELL_BALANCE_END)
        ||(sys_states.val.sys_dch_state == 1) )
    {
        afe_flags.val.afe_CellBalance = 0;  //关闭均衡标志
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
	
	SysLED_Display();
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
		if (CHG_Val - CHG_Val_Bak >= 100 )
		{
			Write_Time_or_mAh(CHG_Val,CHG_FLAG);
			CHG_Val_Bak = CHG_Val;
		}
	}
	else
	{
		DCH_Val = DCH_Val - tmp_cap;
		if (DCH_Val - DCH_Val_Bak >=100 )
		{
			Write_Time_or_mAh(DCH_Val,DCH_FLAG);
			DCH_Val_Bak = DCH_Val;
		}
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