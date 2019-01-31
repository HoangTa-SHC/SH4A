/******************************************************************************/
/* File Name       : learnData.h                                              */
/* Description     : This file is header of the learn data component in the   */
/*                   framework layer.                                         */
/* Author          : TanTrinh                                                 */
/* Created         : April 15, 2018                                           */
/* Modified by     : Tan Trinh                                                */
/* Date Modified   : April 15, 2018                                           */
/* Content Modified:                                                          */
/* Copyright (c) 2015 SH Consuting K.K.                                       */
/*  and SH Consulting VietNam. All Rights Reserved.                           */
/******************************************************************************/
#ifndef LEARN_DATA_H
#define LEARN_DATA_H

#include "frameworkConf.h"
#if FWK_CFG_LEARN_DATA_ENABLE
/******************************************************************************/
/*************************** Standard Header **********************************/
/******************************************************************************/
#include <string.h>
#include <stdio.h>

/******************************************************************************/
/*************************** Project Header ***********************************/
/******************************************************************************/
#include "drv_flash.h"
#include "kernel.h"
#include "id.h"
/******************************************************************************/
/*************************** Macro Definitions ********************************/
/******************************************************************************/
#define LDATA_TEST_ENABLE               FALSE    /* Set to TRUE for fast testing */
// #define FAST_TEST
#define TEST_API

#define LDATA_BIT(n)                    (1 << n)
#define LDATA_ERROR_RETRY_NBR           4

// #if LDATA_TEST_ENABLE
// #define MI_FLASH_BASIC_ADDR             (0x02000000 - MI_BANK_SIZE)
// #define MI_SECTOR_SIZE                  (0x00020000)
// #define MI_BANK_SIZE                    (256 * MI_SECTOR_SIZE)
// #define MI_FLASH_START                  (MI_FLASH_BASIC_ADDR + MI_BANK_SIZE)
// #define MI_FLASH_SIZE                   (7 * MI_BANK_SIZE)
// #else
// #define MI_FLASH_BASIC_ADDR             (0)
// #define MI_SECTOR_SIZE                  (0x00020000)
// #define MI_BANK_SIZE                    (256 * MI_SECTOR_SIZE)
// #define MI_FLASH_START                  (MI_FLASH_BASIC_ADDR + MI_BANK_SIZE)
// #define MI_FLASH_SIZE                   (7 * MI_BANK_SIZE)
// #endif
#define MI_FLASH_BASIC_ADDR             (0)
#define MI_SECTOR_SIZE                  (0x00020000)  // 128kB
#define MI_BANK_SIZE                    (0x02000000)  // 32MB, 256 Sectors
#define MI_FLASH_START                  (0x06000000)  // Bank3
#define MI_FLASH_SIZE                   (0x10000000 - MI_FLASH_START) // 160MB, Bank3:7

#define LDATA_FRAME_DATA_SIZE           (sizeof(SvLearnData))
#ifdef TEST_API
#define MI_NUM_SEC_IN_BANK              10	// fast test
#define LDATA_FRAME_NUM_IN_SECTOR       19
#else
#define MI_NUM_SEC_IN_BANK              256
#define LDATA_FRAME_NUM_IN_SECTOR       19
#endif

#ifdef TEST_API
#define LDATA_DUMMY_NUM                 4	 // fast test
#define LDATA_NORM_IMAGE_SIZE           8	 // fast test
#define LDATA_MINI_IMAGE_NUM            2
#define LDATA_MINI_IMAGE_SIZE           4	 // fast test
#define LDATA_REG_NBR_MAX               8	 // fast test
#define LDATA_REG_FIGURE_NBR_MAX        20
#else
#define LDATA_DUMMY_NUM                 30
#define LDATA_NORM_IMAGE_SIZE           3200
#define LDATA_MINI_IMAGE_NUM            2
#define LDATA_MINI_IMAGE_SIZE           200
#define LDATA_REG_NBR_MAX               480
#define LDATA_REG_FIGURE_NBR_MAX        20
#endif

#define LDATA_REG_STS_SIZE              (sizeof(UH))
#define LDATA_REG_RNUM_SIZE             (sizeof(UH))
#define LDATA_REG_YNUM_SIZE             (sizeof(UH))
#define LDATA_REG_ID_SIZE               (sizeof(UH))
#define LDATA_REG_DUMMY_SIZE            (LDATA_DUMMY_NUM*sizeof(UH))
#define LDATA_ALL_MINI_IMAGE_SIZE		(LDATA_MINI_IMAGE_NUM*LDATA_MINI_IMAGE_SIZE)

#define LDATA_FRAME_STS_OFFSET          (0)
#define LDATA_FRAME_RNUM_OFFSET         (LDATA_FRAME_STS_OFFSET + LDATA_REG_STS_SIZE)
#define LDATA_FRAME_YNUM_OFFSET         (LDATA_FRAME_RNUM_OFFSET + LDATA_REG_RNUM_SIZE)
#define LDATA_FRAME_ID_OFFSET           (LDATA_FRAME_YNUM_OFFSET + LDATA_REG_YNUM_SIZE)
#define LDATA_FRAME_DUMMY_OFFSET        (LDATA_FRAME_ID_OFFSET + LDATA_REG_ID_SIZE)
#define LDATA_FRAME_IMG1_OFFSET         (LDATA_FRAME_DUMMY_OFFSET + LDATA_REG_DUMMY_SIZE)
#define LDATA_FRAME_IMG2_OFFSET			(LDATA_FRAME_IMG1_OFFSET + LDATA_NORM_IMAGE_SIZE)
#define LDATA_FRAME_MINI_IMG_OFFSET		(LDATA_FRAME_IMG2_OFFSET + LDATA_NORM_IMAGE_SIZE)
										
#define CTRL_FLAG_SIZE					(sizeof(UH))
#define CTRL_FLAG_OFFSET				(MI_SECTOR_SIZE - CTRL_FLAG_SIZE)
// #define THE_OLDEST_SECTION				(0x0001)

#define BANK_MAX_NUM                    8
#define BANK0                           (UB)LDATA_BIT(0)	// 0x01
#define BANK1                           (UB)LDATA_BIT(1)	// 0x02
#define BANK2                           (UB)LDATA_BIT(2)	// 0x04
#define BANK3                           (UB)LDATA_BIT(3)	// 0x08
#define BANK4                           (UB)LDATA_BIT(4)	// 0x10
#define BANK5                           (UB)LDATA_BIT(5)	// 0x20
#define BANK6                           (UB)LDATA_BIT(6)	// 0x40
#define BANK7                           (UB)LDATA_BIT(7)	// 0x80

/* Bank selection */
#define BANK3_3		(BANK3)
#define BANK3_4		(BANK3 | BANK4)
#define BANK3_5		(BANK3 | BANK4 | BANK5)
#define BANK3_6		(BANK3 | BANK4 | BANK5 | BANK6)
#define BANK3_7		(BANK3 | BANK4 | BANK5 | BANK6 | BANK7)
#define BANK4_4		(BANK4)
#define BANK4_5		(BANK4 | BANK5)
#define BANK4_6		(BANK4 | BANK5 | BANK6)
#define BANK4_7		(BANK4 | BANK5 | BANK6 | BANK7)
#define BANK5_5		(BANK5)
#define BANK5_6		(BANK5 | BANK6)
#define BANK5_7		(BANK5 | BANK6 | BANK7)
#define BANK6_6		(BANK6)
#define BANK6_7		(BANK6 | BANK7)
#define BANK7_7		(BANK7)

#define APARTMENT_TYPE                  0
#define COMPANY_TYPE                    1

/* Mapping information */
#define CODE_MIN	(0)
#define CODE_MAX	0x9999 //(4800)
#define CODE_RANGE	40//(CODE_MAX - CODE_MIN + 1)
/******************************************************************************/
/************************ Enumerations Definitions ****************************/
/******************************************************************************/
typedef enum {
    LDATA_NOT_YET_STS   = 0xFFFF,  //// Not yet
    LDATA_DUR_REG_STS   = 0xFFFE,  //// During registration
    LDATA_REGISTERD_STS = 0xFFFC, //// Registered. It is the latest data (Newest)
    LDATA_NOT_LATEST_STS= 0x0008, //// Not latest data. Old register , (!) 0xFFF8 cause error
	LDATA_CLEARED_STS   = 0x0B0B,  //// Cleared
    
    LDATA_UNKNOW_STS
} ldataRegStatusType;	//// Registraton status

typedef enum {
	LDATA_CTRL_FLG_OLDEST = 0x0001, // Oldest Section
	LDATA_CTRL_FLG_NORMAL = 0x0A0A, // Occupied Section
	LDATA_CTRL_FLG_ERASED = 0xFFFF, // Read only, cannot write to Flash Memory; Not occupied Section
} ldataCtrlFlagType;
/******************************************************************************/
/*************************** Structures Definitions ***************************/
/******************************************************************************/
typedef struct svLearnDataSt {
    UH RegStatus;   //// Registration status
    UH RegRnum;    //// Registration room number
    UH RegYnum;   //// Registration figure number for each room [0:19]
	UH RegID;
    UH Dummy1[LDATA_DUMMY_NUM];
    UB RegImg1[LDATA_NORM_IMAGE_SIZE];   //// [3200] --> 32 fast test
    UB RegImg2[LDATA_NORM_IMAGE_SIZE];   //// [3200] --> 32 fast test
    UB MiniImg[LDATA_MINI_IMAGE_NUM][LDATA_MINI_IMAGE_SIZE];   //// MiniImg[2][200]	--> [2][20] fast test
}SvLearnData;

typedef struct InfoLearningBankTableSt {
    UB BankNum[LDATA_REG_FIGURE_NBR_MAX];
    UB SectionNum[LDATA_REG_FIGURE_NBR_MAX];
    UB FrameNum[LDATA_REG_FIGURE_NBR_MAX];
    UW Num[LDATA_REG_FIGURE_NBR_MAX];
	UH ID[LDATA_REG_FIGURE_NBR_MAX];
}InfoLearnInBankM;

typedef struct EmployeeListSt {
	UH code;
	UW cnt;
}EmployeeList;
/******************************************************************************/
/*************************** Export Functions *********************************/
/******************************************************************************/
int InitBankArea(UB BankSw);
int InitLearnInfo(UB BankSw, UB Spec);
int AddSvLearnImg(SvLearnData *Data);
int SearchLearnImg(UH SearchNum, UW SearchResult[20][4]);

int InitEmployeeList(void);

/////////////////////////////////////////////////////////////
/*
 * Get location of the oldest Section
 */
void getOldestSection(UW *bankIndex, UW *secIndex, UW *ctrl_flg);

/*
 * Get location of next added learning data
 */
void getCurrentCursor(UW *bankIndex, UW *secIndex, UW *frmIndex);

void get_InfoLearnInBankM(int rNum, int yNum, UW* BankNum, UW* SectionNum, UW* FrameNum, UW* Num, UW* ID);
/////////////////////////////////////////////////////////////
#endif /* FWK_CFG_LEARN_DATA_ENABLE */
#endif /* LEARN_DATA_H */
/*************************** The End ******************************************/
