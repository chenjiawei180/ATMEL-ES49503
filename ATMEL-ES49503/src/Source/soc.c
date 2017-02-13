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
uint32_t uv_cap_val=0;           //����socֵ20161009zzysoc2
uint8_t uv_re_cap_rate=0;        //����socֵ20161009zzysoc2

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
DESCRIPTION		: ���������Լ���ѹ����
INPUT			: None
OUTPUT			: None
NOTICE			: δ���
DATE			: 2016/06/24
*****************************************************************************/
void NormalCapacityProc(void)
{
	static bool  power_first_flag = false;

	uint32_t capacity_volt;
	uint32_t temp = 0;
	//������������ֵ
	if(Total_VBAT > 6553)
	{
		if((power_first_flag == false) && (afe_flags.val.afe_uv_flag == 0))
		{
			if(flash_flags.val.re_cap_update_flag == 0)
			{
				// 6553 = 2.0V   8519 = 2.6V  13762 = 4.2V  SUB = 73400/100 = 52
				//��ѹ���
				capacity_volt = (uint32_t)Total_VBAT * 5000 / 16384;
				g_sys_cap.val.re_cap_rate = VbatToSoc((uint16_t)capacity_volt);

				EEPROM_Write_DATA(EEPROM_INDEX_CAP_VAL, g_sys_cap.val.re_cap_rate, 0);
				g_sys_cap.val.re_cap_rate_old = g_sys_cap.val.re_cap_rate;
				g_sys_cap.val.re_cap_rate_old2 = g_sys_cap.val.re_cap_rate;   //����socֵ20161010zzysoc3
			}
			temp = g_sys_cap.val.re_cap_rate;
			temp = temp * g_sys_cap.val.full_cap / 100;
			//	        	temp = ~temp;  // ȡ��
			//	        	temp+=1;

			g_sys_cap.val.cap_val = temp;
			g_sys_cap.val.cap_val3 = 0 ;        //����socֵ20161010zzysoc4
			power_first_flag = true;
		}
	}

	Cap_Update_Check();//����Ƿ���Ҫ����������������

	FullCap_Update();//�ж��Ƿ���Ҫ�������

	// ����ʣ������
	if(Total_VBAT > 6553)
	{
		if(g_sys_cap.val.cap_val > g_sys_cap.val.full_cap)
		{
			g_sys_cap.val.cap_val = g_sys_cap.val.full_cap;
		}
		if(afe_flags.val.afe_uv_flag == true)
		{
			//����socֵ20161009zzysoc2
			uv_cap_val = g_sys_cap.val.cap_val ;
			g_sys_cap.val.cap_val = 0;//����socֵ20161009zzysoc2ԭ�У�����Ϊ�����
			g_sys_cap.val.re_cap_rate_old2 = g_sys_cap.val.re_cap_rate;//Ƿѹ����//����socֵ20161010zzysoc3
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
		g_sys_cap.val.re_cap_rate_old2 = temp;                              //����socֵ20161010zzysoc3
		if( (g_sys_cap.val.re_cap_rate > 200) || (g_sys_cap.val.cap_val == 0 ) ) //��ֹ��ֵ���
		{
			g_sys_cap.val.re_cap_rate = 0;
		}
		if( g_sys_cap.val.re_cap_rate > 100 )//��ֹ����100
		{
			g_sys_cap.val.re_cap_rate = 100;
		}
	}

	//���ѭ��������Ƿ���1
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
DESCRIPTION		: ��ѹ���
INPUT			: vbat_val ��ѹֵ
OUTPUT			: ����ֵ
NOTICE			: �۰���
DATE			: 2016/06/27
*****************************************************************************/
uint8_t VbatToSoc(uint16_t vbat_val)
{

	uint8_t low = 0;
	uint8_t high = 101;    //�޸�
	uint8_t mid;
	while(low < high)
	{
		mid = (low + high)/2;    // ��ִ�г������ټӷ�����ֹlow + high > 256 �����

		if(vbat_val == Cell_volt[mid])
		{break;}    // �������պ���ȵ�ֵ�������Ϸ���
		if(high - low == 1)
		{
			mid = low;                 // ���ڲ��Ǿ�ȷ���ң�����С�����ڣ�ȡСֵ
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
DESCRIPTION		: �����������³���
INPUT			: None
OUTPUT			: None
NOTICE			: ���ڳ��ŵ糬��һ��ʱ����е�ѹ����
DATE			: 2016/06/24
*****************************************************************************/
void Cap_Update_Check(void)
{
	uint16_t vbat_sub =0;
	uint8_t new_cap_rate;
	uint32_t deta_cap_rate,new_cap_rate2;
	uint32_t capacity_volt;
	//    ULONG temp2 = 0;                //ͬʱ����socֵ20161009zzysoc1
	if((nADC_CURRENT <CUR_CHG_01C)&&(nADC_CURRENT >CUR_DCH_01C))
	{
		sys_flags.val.cap_update_end_flag =0;
		cap_update_reload_cnt =0;
		if(vbat_1min_delay ==0)
		{
			vbat_1min_last_val = Total_VBAT;
			vbat_1min_delay++;
			//��ѹ���
			capacity_volt = (uint32_t)Total_VBAT*5000/16384;
			new_cap_rate = VbatToSoc((uint16_t)capacity_volt);
			//��ѹ���һ��,�������ֵ,10����/���ֵ���ڸ���ʱ��
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
		cap_update_reload_cnt++; //����3�����¼�ʱ
		if(cap_update_reload_cnt >3)
		{
			cap_update_reload_cnt =0;
			vbat_1min_delay =0;
			sys_flags.val.cap_update_end_flag =1;
		}
	}
	
	
	// 20160722���� ��ŵ粹��
	//3.2Vĩ��У׼,5% 3.8V 10%
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
						//��ѹ���
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
						//��ѹ���
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
	
	//������ֵ�ѹ����,��ô���в���
	//���1����һ�ε�ѹ����,����ֵ�ǵ�����/4�����ǲ�����1%
	if(sys_flags.val.re_cap_update_flag == true)
	{
		//��ѹ���
		capacity_volt = (uint32_t)Total_VBAT*5000/16384;
		new_cap_rate = VbatToSoc((uint16_t)capacity_volt);

		sys_flags.val.cap_update_end_flag = 1; //�������0,˵�������������

		if((new_cap_rate -g_sys_cap.val.re_cap_rate>stop_cap_update_val)&&(new_cap_rate >g_sys_cap.val.re_cap_rate))
		{
			//��ֵ����4��Ϊÿ�θ��µ�����ֵ
			deta_cap_rate = new_cap_rate - g_sys_cap.val.re_cap_rate;
			deta_cap_rate>>=2;
			sys_flags.val.cap_update_end_flag = 0;
			if(deta_cap_rate ==0)
			{
				deta_cap_rate =1;
				sys_flags.val.cap_update_end_flag = 1;//����4%�����,�������ڲ���,��Ϊ��������,��������������������
			}
			g_sys_cap.val.re_cap_rate_sum -= deta_cap_rate;//�ŵ�����У׼,��������,���Ӽ���
			//deta_cap_rate = g_sys_cap.val.full_cap*deta_cap_rate/100;
			//g_sys_cap.val.cap_val = g_sys_cap.val.cap_val+deta_cap_rate;
			g_sys_cap.val.re_cap_rate = g_sys_cap.val.re_cap_rate + deta_cap_rate;//����socֵ20161010zzysoc3
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
			g_sys_cap.val.re_cap_rate_sum += deta_cap_rate;//�ŵ�����У׼,�����½�,��������
			//deta_cap_rate = g_sys_cap.val.full_cap*deta_cap_rate/100;
			//g_sys_cap.val.cap_val = g_sys_cap.val.cap_val-deta_cap_rate;
			g_sys_cap.val.re_cap_rate = g_sys_cap.val.re_cap_rate-deta_cap_rate;//����socֵ20161010zzysoc3
		}
		sys_flags.val.re_cap_update_flag = false;
	}
}

/****************************************************************************
FUNCTION		: FullCap_Update
DESCRIPTION		: ������������
INPUT			: None
OUTPUT			: None
NOTICE			: ���ڵ�ѹ����֮��,����cap_update_end_flag��״̬�����ж��Ƿ�������
DATE			: 2016/06/24
*****************************************************************************/
void FullCap_Update(void)
{
	uint32_t full_cap_temp;
	//�������������������,��ô�����������
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
					soc_fcc_save = 0;//���ܳɹ����,����Ҫ���¿�ʼ��¼
					if(afe_flags.val.afe_uv_flag== 0) //���û�з���Ƿѹ�����Ͱ�ԭ���ݽ��У������˾Ͱ�����ǰ������//����socֵ20161009zzysoc2
					{
						uv_cap_val = g_sys_cap.val.cap_val;
						uv_re_cap_rate = g_sys_cap.val.re_cap_rate;
					}
					full_cap_temp = g_sys_cap.val.cap_val2 - uv_cap_val- g_sys_cap.val.cap_val3;//����socֵ20161010zzysoc4 ����cap_val3ӦΪ��ֵ��
					full_cap_temp = full_cap_temp*100;
					full_cap_temp = full_cap_temp/(g_sys_cap.val.re_cap_rate2 - uv_re_cap_rate);
					
					//                    soc_fcc_save = 0;//���ܳɹ����,����Ҫ���¿�ʼ��¼
					//                    full_cap_temp = g_sys_cap.val.cap_val2 - g_sys_cap.val.cap_val- g_sys_cap.val.cap_val3;//����socֵ20161010zzysoc4 ����cap_val3ӦΪ��ֵ��
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
					soc_fcc_save = 0;//���ܳɹ����,����Ҫ���¿�ʼ��¼
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
DESCRIPTION		: �����洢����һ��BLOCK,32WORD����ѭ���洢
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
DESCRIPTION		: ѭ��������¼
INPUT			: None
OUTPUT			: None
NOTICE			: ѭ��������Ϊ3��,����ѭ��:�ۼƳ���7%�ķŵ�����,��¼1��.
NOTICE          : ��ȷŵ�:��ѹ����3V|�����ŵ糬��50%,��¼һ��.��ȳ��:��ѹ����4.1V|������糬��50%��¼һ��.
DATE			: 2016/06/27
*****************************************************************************/
void BatCycleProc(void)
{
	uint8_t soc_rate;
	//����ѭ��
	if((nADC_CURRENT <CURRENT_DCH_05A)&&(g_sys_cap.val.cycle_record_flag ==0))
	{
		if(g_sys_cap.val.re_cap_rate_record <g_sys_cap.val.re_cap_rate)
		{
			g_sys_cap.val.re_cap_rate_record = g_sys_cap.val.re_cap_rate;
		}
		else
		{
			//�ڷŵ�״̬,�����ϴηŵ����� - ��ǰ�ŵ����� ,���ڵ���1%��Ȼ�����
			soc_rate =g_sys_cap.val.re_cap_rate_record - g_sys_cap.val.re_cap_rate;
			g_sys_cap.val.re_cap_rate_sum += soc_rate;
			g_sys_cap.val.re_cap_rate_record = g_sys_cap.val.re_cap_rate;
			//һ����˵,��ѹУ׼���ᳬ�����ֵ,���������7%���ֵ,ֻ�ܵ�����һ�ηŵ�.���ǵ�ѹ����������14%
			//���Ϊ��,��ô��Ҫ�ȵ�ת���˲��ܼ���--��ѹ����ԭ��
			if(g_sys_cap.val.re_cap_rate_sum >6)
			{
				if(g_sys_cap.val.re_cap_rate_sum >100)//�����쳣,��Ӧ�ü�¼,�������
				{
					g_sys_cap.val.re_cap_rate_sum =0;
				}
				//                soc_rate= g_sys_cap.val.re_cap_rate_sum/7;
				g_sys_cap.val.bat_cycle_cnt+= 1;
				g_sys_cap.val.re_cap_rate_sum =0;//�����ϵ粻������0
				EEPROM_Write_DATA(EEPROM_INDEX_CYCLE,g_sys_cap.val.bat_cycle_cnt,1);
				g_sys_cap.val.cycle_record_flag = g_sys_cap.val.re_cap_rate;//��¼һ���Ժ�,���ټ�¼,�ȴ��´μ�¼ʱ����Ϊ0Ҳ������
			}
		}
	}
	else
	{
		g_sys_cap.val.re_cap_rate_record =0;//���ڷŵ��0,��֤�ŵ����������¼
	}
	
	
	if(nADC_CURRENT >CUR_CHG_01C)
	{
		if(g_sys_cap.val.cycle_record_flag >0)
		{
			//�ڼ�¼ѭ�������Ժ�,�ۼƳ��ʱ�䳬��10����,�����¼��־,����2.
			chg_record_cnt++;
			if(chg_record_cnt >2400)  //240 = 1min
			{
				g_sys_cap.val.cycle_record_flag =0;
				chg_record_cnt =0;
			}
		}
	}
	
	//��ȷŵ�
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
			g_sys_cap.val.deep_rate_sum =0;//20160815  AID ������0
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
	//��ȳ��
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
			g_sys_cap.val.deep_rate_chgsum =0;//20160815  AID ������0
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