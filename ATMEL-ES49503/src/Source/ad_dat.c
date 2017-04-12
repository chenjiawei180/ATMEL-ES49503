/*
 * ad_dat.c
 *
 * Created: 2017/2/7 14:12:14
 *  Author: chenjiawei
 */ 

#include "ad_dat.h"
#include "spi.h"
#include "AN49503.h"
#include "history.h"
#include "afe_wr.h"
#include "usart.h"


uint16_t Cell_Value[4][RAM_P_CELL_SEREIES];
uint8_t cell_index = 0;


uint16_t nADC_VBAT = 0;
uint16_t nADC_VCHG = 0;
uint8_t ave_cnt = 0;


const uint16_t TEMP_AD_TABLE [142] = 
{
	15257, 15195, 15130, 15063, 14992, 14919, 14844, 14765, 14683, 14599,
	14512, 14421, 14328, 14231, 14132, 14030, 13924, 13816, 13704, 13590,
	13473, 13353, 13230, 13104, 12975, 12844, 12710, 12574, 12435, 12294,
	12150, 12005, 11857, 11708, 11556, 11403, 11248, 11092, 10935, 10776,
	10617, 10456, 10295, 10133, 9970, 9808, 9645, 9482, 9319, 9156,
	8994, 8832, 8671, 8510, 8350, 8192, 8034, 7877, 7722, 7568,
	7416, 7265, 7115, 6968, 6822, 6678, 6536, 6395, 6257, 6121,
	5987, 5855, 5725, 5597, 5472, 5348, 5227, 5108, 4991, 4877,
	4765, 4655, 4547, 4441, 4338, 4236, 4137, 4040, 3945, 3853,
	3762, 3673, 3586, 3502, 3419, 3338, 3259, 3182, 3107, 3034,
	2962, 2892, 2824, 2757, 2692, 2629, 2567, 2507, 2448, 2391,
	2335, 2281, 2227, 2176, 2125, 2076, 2028, 1981, 1936, 1891,
	1848, 1806, 1765, 1725, 1686, 1648, 1611, 1574, 1539, 1505,
	1471, 1437, 1403, 1369, 1335, 1301, 1267, 1233, 1199, 1165,
	1131, 1097,
};

const int8_t TEMP_TABLE[142] = 
{
	-30, -29, -28, -27, -26, -25, -24, -23, -22, -21,
	-20, -19, -18, -17, -16, -15, -14, -13, -12, -11,
	-10, -9, -8, -7, -6, -5, -4, -3, -2, -1,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
	70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
	90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
	100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
	110, 111,
};

/******************************************************************************
#ifdef DOC
FUNCTION		: vAPI_ADC_Read_Data_bal 均衡时调用 第一部分
DESCRIPTION		: A/d conversion result acquisition
INPUT			: wk_data	: A/D conversion data
OUTPUT			: AD conversion result
UPDATE			: -
DATE			: 2014/09/11
#endif
 ******************************************************************************/
void vAPI_ADC_Read_Data_bal_1(void) 
{
    uint16_t unvol;

    /* Cell Voltage */      //第二部分时测试单体电压
//    for (uci = 0; uci < RAM_P_CELL_SEREIES; uci++) {
//        unvol = ((uint16_t) ucSPI_Conti_RecvData[(CV01_AD_ADDR + uci)*2] << 8) +
//                ((uint16_t) ucSPI_Conti_RecvData[(CV01_AD_ADDR + uci)*2 + 1]);
//        nADC_Cell_Value[uci] = unvol;
//    }
    /* Thermistor */
    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI1_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI1_AD_ADDR)*2 + 1]);
    TEMP_1_PCB = vAPI_IndexNtcTemp(unvol);

    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI2_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI2_AD_ADDR)*2 + 1]);
    TEMP_2_PCB = vAPI_IndexNtcTemp(unvol);

    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI3_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI3_AD_ADDR)*2 + 1]);
    TEMP_3_BAT = vAPI_IndexNtcTemp(unvol);

    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI4_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI4_AD_ADDR)*2 + 1]);
    TEMP_4_BAT = vAPI_IndexNtcTemp(unvol);

    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI5_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI5_AD_ADDR)*2 + 1]);
    TEMP_5_BAT = vAPI_IndexNtcTemp(unvol);

    //	/* VDD50 */
    //	unvol	= ((uint16_t)ucSPI_Conti_RecvData[(VDD50_AD_ADDR)*2] << 8 ) +
    //			  ((uint16_t)ucSPI_Conti_RecvData[(VDD50_AD_ADDR)*2 + 1]);
    //    nADC_VDD50 = unvol;
    /* Current */
    unvol = ((uint16_t) ucSPI_Conti_RecvData[(CVIL_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(CVIL_AD_ADDR)*2 + 1]);
    nADC_CURRENT = unvol;
    //	/* VPAC 整体*/
    unvol = ((uint16_t) ucSPI_Conti_RecvData[(VPAC_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(VPAC_AD_ADDR)*2 + 1]);
    nADC_VPACK = unvol; //nADC_VBAT
    /* GPIO1 输出端*/
    //	unvol	= ((uint16_t)ucSPI_Conti_RecvData[(GPIO1_AD_ADDR)*2] << 8 ) +
    //			  ((uint16_t)ucSPI_Conti_RecvData[(GPIO1_AD_ADDR)*2 + 1]);
    //    nADC_VCHG = unvol;

    /* GPIO2 电芯端*/
    //	unvol	= ((uint16_t)ucSPI_Conti_RecvData[(GPIO2_AD_ADDR)*2] << 8 ) +
    //			  ((uint16_t)ucSPI_Conti_RecvData[(GPIO2_AD_ADDR)*2 + 1]);
    //    nADC_VBAT = unvol;
    nADC_VBAT = nADC_VPACK;
    nADC_VCHG = nADC_VPACK; //zzy20161022
//    vAPI_CalcCell();      //第二部分使用
//    vAPI_CalcTempture();
//    vAPI_Uart_Load();
//    His_Data_Save();
    //	vAPI_CalcFetTh();
}

/****************************************************************************
FUNCTION		: vAPI_IndexNtcTemp
DESCRIPTION		: ADC  Thermistor to value
INPUT			: None
OUTPUT			: None
UPDATE			:
DATE			: 14/10/21
 *****************************************************************************/
int8_t vAPI_IndexNtcTemp(uint16_t temp_ad_val) 
{
    uint8_t low = 0;
    uint16_t table_val;
    table_val = temp_ad_val;
    while (low < 141) 
	{
        if ((table_val > TEMP_AD_TABLE[low]) || (table_val == TEMP_AD_TABLE[low])) 
		{
            break;
        } 
		else 
		{
            low++;
        }
    }
    return TEMP_TABLE[low];
}

/******************************************************************************
FUNCTION		: vAPI_ADC_Read_Data_bal2 均衡时调用的第二部分 单体电压与处理
DESCRIPTION		: A/d conversion result acquisition
INPUT			: wk_data	: A/D conversion data
OUTPUT			: AD conversion result
UPDATE			: -
DATE			: 2014/09/11
 ******************************************************************************/
void vAPI_ADC_Read_Data_bal_2(void) 
{
    uint8_t uci;
    uint16_t unvol;

    /* Cell Voltage */
    for (uci = 0; uci < RAM_P_CELL_SEREIES; uci++) 
	{
        unvol = ((uint16_t) ucSPI_Conti_RecvData[(CV01_AD_ADDR + uci)*2] << 8) +
                ((uint16_t) ucSPI_Conti_RecvData[(CV01_AD_ADDR + uci)*2 + 1]);
        nADC_Cell_Value[uci] = unvol;
    }
    
    vAPI_CalcCell();
    vAPI_CalcTempture();
    //vAPI_Uart_Load();
    His_Data_Save();
}

/****************************************************************************
FUNCTION		: vAPI_CalcCell
DESCRIPTION		: ADC Calc Cell
INPUT			: None
OUTPUT			: None
UPDATE			:
DATE			: 14/09/11
 *****************************************************************************/

void vAPI_CalcCell(void) 
{
    uint8_t i, j;
    uint16_t temp;
    uint16_t Cell_Volt_temp[16];
    uint32_t total_volt;
    static bool cell_first_read = 0;
    if (cell_first_read == 0) 
	{
        cell_first_read = 1;
        for (i = 0; i < 16; i++) 
		{
            for (j = 0; j < 4; j++) 
			{
                Cell_Value[j][i] = nADC_Cell_Value[i] >> 2;
            }
            nADC_Cell_Value[i] = 0;
        }
    } 
	else 
	{
        for (i = 0; i < 16; i++) 
		{
            Cell_Value[cell_index][i] = nADC_Cell_Value[i] >> 2;
            nADC_Cell_Value[i] = 0;
        }
    }
    cell_index++;
    if (cell_index > 3) 
	{
        cell_index = 0;
    }

    for (i = 0; i < 16; i++) 
	{
        for (j = 0; j < 4; j++) 
		{
            nADC_Cell_Value[i] += Cell_Value[j][i];
        }
        Cell_Volt_temp[i] = nADC_Cell_Value[i];
    }

    //冒泡排序
    for (i = 0; i < 15; i++) //冒泡法排序
    {
        for (j = 0; j < 15 - i; j++) 
		{
            if (Cell_Volt_temp[j] > Cell_Volt_temp[j + 1]) 
			{
                temp = Cell_Volt_temp[j];
                Cell_Volt_temp[j] = Cell_Volt_temp[j + 1];
                Cell_Volt_temp[j + 1] = temp;
            }
        }
    }
    total_volt = 0;
    for (i = 3; i < 16; i++)//zzy20161022  2改成9
    {
        total_volt += Cell_Volt_temp[i];
    }
    Total_VBAT = total_volt / 13; //zzy20161020 
    nADC_CELL_MAX = Cell_Volt_temp[15]; // max cell voltage
    nADC_CELL_MIN = Cell_Volt_temp[3]; // min cell voltage 改为9
	
    // 平均电流
    if (nADC_CURRENT < 0) 
	{
        ave_cnt++;
        if (ave_cnt > 3) 
		{
            ave_cnt = 0;
        }
        AVE_CURRENT[ave_cnt] = nADC_CURRENT;
    }
}

/****************************************************************************
#ifdef DOC
FUNCTION		: vAPI_CalcCellTh
DESCRIPTION		: ADC Calc Cell Thermistor
INPUT			: None
OUTPUT			: None
UPDATE			:
DATE			: 14/10/21
#endif
 *****************************************************************************/
void vAPI_CalcTempture(void) 
{
    if (TEMP_1_PCB > TEMP_2_PCB) 
	{
        nADC_TMONI_PCB_MAX = TEMP_1_PCB;
        //nADC_TMONI_PCB_MIN = TEMP_2_PCB;
    } else 
	{
        nADC_TMONI_PCB_MAX = TEMP_2_PCB;
        //nADC_TMONI_PCB_MIN = TEMP_1_PCB;
    }
    if (TEMP_3_BAT > TEMP_4_BAT) 
	{
        if (TEMP_3_BAT > TEMP_5_BAT) 
		{
            nADC_TMONI_BAT_MAX = TEMP_3_BAT;
        } 
		else 
		{
            nADC_TMONI_BAT_MAX = TEMP_5_BAT;
        }
    } 
	else 
	{
        if (TEMP_4_BAT > TEMP_5_BAT) 
		{
            nADC_TMONI_BAT_MAX = TEMP_4_BAT;
        } 
		else 
		{
            nADC_TMONI_BAT_MAX = TEMP_5_BAT;
        }
    }
    //if (TEMP_3_BAT <-28) 
	//{
        //TEMP_3_BAT = TEMP_4_BAT;
    //}
    //if (TEMP_4_BAT <-28) 
	//{
        //TEMP_4_BAT = TEMP_5_BAT;
    //}
    //if (TEMP_5_BAT <-28) 
	//{
        //TEMP_5_BAT = TEMP_4_BAT;
    //}
    if (TEMP_3_BAT < TEMP_4_BAT) 
	{
        if (TEMP_3_BAT < TEMP_5_BAT) 
		{
            nADC_TMONI_BAT_MIN = TEMP_3_BAT;
        } 
		else 
		{
            nADC_TMONI_BAT_MIN = TEMP_5_BAT;
        }
    } 
	else 
	{
        if (TEMP_4_BAT < TEMP_5_BAT) 
		{
            nADC_TMONI_BAT_MIN = TEMP_4_BAT;
        } 
		else 
		{
            nADC_TMONI_BAT_MIN = TEMP_5_BAT;
        }
    }
}

/******************************************************************************
#ifdef DOC
FUNCTION		: ulAPI_ADC_Get_Data
DESCRIPTION		: A/d conversion result acquisition
INPUT			: wk_data	: A/D conversion data
OUTPUT			: AD conversion result
UPDATE			: -
DATE			: 2014/09/11
#endif
 ******************************************************************************/
void vAPI_ADC_Read_Data(void) {
    uint8_t uci;
    uint16_t unvol;

    /* Cell Voltage */
    for (uci = 0; uci < RAM_P_CELL_SEREIES; uci++) {
        unvol = ((uint16_t) ucSPI_Conti_RecvData[(CV01_AD_ADDR + uci)*2] << 8) +
                ((uint16_t) ucSPI_Conti_RecvData[(CV01_AD_ADDR + uci)*2 + 1]);
        nADC_Cell_Value[uci] = unvol;
	//#ifdef OS_DEBUG
		//printf("nADC_Cell_Value[%d] is %d. \r\n",uci,nADC_Cell_Value[uci]*305/1000);	
	//#endif
    }
    /* Thermistor */
    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI1_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI1_AD_ADDR)*2 + 1]);
    TEMP_1_PCB = vAPI_IndexNtcTemp(unvol);

    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI2_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI2_AD_ADDR)*2 + 1]);
    TEMP_2_PCB = vAPI_IndexNtcTemp(unvol);

    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI3_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI3_AD_ADDR)*2 + 1]);
    TEMP_3_BAT = vAPI_IndexNtcTemp(unvol);

    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI4_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI4_AD_ADDR)*2 + 1]);
    TEMP_4_BAT = vAPI_IndexNtcTemp(unvol);

    unvol = ((uint16_t) ucSPI_Conti_RecvData[(TMONI5_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(TMONI5_AD_ADDR)*2 + 1]);
    TEMP_5_BAT = vAPI_IndexNtcTemp(unvol);

	//#ifdef OS_DEBUG
	//printf("温度1 is %d ℃. \r\n",TEMP_1_PCB);
	//printf("温度2 is %d ℃. \r\n",TEMP_2_PCB);
	//printf("温度3 is %d ℃. \r\n",TEMP_3_BAT);
	//printf("温度4 is %d ℃. \r\n",TEMP_4_BAT);
	//printf("温度5 is %d ℃. \r\n",TEMP_5_BAT);
	//#endif

    //	/* VDD50 */
    //	unvol	= ((uint16_t)ucSPI_Conti_RecvData[(VDD50_AD_ADDR)*2] << 8 ) +
    //			  ((uint16_t)ucSPI_Conti_RecvData[(VDD50_AD_ADDR)*2 + 1]);
    //    nADC_VDD50 = unvol;
    /* Current */
    unvol = ((uint16_t) ucSPI_Conti_RecvData[(CVIL_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(CVIL_AD_ADDR)*2 + 1]);
    nADC_CURRENT = unvol;
    //	/* VPAC 整体*/
    unvol = ((uint16_t) ucSPI_Conti_RecvData[(VPAC_AD_ADDR)*2] << 8) +
            ((uint16_t) ucSPI_Conti_RecvData[(VPAC_AD_ADDR)*2 + 1]);
    nADC_VPACK = unvol; //nADC_VBAT
	//#ifdef OS_DEBUG
	//printf("nADC_VPACK is %d. \r\n",nADC_VPACK*610/100);
	//#endif
    /* GPIO1 输出端*/
    //	unvol	= ((uint16_t)ucSPI_Conti_RecvData[(GPIO1_AD_ADDR)*2] << 8 ) +
    //			  ((uint16_t)ucSPI_Conti_RecvData[(GPIO1_AD_ADDR)*2 + 1]);
    //    nADC_VCHG = unvol;

    /* GPIO2 电芯端*/
    //	unvol	= ((uint16_t)ucSPI_Conti_RecvData[(GPIO2_AD_ADDR)*2] << 8 ) +
    //			  ((uint16_t)ucSPI_Conti_RecvData[(GPIO2_AD_ADDR)*2 + 1]);
    //    nADC_VBAT = unvol;
    nADC_VBAT = nADC_VPACK;
    nADC_VCHG = nADC_VPACK; //zzy20161022
    vAPI_CalcCell();
    vAPI_CalcTempture();
    //vAPI_Uart_Load();
    //His_Data_Save();
    //	vAPI_CalcFetTh();
}

/****************************************************************************
FUNCTION		: AFE_HardwareProtection_Write
DESCRIPTION		: 写入硬件参数
INPUT			: None
OUTPUT			: 完成标志,0为完成
NOTICE			: 仅操作一次以免出现某次操作异常,需要写入后读取校对
DATE			: 2016/06/24
*****************************************************************************/
uint8_t AFE_HardwareProtection_Write(void)
{
	ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_UNLOCK);   //unlock IC
	ucSPI_Write(MAC_SPI_DEV,ALARM_CTRL1_ADDR,AFE_HARDWARE_ALARM_EN);   // en SCD OCD OCC CP  假如BIT15 = 1,那么只针对短路告警
	ucSPI_Write(MAC_SPI_DEV,GPIOSEL_ADDR,AFE_ADIQ2_AMARM2_EN);   //enable GPIO5 ADIR&GPIO6 ALARM2
	
	ucSPI_Write(MAC_SPI_DEV,ALARM_CTRL2_ADDR,AFE_200A_75A_40A);   //zzy20161026 50mV/DIV 50A SCD/短路  25mV/DIV 25A OCD/过流 10A OCC/充电过流  10mV/DIV   默认值，不设置
	ucSPI_Write(MAC_SPI_DEV,ALARM_CTRL3_ADDR,AFE_50us_1ms_1ms);   //保护时间  544  00000 50us SCD   0000 1ms OCD  0000 1ms  OCC                 默认值，不设置
	ucSPI_Write(MAC_SPI_DEV,OUVCTL1_ADDR,AFE_H420V_L275V);   //  10110 50mV/DIV 4.2V  10110 2.7V
	ucSPI_Write(MAC_SPI_DEV,OUVCTL2_ADDR,AFE_UV_350mV_1S);   //  延迟和解除值  默认100mV 1S
	//15S --
	ucSPI_Write(MAC_SPI_DEV,CVSEL_ADDR,AFE_CELL13S);   //  15S       zzy20161020
	ucSPI_Write(MAC_SPI_DEV,UVMSK_ADDR,AFE_CELL13S_P);   //  15S     zzy20161020
	ucSPI_Write(MAC_SPI_DEV,OVMSK_ADDR,AFE_CELL13S_P);   //  15S     zzy20161020
	//15S END
	//FET CONTROL
	ucSPI_Write(MAC_SPI_DEV,FDRV_CTRL_ADDR,AFE_FET_AUTO_CONTROL);   //  硬件保护开启
	
	ucSPI_Read(MAC_SPI_DEV,PWR_CTRL_ADDR,spi_read_value);
	PWR_VALUE = spi_read_value[0]|AFE_ADC_EN_CONT;
	ucSPI_Write(MAC_SPI_DEV,PWR_CTRL_ADDR,PWR_VALUE);   //enable AD conti
	
	
	ucSPI_Write(MAC_SPI_DEV,LOCK_ADDR,AFE_LOCK);   //lock IC
	
	delay_us(100);
	if(ucSPI_Read(MAC_SPI_DEV,FDRV_CTRL_ADDR,spi_read_value) ==0)
	{
		if((spi_read_value[0]&AFE_FET_AUTO_CONTROL) ==0)
		{
			return 1;
		}
	}
	if(ucSPI_Read(MAC_SPI_DEV,ALARM_CTRL1_ADDR,spi_read_value) ==0)
	{
		if((spi_read_value[0]&AFE_HARDWARE_ALARM_EN) ==0)
		{
			return 1;
		}
	}
	if(ucSPI_Read(MAC_SPI_DEV,PWR_CTRL_ADDR,spi_read_value) ==0)
	{
		if((spi_read_value[0]&AFE_ADC_EN_CONT) ==0)
		{
			return 1;
		}
	}
	PWR_VALUE = AFE_ADC_EN_CONT; //PWR寄存器包含了MOSFET的控制,需要保护现场
	if(ucSPI_Read(MAC_SPI_DEV,OUVCTL1_ADDR,spi_read_value) ==0)
	{
		if(spi_read_value[0] == AFE_H420V_L275V)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	return 1;
}

/****************************************************************************
#ifdef DOC
FUNCTION		: vAPI_Uart_Load
DESCRIPTION		: Uart Load
INPUT			: None
OUTPUT			: None
UPDATE			:
DATE			: 14/09/11
#endif
 *****************************************************************************/
void vAPI_Uart_Load(void) 
{
    uint8_t uci;
    /* Cell Voltage */
    for (uci = 0; uci < RAM_P_CELL_SEREIES; uci++) {
        TxBuffer[uci * 2 + 10] = nADC_Cell_Value[uci] >> 8;
        TxBuffer[uci * 2 + 10 + 1] = nADC_Cell_Value[uci];
    }
    //TEST
    //    TxBuffer[10] = nADC_CELL_MIN>>8;
    //    TxBuffer[11] = nADC_CELL_MIN;

    /* Thermistor */
    TxBuffer[50] = TEMP_1_PCB; //nADC_TMONT_Value[0]>>8
    TxBuffer[51] = TEMP_2_PCB; //nADC_TMONT_Value[0]
    TxBuffer[52] = TEMP_3_BAT;
    TxBuffer[53] = TEMP_4_BAT;
    TxBuffer[54] = TEMP_5_BAT;
    /* VDD50 */
    //    TxBuffer[44] = nADC_VDD50>>8;
    //    TxBuffer[45] = nADC_VDD50;
    /* VCHG */
    TxBuffer[44] = nADC_VCHG >> 8;
    TxBuffer[45] = nADC_VCHG;
    /* Current */
    TxBuffer[48] = nADC_CURRENT >> 8;
    TxBuffer[49] = nADC_CURRENT;
    /* VPAC */
    TxBuffer[46] = nADC_VPACK >> 8;
    TxBuffer[47] = nADC_VPACK;
    /* GPIO1 */
    TxBuffer[42] = nADC_VBAT >> 8; //nADC_VBAT
    TxBuffer[43] = nADC_VBAT;
}