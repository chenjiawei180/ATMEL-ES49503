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
DESCRIPTION		: AFE��Ϣ�������
INPUT			: None
OUTPUT			: None
UPDATE			:
DATE			: 2016/06/24
*****************************************************************************/
void AFE_Control(void)
{
	HardwareProtection();//Ӳ������
	SoftwareProtection();//�������
	SoftMeansureControl(); //����ɼ�����

	Cell_Balance(); //����
	//һ�ж�����,����PCB������,˵�����ܳ����˷���,��ŵ�ܶ��ر�,ֱ���¶Ƚ��ͻ�85��
	PCB_Protect();
}

/****************************************************************************
FUNCTION		: HardwareProtection
DESCRIPTION		: Ӳ����������
INPUT			: None
OUTPUT			: None
UPDATE			:
DATE			: 2016/06/24
*****************************************************************************/
void HardwareProtection(void)
{
	//�������õ�ѹ����ֵ,δ�ɹ���������,ֱ���ɹ�
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
	AFE_HardwareProtection_Read(); //��ȡSPI������Ϣ
	
	//AFE��ȡ����Ϣ����  OCC OCD SCD
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
DESCRIPTION		: �������
INPUT			: None
OUTPUT			: None
NOTICE			: ��ִ��Ӳ������,�ٽ����������
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
				if(soft_cp_cnt >PROTECT_DELAY_2S)  //ѭ��8��,250*8 = 2S
				{
					soft_cp_cnt =0;
					sys_states.val.soft_chg_protect =1;
					sys_states.val.chg_temp_protect = 1;
				}
			}
			else
			{
				soft_cp_cnt =0;
				if(nADC_CURRENT >0) //���
				{
					if(nADC_CURRENT >CURRENT_CHG_STATE)
					{
						if(nADC_CURRENT >CURRENT_CHGOC_PROTECT)
						{
							soft_occ_cnt++;
							if(soft_occ_cnt >PROTECT_DELAY_1S) // 4*250ms = 1S//��ȡ��zzy�����ƺ�PROTECT_DELAY_1S��1
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
				if(soft_dp_cnt >PROTECT_DELAY_2S) //�����ӳ�2S
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
							if(soft_ocd_cnt >PROTECT_DELAY_1S)  //�����ӳ�1S  //zzy20161025 PROTECT_DELAY_1S ��Ϊ1 ���������ƺ�������ȡ��
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
DESCRIPTION		: �����о��ѹ�ɼ�����
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
	// ѹ���500mV��ʾ��о����
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
	// ��͵�ѹ����1.5V��ʾ��о����
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
	// ��ߵ�ѹ����4.3V��ʾ��о���ϸ�Ϊ
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
	// ����2.5V����SHDN
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
			sys_flags.val.cell_high_alarm_flag = 1;//ԭ��д���˰�zzy sys_flags.val.cell_low_alarm_flag = 1;
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
DESCRIPTION		: �������
INPUT			: None
OUTPUT			: None
NOTICE			:   �ر�OV/UV����---��������--ѡ����Ҫ�������---��������
 * 
 * 
 * 
DATE			: 2016/06/24
*****************************************************************************/
void Cell_Balance(void)
{
    if((nADC_CELL_MAX > VCELL_BALANCE_Open) && (nADC_CELL_MAX-nADC_CELL_MIN > VCELL_BALANCE_START))
    {
        afe_flags.val.afe_CellBalance = 1;  //���������־
    }
    //�رվ���״̬����ߵ�ѹС�ڹرյ�ѹ4.0V��ѹ��С�ڽ���ѹ����ڷŵ�״̬
    if((nADC_CELL_MAX < VCELL_BALANCE_Close) 
        || ((nADC_CELL_MAX-nADC_CELL_MIN) < VCELL_BALANCE_END)
        ||(sys_states.val.sys_dch_state == 1) )
    {
        afe_flags.val.afe_CellBalance = 0;  //�رվ����־
    }
    if(afe_flags.val.afe_CellBalance == 1)
    {
        Balanc_index ++;
        if(Balanc_index < 25) //�ؾ��⡣�жϵ�ѹ
        {
            Cells_Bal_Close();
            if(Balanc_index >20)
            {
                Cells_Bal_Judge();// CELL BALANCE
            }
        }                
        else if ((Balanc_index < 75) )
        {
            g_bal_state.VAL = g_bal_need.VAL & 0x5555; //�ر�ż��λ
            Cells_Bal_Open();
        }
        else if(Balanc_index > 80)
        {
            g_bal_state.VAL = g_bal_need.VAL & 0xAAAA; //�ر�����λ
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
DESCRIPTION		: PCB���±���
INPUT			: None
OUTPUT			: None
NOTICE			: ������ǿ�ƹر��ౣ��,ֻ����ر�,�������,����λ��Ӧ������־λ,�������������߼�����
DATE			: 2016/06/24
*****************************************************************************/
void PCB_Protect(void)
{
	if(nADC_TMONI_PCB_MAX > TEMP_PCB_PROTECT)
	{
		soft_pcb_ot_cnt++;
		if(soft_pcb_ot_cnt >PROTECT_DELAY_2S) //�����ӳ�2S
		{
			soft_pcb_ot_cnt =9; //����2Sλ��,����2S��--��պý��
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
DESCRIPTION		: ϵͳ250ms����
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
		if(AFE_OC_DELAY_CNT!=45)//zzy20161026 ����ֻ�е����������Ż�ﵽ45�����治�䣬����ָ�
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
		if(AFE_SCD_DELAY_CNT!=45)//����ֻ�е����������Ż�ﵽ45�����治�䣬����ָ�
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
		if(AFE_OCC_DELAY_CNT!=45)//����ֻ�е��β�������̰���ʱ�Ż�ﵽ45�����治�䣬����ָ�
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
DESCRIPTION		: �������������׷���
INPUT			: None
OUTPUT			: None
NOTICE			: TMIER�����¼
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
	
    //��Ӽ����ۻ���������ۻ��ŵ���
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
	if(g_sys_cap.val.cap_val >0)//����socֵ20161010zzysoc4 �������ֵС�ڵ����㣬��ʹ�� g_sys_cap.val.cap_val3���渺����ֵ
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