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
volatile uint16_t sw_change_lowpower_cnt=0;        //zzy20161101低功耗


/**
  * @brief  AFE初始化.
  * @param  None
  * @retval None
  */

void AFE_Init(void)
{
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_UNLOCK);   //unlock IC
    //上电清AD值
    //    ucSPI_Write(MAC_SPI_DEV,STAT_ADDR,AFE_VAD_DONE);   //clear VAD_DONE
    //    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,AFE_AVD_LATCH);   //END AD
    
    ucSPI_Write(MAC_SPI_DEV,GPIO_CTRL4_ADDR,AFE_TM_PULLUP);   //set tm1~5 pullup
    ucSPI_Write(MAC_SPI_DEV,GVSEL_ADDR,AFE_OTHER_AD_ALL_ON);   //enable other AD
    ucSPI_Write(MAC_SPI_DEV,GPIO_CTRL1_ADDR,AFE_GPIO456_GPIO1_EN);   //enable GPIO1 INPUT GPIO5 OUTPUT
    
    //    ucSPI_Write(MAC_SPI_DEV,GPIO_CTRL2_ADDR,0x0008);   //使能GPIO4下拉
    //    ucSPI_Write(MAC_SPI_DEV,GPIOSEL_ADDR,AFE_ADIQ2_EN);   //enable GPIO5 ADIR
    //       ucSPI_Write(MAC_SPI_DEV,OUVCTL1_ADDR,AFE_OV45_UV30);   //set OV UV value
    //       ucSPI_Write(MAC_SPI_DEV,OUVCTL2_ADDR,AFE_ALARM_3V5);   //set Alarm Volt value
    
    //    ucSPI_Write(MAC_SPI_DEV,ADCTRL2_ADDR,AFE_ADIH_EN);   //enable High Speed Cur AD
    ucSPI_Write(MAC_SPI_DEV,ADCTRL2_ADDR,AFE_ADIL_EN);   //enable low Speed Cur AD
    
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_LOCK);   //lock IC
    //    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,AFE_ADC_EN_TRG);   //enable AD one short
    //ucSPI_Read(MAC_SPI_DEV,ADCTRL2_ADDR,spi_read_value);        //zzy?
}

/************************SPI通信逻辑*******************************************
 *                        写入AD内容以及AD模式
 *                                ↓
 *            等待VAD_DONE和AVD_LATCH完成,重复写入这2个值，直到读取为0
 *                                ↓
 *       当AD完成以后，判断250ms是否到时，到时进入SPI通信，方式为一口气读取56*2+1
 *                                ↓
 *                     将SPI读取到的内容处理以及分类和校准
 *                                ↓
 *                           再次回到开头→↑
******************************************************************************/
void AFE_Reg_Read(void)
{
	SPI_AllReg_WR();        //zzy20161101 除此之外为增加项目
}

/****************************************************************************
FUNCTION		: SPI_AllReg_WR
DESCRIPTION		: 读取AFE数据
INPUT			: None
OUTPUT			: None
NOTICE			: 收到电流完成标志--退出STB模式--开启通信--读取AD完成标志,清除，判断--读取AD值
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
		//if(ID_IN_Read()) printf("ID_IN 高. \r\n");
		//else printf("ID_IN 低. \r\n");
		//if(ID_END_Read()) printf("ID_END 高. \r\n");
		//else printf("ID_END 低. \r\n");
		//if(SOV_Read()) printf("SOV 高. \r\n");
		//else printf("SOV 低. \r\n");
		//uint16_t adc_value = 0;
		//Adc_Read_AdcValue(&adc_value);
		//printf("ADC = %d. \r\n",adc_value);
		#endif
		Time_update();
	    sys_flags.val.afe_adirq2_flag =0;
	    sys_flags.val.afe_connect_flag =0;
	    //Wdt_Clear(); //应该在AFE响应一次以后喂狗，以防AFE复位导致芯片无法跟AFE契合工作
		STB_Low();
		SPI_Slave_High();		//开启通信需要延迟2ms以上
	    delay_ms(3);

	    sleep_delay_cycle++; //休眠延迟,第一次读取不一定是正确值,防止错误进入休眠无法唤醒
	    if(afe_flags.val.afe_CellBalance == 1)
	    {
		    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,(AFE_AVD_LATCH | 0x0100));   //设置设置开启
		    Cells_Bal_Close();
		    ucSPI_Read(MAC_SPI_DEV,STAT_ADDR,spi_read_value);
		    ucSPI_Write(MAC_SPI_DEV,STAT_ADDR,AFE_VAD_DONE);   //clear VAD_DONE
		    delay_us(100);
		    //确实是AD完成,读取数据
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
		    //确实是AD完成,读取数据

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
    
	    NormalCapacityProc();//容量更新
	    Sys_250ms_tick();    //系统250ms更新
   
	    //        if(low_power_cnt>8)         //zzy20161101  运行3次（11-8）
	    //            {
	    //                low_power_cnt++;
	    //            }
    }
}

/****************************************************************************
FUNCTION		: Cells_Bal_Close
DESCRIPTION		: 关闭电池均衡功能
INPUT			: None
OUTPUT			: None
NOTICE			: 设置关闭、取消电池电芯选定、关闭平衡、开启OVUV、设置5VLDO为低功耗
                    设置活动模式还是休眠模式
DATE			: 2017/01/24
*****************************************************************************/
void Cells_Bal_Close(void)
{
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_UNLOCK);   //unlock IC
    
    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,AFE_AVD_LATCH );   //设置关闭             
    ucSPI_Write(MAC_SPI_DEV,CBSEL_ADDR,0x0000 );            //取消电池选定  
    ucSPI_Write(MAC_SPI_DEV,CB_CTL_ADDR,AFE_Balance_Dis );  //关闭均衡       
    ucSPI_Write(MAC_SPI_DEV,UVMSK_ADDR,AFE_CELL13S_P);   //  15S     开启UVOV
    ucSPI_Write(MAC_SPI_DEV,OVMSK_ADDR,AFE_CELL13S_P);   //  15S     开启UVOV        
    ucSPI_Write(MAC_SPI_DEV,SPICTL_ADDR,AFE_SPI_LP50);   //NM50_EN   AFE_SPI_NM50
    
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_LOCK);   //lock IC
}

/****************************************************************************
FUNCTION		: AFE_ONE_VPC_ADC
DESCRIPTION		: 读取一次AFE的ADC数据
INPUT			: None
OUTPUT			: None
NOTICE			: 读取的AD值为开启预充电150ms以后的AD值（猜测）
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
DESCRIPTION		: 读取AFE的硬件保护信息
INPUT			: None
OUTPUT			: None
NOTICE			: 直接读取STAT寄存器,OV,UV属于电压达到自动解除,所以定期清除标志
*                 SCD,OCD,OCC为硬件保护,软件清除,需要软件锁住放电,再清除标志位,硬件恢复完毕,软件开启放电
DATE			: 2016/06/24
*****************************************************************************/
void AFE_HardwareProtection_Read(void)
{
	uint16_t flag;
	//   ucSPI_Read(MAC_SPI_DEV,MODE_STAT_ADDR,spi_read_value);  // SCD OCD DCD FLAG---出现
	
	afe_flags.val.afe_read_reg_err_flag =0;
	if(ucSPI_Read(MAC_SPI_DEV,STAT_ADDR,spi_read_value)==0)  // UV HV SCD OCD DCD FLAG ---保护
	{
		flag = spi_read_value[0]&ST_PROTECT;
		flag &= 0x0370;
		afe_flags.VAL &= 0xFC8F;
		afe_flags.VAL |= flag;
		//保护恢复
		ucSPI_Write(MAC_SPI_DEV,OVL_STAT_ADDR,0x0000);   //clear OV
		ucSPI_Write(MAC_SPI_DEV,UVL_STAT_ADDR,0x0000);   //clear UV
		if((afe_flags.val.afe_ov_flag == 1)||(afe_flags.val.afe_uv_flag == 1))
		{
			sys_flags.val.afe_volt_protect_flag = 1;
		}
		//20160912 清除 afe_volt_protect_flag标志
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
	//20160907   0909 确认 屏蔽
	//     // 20160728 保护恢复 --保护--ovuv置位---解除--ovuv清0----手动写入crv+clr----输出恢复
	//    if((afe_flags.val.afe_ov_flag ==0)&&(afe_flags.val.afe_uv_flag ==0))
	//    {
	//        if(sys_flags.val.afe_volt_protect_flag == 1)
	//        {
	//            ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,0xa000);   //clear FDR
	//            __delay_us(10);
	//            if(ucSPI_Read(MAC_SPI_DEV,FDRV_CTRL_ADDR,spi_read_value)==0)  // UV HV SCD OCD DCD FLAG ---保护
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
		//保护恢复
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
	
	//FET DRIVE READ FET状态
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
 * @Output     :所有需要均衡，满足均衡条件的电芯都开启，在输出前根据时间切换奇数偶数
 * @Notice     ：
 * @Date       ：2016.12.8 20170123 zzyESben
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
//        if (diff_volt > VCELL_BALANCE_START) // 开始值50mV
//        {
            for (i = 0; i < 16; i++)//CELLS_NUM  以下需要奇数偶数分开
                //			for( i = j;i < 16;i += 2 )
            {
                if (nADC_Cell_Value[i] > nADC_CELL_MIN) 
                {
                    diff_volt = nADC_Cell_Value[i] - nADC_CELL_MIN; //每节电芯电压与最低电芯电压的压差
                    if (diff_volt > VCELL_BALANCE_START) //开始值  50mV
                    {
                        g_bal_need.VAL |= (0x01 << i); //将相应的标志位置高
                    } else if (diff_volt < VCELL_BALANCE_END) //结束值 电压精度 25？
                    {
                        g_bal_need.VAL &= ~(0x01 << i); //将相应的标志位置低
                    }
                }
            }
//        }
    } else if (sys_states.val.sys_dch_state == 0) //cell balance in relax mode
    {
//        if (diff_volt > VCELL_BALANCE_START) // 开始值50mV 静止状态下可小一点
//        {
            for (i = 0; i < 16; i++)//CELLS_NUM  以下需要奇数偶数分开
                //			for( i = j;i < 16;i += 2 )
            {
                if (nADC_Cell_Value[i] > nADC_CELL_MIN) 
                {
                    diff_volt = nADC_Cell_Value[i] - nADC_CELL_MIN; //每节电芯电压与最低电芯电压的压差
                    if (diff_volt > VCELL_BALANCE_START) //开始值  50mV
                    {
                        g_bal_need.VAL |= (0x01 << i); //将相应的标志位置高
                    } else if (diff_volt < VCELL_BALANCE_END) //结束值 电压精度 25？
                    {
                        g_bal_need.VAL &= ~(0x01 << i); //将相应的标志位置低
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
DESCRIPTION		: 做开启电池平衡工作
INPUT			: None
OUTPUT			: None
NOTICE			: 设置活动模式还是休眠模式、设置5VLDO处于正常模式，关闭OV、UV功能
                    设置标志位开启平衡、设置开启、
DATE			: 2017/01/24
*****************************************************************************/
void Cells_Bal_Open(void)
{
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_UNLOCK);   //unlock IC
    
    ucSPI_Write(MAC_SPI_DEV,SPICTL_ADDR,AFE_SPI_NM50);   //NM50_EN   AFE_SPI_NM50       
    ucSPI_Write(MAC_SPI_DEV,UVMSK_ADDR,0xFFFF);   //  15S     关闭OVUV?
    ucSPI_Write(MAC_SPI_DEV,OVMSK_ADDR,0xFFFF);   //  15S     关闭OVUV?        
    ucSPI_Write(MAC_SPI_DEV,CB_CTL_ADDR,AFE_Balance_EN );             //开启均衡
    ucSPI_Write(MAC_SPI_DEV,CBSEL_ADDR,g_bal_state.VAL );             //选定均衡电池
    ucSPI_Write(MAC_SPI_DEV,OP_MODE_ADDR,(AFE_AVD_LATCH | 0x0100));   //设置设置开启  
    
    ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_LOCK);   //lock IC
}