/******************************************************************************/
/* File Name       : learnData.c                                              */
/* Description     : This file is source code of the learn data component in  */
/*                   the framework layer.                                     */
/* Author          : TanTrinh                                                 */
/* Created         : April 15, 2018                                           */
/* Modified by     : Tan Trinh                                                */
/* Date Modified   : April 15, 2018                                           */
/* Content Modified:                                                          */
/* Copyright (c) 2015 SH Consuting K.K.                                       */
/*  and SH Consulting VietNam. All Rights Reserved.                           */
/******************************************************************************/
#include "frameworkConf.h"
#if 1//FWK_CFG_LEARN_DATA_ENABLE

/******************************************************************************/
/*************************** Project Header ***********************************/
/******************************************************************************/
#include "learnData.h"
#ifdef FAST_TEST
#include "tsk_learnData.h"	// fast test
#endif
/******************************************************************************/
/*************************** Macro Definitions ********************************/
/******************************************************************************/
#define LDATA_FINGER_INFO_SIZE          (3)
#define LDATA_WORD_SIZE                 (sizeof(UW))

/* Group definitions for configuration in flash memory ************************/
#define CALC_SBANK_ADDR(sbank)         (MI_FLASH_BASIC_ADDR+(sbank*MI_BANK_SIZE))
#define CALC_AREA_SIZE(sbank, ebank)   ((ebank-sbank+1)*MI_BANK_SIZE)
#define CALC_EBANK_ADDR(ebank)           (MI_FLASH_BASIC_ADDR+((ebank+1)*MI_BANK_SIZE)-1)


/* number of frame in learning data area */
#define CALC_FRAME_NUM_IN_LAREA        ((CALC_AREA_SIZE(ldataSBank, ldataEBank)/MI_SECTOR_SIZE) * LDATA_FRAME_NUM_IN_SECTOR)

// #define LDATA_ALL_IMG_SIZE              ((2*LDATA_NORM_IMAGE_SIZE) + (LDATA_MINI_IMAGE_NUM*LDATA_MINI_IMAGE_SIZE))
/******************************************************************************/
/*************************** Structures Definitions ***************************/
/******************************************************************************/
typedef struct Info_ST {
	UB bankIndex;
	UB secIndex;
	UB frmIndex;
	// UW bankAddr;
	UW secAddr;
	UW frmAddr;
	UB num;
} Info;
/******************************************************************************/
/*************************** Enum Definitions *********************************/
/******************************************************************************/


/******************************************************************************/
/*************************** Extern Functions *********************************/
/******************************************************************************/
#if FWK_LD_SEMAPHORE_ENABLE
extern void ldataosInit(void);
extern void ldataosGetSemaphore(void);
extern void ldataosReleaseSemaphore(void);
#endif

extern void miInit(void);
extern BOOL miEraseSector(UW secAddr);
extern BOOL miReadHword(UW addr, UH* halfWord);
extern BOOL miReadRange(UW startAddr, UB* buffer, UW length);
extern BOOL miWriteRange(UW startAddr, UB* buffer, UW length);
extern BOOL miWriteSector(UW secAddr, UB* buffer, UW length);
extern BOOL miWriteInSector(UW sectorAddr, UW offset, UB* buffer, UW length);

/******************************************************************************/
/*************************** Functions Prototype ******************************/
/******************************************************************************/
static int UpdateLearnInfo(UB BankSw, UB Spec);
static BOOL ldataFindCurLatestFinger(UH rNum, UH yNum, UW* curBankNum,
                                     UW* curSecNum, UW* curFrmNum);
static BOOL ldataUpdateCurLatestToOld(UW curBankNum, UW curSecNum, UW curFrmNum);
static BOOL ldataRemoveAllData(void);
static BOOL ldataCheckFrameOnlyOneData(UW secAddr, UB* number, Info* learnDataInSection[]);
static BOOL ldataMoveOnlyOneDataToTop(UB bankNum, UB sectorNum, UB frameNum);
static BOOL ldataStoreFrameData(UW frmAddr, SvLearnData *lDataPtr);

static BOOL lcheck_BankIndex(UW bankIndex);
static BOOL lcheck_SecIndex(UW secIndex);
static BOOL lcheck_FrmIndex(UW frmIndex);
static BOOL lcheck_RegStatus(UH checked_val, UH expected_val);
static BOOL lcheck_RegRnum(UH checked_val);
static BOOL lcheck_RegYnum(UH checked_val);
static BOOL lcheck_RegImg1(UH* checked_finger_vein);	// need modify to check all buffer
static BOOL lcheck_SvLearnData(SvLearnData* ld ,UH data);
static BOOL lcheck_ID(UW code);
static BOOL lmap_checkIsNewID(UW code);

static UW lcalc_BankAddr(UW bankIndex);
static UW lcalc_SectionAddr(UW bankIndex, UW secIndex);
static UW lcalc_FrameAddr(UW bankIndex, UW secIndex, UW frmIndex);

static BOOL lread_Frame(UW frmAddr, SvLearnData *learnDataPtr);
static void lwrite_Frame(UW frmAddr, SvLearnData* learnDataPtr);
// static void lmove_Frame(UW frmAddr1, UW frmAddr2);
static void lremove_Frame(UW frmAddr);
static BOOL lremove_Section(UW secAddr);
static BOOL lremove_All(void);
static BOOL lread_RegStatus(UW frmAddr, UH *RegStatus);
static void lwrite_RegStatus(UW frmAddr, UH RegStatus);
static BOOL lread_RegRnum(UW frmAddr, UH *RegRnum);
static void lwrite_RegRnum(UW frmAddr, UH RegRnum);
static BOOL lread_RegYnum(UW frmAddr, UH *RegYnum);
static void lwrite_RegYnum(UW frmAddr, UH RegYnum);
static BOOL lread_RegID(UW frmAddr, UH *RegID);
static void lwrite_RegID(UW frmAddr, UH RegID);
static void lwrite_Dummy1(UW frmAddr, UH* Dummy1);
static void lwrite_RegImg(UW frmAddr, UH* RegImg);
static void lwrite_MiniImg(UW frmAddr, UH* MiniImg);
static BOOL lread_CtrlFlg(UW secAddr, UH* ctrl_flg);
static void lwrite_CtrlFlg(UW secAddr, UH ctrl_flg);

static BOOL lread_FrameByIndex(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData *learnDataPtr);
static void lremove_FrameByIndex(UW bankIndex, UW secIndex, UW frmIndex);
static void lmove_FrameByIndex(UW bankIndex1, UW secIndex1, UW frmIndex1, UW bankIndex2, UW secIndex2, UW frmIndex2);
static BOOL lremove_SectionByIndex(UW bankIndex, UW secIndex)
// static UW lcalc_CtrlFlgAddr(UW bankIndex, UW secIndex);
static UH lread_CtrlFlgByIndex(UW bankIndex, UW secIndex);

static void lwrite_CtrlFlgByIndex(UW bankIndex, UW secIndex, UH value);
// static void lset_OldestSection(UW bankIndex, UW secIndex);
// static void lclear_OldestSection(UW bankIndex, UW secIndex);
static void lmap_writeMapInfoTable(UH rNum, UH yNum, UW bankIndex, UW secIndex, UW frmIndex, UW num, UH id);
static void lmap_readMapInfoTable(UH rNum, UH yNum, UW *bankIndex, UW *secIndex, UW *frmIndex, UW *num, UH *id);
static BOOL lmap_getIndexFromIDList(UH code, UH *index);
static BOOL lmap_updateMapInfoTable_location(UW bankIndex, UW secIndex, UW frmIndex);
static BOOL lmap_updateMapInfoTable_num(UH rNum, UH yNum);
static BOOL lmap_updateIDList(UW bankIndex, UW secIndex, UW frmIndex);
static void lupdateLearnInfo(void); // --> UpdateLearnInfo()
// static void lscanRegIDInFlash(void);
static BOOL lmap_getCountFromIDList(UH code, UW *count);
static void lupdate_NotTheLatestFrame(SvLearnData* new_learnDataPtr);
static void lupdate_NextFrameLocation(UW bankIndex, UW secIndex, UW frmIndex);
static BOOL lstoreData(UW frmAddr, SvLearnData *learnDataPtr);
static BOOL lstoreToNewSection(SvLearnData *learnDataPtr);
static BOOL lstore1stFrame(SvLearnData *learnDataPtr);
/******************************************************************************/
/*************************** Local Constant Variables *************************/
/******************************************************************************/
// static volatile SvLearnData InitlearnData = {
    // LDATA_NOT_YET_STS, /* not yet */
    // 0,                 /* room 0 */
    // 1                  /* Registration figure number */
// };

/******************************************************************************/
/*************************** Local Variables **********************************/
/******************************************************************************/
static volatile UB g_start_bank_index, g_end_bank_index;
static UB g_bank_num_max;
static volatile UB ldataActivedBank;
static volatile UH g_control_flag;
static volatile UW g_FrameNumber;
static volatile InfoLearnInBankM InfoLearningBankTable[LDATA_REG_NBR_MAX]; //// 480
// static volatile UH roomNumList[] = {
    // LDATA_REG_NBR_MAX, //// 480
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX,
    // LDATA_REG_NBR_MAX
// };
static UW g_bankIndex=0, g_secIndex=0, g_frmIndex=0;
static UW g_next_bankIndex=0, g_next_secIndex=0;
static UW g_oldest_bank_index=0, g_oldest_section_index=0;
// static UH g_ctrl_flg_oldest=0;
static SvLearnData g_learnData;
static Info g_learnDataInSection[LDATA_FRAME_NUM_IN_SECTOR];
static EmployeeList g_flash_data_info[4800];
static UW g_total_empl=0; // 480*10
static UB g_add_first_frame=0;
static g_movedDataLocation[19];


#ifdef TEST_API
void get_InfoLearnInBankM(int rNum, int yNum, UB* BankNum, UB* SectionNum, UB* FrameNum, UB* Num)
{
	*BankNum = InfoLearningBankTable[rNum].BankNum[yNum];
	*SectionNum = InfoLearningBankTable[rNum].SectionNum[yNum];
	*FrameNum = InfoLearningBankTable[rNum].FrameNum[yNum];
	*Num = InfoLearningBankTable[rNum].Num[yNum];
}
#endif

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
	{	B3S1F0,				B3S3F2-1,						B3S3F2		},					// Store 2 unique data in 1 Section Bank=3 Section=3 Frame=1,3 (2nd, 4th)
	// {	XXXXXXXXXX,				XXXXXXXXXX,						B3S3F4		},			// Store 2 unique data in 1 Section
	{	B3S3F4,				B3S5F0-1,						B3S5F0		},					// Store 19 unique data in 1 Section
	// {	XXXXXXXXXX,				XXXXXXXXXX,						B3S5F18		},		    // Store 19 unique data in 1 Section
	{	B3S6F0,				B3S8F2-1,						B3S8F2		},					// Store 1 unique data in 1 Section , Bank=3 Section=7 Frame=2
	{	B3S8F2,				B3S255F18-1,					B3S255F18		},		// Store full Bank3
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
/*************************** Functions Definition *****************************/
/******************************************************************************/
/*
 *
 */
int InitEmployeeList(void)
{
	g_total_empl = 0;
	memset(g_flash_data_info, 0, sizeof(g_flash_data_info));
	return 0;
}

void InitInfoLearningBankTable(void)
{
	memset(InfoLearningBankTable, 0, sizeof(InfoLearningBankTable));
}

/*
 * Call after InitBankArea() to initialize g_start_bank_index
 */
int InitOldestSection(void)
{
	// g_oldest_bank_index = g_start_bank_index;
	// g_oldest_section_index = 0;
	g_add_first_frame = 1;
}
/******************************************************************************/
/* Function Name: InitBankArea                                                */
/* Description  : This function will initial the learn data area in the flash */
/*                memory from the bank 3 to bank 7                            */
/* Parameter    : Input: BankSw - is list of bank bits that is value as below:*/
/*                       BANK7: bit 7.                                        */
/*                       BANK6: bit 6.                                        */
/*                       BANK5: bit 5.                                        */
/*                       BANK4: bit 4.                                        */
/*                       BANK3: bit 3.                                        */
/*                       BANK2: bit 2.                                        */
/*                       BANK1: bit 1.                                        */
/* Return Value : int - return the zero value when initials successful, else  */
/*                to return the one value.                                    */
/* Remarks      : None                                                        */
/******************************************************************************/
int InitBankArea(UB BankSw)
{
    BOOL result;
    UB bankIndex;
    
    result = FALSE;
    if((BankSw & BANK0)==0) {
        miInit();
        /* find start bank and end bank in flash memory for learn data area */
        g_start_bank_index = 0;
        g_end_bank_index = 0;

		for(bankIndex = 1; bankIndex < BANK_MAX_NUM; bankIndex++){
			if(g_start_bank_index == 0){
				if(BankSw & (UB)LDATA_BIT(bankIndex))	// b00000001 << 0 = 0x01, b00000001 << 1 = b00000010, b00000001 << 7 = b10000000
				{
					g_start_bank_index = bankIndex;	// 1:7
					break;
				}
			}
		}
		
		for(g_end_bank_index = bankIndex; bankIndex < BANK_MAX_NUM; bankIndex++)
		{
			
			if(!(BankSw & (UB)LDATA_BIT(bankIndex)))
			{
				break;
			}
			else
				g_end_bank_index = bankIndex;	// 1:7
		}		
    }
	
	g_bank_num_max = g_end_bank_index - g_start_bank_index + 1;
	result = TRUE;
    
    if(result)
    {
        // result = ldataRemoveAllData();
		result = lremove_All();
        if(result) {
            g_control_flag = 0x0001;
#if FWK_LD_SEMAPHORE_ENABLE
            ldataosInit();
#endif
            ldataActivedBank = BankSw;
            g_FrameNumber = 0;
			InitInfoLearningBankTable();
        }
    }
    ////////////////////////////////////
	InitEmployeeList();
	InitOldestSection();
	///////////////////////////////////
	return ((result == TRUE) ? 0 : 1);
}

/******************************************************************************/
/* Function Name: InitLearnInfo                                               */
/* Description  : This function will update InfoLearnInBankM by searching the */
/*                latest learn data from sector (store ring buffer value) to  */
/*                0xFFFFFFFF in specified Banks (BankSw).                     */
/* Parameter    : Input: BankSw - is list of bank bits that is value as below:*/
/*                       BANK7: bit 7.                                        */
/*                       BANK6: bit 6.                                        */
/*                       BANK5: bit 5.                                        */
/*                       BANK4: bit 4.                                        */
/*                       BANK3: bit 3.                                        */
/*                       BANK2: bit 2.                                        */
/*                       BANK1: bit 1.                                        */
/*                Input: Spec - is a kink of structure that will generate to  */
/*                       remap latest learn data on flash memory.             */
/*                       0: For Apartment type(Create InfoLearnInBankM).      */
/*                       1: For Company type (Create InfoLearnInBankC).       */
/* Return Value : int - return the zero value when updates successful, else to*/
/*                return the one value.                                       */
/* Remarks      : None                                                        */
/******************************************************************************/
int InitLearnInfo(UB BankSw, UB Spec)
{
    volatile BOOL result, searchFinish, searchFromFirst;
    volatile UB bankIndex, bankStoreRingVal;
    volatile UH yNum, rNum, secStoreRingVal;
    volatile UH fingerInfo[LDATA_FINGER_INFO_SIZE]; //// 3 bytes : status, room, finger
    volatile UW bankAddr, secAddr, frmAddr, secIndex, frmIndex, temp;
    volatile SvLearnData* learnDataPtr;
    
#if FWK_LD_SEMAPHORE_ENABLE
    ldataosGetSemaphore();
#endif

    result = ((BankSw>0)&((Spec==APARTMENT_TYPE)||(Spec==COMPANY_TYPE)));
    if(result && (Spec == APARTMENT_TYPE)) {
        secStoreRingVal = 0;
        bankStoreRingVal = 0;
        searchFinish = FALSE;
        // memset(&InfoLearningBankTable[0], 0xFF, LDATA_REG_NBR_MAX*sizeof(InfoLearnInBankM));
		InitInfoLearningBankTable();
        /* Find location of ring buffer value */
        for(bankIndex=0; bankIndex<BANK_MAX_NUM; bankIndex++) {
            if((BankSw&LDATA_BIT(bankIndex))!=0) {
                for(secIndex=0; secIndex<MI_NUM_SEC_IN_BANK; secIndex++) {
					secAddr = lcalc_SectionAddr(bankIndex, secIndex);
                    result = miReadRange(secAddr+CTRL_FLAG_OFFSET, (UB*)fingerInfo, sizeof(UH));
                    if(result && (fingerInfo[0]==THE_OLDEST_SECTION)) { //// 0x0001
                        secStoreRingVal = secIndex; //// top or oldest data location
                        bankStoreRingVal = bankIndex; //// top or oldest data location
                        searchFinish = TRUE;
                        break;
                    }
                }
            }
            if(searchFinish)
            {
                break;
            }
        }
        if(result && searchFinish)
        {
            searchFinish = FALSE;
            searchFromFirst = FALSE;
            learnDataPtr = (SvLearnData*)fingerInfo;
            /* Find latest learn data from sector (store ring buffer value) to 0xFFFFFFFF in BankSw.*/
            for(bankIndex=bankStoreRingVal; bankIndex<BANK_MAX_NUM; bankIndex++) {
                if((BankSw&LDATA_BIT(bankIndex))!=0) {
                    for(secIndex=secStoreRingVal; secIndex<MI_NUM_SEC_IN_BANK; secIndex++) {                     
                        for(frmIndex=0; frmIndex<LDATA_FRAME_NUM_IN_SECTOR; frmIndex++) {
							frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
                            result = miReadRange(frmAddr, (UB*)fingerInfo, sizeof(fingerInfo));
                            if(result)
                            {
                                if((fingerInfo[0]!=0xFFFF)&&(fingerInfo[1]!=0xFFFF)) { //// frames storing data
                                    rNum = learnDataPtr->RegRnum;
                                    yNum = learnDataPtr->RegYnum;
                                    if(learnDataPtr->RegStatus==LDATA_REGISTERD_STS) { //// 0xFFFC new registerd
                                        InfoLearningBankTable[rNum].BankNum[yNum]    = bankIndex;
                                        InfoLearningBankTable[rNum].SectionNum[yNum] = secIndex;
                                        InfoLearningBankTable[rNum].FrameNum[yNum]   = frmIndex;
                                        InfoLearningBankTable[rNum].Num[yNum]++; //// update all data when adding new data
                                        // roomNumList[yNum] = rNum;
                                    }else if(learnDataPtr->RegStatus==LDATA_NOT_LATEST_STS) { //// 0xFFF8 old registerd
                                        InfoLearningBankTable[rNum].BankNum[yNum]    = bankIndex;
                                        InfoLearningBankTable[rNum].SectionNum[yNum] = secIndex;
                                        InfoLearningBankTable[rNum].FrameNum[yNum]   = frmIndex;
                                        InfoLearningBankTable[rNum].Num[yNum]++;
                                    }
                                }else { //// frames don't have data, all data are 0xFFFF
                                    searchFinish = TRUE;
                                }
                            }
                            if(searchFinish || (!result))
                            {
                                break;
                            }
                        }
                        if(searchFinish || (!result))
                        {
                            break;
                        }
                    }
                    secStoreRingVal = 0;
                }
                if(searchFinish || (!result))
                {
                    break;
                }
                
                if(result)
                {
                    if((!searchFromFirst) && (bankIndex==(BANK_MAX_NUM-1)))
                    {
                        if(!searchFinish)
                        {
                            /* In writing full case, Find latest learn data from first bank to 0xFFFFFFFF in BankSw.*/
                            bankIndex = g_start_bank_index-1;
                            searchFromFirst = TRUE;
                        }
                    }else if(searchFromFirst && (bankIndex==bankStoreRingVal)) {
                        break;
                    }
                }
            }
        }else{
            result = FALSE;
        }
    }

#if FWK_LD_SEMAPHORE_ENABLE
    ldataosReleaseSemaphore();
#endif
    (result ? 0 : 1);
}

/******************************************************************************/
/* Function Name: AddSvLearnImg                                               */
/* Description  : This function will store new learn data into flash memory.  */
/* Parameter    : Input: Data - is a buffer that stores figure images and     */
/*                       current room that finger joins last time.            */
/* Return Value : int - return the zero value when stores successful, else to */
/*                return the one value.                                       */
/* Remarks      : None                                                        */
/******************************************************************************/
int AddSvLearnImg(SvLearnData *Data)
{
    volatile BOOL result, updateCurLatestData, notifyOnlyOneData;
    volatile UB numOnlyOneData, bankNumOnlyOneData, secNumOnlyOneData, frmNumOnlyOneData;
    volatile UH ringBufferValue;
    volatile UW frmNumInArea, frmOffsetInArea, secAddr, frmIndex, frmAddr;
    volatile UW latestBankNum, latestSecNum, latestFrmNum;
    
#if FWK_LD_SEMAPHORE_ENABLE
    ldataosGetSemaphore();
#endif

#ifdef FAST_TEST
		fast_test(&g_FrameNumber);
#endif

	/* Check valid of input parameters */
    result = (Data != NULL);
    if(result) {
        result = ((Data->RegRnum<LDATA_REG_NBR_MAX)&&(Data->RegYnum<LDATA_REG_FIGURE_NBR_MAX));
    }
    if(result)
    {
        notifyOnlyOneData = FALSE;
        frmNumInArea = CALC_FRAME_NUM_IN_LAREA; //// number of frame in learning data area 
        frmOffsetInArea = g_FrameNumber % frmNumInArea; //// need convert bankIndex g_FrameNumber = g_FrameNumber-1
        
        /* calculate sector address */ //// BUG: secAddr, frmIndex, frmAddr calc wrong
        secAddr = CALC_SBANK_ADDR(g_start_bank_index);
        secAddr += (frmOffsetInArea/LDATA_FRAME_NUM_IN_SECTOR)*MI_SECTOR_SIZE;
        frmIndex = (frmOffsetInArea % LDATA_FRAME_NUM_IN_SECTOR); //// number frame in a sector (19)
        frmAddr = secAddr + (frmIndex * LDATA_FRAME_DATA_SIZE);
        
        if(g_FrameNumber <= 0) { //// write first data
            // result = miWriteInSector(secAddr,
                                     // CTRL_FLAG_OFFSET,
                                     // (UB*)&g_control_flag, //// write 0x0001 or 0xFFFF to Sector control area
                                     // CTRL_FLAG_SIZE);

			lwrite_CtrlFlg(secAddr, &g_control_flag);
			result = TRUE;
            // dly_tsk(1/MSEC);
        }else if(g_FrameNumber >= frmNumInArea)  //// overwrite new data
        {///////////////////////////////////////
            if(frmIndex == 0) {
                /* checking only one learn data in next sector */
                numOnlyOneData = 0;
                // result = ldataCheckFrameOnlyOneData(secAddr,	// almost return TRUE, !should check next Sector
                                                    // &numOnlyOneData,
                                                    // &bankNumOnlyOneData,
                                                    // &secNumOnlyOneData,
                                                    // &frmNumOnlyOneData);
				result = ldataCheckFrameOnlyOneData(secAddr, &numOnlyOneData, &g_learnDataInSection);
				//bankNumOnlyOneData = g_learnDataInSection[i].bankIndex;
				//secNumOnlyOneData = g_learnDataInSection[i].secIndex;
				//frmNumOnlyOneData = g_learnDataInSection[i].frmIndex;
				
				// process only one data 
                if(result) {
                    if(numOnlyOneData == 1) {
                        if(frmNumOnlyOneData>0) {
                            result = ldataMoveOnlyOneDataToTop(bankNumOnlyOneData,
                                                               secNumOnlyOneData,
                                                               frmNumOnlyOneData);
                        } else {
                            g_control_flag = 0xFFFF;
                            // result = miWriteRange(secAddr+CTRL_FLAG_OFFSET,
                                                  // (UB*)&g_control_flag, CTRL_FLAG_SIZE);
							lwrite_CtrlFlg(secAddr, g_control_flag);
							result = TRUE;
                        }
                        
                        if(result) {
                            frmIndex++; // WRITE NEW DATA TO SECOND AREA - NEXT TO ONLY ONE DATA
                            frmAddr += LDATA_FRAME_DATA_SIZE;
                            notifyOnlyOneData = TRUE;
                        }
                    } else if(numOnlyOneData == 19) { //// all next section contains 19 only 1 data
                        g_control_flag = 0xFFFF;
                        // result = miWriteRange(secAddr+CTRL_FLAG_OFFSET,
                                              // (UB*)&g_control_flag, CTRL_FLAG_SIZE);
                        lwrite_CtrlFlg(secAddr, g_control_flag);
						result = TRUE;
                        secAddr += MI_SECTOR_SIZE;
                    } else { //// 1 < numOnlyOneData < 19 and before store to next Section
                        result = miRemoveDataInSector(secAddr);
                    }
                }
                
                if(result) {
                    g_control_flag = 0x0001; //// oldest data
                    if((secAddr+MI_SECTOR_SIZE)<CALC_EBANK_ADDR(g_end_bank_index)) { //// section in current bank
                        // result = miWriteRange(secAddr+MI_SECTOR_SIZE+CTRL_FLAG_OFFSET, //// put 0x0001 to next section - which had only 1 data before
                                              // (UB*)&g_control_flag, CTRL_FLAG_SIZE);
						lwrite_CtrlFlg(secAddr, g_control_flag);
						result = TRUE;
                    }else{ //// section in next bank
                        // result = miWriteRange(CALC_SBANK_ADDR(g_start_bank_index)+CTRL_FLAG_OFFSET, //// put 0x0001 to next section of NEXT BANK - which had only 1 data before
                                              // (UB*)&g_control_flag, CTRL_FLAG_SIZE);
						lwrite_CtrlFlg(secAddr, g_control_flag);
						result = TRUE; 
                    }
                    
                    dly_tsk(1/MSEC);
                }
            }
        } //// end checking only 1 data
        
        updateCurLatestData = FALSE;
        if(result) {
            updateCurLatestData = ldataFindCurLatestFinger(Data->RegRnum, //// SAVE CURRENT LATEST FINGER
															Data->RegYnum,
                                                           &latestBankNum,
                                                           &latestSecNum,
                                                           &latestFrmNum);
            result = TRUE;
            result = lstoreData(frmAddr, Data); //// STORE NEW DATA
            // dly_tsk(1/MSEC);
        }
        
        if(result && updateCurLatestData) {
            result = ldataUpdateCurLatestToOld(latestBankNum,latestSecNum,latestFrmNum);
        }
        
        if(result) {
            if(UpdateLearnInfo(ldataActivedBank, APARTMENT_TYPE)==0) {
                g_FrameNumber++;
                if(g_FrameNumber == (10 * frmNumInArea)) {
                    /* reset to avoid over number UW */
                    g_FrameNumber = frmNumInArea;
                }
            }
            
            if(notifyOnlyOneData) {
                /* add code here to signal event or toggle a flag */
            }
        }
    }
    
#if FWK_LD_SEMAPHORE_ENABLE
    ldataosReleaseSemaphore();
#endif
    
    return (result ? 0 : 1);

    // return ((Data != NULL) ? 0 : 1);
}

/******************************************************************************/
/* Function Name: SearchLearnImg                                              */
/* Description  : This function will search fingers that joins in a room.            */
/* Parameter    : Input: SearchNum - is the registration number that has range*/
/*                       values from 0 to 480.                                */
/*                Output: SearchResult - is a buffer that stores the learning */
/*                        data for specified registration number.             */
/* Return Value : int - return the zero value when search successful, else to */
/*                return the one value.                                       */
/* Remarks      : None                                                        */
/******************************************************************************/
int SearchLearnImg(UH SearchNum, UH* SearchResult[20][3])
{
    volatile BOOL result;
    volatile UW fingerIndex;
    
#if FWK_LD_SEMAPHORE_ENABLE
    ldataosGetSemaphore();
#endif
    
    result = (SearchNum < LDATA_REG_NBR_MAX);
    if(result)
    {
        for(fingerIndex = 0; fingerIndex < LDATA_REG_FIGURE_NBR_MAX; fingerIndex++) {)
			*SearchResult[fingerIndex][0] = InfoLearningBankTable[SearchNum].BankNum[fingerIndex];
			*SearchResult[fingerIndex][1] = InfoLearningBankTable[SearchNum].SectionNum[fingerIndex];
			*SearchResult[fingerIndex][2] = InfoLearningBankTable[SearchNum].FrameNum[fingerIndex];
        }
    }
    
#if FWK_LD_SEMAPHORE_ENABLE
    ldataosReleaseSemaphore();
#endif

    return (result ? 0 : 1);
}

/******************************************************************************/
/* Function Name: UpdateLearnInfo                                             */
/* Description  : This function will update InfoLearnInBankM by searching data*/
/*                in specified Banks.                                         */
/* Parameter    : Input: BankSw - is list of bank bits that is value as below:*/
/*                       BANK7: bit 7.                                        */
/*                       BANK6: bit 6.                                        */
/*                       BANK5: bit 5.                                        */
/*                       BANK4: bit 4.                                        */
/*                       BANK3: bit 3.                                        */
/*                       BANK2: bit 2.                                        */
/*                       BANK1: bit 1.                                        */
/*                Input: Spec - is a kind of structure that will generate to  */
/*                       remap latest learn data on flash memory.             */
/*                       0: For Apartment type(Create InfoLearnInBankM).      */
/*                       1: For Company type (Create InfoLearnInBankC).       */
/* Return Value : int - return the zero value when updates successful, else to*/
/*                return the one value.                                       */
/* Remarks      : None                                                        */
/******************************************************************************/
static int UpdateLearnInfo(UB BankSw, UB Spec)
{
    BOOL result;
    result = ((BankSw>0)&((Spec==APARTMENT_TYPE) || (Spec==COMPANY_TYPE)));
    if(result && (Spec == APARTMENT_TYPE))
    {
		lupdateLearnInfo();
    }
    
    // return (result ? 0 : 1);
	return 0;
}

/******************************************************************************/
/* Function Name: ldataFindCurLatestFinger                                    */
/* Description  : This function will find current latest learn data of finger.*/
/* Parameter    : Input: yNum - is number of finger that needs to find latest */
/*                       learn data on flash memory.                          */
/*                Output: curBankNum - is bank number to store current latest */
/*                        learn data of yNum.                                 */
/*                Output: curSecNum - is sector number to store current latest*/
/*                        learn data of yNum.                                 */
/*                Output: curFrmNum - is frame number to store current latest */
/*                        learn data of yNum.                                 */
/* Return Value : BOOL - return the TRUE value when finds successful, else to */
/*                return the FALSE value.                                     */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL ldataFindCurLatestFinger(UH rNum, UH yNum, UW* curBankNum,
                                     UW* curSecNum, UW* curFrmNum)
{
    BOOL result;
    UW bankIndex, secIndex, frmIndex;
    SvLearnData *learnDataPtr;
	
	learnDataPtr = &g_learnData;
    result =((curBankNum!=NULL) && (curSecNum!=NULL) && (curFrmNum!=NULL));
	if(result)
	{
		result = lcheck_RegRnum(rNum);
	}
	
	if(result)
	{
		result = lcheck_RegYnum(yNum);
	}
	
    if(result == FALSE)
	{
		goto END_FUNC;
	}

	// Search latest finger register of room rNum, finger yNum
	lmap_readMapInfoTable(rNum, yNum, &bankIndex, &secIndex, &frmIndex, 0, 0);
	
	lread_FrameByIndex(bankIndex, secIndex, frmIndex, learnDataPtr);
	result = lcheck_RegStatus(learnDataPtr->RegStatus, LDATA_REGISTERD_STS); // 0xFFFC
	if(result == TRUE)
	{
		// Return latest finger register
		*curBankNum = bankIndex;
		*curSecNum  = secIndex;
		*curFrmNum  = frmIndex;
		goto END_FUNC;
	}

END_FUNC:
    return result;
}

/******************************************************************************/
/* Function Name: ldataUpdateCurLatestToOld                                   */
/* Description  : This function will update current latest learn data to old  */
/*                data on flash memory.                                       */
/* Parameter    : Input: bankIndex - is bank number to store current latest  */
/*                       learn data.                                          */
/* Parameter    : Input: secIndex - is sector number to store current latest */
/*                       learn data.                                          */
/* Parameter    : Input: frmIndex - is frame number to store current latest  */
/*                       learn data.                                          */
/* Return Value : BOOL - return the TRUE value when updates successful, else  */
/*                to return the FALSE value.                                  */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL ldataUpdateCurLatestToOld(UW bankIndex, UW secIndex, UW frmIndex)
{
    UW frmAddr;
    
	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	lwrite_RegStatus(frmAddr, LDATA_NOT_LATEST_STS); //0xFFF8
    return TRUE;
}

/******************************************************************************/
/* Function Name: ldataRemoveAllData                                          */
/* Description  : This function will remove all data in learn data area.      */
/* Parameter    : None                                                        */
/* Return Value : BOOL - return the TRUE value when erases successful, else to*/
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL ldataRemoveAllData(void)
{
    volatile BOOL result;
    volatile UW retryNumber, secAddr, sAddr, size;
    
    size = CALC_AREA_SIZE(g_start_bank_index, g_end_bank_index);
    sAddr = CALC_SBANK_ADDR(g_start_bank_index);
    for(secAddr = 0; secAddr < size; secAddr += MI_SECTOR_SIZE) {
        result = miRemoveDataInSector(sAddr + secAddr);
        if(!result) {
            break;
        }
        // dly_tsk( 50/MSEC );
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: ldataStoreFrameData                                         */
/* Description  : This function will store a learn data into the flash memory.*/
/* Parameter    : Input: frmAddr - is address of frame data in the flash */
/*                       memory.                                              */
/*                Input: lDataPtr - is a buffer that needs to store into the  */
/*                       flash memory.                                        */
/* Return Value : BOOL - return the TRUE value when erases successful, else to*/
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL ldataStoreFrameData(UW frmAddr, SvLearnData *lDataPtr)
{
    BOOL result;
    UW uwSize;
	
    lDataPtr->RegStatus = (UH)LDATA_DUR_REG_STS;
    result = miWriteRange(frmAddr+LDATA_FRAME_STS_OFFSET, (UB*)&lDataPtr->RegStatus, LDATA_REG_STS_SIZE);
    if(result)
	{
        dly_tsk(1/MSEC);
        result = miWriteRange(frmAddr+LDATA_FRAME_IMG1_OFFSET, (UB*)&lDataPtr->RegImg1[0], LDATA_ALL_IMG_SIZE);
	}
    
	if(result)
	{
		dly_tsk(1/MSEC);
		result = miWriteRange(frmAddr+LDATA_FRAME_RNUM_OFFSET, (UB*)&lDataPtr->RegRnum, LDATA_REG_RNUM_SIZE+LDATA_REG_YNUM_SIZE+LDATA_REG_DUMMY_SIZE);
	}
	
	if(result)
	{
		dly_tsk(1/MSEC);
		lDataPtr->RegStatus = (UH)LDATA_REGISTERD_STS;
		result = miWriteRange(frmAddr+LDATA_FRAME_STS_OFFSET, (UB*)&lDataPtr->RegStatus, LDATA_REG_STS_SIZE);
	}
    
    return result;
}

/******************************************************************************/
/* Function Name: ldataCheckFrameOnlyOneData                                  */
/* Description  : This function will check only one learn data in a sector.   */
/* Parameter    : Input: secAddr - is address of a sector that needs to */
/*                       check only one learn data.                           */
/*                Output: number - is number of the only one learn data in    */
/*                        this sector                                         */
/*                Output: bankNum - is current bank that stores first only one*/
/*                        learn data.                                         */
/*                Output: sectorNum - is current sector that stores first only*/
/*                        one learn data.                                     */
/*                Output: frameNum - is frame number that stores first only   */
/*                        one learn data.                                     */
/* Return Value : BOOL - return the TRUE value when checks successful, else to*/
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL ldataCheckFrameOnlyOneData(UW secAddr, UB* number, Info* learnDataInSection[])
{
	UW frmAddr;
	int i;
    SvLearnData *learnDataPtr;
	UH rNum, yNum;
	UW cnt_onlyOneData;
	
	learnDataPtr = &g_learnData;

	cnt_onlyOneData = 0;
	frmAddr = secAddr;
	for(i = 0; i < LDATA_FRAME_NUM_IN_SECTOR; i++)
	{
		frmAddr += sizeof(SvLearnData)*i;
		// Get Learn Data stored in frame address
		lread_Frame(frmAddr, learnDataPtr);
		// Read room, finger bankIndex info
		rNum = learnDataPtr->RegRnum;
        yNum = learnDataPtr->RegYnum;
		// Search .Num in InfoLearnInBankM
		if(InfoLearningBankTable[rNum].Num[yNum] == 1)
		{
			// +1 number of only one data counter
			cnt_onlyOneData++;
		}
		
		// save bankIndex, secIndex, frmIndex, of this section to Info table
		learnDataInSection[i]->bankIndex = InfoLearningBankTable[rNum].BankNum[yNum];
		learnDataInSection[i]->secIndex = InfoLearningBankTable[rNum].SectionNum[yNum];
		learnDataInSection[i]->frmIndex = InfoLearningBankTable[rNum].FrameNum[yNum];
		learnDataInSection[i]->secAddr = secAddr;
		learnDataInSection[i]->frmAddr = frmAddr;
		learnDataInSection[i]->num = InfoLearningBankTable[rNum].Num[yNum];
	}
	
	// save only one data counter
	*number = cnt_onlyOneData;
	
    return (cnt_onlyOneData > 0);
}

/******************************************************************************/
/* Function Name: ldataMoveOnlyOneDataToTop                                   */
/* Description  : This function will move the only one learn data to top of   */
/*                same sector.                                                */
/* Parameter    : bankIndex, secIndex, frmIndex - Current only one data       */
/*                location                                                    */
/* Return Value : BOOL - return the TRUE value when moves successful, else to */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL ldataMoveOnlyOneDataToTop(UB bankIndex, UB secIndex, UB frmIndex)
{
    BOOL result;
    UW secAddr, frmAddr, pre_secAddr;
    SvLearnData *learnDataPtr;
	
	learnDataPtr = &g_learnData;
	result = lread_FrameByIndex(bankIndex, secIndex, frmIndex, learnDataPtr);

	if(result)
	{
		/* Clear previous Section */
		pre_secAddr = lcalc_SectionAddr(bankIndex, secIndex) - MI_SECTOR_SIZE;
		result = miRemoveDataInSector(pre_secAddr);	// clear previous Section
	}
	
	if(result)
	{
		/* Write to 1st frame of previous Section */
		// result = miWriteRange(secAddr, (UB*)&InitlearnData, LDATA_FRAME_DATA_SIZE);
		lwrite_Frame(pre_secAddr, learnDataPtr);
	}
    
    return result;
}


/*
 *
 */

static BOOL lcheck_BankIndex(UW bankIndex)
{
	return (bankIndex >= g_start_bank_index && bankIndex <= g_end_bank_index);
}

static BOOL lcheck_SecIndex(UW secIndex)
{
	return (secIndex >= 0 && secIndex < MI_NUM_SEC_IN_BANK);
}

static BOOL lcheck_FrmIndex(UW frmIndex)
{
	return (frmIndex >= 0 && frmIndex < LDATA_FRAME_NUM_IN_SECTOR);
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

static BOOL foo(UH checked_val, UH expected_val)
{
	UW expected;
	expected = (UW)(expected_val << 24) | (UW)(expected_val << 16) | (UW)(expected_val << 8) | (UW)(expected_val << 0);
	return checked_val == expected;
}

static BOOL lcheck_RegImg1(UH* checked_finger_vein)
{
	// Not finished yet
	foo(checked_finger_vein[0], 0x5555);
	return TRUE;
}
static BOOL lcheck_SvLearnData(SvLearnData* ld ,UH data)
{	
	BOOL result;
	result = FALSE;
	result = lcheck_RegStatus(ld->RegStatus, 0xFFFC); // 0xFFFC : Registered (Newest)
	if(result == TRUE) 
	{
		result = lcheck_RegRnum(ld->RegRnum);
	}
	
	if(result == TRUE) 
	{
		result = lcheck_RegYnum(ld->RegYnum);
	}
	
	// if(result == TRUE) 
	// {
		// result = lcheck_RegImg1(ld->RegImg1, data);
	// }
	
	return result;
}

static BOOL lcheck_ID(UW code)
{
	return (CODE_MIN <= code && code <= CODE_MAX);
}

static UW lcalc_BankAddr(UW bankIndex)
{
	UW bankAddr;
	BOOL ret = FALSE;
	bankAddr = 0;

	ret = lcheck_BankIndex(bankIndex);
	if(ret == TRUE)
	{
		bankAddr = MI_FLASH_BASIC_ADDR + (bankIndex*MI_BANK_SIZE);
	}

	return bankAddr;
}

static UW lcalc_SectionAddr(UW bankIndex, UW secIndex)
{
	UW bankAddr, secAddr;
	BOOL ret = FALSE;
	secAddr = 0;

	ret = lcheck_BankIndex(bankIndex);
	if(ret == TRUE)
	{
		ret = lcheck_SecIndex(secIndex);
	}

	if(ret == TRUE)
	{
		bankAddr = MI_FLASH_BASIC_ADDR + (bankIndex*MI_BANK_SIZE);
		secAddr = bankAddr + secIndex*MI_SECTOR_SIZE;
	}
	
	return secAddr;	
}

static UW lcalc_FrameAddr(UW bankIndex, UW secIndex, UW frmIndex)
{
	UW bankAddr, secAddr, frmAddr;
	BOOL ret = FALSE;
	frmAddr = 0;

	ret = lcheck_BankIndex(bankIndex);
	if(ret == TRUE)
	{
		ret = lcheck_SecIndex(secIndex);
	}

	if(ret == TRUE)
	{
		ret = lcheck_FrmIndex(frmIndex);
	}

	if(ret == TRUE)
	{
		bankAddr = MI_FLASH_BASIC_ADDR + (bankIndex*MI_BANK_SIZE);
		secAddr = bankAddr + secIndex*MI_SECTOR_SIZE;
		frmAddr = secAddr + frmIndex*sizeof(SvLearnData);
	}

	return frmAddr;	
}

static BOOL lread_HWord(UW uwFp, UH *puhBp)
{
	ER errCode;
	
	errCode = FlRead(uwFp, puhBp, 1);
	if(errCode != E_OK) {
		return FALSE;
	}
	return TRUE;
}

static UW lwrite_HWord(UW uwFp, UH uhData)
{
	UW uwSize;
	
	uwSize = FlWrite(uwFp, (UH*)&uhData, 1);
	return uwSize;
}


static UW lread_Buffer(UW uwFp, UH *puhBp, UW n)
{
	ER errCode;
	
	errCode = FlRead(uwFp, puhBp, n/sizeof(UH));
	if(errCode != E_OK) {
		return FALSE;
	}
	return TRUE;
}

static UW lwrite_Buffer(UW uwFp, UH *puhBp, UW n)
{
	UW uwSize;
	
	uwSize = FlWrite(uwFp, puhBp, n/sizeof(UH));
	return uwSize;
}

static BOOL lread_Frame(UW frmAddr, SvLearnData *learnDataPtr)
{
	return lread_Buffer(frmAddr, (UH*)learnDataPtr, sizeof(SvLearnData));
}

static void lwrite_Frame(UW frmAddr, SvLearnData* learnDataPtr)
{
	lwriteBuffer(frmAddr, (UH*)learnDataPtr, sizeof(SvLearnData));
}

static void lremove_Frame(UW frmAddr)
{
	UW n;
	UH val;
	UH	*puhAddr;

	puhAddr = (UH*)frmAddr;
	n = sizeof(SvLearnData)/sizeof(UH);
	
	while( n ) {
		n--;
		lwriteHWord((UW)puhAddr, 0xFFFF);
		puhAddr++;
	}
}

static BOOL lremove_Section(UW secAddr)
{
    ER errCode;
	errCode = FlErase(secAddr);	// reset to 0xFFFF
    return (errCode == E_OK);
}

static BOOL lremove_All(void)
{
	UW secAddr;
	UW size;
	BOOL ret;
	
	secAddr = lcalc_SectionAddr(g_start_bank_index, 0);
	size = CALC_AREA_SIZE(g_start_bank_index, g_end_bank_index);
	
	while( secAddr < size ) {
		ret = lremove_Section(secAddr);
		if(ret == FALSE)
			break;
		
		secAddr += MI_SECTOR_SIZE;
	}
	return ret;
}

static BOOL lread_RegStatus(UW frmAddr, UH *RegStatus)
{
	return lread_HWord(frmAddr + LDATA_FRAME_STS_OFFSET, RegStatus);
}

static void lwrite_RegStatus(UW frmAddr, UH RegStatus)
{
	lwriteHWord(frmAddr + LDATA_FRAME_STS_OFFSET, RegStatus);
}

static BOOL lread_RegRnum(UW frmAddr, UH *RegRnum)
{
	return lread_HWord(frmAddr + LDATA_FRAME_RNUM_OFFSET, RegRnum);
}

static void lwrite_RegRnum(UW frmAddr, UH RegRnum)
{
	lwriteHWord(frmAddr + LDATA_FRAME_RNUM_OFFSET, RegRnum);
}

static BOOL lread_RegYnum(UW frmAddr, UH *RegYnum)
{
	return lread_HWord(frmAddr + LDATA_FRAME_YNUM_OFFSET, RegYnum);
}

static void lwrite_RegYnum(UW frmAddr, UH RegYnum)
{
	lwriteHWord(frmAddr + LDATA_FRAME_YNUM_OFFSET, RegYnum);
}

static BOOL lread_RegID(UW frmAddr, UH *RegID)
{
	return lread_HWord(frmAddr + LDATA_FRAME_ID_OFFSET, RegID);
}

static void lwrite_RegID(UW frmAddr, UH RegID)
{
	lwriteHWord(frmAddr + LDATA_FRAME_ID_OFFSET, RegID);
}

static void lwrite_Dummy1(UW frmAddr, UH* Dummy1)
{
	lwriteBuffer(frmAddr + LDATA_FRAME_DUMMY_OFFSET, (UH*)Dummy1, LDATA_REG_DUMMY_SIZE);
}

static void lwrite_RegImg(UW frmAddr, UH* RegImg)
{
	lwriteBuffer(frmAddr + LDATA_FRAME_IMG1_OFFSET, (UH*)RegImg, LDATA_NORM_IMAGE_SIZE);
}

static void lwrite_MiniImg(UW frmAddr, UH* MiniImg)
{
	lwriteBuffer(frmAddr + LDATA_FRAME_MINI_IMG_OFFSET, (UH*)MiniImg, LDATA_ALL_MINI_IMAGE_SIZE);
}

static BOOL lread_CtrlFlg(UW secAddr, UH* ctrl_flg)
{
	return lread_HWord(secAddr + CTRL_FLAG_OFFSET, ctrl_flg);
}

static void lwrite_CtrlFlg(UW secAddr, UH ctrl_flg)
{
	lwriteHWord(secAddr + CTRL_FLAG_OFFSET, ctrl_flg);
}

static BOOL lread_FrameByIndex(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData *learnDataPtr)
{
	BOOL result;
	UW frmAddr;

	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	result = lread_Frame(frmAddr, learnDataPtr);
	return result;
}

static BOOL lwrite_FrameByIndex(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData *learnDataPtr)
{
	BOOL result;
	UW frmAddr;

	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	result = lwrite_Frame(frmAddr, learnDataPtr);
	return result;
}

static void lremove_FrameByIndex(UW bankIndex, UW secIndex, UW frmIndex)
{
	UH val = 0xFFFF;
	UW frmAddr;

	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	lremove_Frame(frmAddr);
}

static BOOL lremove_SectionByIndex(UW bankIndex, UW secIndex)
{
	BOOL result;
	UW secAddr;
	
	secAddr = lcalc_SectionAddr(bankIndex, secIndex);
	lremove_Section(secAddr);
	
	return result;
}

static void lmove_FrameByIndex(UW bankIndex1, UW secIndex1, UW frmIndex1, UW bankIndex2, UW secIndex2, UW frmIndex2)
{
	SvLearnData* learnDataPtr;
	learnDataPtr = &g_learnData;
	lread_FrameByIndex(bankIndex1, secIndex1, frmIndex1, learnDataPtr);
	lwrite_FrameByIndex(bankIndex2, secIndex2, frmIndex2, learnDataPtr);
	lremove_FrameByIndex(bankIndex1, secIndex1, frmIndex1);
}

// static UW lcalc_CtrlFlgAddr(UW bankIndex, UW secIndex)
// {
	// UW secAddr, ctrl_addr;
	// BOOL ret = FALSE;

	// ctrl_addr = 0;
	// ret = lcheck_BankIndex(bankIndex);
	// if(ret == TRUE)
	// {
		// ret = lcheck_SecIndex(secIndex);
	// }

	// if(ret == TRUE)
	// {
		// secAddr = lcalc_SectionAddr(bankIndex, secIndex);
		// ctrl_addr = secAddr + CTRL_FLAG_OFFSET;
	// }
	// return ctrl_addr;
// }

static UH lread_CtrlFlgByIndex(UW bankIndex, UW secIndex)
{
	UW secAddr;
	UH ctrl_flg = 0;
	secAddr = lcalc_SectionAddr(bankIndex, secIndex);
	lread_CtrlFlg(secAddr, &ctrl_flg);

	return ctrl_flg;
}

/*
 * ctrl_flg: 0x0001 - the oldest data, 0xFFFF - old data
 */
static void lwrite_CtrlFlgByIndex(UW bankIndex, UW secIndex, UH ctrl_flg)
{
	UW secAddr = 0; 	// 32-bit address
	secAddr = lcalc_SectionAddr(bankIndex, secIndex);
	lwrite_CtrlFlg(secAddr, ctrl_flg);
}

// static void lset_OldestSection(UW bankIndex, UW secIndex)
// {
	// lwrite_CtrlFlgByIndex(bankIndex, secIndex, 0x0001);
// }

// static void lclear_OldestSection(UW bankIndex, UW secIndex)
// {
	// lwrite_CtrlFlgByIndex(bankIndex, secIndex, 0xFFFF);
// }

static void lmap_writeMapInfoTable(UH rNum, UH yNum, UW bankIndex, UW secIndex, UW frmIndex, UW num, UH id)
{
	InfoLearningBankTable[rNum].BankNum[yNum]    = bankIndex;
	InfoLearningBankTable[rNum].SectionNum[yNum] = secIndex;
	InfoLearningBankTable[rNum].FrameNum[yNum]   = frmIndex;
	
	if(num != 0xFFFF)
		InfoLearningBankTable[rNum].Num[yNum]        = num;
	
	if(id != 0xFFFF)
		InfoLearningBankTable[rNum].ID[yNum]         = id;
}

static void lmap_readMapInfoTable(UH rNum, UH yNum, UW *bankIndex, UW *secIndex, UW *frmIndex, UW *num, UH *id)
{
	*bankIndex = InfoLearningBankTable[rNum].BankNum[yNum];
	*secIndex  = InfoLearningBankTable[rNum].SectionNum[yNum];
	*frmIndex  = InfoLearningBankTable[rNum].FrameNum[yNum];
	*num       = InfoLearningBankTable[rNum].FrameNum[yNum];
	*id        = InfoLearningBankTable[rNum].ID[yNum];
}

static BOOL lmap_checkIsNewID(UW code)
{
	int i;
	BOOL ret;
	
	for(i=0; i < g_total_empl; i++)
	{
		if(code == g_flash_data_info[i].code)
		{
			return FALSE; // already in flash
		}
	}
	return TRUE;
}

static BOOL lmap_getIndexFromIDList(UW code, UH *index)
{
	int i;
	BOOL ret;
	ret = lcheck_ID(code);
	if(FALSE)
		return FALSE;
	
	for(i=0; i < g_total_empl; i++)
	{
		if(code == g_flash_data_info[i].code)
		{
			*index = i;
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL lmap_getCountFromIDList(UH code, UW *count)
{
	UW index;
	BOOL ret;
	
	*count = 0;
	index = 0;
	ret = lcheck_ID(code);
	if(ret == FALSE)
	{
		return FALSE;
	}
	
	ret = lmap_getIndexFromIDList(code, &index);
	if(ret == TRUE)
	{
		*count = g_flash_data_info[index].cnt;
		return TRUE;
	}
	
	return FALSE;
}

static BOOL lmap_updateMapInfoTable_location(UW bankIndex, UW secIndex, UW frmIndex)
{
	BOOL ret;
	UH regStatus, rNum, yNum, id;
	UW frmAddr;
	
	ret = FALSE;
	id = 0xFFFF;
	regStatus = 0xFFFF;
	
	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	lread_RegStatus(frmAddr, &regStatus);
	ret = lcheck_RegStatus(regStatus, LDATA_REGISTERD_STS); // 0xFFFC
	if(ret == TRUE)
	{
		// Save latest registration to Mapping info table
		lread_RegRnum(frmAddr, &rNum);
		lread_RegYnum(frmAddr, &yNum);
		lread_RegID(frmAddr, &id);
		lmap_writeMapInfoTable(rNum, yNum, bankIndex, secIndex, frmIndex, 0xFFFF, id);
	}
	
	return ret;
}

static BOOL lmap_updateMapInfoTable_num(UH rNum, UH yNum)
{
	UH id;
	BOOL ret;
	UW cnt;
	id = InfoLearningBankTable[rNum].ID[yNum];
	ret = lmap_getCountFromIDList(id, &cnt);
	if(ret == FALSE)
		return FALSE;
	
	InfoLearningBankTable[rNum].Num[yNum] = cnt;
	return TRUE;
}

static BOOL lmap_updateIDList(UW bankIndex, UW secIndex, UW frmIndex)
{
	BOOL ret;
	UH regStatus, id;
	UH index;
	UW frmAddr;
	
	ret = FALSE;
	id = 0xFFFF;
	regStatus = 0xFFFF;
	
	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	// Read status
	lread_RegStatus(frmAddr, &regStatus);

	// Check status = 0xFFFC or 0xFFF8
	ret = lcheck_RegStatus(regStatus, LDATA_REGISTERD_STS);
	ret |= lcheck_RegStatus(regStatus, LDATA_NOT_LATEST_STS));
	if(ret == FALSE)
	{
		return FALSE;
	}
	
	// Read ID
	lread_RegID(frmAddr, &id);
	ret = lcheck_ID(id);
	if(ret == FALSE)
	{
		return FALSE;
	}
	
	ret = lmap_checkIsNewID(id);
	if(ret == TRUE)
	{
		// New ID
		g_total_empl++;
		index = g_total_empl-1;		// latest index
		g_flash_data_info[index].code = id;	// add new ID to list
		g_flash_data_info[index].cnt = 1;		// start new counter
	}else
	{
		// The same ID
		ret = lmap_getIndexFromIDList(id, &index);
		if(ret == TRUE)
		{
			g_flash_data_info[index].cnt++;	// increase counter
		}
	}
	
	return TRUE;
}

static void lupdateLearnInfo(void)
{
	UW bankIndex, secIndex, frmIndex;
	UH rNum, yNum;
	BOOL ret;
	
	// Scan Flash
	for(bankIndex=g_start_bank_index; bankIndex<=g_end_bank_index; bankIndex++)
	{
		for(secIndex=0; secIndex<MI_NUM_SEC_IN_BANK; secIndex++)
		{
			for(frmIndex=0; frmIndex<LDATA_FRAME_NUM_IN_SECTOR; frmIndex++)
			{
				// Update .BankNum, .SectionNum, .FrameNum, .ID
				lmap_updateMapInfoTable_location(bankIndex, secIndex, frmIndex);
				// Update .code, .cnt
				ret = lmap_updateIDList(bankIndex, secIndex, frmIndex);
				if(ret == FALSE)
					continue;
			}
		}
	}
	
	// Scan Mapping info table
	// Update .Num
	for(rNum=0; rNum<=LDATA_REG_NBR_MAX rNum++)
	{
		for(yNum=0; yNum<LDATA_REG_FIGURE_NBR_MAX; yNum++)
		{
			ret = lmap_updateMapInfoTable_num(rNum, yNum);
			if(ret == FALSE)
				continue;
		}
	}
}

static void lshiftOldestSectionFlag(void)
{
	// Set the current oldest Section to normal Section
	lwrite_CtrlFlgByIndex(g_oldest_bank_index, g_oldest_section_index, 0xFFFF);
	
	// Move to next Section
	g_oldest_bank_index++;
	g_oldest_section_index++;
	
	if(g_oldest_bank_index > g_end_bank_index)
		g_oldest_bank_index = g_start_bank_index;
	
	if(g_oldest_section_index >= MI_NUM_SEC_IN_BANK)
		g_oldest_section_index = 0;
	
	// Set next Section to the oldest Section
	lwrite_CtrlFlgByIndex(g_oldest_bank_index, g_oldest_section_index, 0x0001);
}

static void lupdate_TheLatestFrame(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData* new_learnDataPtr)
{
	// UW frmAddr;
	// UH rNum, yNum;

	// rNum = new_learnDataPtr->RegRnum;
	// yNum = new_learnDataPtr->RegYnum;
	// InfoLearningBankTable[rNum].BankNum[yNum]    = bankIndex;
	// InfoLearningBankTable[rNum].SectionNum[yNum] = secIndex;
	// InfoLearningBankTable[rNum].FrameNum[yNum]   = frmIndex;

}
/*
 * Call after adding new learn data, before updateDataInfo
 */
static void lupdate_NotTheLatestFrame(SvLearnData* new_learnDataPtr)
{
	UW bankIndex, secIndex, frmIndex;
	UW frmAddr;
	UH rNum, yNum;
	UH id;
	UW cnt;
	BOOL ret;
	SvLearnData* old_learnDataPtr;

	bankIndex = secIndex = frmIndex = 0;
	old_learnDataPtr = &g_learnData;
	
	ret = lcheck_RegStatus(new_learnDataPtr->RegStatus, LDATA_REGISTERD_STS); // 0xFFFC
	if(ret == TRUE)
	{
		// Get new learn data info
		rNum = new_learnDataPtr->RegRnum;
		yNum = new_learnDataPtr->RegYnum;
		// Get location in flash of latest data from Mapping info
		lmap_readMapInfoTable(rNum, yNum, &bankIndex, &secIndex, &frmIndex, &cnt, &id);
		// Change current latest data in flash to old data
		frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
		lwrite_RegStatus(frmAddr, LDATA_NOT_LATEST_STS); // 0xFFF8	
	}
}

static BOOL lstoreData(UW frmAddr, SvLearnData *learnDataPtr)
{
    BOOL result;
    UW uwSize;
	
    learnDataPtr->RegStatus = (UH)LDATA_DUR_REG_STS;
	lwrite_RegStatus(frmAddr, learnDataPtr->RegStatus);
	
	lwrite_RegImg(frmAddr, learnDataPtr->RegImg1);
	lwrite_RegImg(frmAddr, learnDataPtr->RegImg2);
	lwrite_MiniImg(frmAddr, learnDataPtr->MiniImg);

	lwrite_RegRnum(frmAddr, learnDataPtr->RegRnum);
	lwrite_RegYnum(frmAddr, learnDataPtr->RegYnum);
	lwrite_RegID(frmAddr, learnDataPtr->RegID);
	lwrite_Dummy1(frmAddr, learnDataPtr->Dummy1);
		
	learnDataPtr->RegStatus = (UH)LDATA_REGISTERD_STS;
    lwrite_RegStatus(frmAddr, learnDataPtr->RegStatus);
    return TRUE;
}

/*
 * Call when g_frmIndex = 0
 */
static BOOL lstoreToNewSection(SvLearnData* learnDataPtr)
{
	UW bankIndex, secIndex;
	UW frmAddr;
	UW cur_secAddr, next_secAddr;
	UH cur_ctrl_flg, next_ctrl_flg;
	// BOOL ret;
	
	// ret = FALSE;
	if(g_frmIndex != 0)
		return FALSE;
	
	bankIndex = g_bankIndex;
	secIndex = g_secIndex;
	cur_ctrl_flg = next_ctrl_flg =0xFFFF;
	
	//check control flag of current Section
	cur_secAddr = lcalc_SectionAddr(bankIndex, secIndex);
	next_secAddr = lcalc_SectionAddr(bankIndex, secIndex+1);
	// lread_CtrlFlg(cur_secAddr, &cur_ctrl_flg);
	lread_CtrlFlg(next_secAddr, &next_ctrl_flg);

	switch(next_ctrl_flg)
	{
		case 0x0001:	// The oldest Section
		// Shift control flag to next Section
		lshiftOldestSectionFlag();
		// Clear all frame in current Section
		// ret = miRemoveDataInSector(cur_secAddr);
		lremove_Section(cur_secAddr);
		// Store new data to 1st frame of current Section
		frmAddr = lcalc_FrameAddr(bankIndex, secIndex, 0);
		lstoreData(frmAddr, learnDataPtr);
		break;
		case 0xFFFF:
		// Skip
		break;
		default:
		break;
	}
	
	return TRUE;
}

/*
 * Call when g_bankIndex = g_start_bank_index && g_secIndex = g_frmIndex = 0
 */
static BOOL lstore1stFrame(SvLearnData* learnDataPtr)
{
	UW bankIndex, secIndex;
	UW frmAddr;
	UW cur_secAddr, next_secAddr;
	UH cur_ctrl_flg, next_ctrl_flg;
	BOOL ret;
	
	if(g_bankIndex == g_start_bank_index)
	{
		if(g_add_first_frame == 1)
		{
			// Clear flag
			g_add_first_frame = 0;
			// Set begin of oldest Section
			g_oldest_bank_index = g_start_bank_index;
			g_oldest_section_index = 0;
			// Remove all data
			// ldataRemoveAllData();
			ret = lremove_All();
			// Store data
			frmAddr = lcalc_FrameAddr(g_start_bank_index, 0, 0);	// store 1st Frame
			lstoreData(frmAddr, learnDataPtr);
			// Initialize Mapping Info, ID List
		}
	}
	
	return ret;
}

static void lcalc_nextSection(UW cur_bankIndex, UW cur_secIndex, UW *next_bankIndex, UW *next_secIndex)
{
	
	*next_secIndex = cur_secIndex+1;
	if(*next_secIndex >= MI_NUM_SEC_IN_BANK)
	{
		*next_secIndex = 0;
		*next_bankIndex = cur_bankIndex+1;
		if(*next_bankIndex > g_end_bank_index)
			*next_bankIndex = g_start_bank_index;
	}
}

static void lupdate_NextFrameLocation(UW bankIndex, UW secIndex, UW frmIndex)
{
	g_bankIndex = bankIndex;
	g_secIndex = secIndex;
	g_frmIndex = frmIndex;
	lcalc_nextSection(bankIndex, secIndex, &g_next_bankIndex, &g_next_secIndex);
}

/*
 * Return TRUE and num is number of only-one data.
 * Otherwise return FALSE and num=0
 */
static BOOL lcheck_OnlyOneData(UW cur_bankIndex, UW cur_secIndex, UW *num)
{
	UW next_bankIndex, next_secIndex, frmIndex;
	UW frmAddr;
	BOOL ret;
	UH id;
	UW total, cnt;

	total = 0;
	lcalc_nextSection(cur_bankIndex, cur_secIndex, &next_bankIndex, &next_secIndex);
	memset(g_movedDataLocation, 0, sizeof(g_movedDataLocation));
	for(frmIndex=0; frmIndex<LDATA_FRAME_NUM_IN_SECTOR; frmIndex++)
	{
		frmAddr = lcalc_FrameAddr(next_bankIndex, next_secIndex, frmIndex);
		lread_RegID(frmAddr, &id);
		cnt = 0;
		ret = lmap_getCountFromIDList(id, &cnt);
		if(ret == FALSE)
			continue;
		
		if(cnt == 1)
		{
			total++;
			g_movedDataLocation[frmIndex] = 1;	// mark this index has only-one data
		}
	}
	
	if(total > 0)
	{
		*num = total;
		ret = TRUE;
	}

	return ret;
}

/*
 * Before: call lcheck_OnlyOneData() to get num
 * After: call lstoreData(), update ID List, update mapping info
 * 
 */
static BOOL lsave_OnlyOneData(UW cur_bankIndex, UW cur_secIndex, UW num)
{
	UW next_bankIndex, next_secIndex, next_frmIndex, cur_frmIndex;
	UW frmAddr;
	BOOL ret;

	cur_frmIndex = 0;
	lcalc_nextSection(cur_bankIndex, cur_secIndex, &next_bankIndex, &next_secIndex);
	// Shift control flag
	lremove_SectionByIndex(cur_bankIndex, cur_secIndex);	// Remove data in current Section
	// Move only one data
	for(next_frmIndex=0; next_frmIndex<LDATA_FRAME_NUM_IN_SECTOR; next_frmIndex++)
	{
		if(g_movedDataLocation[next_frmIndex] == 1)
		{
			lmove_FrameByIndex(next_bankIndex, next_secIndex, next_frmIndex, cur_bankIndex, cur_secIndex, cur_frmIndex);
			cur_frmIndex++;
		}
	}
	
	// Update current location
	if(num == LDATA_FRAME_NUM_IN_SECTOR)
	{
		// 19 only-one data
		lupdate_NextFrameLocation(next_bankIndex, next_secIndex, 0);
	}
	else{
		lupdate_NextFrameLocation(cur_bankIndex, cur_secIndex, cur_frmIndex);
	}
	// // Store new data
	// frmAddr = lcalc_FrameAddr(g_bankIndex, g_secIndex, g_frmIndex);
	// lstoreData(frmAddr, learnDataPtr);
	return (cur_frmIndex > 0);
}


#endif /* FWK_CFG_LEARN_DATA_ENABLE */
/*************************** The End ******************************************/
