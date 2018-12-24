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
#define LDATA_TEST_ENABLE               FALSE    /* Set to TRUE for fast testing */

#define LDATA_BIT(n)                    (1 << n)
#define LDATA_ERROR_RETRY_NBR           4

#if LDATA_TEST_ENABLE
#define MI_FLASH_BASIC_ADDR             (0x08000000 - MI_BANK_SIZE)
#define MI_SECTOR_SIZE                  (128 * 1024)
#define MI_BANK_SIZE                    (4 * MI_SECTOR_SIZE)
#define MI_FLASH_START                  (MI_FLASH_BASIC_ADDR + MI_BANK_SIZE)
#define MI_FLASH_SIZE                   (7 * MI_BANK_SIZE)
#else
#define MI_FLASH_BASIC_ADDR             (0)
#define MI_SECTOR_SIZE                  (128 * 1024)
#define MI_BANK_SIZE                    (256 * MI_SECTOR_SIZE)
#define MI_FLASH_START                  (MI_FLASH_BASIC_ADDR + MI_BANK_SIZE)
#define MI_FLASH_SIZE                   (7 * MI_BANK_SIZE)
#endif

#define MI_NUM_SEC_IN_BANK              (MI_BANK_SIZE / MI_SECTOR_SIZE)

#define LDATA_DUMMY_NUM                 32
#define LDATA_NORM_IMAGE_SIZE           3200
#define LDATA_MINI_IMAGE_NUM            2
#define LDATA_MINI_IMAGE_SIZE           200
#define LDATA_REG_NBR_MAX               480
#define LDATA_REG_FIGURE_NBR_MAX        20

#define BANK_MAX_NUM                    8
#define BANK0                           (UB)LDATA_BIT(0)
#define BANK1                           (UB)LDATA_BIT(1)
#define BANK2                           (UB)LDATA_BIT(2)
#define BANK3                           (UB)LDATA_BIT(3)
#define BANK4                           (UB)LDATA_BIT(4)
#define BANK5                           (UB)LDATA_BIT(5)
#define BANK6                           (UB)LDATA_BIT(6)
#define BANK7                           (UB)LDATA_BIT(7)

#define APARTMENT_TYPE                  0
#define COMPANY_TYPE                    1

/******************************************************************************/
/************************ Enumerations Definitions ****************************/
/******************************************************************************/
typedef enum {
    LDATA_NOT_YET_STS   = 0xFFFF,
    LDATA_DUR_REG_STS   = 0xFFFE,
    LDATA_REGISTERD_STS = 0xFFFC,
    LDATA_NOT_LATEST_STS= 0xFFF8,
    
    LDATA_UNKNOW_STS
} ldataRegStatusType;

/******************************************************************************/
/*************************** Structures Definitions ***************************/
/******************************************************************************/
typedef struct svLearnDataSt {
    UH RegStatus;
    UH RegRnum;
    UH RegYnum;
    UH Dummy1[LDATA_DUMMY_NUM];
    UB RegImg1[LDATA_NORM_IMAGE_SIZE];
    UB RegImg2[LDATA_NORM_IMAGE_SIZE];
    UB MiniImg[LDATA_MINI_IMAGE_NUM][LDATA_MINI_IMAGE_SIZE];
}SvLearnData;

/******************************************************************************/
/*************************** Export Functions *********************************/
/******************************************************************************/
int InitBankArea(UB BankSw);
int InitLearnInfo(UB BankSw, UB Spec);
int AddSvLearnImg(SvLearnData *Data);
int SearchLearnImg(UH SearchNum, UH* SearchResult[20][3]);

#endif /* FWK_CFG_LEARN_DATA_ENABLE */
#endif /* LEARN_DATA_H */
/*************************** The End ******************************************/