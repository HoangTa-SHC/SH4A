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
static BOOL lreadFrame(UW frmAddr, SvLearnData *learnDataPtr);
static BOOL lreadFrameByIndex(UW bank_index, UW section_index, UW frame_index, SvLearnData *learnDataPtr);
static UW lcalc_CtrlFlgAddr(UW bank_index, UW section_index);
static volatile UH lread_CtrlFlg(UW bank_index, UW section_index);
static void lchangeToSvLearnResult(SvLearnResult *ld, SvLearnData *learnDataPtr);
static void printCurrentMappingInformation(void);
static BOOL lcheck_BankIndex(UW bank_index);
static BOOL lcheck_SecIndex(UW section_index);
static BOOL lcheck_FrmIndex(UW frame_index);
static BOOL lcheck_RegStatus(UH checked_val, UH expected_val);
static BOOL lcheck_RegRnum(UH checked_val);
static BOOL lcheck_RegYnum(UH checked_val);
static BOOL lcheck_RegImg1(UH checked_val, UH expected_val);
static BOOL lcheck_SvLearnDataResult(SvLearnResult* ld ,UB data);
static BOOL lcheck_ctrl_flg(UW bank_index_oldest, UW section_index_oldest, UW erased_bank_index, UW erased_section_index);
static BOOL lcheck_ClearSection(UW bank_index, UW section_index);
static BOOL lcheck_notify_data(int notifyCase);
static void lcheck_PrepareUniqueData(void);
#ifdef FAST_TEST
static void fast_test(UW *num);
#endif
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
/******************************************************************************/
/*************************** Local Functions **********************************/
/******************************************************************************/
/******************************************************************************/
/*************************** Debug  *******************************************/
/******************************************************************************/
#define DEBUG
#ifdef DEBUG
#define DBG_LINE_TOTAL 180
static char log_table[DBG_LINE_TOTAL][100];
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

    return learnDataPtr;
}

static Location lget_Location(int index)
{
	Location loc;
	switch (index){
	case FIRST_LOCATION                : {loc.bankIndex = 3, loc.secIndex = 0, loc.frmIndex = 0; break;}
	case SECOND_LOCATION               : {loc.bankIndex = 3, loc.secIndex = 0, loc.frmIndex = 1; break;}
	case FULL_FIRST_SECTION_LOCATION   : {loc.bankIndex = 3, loc.secIndex = 0, loc.frmIndex = LDATA_FRAME_NUM_IN_SECTOR-1; break;}
	case START_SECOND_SECTION_LOCATION : {loc.bankIndex = 3, loc.secIndex = 1, loc.frmIndex = 0; break;}
	case FULL_FIRST_BANK_LOCATION      : {loc.bankIndex = 3, loc.secIndex = MI_NUM_SEC_IN_BANK-1, loc.frmIndex = LDATA_FRAME_NUM_IN_SECTOR-1; break;}
	case FULL_ALL_BANK_LOCATION        : {loc.bankIndex = BANK_NUM_MAX, loc.secIndex = MI_NUM_SEC_IN_BANK-1, loc.frmIndex = LDATA_FRAME_NUM_IN_SECTOR-1; break;}
	case RESTART_FIRST_BANK_LOCATION   : {loc.bankIndex = 3, loc.secIndex = 0, loc.frmIndex = 0; break;}
	case YELLOW_1_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 3, loc.frmIndex = 1; break;}
	case YELLOW_2_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 3, loc.frmIndex = 3; break;}
	case ORANGE_1_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 0; break;}
	case ORANGE_2_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 1; break;}
	case ORANGE_3_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 2; break;}
	case ORANGE_4_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 3; break;}
	case ORANGE_5_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 4; break;}
	case ORANGE_6_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 5; break;}
	case ORANGE_7_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 6; break;}
	case ORANGE_8_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 7; break;}
	case ORANGE_9_PREPARED_LOCATION  : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 8; break;}
	case ORANGE_10_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 9; break;}
	case ORANGE_11_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 10; break;}
	case ORANGE_12_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 11; break;}
	case ORANGE_13_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 12; break;}
	case ORANGE_14_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 13; break;}
	case ORANGE_15_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 14; break;}
	case ORANGE_16_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 15; break;}
	case ORANGE_17_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 16; break;}
	case ORANGE_18_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 17; break;}
	case ORANGE_19_PREPARED_LOCATION : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 18; break;}
	case GREEN_1_PREPARED_LOCATION   : {loc.bankIndex = 3, loc.secIndex = 8, loc.frmIndex = 1; break;}
	case YELLOW_1_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 2, loc.frmIndex = 0; break;}
	case YELLOW_2_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 2, loc.frmIndex = 1; break;}
	case RED_NEW_2_LOCATION          : {loc.bankIndex = 3, loc.secIndex = 2, loc.frmIndex = 2; break;}
	case ORANGE_1_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 0; break;}
	case ORANGE_2_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 1; break;}
	case ORANGE_3_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 2; break;}
	case ORANGE_4_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 3; break;}
	case ORANGE_5_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 4; break;}
	case ORANGE_6_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 5; break;}
	case ORANGE_7_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 6; break;}
	case ORANGE_8_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 7; break;}
	case ORANGE_9_MOVED_LOCATION     : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 8; break;}
	case ORANGE_10_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 9; break;}
	case ORANGE_11_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 10; break;}
	case ORANGE_12_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 11; break;}
	case ORANGE_13_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 12; break;}
	case ORANGE_14_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 13; break;}
	case ORANGE_15_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 14; break;}
	case ORANGE_16_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 15; break;}
	case ORANGE_17_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 16; break;}
	case ORANGE_18_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 17; break;}
	case ORANGE_19_MOVED_LOCATION    : {loc.bankIndex = 3, loc.secIndex = 4, loc.frmIndex = 18; break;}
	case RED_NEW_19_LOCATION         : {loc.bankIndex = 3, loc.secIndex = 5, loc.frmIndex = 0; break;}
	case GREEN_1_MOVED_LOCATION      : {loc.bankIndex = 3, loc.secIndex = 7, loc.frmIndex = 0; break;}
	case RED_NEW_1_LOCATION          : {loc.bankIndex = 3, loc.secIndex = 7, loc.frmIndex = 1; break;}
	
	default                         :  {loc.bankIndex = 0, loc.secIndex = 0, loc.frmIndex = 0; break;}
	};
	
	return loc;
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
	if(ret == FALSE)
		DBG_PRINT3("Invalid Bank %d [%d:%d]", bank_index, BANK_NUM_MIN, BANK_NUM_MAX);
	
	if(ret == TRUE)
	{
		ret = lcheck_SecIndex(section_index);
		if(ret == FALSE)
			DBG_PRINT3("Invalid Section %d [%d:%d]", section_index, 0, MI_NUM_SEC_IN_BANK-1);
	}		

	if(ret == TRUE)
	{
		ret = lcheck_FrmIndex(frame_index);
		if(ret == FALSE)
			DBG_PRINT3("Invalid Frame %d [%d:%d]", frame_index, 0, LDATA_FRAME_NUM_IN_SECTOR-1);
	}		

	if(ret == TRUE)
	{
		bankAddr = MI_FLASH_BASIC_ADDR + (bank_index*MI_BANK_SIZE);
		secAddr = bankAddr + section_index*MI_SECTOR_SIZE;
		frmAddr = secAddr + frame_index*sizeof(SvLearnData);
	}

	return frmAddr;	
}

static BOOL lreadFrame(UW frmAddr, SvLearnData *learnDataPtr)
{
	ER errCode;

	errCode = FlRead(frmAddr, (UH*)learnDataPtr, sizeof(SvLearnData)/sizeof(UH) );
	return (errCode == E_OK);
}

static BOOL lreadFrameByIndex(UW bank_index, UW section_index, UW frame_index, SvLearnData *learnDataPtr)
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
	if(ret == FALSE)
		DBG_PRINT3("Invalid Bank %d [%d:%d]", bank_index, BANK_NUM_MIN, BANK_NUM_MAX);
	
	if(ret == TRUE)
	{
		ret = lcheck_SecIndex(section_index);
		if(ret == FALSE)
			DBG_PRINT3("Invalid Section %d [%d:%d]", section_index, 0, MI_NUM_SEC_IN_BANK-1);
	}

	if(ret == TRUE)
	{
		bankAddr = MI_FLASH_BASIC_ADDR + (bank_index*MI_BANK_SIZE);
		secAddr = bankAddr + section_index*MI_SECTOR_SIZE;
		ctrl_addr = secAddr + CTRL_FLAG_OFFSET;
	}

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

static void lchangeToSvLearnResult(SvLearnResult* ld, SvLearnData* learnDataPtr)
{
	ld->RegStatus = learnDataPtr->RegStatus;
	ld->RegRnum = learnDataPtr->RegRnum;
	ld->RegYnum = learnDataPtr->RegYnum;
	ld->RegID   = learnDataPtr->RegID;
	ld->RegImg1 = learnDataPtr->RegImg1[0]; // 1st byte
}

static void printCurrentMappingInformation(void)
{
	UW BankNum, SectionNum, FrameNum, Num;
	UH id;
	UH rNum, yNum;
	int i;

	DBG_PRINT0("Print current Mapping Information (InfoLearningBankTable[][]):");
/* 	// RED room
	get_InfoLearnInBankM(RED_ROOM, RED_1_YNUM, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("RED_1 [%d][%d]: {B%dS%dF%d, Num=%d}", RED_ROOM, RED_1_YNUM, BankNum, SectionNum, FrameNum, Num);

	// GREEN room
	get_InfoLearnInBankM(GREEN_ROOM, GREEN_1_YNUM, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("GREEN_1 [%d][%d]: {B%dS%dF%d, Num=%d}", GREEN_ROOM, GREEN_1_YNUM, BankNum, SectionNum, FrameNum, Num);
	
	// YELLOW room
	get_InfoLearnInBankM(YELLOW_ROOM, YELLOW_1_YNUM, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("YELLOW_1 [%d][%d]: {B%dS%dF%d, Num=%d}", YELLOW_ROOM, YELLOW_1_YNUM, BankNum, SectionNum, FrameNum, Num);
	get_InfoLearnInBankM(YELLOW_ROOM, YELLOW_2_YNUM, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("YELLOW_2 [%d][%d]: {B%dS%dF%d, Num=%d}", YELLOW_ROOM, YELLOW_2_YNUM, BankNum, SectionNum, FrameNum, Num);

	// ORANGE room
	for(i = 0; i<19; i++)
	{
		get_InfoLearnInBankM(ORANGE_ROOM, ORANGE_1_YNUM+i, &BankNum, &SectionNum, &FrameNum, &Num);
		DBG_PRINT7("ORANGE_%d [%d][%d]: {B%dS%dF%d, Num=%d}", i+1, ORANGE_ROOM, ORANGE_1_YNUM+i, BankNum, SectionNum, FrameNum, Num);
	}

	// BLUE room
	get_InfoLearnInBankM(BLUE_ROOM_1, BLUE_1_YNUM, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d}", BLUE_ROOM_1, BLUE_1_YNUM, BankNum, SectionNum, FrameNum, Num);
	get_InfoLearnInBankM(BLUE_ROOM_2, BLUE_2_YNUM, &BankNum, &SectionNum, &FrameNum, &Num);
	DBG_PRINT6("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d}", BLUE_ROOM_2, BLUE_2_YNUM, BankNum, SectionNum, FrameNum, Num); */
	
	for(rNum = 0; rNum < LDATA_REG_NBR_MAX; rNum++)
	{
        for(yNum = 0; yNum < LDATA_REG_FIGURE_NBR_MAX; yNum++)
		{
			get_InfoLearnInBankM(rNum, yNum, &BankNum, &SectionNum, &FrameNum, &Num, &id);
			if(Num == 0)
				continue;
			else
				DBG_PRINT7("[%d][%d]: {B%dS%dF%d, Num=%d, ID=%d}", rNum, yNum, BankNum, SectionNum, FrameNum, Num, id);
        }
    }
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

static BOOL lcheck_RegStatus(UH checked_val, UH expected_val)
{
    // LDATA_NOT_YET_STS   = 0xFFFF,  //// Not yet
    // LDATA_DUR_REG_STS   = 0xFFFE,  //// During registration
    // LDATA_REGISTERD_STS = 0xFFFC, //// Registered. It is the latest data
    // LDATA_NOT_LATEST_STS= 0xFFF8, //// Not latest data. Old register
    // LDATA_UNKNOW_STS
	return checked_val == expected_val;
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
	return checked_val == expected_val;
}

static BOOL lcheck_SvLearnDataResult(SvLearnResult* ld ,UB data)
{	
	BOOL result;
	result = TRUE;
	
	if(lcheck_RegStatus(ld->RegStatus, LDATA_REGISTERD_STS) == FALSE)
	{
		result = FALSE;
		DBG_PRINT2("Check RegStatus failed: 0x%0.4x (expected 0x%0.4x)", ld->RegStatus, LDATA_REGISTERD_STS);
	}
	
	if(lcheck_RegRnum(ld->RegRnum) == FALSE)
	{
		result = FALSE;
		DBG_PRINT2("Check RegRnum failed: %d (expected [0:%d])", ld->RegRnum, LDATA_REG_NBR_MAX-1);
	}
	
	if(lcheck_RegYnum(ld->RegYnum) == FALSE) 
	{
		result = FALSE;
		DBG_PRINT2("Check RegRnum failed: %d (expected [0:%d])", ld->RegYnum, LDATA_REG_FIGURE_NBR_MAX-1);
	}
	
	if(lcheck_RegImg1((UB)ld->RegImg1, data) == FALSE) 
	{
		result = FALSE;
		DBG_PRINT2("Check RegImg1 failed: 0x%0.2x (expected 0x%0.2x)", ld->RegImg1, data);
	}
	
	return result;
}

static BOOL lcheck_SvLearnDataAt(UW bank_index, UW section_index, UW frame_index, SvLearnData *expected_ldPtr)
{
	SvLearnData *learnDataPtr;
	SvLearnResult *ld_result;
	UW frmAddr;
	BOOL result;
	result = FALSE;
	
	learnDataPtr = &g_learnData;
	ld_result = &g_learnDataResult;
	frmAddr = lcalc_FrameAddr(bank_index, section_index, frame_index);
	result = lreadFrame(frmAddr, learnDataPtr);
	// lreadFrameByIndex(bank_index, section_index, frame_index, learnDataPtr);
	lchangeToSvLearnResult(ld_result, learnDataPtr);
	result = lcheck_SvLearnDataResult(ld_result, expected_ldPtr->RegImg1[0]);
	DBG_PRINT8("Check SvLearnData at [B%dS%dF%d]{Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", bank_index, section_index, frame_index, learnDataPtr->RegRnum, learnDataPtr->RegYnum, learnDataPtr->RegID, learnDataPtr->RegStatus, learnDataPtr->RegImg1[0]);
	if(result == FALSE)
	{
		DBG_PRINT8("Expected [B%dS%dF%d]{Rnum=%d, Ynum=%d, ID=%d, Status=0x%0.4x, data=0x%0.2x}", bank_index, section_index, frame_index, expected_ldPtr->RegRnum, expected_ldPtr->RegYnum, expected_ldPtr->RegID, LDATA_REGISTERD_STS, expected_ldPtr->RegImg1[0]);
		DBG_PRINT0("Failed");
	}else
	{
		DBG_PRINT0("Success");
	}
	
	return result;
}

static BOOL lcheck_SvLearnDataAtIsRemoved(UW bank_index, UW section_index, UW frame_index)
{
	SvLearnData *learnDataPtr;
	UW frmAddr;
	BOOL result;
	
	learnDataPtr = &g_learnData;
	frmAddr = lcalc_FrameAddr(bank_index, section_index, frame_index);
	result = lreadFrame(frmAddr, learnDataPtr);

	return lcheck_RegStatus(learnDataPtr->RegStatus, LDATA_NOT_YET_STS);
}

static BOOL lcheck_ctrl_flg(UW bank_index_oldest, UW section_index_oldest, UW erased_bank_index, UW erased_section_index)
{
	UH ctrl_flg_erased, ctrl_flg_oldest;
	
	ctrl_flg_erased = lread_CtrlFlg(erased_bank_index, erased_section_index);
	ctrl_flg_oldest = lread_CtrlFlg(bank_index_oldest, section_index_oldest);
	// DBG_PRINT4("Check Control flag of current Section B%dS%d: 0x%0.4x (expected 0x%0.4x)", erased_bank_index, erased_section_index, ctrl_flg_erased, LDATA_CTRL_FLG_ERASED);
	// DBG_PRINT4("Check Control flag of oldest Section B%dS%d: 0x%0.4x (expected 0x%0.4x)", bank_index_oldest, section_index_oldest, ctrl_flg_oldest, LDATA_CTRL_FLG_OLDEST);
	DBG_PRINT6("Check control flag of current Section B%dS%d: 0x%0.4x and Oldest Section B%dS%d: 0x%0.4x", erased_bank_index, erased_section_index, ctrl_flg_erased, bank_index_oldest, section_index_oldest, ctrl_flg_oldest);
	////////get local variables of learnData.c//////////////////////////////////
	getOldestSection(&g_bank_oldest_index, &g_section_oldest_index, &g_ctrl_flg_oldest);
	// getCurrentCursor(&g_bank_index, &g_section_index, &g_frame_index);
	// DBG_PRINT6("Check current oldest Section B%dS%d: 0x%0.4x, next location B%dS%dF%d", g_bank_oldest_index, g_section_oldest_index, g_ctrl_flg_oldest, g_bank_index, g_section_index, g_frame_index); 
	//////////////////////////////////////////
	// if(ctrl_flg_erased == LDATA_CTRL_FLG_ERASED && ctrl_flg_oldest == LDATA_CTRL_FLG_OLDEST)
	if(bank_index_oldest == g_bank_oldest_index && section_index_oldest == g_section_oldest_index)
	{
		DBG_PRINT0("Success");
		return TRUE;
	}
	DBG_PRINT0("Failed");
	return FALSE;
}

static BOOL lcheck_notify_data(int notifyCase)
{
	UB expected_data;
	SvLearnData *expected_ldPtr;
	BOOL ret = FALSE;
	Location loc;
	
	switch (notifyCase){
	case 1:
	{
		// Check if unique data 1 was moved to 1st frame of Section
		DBG_PRINT0("[TC_UNIQUE_ADD_01] Check if 1 unique data was moved to Section B3S7");
		loc = lget_Location(GREEN_1_MOVED_LOCATION);
		expected_ldPtr = lcreateLearnData(LDATA_FINGER_GREEN_1_POS);
		ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);

		DBG_PRINT0("[TC_UNIQUE_ADD_01] Check if new data was add to B3S7F1");
		loc = lget_Location(RED_NEW_1_LOCATION);
		expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
		expected_ldPtr->RegStatus = LDATA_NOT_LATEST_STS;
		ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);

		break;
	}
	case 2:
	{
		// Check if unique data 1 was moved to 1st frame of Section
		DBG_PRINT0("[TC_UNIQUE_ADD_02] Check if 2 unique data was moved to Section B3S2");
		
		loc = lget_Location(YELLOW_1_MOVED_LOCATION);
		expected_ldPtr = lcreateLearnData(LDATA_FINGER_YELLOW_1_POS);
		ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);

		// Check if unique data 2 was moved to 2nd frame of Section		
		loc = lget_Location(YELLOW_2_MOVED_LOCATION);
		expected_ldPtr = lcreateLearnData(LDATA_FINGER_YELLOW_2_POS);
		ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);

		// Check if new data was stored at 3rd frame of Section		
		DBG_PRINT0("[TC_UNIQUE_ADD_02] Check if new data was add to B3S2F2");
		loc = lget_Location(RED_NEW_2_LOCATION);
		expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
		expected_ldPtr->RegStatus = LDATA_NOT_LATEST_STS;
		ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);

		break;
	}
	case 19:
	{
		// Check if all 19 unique data was moved to current Section
		// from Bank3 Section5 to Bank3 Section4
		int i;
		DBG_PRINT0("[TC_UNIQUE_ADD_19] Check if 19 unique data were moved to Section B3S4");
		for(i = 0; i<19; i++)
		{
			loc = lget_Location(ORANGE_1_MOVED_LOCATION+i);
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_ORANGE_1_POS+i);
			ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);
		}
			
		// Check if new data was stored at next Section
		// Bank3 Section5 Frame0	
		DBG_PRINT0("[TC_UNIQUE_ADD_19] Check if new data was add to B3S5F0");
		loc = lget_Location(RED_NEW_19_LOCATION);			
		expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
		expected_ldPtr->RegStatus = LDATA_NOT_LATEST_STS;
		ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);

		break;
	}
	default:
	break;
	};
	
	return ret;
}

static BOOL lcheck_ClearSection(UW bank_index, UW section_index)
{
	BOOL ret;
	DBG_PRINT2("Check if Section B%dS%d was cleared", bank_index, section_index);
	ret = lcheck_SvLearnDataAtIsRemoved(bank_index, section_index, N_FRM_PER_SECT-1);
	ret &= lcheck_SvLearnDataAtIsRemoved(bank_index, section_index, N_FRM_PER_SECT-2);
	ret &= lcheck_SvLearnDataAtIsRemoved(bank_index, section_index, N_FRM_PER_SECT-3);
	ret &= lcheck_SvLearnDataAtIsRemoved(bank_index, section_index, N_FRM_PER_SECT-4);
	if(ret == TRUE)
		DBG_PRINT0("Success");
	else
		DBG_PRINT0("Failed");
	
	return ret;
}

static void lcheck_PrepareUniqueData(void)
{	
	Location loc;
	SvLearnData *expected_ldPtr;
	BOOL ret = FALSE;
	int i;
	
	DBG_PRINT0("Check preparing unique data:");
	DBG_PRINT0("Check storing 2 unique data in 1 Section at B3S3F1 & B3S3F3");
	{
		loc = lget_Location(YELLOW_1_PREPARED_LOCATION);
		expected_ldPtr = &g_LearnDataArray[LDATA_FINGER_YELLOW_1_INDEX]; // YELLOW 1
		ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);

		loc = lget_Location(YELLOW_2_PREPARED_LOCATION);
		expected_ldPtr = &g_LearnDataArray[LDATA_FINGER_YELLOW_2_INDEX]; // YELLOW 2
		ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);
		
		if(ret == TRUE)
		{
			DBG_PRINT0("Success storing 2 unique data in 1 Section at B3S3F1 & B3S3F3");
		}else
		{
			DBG_PRINT0("Failed storing 2 unique data in 1 Section at B3S3F1 & B3S3F3");
		}
	}

	DBG_PRINT0("Check storing 19 unique data in 1 Section from B3S5F0 to B3S5F18");
	{
		for(i = 0; i<19; i++)
		{
			loc = lget_Location(ORANGE_1_PREPARED_LOCATION+i);
			expected_ldPtr = &g_LearnDataArray[LDATA_FINGER_ORANGE_1_INDEX+i]; // ORANGE 1->19
			ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);
		}
		
		if(ret == TRUE)
		{
			DBG_PRINT0("Success storing 19 unique data in 1 Section from B3S5F0 to B3S5F18");
		}else
		{
			DBG_PRINT0("Failed storing 19 unique data in 1 Section from B3S5F0 to B3S5F18");
		}
	}

	DBG_PRINT0("Check storing 1 unique data in 1 Section at B3S8F1");
	{
		loc = lget_Location(GREEN_1_PREPARED_LOCATION);
		expected_ldPtr = &g_LearnDataArray[LDATA_FINGER_GREEN_1_INDEX]; // GREEN 1
		ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);
		if(ret == TRUE)
		{
			DBG_PRINT0("Success storing 1 unique data in 1 Section at B3S8F1");
		}else
		{
			DBG_PRINT0("Failed storing 1 unique data in 1 Section at B3S8F1");
		}
	}
}

static int TC_ADD_FUNC(void)
{
    int result;
    SvLearnData *learnDataPtr;
	SvLearnData *expected_ldPtr;
	int ret;
	UB expected_data;
	Location loc;
	
	expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
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
			loc = lget_Location(FIRST_LOCATION);
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			expected_ldPtr->RegStatus = LDATA_REGISTERD_STS;
			expected_data = lFingerDataImg(g_numberOfLearnData);	// LDATA_FINGER_RED_1_INDEX
			ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);
		}else if(g_numberOfLearnData == B3S0F1)
		{
			/* Store 2nd data
			 * Bank=3 Section=0 Frame=1 [0x06000000]
			 * Bank=3 [0x06000000], Section=0 [0x06000000], Frame=1 [0x06001ad6] */
			DBG_PRINT0("[TC_ADD_02] Store 2nd data at B3S0F1");
			loc = lget_Location(SECOND_LOCATION);
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			expected_ldPtr->RegStatus = LDATA_REGISTERD_STS;
			expected_data = lFingerDataImg(g_numberOfLearnData);	// LDATA_FINGER_RED_1_INDEX
			ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);	
			
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
	Location loc;

	expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
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
		if(g_numberOfLearnData == B3S0F18)
		{
			/* Store until full data in Section0 (19th)
			 * Bank=3 Section=0 Frame=18
			 * Bank=3 [0x06000000], Section=0 [0x06000000], Frame=18 [0x0601e30c] */
			DBG_PRINT0("[TC_RING_01] Store full Section0 (19th) at B3S0F18");

			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,0, 3,0);	// Bank3 Section0 ctrl_flg : 0x0001
			// Check learn data
			loc = lget_Location(FULL_FIRST_SECTION_LOCATION);
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			expected_ldPtr->RegStatus = LDATA_REGISTERD_STS;
			ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);
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
			ret = lcheck_ClearSection(3,1);
			
			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,0, 3,1);	// Bank3 Section0 ctrl_flg : 0x0001
			// Check learn data
			loc = lget_Location(START_SECOND_SECTION_LOCATION);
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			expected_ldPtr->RegStatus = LDATA_REGISTERD_STS;
			ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);
		}
		else if(g_numberOfLearnData == B3S255F18)
		{
			/* Store until full all Sections in Bank3 (full Bank3)
			 * Bank=3 Section=255 Frame=18
			 * Bank=3 [0x06000000], Section=255 [0x07fe0000], Frame=18 [0x07ffe30c] */
			DBG_PRINT1("[TC_RING_03] Store full Bank3 at B3S%dF18", (MI_NUM_SEC_IN_BANK-1));

			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,0, BANK_NUM_MIN,(MI_NUM_SEC_IN_BANK-1));	// Bank3 Section0 ctrl_flg : 0x0001 (fast test MI_NUM_SEC_IN_BANK=10)			
			// Check learn data
			loc = lget_Location(FULL_FIRST_BANK_LOCATION);
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			expected_ldPtr->RegStatus = LDATA_REGISTERD_STS;
			ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);
		}
		else if(g_numberOfLearnData == B7S255F18)
		{
			/* Store until full all Sections in Bank3 (full Bank7)
			 * Bank=7 Section=255 Frame=18
			 * Bank=7 [0x0e000000], Section=255 [0x0ffe0000], Frame=18 [0x0fffe30c] */
			DBG_PRINT1("[TC_RING_04] Store full Bank7 at B7S%dF18",(MI_NUM_SEC_IN_BANK-1));
						
			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,0, BANK_NUM_MAX,(MI_NUM_SEC_IN_BANK-1));	// Bank3 Section0 ctrl_flg : 0x0001		
			// Check learn data
			loc = lget_Location(FULL_ALL_BANK_LOCATION);
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			expected_ldPtr->RegStatus = LDATA_REGISTERD_STS;
			ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);
		}
		else if(g_numberOfLearnData == (FULL_ALL_BANK+B3S0F0))
		{
			/* Store until full all Sections in Bank3 (full Bank7 +1frame )
			 * Bank=3 Section=0 Frame=0
			 * Bank=7 [0x06000000], Section=0 [0x06000000], Frame=0 [0x06000000] */
			DBG_PRINT0("[TC_RING_05] Store full Bank7 +1 frame at (FULL_ALL_BANK+B3S0F0)");

			// Check remove all data Bank3 Section0
			ret = lcheck_ClearSection(3,0);

			// Check ctrl_flg
			ret = lcheck_ctrl_flg(3,1, 3,0);	// Bank3 Section1 ctrl_flg : 0x0001
			// Check learn data
			loc = lget_Location(RESTART_FIRST_BANK_LOCATION);
			expected_ldPtr = lcreateLearnData(LDATA_FINGER_RED_1_POS);
			expected_ldPtr->RegStatus = LDATA_REGISTERD_STS;
			ret = lcheck_SvLearnDataAt(loc.bankIndex, loc.secIndex, loc.frmIndex, expected_ldPtr);

			DBG_PRINT0("[TC_RING_FUNC] Done");
			return 0;
		}
		////////////////////////////////////////////////////////////
		/* add Unique data */
		if(g_numberOfLearnData == B3S3F1 || g_numberOfLearnData == B3S3F3)
		{
			// DBG_PRINT0("[TC_UNIQUE_PREPARE_02] Store 2 unique data in 1 Section at B3S3F1 & B3S3F3");
		}
		else if(B3S5F0 <= g_numberOfLearnData && g_numberOfLearnData <= B3S5F18)
		{
			// DBG_PRINT0("[TC_UNIQUE_PREPARE_19] Store 19 unique data in 1 Section from B3S5F0 to B3S5F18");
		}
		else if(g_numberOfLearnData == B3S8F1)
		{
			// DBG_PRINT0("[TC_UNIQUE_PREPARE_01] Store 1 unique data in 1 Section at B3S8F1");
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
			/* Check remove all data Bank3 Section2 */
			ret = lcheck_ClearSection(3,2);
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
			/* Check remove all data Bank3 Section5 */
			ret = lcheck_ClearSection(3,5);
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
			/* Check remove all data Bank3 Section7 */
			ret = lcheck_ClearSection(3,7);
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

	UW BankNum, SectionNum, FrameNum, Num;
	UH id;

	DBG_PRINT0("[TC_MAPPING_FUNC] Test updating latest registered location to InfoLearningBankTable[][]");
	learnDataPtr = &g_LearnDataArray[LDATA_FINGER_BLUE_1_INDEX];
	learnDataPtr->RegRnum = BLUE_ROOM_1;
	learnDataPtr->RegYnum = BLUE_1_YNUM;
	result = AddSvLearnImg(learnDataPtr);
	
	DBG_PRINT2("BLUE member inserts finger at room %d, finger index %d", BLUE_ROOM_1, BLUE_1_YNUM);
	get_InfoLearnInBankM(BLUE_ROOM_1, BLUE_1_YNUM, &BankNum, &SectionNum, &FrameNum, &Num, &id);
	DBG_PRINT7("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d, ID=%d}", BLUE_ROOM_1, BLUE_1_YNUM, BankNum, SectionNum, FrameNum, Num, id);
	get_InfoLearnInBankM(BLUE_ROOM_2, BLUE_2_YNUM, &BankNum, &SectionNum, &FrameNum, &Num, &id);
	DBG_PRINT7("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d, ID=%d}", BLUE_ROOM_2, BLUE_2_YNUM, BankNum, SectionNum, FrameNum, Num, id);
	g_numberOfLearnData++;
	
	learnDataPtr = &g_LearnDataArray[LDATA_FINGER_BLUE_1_INDEX];
	learnDataPtr->RegRnum = BLUE_ROOM_1;
	learnDataPtr->RegYnum = BLUE_1_YNUM;
	result = AddSvLearnImg(learnDataPtr);
	DBG_PRINT2("BLUE member inserts finger at same room %d and finger index %d", BLUE_ROOM_1, BLUE_1_YNUM);
	get_InfoLearnInBankM(BLUE_ROOM_1, BLUE_1_YNUM, &BankNum, &SectionNum, &FrameNum, &Num, &id);
	DBG_PRINT7("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d, ID=%d}", BLUE_ROOM_1, BLUE_1_YNUM, BankNum, SectionNum, FrameNum, Num, id);
	get_InfoLearnInBankM(BLUE_ROOM_2, BLUE_2_YNUM, &BankNum, &SectionNum, &FrameNum, &Num, &id);
	DBG_PRINT7("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d, ID=%d}", BLUE_ROOM_2, BLUE_2_YNUM, BankNum, SectionNum, FrameNum, Num, id);
	g_numberOfLearnData++;

	learnDataPtr = &g_LearnDataArray[LDATA_FINGER_BLUE_1_INDEX];
	learnDataPtr->RegRnum = BLUE_ROOM_1;
	learnDataPtr->RegYnum = BLUE_1_YNUM;
	result = AddSvLearnImg(learnDataPtr);
	DBG_PRINT2("BLUE member inserts finger at same room %d and finger index %d", BLUE_ROOM_1, BLUE_1_YNUM);
	get_InfoLearnInBankM(BLUE_ROOM_1, BLUE_1_YNUM, &BankNum, &SectionNum, &FrameNum, &Num, &id);
	DBG_PRINT7("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d, ID=%d}", BLUE_ROOM_1, BLUE_1_YNUM, BankNum, SectionNum, FrameNum, Num, id);
	get_InfoLearnInBankM(BLUE_ROOM_2, BLUE_2_YNUM, &BankNum, &SectionNum, &FrameNum, &Num, &id);
	DBG_PRINT7("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d, ID=%d}", BLUE_ROOM_2, BLUE_2_YNUM, BankNum, SectionNum, FrameNum, Num, id);
	g_numberOfLearnData++;
	
	learnDataPtr = &g_LearnDataArray[LDATA_FINGER_BLUE_1_INDEX];
	learnDataPtr->RegRnum = BLUE_ROOM_2;
	learnDataPtr->RegYnum = BLUE_2_YNUM;
	result = AddSvLearnImg(learnDataPtr);
	DBG_PRINT2("BLUE member inserts finger at different room %d and finger index %d", BLUE_ROOM_2, BLUE_2_YNUM);
	get_InfoLearnInBankM(BLUE_ROOM_1, BLUE_1_YNUM, &BankNum, &SectionNum, &FrameNum, &Num, &id);
	DBG_PRINT7("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d, ID=%d}", BLUE_ROOM_1, BLUE_1_YNUM, BankNum, SectionNum, FrameNum, Num, id);
	get_InfoLearnInBankM(BLUE_ROOM_2, BLUE_2_YNUM, &BankNum, &SectionNum, &FrameNum, &Num, &id);
	DBG_PRINT7("BLUE_1 [%d][%d]: {B%dS%dF%d, Num=%d, ID=%d}", BLUE_ROOM_2, BLUE_2_YNUM, BankNum, SectionNum, FrameNum, Num, id);
	g_numberOfLearnData++;

	DBG_PRINT0("[TC_MAPPING_FUNC] Done");
	return 0;
}

static int TC_SEARCH_FUNC(void)
{
	UW roomInfo[20][4];
	int i = 0;
	
	DBG_PRINT0("[TC_SEARCH_FUNC] Test SearchLearnImg(), search latest registered location (Bank-Section-Frame)");
	memset(roomInfo, 0, sizeof(roomInfo));
	// RED room
	SearchLearnImg(RED_ROOM, roomInfo);
	DBG_PRINT6("Room %d, finger %d: {B%dS%dF%d, ID=%d}", RED_ROOM, RED_1_YNUM, roomInfo[RED_1_YNUM][0], roomInfo[RED_1_YNUM][1], roomInfo[RED_1_YNUM][2], roomInfo[RED_1_YNUM][3]);
	// SearchLearnImg(BLUE_ROOM_1, roomInfo);
	DBG_PRINT6("Room %d, finger %d: {B%dS%dF%d, ID=%d}", BLUE_ROOM_1, BLUE_1_YNUM, roomInfo[BLUE_1_YNUM][0], roomInfo[BLUE_1_YNUM][1], roomInfo[BLUE_1_YNUM][2], roomInfo[BLUE_1_YNUM][3]);
	
	// GREEN room
	SearchLearnImg(GREEN_ROOM, roomInfo);
	DBG_PRINT6("Room %d, finger %d: {B%dS%dF%d, ID=%d}", GREEN_ROOM, GREEN_1_YNUM, roomInfo[GREEN_1_YNUM][0], roomInfo[GREEN_1_YNUM][1], roomInfo[GREEN_1_YNUM][2], roomInfo[GREEN_1_YNUM][3]);
	// SearchLearnImg(BLUE_ROOM_2, roomInfo);
	DBG_PRINT6("Room %d, finger %d: {B%dS%dF%d, ID=%d}", BLUE_ROOM_2, BLUE_2_YNUM, roomInfo[BLUE_2_YNUM][0], roomInfo[BLUE_2_YNUM][1], roomInfo[BLUE_2_YNUM][2], roomInfo[BLUE_2_YNUM][3]);	
	
	// YELLOW room
	SearchLearnImg(YELLOW_ROOM, roomInfo);
	DBG_PRINT6("Room %d, finger %d: {B%dS%dF%d, ID=%d}", YELLOW_ROOM, YELLOW_1_YNUM, roomInfo[YELLOW_1_YNUM][0], roomInfo[YELLOW_1_YNUM][1], roomInfo[YELLOW_1_YNUM][2], roomInfo[YELLOW_1_YNUM][3]);
	DBG_PRINT6("Room %d, finger %d: {B%dS%dF%d, ID=%d}", YELLOW_ROOM, YELLOW_2_YNUM, roomInfo[YELLOW_2_YNUM][0], roomInfo[YELLOW_2_YNUM][1], roomInfo[YELLOW_2_YNUM][2], roomInfo[YELLOW_2_YNUM][3]);

	// ORANGE room
	SearchLearnImg(ORANGE_ROOM, roomInfo);
	for(i = 0; i<19; i++)
	{
		DBG_PRINT6("Room %d, finger %d: {B%dS%dF%d, ID=%d}",ORANGE_ROOM, ORANGE_1_YNUM+i, roomInfo[ORANGE_1_YNUM+i][0], roomInfo[ORANGE_1_YNUM+i][1], roomInfo[ORANGE_1_YNUM+i][2], roomInfo[ORANGE_1_YNUM+i][3]);
	}
	DBG_PRINT0("[TC_SEARCH_FUNC] Done.");	
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
	SvLearnData *learnDataPtr;
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

	/* Create member ID*/
	g_LearnDataArray[LDATA_FINGER_GREEN_1_INDEX].RegID = GREEN_1_ID;
	g_LearnDataArray[LDATA_FINGER_YELLOW_1_INDEX].RegID = YELLOW_1_ID;
	g_LearnDataArray[LDATA_FINGER_YELLOW_2_INDEX].RegID = YELLOW_2_ID;
	for(i=0; i<19; i++)
	{
		g_LearnDataArray[LDATA_FINGER_ORANGE_1_INDEX+i].RegID = ORANGE_1_ID + i;
	}
	g_LearnDataArray[LDATA_FINGER_RED_1_INDEX].RegID = RED_1_ID;
	g_LearnDataArray[LDATA_FINGER_BLUE_1_INDEX].RegID = BLUE_1_ID;
	
	/* Create room number*/
	g_LearnDataArray[LDATA_FINGER_GREEN_1_INDEX].RegRnum = GREEN_ROOM;
	g_LearnDataArray[LDATA_FINGER_YELLOW_1_INDEX].RegRnum = YELLOW_ROOM;
	g_LearnDataArray[LDATA_FINGER_YELLOW_2_INDEX].RegRnum = YELLOW_ROOM;
	for(i=0; i<19; i++)
	{
		g_LearnDataArray[LDATA_FINGER_ORANGE_1_INDEX+i].RegRnum = ORANGE_ROOM;
	}
	g_LearnDataArray[LDATA_FINGER_RED_1_INDEX].RegRnum = RED_ROOM;
	g_LearnDataArray[LDATA_FINGER_BLUE_1_INDEX].RegRnum = BLUE_ROOM_1;	// BLUE has 2 rooms
	
	/* Create finger index*/
	g_LearnDataArray[LDATA_FINGER_GREEN_1_INDEX].RegYnum = GREEN_1_YNUM;
	g_LearnDataArray[LDATA_FINGER_YELLOW_1_INDEX].RegYnum = YELLOW_1_YNUM;
	g_LearnDataArray[LDATA_FINGER_YELLOW_2_INDEX].RegYnum = YELLOW_2_YNUM;
	for(i=0; i<19; i++)
	{
		g_LearnDataArray[LDATA_FINGER_ORANGE_1_INDEX+i].RegYnum = ORANGE_1_YNUM + i;
	}
	g_LearnDataArray[LDATA_FINGER_RED_1_INDEX].RegYnum = RED_1_YNUM;
	g_LearnDataArray[LDATA_FINGER_BLUE_1_INDEX].RegYnum = BLUE_1_YNUM;
	
#ifdef DEBUG
	dbg_init();
#endif

	// Initialize global variables
	g_numberOfLearnData = 0;
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
				printCurrentMappingInformation();
				DBG_PRINT1("Switch to LDATA_RING_CASE, num=%d", g_numberOfLearnData);
			}else goto END_TASK;    /* show test case fail */
		break;
		case LDATA_RING_CASE:
			ret = TC_RING_FUNC();
			if(ret == 0)
			{
				testCase = LDATA_NOTIFY_CASE;
				lcheck_PrepareUniqueData();
				printCurrentMappingInformation();
				DBG_PRINT1("Switch to LDATA_NOTIFY_CASE, num=%d", g_numberOfLearnData);
			}else goto END_TASK;    /* show test case fail */
		break;
		case LDATA_NOTIFY_CASE:
			ret = TC_NOTIFY_FUNC();
			if(ret == 0)
			{
				testCase = LDATA_MAPPING_CASE;
				printCurrentMappingInformation();
				DBG_PRINT1("Switch to LDATA_MAPPING_CASE, num=%d", g_numberOfLearnData);
			}else goto END_TASK;    /* show test case fail */
		break;
		case LDATA_MAPPING_CASE:
			ret = TC_MAPPING_FUNC();
			if(ret == 0)
			{
				testCase = LDATA_SEARCH_CASE;
				printCurrentMappingInformation();
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
