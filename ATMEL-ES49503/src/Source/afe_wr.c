/*
 * afe_wr.c
 *
 * Created: 2017/2/7 13:34:09
 *  Author: chenjiawei
 */ 

#include "afe_wr.h"
#include "spi.h"
#include "ad_dat.h"
#include "protect.h"
#include "soc.h"
#include "wdt.h"
#include "history.h"

#ifdef OS_DEBUG
#include "adc.h"
#include "gpio.h"
#endif
	
volatile uint8_t AFE_OC_DELAY_CNT_LIMIT =0;         //zzy20161026
volatile uint8_t AFE_SCD_DELAY_CNT_LIMIT =0;        //zzy20161026
volatile uint8_t AFE_OCC_DELAY_CNT_LIMIT =0;        //zzy20161026
volatile uint16_t sw_change_lowpower_cnt=0;        //zzy20161101�͹���


/**
  * @brief  AFE��ʼ��.
  * @param  None
  * @retval None
  */

void AFE_Init(void)
{
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_UNLOCK);   //unlock IC
    //�ϵ���ADֵ
    //    ucSPI_Write(MAC_SPI_DEV,STAT_ADDR,AFE_VAD_DONE);   //clear VAD_DONE
    //    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,AFE_AVD_LATCH);   //END AD
    
    ucSPI_Write(MAC_SPI_DEV,GPIO_CTRL4_ADDR,AFE_TM_PULLUP);   //set tm1~5 pullup
    ucSPI_Write(MAC_SPI_DEV,GVSEL_ADDR,AFE_OTHER_AD_ALL_ON);   //enable other AD
    ucSPI_Write(MAC_SPI_DEV,GPIO_CTRL1_ADDR,AFE_GPIO456_GPIO1_EN);   //enable GPIO1 INPUT GPIO5 OUTPUT
    
    //    ucSPI_Write(MAC_SPI_DEV,GPIO_CTRL2_ADDR,0x0008);   //ʹ��GPIO4����
    //    ucSPI_Write(MAC_SPI_DEV,GPIOSEL_ADDR,AFE_ADIQ2_EN);   //enable GPIO5 ADIR
    //       ucSPI_Write(MAC_SPI_DEV,OUVCTL1_ADDR,AFE_OV45_UV30);   //set OV UV value
    //       ucSPI_Write(MAC_SPI_DEV,OUVCTL2_ADDR,AFE_ALARM_3V5);   //set Alarm Volt value
    
    //    ucSPI_Write(MAC_SPI_DEV,ADCTRL2_ADDR,AFE_ADIH_EN);   //enable High Speed Cur AD
    ucSPI_Write(MAC_SPI_DEV,ADCTRL2_ADDR,AFE_ADIL_EN);   //enable low Speed Cur AD
    
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_LOCK);   //lock IC
    //    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,AFE_ADC_EN_TRG);   //enable AD one short
    //ucSPI_Read(MAC_SPI_DEV,ADCTRL2_ADDR,spi_read_value);        //zzy?
}

/************************SPIͨ���߼�*******************************************
 *                        д��AD�����Լ�ADģʽ
 *                                ��
 *            �ȴ�VAD_DONE��AVD_LATCH���,�ظ�д����2��ֵ��ֱ����ȡΪ0
 *                                ��
 *       ��AD����Ժ��ж�250ms�Ƿ�ʱ����ʱ����SPIͨ�ţ���ʽΪһ������ȡ56*2+1
 *                                ��
 *                     ��SPI��ȡ�������ݴ����Լ������У׼
 *                                ��
 *                           �ٴλص���ͷ����
******************************************************************************/
void AFE_Reg_Read(void)
{
	SPI_AllReg_WR();        //zzy20161101 ����֮��Ϊ������Ŀ
}

/****************************************************************************
FUNCTION		: SPI_AllReg_WR
DESCRIPTION		: ��ȡAFE����
INPUT			: None
OUTPUT			: None
NOTICE			: �յ�������ɱ�־--�˳�STBģʽ--����ͨ��--��ȡAD��ɱ�־,������ж�--��ȡADֵ
DATE			: 2016/06/24
*****************************************************************************/

void SPI_AllReg_WR(void)
{
    if(sys_flags.val.afe_adirq2_flag == 1)
    {
		#ifdef OS_DEBUG
		Usart_process();
		MCU_STOP_Toggle();
		ID_OUT_Toggle();
		COM_RES_Low();
		//if(ID_IN_Read()) printf("ID_IN ��. \r\n");
		//else printf("ID_IN ��. \r\n");
		//if(ID_END_Read()) printf("ID_END ��. \r\n");
		//else printf("ID_END ��. \r\n");
		//if(SOV_Read()) printf("SOV ��. \r\n");
		//else printf("SOV ��. \r\n");
		//uint16_t adc_value = 0;
		//Adc_Read_AdcValue(&adc_value);
		//printf("ADC = %d. \r\n",adc_value);
		#endif
		Time_update();
	    sys_flags.val.afe_adirq2_flag =0;
	    sys_flags.val.afe_connect_flag =0;
	    //Wdt_Clear(); //Ӧ����AFE��Ӧһ���Ժ�ι�����Է�AFE��λ����оƬ�޷���AFE���Ϲ���
		STB_Low();
		SPI_Slave_High();		//����ͨ����Ҫ�ӳ�2ms����
	    delay_ms(3);

	    sleep_delay_cycle++; //�����ӳ�,��һ�ζ�ȡ��һ������ȷֵ,��ֹ������������޷�����
	    if(afe_flags.val.afe_CellBalance == 1)
	    {
		    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,(AFE_AVD_LATCH | 0x0100));   //�������ÿ���
		    Cells_Bal_Close();
		    ucSPI_Read(MAC_SPI_DEV,STAT_ADDR,spi_read_value);
		    ucSPI_Write(MAC_SPI_DEV,STAT_ADDR,AFE_VAD_DONE);   //clear VAD_DONE
		    delay_us(100);
		    //ȷʵ��AD���,��ȡ����
		    if((spi_read_value[0]&0x0005) == 0x0005)
		    {
			    ucSPI_Continue_Read(MAC_SPI_DEV,MAC_AN49503_READ_ADR,MAC_AN49503_READ_CNT);
			    vAPI_ADC_Read_Data_bal_1();
		    }
		    delay_us(200);
		    delay_ms(1);
		    AFE_ONE_VPC_ADC();
		    if((spi_read_value[0]&0x0001) == 0x0001)
		    {
			    ucSPI_Continue_Read(MAC_SPI_DEV,MAC_AN49503_READ_ADR,MAC_AN49503_READ_CNT);
			    vAPI_ADC_Read_Data_bal_2();
		    }
	    }
	    else
	    {	
		    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,AFE_AVD_LATCH);   //END AD
		    
		    ucSPI_Read(MAC_SPI_DEV,STAT_ADDR,spi_read_value);

		    ucSPI_Write(MAC_SPI_DEV,STAT_ADDR,AFE_VAD_DONE);   //clear VAD_DONE
		    delay_us(100);
		    //ȷʵ��AD���,��ȡ����

		    if((spi_read_value[0]&0x0005) == 0x0005)
		    {
			    ucSPI_Continue_Read(MAC_SPI_DEV,MAC_AN49503_READ_ADR,MAC_AN49503_READ_CNT);
			    vAPI_ADC_Read_Data();
		    }
	    }
	    AFE_Init();
	    AFE_Control();

	    if(afe_flags.val.afe_CellBalance == 1)
	    {
		    ucSPI_Write(MAC_SPI_DEV,SPICTL_ADDR,AFE_SPI_NM50);   //LP50_EN   AFE_SPI_LP50  cellzzy
	    }
	    else
	    {
		    ucSPI_Write(MAC_SPI_DEV,SPICTL_ADDR,AFE_SPI_LP50);   //LP50_EN   AFE_SPI_LP50  cellzzy
	    }
	    SPI_Slave_Low();
	    delay_ms(3);
    
	    NormalCapacityProc();//��������
	    Sys_250ms_tick();    //ϵͳ250ms����
   
	    //        if(low_power_cnt>8)         //zzy20161101  ����3�Σ�11-8��
	    //            {
	    //                low_power_cnt++;
	    //            }
    }
}

/****************************************************************************
FUNCTION		: Cells_Bal_Close
DESCRIPTION		: �رյ�ؾ��⹦��
INPUT			: None
OUTPUT			: None
NOTICE			: ���ùرա�ȡ����ص�оѡ�����ر�ƽ�⡢����OVUV������5VLDOΪ�͹���
                    ���ûģʽ��������ģʽ
DATE			: 2017/01/24
*****************************************************************************/
void Cells_Bal_Close(void)
{
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_UNLOCK);   //unlock IC
    
    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,AFE_AVD_LATCH );   //���ùر�             
    ucSPI_Write(MAC_SPI_DEV,CBSEL_ADDR,0x0000 );            //ȡ�����ѡ��  
    ucSPI_Write(MAC_SPI_DEV,CB_CTL_ADDR,AFE_Balance_Dis );  //�رվ���       
    ucSPI_Write(MAC_SPI_DEV,UVMSK_ADDR,AFE_CELL13S_P);   //  15S     ����UVOV
    ucSPI_Write(MAC_SPI_DEV,OVMSK_ADDR,AFE_CELL13S_P);   //  15S     ����UVOV        
    ucSPI_Write(MAC_SPI_DEV,SPICTL_ADDR,AFE_SPI_LP50);   //NM50_EN   AFE_SPI_NM50
    
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_LOCK);   //lock IC
}

/****************************************************************************
FUNCTION		: AFE_ONE_VPC_ADC
DESCRIPTION		: ��ȡһ��AFE��ADC����
INPUT			: None
OUTPUT			: None
NOTICE			: ��ȡ��ADֵΪ����Ԥ���150ms�Ժ��ADֵ���²⣩
DATE			: 2016/06/24
*****************************************************************************/
uint16_t AFE_ONE_VPC_ADC(void)
{
	uint8_t i;
	uint16_t vpc = 0;
	for(i=0;i<200;i++)
	{
		ucSPI_Read(MAC_SPI_DEV,STAT_ADDR,spi_read_value);
		if(spi_read_value[0]&0x0001)
		{
			ucSPI_Write(MAC_SPI_DEV,STAT_ADDR,0x0001);   //clear VAD_DONE
			ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,0x4001);   //END AD
			delay_ms(10);
			ucSPI_Read(MAC_SPI_DEV,GPIO1_AD_ADDR,spi_read_value);//GPIO1_AD_ADDR VPAC_AD_ADDR
			vpc = spi_read_value[0];
			break;
		}
		delay_ms(1);
	}
	return vpc;
}

/****************************************************************************
FUNCTION		: AFE_HardwareProtection_Read
DESCRIPTION		: ��ȡAFE��Ӳ��������Ϣ
INPUT			: None
OUTPUT			: None
NOTICE			: ֱ�Ӷ�ȡSTAT�Ĵ���,OV,UV���ڵ�ѹ�ﵽ�Զ����,���Զ��������־
*                 SCD,OCD,OCCΪӲ������,������,��Ҫ�����ס�ŵ�,�������־λ,Ӳ���ָ����,��������ŵ�
DATE			: 2016/06/24
*****************************************************************************/
void AFE_HardwareProtection_Read(void)
{
	uint16_t flag;
	//   ucSPI_Read(MAC_SPI_DEV,MODE_STAT_ADDR,spi_read_value);  // SCD OCD DCD FLAG---����
	
	afe_flags.val.afe_read_reg_err_flag =0;
	if(ucSPI_Read(MAC_SPI_DEV,STAT_ADDR,spi_read_value)==0)  // UV HV SCD OCD DCD FLAG ---����
	{
		flag = spi_read_value[0]&ST_PROTECT;
		flag &= 0x0370;
		afe_flags.VAL &= 0xFC8F;
		afe_flags.VAL |= flag;
		//�����ָ�
		ucSPI_Write(MAC_SPI_DEV,OVL_STAT_ADDR,0x0000);   //clear OV
		ucSPI_Write(MAC_SPI_DEV,UVL_STAT_ADDR,0x0000);   //clear UV
		if((afe_flags.val.afe_ov_flag == 1)||(afe_flags.val.afe_uv_flag == 1))
		{
			sys_flags.val.afe_volt_protect_flag = 1;
		}
		//20160912 ��� afe_volt_protect_flag��־
		if((afe_flags.val.afe_ov_flag ==0)&&(afe_flags.val.afe_uv_flag ==0))
		{
			if(sys_flags.val.afe_volt_protect_flag == 1)
			{
				sys_flags.val.afe_volt_protect_flag =0;
			}
		}
	}
	else
	{
		afe_flags.val.afe_read_reg_err_flag =1;
	}
	//20160907   0909 ȷ�� ����
	//     // 20160728 �����ָ� --����--ovuv��λ---���--ovuv��0----�ֶ�д��crv+clr----����ָ�
	//    if((afe_flags.val.afe_ov_flag ==0)&&(afe_flags.val.afe_uv_flag ==0))
	//    {
	//        if(sys_flags.val.afe_volt_protect_flag == 1)
	//        {
	//            ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,0xa000);   //clear FDR
	//            __delay_us(10);
	//            if(ucSPI_Read(MAC_SPI_DEV,FDRV_CTRL_ADDR,spi_read_value)==0)  // UV HV SCD OCD DCD FLAG ---����
	//            {
	//                flag = spi_read_value[0]&0x4000;
	//                if(flag>0)
	//                {
	//                    afe_flags.val.afe_fdr_rcv_flag =1;
	//                }
	//                else
	//                {
	//                    afe_flags.val.afe_fdr_rcv_flag =0;
	//                    sys_flags.val.afe_volt_protect_flag =0;
	//                }
	//            }
	//        }
	//    }
	
	if(afe_flags.VAL&AFE_SCD_OCD_OCC)
	{
		//�����ָ�
		if(AFE_OC_DELAY_CNT >40) //250ms * 4 = 1S
		{
			if(AFE_OC_DELAY_CNT_LIMIT <6)
			{
				AFE_OC_DELAY_CNT_LIMIT++;
				AFE_OC_DELAY_CNT =0;
				ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_CLR);   //clear FDR
				ucSPI_Write(MAC_SPI_DEV,STAT_ADDR,ST_PROTECT);   //clear SCD OCD OCC
				ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_AUTO_CONTROL);   //clear FDR
				ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_AUTO_CONTROL);   //clear FDR
			}
			else
			{
				AFE_OC_DELAY_CNT = 41;
			}

		}
		if(AFE_SCD_DELAY_CNT >80)
		{
			if(AFE_SCD_DELAY_CNT_LIMIT <6)
			{
				AFE_SCD_DELAY_CNT_LIMIT++;
				AFE_SCD_DELAY_CNT =0;
				ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_CLR);   //clear FDR
				ucSPI_Write(MAC_SPI_DEV,STAT_ADDR,ST_PROTECT);   //clear SCD OCD OCC
				ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_AUTO_CONTROL);   //clear FDR
				ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_AUTO_CONTROL);   //clear FDR
			}
			else
			{
				AFE_SCD_DELAY_CNT = 41;
			}
		}
		if(AFE_OCC_DELAY_CNT >40)
		{
			if(AFE_OCC_DELAY_CNT_LIMIT <6)
			{
				AFE_OCC_DELAY_CNT_LIMIT++;
				AFE_OCC_DELAY_CNT =0;
				ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_CLR);   //clear FDR
				ucSPI_Write(MAC_SPI_DEV,STAT_ADDR,ST_PROTECT);   //clear SCD OCD OCC
				ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_AUTO_CONTROL);   //clear FDR
				ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_AUTO_CONTROL);   //clear FDR     _LIMIT
			}
			else
			{
				AFE_OCC_DELAY_CNT = 41;
			}
		}
	}
	if(ucSPI_Read(MAC_SPI_DEV,FDRV_CTRL_ADDR,spi_read_value) ==0)
	{
		if((spi_read_value[0]&AFE_FET_CLR_BIT) ==0)//xxy zzy
		{
			
			afe_flags.val.afe_FET_CLR_BIT=0;
		}
		else
		{
			afe_flags.val.afe_FET_CLR_BIT=1;
			ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_AUTO_CONTROL);   //clear FDR
		}
	}
	
	//FET DRIVE READ FET״̬
	if(ucSPI_Read(MAC_SPI_DEV,FDRVSTAT_ADDR,spi_read_value) == 0)  // FDR PROTECT
	{
		flag = spi_read_value[0]&0x000C;
		flag >>=2;
		afe_flags.VAL &= 0xFFFC;
		afe_flags.VAL |= flag;
	}
	else
	{
		afe_flags.val.afe_read_reg_err_flag =1;
	}
}

/********************************************************
 * @Description: cells balance judge
 * @Input      :
 * @Output     :������Ҫ���⣬������������ĵ�о�������������ǰ����ʱ���л�����ż��
 * @Notice     ��
 * @Date       ��2016.12.8 20170123 zzyESben
 *********************************************************/
void Cells_Bal_Judge(void) 
{
    uint8_t i = 0;
//    u16 j = 0;
    uint16_t diff_volt = 0;
    
    diff_volt = nADC_CELL_MAX - nADC_CELL_MIN;
    
    //g_bal_state.VAL = 0;      //clear the flag
    if (sys_states.val.sys_chg_state == 1) //cell balance in charge mode
    {
//        if (diff_volt > VCELL_BALANCE_START) // ��ʼֵ50mV
//        {
            for (i = 0; i < 16; i++)//CELLS_NUM  ������Ҫ����ż���ֿ�
                //			for( i = j;i < 16;i += 2 )
            {
                if (nADC_Cell_Value[i] > nADC_CELL_MIN) 
                {
                    diff_volt = nADC_Cell_Value[i] - nADC_CELL_MIN; //ÿ�ڵ�о��ѹ����͵�о��ѹ��ѹ��
                    if (diff_volt > VCELL_BALANCE_START) //��ʼֵ  50mV
                    {
                        g_bal_need.VAL |= (0x01 << i); //����Ӧ�ı�־λ�ø�
                    } else if (diff_volt < VCELL_BALANCE_END) //����ֵ ��ѹ���� 25��
                    {
                        g_bal_need.VAL &= ~(0x01 << i); //����Ӧ�ı�־λ�õ�
                    }
                }
            }
//        }
    } else if (sys_states.val.sys_dch_state == 0) //cell balance in relax mode
    {
//        if (diff_volt > VCELL_BALANCE_START) // ��ʼֵ50mV ��ֹ״̬�¿�Сһ��
//        {
            for (i = 0; i < 16; i++)//CELLS_NUM  ������Ҫ����ż���ֿ�
                //			for( i = j;i < 16;i += 2 )
            {
                if (nADC_Cell_Value[i] > nADC_CELL_MIN) 
                {
                    diff_volt = nADC_Cell_Value[i] - nADC_CELL_MIN; //ÿ�ڵ�о��ѹ����͵�о��ѹ��ѹ��
                    if (diff_volt > VCELL_BALANCE_START) //��ʼֵ  50mV
                    {
                        g_bal_need.VAL |= (0x01 << i); //����Ӧ�ı�־λ�ø�
                    } else if (diff_volt < VCELL_BALANCE_END) //����ֵ ��ѹ���� 25��
                    {
                        g_bal_need.VAL &= ~(0x01 << i); //����Ӧ�ı�־λ�õ�
                    }
                }
            }
//        }
    } 
    else if (sys_states.val.sys_dch_state == 1) //if the battery is in discharge mode ,disable all cell balance flag
    {
        g_bal_need.VAL = 0;
    }
    //g_bal_state.VAL = 0;

}

/****************************************************************************
FUNCTION		: Cells_Bal_Open
DESCRIPTION		: ���������ƽ�⹤��
INPUT			: None
OUTPUT			: None
NOTICE			: ���ûģʽ��������ģʽ������5VLDO��������ģʽ���ر�OV��UV����
                    ���ñ�־λ����ƽ�⡢���ÿ�����
DATE			: 2017/01/24
*****************************************************************************/
void Cells_Bal_Open(void)
{
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_UNLOCK);   //unlock IC
    
    ucSPI_Write(MAC_SPI_DEV,SPICTL_ADDR,AFE_SPI_NM50);   //NM50_EN   AFE_SPI_NM50       
    ucSPI_Write(MAC_SPI_DEV,UVMSK_ADDR,0xFFFF);   //  15S     �ر�OVUV?
    ucSPI_Write(MAC_SPI_DEV,OVMSK_ADDR,0xFFFF);   //  15S     �ر�OVUV?        
    ucSPI_Write(MAC_SPI_DEV,CB_CTL_ADDR,AFE_Balance_EN );             //��������
    ucSPI_Write(MAC_SPI_DEV,CBSEL_ADDR,g_bal_state.VAL );             //ѡ��������
    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,(AFE_AVD_LATCH | 0x0100));   //�������ÿ���  
    
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_LOCK);   //lock IC
}