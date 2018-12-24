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
#if FWK_CFG_LEARN_DATA_ENABLE

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
#define LDATA_FRAME_DATA_SIZE           (sizeof(SvLearnData))

/* Group definitions for configuration in flash memory ************************/
#define LDATA_START_ADDR(sbank)         (MI_FLASH_BASIC_ADDR+(sbank*MI_BANK_SIZE))
#define LDATA_AREA_SIZE(sbank, ebank)   ((ebank-sbank+1)*MI_BANK_SIZE)
#define LDATA_END_ADDR(ebank)           (MI_FLASH_BASIC_ADDR+((ebank+1)*MI_BANK_SIZE)-1)

/* This is offset ring value in each sector */
#define LDATA_RING_BUF_VALUE            0x0001
#define LDATA_RING_VALUE_SIZE           (sizeof(UH))
#define LDATA_RING_VALUE_OFFSET         (MI_SECTOR_SIZE-LDATA_RING_VALUE_SIZE)

/* number frame in a sector */
#define LDATA_FRAME_NUM_IN_SECTOR       19  /* (MI_SECTOR_SIZE/LDATA_FRAME_DATA_SIZE) */

/* number of frame in learning data area */
#define LDATA_FRAME_NUM_IN_LAREA        ((LDATA_AREA_SIZE(ldataSBank, ldataEBank)/MI_SECTOR_SIZE) * LDATA_FRAME_NUM_IN_SECTOR)

#define LDATA_REG_STS_SIZE              (sizeof(UH))
#define LDATA_REG_RNUM_SIZE             (sizeof(UH))
#define LDATA_REG_YNUM_SIZE             (sizeof(UH))
#define LDATA_REG_DUMMY_SIZE            (LDATA_DUMMY_NUM*sizeof(UH))
#define LDATA_ALL_IMG_SIZE              ((2*LDATA_NORM_IMAGE_SIZE) + \
                                        (LDATA_MINI_IMAGE_NUM*LDATA_MINI_IMAGE_SIZE))

#define LDATA_FRAME_STS_OFFSET          (0)
#define LDATA_FRAME_RNUM_OFFSET         LDATA_REG_STS_SIZE
#define LDATA_FRAME_IMG1_OFFSET         (LDATA_REG_STS_SIZE + \
                                        LDATA_REG_RNUM_SIZE + \
                                        LDATA_REG_YNUM_SIZE + LDATA_REG_DUMMY_SIZE)

/******************************************************************************/
/*************************** Structures Definitions ***************************/
/******************************************************************************/
typedef struct InfoLearningBankTableSt {
    UB BankNum[LDATA_REG_FIGURE_NBR_MAX]; //// 20
    UB SectionNum[LDATA_REG_FIGURE_NBR_MAX];
    UB FrameNum[LDATA_REG_FIGURE_NBR_MAX];
    UB Num[LDATA_REG_FIGURE_NBR_MAX];
}InfoLearnInBankM;

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
static BOOL ldataFindCurLatestFinger(UH yNum, UW* curBankNum,
                                     UW* curSecNum, UW* curFrmNum);
static BOOL ldataUpdateCurLatestToOld(UW curBankNum, UW curSecNum, UW curFrmNum);
static BOOL ldataRemoveAllData(void);
static BOOL ldataCheckFrameOnlyOneData(UW sectorAddress, UB* number,
                                       UB* bankNum, UB* sectorNum, UB* frameNum);
static BOOL ldataMoveOnlyOneDataToTop(UB bankNum, UB sectorNum, UB frameNum);
static BOOL ldataStoreFrameData(UW frameAddress, SvLearnData *lDataPtr);

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
    UB index;
    
    result = FALSE;
    if((BankSw & BANK0)==0) {
        miInit();
        /* find start bank and end bank in flash memory for learn data area */
        ldataSBank = 0;
        ldataEBank = 0;
        // for(index = 1; index <= BANK_MAX_NUM; index++) {
            // if(ldataSBank == 0) {
                // if(BankSw & (UB)LDATA_BIT(index)) {
                    // ldataSBank = index;
                // }
            // } else {
                // if(index == BANK_MAX_NUM) {
                    // if(BankSw & LDATA_BIT(index)) {
                        // ldataEBank = index;
                        // result = TRUE;
                        // break;
                    // }
                // }else if((BankSw & LDATA_BIT(index)) == 0) {
                    // ldataEBank = index-1;
                    // result = TRUE;
                    // break;
                // }
            // }
        // }
		for(index = 1; index < BANK_MAX_NUM; index++){
			if(ldataSBank == 0){
				if(BankSw & (UB)LDATA_BIT(index))	// b00000001 << 0 = 0x01, b00000001 << 1 = b00000010, b00000001 << 7 = b10000000
				{
					ldataSBank = index;	// 1:7
					break;
				}
			}
		}
		
		for(ldataEBank = index; index < BANK_MAX_NUM; index++)
		{
			
			if(!(BankSw & (UB)LDATA_BIT(index)))
			{
				break;
			}
			else
				ldataEBank = index;	// 1:7
		}		
    }
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
    volatile UB index, bankStoreRingVal;
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
        for(index=0; index<BANK_MAX_NUM; index++) {
            if((BankSw&LDATA_BIT(index))!=0) {
                bankAddr = MI_FLASH_BASIC_ADDR + (index*MI_BANK_SIZE);
                for(secIndex=0; secIndex<MI_NUM_SEC_IN_BANK; secIndex++) {
                    secAddr = bankAddr + (secIndex*MI_SECTOR_SIZE);
                    result = miReadRange(secAddr+LDATA_RING_VALUE_OFFSET, (UB*)fingerInfo, sizeof(UH));
                    if(result && (fingerInfo[0]==LDATA_RING_BUF_VALUE)) { //// 0x0001
                        secStoreRingVal = secIndex; //// top or oldest data location
                        bankStoreRingVal = index; //// top or oldest data location
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
            for(index=bankStoreRingVal; index<BANK_MAX_NUM; index++) {
                if((BankSw&LDATA_BIT(index))!=0) {
                    bankAddr = MI_FLASH_BASIC_ADDR + (index*MI_BANK_SIZE);
                    for(secIndex=secStoreRingVal; secIndex<MI_NUM_SEC_IN_BANK; secIndex++) {
                        secAddr = bankAddr + (secIndex*MI_SECTOR_SIZE);
                        for(frmIndex=0; frmIndex<LDATA_FRAME_NUM_IN_SECTOR; frmIndex++) {
                            frmAddr = secAddr+(frmIndex*LDATA_FRAME_DATA_SIZE);
                            result = miReadRange(frmAddr, (UB*)fingerInfo, sizeof(fingerInfo));
                            if(result)
                            {
                                if((fingerInfo[0]!=0xFFFF)&&(fingerInfo[1]!=0xFFFF)) { //// frames storing data
                                    rNum = learnDataPtr->RegRnum;
                                    yNum = learnDataPtr->RegYnum;
                                    if(learnDataPtr->RegStatus==LDATA_REGISTERD_STS) { //// 0xFFFC new registerd
                                        InfoLearningBankTable[rNum].BankNum[yNum]   = index;
                                        InfoLearningBankTable[rNum].SectionNum[yNum]= secAddr;
                                        InfoLearningBankTable[rNum].FrameNum[yNum]  = frmIndex;
                                        InfoLearningBankTable[rNum].Num[yNum]++; //// update all data when adding new data
                                        roomNumList[yNum] = rNum;
                                    }else if(learnDataPtr->RegStatus==LDATA_NOT_LATEST_STS) { //// 0xFFF8 old registerd
                                        InfoLearningBankTable[rNum].BankNum[yNum]   = index;
                                        InfoLearningBankTable[rNum].SectionNum[yNum]= secAddr;
                                        InfoLearningBankTable[rNum].FrameNum[yNum]  = frmIndex;
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
                    if((!searchFromFirst) && (index==(BANK_MAX_NUM-1)))
                    {
                        if(!searchFinish)
                        {
                            /* In writing full case, Find latest learn data from first bank to 0xFFFFFFFF in BankSw.*/
                            index = ldataSBank-1;
                            searchFromFirst = TRUE;
                        }
                    }else if(searchFromFirst && (index==bankStoreRingVal)) {
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
        frmOffsetInArea = ldataFrameNumber % frmNumInArea; //// need convert index ldataFrameNumber = ldataFrameNumber-1
        
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
            dly_tsk(1/MSEC);
        }else if(ldataFrameNumber >= frmNumInArea)  //// overwrite new data
        {///////////////////////////////////////
            if(frmIndex == 0) {
                /* checking only one learn data in next sector */
                numOnlyOneData = 0;
                result = ldataCheckFrameOnlyOneData(secAddr,	// almost return TRUE, !should check next Sector
                                                    &numOnlyOneData,
                                                    &bankNumOnlyOneData,
                                                    &secNumOnlyOneData,
                                                    &frmNumOnlyOneData);
                if(result) {
                    if(numOnlyOneData == 1) {
                        if(frmNumOnlyOneData>0) {
                            result = ldataMoveOnlyOneDataToTop(bankNumOnlyOneData,
                                                               secNumOnlyOneData,
                                                               frmNumOnlyOneData);
                        } else {
                            ldataRingBufferValue = 0xFFFF;
                            result = miWriteRange(secAddr+LDATA_RING_VALUE_OFFSET,
                                                  (UB*)&ldataRingBufferValue,
                                                  LDATA_RING_VALUE_SIZE);
                        }
                        
                        if(result) {
                            frmIndex++; // WRITE NEW DATA TO SECOND AREA - NEXT TO ONLY ONE DATA
                            frmAddr += LDATA_FRAME_DATA_SIZE;
                            notifyOnlyOneData = TRUE;
                        }
                    } else if(numOnlyOneData == 19) { //// all next section contains 19 only 1 data
                        ldataRingBufferValue = 0xFFFF;
                        result = miWriteRange(secAddr+LDATA_RING_VALUE_OFFSET,
                                              (UB*)&ldataRingBufferValue, LDATA_RING_VALUE_SIZE);
                        
                        secAddr += MI_SECTOR_SIZE;
                    } else { //// 1 < numOnlyOneData < 19 and before store to next Section
                        result = miRemoveDataInSector(secAddr);
                    }
                }
                
                if(result) {
                    ldataRingBufferValue = 0x0001; //// oldest data
                    if((secAddr+MI_SECTOR_SIZE)<LDATA_END_ADDR(ldataEBank)) { //// section in current bank
                        result = miWriteRange(secAddr+MI_SECTOR_SIZE+LDATA_RING_VALUE_OFFSET, //// put 0x0001 to next section - which had only 1 data before
                                              (UB*)&ldataRingBufferValue, LDATA_RING_VALUE_SIZE);
                    }else{ //// section in next bank
                        result = miWriteRange(LDATA_START_ADDR(ldataSBank)+LDATA_RING_VALUE_OFFSET, //// put 0x0001 to next section of NEXT BANK - which had only 1 data before
                                              (UB*)&ldataRingBufferValue, LDATA_RING_VALUE_SIZE);
                    }
                    
                    dly_tsk(1/MSEC);
                }
            }
        } //// end checking only 1 data
        
        updateCurLatestData = FALSE;
        if(result) {
            updateCurLatestData = ldataFindCurLatestFinger(Data->RegYnum, //// SAVE CURRENT LATEST FINGER
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
    
    result = (SearchNum<=LDATA_REG_NBR_MAX); //// < 480 ??<=??
    if(result)
    {
        for(fingerIndex = 0; fingerIndex < LDATA_REG_FIGURE_NBR_MAX; fingerIndex++) { //// < 20
            if(roomNumList[fingerIndex]<SearchNum) {
                *SearchResult[fingerIndex][0] = InfoLearningBankTable[roomNumList[fingerIndex]].BankNum[fingerIndex];
                *SearchResult[fingerIndex][1] = InfoLearningBankTable[roomNumList[fingerIndex]].SectionNum[fingerIndex];
                *SearchResult[fingerIndex][2] = InfoLearningBankTable[roomNumList[fingerIndex]].FrameNum[fingerIndex];
            }
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
/*                Input: Spec - is a kink of structure that will generate to  */
/*                       remap latest learn data on flash memory.             */
/*                       0: For Apartment type(Create InfoLearnInBankM).      */
/*                       1: For Company type (Create InfoLearnInBankC).       */
/* Return Value : int - return the zero value when updates successful, else to*/
/*                return the one value.                                       */
/* Remarks      : None                                                        */
/******************************************************************************/
static int UpdateLearnInfo(UB BankSw, UB Spec)
{
    volatile BOOL result;
    volatile UB index;
    volatile UH yNum, rNum;
    volatile UH fingerInfo[LDATA_FINGER_INFO_SIZE]; //// 3 bytes : status, room, finger
    volatile UW bankAddr, secAddr, frmAddr, secIndex, frmIndex;
    volatile SvLearnData* dataPtr;
    
    result = ((BankSw>0)&((Spec==APARTMENT_TYPE) || (Spec==COMPANY_TYPE)));
    if(result && (Spec == APARTMENT_TYPE))
    {
        dataPtr = (SvLearnData*)fingerInfo;
        memset(&InfoLearningBankTable[0], 0xFF, LDATA_REG_NBR_MAX*sizeof(InfoLearnInBankM));
        for(index=0; index<BANK_MAX_NUM; index++)
        {
            if((BankSw&LDATA_BIT(index))!=0)
            {
                bankAddr = MI_FLASH_BASIC_ADDR + (index*MI_BANK_SIZE);
                for(secIndex=0; secIndex<MI_NUM_SEC_IN_BANK; secIndex++)
                {
                    secAddr = bankAddr + (secIndex*MI_SECTOR_SIZE);
                    for(frmIndex=0; frmIndex<LDATA_FRAME_NUM_IN_SECTOR; frmIndex++)
                    {
                        frmAddr = secAddr+(frmIndex*LDATA_FRAME_DATA_SIZE);
                        result = miReadRange(frmAddr, (UB*)fingerInfo, sizeof(fingerInfo));
                        if(result)
                        {
                            rNum = dataPtr->RegRnum;
                            yNum = dataPtr->RegYnum;
                            if(dataPtr->RegStatus==LDATA_REGISTERD_STS)
                            {
                                InfoLearningBankTable[rNum].BankNum[yNum]   = index;
#ifdef TEST_API
                                InfoLearningBankTable[rNum].SectionNum[yNum]= secIndex;//secAddr; correct typo
#else
                                InfoLearningBankTable[rNum].SectionNum[yNum]= secAddr;
#endif
                                InfoLearningBankTable[rNum].FrameNum[yNum]  = frmIndex;
                                InfoLearningBankTable[rNum].Num[yNum]++;
                                roomNumList[yNum] = rNum;
                            }else if(dataPtr->RegStatus==LDATA_NOT_LATEST_STS) {
                                InfoLearningBankTable[rNum].BankNum[yNum]   = index;
#ifdef TEST_API
                                InfoLearningBankTable[rNum].SectionNum[yNum]= secIndex;//secAddr; correct typo
#else
                                InfoLearningBankTable[rNum].SectionNum[yNum]= secAddr;
#endif
                                InfoLearningBankTable[rNum].FrameNum[yNum]  = frmIndex;
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
static BOOL ldataFindCurLatestFinger(UH yNum, UW* curBankNum,
                                     UW* curSecNum, UW* curFrmNum)
{
    volatile BOOL result;
    volatile UB index;
    volatile UH fingerInfo[LDATA_FINGER_INFO_SIZE];
    volatile UW bankAddr, secAddr, frmAddr, secIndex, frmIndex;
    volatile SvLearnData* dataPtr;
    
    result =((yNum<LDATA_REG_FIGURE_NBR_MAX)&&
            (curBankNum!=NULL)&&(curSecNum!=NULL)&&(curFrmNum!=NULL));
    if(result)
    {
        dataPtr = (SvLearnData*)fingerInfo;
        for(index=0; index<BANK_MAX_NUM; index++)
        {
            result = ((ldataActivedBank&LDATA_BIT(index))!=0);
            if(result)
            {
                bankAddr = MI_FLASH_BASIC_ADDR + (index*MI_BANK_SIZE);
                for(secIndex=0; secIndex<MI_NUM_SEC_IN_BANK; secIndex++)
                {
                    secAddr = bankAddr + (secIndex*MI_SECTOR_SIZE);
                    for(frmIndex=0; frmIndex<LDATA_FRAME_NUM_IN_SECTOR; frmIndex++)
                    {
                        frmAddr = secAddr+(frmIndex*LDATA_FRAME_DATA_SIZE);
                        result = miReadRange(frmAddr, (UB*)fingerInfo, sizeof(fingerInfo));
                        if(result)
                        {
                            result = ((yNum==dataPtr->RegYnum)&&(dataPtr->RegStatus==LDATA_REGISTERD_STS));
                            if(result)
                            {
                                *curBankNum = index;
                                *curSecNum  = secIndex;
                                *curFrmNum  = frmIndex;
                                break;
                            }
                        }
                    }
                    
                    if(result)
                    {
                        break;
                    }
                }
            }
            
            if(result)
            {
                break;
            }
        }
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: ldataUpdateCurLatestToOld                                   */
/* Description  : This function will update current latest learn data to old  */
/*                data on flash memory.                                       */
/* Parameter    : Input: curBankNum - is bank number to store current latest  */
/*                       learn data.                                          */
/* Parameter    : Input: curSecNum - is sector number to store current latest */
/*                       learn data.                                          */
/* Parameter    : Input: curFrmNum - is frame number to store current latest  */
/*                       learn data.                                          */
/* Return Value : BOOL - return the TRUE value when updates successful, else  */
/*                to return the FALSE value.                                  */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL ldataUpdateCurLatestToOld(UW curBankNum, UW curSecNum, UW curFrmNum)
{
    volatile BOOL result;
    volatile UH regStatus;
    volatile UW address;
    
    address = MI_FLASH_BASIC_ADDR;
    address += curBankNum*MI_BANK_SIZE;             /* start bank address   */
    address += curSecNum*MI_SECTOR_SIZE;            /* start sector address */
    address += curFrmNum*LDATA_FRAME_DATA_SIZE;     /* frame address        */
    
    regStatus = (UH)LDATA_NOT_LATEST_STS;
    result = miWriteRange(address + LDATA_FRAME_STS_OFFSET,
                          (UB*)&regStatus, LDATA_REG_STS_SIZE);
    
    return result;
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
    volatile UW retryNumber, index, sAddr, size;
    
    size = LDATA_AREA_SIZE(ldataSBank, ldataEBank);
    sAddr = LDATA_START_ADDR(ldataSBank);
    for(index = 0; index < size; index += MI_SECTOR_SIZE) {
        result = miRemoveDataInSector(sAddr + index);
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
/* Parameter    : Input: frameAddress - is address of frame data in the flash */
/*                       memory.                                              */
/*                Input: lDataPtr - is a buffer that needs to store into the  */
/*                       flash memory.                                        */
/* Return Value : BOOL - return the TRUE value when erases successful, else to*/
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL ldataStoreFrameData(UW frameAddress, SvLearnData *lDataPtr)
{
    volatile BOOL result;
    
    lDataPtr->RegStatus = (UH)LDATA_DUR_REG_STS;
    result = miWriteRange(frameAddress+LDATA_FRAME_STS_OFFSET,
                          (UB*)&lDataPtr->RegStatus, LDATA_REG_STS_SIZE);
    if(result) {
        dly_tsk(1/MSEC);
        result = miWriteRange(frameAddress+LDATA_FRAME_IMG1_OFFSET,
                              (UB*)&lDataPtr->RegImg1[0], LDATA_ALL_IMG_SIZE);
        if(result) {
            dly_tsk(1/MSEC);
            result = miWriteRange(frameAddress+LDATA_FRAME_RNUM_OFFSET,
                                  (UB*)&lDataPtr->RegRnum,
                                  LDATA_REG_RNUM_SIZE+LDATA_REG_YNUM_SIZE+LDATA_REG_DUMMY_SIZE);
            if(result) {
                dly_tsk(1/MSEC);
                lDataPtr->RegStatus = (UH)LDATA_REGISTERD_STS;
                result = miWriteRange(frameAddress+LDATA_FRAME_STS_OFFSET,
                                      (UB*)&lDataPtr->RegStatus, LDATA_REG_STS_SIZE);
            }
        }
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: ldataCheckFrameOnlyOneData                                  */
/* Description  : This function will check only one learn data in a sector.   */
/* Parameter    : Input: sectorAddress - is address of a sector that needs to */
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
static BOOL ldataCheckFrameOnlyOneData(UW sectorAddress, UB* number,
                                       UB* bankNum, UB* sectorNum, UB* frameNum)
{
    volatile BOOL result;
    volatile UW roomIndex, numberFrm[LDATA_REG_FIGURE_NBR_MAX], regRnum, regYnum;
    volatile UB Ynum, currBankNum, currSectionNum, numOnlyOne;
    
    result = ((number!=NULL)&&(bankNum!=NULL)&&(sectorNum!=NULL)&&(frameNum!=NULL));
    if(result)
    {
        *number = 0;
        currBankNum = (sectorAddress - MI_FLASH_BASIC_ADDR) / MI_BANK_SIZE;
        currSectionNum = ((sectorAddress - MI_FLASH_BASIC_ADDR) % MI_BANK_SIZE) / MI_SECTOR_SIZE;
        for(Ynum = 0; Ynum < LDATA_REG_FIGURE_NBR_MAX; Ynum++) { //// 20
            numberFrm[Ynum] = 0;
            for(roomIndex = 0; roomIndex < LDATA_REG_NBR_MAX; roomIndex++) { //// 480
                numberFrm[Ynum] += InfoLearningBankTable[roomIndex].Num[Ynum];
            }
        }
        numOnlyOne = 0;
        for(Ynum = 0; Ynum < LDATA_REG_FIGURE_NBR_MAX; Ynum++) { //// 20
            if(numberFrm[Ynum] == (UW)1) {
                for(roomIndex = 0; roomIndex < LDATA_REG_NBR_MAX; roomIndex++) {
                    if( (InfoLearningBankTable[roomIndex].BankNum[Ynum]==currBankNum) &&
                        (InfoLearningBankTable[roomIndex].SectionNum[Ynum]==currSectionNum)) {
                        if(InfoLearningBankTable[roomIndex].Num[Ynum] == (UB)1) {
                            numOnlyOne++;
                            if(numOnlyOne == (UB)1) {
                                *bankNum = InfoLearningBankTable[roomIndex].BankNum[Ynum];
                                *sectorNum = InfoLearningBankTable[roomIndex].SectionNum[Ynum];
                                *frameNum = InfoLearningBankTable[roomIndex].FrameNum[Ynum];
                                regRnum = roomIndex;
                                regYnum = Ynum;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if(numOnlyOne == (UB)1) {
            InfoLearningBankTable[regRnum].FrameNum[regYnum] = 0;
        }
        *number = numOnlyOne;
        result = TRUE;
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: ldataMoveOnlyOneDataToTop                                   */
/* Description  : This function will move the only one learn data to top of   */
/*                same sector.                                                */
/* Parameter    : None                                                        */
/* Return Value : BOOL - return the TRUE value when moves successful, else to */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL ldataMoveOnlyOneDataToTop(UB bankNum, UB sectorNum, UB frameNum)
{
    volatile BOOL result;
    volatile UW secAddr, frmAddr;
    
    result = FALSE;
    if((bankNum>0)&&(bankNum<=7)&&(frameNum<LDATA_REG_FIGURE_NBR_MAX)) {
        secAddr = MI_FLASH_BASIC_ADDR + (bankNum * MI_BANK_SIZE) + (sectorNum * MI_SECTOR_SIZE);
        frmAddr = secAddr + (frameNum * LDATA_FRAME_DATA_SIZE);
        result = miReadRange(frmAddr, (UB*)&InitlearnData, LDATA_FRAME_DATA_SIZE);
        if(result) {
            result = miRemoveDataInSector(secAddr);
            if(result) {
                result = miWriteRange(secAddr, (UB*)&InitlearnData, LDATA_FRAME_DATA_SIZE);
            }
        }
    }
    
    return result;
}

#endif /* FWK_CFG_LEARN_DATA_ENABLE */
/*************************** The End ******************************************/
