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

#define CALC_BANK_ADDR(b)		(MI_FLASH_BASIC_ADDR + (b)*MI_BANK_SIZE)
#define CALC_SEC_ADDR(b, s)		(CALC_BANK_ADDR(b) + (s)*MI_SECTOR_SIZE)
#define CALC_FRM_ADDR(b, s, f)	(CALC_SEC_ADDR(b, s) + (f)*sizeof(SvLearnData))

/* number of frame in learning data area */
#define CALC_FRAME_NUM_IN_LAREA        ((CALC_AREA_SIZE(ldataSBank, ldataEBank)/MI_SECTOR_SIZE) * LDATA_FRAME_NUM_IN_SECTOR)

// #define LDATA_ALL_IMG_SIZE              ((2*LDATA_NORM_IMAGE_SIZE) + (LDATA_MINI_IMAGE_NUM*LDATA_MINI_IMAGE_SIZE))
/******************************************************************************/
/*************************** Structures Definitions ***************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Enum Definitions *********************************/
/******************************************************************************/
enum {
	PROCESS_NORMAL = 0,
	PROCESS_1ST_DATA,
	PROCESS_NEW_SECTION,
	PROCESS_ONLY_ONE_DATA_IN_NEXT_SECTION,
	PROCESS_LATEST_DATA_IN_NEXT_SECTION,
	PROCESS_UNIQUE_DATA_IN_NEXT_SECTION,
};
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
static BOOL ldataCheckFrameOnlyOneData(UW secAddr, UB* number);
static BOOL ldataMoveOnlyOneDataToTop(UB bankNum, UB sectorNum, UB frameNum);
static BOOL ldataStoreFrameData(UW frmAddr, SvLearnData *lDataPtr);

static int linitEmployeeList(void);
static void linitInfoLearningBankTable(void);
static int linitOldestSection(void);
static BOOL linitBankArea(UB BankSw);
static BOOL lcheck_BankIndex(UW bankIndex);
static BOOL lcheck_SecIndex(UW secIndex);
static BOOL lcheck_FrmIndex(UW frmIndex);
static BOOL lcheck_RegStatus(UH checked_val, UH expected_val);
static BOOL lcheck_RegRnum(UH checked_val);
static BOOL lcheck_RegYnum(UH checked_val);
static BOOL lcheck_ID(UW code);
static UW lcalc_BankAddr(UW bankIndex);
static UW lcalc_SectionAddr(UW bankIndex, UW secIndex);
static UW lcalc_FrameAddr(UW bankIndex, UW secIndex, UW frmIndex);
static void lcalc_nextSection(UW cur_bankIndex, UW cur_secIndex, UW *next_bankIndex, UW *next_secIndex);
static BOOL lmem_rd_hw(UW uwFp, UH *puhBp);
static UW lmem_wr_hw(UW uwFp, UH uhData);
static UW lmem_rd_buf(UW uwFp, UH *puhBp, UW n);
static UW lmem_wr_buf(UW uwFp, UH *puhBp, UW n);
static BOOL lmem_rm_sec(UW secAddr);
static BOOL lmem_rm_all(void);
static BOOL ldat_rd_Frm(UW frmAddr, SvLearnData *learnDataPtr);
static void ldat_wr_Frm(UW frmAddr, SvLearnData* learnDataPtr);
static void ldat_rm_Frm(UW frmAddr);
static void ldat_mv_Frm(UW frmAddr1, UW frmAddr2);
static BOOL ldat_rd_RegStatus(UW frmAddr, UH *RegStatus);
static void ldat_wr_RegStatus(UW frmAddr, UH RegStatus);
static BOOL ldat_rd_RegRnum(UW frmAddr, UH *RegRnum);
static void ldat_wr_RegRnum(UW frmAddr, UH RegRnum);
static BOOL ldat_rd_RegYnum(UW frmAddr, UH *RegYnum);
static void ldat_wr_RegYnum(UW frmAddr, UH RegYnum);
static BOOL ldat_rd_RegID(UW frmAddr, UH *RegID);
static void ldat_wr_RegID(UW frmAddr, UH RegID);
static void ldat_wr_Dummy1(UW frmAddr, UH* Dummy1);
static void ldat_wr_RegImg(UW frmAddr, UH* RegImg);
static void ldat_wr_MiniImg(UW frmAddr, UH* MiniImg);
static BOOL ldat_rd_CtrlFlg(UW secAddr, UH* ctrl_flg);
static void ldat_wr_CtrlFlg(UW secAddr, UH ctrl_flg);
static BOOL ldat_rd_FrmByIdx(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData *learnDataPtr);
static BOOL ldat_wr_FrmByIdx(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData *learnDataPtr);
static void ldat_rm_FrmByIdx(UW bankIndex, UW secIndex, UW frmIndex);
static void ldat_mv_FrmByIdx(UW bankIndex1, UW secIndex1, UW frmIndex1, UW bankIndex2, UW secIndex2, UW frmIndex2);
static BOOL ldat_rm_SecByIdx(UW bankIndex, UW secIndex);
static BOOL ldat_rd_CtrlFlgByIdx(UW bankIndex, UW secIndex, UH* ctrl_flg);
static void ldat_wr_CtrlFlgByIdx(UW bankIndex, UW secIndex, UH ctrl_flg);
static void lmap_writeMapInfoTable(UH rNum, UH yNum, UW bankIndex, UW secIndex, UW frmIndex, UW num, UH id);
static void lmap_readMapInfoTable(UH rNum, UH yNum, UW *bankIndex, UW *secIndex, UW *frmIndex, UW *num, UH *id);
static BOOL lmap_checkIsNewID(UW code);
static BOOL lmap_getIndexFromIDList(UH code, UH *index);
static BOOL lmap_getCountFromIDList(UH code, UW *count);
static BOOL lmap_updateMapInfoTable_location(UW bankIndex, UW secIndex, UW frmIndex);
static BOOL lmap_updateMapInfoTable_num(UH rNum, UH yNum);
static BOOL lmap_updateIDList(UW bankIndex, UW secIndex, UW frmIndex);
static void lupdateLearnInfo(void);
static void lshift_oldest_section(void);
static void lupdate_TheLatestFrame(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData* new_learnDataPtr);
static void lupdate_NotTheLatestFrame(SvLearnData* learnDataPtr);
static void lupdate_NextFrameLocation(UW bankIndex, UW secIndex, UW frmIndex);
static BOOL ladd_data(UW frmAddr, SvLearnData *learnDataPtr);
static BOOL ladd_process_new_section(UW bankIndex, UW secIndex, SvLearnData* learnDataPtr);
static BOOL ladd_process_start(void);
static BOOL ladd_check_unique_data(UW cur_bankIndex, UW cur_secIndex, UW *num);
static BOOL ladd_process_save_unique_data(UW cur_bankIndex, UW cur_secIndex, UW num);
static BOOL laddSvLearnImg(SvLearnData *learnDataPtr);
/******************************************************************************/
/*************************** Local Constant Variables *************************/
/******************************************************************************/


/******************************************************************************/
/*************************** Local Variables **********************************/
/******************************************************************************/
static volatile UB g_start_bank_index, g_end_bank_index;
static UB g_bank_num_max;
static volatile UB ldataActivedBank;
static volatile UW g_FrameNumber;
static volatile InfoLearnInBankM InfoLearningBankTable[LDATA_REG_NBR_MAX]; //// 480

static UW g_bankIndex=0, g_secIndex=0, g_frmIndex=0;
static UW g_next_bankIndex=0, g_next_secIndex=0, g_next_frmIndex=1;
static UW g_oldest_bank_index=0, g_oldest_section_index=0;

static SvLearnData g_learnData;
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
	result = linitBankArea(BankSw);
    
    if(result)
    {
		result = lmem_rm_all();
    }
	
	if(result)
	{
#if FWK_LD_SEMAPHORE_ENABLE
		ldataosInit();
#endif
		ldataActivedBank = BankSw;
		
		// Initialize oldest Section
		linitOldestSection();
		// Initialize Mapping info
		linitInfoLearningBankTable();
		linitEmployeeList();
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
	BOOL result;
#if FWK_LD_SEMAPHORE_ENABLE
	ldataosInit();
	ldataosGetSemaphore();
#endif  
	result = (BankSw > 0);
	if(result == FALSE)
		goto END_FUNC;
	
	if(Spec == APARTMENT_TYPE)
	{
		result = linitBankArea(BankSw);
		if(result)
		{
			result = lmem_rm_all();
		}

		if(result)
		{
			ldataActivedBank = BankSw;
			// Initialize oldest Section
			linitOldestSection();
			// Initialize Mapping info
			linitInfoLearningBankTable();
			linitEmployeeList();
			// Search latest data and update Mapping info
			lupdateLearnInfo(); // --> UpdateLearnInfo(ldataActivedBank, APARTMENT_TYPE);
		}

	}else if(Spec == APARTMENT_TYPE)
	{
		// Not implemented
	}
#if FWK_LD_SEMAPHORE_ENABLE
	ldataosReleaseSemaphore();
#endif
END_FUNC:
	return (result ? 0 : 1);
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
    BOOL result;
#if FWK_LD_SEMAPHORE_ENABLE
    ldataosGetSemaphore();
#endif

#ifdef FAST_TEST
	fast_test(&g_FrameNumber);
#endif

	/* Check valid of input parameters */
	if(Data == NULL)
		goto END_FUNC;
	
	result = laddSvLearnImg(Data);
	
	lupdateLearnInfo();  // UpdateLearnInfo(ldataActivedBank, APARTMENT_TYPE);
	

#if FWK_LD_SEMAPHORE_ENABLE
    ldataosReleaseSemaphore();
#endif
END_FUNC:
    return (result ? 0 : 1);
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
        for(fingerIndex = 0; fingerIndex < LDATA_REG_FIGURE_NBR_MAX; fingerIndex++) {
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
	result = (BankSw > 0);
	if(result == FALSE)
		goto END_FUNC;
	
    if(Spec == APARTMENT_TYPE)
    {
		lupdateLearnInfo();
    }else if(Spec == COMPANY_TYPE)
	{
		// Not implemented
	}
END_FUNC:
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
static BOOL ldataFindCurLatestFinger(UH rNum, UH yNum, UW* curBankNum, UW* curSecNum, UW* curFrmNum)
{
    return TRUE;
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
    return TRUE;
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
static BOOL ldataCheckFrameOnlyOneData(UW secAddr, UB* number)
{
    return TRUE;
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
    return TRUE;
}

/*
 *
 */
static int linitEmployeeList(void)
{
	g_total_empl = 0;
	memset(g_flash_data_info, 0, sizeof(g_flash_data_info));
	return 0;
}

static void linitInfoLearningBankTable(void)
{
	memset(InfoLearningBankTable, 0, sizeof(InfoLearningBankTable));
}

/*
 * Call after InitBankArea() to initialize g_start_bank_index
 */
static int linitOldestSection(void)
{
	g_oldest_bank_index = g_start_bank_index;
	g_oldest_section_index = 0;
	g_add_first_frame = 1;
}

static BOOL linitBankArea(UB BankSw)
{  
    if(BankSw & BANK0)
		return FALSE;	// Invalid Bank
	
	FlInit( SEM_FL );	// Initilize flash driver miInit();
	/* Find start bank and end bank in flash memory for learn data area */
	g_start_bank_index = g_end_bank_index = g_bank_num_max =0;

	for(g_start_bank_index = 1; g_start_bank_index < BANK_MAX_NUM; g_start_bank_index++)
	{
		if(BankSw & (UB)LDATA_BIT(g_start_bank_index))
			break;
	}
	
	for(g_end_bank_index = g_start_bank_index; g_end_bank_index < BANK_MAX_NUM; g_end_bank_index++)
	{
		if(!(BankSw & (UB)LDATA_BIT(g_end_bank_index+1)))
			break;
	}	
	
	g_bank_num_max = g_end_bank_index - g_start_bank_index + 1;
	
	// Start location
	g_bankIndex = g_next_bankIndex = g_start_bank_index;
	g_secIndex = g_next_secIndex = 0;
	g_frmIndex = 0;
	g_next_frmIndex = 1;

	return TRUE;
}

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

static BOOL lmem_rd_hw(UW uwFp, UH *puhBp)
{
	ER errCode;
	
	errCode = FlRead(uwFp, puhBp, 1);
	if(errCode != E_OK) {
		return FALSE;
	}
	return TRUE;
}

static UW lmem_wr_hw(UW uwFp, UH uhData)
{
	UW uwSize;
	
	uwSize = FlWrite(uwFp, (UH*)&uhData, 1);
	return uwSize;
}


static UW lmem_rd_buf(UW uwFp, UH *puhBp, UW n)
{
	ER errCode;
	
	errCode = FlRead(uwFp, puhBp, n/sizeof(UH));
	if(errCode != E_OK) {
		return FALSE;
	}
	return TRUE;
}

static UW lmem_wr_buf(UW uwFp, UH *puhBp, UW n)
{
	UW uwSize;
	
	uwSize = FlWrite(uwFp, puhBp, n/sizeof(UH));
	return uwSize;
}

static BOOL lmem_rm_sec(UW secAddr)
{
    ER errCode;
	errCode = FlErase(secAddr);	// reset to 0xFFFF
    return (errCode == E_OK);
}

static BOOL lmem_rm_all(void)
{
	UW secAddr;
	UW size;
	BOOL ret;
	
	secAddr = lcalc_SectionAddr(g_start_bank_index, 0);
	size = CALC_AREA_SIZE(g_start_bank_index, g_end_bank_index);
	
	while( secAddr < size ) {
		ret = lmem_rm_sec(secAddr);
		if(ret == FALSE)
			break;
		
		secAddr += MI_SECTOR_SIZE;
	}
	return ret;
}

static BOOL ldat_rd_Frm(UW frmAddr, SvLearnData *learnDataPtr)
{
	return lmem_rd_buf(frmAddr, (UH*)learnDataPtr, sizeof(SvLearnData));
}

static void ldat_wr_Frm(UW frmAddr, SvLearnData* learnDataPtr)
{
	lmem_wr_buf(frmAddr, (UH*)learnDataPtr, sizeof(SvLearnData));
}

static void ldat_rm_Frm(UW frmAddr)
{
	UW n;
	UH val;
	UH	*puhAddr;

	puhAddr = (UH*)frmAddr;
	n = sizeof(SvLearnData)/sizeof(UH);
	
	while( n ) {
		n--;
		lmem_wr_hw((UW)puhAddr, 0xFFFF);
		puhAddr++;
	}
}

static void ldat_mv_Frm(UW frmAddr1, UW frmAddr2)
{
	SvLearnData* learnDataPtr;
	learnDataPtr = &g_learnData;
	ldat_rd_Frm(frmAddr1, learnDataPtr);
	ldat_wr_Frm(frmAddr2, learnDataPtr);
	ldat_rm_Frm(frmAddr1);
}
static BOOL ldat_rd_RegStatus(UW frmAddr, UH *RegStatus)
{
	return lmem_rd_hw(frmAddr + LDATA_FRAME_STS_OFFSET, RegStatus);
}

static void ldat_wr_RegStatus(UW frmAddr, UH RegStatus)
{
	lmem_wr_hw(frmAddr + LDATA_FRAME_STS_OFFSET, RegStatus);
}

static BOOL ldat_rd_RegRnum(UW frmAddr, UH *RegRnum)
{
	return lmem_rd_hw(frmAddr + LDATA_FRAME_RNUM_OFFSET, RegRnum);
}

static void ldat_wr_RegRnum(UW frmAddr, UH RegRnum)
{
	lmem_wr_hw(frmAddr + LDATA_FRAME_RNUM_OFFSET, RegRnum);
}

static BOOL ldat_rd_RegYnum(UW frmAddr, UH *RegYnum)
{
	return lmem_rd_hw(frmAddr + LDATA_FRAME_YNUM_OFFSET, RegYnum);
}

static void ldat_wr_RegYnum(UW frmAddr, UH RegYnum)
{
	lmem_wr_hw(frmAddr + LDATA_FRAME_YNUM_OFFSET, RegYnum);
}

static BOOL ldat_rd_RegID(UW frmAddr, UH *RegID)
{
	return lmem_rd_hw(frmAddr + LDATA_FRAME_ID_OFFSET, RegID);
}

static void ldat_wr_RegID(UW frmAddr, UH RegID)
{
	lmem_wr_hw(frmAddr + LDATA_FRAME_ID_OFFSET, RegID);
}

static void ldat_wr_Dummy1(UW frmAddr, UH* Dummy1)
{
	lmem_wr_buf(frmAddr + LDATA_FRAME_DUMMY_OFFSET, (UH*)Dummy1, LDATA_REG_DUMMY_SIZE);
}

static void ldat_wr_RegImg(UW frmAddr, UH* RegImg)
{
	lmem_wr_buf(frmAddr + LDATA_FRAME_IMG1_OFFSET, (UH*)RegImg, LDATA_NORM_IMAGE_SIZE);
}

static void ldat_wr_MiniImg(UW frmAddr, UH* MiniImg)
{
	lmem_wr_buf(frmAddr + LDATA_FRAME_MINI_IMG_OFFSET, (UH*)MiniImg, LDATA_ALL_MINI_IMAGE_SIZE);
}

static BOOL ldat_rd_CtrlFlg(UW secAddr, UH* ctrl_flg)
{
	return lmem_rd_hw(secAddr + CTRL_FLAG_OFFSET, ctrl_flg);
}

static void ldat_wr_CtrlFlg(UW secAddr, UH ctrl_flg)
{
	lmem_wr_hw(secAddr + CTRL_FLAG_OFFSET, ctrl_flg);
}

static BOOL ldat_rd_FrmByIdx(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData *learnDataPtr)
{
	BOOL result;
	UW frmAddr;

	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	result = ldat_rd_Frm(frmAddr, learnDataPtr);
	return result;
}

static BOOL ldat_wr_FrmByIdx(UW bankIndex, UW secIndex, UW frmIndex, SvLearnData *learnDataPtr)
{
	UW frmAddr;

	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	ldat_wr_Frm(frmAddr, learnDataPtr);
	return TRUE;
}

static void ldat_rm_FrmByIdx(UW bankIndex, UW secIndex, UW frmIndex)
{
	UH val = 0xFFFF;
	UW frmAddr;

	frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
	ldat_rm_Frm(frmAddr);
}

static void ldat_mv_FrmByIdx(UW bankIndex1, UW secIndex1, UW frmIndex1, UW bankIndex2, UW secIndex2, UW frmIndex2)
{
	UW frmAddr1, frmAddr2;
	
	frmAddr1 = lcalc_FrameAddr(bankIndex1, secIndex1, frmIndex1);
	frmAddr2 = lcalc_FrameAddr(bankIndex2, secIndex2, frmIndex2);
	ldat_mv_Frm(frmAddr1, frmAddr2);	
}

static BOOL ldat_rm_SecByIdx(UW bankIndex, UW secIndex)
{
	UW secAddr;
	
	secAddr = lcalc_SectionAddr(bankIndex, secIndex);
	return lmem_rm_sec(secAddr);
}

static BOOL ldat_rd_CtrlFlgByIdx(UW bankIndex, UW secIndex, UH* ctrl_flg)
{
	UW secAddr;
	secAddr = lcalc_SectionAddr(bankIndex, secIndex);
	return ldat_rd_CtrlFlg(secAddr, ctrl_flg);
}

/*
 * ctrl_flg: 0x0001 - the oldest data, 0xFFFF - old data
 */
static void ldat_wr_CtrlFlgByIdx(UW bankIndex, UW secIndex, UH ctrl_flg)
{
	UW secAddr;
	secAddr = lcalc_SectionAddr(bankIndex, secIndex);
	ldat_wr_CtrlFlg(secAddr, ctrl_flg);
}

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

static BOOL lmap_getIndexFromIDList(UH code, UH *index)
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

	ret = lmap_getIndexFromIDList(code, (UH*)&index);
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
	ldat_rd_RegStatus(frmAddr, &regStatus);
	ret = lcheck_RegStatus(regStatus, LDATA_REGISTERD_STS); // 0xFFFC
	if(ret == TRUE)
	{
		// Save latest registration to Mapping info table
		ldat_rd_RegRnum(frmAddr, &rNum);
		ldat_rd_RegYnum(frmAddr, &yNum);
		ldat_rd_RegID(frmAddr, &id);
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
	ret = lmap_getCountFromIDList(id, (UW*)&cnt);
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
	ldat_rd_RegStatus(frmAddr, &regStatus);

	// Check status = 0xFFFC or 0xFFF8
	ret = lcheck_RegStatus(regStatus, LDATA_REGISTERD_STS);
	ret |= lcheck_RegStatus(regStatus, LDATA_NOT_LATEST_STS);
	if(ret == FALSE)
	{
		return FALSE;
	}
	
	// Read ID
	ldat_rd_RegID(frmAddr, &id);
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
		ret = lmap_getIndexFromIDList(id, (UH*)&index);
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
	for(rNum=0; rNum<=LDATA_REG_NBR_MAX; rNum++)
	{
		for(yNum=0; yNum<LDATA_REG_FIGURE_NBR_MAX; yNum++)
		{
			ret = lmap_updateMapInfoTable_num(rNum, yNum);
			if(ret == FALSE)
				continue;
		}
	}
}

/*
 * Shift and update the oldest Section location
 */
static void lshift_oldest_section(void)
{
	UW cur_bankIndex, cur_secIndex, next_bankIndex, next_secIndex;
	if(g_bankIndex == g_oldest_bank_index && g_secIndex == g_oldest_section_index)
	{
		cur_bankIndex = g_oldest_bank_index;
		cur_secIndex = g_oldest_section_index;
		
		// Set the current oldest Section to normal Section
		ldat_wr_CtrlFlgByIdx(cur_bankIndex, cur_secIndex, 0xFFFF);
		// Move to next Section
		lcalc_nextSection(cur_bankIndex, cur_secIndex, &next_bankIndex, &next_secIndex);
		// Set next Section to the oldest Section
		ldat_wr_CtrlFlgByIdx(next_bankIndex, next_secIndex, 0x0001);

		g_oldest_bank_index = next_bankIndex;
		g_oldest_section_index = next_secIndex;
	}
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
static void lupdate_NotTheLatestFrame(SvLearnData* learnDataPtr)
{
	UW bankIndex, secIndex, frmIndex;
	UW frmAddr;
	UH rNum, yNum;
	UH id;
	UW cnt;
	BOOL ret;

	bankIndex = secIndex = frmIndex = 0;
	ret = lcheck_RegStatus(learnDataPtr->RegStatus, LDATA_REGISTERD_STS); // 0xFFFC
	if(ret == TRUE)
	{
		// Get new learn data info
		rNum = learnDataPtr->RegRnum;
		yNum = learnDataPtr->RegYnum;
		// Get location in flash of latest data from Mapping info
		lmap_readMapInfoTable(rNum, yNum, &bankIndex, &secIndex, &frmIndex, &cnt, &id);
		// Change current latest data in flash to old data
		frmAddr = lcalc_FrameAddr(bankIndex, secIndex, frmIndex);
		ldat_wr_RegStatus(frmAddr, LDATA_NOT_LATEST_STS); // 0xFFF8	
	}
}

static void lupdate_NextFrameLocation(UW bankIndex, UW secIndex, UW frmIndex)
{
	if(bankIndex != 0)
		g_bankIndex = bankIndex;
	else g_bankIndex = g_next_bankIndex;
	
	if(secIndex != 0)
		g_secIndex = secIndex;
	else g_secIndex = g_next_secIndex;
	
	if(frmIndex != 0)
		g_frmIndex = frmIndex;
	else g_frmIndex = g_next_frmIndex;
		
	g_next_frmIndex = g_frmIndex + 1;
	if(g_next_frmIndex == LDATA_FRAME_NUM_IN_SECTOR)
	{
		g_next_frmIndex = 0;
		lcalc_nextSection(bankIndex, secIndex, &g_next_bankIndex, &g_next_secIndex);
	}else
	{
		g_next_bankIndex = g_bankIndex;
		g_next_secIndex = g_secIndex;
	}
}

static BOOL ladd_data(UW frmAddr, SvLearnData *learnDataPtr)
{
    BOOL result;
    UW uwSize;
	
    learnDataPtr->RegStatus = (UH)LDATA_DUR_REG_STS;
	ldat_wr_RegStatus(frmAddr, learnDataPtr->RegStatus);
	
	ldat_wr_RegImg(frmAddr, (UH*)learnDataPtr->RegImg1);
	ldat_wr_RegImg(frmAddr, (UH*)learnDataPtr->RegImg2);
	ldat_wr_MiniImg(frmAddr, (UH*)learnDataPtr->MiniImg);

	ldat_wr_RegRnum(frmAddr, learnDataPtr->RegRnum);
	ldat_wr_RegYnum(frmAddr, learnDataPtr->RegYnum);
	ldat_wr_RegID(frmAddr, learnDataPtr->RegID);
	ldat_wr_Dummy1(frmAddr, (UH*)learnDataPtr->Dummy1);
		
	learnDataPtr->RegStatus = (UH)LDATA_REGISTERD_STS;
    ldat_wr_RegStatus(frmAddr, learnDataPtr->RegStatus);
    return TRUE;
}

/*
 * Call when g_frmIndex = 0
 * Shift oldest control flag, remove data in section, store new data
 */
static BOOL ladd_process_new_section(UW bankIndex, UW secIndex, SvLearnData* learnDataPtr)
{

	UW cur_secAddr;

	// if(g_frmIndex != 0)
		// return FALSE;

	// Shift 0x0001 to next Section
	lshift_oldest_section();
	// Clear all frame in current Section
	cur_secAddr = lcalc_SectionAddr(bankIndex, secIndex);
	lmem_rm_sec(cur_secAddr);

	return TRUE;
}

/*
 * Call when g_bankIndex = g_start_bank_index && g_secIndex = g_frmIndex = 0
 */
static BOOL ladd_process_start(void)
{
	BOOL ret = FALSE;
	
	// Remove all data
	ret = lmem_rm_all();
	
	return ret;
}

/*
 * Return TRUE and num is number of only-one data.
 * Otherwise return FALSE and num=0
 * Check both only-one data and the latest data
 */
static BOOL ladd_check_unique_data(UW cur_bankIndex, UW cur_secIndex, UW *num)
{
	UW next_bankIndex, next_secIndex, frmIndex;
	UW frmAddr;
	BOOL ret;
	UH id, RegStatus;
	UW total, cnt;

	total = 0;
	lcalc_nextSection(cur_bankIndex, cur_secIndex, &next_bankIndex, &next_secIndex);
	memset(g_movedDataLocation, 0, sizeof(g_movedDataLocation));
	for(frmIndex=0; frmIndex<LDATA_FRAME_NUM_IN_SECTOR; frmIndex++)
	{
		frmAddr = lcalc_FrameAddr(next_bankIndex, next_secIndex, frmIndex);
		ldat_rd_RegStatus(frmAddr, &RegStatus);
		ldat_rd_RegID(frmAddr, &id);
		cnt = 0;
		
		ret = lmap_getCountFromIDList(id, (UW*)&cnt);
		if(ret == FALSE)
			continue;
		
		ret = lcheck_RegStatus(RegStatus, LDATA_REGISTERD_STS); // 0xFFFC
		if(ret == FALSE)
			continue;
		
		if(cnt == 1 || RegStatus == 0xFFFC)
		{
			total++;
			g_movedDataLocation[frmIndex] = 1;	// mark this index as only-one data or the latest data
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
 * Before: call lcheck_OnlyOneData() to get num; Shift control flag ()
 * After: call ladd_data(), update ID List, update mapping info
 * Save both only-one data and the latest data
 */
static BOOL ladd_process_save_unique_data(UW cur_bankIndex, UW cur_secIndex, UW num)
{
	UW next_bankIndex, next_secIndex, next_frmIndex, cur_frmIndex;
	UW frmAddr;
	BOOL ret;

	cur_frmIndex = 0;
	// Shift 0x0001 to next Section
	lshift_oldest_section();
	// Remove data in current Section
	ldat_rm_SecByIdx(cur_bankIndex, cur_secIndex);
	lcalc_nextSection(cur_bankIndex, cur_secIndex, &next_bankIndex, &next_secIndex);
	// Move only-one/latest data
	for(next_frmIndex=0; next_frmIndex<LDATA_FRAME_NUM_IN_SECTOR; next_frmIndex++)
	{
		if(g_movedDataLocation[next_frmIndex] == 1)
		{
			ldat_mv_FrmByIdx(next_bankIndex, next_secIndex, next_frmIndex, cur_bankIndex, cur_secIndex, cur_frmIndex);
			cur_frmIndex++;
			lupdate_NextFrameLocation(cur_bankIndex, cur_secIndex, cur_frmIndex);	// update each time moving data
		}
	}

	return (cur_frmIndex > 0);
}

static BOOL laddSvLearnImg(SvLearnData *learnDataPtr)
{
	// Variables declaration
	UW next_bankIndex, next_secIndex, next_frmIndex;
	UW cur_bankIndex, cur_secIndex, cur_frmIndex;
	int process_flag;
	UW sum_unique = 0;
	BOOL ret = FALSE;
	UW frmAddr = 0;
	
	process_flag = PROCESS_NORMAL;
	// Check 1st Frame
	if(g_add_first_frame == 1)
	{
		linitOldestSection();
		g_add_first_frame = 0;
		process_flag = PROCESS_1ST_DATA;
	}
	else if(g_frmIndex == 0) // Check new Section
	{
		// check RegStatus=0xFFFC and only-one data in next Section
		ret = ladd_check_unique_data(g_bankIndex, g_secIndex, &sum_unique);
		if(ret == TRUE)
		{
			process_flag = PROCESS_UNIQUE_DATA_IN_NEXT_SECTION;
		}else{
			// Check RingBuffer
			process_flag = PROCESS_NEW_SECTION;
		}
	}

	switch (process_flag) {
	case PROCESS_1ST_DATA:
		// Set begin of oldest Section
		ret = ladd_process_start();
	break;
	case PROCESS_UNIQUE_DATA_IN_NEXT_SECTION:
		ret = ladd_process_save_unique_data(g_bankIndex, g_secIndex, sum_unique);
		if(sum_unique == LDATA_FRAME_NUM_IN_SECTOR)
		{
			laddSvLearnImg(learnDataPtr);
		}
	break;
	case PROCESS_NEW_SECTION:
		ret = ladd_process_new_section(g_bankIndex, g_secIndex, learnDataPtr);
		// Store new data to 1st frame of current Section
	break;
	case PROCESS_NORMAL:
		// frmAddr = lcalc_FrameAddr(g_bankIndex, g_secIndex, g_frmIndex);
		// ret = ladd_data(frmAddr, learnDataPtr);
	break;
	default:
		// Do nothing
	break;
	}
	 
	
	// Store data
	frmAddr = lcalc_FrameAddr(g_start_bank_index, g_secIndex, g_frmIndex);
	ret = ladd_data(frmAddr, learnDataPtr);
	if(ret == TRUE)
	{
		lupdate_NextFrameLocation(0, 0, 0);	// auto update next location
		if(g_add_first_frame == 0)
			lupdate_NotTheLatestFrame(learnDataPtr); // Change RegStatus 0xFFFC -> 0xFFF8
	}

	return ret;
}
#endif /* FWK_CFG_LEARN_DATA_ENABLE */
/*************************** The End ******************************************/
