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

/******************************************************************************/
/*************************** Macro Definitions ********************************/
/******************************************************************************/
// #define LDATA_TEST_ENABLE               FALSE    /* Set to TRUE for fast testing */
#define FAST_TEST
#define TEST_API

#define LDATA_BIT(n)                    (1 << n)
#define LDATA_ERROR_RETRY_NBR           4

// #if LDATA_TEST_ENABLE
// #define MI_FLASH_BASIC_ADDR             (0x02000000 - MI_BANK_SIZE)
// #define MI_SECTOR_SIZE                  (128 * 1024)    //// 128kB = 0x2 0000
// #define MI_BANK_SIZE                    (4 * MI_SECTOR_SIZE)    //// 4 Sectors
// #define MI_FLASH_START                  (MI_FLASH_BASIC_ADDR + MI_BANK_SIZE)
// #define MI_FLASH_SIZE                   (7 * MI_BANK_SIZE)
// #else
// #define MI_FLASH_BASIC_ADDR             (0)
// #define MI_SECTOR_SIZE                  (128 * 1024)	//// 0x20000
// #define MI_BANK_SIZE                    (256 * MI_SECTOR_SIZE)  //// 256 Sectors , 0x2000000
// #define MI_FLASH_START                  (MI_FLASH_BASIC_ADDR + MI_BANK_SIZE)
// #define MI_FLASH_SIZE                   (7 * MI_BANK_SIZE)
// #endif
#define MI_FLASH_BASIC_ADDR             (0)
#define MI_SECTOR_SIZE                  (0x00020000)  //(128k)	//// 0x20000
#define MI_BANK_SIZE                    (0x02000000) //(256 * MI_SECTOR_SIZE)  //// 256 Sectors , 0x2000000
#define MI_FLASH_START                  (0x06000000) // Bank3
#define MI_FLASH_SIZE                   (0x10000000 - MI_FLASH_START)

#define LDATA_FRAME_DATA_SIZE           (sizeof(SvLearnData))
#ifdef TEST_API
#define MI_NUM_SEC_IN_BANK              10	// fast test
#define LDATA_FRAME_NUM_IN_SECTOR       19  /* (MI_SECTOR_SIZE/LDATA_FRAME_DATA_SIZE) */
#else
#define MI_NUM_SEC_IN_BANK              256//(MI_BANK_SIZE / MI_SECTOR_SIZE)
#define LDATA_FRAME_NUM_IN_SECTOR       19  /* (MI_SECTOR_SIZE/LDATA_FRAME_DATA_SIZE) */
#endif

#ifdef TEST_API
#define LDATA_DUMMY_NUM                 4
#define LDATA_NORM_IMAGE_SIZE           8//3200	 fast test
#define LDATA_MINI_IMAGE_NUM            2
#define LDATA_MINI_IMAGE_SIZE           4//200		fast test
#define LDATA_REG_NBR_MAX               480   //// No. rooms max
#define LDATA_REG_FIGURE_NBR_MAX        20  //// No. figure max
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

#define BANK_MAX_NUM                    8
#define BANK0                           (UB)LDATA_BIT(0)
#define BANK1                           (UB)LDATA_BIT(1)
#define BANK2                           (UB)LDATA_BIT(2)
#define BANK3                           (UB)LDATA_BIT(3)
#define BANK4                           (UB)LDATA_BIT(4)
#define BANK5                           (UB)LDATA_BIT(5)
#define BANK6                           (UB)LDATA_BIT(6)
#define BANK7                           (UB)LDATA_BIT(7)

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

/******************************************************************************/
/************************ Enumerations Definitions ****************************/
/******************************************************************************/
typedef enum {
    LDATA_NOT_YET_STS   = 0xFFFF,  //// Not yet
    LDATA_DUR_REG_STS   = 0xFFFE,  //// During registration
    LDATA_REGISTERD_STS = 0xFFFC, //// Registered. It is the latest data (Newest)
    LDATA_NOT_LATEST_STS= 0xFFF8, //// Not latest data. Old register
    
    LDATA_UNKNOW_STS
} ldataRegStatusType;	//// Registraton status

/******************************************************************************/
/*************************** Structures Definitions ***************************/
/******************************************************************************/
typedef struct svLearnDataSt {
    UH RegStatus;   //// Registration status
    UH RegRnum;    //// Registration room number
    UH RegYnum;   //// Registration figure number for each room [0:19]
	UW RegID;
    UH Dummy1[LDATA_DUMMY_NUM];
    UB RegImg1[LDATA_NORM_IMAGE_SIZE];   //// [3200] --> 32 fast test
    UB RegImg2[LDATA_NORM_IMAGE_SIZE];   //// [3200] --> 32 fast test
    UB MiniImg[LDATA_MINI_IMAGE_NUM][LDATA_MINI_IMAGE_SIZE];   //// MiniImg[2][200]	--> [2][20] fast test
}SvLearnData;

typedef struct InfoLearningBankTableSt {
    UB BankNum[LDATA_REG_FIGURE_NBR_MAX];
    UB SectionNum[LDATA_REG_FIGURE_NBR_MAX];
    UB FrameNum[LDATA_REG_FIGURE_NBR_MAX];
    UB Num[LDATA_REG_FIGURE_NBR_MAX];
	UW ID[LDATA_REG_FIGURE_NBR_MAX];
}InfoLearnInBankM;

/******************************************************************************/
/*************************** Export Functions *********************************/
/******************************************************************************/
int InitBankArea(UB BankSw);
int InitLearnInfo(UB BankSw, UB Spec);
int AddSvLearnImg(SvLearnData *Data);
int SearchLearnImg(UH SearchNum, UH* SearchResult[20][3]);

#ifdef TEST_API
void get_InfoLearnInBankM(int rNum, int yNum, UB* BankNum, UB* SectionNum, UB* FrameNum, UB* Num);
#endif

#endif /* FWK_CFG_LEARN_DATA_ENABLE */
#endif /* LEARN_DATA_H */
/*************************** The End ******************************************/
