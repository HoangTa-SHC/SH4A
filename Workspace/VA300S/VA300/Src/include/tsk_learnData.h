/******************************************************************************/
/* File Name       : tsk_learnData.h                                          */
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
#ifndef TASK_LEARN_DATA_H
#define TASK_LEARN_DATA_H

/******************************************************************************/
/*************************** Standard Header **********************************/
/******************************************************************************/
#include <stdio.h>
#include <string.h>

/******************************************************************************/
/*************************** Project Header ***********************************/
/******************************************************************************/
#include "kernel.h"
#include "sh7750.h"
#include "sh7750R.h"
#include "id.h"
#include "command.h"
#include "drv_fpga.h"
#include "err_ctrl.h"

#include "va300.h"
#include "drv_buz.h"
#include "id.h"
#include "learnData.h"

/******************************************************************************/
/*************************** Macro Definitions ********************************/
/******************************************************************************/
#if FWK_CFG_LEARN_DATA_ENABLE
#define TSK_LEARN_DATA_TEST_ENABLE  TRUE
#endif

#define BANK_NUM_MIN (3)
#define BANK_NUM_MAX (7)
#define N_BANK_INIT (5)

#if defined N_BANK_INIT && N_BANK_INIT==1
#define BANK_SELECTED   (BANK3)
#define BANK_OFFSET 3
#elif N_BANK_INIT && N_BANK_INIT==2
#define BANK_SELECTED   (BANK3|BANK4)
#define BANK_OFFSET 3
#elif N_BANK_INIT && N_BANK_INIT==5
#define BANK_SELECTED   (BANK3|BANK4|BANK5|BANK6|BANK7)
#define BANK_OFFSET 3
#endif

#define N_FRM_PER_SECT      (LDATA_FRAME_NUM_IN_SECTOR)
#define N_SECT_PER_BANK     (MI_NUM_SEC_IN_BANK)	// 256
#define N_FRM_PER_BANK      (N_FRM_PER_SECT*N_SECT_PER_BANK)//(19*256)

// B[3:7]S[0:255]F[0:18]	(n_frames)
#define B3S0F0	(1)
#define B3S0F1	(2)
#define B3S0F2	(3)
#define B3S0F18	(N_FRM_PER_SECT)
#define B3S1F0	(N_FRM_PER_SECT*1+1)
#define B3S2F0	(N_FRM_PER_SECT*2+1)	// (+FULL_ALL_BANK) check 2 unique data 19*2+1 = 39
#define B3S2F2	(N_FRM_PER_SECT*2+3)	// (+FULL_ALL_BANK) reset after clear Section and save 2 unique data + 1 new data
#define B3S3F0	(N_FRM_PER_SECT*3+1)	
#define B3S3F1	(N_FRM_PER_SECT*3+2)	// Store 2 unique data
#define B3S3F3	(N_FRM_PER_SECT*3+4)	// Store 2 unique data
#define B3S4F0	(N_FRM_PER_SECT*4+1)	// (+FULL_ALL_BANK) Check 19 unique data 19*4+1 = 77
#define B3S5F0	(N_FRM_PER_SECT*5+1)	// (+FULL_ALL_BANK) reset after clear Section and save 19 unique data + 1 new data
#define B3S5F0	(N_FRM_PER_SECT*5+1)	// Store 19 unique data
#define B3S5F18	(N_FRM_PER_SECT*5+19)	// Store 19 unique data
#define B3S6F0	(N_FRM_PER_SECT*6+1)
#define B3S7F0	(N_FRM_PER_SECT*7+1) 	// (+FULL_ALL_BANK) Check 1 unique data
#define B3S7F1	(N_FRM_PER_SECT*7+2)	// (+FULL_ALL_BANK) reset after clear Section and save 1 unique data + 1 new data
#define B3S8F1	(N_FRM_PER_SECT*8+2)	// Store 1 unique data
#define B3S255F18 (N_FRM_PER_BANK)		/// 256*19
#define B4S0F0	(N_FRM_PER_BANK+1)
#define B7S255F18 (N_FRM_PER_BANK*N_BANK_INIT) /// (256*19)*5
#define FULL_BANK3 (N_FRM_PER_BANK)
#define FULL_ALL_BANK (N_FRM_PER_BANK*N_BANK_INIT)


// Unique data position
#define LDATA_FINGER_RED_1_POS (0)
#define LDATA_FINGER_YELLOW_1_POS (B3S3F1)
#define LDATA_FINGER_YELLOW_2_POS (B3S3F3)
#define LDATA_FINGER_ORANGE_1_POS ((B3S5F0)+0)
#define LDATA_FINGER_ORANGE_2_POS ((B3S5F0)+1)
#define LDATA_FINGER_ORANGE_3_POS ((B3S5F0)+2)
#define LDATA_FINGER_ORANGE_4_POS ((B3S5F0)+3)
#define LDATA_FINGER_ORANGE_5_POS ((B3S5F0)+4)
#define LDATA_FINGER_ORANGE_6_POS ((B3S5F0)+5)
#define LDATA_FINGER_ORANGE_7_POS ((B3S5F0)+6)
#define LDATA_FINGER_ORANGE_8_POS ((B3S5F0)+7)
#define LDATA_FINGER_ORANGE_9_POS ((B3S5F0)+8)
#define LDATA_FINGER_ORANGE_10_POS ((B3S5F0)+9)
#define LDATA_FINGER_ORANGE_11_POS ((B3S5F0)+10)
#define LDATA_FINGER_ORANGE_12_POS ((B3S5F0)+11)
#define LDATA_FINGER_ORANGE_13_POS ((B3S5F0)+12)
#define LDATA_FINGER_ORANGE_14_POS ((B3S5F0)+13)
#define LDATA_FINGER_ORANGE_15_POS ((B3S5F0)+14)
#define LDATA_FINGER_ORANGE_16_POS ((B3S5F0)+15)
#define LDATA_FINGER_ORANGE_17_POS ((B3S5F0)+16)
#define LDATA_FINGER_ORANGE_18_POS ((B3S5F0)+17)
#define LDATA_FINGER_ORANGE_19_POS ((B3S5F0)+18)
#define LDATA_FINGER_GREEN_1_POS (B3S8F1)
/******************************************************************************/
/************************ Enumerations Definitions ****************************/
/******************************************************************************/

enum{
	LDATA_FINGER_RED_1_INDEX,	// default data
	LDATA_FINGER_BLUE_1_INDEX,	// check mapping table
	/* 2 unique data */
	LDATA_FINGER_YELLOW_1_INDEX,
	LDATA_FINGER_YELLOW_2_INDEX,
	/* 19 unique data */
    LDATA_FINGER_ORANGE_1_INDEX,          
    LDATA_FINGER_ORANGE_2_INDEX,          
    LDATA_FINGER_ORANGE_3_INDEX,          
    LDATA_FINGER_ORANGE_4_INDEX,          
    LDATA_FINGER_ORANGE_5_INDEX,          
    LDATA_FINGER_ORANGE_6_INDEX,          
    LDATA_FINGER_ORANGE_7_INDEX,          
    LDATA_FINGER_ORANGE_8_INDEX,          
    LDATA_FINGER_ORANGE_9_INDEX,          
    LDATA_FINGER_ORANGE_10_INDEX,         
    LDATA_FINGER_ORANGE_11_INDEX,         
    LDATA_FINGER_ORANGE_12_INDEX,         
    LDATA_FINGER_ORANGE_13_INDEX,         
    LDATA_FINGER_ORANGE_14_INDEX,         
    LDATA_FINGER_ORANGE_15_INDEX,         
    LDATA_FINGER_ORANGE_16_INDEX,         
    LDATA_FINGER_ORANGE_17_INDEX,         
    LDATA_FINGER_ORANGE_18_INDEX,         
    LDATA_FINGER_ORANGE_19_INDEX,
	/* 1 unique data */
	LDATA_FINGER_GREEN_1_INDEX,
	/* Total */
	LDATA_FINGER_INDEX_TOTAL
};

enum {
	GREEN_ID = 0x0100 + 4,
	YELLOW_ID = LDATA_FINGER_YELLOW_1_INDEX + 0x0200,
	ORANGE_ID = LDATA_FINGER_ORANGE_1_INDEX + 0x1900,
	RED_ID = 7,
};
/////////////////////////////////
// Test case
/////////////////////////////////

/******************************************************************************/
/*************************** Structures Definitions ***************************/
/******************************************************************************/
typedef struct SvLearnResult_ST
{
	UH RegStatus;
	UH RegRnum;
	UH RegYnum;
	UH RegID;
	UB RegImg1; // first 1-byte
}SvLearnResult;

typedef struct SearchLearnDataResult_ST
{
	int result;
	UH LearnInfo[LDATA_REG_FIGURE_NBR_MAX][3];
}SearchLearnDataResult;

/******************************************************************************/
/*************************** Export Functions *********************************/
/******************************************************************************/
void TaskLearnDataInit(void);
TASK TaskLearnData(void);

#endif /* TASK_LEARN_DATA_H */
/*************************** The End ******************************************/
