/******************************************************************************/
/* File Name       : tsk_learnData.c                                          */
/* Description     : This file is source code of the learn data thread in the */
/*                   application layer.                                       */
/* Author          : TanTrinh                                                 */
/* Created         : April 15, 2018                                           */
/* Modified by     : Tan Trinh                                                */
/* Date Modified   : April 15, 2018                                           */
/* Content Modified:                                                          */
/* Copyright (c) 2015 SH Consuting K.K.                                       */
/*  and SH Consulting VietNam. All Rights Reserved.                           */
/******************************************************************************/

/******************************************************************************/
/*************************** Project Header ***********************************/
/******************************************************************************/
#include "tsk_learnData.h"

/******************************************************************************/
/*************************** Macro Definitions ********************************/
/******************************************************************************/
#define TSK_PRI_LEARN_DATA              5
#define TSK_STACK_SIZE_LEARN_DATA       (128*1024)

#define LDATA_REG_FINGER_RED_NUM        1
#define LDATA_REG_FINGER_BLUE_NUM       2
#define LDATA_REG_FINGER_GREEN_NUM      3
#define LDATA_REG_FINGER_NUM            3

/* Test Flash Memory */
#define LDATA_WORD_SIZE                 (sizeof(UW))
#define LDATA_FRAME_DATA_SIZE           (sizeof(SvLearnData))
// #define CTRL_FLAG_SIZE					(sizeof(char)*2) //// 2 bytes
// #define CTRL_FLAG_OFFSET				(MI_SECTOR_SIZE - CTRL_FLAG_SIZE)
//////////////////////////////////////


/******************************************************************************/
/************************ Enumerations Definitions ****************************/
/******************************************************************************/
typedef enum {
    LDATA_ADD_CASE    = 0,
    LDATA_SEARCH_CASE,
    LDATA_RING_CASE,
	LDATA_MAPPING_CASE,
    LDATA_NOTIFY_CASE,
    
    LDATA_UNKNOW_CASE,
} ldataTestCaseType;

enum{
	TC_INIT_01 = 0,
	TC_INIT_02,
	TC_INIT_TOTAL
};

enum{
    TC_ADD_01 = 0,
    TC_ADD_02,
    TC_RING_01,
    TC_RING_02,
    TC_RING_03,
    TC_RING_04,
    TC_RING_05,
	TC_UNIQUE_PREPARE_01,
	TC_UNIQUE_PREPARE_02,
	TC_UNIQUE_PREPARE_19,
	TC_UNIQUE_ADD_01,	// 1 unique data in 1 Section
    TC_UNIQUE_ADD_02,	// 2 unique data in 1 Section
    TC_UNIQUE_ADD_19,	// 19 unique data in 1 Section
    TC_ADD_TOTAL
};

enum{
	TC_SEARCH_01 = 0, // Search room 0
	TC_SEARCH_02, // Search room 479
	TC_SEARCH_03, // Search room 480
	TC_SEARCH_TOTAL
};


/******************************************************************************/
/*************************** External Functions Prototype ******************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Functions Prototype ******************************/
/******************************************************************************/
static int lcalc_FingerDataIndex(int pos);
static UB lFingerDataImg(int pos);
static SvLearnData* lcreateLearnData(int pos);
static BOOL lmiReadHword(UW addr, UH* halfWord);
static BOOL lmireadWord(UW addr, UW* Word);
static BOOL lmiReadRange(UW addr, UH* buffer, UW length);
static int lcalc_LDLocation(int numberOfLearnData, UW* bank_index, UW* section_index, UW* frame_index);
static UW lcalc_FrameAddr(UW bank_index, UW section_index, UW frame_index);
static BOOL lreadFrame(UW bank_index, UW section_index, UW frame_index, SvLearnData *learnDataPtr);
static UW lcalc_CtrlFlgAddr(UW bank_index, UW section_index);
static volatile UH lread_CtrlFlg(UW bank_index, UW section_index);
static void lwrite_CtrlFlg(UW bank_index, UW section_index, UH value);
static void lchangeToSvLearnResult(SvLearnResult *ld, SvLearnData *lDataPtr);
static BOOL lcheck_BankIndex(UW bank_index);
static BOOL lcheck_SecIndex(UW section_index);
static BOOL lcheck_FrmIndex(UW frame_index);
static BOOL lcheck_RegStatus(UH checked_val);
static BOOL lcheck_RegRnum(UH checked_val);
static BOOL lcheck_RegYnum(UH checked_val);
static BOOL lcheck_RegImg1(UH checked_val, UH expected_val);
static BOOL lcheck_SvLearnDataResult(SvLearnResult* ld ,UB data);
static BOOL lcheck_ctrl_flg(UW bank_index_oldest, UW section_index_oldest, UW erased_bank_index, UW erased_section_index);
static BOOL lcheck_RemoveAllData(UW bank_index, UW section_index);
static BOOL lcheck_notify_data(int notifyCase);
static BOOL lcheck_SearchLearnImg(int index, UH SearchNum, int expected_val);

// static void init_InitBankAreaResult(void);
// static void init_AddSvLearnImgResult(void);
// static void init_NotifyDataResult(void);
#ifdef FAST_TEST
static void fast_test(UW *num);
#endif
static void init_SearchLearnImgResult(void);
static void TaskLearnDataInit(void);

TASK TaskLearnData(void);
void taskLearnDataHandler(void);

/******************************************************************************/
/*************************** Constant Local Variables *************************/
/******************************************************************************/
// #if TSK_LEARN_DATA_TEST_ENABLE
static const T_CTSK ctsk_learn_data = {TA_HLNG, NULL, (FP)TaskLearnData, TSK_PRI_LEARN_DATA,
                                       TSK_STACK_SIZE_LEARN_DATA, NULL, "Lean Data Task"};

//static volatile SvLearnData ldRegFingerRed, ldRegFingerBlue, ldRegFingerGreen[4];
static volatile SvLearnData g_LearnDataArray[LDATA_FINGER_INDEX_TOTAL];
static volatile UB g_FingerDataImgArray[LDATA_FINGER_INDEX_TOTAL] = {
	/* default data */
	0x11,                  // LDATA_FINGER_RED_1_INDEX
	0x22,                  // LDATA_FINGER_BLUE_1_INDEX
    /* 2 only one data */
    0x41, //'+'            // LDATA_FINGER_YELLOW_1_INDEX
    0x42, //'='            // LDATA_FINGER_YELLOW_2_INDEX       
    /* 19 only one data */
    0x51,                  // LDATA_FINGER_ORANGE_1_INDEX          
    0x52,                  // LDATA_FINGER_ORANGE_2_INDEX          
    0x53,                  // LDATA_FINGER_ORANGE_3_INDEX          
    0x54,                  // LDATA_FINGER_ORANGE_4_INDEX          
    0x55,                  // LDATA_FINGER_ORANGE_5_INDEX          
    0x56,                  // LDATA_FINGER_ORANGE_6_INDEX          
    0x57,                  // LDATA_FINGER_ORANGE_7_INDEX          
    0x58,                  // LDATA_FINGER_ORANGE_8_INDEX          
    0x59,                  // LDATA_FINGER_ORANGE_9_INDEX          
    0x5A,                  // LDATA_FINGER_ORANGE_10_INDEX         
    0x5B,                  // LDATA_FINGER_ORANGE_11_INDEX         
    0x5C,                  // LDATA_FINGER_ORANGE_12_INDEX         
    0x5D,                  // LDATA_FINGER_ORANGE_13_INDEX         
    0x5E,                  // LDATA_FINGER_ORANGE_14_INDEX         
    0x5F,                  // LDATA_FINGER_ORANGE_15_INDEX         
    0x60,                  // LDATA_FINGER_ORANGE_16_INDEX         
    0x61,                  // LDATA_FINGER_ORANGE_17_INDEX         
    0x62,                  // LDATA_FINGER_ORANGE_18_INDEX         
    0x63,                  // LDATA_FINGER_ORANGE_19_INDEX         
	/* 1 only one data */
    0x33 //'%'             // LDATA_FINGER_GREEN_1_INDEX
};

// #endif

/******************************************************************************/
/*************************** Local Variables **********************************/
/******************************************************************************/
static UW g_numberOfLearnData;
static ID s_idTskLearData;
static UW g_bank_index=0, g_section_index=0, g_frame_index=0;
static UW g_bank_oldest_index=0, g_section_oldest_index=0;
static UH g_ctrl_flg_oldest=0;
static SvLearnData g_learnData;
static SvLearnResult g_learnDataResult;
static SearchLearnDataResult g_SearchLearnDataResult[TC_SEARCH_TOTAL];
/******************************************************************************/
/*************************** Local Functions **********************************/
/******************************************************************************/
/******************************************************************************/
/*************************** Debug  *******************************************/
/******************************************************************************/
#define DEBUG
#ifdef DEBUG
#define DBG_LINE_TOTAL 180
static char log_table[DBG_LINE_TOTAL][80];
static void dbg_init(void)
{
	memset(log_table, 0, sizeof(log_table));
}
#define DBG_PRINT0(msg)						dbg_print(msg, 0, 0, 0, 0, 0, 0, 0, 0)
#define DBG_PRINT1(msg, arg1)				dbg_print(msg, arg1, 0, 0, 0, 0, 0, 0, 0)
#define DBG_PRINT2(msg, arg1, arg2)			dbg_print(msg, arg1, arg2, 0, 0, 0, 0, 0, 0)
#define DBG_PRINT3(msg, arg1, arg2, arg3)	dbg_print(msg, arg1, arg2,arg3, 0, 0, 0, 0, 0)
#define DBG_PRINT4(msg, arg1, arg2, arg3, arg4)				dbg_print(msg, arg1, arg2, arg3, arg4, 0, 0, 0, 0)
#define DBG_PRINT5(msg, arg1, arg2, arg3, arg4, arg5)		dbg_print(msg, arg1, arg2, arg3, arg4, arg5, 0, 0, 0)
#define DBG_PRINT6(msg, arg1, arg2, arg3, arg4, arg5, arg6)	dbg_print(msg, arg1, arg2, arg3, arg4, arg5, arg6, 0, 0)
#define DBG_PRINT7(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7)		dbg_print(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, 0)
#define DBG_PRINT8(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)	dbg_print(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
static void dbg_print(char* msg, UW arg1, UW arg2, UW arg3, UW arg4, UW arg5, UW arg6, UW arg7, UW arg8)
{
	static int i = 0;
	if(i < DBG_LINE_TOTAL)
	{
		sprintf((void*)log_table[i++], (void*)msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	}else
	{
		i = 0;
		sprintf((void*)log_table[i++], (void*)msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	}
}
#else
#define DBG_PRINT0(msg)					
#define DBG_PRINT1(msg, arg1)				
#define DBG_PRINT2(msg, arg1, arg2)		
#define DBG_PRINT3(msg, arg1, arg2,arg3)
#define DBG_PRINT4(msg, arg1, arg2, arg3, arg4)
#define DBG_PRINT5(msg, arg1, arg2, arg3, arg4, arg5)
#define DBG_PRINT6(msg, arg1, arg2, arg3, arg4, arg5, arg6)
#define DBG_PRINT7(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define DBG_PRINT8(msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#endif

static int lcalc_FingerDataIndex(int pos)
{
	int index = 0;

	switch (pos){
	case LDATA_FINGER_YELLOW_1_POS  :  {index = LDATA_FINGER_YELLOW_1_INDEX;    break;}
	case LDATA_FINGER_YELLOW_2_POS  :  {index = LDATA_FINGER_YELLOW_2_INDEX;    break;}
	case LDATA_FINGER_ORANGE_1_POS  :  {index = LDATA_FINGER_ORANGE_1_INDEX;    break;}
	case LDATA_FINGER_ORANGE_2_POS  :  {index = LDATA_FINGER_ORANGE_2_INDEX;    break;}
	case LDATA_FINGER_ORANGE_3_POS  :  {index = LDATA_FINGER_ORANGE_3_INDEX;    break;}
	case LDATA_FINGER_ORANGE_4_POS  :  {index = LDATA_FINGER_ORANGE_4_INDEX;    break;}
	case LDATA_FINGER_ORANGE_5_POS  :  {index = LDATA_FINGER_ORANGE_5_INDEX;    break;}
	case LDATA_FINGER_ORANGE_6_POS  :  {index = LDATA_FINGER_ORANGE_6_INDEX;    break;}
	case LDATA_FINGER_ORANGE_7_POS  :  {index = LDATA_FINGER_ORANGE_7_INDEX;    break;}
	case LDATA_FINGER_ORANGE_8_POS  :  {index = LDATA_FINGER_ORANGE_8_INDEX;    break;}
	case LDATA_FINGER_ORANGE_9_POS  :  {index = LDATA_FINGER_ORANGE_9_INDEX;    break;}
	case LDATA_FINGER_ORANGE_10_POS :  {index = LDATA_FINGER_ORANGE_10_INDEX;   break;}
	case LDATA_FINGER_ORANGE_11_POS :  {index = LDATA_FINGER_ORANGE_11_INDEX;   break;}
	case LDATA_FINGER_ORANGE_12_POS :  {index = LDATA_FINGER_ORANGE_12_INDEX;   break;}
	case LDATA_FINGER_ORANGE_13_POS :  {index = LDATA_FINGER_ORANGE_13_INDEX;   break;}
	case LDATA_FINGER_ORANGE_14_POS :  {index = LDATA_FINGER_ORANGE_14_INDEX;   break;}
	case LDATA_FINGER_ORANGE_15_POS :  {index = LDATA_FINGER_ORANGE_15_INDEX;   break;}
	case LDATA_FINGER_ORANGE_16_POS :  {index = LDATA_FINGER_ORANGE_16_INDEX;   break;}
	case LDATA_FINGER_ORANGE_17_POS :  {index = LDATA_FINGER_ORANGE_17_INDEX;   break;}
	case LDATA_FINGER_ORANGE_18_POS :  {index = LDATA_FINGER_ORANGE_18_INDEX;   break;}
	case LDATA_FINGER_ORANGE_19_POS :  {index = LDATA_FINGER_ORANGE_19_INDEX;   break;}
	case LDATA_FINGER_GREEN_1_POS   :  {index = LDATA_FINGER_GREEN_1_INDEX;     break;}
	//case LDATA_FINGER_BLUE_1_POS   :  {index = LDATA_FINGER_BLUE_1_INDEX;     break;}
	case LDATA_FINGER_RED_1_POS :
	default                         :  {index = LDATA_FINGER_RED_1_INDEX;       break;}
	};
	return index;
}

static UB lFingerDataImg(int pos)
{
	return g_FingerDataImgArray[lcalc_FingerDataIndex(pos)];
}

static SvLearnData* lcreateLearnData(int pos)
{
    SvLearnData *learnDataPtr;
    int i;
	i = lcalc_FingerDataIndex(pos);
	learnDataPtr = &g_LearnDataArray[lcalc_FingerDataIndex(pos)];
	if(i == LDATA_FINGER_GREEN_1_INDEX)
	{
		learnDataPtr->RegRnum = 1;
		learnDataPtr->RegYnum = i-LDATA_FINGER_GREEN_1_INDEX;
		learnDataPtr->RegID = 0x0100 + 4; //4;	// GREEN ID
	}
	else if(i == LDATA_FINGER_YELLOW_1_INDEX || i == LDATA_FINGER_YELLOW_2_INDEX)
    {
    	learnDataPtr->RegRnum = 2;
    	learnDataPtr->RegYnum = i-LDATA_FINGER_YELLOW_1_INDEX;
		learnDataPtr->RegID = i-LDATA_FINGER_YELLOW_1_INDEX + 0x0200;	// YELLOW ID
    }
    else if(i >= LDATA_FINGER_ORANGE_1_INDEX && i <= LDATA_FINGER_ORANGE_19_INDEX)
    {
    	learnDataPtr->RegRnum = 3;
    	learnDataPtr->RegYnum = i-LDATA_FINGER_ORANGE_1_INDEX;
		learnDataPtr->RegID = i-LDATA_FINGER_ORANGE_1_INDEX + 0x1900;	// ORANGE ID
    }else // i == LDATA_FINGER_RED_1_INDEX
    {
    	learnDataPtr->RegRnum = 0;//20;
    	learnDataPtr->RegYnum = 0;
		learnDataPtr->RegID = 7;	// RED ID
    }
    return learnDataPtr;
}
/******************************************************************************/
/* Function Name: miReadHword                                                 */
/* Description  : This function will read half word data from flash memory.   */
/* Parameter    : Input: addr - is start address that needs to read data from */
/*                       flash memory. value of this is nulti half word.      */
/*                Input: buffer - is a pointer that stores data for reading   */
/*                Input: length - is size that reads data.                    */
/* Return Value : BOOL - return the TRUE value when reads successful, else to */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL lmiReadHword(UW addr, UH* halfWord)
{
    BOOL result;
    
    result = FALSE;
    if((addr >= MI_FLASH_START) && (addr < (MI_FLASH_START + MI_FLASH_SIZE))) {
        if((halfWord != NULL) && ((addr % sizeof(UH)) == 0)) {
            result = (FlRead(addr, halfWord, 1) == E_OK);
        }
    }
    
    return result;
}

static BOOL lmireadWord(UW addr, UW* Word)
{
    BOOL result;
    
    result = FALSE;
    if((addr >= MI_FLASH_START) && (addr < (MI_FLASH_START + MI_FLASH_SIZE))) {
        if((Word != NULL) && ((addr % sizeof(UH)) == 0)) {
            result = (FlRead(addr, Word, 2) == E_OK);
        }
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: miReadRange                                                 */
/* Description  : This function will read data at start address from flash.   */
/* Parameter    : Input: addr - is start address that needs to read data from */
/*                       flash memory                                         */
/*                Input: buffer - is a pointer that stores data for reading   */
/*                Input: length - is size that reads data.                    */
/* Return Value : BOOL - return the TRUE value when reads successful, else to */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL lmiReadRange(UW addr, UH* buffer, UW length)
{
    BOOL result;
    ER errCode;
    
    result = FALSE;
	errCode = FlRead(addr, (UH*)buffer, length/sizeof(UH));
	if(errCode != E_OK) {
		result = FALSE;
	}
    
    return result;
}

static int lcalc_LDLocation(int numberOfLearnData, UW* bank_index, UW* section_index, UW* frame_index)
{
	UW data_index;
	data_index = numberOfLearnData - 1; //// amount to index
	if(data_index >= 0)
	{
	*bank_index = ((data_index / N_FRM_PER_BANK) % N_BANK_INIT) + BANK_OFFSET; //// [0:4]+3 = [3:7]
	*section_index = (data_index % N_FRM_PER_BANK) / N_FRM_PER_SECT; //// 0:255
	*frame_index = (data_index % N_FRM_PER_BANK) % N_FRM_PER_SECT; //// 0:19
	}
	else {
		return -1;
	}

	return 0;
}

static UW lcalc_FrameAddr(UW bank_index, UW section_index, UW frame_index)
{
	UW bankAddr, secAddr, frmAddr;	// 32-bit address
	BOOL ret = FALSE;
	frmAddr = 0;

	ret = lcheck_BankIndex(bank_index);
	if(ret == TRUE)
	{
		ret = lcheck_SecIndex(section_index);
	}

	if(ret == TRUE)
	{
		ret = lcheck_FrmIndex(frame_index);
	}

	if(ret == TRUE)
	{
		bankAddr = MI_FLASH_BASIC_ADDR + (bank_index*MI_BANK_SIZE);
		secAddr = bankAddr + section_index*MI_SECTOR_SIZE;// + 0x100000; // !!!! add 0x100000 to fix bug of learnData.c
		frmAddr = secAddr + frame_index*sizeof(SvLearnData);
	}

//	if(bank_index >= BANK_NUM_MIN && bank_index <= BANK_NUM_MAX)
//	{
//		if(section_index >= 0  && section_index < N_SECT_PER_BANK)
//		{
//			if(frame_index >= 0 && frame_index < N_FRM_PER_SECT)
//			{
//				bankAddr = MI_FLASH_BASIC_ADDR + (bank_index*MI_BANK_SIZE);
//				secAddr = bankAddr + section_index*MI_SECTOR_SIZE;// + 0x100000; // !!!! add 0x100000 to fix bug of learnData.c
//				frmAddr = secAddr + frame_index*sizeof(SvLearnData);
//			}
//		}
//	}
	return frmAddr;	
}

static BOOL lreadFrame(UW bank_index, UW section_index, UW frame_index, SvLearnData *learnDataPtr)
{
	BOOL result;
	ER errCode;
	UW frmAddr;

	result = FALSE;
	frmAddr = lcalc_FrameAddr(bank_index, section_index, frame_index);
	errCode = FlRead(frmAddr, (UH*)learnDataPtr, sizeof(SvLearnData)/sizeof(UH) );
	if(errCode != E_OK) {
		result = FALSE;
	}
	return result;
}

/*
 * bank_index = [3:7]
 * section_index = [0:18]
*/ 
static UW lcalc_CtrlFlgAddr(UW bank_index, UW section_index)
{
	UW bankAddr, secAddr;	// 32-bit address
	UW ctrl_addr; 	// 32-bit address
	BOOL ret = FALSE;

	ctrl_addr = 0;
	ret = lcheck_BankIndex(bank_index);
	if(ret == TRUE)
	{
		ret = lcheck_SecIndex(section_index);
	}

	if(ret == TRUE)
	{
		bankAddr = MI_FLASH_BASIC_ADDR + (bank_index*MI_BANK_SIZE);
		secAddr = bankAddr + section_index*MI_SECTOR_SIZE;
		ctrl_addr = secAddr + CTRL_FLAG_OFFSET;
	}


//	if(bank_index >= BANK_NUM_MIN && bank_index <= BANK_NUM_MAX)
//	{
//		if(section_index >= 0  && section_index < N_SECT_PER_BANK)
//		{
//			bankAddr = MI_FLASH_BASIC_ADDR + (bank_index*MI_BANK_SIZE);
//			secAddr = bankAddr + section_index*MI_SECTOR_SIZE;
//			ctrl_addr = secAddr + CTRL_FLAG_OFFSET;
//		}else {
//			return 0;
//		}
//	}else return 0;
	return ctrl_addr;
}

static volatile UH lread_CtrlFlg(UW bank_index, UW section_index)
{
	UW ctrl_addr;
	volatile UH value = 0;
	ctrl_addr = lcalc_CtrlFlgAddr(bank_index, section_index);
	if(ctrl_addr != 0)
	{
		lmiReadHword(ctrl_addr, &value);
	}
	if(value == 0x0001)
	{
		// Update oldest Section
		g_bank_oldest_index = bank_index;
		g_section_oldest_index = section_index;
		g_ctrl_flg_oldest = 0x0001;
	}
	return value;
}

static void lwrite_CtrlFlg(UW bank_index, UW section_index, UH value)
{
	// UW ctrl_addr = 0; 	// 32-bit address
	// UH *p;
	// ctrl_addr = lcalc_CtrlFlgAddr(bank_index, section_index);
	// if(ctrl_addr != 0)
	// {
		// p = (UH*)(ctrl_addr); 
		// *p = value; // 16-bit value
	// }
}

static void lchangeToSvLearnResult(SvLearnResult* ld, SvLearnData* lDataPtr)
{
	ld->RegStatus = lDataPtr->RegStatus;
	ld->RegRnum = lDataPtr->RegRnum;
	ld->RegYnum = lDataPtr->RegYnum;
	ld->RegID   = lDataPtr->RegID;
	// ld->RegImg1 = (UW)(lDataPtr->RegImg1[3] << 24) | (UW)(lDataPtr->RegImg1[2] << 16) | (UW)(lDataPtr->RegImg1[1] << 8) | (UW)(lDataPtr->RegImg1[0] << 0);
	ld->RegImg1 = lDataPtr->RegImg1[0]; // 1st byte
}

static BOOL lcheck_BankIndex(UW bank_index)
{
	return (bank_index >= BANK_NUM_MIN && bank_index <= BANK_NUM_MAX);
}

static BOOL lcheck_SecIndex(UW section_index)
{
	return (section_index >= 0  && section_index < N_SECT_PER_BANK);
}

static BOOL lcheck_FrmIndex(UW frame_index)
{
	return (frame_index >= 0 && frame_index < N_FRM_PER_SECT);
}

static BOOL lcheck_RegStatus(UH checked_val)
{
    // LDATA_NOT_YET_STS   = 0xFFFF,  //// Not yet
    // LDATA_DUR_REG_STS   = 0xFFFE,  //// During registration
    // LDATA_REGISTERD_STS = 0xFFFC, //// Registered. It is the latest data
    // LDATA_NOT_LATEST_STS= 0xFFF8, //// Not latest data. Old register
    // LDATA_UNKNOW_STS
	return checked_val == LDATA_REGISTERD_STS;
}

static BOOL lcheck_RegRnum(UH checked_val)
{
	return checked_val < LDATA_REG_NBR_MAX;
}

static BOOL lcheck_RegYnum(UH checked_val)
{
	return checked_val < LDATA_REG_FIGURE_NBR_MAX;
}

static BOOL lcheck_RegImg1(UH checked_val, UH expected_val)
{
	// DBG_PRINT2("[lcheck_RegImg1]: 0x%0.2x (expected 0x%0.2x)", checked_val, expected_val);
	return checked_val == expected_val;
}

static BOOL lcheck_SvLearnDataResult(SvLearnResult* ld ,UB data)
{	
	BOOL result;
	result = FALSE;
	result = lcheck_RegStatus(ld->RegStatus); // 0xFFFC : Registered (Newest)
	// if(result == TRUE) 
	{
		result &= lcheck_RegRnum(ld->RegRnum);
	}
	
	// if(result == TRUE) 
	{
		result &= lcheck_RegYnum(ld->RegYnum);
	}
	
	// if(result == TRUE) 
	{
		result &= lcheck_RegImg1((UB)ld->RegImg1, data);
	}
	
	return result;
}

static BOOL lcheck_SvLearnDataAt(UW bank_index, UW section_index, UW frame_index, UH data)
{
	SvLearnData *lDataPtr;
	SvLearnResult *ld_result;
	BOOL result;
	result = FALSE;
	
	lDataPtr = &g_learnData;
	ld_result = &g_learnDataResult;
	lreadFrame(bank_index, section_index, frame_index, lDataPtr);
	lchangeToSvLearnResult(ld_result, lDataPtr);
	result = lcheck_SvLearnDataResult(ld_result, (UB)data);
	DBG_PRINT8("Atual result {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", bank_index, section_index, frame_index, lDataPtr->RegRnum, lDataPtr->RegYnum, lDataPtr->RegID, lDataPtr->RegStatus, lDataPtr->RegImg1[0]);
	return result;
}

static BOOL lcheck_SvLearnDataAtIsRemoved(UW bank_index, UW section_index, UW frame_index)
{
	SvLearnData *lDataPtr;
	UW secAddr, frmAddr;
	BOOL result;
	SvLearnResult ld_result;
	result = FALSE;
	
	lDataPtr = &g_learnData;
	// ld_result = &g_learnDataResult
	lreadFrame(bank_index, section_index, frame_index, lDataPtr);
	lchangeToSvLearnResult(&ld_result, lDataPtr);
	result = lcheck_SvLearnDataResult(&ld_result, 0xFF);
	
	return result;
}

static BOOL lcheck_ctrl_flg(UW bank_index_oldest, UW section_index_oldest, UW erased_bank_index, UW erased_section_index)
{
	UH ctrl_flg_erased, ctrl_flg_oldest;
	ctrl_flg_erased = lread_CtrlFlg(erased_bank_index, erased_section_index);
	ctrl_flg_oldest = lread_CtrlFlg(bank_index_oldest, section_index_oldest);
	DBG_PRINT4("Control flag of B%dS%d: 0x%0.4x (expected 0x%0.4x)", erased_bank_index, erased_section_index, ctrl_flg_erased, LDATA_CTRL_FLG_ERASED);
	DBG_PRINT4("Control flag of B%dS%d: 0x%0.4x (expected 0x%0.4x)", bank_index_oldest, section_index_oldest, ctrl_flg_oldest, LDATA_CTRL_FLG_OLDEST);
	//////////////////////////////////////////
	getOldestSection(&g_bank_oldest_index, &g_section_oldest_index, &g_ctrl_flg_oldest);	
	DBG_PRINT4("Current oldest B%dS%d: 0x%0.4x", g_bank_oldest_index, g_section_oldest_index, g_ctrl_flg_oldest, LDATA_CTRL_FLG_OLDEST); 
	getCurrentCursor(&g_bank_index, &g_section_index, &g_frame_index);
	DBG_PRINT3("Current location B%dS%dF%d", g_bank_index, g_section_index, g_frame_index);
	//////////////////////////////////////////
	if(ctrl_flg_erased == LDATA_CTRL_FLG_ERASED && ctrl_flg_oldest == LDATA_CTRL_FLG_OLDEST)
	{
		return TRUE;
	}
	return FALSE;
}

static BOOL lcheck_notify_data(int notifyCase)
{
	UB expected_data;
	SvLearnData *expected_ldPtr;
	BOOL ret = FALSE;
	
	switch (notifyCase){
	case 1:
	{
		// Check if unique data 1 was moved to 1st frame of section
		DBG_PRINT0("[TC_UNIQUE_ADD_01] Check if 1 unique data was moved to B3S7");

		expected_ldPtr = lcreateLearnData(LDATA_FINGER_GREEN_1_POS);
		ret = lcheck_SvLearnDataAt(3,7,0, expected_ldPtr->RegImg1[0]);
		DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", 3, 7, 0, expected_ldPtr->RegRnum, expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
		// DBG_PRINT1("[TC_UNIQUE_ADD_01] Result: %d", ret);
		
		// if(ret == TRUE)
		{			
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			ret = lcheck_SvLearnDataAt(3,7,1, expected_ldPtr->RegImg1[0]);
			DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", 3, 7, 1, expected_ldPtr->RegRnum, 	expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
			DBG_PRINT1("[TC_UNIQUE_ADD_01] Check new data at B3S7F1: %d", ret);
		}
		break;
	}
	case 2:
	{
		// Check if unique data 1 was moved to 1st frame of section
		DBG_PRINT0("[TC_UNIQUE_ADD_02] Check if 2 unique data was moved to B3S2");
		
		expected_ldPtr = lcreateLearnData(LDATA_FINGER_YELLOW_1_POS);
		ret = lcheck_SvLearnDataAt(3,2,0, expected_ldPtr->RegImg1[0]);
		// DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", 3, 2, 0, expected_ldPtr->RegRnum, expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
		// DBG_PRINT1("[TC_UNIQUE_ADD_02] Result: %d", ret);
		
		// if(ret == TRUE)
		{
			// Check if unique data 2 was moved to 2nd frame of section		
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_YELLOW_2_POS);
			ret = lcheck_SvLearnDataAt(3,2,1, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", 3, 2, 1, expected_ldPtr->RegRnum, expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT1("[TC_UNIQUE_ADD_02] Result: %d", ret);
		}
		
		// if(ret == TRUE)
		{
			// Check if new data was stored at 3rd frame of section		
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			ret = lcheck_SvLearnDataAt(3,2,2, expected_ldPtr->RegImg1[0]);
			DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", 3, 2, 2, expected_ldPtr->RegRnum, 	expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
			DBG_PRINT1("[TC_UNIQUE_ADD_02] Check new data at B3S2F2: %d", ret);
		}
		break;
	}
	case 19:
	{
		// Check if all 19 unique data was moved to current section
		// from Bank3 Section5 to Bank3 Section4
		int i;
		DBG_PRINT0("[TC_UNIQUE_ADD_19] Check if 19 unique data were moved to B3S4");
		for(i = 0; i<19; i++)
		{			
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_ORANGE_1_POS+i);
			ret = lcheck_SvLearnDataAt(3,4,i, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", 3, 4, i, expected_ldPtr->RegRnum, expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT1("[TC_UNIQUE_ADD_19] Result: %d", ret);
		}
			
		// if(ret == TRUE)
		{
			// Check if new data was stored at next Section
			// Bank3 Section5 Frame0					
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			ret = lcheck_SvLearnDataAt(3,5,0, expected_ldPtr->RegImg1[0]);
			DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", 3, 5, 0, expected_ldPtr->RegRnum, 	expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
			DBG_PRINT1("[TC_UNIQUE_ADD_19] Check new data at B3S5F0: %d", ret);
		}
		break;
	}
	default:
	break;
	};
	
	return ret;
}

static BOOL lcheck_RemoveAllData(UW bank_index, UW section_index)
{
	BOOL ret = 0;
	ret |= lcheck_SvLearnDataAtIsRemoved(bank_index, section_index, N_FRM_PER_SECT-1);
	ret |= lcheck_SvLearnDataAtIsRemoved(bank_index, section_index, N_FRM_PER_SECT-2);
	ret |= lcheck_SvLearnDataAtIsRemoved(bank_index, section_index, N_FRM_PER_SECT-3);
	ret |= lcheck_SvLearnDataAtIsRemoved(bank_index, section_index, N_FRM_PER_SECT-4);
	return (ret==0 ? TRUE : FALSE);
}

static BOOL lcheck_SearchLearnImg(int index, UH SearchNum, int expected_val)
{
	int result;
	int fingerIndex;
	UH *SearchLearnDataResultPtr[LDATA_REG_FIGURE_NBR_MAX][3];
	
	for(fingerIndex=0; fingerIndex<LDATA_REG_FIGURE_NBR_MAX; fingerIndex++)
	{
		SearchLearnDataResultPtr[fingerIndex][0] = &g_SearchLearnDataResult[index].LearnInfo[fingerIndex][0];
		SearchLearnDataResultPtr[fingerIndex][1] = &g_SearchLearnDataResult[index].LearnInfo[fingerIndex][1];
		SearchLearnDataResultPtr[fingerIndex][2] = &g_SearchLearnDataResult[index].LearnInfo[fingerIndex][2];
	}
	
	result = SearchLearnImg(SearchNum, SearchLearnDataResultPtr);
	g_SearchLearnDataResult[index].result = result;

	return (result == expected_val);
}

static void init_SearchLearnImgResult(void)
{
	int testCase;
	int fingerIndex;
	// memset((void*)g_SearchLearnDataResult, 0xFF, sizeof(g_SearchLearnDataResult));
	
	for(testCase = TC_SEARCH_01; testCase < TC_SEARCH_TOTAL; testCase++)
	{	
		g_SearchLearnDataResult[testCase].result = -1;
		for(fingerIndex=0; fingerIndex<LDATA_REG_FIGURE_NBR_MAX; fingerIndex++)
		{
			g_SearchLearnDataResult[testCase].LearnInfo[fingerIndex][0] = 0;
			g_SearchLearnDataResult[testCase].LearnInfo[fingerIndex][1] = 0;
			g_SearchLearnDataResult[testCase].LearnInfo[fingerIndex][2] = 0;
		}
	}
}

static int TC_ADD_FUNC(void)
{
    int result;
    SvLearnData *learnDataPtr;
	int ret;
	UB expected_data;
			
    while(TRUE)
    {
#ifdef FAST_TEST
		fast_test(&g_numberOfLearnData);
#endif
		learnDataPtr = lcreateLearnData(g_numberOfLearnData+1);
		result = AddSvLearnImg(learnDataPtr);
		if(result != 0) {
			return -1;	// Test case fail
		}
	
		g_numberOfLearnData++;    
		if(g_numberOfLearnData == B3S0F0)
		{
			/* Store 1st data
			 * Bank=3 Section=0 Frame=0 [0x06000000]
			 * Bank=3 [0x06000000], Section=0 [0x06000000], Frame=0 [0x06000000] */
			DBG_PRINT0("[TC_ADD_01] Store 1st data at B3S0F0");
			expected_data = lFingerDataImg(g_numberOfLearnData);	// LDATA_FINGER_RED_1_INDEX
			ret = lcheck_SvLearnDataAt(3, 0, 0, expected_data);
			DBG_PRINT2("[TC_ADD_01] Check data: %d (expected 0x%0.2x)", ret, expected_data);

		}else if(g_numberOfLearnData == B3S0F1)
		{
			/* Store 2nd data
			 * Bank=3 Section=0 Frame=1 [0x06000000]
			 * Bank=3 [0x06000000], Section=0 [0x06000000], Frame=1 [0x06001ad6] */
			DBG_PRINT0("[TC_ADD_02] Store 2nd data at B3S0F1");
			expected_data = lFingerDataImg(g_numberOfLearnData);	// LDATA_FINGER_RED_1_INDEX
			ret = lcheck_SvLearnDataAt(3, 0, 1, expected_data);	
			DBG_PRINT2("[TC_ADD_02] Check data: %d (expected 0x%0.2x)", ret, expected_data);
					
			DBG_PRINT0("[TC_ADD_FUNC] Done");
			return 0;	// testCase = LDATA_RING_CASE;
		}

	} // end while

	return -1; // Test case fail
}

static int TC_RING_FUNC(void)
{
    int result;
    SvLearnData *learnDataPtr;
	SvLearnData *expected_ldPtr;
	int ret;
	UB expected_data;
    
    while(TRUE)
    {
#ifdef FAST_TEST
		fast_test(&g_numberOfLearnData);
#endif
		/* Check Ring Buffer : Check control area of Section0 */
		learnDataPtr = lcreateLearnData(g_numberOfLearnData+1);
		result = AddSvLearnImg(learnDataPtr);
		if(result != 0) {
			return -1;	// Test case fail
		}
		
		g_numberOfLearnData++;
		if(g_numberOfLearnData == B3S0F18)
		{
			/* Store until full data in Section0 (19th)
			 * Bank=3 Section=0 Frame=18
			 * Bank=3 [0x06000000], Section=0 [0x06000000], Frame=18 [0x0601e30c] */			
			DBG_PRINT0("[TC_RING_01] Store full Section0 (19th) at B3S0F18");

			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,0, 3,1);	// Bank3 Section0 ctrl_flg : 0x0001
			DBG_PRINT1("[TC_RING_01] Check ctrl flag: %d", ret);
			
			// Check learn data
			expected_data = lFingerDataImg(g_numberOfLearnData);	// LDATA_FINGER_RED_1_INDEX
			ret = lcheck_SvLearnDataAt(3, 0, 18, expected_data);
			DBG_PRINT2("[TC_RING_01] Check data: %d (expected 0x%0.2x)", ret, expected_data);
		}
		else if(g_numberOfLearnData == B3S1F0)
		{
			/* Store until first data in Section1 (20th)
			 * g_numberOfLearnData=20 Bank=3 Section=1 Frame=0 [0x06020000] 
			 * Bank=3 [0x06000000], Section=1 [0x06020000], Frame=0 [0x06020000] 
			 * Remove all data in Bank3 Section1
			 * Put data at first area in Bank3 Section1 */
			DBG_PRINT0("[TC_RING_02] Store 1st data Section1 (20th) at B3S1F0");
					
			// Check remove all data Bank3 Section1
			ret = lcheck_RemoveAllData(3,1);
			DBG_PRINT1("[TC_RING_02] Check clear section: %d", ret);
			
			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,0, 3,1);	// Bank3 Section0 ctrl_flg : 0x0001
			DBG_PRINT1("[TC_RING_02] Check ctrl flag: %d", ret);
			
			// Check learn data
			expected_data = lFingerDataImg(g_numberOfLearnData);	// LDATA_FINGER_RED_1_INDEX
			ret = lcheck_SvLearnDataAt(3, 1, 0, expected_data);
			DBG_PRINT2("[TC_RING_02] Check data: %d (expected 0x%0.2x)", ret, expected_data);
		}
		else if(g_numberOfLearnData == B3S255F18)
		{
			/* Store until full all Sections in Bank3 (full Bank3)
			 * Bank=3 Section=255 Frame=18
			 * Bank=3 [0x06000000], Section=255 [0x07fe0000], Frame=18 [0x07ffe30c] */
			DBG_PRINT0("[TC_RING_03] Store full Bank3 at B3S255F18");
						
			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,0, 3,255);	// Bank3 Section0 ctrl_flg : 0x0001
			DBG_PRINT1("[TC_RING_03] Check ctrl flag: %d", ret);
			
			// Check learn data
			expected_data = lFingerDataImg(g_numberOfLearnData);	// LDATA_FINGER_RED_1_INDEX
			ret = lcheck_SvLearnDataAt(3, 255, 18, expected_data);
			DBG_PRINT2("[TC_RING_03] Check data: %d (expected 0x%0.2x)", ret, expected_data);
		}
		else if(g_numberOfLearnData == B7S255F18)
		{
			/* Store until full all Sections in Bank3 (full Bank7)
			 * Bank=7 Section=255 Frame=18
			 * Bank=7 [0x0e000000], Section=255 [0x0ffe0000], Frame=18 [0x0fffe30c] */
			DBG_PRINT0("[TC_RING_04] Store full Bank7 at B7S255F18");
						
			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,0, 7,255);	// Bank3 Section0 ctrl_flg : 0x0001
			DBG_PRINT1("[TC_RING_04] Check ctrl flag: %d", ret);
			
			// Check learn data
			expected_data = lFingerDataImg(g_numberOfLearnData);	// LDATA_FINGER_RED_1_INDEX
			ret = lcheck_SvLearnDataAt(7, 255, 18, expected_data);
			DBG_PRINT2("[TC_RING_04] Check data: %d (expected 0x%0.2x)", ret, expected_data);
		}
		else if(g_numberOfLearnData == (FULL_ALL_BANK+B3S0F0))
		{
			/* Store until full all Sections in Bank3 (full Bank7 +1frame )
			 * Bank=3 Section=0 Frame=0
			 * Bank=7 [0x06000000], Section=0 [0x06000000], Frame=0 [0x06000000] */
			DBG_PRINT0("[TC_RING_05] Store full Bank7 +1 frame at (FULL_ALL_BANK+B3S0F0)");

			// Check remove all data Bank3 Section0
			ret = lcheck_RemoveAllData(3,0);
			DBG_PRINT1("[TC_RING_05] Check clear section: %d", ret);
						
			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,1, 3,0);	// Bank3 Section1 ctrl_flg : 0x0001
			DBG_PRINT1("[TC_RING_05] Check ctrl flag: %d", ret);
			// Check learn data
			expected_data = lFingerDataImg(g_numberOfLearnData);	// LDATA_FINGER_RED_1_INDEX
			ret = lcheck_SvLearnDataAt(3, 0, 0, expected_data);
			DBG_PRINT2("[TC_RING_05] Check data: %d (expected 0x%0.2x)", ret, expected_data);
			
			DBG_PRINT0("[TC_RING_FUNC] Done");
			return 0;
		}
			
		////////////////////////////////////////////////////////////
		/* add Unique data */
		if(g_numberOfLearnData == B3S3F1 || g_numberOfLearnData == B3S3F3)
		{
			/* Store 2 unique fingers in 1 Section, prepare for LDATA_NOTIFY_CASE
			 * g_numberOfLearnData=59,61 Bank=3 Section=3 Frame=1,3 (2nd, 4th)
			 * Bank=3 [0x06000000], Section=3 [0x06060000], Frame=1 [0x06061ad6], Frame=3 [0x06065082] */
			DBG_PRINT0("[TC_UNIQUE_PREPARE_02] Store 2 unique data in 1 Section at B3S3F1 & B3S3F3");
			lcalc_LDLocation(g_numberOfLearnData, &g_bank_index, &g_section_index, &g_frame_index);
			expected_ldPtr = lcreateLearnData(g_numberOfLearnData); // YELLOW 1 2
			ret = lcheck_SvLearnDataAt(g_bank_index,g_section_index,g_frame_index, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", g_bank_index, g_section_index, g_frame_index, expected_ldPtr->RegRnum, expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT1("[TC_UNIQUE_PREPARE_02] Result: %d", ret);
		}
		else if(B3S5F0 <= g_numberOfLearnData && g_numberOfLearnData <= B3S5F18)
		{
			/* Store 19 (full) unique fingers in 1 Section, prepare for LDATA_NOTIFY_CASE
			 * Bank=3 Section=5 Frame=0-18
			 * Bank=3 [0x06000000], Section=5 [0x060a0000], Frame=0 [0x060a0000] - Frame=18 [0x060be30c] */
			DBG_PRINT0("[TC_UNIQUE_PREPARE_19] Store 19 unique data in 1 Section from B3S5F0 to B3S5F18");
			lcalc_LDLocation(g_numberOfLearnData, &g_bank_index, &g_section_index, &g_frame_index);
			expected_ldPtr = lcreateLearnData(g_numberOfLearnData); // ORANGE 1->19
			ret = lcheck_SvLearnDataAt(g_bank_index,g_section_index,g_frame_index, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", g_bank_index, g_section_index, g_frame_index, expected_ldPtr->RegRnum, expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT1("[TC_UNIQUE_PREPARE_19] Result: %d", ret);
		}
		else if(g_numberOfLearnData == B3S8F1)
		{
			/* Store 1 unique fingers in 1 Section, prepare for LDATA_NOTIFY_CASE
			 * Bank=3 Section=8 Frame=2 */
			DBG_PRINT0("[TC_UNIQUE_PREPARE_01] Store 1 unique data in 1 Section at B3S8F1");
			lcalc_LDLocation(g_numberOfLearnData, &g_bank_index, &g_section_index, &g_frame_index);
			expected_ldPtr = lcreateLearnData(g_numberOfLearnData); // GREEN 1
			ret = lcheck_SvLearnDataAt(g_bank_index,g_section_index,g_frame_index, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT8("Expectation: {B%dS%dF%d, Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", g_bank_index, g_section_index, g_frame_index, expected_ldPtr->RegRnum, expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
			// DBG_PRINT1("[TC_UNIQUE_PREPARE_01] Result: %d", ret);
		}
		//////////////////////////////////
	}
	return -1;
}

static int TC_NOTIFY_FUNC(void)
{
    int result;
    SvLearnData *learnDataPtr;
    int index, ret;
	UB expected_data;
			
    while(TRUE)
    {
#ifdef FAST_TEST
		fast_test(&g_numberOfLearnData);
#endif
		/* Check Ring Buffer : Check control area of Section0 */
		learnDataPtr = lcreateLearnData(g_numberOfLearnData+1);
		result = AddSvLearnImg(learnDataPtr);
		if(result != 0) {
			return -1;	// Test case fail
		}
		
		///////////////////////////
		getCurrentCursor(&g_bank_index, &g_section_index, &g_frame_index);
		///////////////////////////
		g_numberOfLearnData++;
		if(g_bank_index == 3 && g_section_index == 2 && g_frame_index == 3)
		{
			/* Check 2 only one data in flash memory Section3 after calling AddSvLearnImg */

			/* Check ctrl_flg */
			ret = lcheck_ctrl_flg(3,3, 3,2);	// Bank3 Section3 ctrl_flg : 0x0001
			DBG_PRINT1("[TC_UNIQUE_ADD_02] Check ctrl flag: %d", ret);
			/* Check remove all data Bank3 Section2 */
			ret = lcheck_RemoveAllData(3,2);
			DBG_PRINT1("[TC_UNIQUE_ADD_02] Check clear section: %d", ret);
			/* Check data */
			ret = lcheck_notify_data(2);
			DBG_PRINT1("[TC_UNIQUE_ADD_02] Check notify data: %d", ret);
		}
		else if(g_bank_index == 3 && g_section_index == 5 && g_frame_index == 1)
		{
			/* Check 19 only one data at (FULL_ALL_BANK+B3S4F0)*/
			/* Get result */
			/* Check ctrl_flg */
			ret = lcheck_ctrl_flg(3,6, 3,5);	// Bank3 Section6 ctrl_flg : 0x0001
			DBG_PRINT1("[TC_UNIQUE_ADD_19] Check ctrl flag: %d", ret);
			/* Check remove all data Bank3 Section5 */
			ret = lcheck_RemoveAllData(3,5);
			DBG_PRINT1("[TC_UNIQUE_ADD_19] Check clear section: %d", ret);
			/* Check data */
			ret = lcheck_notify_data(19);
			DBG_PRINT1("[TC_UNIQUE_ADD_19] Check notify data: %d", ret);
		}
		else if(g_bank_index == 3 && g_section_index == 7 && g_frame_index == 2)
		{
			/* Check 1 only one data in flash memory Section0 after calling AddSvLearnImg */
			/* Get result */
			/* Check ctrl_flg */
			ret = lcheck_ctrl_flg(3,8, 3,7);	// Bank3 Section8 ctrl_flg : 0x0001
			DBG_PRINT1("[TC_UNIQUE_ADD_01] Check ctrl flag: %d", ret);
			/* Check remove all data Bank3 Section7 */
			ret = lcheck_RemoveAllData(3,7);
			DBG_PRINT1("[TC_UNIQUE_ADD_01] Check clear section: %d", ret);
			/* Check data */
			ret = lcheck_notify_data(1);
			DBG_PRINT1("[TC_UNIQUE_ADD_01] Check notify data: %d", ret);

			DBG_PRINT1("[TC_NOTIFY_FUNC] Done, num=%d", g_numberOfLearnData);
			return 0;
		}
	
	}
	return -1;
}

static int TC_MAPPING_FUNC(void)
{
    int result;
    SvLearnData *learnDataPtr;
    int index, ret;
	UB expected_data;

	UB BankNum, SectionNum, FrameNum, Num;

#ifdef TEST_API
	learnDataPtr = &g_LearnDataArray[LDATA_FINGER_BLUE_1_INDEX];
//    int i;
	DBG_PRINT0("[TC_MAPPING_FUNC] InfoLearningBankTable");
	learnDataPtr->RegRnum = 21;
	learnDataPtr->RegYnum = 1;
	result = AddSvLearnImg(learnDataPtr);
	g_numberOfLearnData++;
	learnDataPtr->RegRnum = 22;
	learnDataPtr->RegYnum = 2;
	result = AddSvLearnImg(learnDataPtr);
	g_numberOfLearnData++;
	learnDataPtr->RegRnum = 23;
	learnDataPtr->RegYnum = 3;
	result = AddSvLearnImg(learnDataPtr);
	g_numberOfLearnData++;
	learnDataPtr->RegRnum = 24;
	learnDataPtr->RegYnum = 4;
	result = AddSvLearnImg(learnDataPtr);
	g_numberOfLearnData++;

	get_InfoLearnInBankM(21, 1, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("[%d][%d] {%d, %d, %d, %d}", 21, 1, BankNum, SectionNum, FrameNum, Num);
	get_InfoLearnInBankM(22, 2, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("[%d][%d] {%d, %d, %d, %d}", 22, 1, BankNum, SectionNum, FrameNum, Num);
	get_InfoLearnInBankM(23, 3, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("[%d][%d] {%d, %d, %d, %d}", 23, 1, BankNum, SectionNum, FrameNum, Num);
	get_InfoLearnInBankM(24, 4, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("[%d][%d] {%d, %d, %d, %d}", 24, 1, BankNum, SectionNum, FrameNum, Num);
	
	DBG_PRINT1("[TC_MAPPING_FUNC] Done, num=%d", g_numberOfLearnData);
#else
	DBG_PRINT1("[TC_MAPPING_FUNC] Do nothing, num=%d", g_numberOfLearnData);
#endif
	return 0;
	
}
static int TC_SEARCH_FUNC(void)
{
	int ret = 0;
	ret = lcheck_SearchLearnImg(TC_SEARCH_01, 0, 0); //// search fingers of room 0 
	DBG_PRINT2("[TC_SEARCH_01] Check search: %d (expected %d)", ret, 0);
	ret = lcheck_SearchLearnImg(TC_SEARCH_02, LDATA_REG_NBR_MAX-1, 0); //// search fingers of room 479 
	DBG_PRINT2("[TC_SEARCH_02] Check search: %d (expected %d)", ret, 0);
	ret = lcheck_SearchLearnImg(TC_SEARCH_03, LDATA_REG_NBR_MAX, 1); //// search fingers of room 480
	DBG_PRINT2("[TC_SEARCH_03] Check search: %d (expected %d)", ret, 1);
	DBG_PRINT0("[TC_SEARCH_03] Done.");	
	return 0; // Success
}
/******************************************************************************/
/*************************** Functions Definition *****************************/
/******************************************************************************/
/******************************************************************************/
/* Function Name: TaskLearnDataInit                                           */
/* Description  : This function will initail a thread for testing and create  */
/*                learn data for three finger number.                         */
/* Parameter    : None                                                        */
/* Return Value : BOOL - return the TRUE value when initials successful, else */
/*                       to return FALSE value.                               */
/* Remarks      : None                                                        */
/******************************************************************************/
void TaskLearnDataInit(void)
{
    UH index;
    UH i;
	/* red : common data */
	/* blue : common data */
    /* green : 1 only one data */ 
    /* yellow : 2 only one data */
    /* orange : 19 only one data */
    for(i=0; i<LDATA_FINGER_INDEX_TOTAL; i++)
    {
        memset(&g_LearnDataArray[i], 0, sizeof(SvLearnData));
        memset(&g_LearnDataArray[i].RegImg1[0], g_FingerDataImgArray[i], LDATA_NORM_IMAGE_SIZE);
        memset(&g_LearnDataArray[i].RegImg2[0], g_FingerDataImgArray[i], LDATA_NORM_IMAGE_SIZE);
        memset(&g_LearnDataArray[i].MiniImg[0][0], g_FingerDataImgArray[i], LDATA_MINI_IMAGE_SIZE);
        memset(&g_LearnDataArray[i].MiniImg[1][0], g_FingerDataImgArray[i], LDATA_MINI_IMAGE_SIZE);
    }

	// init_InitBankAreaResult();
	// init_AddSvLearnImgResult();
	init_SearchLearnImgResult();
	// init_NotifyDataResult();
#ifdef DEBUG
	dbg_init();
#endif

	// Initialize global variables
	g_numberOfLearnData = 0;	// global val
    
}


#ifdef FAST_TEST
static struct fast_test_ST
{
	const UW curr;
	const UW next;
	const UW value3;
} ft[] = {
	/* curr					next 							value3 [if() in main]*/
	// {	XXXXXXXXXX,				XXXXXXXXXX,						B3S0F0		},			// Store 1st data
	// {	XXXXXXXXXX,				XXXXXXXXXX,						B3S0F1		},			// Store 2nd data
	{	B3S0F2,						B3S0F18-1,						B3S0F18		},			// Store full Section0
	// {	XXXXXXXXXX,				XXXXXXXXXX,						B3S1F0		},			// Store 1st data Section1  , RingBuffer Bank3 Section0:0x0001 Section1:0xFFFF
	{	B3S1F0,				B3S3F1-1,						B3S3F1		},					// Store 2 unique data in 1 Section Bank=3 Section=3 Frame=1,3 (2nd, 4th)
	// {	XXXXXXXXXX,				XXXXXXXXXX,						B3S3F3		},			// Store 2 unique data in 1 Section
	{	B3S3F3,				B3S5F0-1,						B3S5F0		},					// Store 19 unique data in 1 Section
	// {	XXXXXXXXXX,				XXXXXXXXXX,						B3S5F18		},		    // Store 19 unique data in 1 Section
	{	B3S6F0,				B3S8F1-1,						B3S8F1		},					// Store 1 unique data in 1 Section , Bank=3 Section=7 Frame=2
	{	B3S8F1,				B3S255F18-1,					B3S255F18		},		// Store full Bank3
	{	B4S0F0,				B7S255F18-1,					B7S255F18		},		// Store full Bank7 , RingBuffer
	// {	XXXXXXXXXX,				XXXXXXXXXX,		(FULL_ALL_BANK+B3S0F0)		},	// RingBuffer: 2. ctrl area of Bank3 Section0:0xFFFF Section1:0x0001
	{	(FULL_ALL_BANK+B3S0F0),	(FULL_ALL_BANK+B3S2F0)-1,	(FULL_ALL_BANK+B3S2F0)		},	// Check 2 unique data in 1 Section
	{	(FULL_ALL_BANK+B3S2F0),	(FULL_ALL_BANK+B3S4F0)-1,	(FULL_ALL_BANK+B3S4F0)		},	// Check 19 unique data in 1 Section
	{	(FULL_ALL_BANK+B3S4F0),	(FULL_ALL_BANK+B3S7F0)-1,	(FULL_ALL_BANK+B3S7F0)		},	// Check 1 unique data in 1 Section N_BANK_INIT
	// {XXXXXXXXXX,				XXXXXXXXXX,				XXXXXXXXXX},			// 	
};

static void fast_test(UW *num)
{
	UW curr, next;
	int i;
	curr = *num;
	
	for(i=0; i < sizeof(ft)/sizeof(ft[0]); i++)
	{
		if(curr == ft[i].curr)
		{
			next = ft[i].next;
			*num = next;
			break;
		}
	}	
}
#endif

/******************************************************************************/
/* Function Name: TaskLearnData                                               */
/* Description  : This thread will test the learn data management with the    */
/*                simulator data.                                             */
/* Parameter    : None                                                        */
/* Return Value : None                                                        */
/* Remarks      : None                                                        */
/******************************************************************************/
TASK TaskLearnData(void)
{
    int ret;
    ldataTestCaseType testCase;
    SvLearnData *learnDataPtr;

	testCase = LDATA_ADD_CASE;      /* get character */
    
    TaskLearnDataInit();
	DBG_PRINT0("Start TaskLearnData");

	// ret = InitBankArea(BANK0);
    ret = InitBankArea(BANK_SELECTED);
	DBG_PRINT2("Check InitBankArea: %d (expected %d)", ret, 0);
	
    if(ret!=0)
    {
        goto END_TASK;
    }
	
    while(TRUE)
    {
        switch(testCase) {
		case LDATA_ADD_CASE:
			ret = TC_ADD_FUNC();
			if(ret == 0)
			{
				testCase = LDATA_RING_CASE;
				DBG_PRINT1("Switch to LDATA_RING_CASE, num=%d", g_numberOfLearnData);
			}else goto END_TASK;    /* show test case fail */
		break;
		case LDATA_RING_CASE:
			ret = TC_RING_FUNC();
			if(ret == 0)
			{
				testCase = LDATA_NOTIFY_CASE;
				DBG_PRINT1("Switch to LDATA_NOTIFY_CASE, num=%d", g_numberOfLearnData);
			}else goto END_TASK;    /* show test case fail */
		break;
		case LDATA_NOTIFY_CASE:
			ret = TC_NOTIFY_FUNC();
			if(ret == 0)
			{
				testCase = LDATA_MAPPING_CASE;
				DBG_PRINT1("Switch to LDATA_MAPPING_CASE, num=%d", g_numberOfLearnData);
			}else goto END_TASK;    /* show test case fail */
		break;
		case LDATA_MAPPING_CASE:
			ret = TC_MAPPING_FUNC();
			if(ret == 0)
			{
				testCase = LDATA_SEARCH_CASE;
				DBG_PRINT1("Switch to LDATA_SEARCH_CASE, num=%d", g_numberOfLearnData);
			}else goto END_TASK;    /* show test case fail  */
		break;
		case LDATA_SEARCH_CASE:
			ret = TC_SEARCH_FUNC();
			if(ret == 0)
			{
				testCase = LDATA_UNKNOW_CASE;
				DBG_PRINT1("Switch to LDATA_UNKNOW_CASE, num=%d", g_numberOfLearnData);
			}
		break;
		default:
			goto END_TASK;
		break;
        }// END SWITCH
    }// END WHILE

END_TASK:
    while(TRUE)
    {
        dly_tsk( 100/MSEC );
    }
}

/*************************** The End ******************************************/
