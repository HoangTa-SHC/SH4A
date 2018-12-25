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
#define LDATA_START_ADDR(sbank)         (MI_FLASH_BASIC_ADDR+(sbank*MI_BANK_SIZE))
#define LDATA_AREA_SIZE(sbank, ebank)   ((ebank-sbank+1)*MI_BANK_SIZE)
#define LDATA_END_ADDR(ebank)           (MI_FLASH_BASIC_ADDR+((ebank+1)*MI_BANK_SIZE)-1)

/* This is offset ring value in each sector */
#define LDATA_RING_BUF_VALUE            0x0001
#define LDATA_RING_VALUE_SIZE           (sizeof(UH))
#define LDATA_RING_VALUE_OFFSET         (MI_SECTOR_SIZE-(sizeof(UH)))

/* number of frame in learning data area */
#define LDATA_FRAME_NUM_IN_LAREA        ((LDATA_AREA_SIZE(ldataSBank, ldataEBank)/MI_SECTOR_SIZE) * LDATA_FRAME_NUM_IN_SECTOR)

#define LDATA_ALL_IMG_SIZE              ((2*LDATA_NORM_IMAGE_SIZE) + (LDATA_MINI_IMAGE_NUM*LDATA_MINI_IMAGE_SIZE))
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
static UW lcalcBankAddr(UW bankIndex);
static UW lcalcSectionAddr(UW bankIndex, UW secIndex);
static UW lcalcFrameAddr(UW bankIndex, UW secIndex, UW frmIndex);
static UW lcalc_FrameAddr(UW bankIndex, UW secIndex, UW frmIndex);
static BOOL lread_FrameByAddr(UW frmAddr, SvLearnData *learnDataPtr);
static void lwrite_FrameByAddr(UW frmAddr, SvLearnData* learnDataPtr);
static void lwrite_RegStatus(UW frmAddr, UH RegStatus);
static void lwrite_RegRnum(UW frmAddr, UH RegRnum);
static void lwrite_RegYnum(UW frmAddr, UH RegYnum);
static void lwrite_Dummy1(UW frmAddr, UH* Dummy1);
static void lwrite_RegImg(UW frmAddr, UH* RegImg);
static void lwrite_MiniImg(UW frmAddr, UH* MiniImg);
static BOOL lread_FrameByIndex(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData *learnDataPtr);
static UW lcalc_CtrlFlgAddr(UW bankIndex, UW secIndex);
static volatile UH lread_CtrlFlgByIndex(UW bankIndex, UW secIndex);
static void lwrite_CtrlFlg(UW secAddr, UH ctrl_flg);
static void lwrite_CtrlFlgByIndex(UW bankIndex, UW secIndex, UH value);
static void lset_OldestSection(UW bankIndex, UW secIndex);
static void lclear_OldestSection(UW bankIndex, UW secIndex);
/******************************************************************************/
/*************************** Local Constant Variables *************************/
/******************************************************************************/
static volatile SvLearnData InitlearnData = {
    LDATA_NOT_YET_STS, /* not yet */
    0,                 /* room 0 */
    1                  /* Registration figure number */
};

/******************************************************************************/
/*************************** Local Variables **********************************/
/******************************************************************************/
static volatile UB ldataSBank, ldataEBank;
static UB g_bank_num_max;
static volatile UB ldataActivedBank;
static volatile UH ldataRingBufferValue;
static volatile UW ldataFrameNumber;
static volatile InfoLearnInBankM InfoLearningBankTable[LDATA_REG_NBR_MAX]; //// 480
static volatile UH roomNumList[] = {
    LDATA_REG_NBR_MAX, //// 480
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX,
    LDATA_REG_NBR_MAX
};
static UW g_bankIndex=0, g_secIndex=0, g_frmIndex=0;
static UW g_bank_oldest_index=0, g_section_oldest_index=0;
static UH g_ctrl_flg_oldest=0;
static SvLearnData g_learnData;
static Info g_learnDataInSection[LDATA_FRAME_NUM_IN_SECTOR];



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
        ldataSBank = 0;
        ldataEBank = 0;

		for(bankIndex = 1; bankIndex < BANK_MAX_NUM; bankIndex++){
			if(ldataSBank == 0){
				if(BankSw & (UB)LDATA_BIT(bankIndex))	// b00000001 << 0 = 0x01, b00000001 << 1 = b00000010, b00000001 << 7 = b10000000
				{
					ldataSBank = bankIndex;	// 1:7
					break;
				}
			}
		}
		
		for(ldataEBank = bankIndex; bankIndex < BANK_MAX_NUM; bankIndex++)
		{
			
			if(!(BankSw & (UB)LDATA_BIT(bankIndex)))
			{
				break;
			}
			else
				ldataEBank = bankIndex;	// 1:7
		}		
    }
	
	g_bank_num_max = ldataEBank - ldataSBank + 1;
	result = TRUE;
    
    if(result)
    {
        result = ldataRemoveAllData();
        if(result) {
            ldataRingBufferValue = 0x0001;
#if FWK_LD_SEMAPHORE_ENABLE
            ldataosInit();
#endif
            ldataActivedBank = BankSw;
            ldataFrameNumber = 0;
            memset(&InfoLearningBankTable[0],
                   0, LDATA_REG_NBR_MAX*sizeof(InfoLearnInBankM)); //// 480*
        }
    }
    
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
        memset(&InfoLearningBankTable[0], 0xFF, LDATA_REG_NBR_MAX*sizeof(InfoLearnInBankM));
        /* Find location of ring buffer value */
        for(bankIndex=0; bankIndex<BANK_MAX_NUM; bankIndex++) {
            if((BankSw&LDATA_BIT(bankIndex))!=0) {
                for(secIndex=0; secIndex<MI_NUM_SEC_IN_BANK; secIndex++) {
					secAddr = lcalcSectionAddr(bankIndex, secIndex);
                    result = miReadRange(secAddr+LDATA_RING_VALUE_OFFSET, (UB*)fingerInfo, sizeof(UH));
                    if(result && (fingerInfo[0]==LDATA_RING_BUF_VALUE)) { //// 0x0001
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
							frmAddr = lcalcFrameAddr(bankIndex, secIndex, frmIndex);
                            result = miReadRange(frmAddr, (UB*)fingerInfo, sizeof(fingerInfo));
                            if(result)
                            {
                                if((fingerInfo[0]!=0xFFFF)&&(fingerInfo[1]!=0xFFFF)) { //// frames storing data
                                    rNum = learnDataPtr->RegRnum;
                                    yNum = learnDataPtr->RegYnum;
                                    if(learnDataPtr->RegStatus==LDATA_REGISTERD_STS) { //// 0xFFFC new registerd
                                        InfoLearningBankTable[rNum].Num[yNum]++; //// update all data when adding new data
										lupdate_NextFrameLocation(rNum, yNum, bankIndex, secIndex, frmIndex, InfoLearningBankTable[rNum].Num[yNum]);
                                        roomNumList[yNum] = rNum;
                                    }else if(learnDataPtr->RegStatus==LDATA_NOT_LATEST_STS) { //// 0xFFF8 old registerd
                                        InfoLearningBankTable[rNum].Num[yNum]++;
										lupdate_NextFrameLocation(rNum, yNum, bankIndex, secIndex, frmIndex, InfoLearningBankTable[rNum].Num[yNum]);
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
                            bankIndex = ldataSBank-1;
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
		fast_test(&ldataFrameNumber);
#endif

	/* Check valid of input parameters */
    result = (Data != NULL);
    if(result) {
        result = ((Data->RegRnum<LDATA_REG_NBR_MAX)&&(Data->RegYnum<LDATA_REG_FIGURE_NBR_MAX));
    }
    if(result)
    {
        notifyOnlyOneData = FALSE;
        frmNumInArea = LDATA_FRAME_NUM_IN_LAREA; //// number of frame in learning data area 
        frmOffsetInArea = ldataFrameNumber % frmNumInArea; //// need convert bankIndex ldataFrameNumber = ldataFrameNumber-1
        
        /* calculate sector address */ //// BUG: secAddr, frmIndex, frmAddr calc wrong
        secAddr = LDATA_START_ADDR(ldataSBank);
        secAddr += (frmOffsetInArea/LDATA_FRAME_NUM_IN_SECTOR)*MI_SECTOR_SIZE;
        frmIndex = (frmOffsetInArea % LDATA_FRAME_NUM_IN_SECTOR); //// number frame in a sector (19)
        frmAddr = secAddr + (frmIndex * LDATA_FRAME_DATA_SIZE);
        
        if(ldataFrameNumber <= 0) { //// write first data
            result = miWriteInSector(secAddr,
                                     LDATA_RING_VALUE_OFFSET,
                                     (UB*)&ldataRingBufferValue, //// write 0x0001 or 0xFFFF to Sector control area
                                     LDATA_RING_VALUE_SIZE);
			lwrite_CtrlFlg(secAddr, ldataRingBufferValue);
			result = TRUE;
            dly_tsk(1/MSEC);
        }else if(ldataFrameNumber >= frmNumInArea)  //// overwrite new data
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
                            ldataRingBufferValue = 0xFFFF;
                            // result = miWriteRange(secAddr+LDATA_RING_VALUE_OFFSET,
                                                  // (UB*)&ldataRingBufferValue, LDATA_RING_VALUE_SIZE);
							lwrite_CtrlFlg(secAddr, ldataRingBufferValue);
							result = TRUE;
                        }
                        
                        if(result) {
                            frmIndex++; // WRITE NEW DATA TO SECOND AREA - NEXT TO ONLY ONE DATA
                            frmAddr += LDATA_FRAME_DATA_SIZE;
                            notifyOnlyOneData = TRUE;
                        }
                    } else if(numOnlyOneData == 19) { //// all next section contains 19 only 1 data
                        ldataRingBufferValue = 0xFFFF;
                        // result = miWriteRange(secAddr+LDATA_RING_VALUE_OFFSET,
                                              // (UB*)&ldataRingBufferValue, LDATA_RING_VALUE_SIZE);
                        lwrite_CtrlFlg(secAddr, ldataRingBufferValue);
						result = TRUE;
                        secAddr += MI_SECTOR_SIZE;
                    } else { //// 1 < numOnlyOneData < 19 and before store to next Section
                        result = miRemoveDataInSector(secAddr);
                    }
                }
                
                if(result) {
                    ldataRingBufferValue = 0x0001; //// oldest data
                    if((secAddr+MI_SECTOR_SIZE)<LDATA_END_ADDR(ldataEBank)) { //// section in current bank
                        // result = miWriteRange(secAddr+MI_SECTOR_SIZE+LDATA_RING_VALUE_OFFSET, //// put 0x0001 to next section - which had only 1 data before
                                              // (UB*)&ldataRingBufferValue, LDATA_RING_VALUE_SIZE);
						lwrite_CtrlFlg(secAddr, ldataRingBufferValue);
						result = TRUE;
                    }else{ //// section in next bank
                        // result = miWriteRange(LDATA_START_ADDR(ldataSBank)+LDATA_RING_VALUE_OFFSET, //// put 0x0001 to next section of NEXT BANK - which had only 1 data before
                                              // (UB*)&ldataRingBufferValue, LDATA_RING_VALUE_SIZE);
						lwrite_CtrlFlg(secAddr, ldataRingBufferValue);
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
            result = ldataStoreFrameData(frmAddr, Data); //// STORE NEW DATA
            dly_tsk(1/MSEC);
        }
        
        if(result && updateCurLatestData) {
            result = ldataUpdateCurLatestToOld(latestBankNum,latestSecNum,latestFrmNum);
        }
        
        if(result) {
            if(UpdateLearnInfo(ldataActivedBank, APARTMENT_TYPE)==0) {
                ldataFrameNumber++;
                if(ldataFrameNumber == (10 * frmNumInArea)) {
                    /* reset to avoid over number UW */
                    ldataFrameNumber = frmNumInArea;
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
    UH yNum, rNum;
    // UH fingerInfo[LDATA_FINGER_INFO_SIZE]; //// 3 bytes : status, room, finger
    UW bankAddr, secAddr, frmAddr;
	UW bankIndex, secIndex, frmIndex;
    // volatile SvLearnData* dataPtr;
	SvLearnData *learnDataPtr;
    
	learnDataPtr = &g_learnData;
    result = ((BankSw>0)&((Spec==APARTMENT_TYPE) || (Spec==COMPANY_TYPE)));
    if(result && (Spec == APARTMENT_TYPE))
    {
        // dataPtr = (SvLearnData*)fingerInfo;
        memset(&InfoLearningBankTable[0], 0xFF, LDATA_REG_NBR_MAX*sizeof(InfoLearnInBankM));
        for(bankIndex=ldataSBank; bankIndex<=ldataEBank; bankIndex++)
        {
            if((BankSw&LDATA_BIT(bankIndex))!=0)
            {
                // bankAddr = MI_FLASH_BASIC_ADDR + (bankIndex*MI_BANK_SIZE);
                for(secIndex=0; secIndex<MI_NUM_SEC_IN_BANK; secIndex++)
                {
                    // secAddr = bankAddr + (secIndex*MI_SECTOR_SIZE);
                    for(frmIndex=0; frmIndex<LDATA_FRAME_NUM_IN_SECTOR; frmIndex++)
                    {
                        // frmAddr = secAddr+(frmIndex*LDATA_FRAME_DATA_SIZE);
						lread_FrameByIndex(bankIndex, secIndex, frmIndex, learnDataPtr);
                        // result = miReadRange(frmAddr, (UB*)fingerInfo, sizeof(fingerInfo));
                        if(result)
                        {
                            rNum = learnDataPtr->RegRnum;
                            yNum = learnDataPtr->RegYnum;
                            if(learnDataPtr->RegStatus==LDATA_REGISTERD_STS)
                            {
								// Save latest data to Mapping Table
                                InfoLearningBankTable[rNum].BankNum[yNum]   = bankIndex;
                                InfoLearningBankTable[rNum].SectionNum[yNum]= secIndex;
                                InfoLearningBankTable[rNum].FrameNum[yNum]  = frmIndex;
							}
							
							// Compare Vein finger to count number of register of one person
							// result = lcheck_RegImg1(learnDataPtr->RegImg1); // not finished yet
							if(result)
							{
								InfoLearningBankTable[rNum].Num[yNum]++;
							}
							
                            
                        }
                    }
                }
            }
        }
    }
    
    return (result ? 0 : 1);
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
	bankIndex = InfoLearningBankTable[rNum].BankNum[yNum];
	secIndex  = InfoLearningBankTable[rNum].SectionNum[yNum];
	frmIndex  = InfoLearningBankTable[rNum].FrameNum[yNum];
	
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
    UH regStatus;
    UW frmAddr;
    
	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
    regStatus = (UH)LDATA_NOT_LATEST_STS;	//0xFFF8
	lwrite_RegStatus(frmAddr, regStatus);
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
    
    size = LDATA_AREA_SIZE(ldataSBank, ldataEBank);
    sAddr = LDATA_START_ADDR(ldataSBank);
    for(secAddr = 0; secAddr < size; secAddr += MI_SECTOR_SIZE) {
        result = miRemoveDataInSector(sAddr + secAddr);
        if(!result) {
            break;
        }
        dly_tsk( 50/MSEC );
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
// static BOOL ldataStoreFrameData(UW frmAddr, SvLearnData *lDataPtr)
// {
    // BOOL result;
    // UW uwSize;
	
    // lDataPtr->RegStatus = (UH)LDATA_DUR_REG_STS;
    // result = miWriteRange(frmAddr+LDATA_FRAME_STS_OFFSET, (UB*)&lDataPtr->RegStatus, LDATA_REG_STS_SIZE);
    // if(result)
	// {
        // dly_tsk(1/MSEC);
        // result = miWriteRange(frmAddr+LDATA_FRAME_IMG1_OFFSET, (UB*)&lDataPtr->RegImg1[0], LDATA_ALL_IMG_SIZE);
	// }
    
	// if(result)
	// {
		// dly_tsk(1/MSEC);
		// result = miWriteRange(frmAddr+LDATA_FRAME_RNUM_OFFSET, (UB*)&lDataPtr->RegRnum, LDATA_REG_RNUM_SIZE+LDATA_REG_YNUM_SIZE+LDATA_REG_DUMMY_SIZE);
	// }
	
	// if(result)
	// {
		// dly_tsk(1/MSEC);
		// lDataPtr->RegStatus = (UH)LDATA_REGISTERD_STS;
		// result = miWriteRange(frmAddr+LDATA_FRAME_STS_OFFSET, (UB*)&lDataPtr->RegStatus, LDATA_REG_STS_SIZE);
	// }
    
    // return result;
// }
static BOOL ldataStoreFrameData(UW frmAddr, SvLearnData *lDataPtr)
{
    BOOL result;
    UW uwSize;
	
    lDataPtr->RegStatus = (UH)LDATA_DUR_REG_STS;
	lwrite_RegStatus(frmAddr, lDataPtr->RegStatus);
	
	lwrite_RegImg(frmAddr, lDataPtr->RegImg1);
	lwrite_RegImg(frmAddr, lDataPtr->RegImg2);
	lwrite_MiniImg(frmAddr, lDataPtr->MiniImg);

	lwrite_RegRnum(frmAddr, lDataPtr->RegRnum);
	lwrite_RegYnum(frmAddr, lDataPtr->RegYnum);
	lwrite_Dummy1(frmAddr, lDataPtr->Dummy1);
		
	lDataPtr->RegStatus = (UH)LDATA_REGISTERD_STS;
    lwrite_RegStatus(frmAddr, lDataPtr->RegStatus);
    return TRUE;
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
		// Get LearnData stored in frame address
		lread_FrameByAddr(frmAddr, learnDataPtr);
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
		pre_secAddr = lcalcSectionAddr(bankIndex, secIndex) - MI_SECTOR_SIZE;
		result = miRemoveDataInSector(pre_secAddr);	// clear previous Section
	}
	
	if(result)
	{
		/* Write to 1st frame of previous Section */
		// result = miWriteRange(secAddr, (UB*)&InitlearnData, LDATA_FRAME_DATA_SIZE);
		lwrite_FrameByAddr(pre_secAddr, learnDataPtr);
	}
    
    return result;
}


/*
 *
 */

static BOOL lcheck_BankIndex(UW bankIndex)
{
	return (bankIndex >= ldataSBank && bankIndex <= ldataEBank);
}

static BOOL lcheck_SecIndex(UW secIndex)
{
	return (secIndex >= 0  && secIndex < MI_NUM_SEC_IN_BANK);
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

static UW lcalcBankAddr(UW bankIndex)
{
	UW bankAddr;
	bankAddr = MI_FLASH_BASIC_ADDR + (bankIndex*MI_BANK_SIZE);
	return bankAddr;
}

static UW lcalcSectionAddr(UW bankIndex, UW secIndex)
{
	UW bankAddr, secAddr;
	bankAddr = lcalcBankAddr(bankIndex);
	secAddr = bankAddr + secIndex*MI_SECTOR_SIZE;
	return secAddr;
}

static UW lcalcFrameAddr(UW bankIndex, UW secIndex, UW frmIndex)
{
	UW secAddr, frmAddr;
	secAddr = lcalcSectionAddr(bankIndex, secIndex);
	frmAddr = secAddr + frmIndex*sizeof(SvLearnData);
	return frmAddr;
}

static UW lcalc_FrameAddr(UW bankIndex, UW secIndex, UW frmIndex)
{
	UW bankAddr, secAddr, frmAddr;	// 32-bit address
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
		secAddr = bankAddr + secIndex*MI_SECTOR_SIZE;// + 0x100000; // !!!! add 0x100000 to fix bug of learnData.c
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

static BOOL lread_FrameByAddr(UW frmAddr, SvLearnData *learnDataPtr)
{
	return lread_Buffer(frmAddr, (UH*)learnDataPtr, sizeof(SvLearnData));
}

static void lwrite_FrameByAddr(UW frmAddr, SvLearnData* learnDataPtr)
{
	lwriteBuffer(frmAddr, (UH*)learnDataPtr, sizeof(SvLearnData));
}

static BOOL lread_RegStatus(UW frmAddr, UH *RegStatus)
{
	return lread_HWord(frmAddr + LDATA_FRAME_STS_OFFSET, RegStatus);
}

static void lwrite_RegStatus(UW frmAddr, UH RegStatus)
{
	lwriteHWord(frmAddr + LDATA_FRAME_STS_OFFSET, RegStatus);
}

static void lwrite_RegRnum(UW frmAddr, UH RegRnum)
{
	lwriteHWord(frmAddr + LDATA_FRAME_RNUM_OFFSET, RegRnum);
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

static void lwrite_CtrlFlg(UW secAddr, UH ctrl_flg)
{
	lwriteHWord(secAddr + CTRL_FLAG_OFFSET, ctrl_flg);
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

static BOOL lread_FrameByIndex(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData *learnDataPtr)
{
	BOOL result;
	UW frmAddr;

	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	result = lread_FrameByAddr(frmAddr, learnDataPtr);
	return result;
}

static void lremove_FrameByIndex(UW bankIndex, UW secIndex, UW frmIndex)
{
	UH val = 0xFFFF;
	UW frmAddr;

	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	lremove_Frame(frmAddr);
}

static UW lcalc_CtrlFlgAddr(UW bankIndex, UW secIndex)
{
	UW secAddr, ctrl_addr;
	BOOL ret = FALSE;

	ctrl_addr = 0;
	ret = lcheck_BankIndex(bankIndex);
	if(ret == TRUE)
	{
		ret = lcheck_SecIndex(secIndex);
	}

	if(ret == TRUE)
	{
		secAddr = lcalcSectionAddr(bankIndex, secIndex);
		ctrl_addr = secAddr + CTRL_FLAG_OFFSET;
	}
	return ctrl_addr;
}

static volatile UH lread_CtrlFlgByIndex(UW bankIndex, UW secIndex)
{
	UW ctrl_addr;
	volatile UH value = 0;
	ctrl_addr = lcalc_CtrlFlgAddr(bankIndex, secIndex);
	if(ctrl_addr != 0)
	{
		FlRead(ctrl_addr, (UH*)&value, 1);
	}
	if(value == 0x0001)
	{
		// Update oldest Section
		g_bank_oldest_index = bankIndex;
		g_section_oldest_index = secIndex;
		g_ctrl_flg_oldest = 0x0001;
	}
	return value;
}

/*
 * value: 0x0001 - the oldest data, 0xFFFF - old data
 */
static void lwrite_CtrlFlgByIndex(UW bankIndex, UW secIndex, UH value)
{
	UW ctrl_addr = 0; 	// 32-bit address
	ctrl_addr = lcalc_CtrlFlgAddr(bankIndex, secIndex);
	if(ctrl_addr != 0)
	{
		lwriteHWord(ctrl_addr, value);
	}
}

static void lset_OldestSection(UW bankIndex, UW secIndex)
{
	lwrite_CtrlFlgByIndex(bankIndex, secIndex, 0x0001);
}

static void lclear_OldestSection(UW bankIndex, UW secIndex)
{
	lwrite_CtrlFlgByIndex(bankIndex, secIndex, 0xFFFF);
}

static UW ldataCountFingerPattern(UW id)
{
	UW bankIndex, secIndex, frmIndex;
	UW frmAddr;
	UW cnt;
	UH RegStatus;
	UW RegID;
	SvLearnData* learnDataPtr;
	
	learnDataPtr = &g_learnData;
	cnt = 0;
	for(bankIndex=ldataSBank; bankIndex<=ldataEBank; bankIndex++)
	{
		for(secIndex=0; secIndex<MI_NUM_SEC_IN_BANK; secIndex++)
		{
			for(frmIndex=0; frmIndex<LDATA_FRAME_NUM_IN_SECTOR; frmIndex++)
			{
				frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
				lread_RegStatus(frmAddr, &RegStatus);
				if(RegStatus == LDATA_DUR_REG_STS || RegStatus == LDATA_REGISTERD_STS)
				{
					lread_RegID(frmAddr, &RegID);
					if(RegID == id)
					{
						cnt++;
					}
				}				
			}
		}
	}
	return cnt;
}

static void lwrite_InfoLearnInBankM(UH rNum, UH yNum, UW bankIndex, UW secIndex, UW frmIndex, UW num)
{
	InfoLearningBankTable[rNum].BankNum[yNum]    = bankIndex;
	InfoLearningBankTable[rNum].SectionNum[yNum] = secIndex;
	InfoLearningBankTable[rNum].FrameNum[yNum]   = frmIndex;
	InfoLearningBankTable[rNum].Num[yNum]        = num;
}

static void lread_InfoLearnInBankM(UH rNum, UH yNum, UW *bankIndex, UW *secIndex, UW *frmIndex, UW *num)
{
	*bankIndex = InfoLearningBankTable[rNum].BankNum[yNum];
	*secIndex  = InfoLearningBankTable[rNum].SectionNum[yNum];
	*frmIndex  = InfoLearningBankTable[rNum].FrameNum[yNum];
	*num       = InfoLearningBankTable[rNum].FrameNum[yNum];
}

static void lupdate_NextFrameLocation(UW bankIndex, UW secIndex, UW frmIndex)
{
	g_bankIndex = bankIndex;
	g_secIndex = secIndex;
	g_frmIndex = frmIndex;
}

static BOOL lupdateInfo(UW latest_bankIndex, UW latest_secIndex, UW latest_frmIndex)
{
	int result;
	UH cur_rNum, cur_yNum, cur_RegStatus;
	UH latest_rNum, latest_yNum, latest_RegStatus;
	UW bankIndex, secIndex, frmIndex, num;
	UW cur_bankIndex, cur_secIndex, cur_frmIndex, cur_num;
	SvLearnData* learnDataPtr;
	
	learnDataPtr = &g_learnData;
	
	if(lcheck_BankIndex(latest_bankIndex) || lcheck_SecIndex(latest_secIndex) || lcheck_FrmIndex(latest_frmIndex))
	{
		goto END_FUNC; // 0xFFFF
	}
	result = lread_FrameByIndex(latest_bankIndex, latest_secIndex, latest_frmIndex, learnDataPtr);
	
	latest_rNum = learnDataPtr->RegRnum;
	latest_yNum = learnDataPtr->RegYnum;
	latest_RegStatus = learnDataPtr->RegStatus;
	
	// Read current info of room, finger index in mapping table
	lread_InfoLearnInBankM(rNum, yNum, &cur_bankIndex, &cur_secIndex, &cur_frmIndex, &cur_num);
	if(lcheck_BankIndex(cur_bankIndex) || lcheck_SecIndex(cur_secIndex) || lcheck_FrmIndex(cur_frmIndex))
	{
		goto END_FUNC; // 0xFFFF
	}
	
	// Read RegStatus of current info of room, finger in flash memory
	result = lread_FrameByIndex(cur_bankIndex, cur_secIndex, cur_frmIndex, learnDataPtr);
	
	cur_rNum = learnDataPtr->RegRnum;
	cur_yNum = learnDataPtr->RegYnum;
	cur_RegStatus = learnDataPtr->RegStatus;
	
	if(cur_rNum == latest_rNum && cur_yNum == latest_yNum)
	{
		// Change RegStatus to old data
		UW frmAddr;
		frmAddr = lcalc_FrameAddr(cur_bankIndex, cur_secIndex, cur_frmIndex);
		lwrite_RegStatus(frmAddr, 0xFFF8);
	}
	
	
	// update mapping information
	// count all data with same RegID in flash
	
	
END_FUNC:
	return result;
}
#endif /* FWK_CFG_LEARN_DATA_ENABLE */
/*************************** The End ******************************************/
