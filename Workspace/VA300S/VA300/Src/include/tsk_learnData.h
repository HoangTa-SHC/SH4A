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
#define N_SECT_PER_BANK     (MI_NUM_SEC_IN_BANK)
#define N_FRM_PER_BANK      (N_FRM_PER_SECT*N_SECT_PER_BANK)

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
	RED_1_ID = 7,
	GREEN_1_ID = 101,
	YELLOW_1_ID = 201,
	YELLOW_2_ID = 202,
	ORANGE_1_ID = 301,
	ORANGE_2_ID = 302,
	ORANGE_3_ID = 303,
	ORANGE_4_ID = 304,
	ORANGE_5_ID = 305,
	ORANGE_6_ID = 306,
	ORANGE_7_ID = 307,
	ORANGE_8_ID = 308,
	ORANGE_9_ID = 309,
	ORANGE_10_ID = 310,
	ORANGE_11_ID = 311,
	ORANGE_12_ID = 312,
	ORANGE_13_ID = 313,
	ORANGE_14_ID = 314,
	ORANGE_15_ID = 315,
	ORANGE_16_ID = 316,
	ORANGE_17_ID = 317,
	ORANGE_18_ID = 318,
	ORANGE_19_ID = 319,
	BLUE_1_ID = 401,
};

enum {
	RED_ROOM = 0,
	GREEN_ROOM = 1,
	YELLOW_ROOM = 2,
	ORANGE_ROOM = 3,
	BLUE_ROOM_1 = RED_ROOM,
	BLUE_ROOM_2 = GREEN_ROOM,
};

enum {
	RED_1_YNUM = 0,
	GREEN_1_YNUM = 0,
	YELLOW_1_YNUM = 1,
	YELLOW_2_YNUM = 2,
	ORANGE_1_YNUM = 1,
	ORANGE_2_YNUM = 2,
	ORANGE_3_YNUM = 3,
	ORANGE_4_YNUM = 4,
	ORANGE_5_YNUM = 5,
	ORANGE_6_YNUM = 6,
	ORANGE_7_YNUM = 7,
	ORANGE_8_YNUM = 8,
	ORANGE_9_YNUM = 9,
	ORANGE_10_YNUM = 10,
	ORANGE_11_YNUM = 11,
	ORANGE_12_YNUM = 12,
	ORANGE_13_YNUM = 13,
	ORANGE_14_YNUM = 14,
	ORANGE_15_YNUM = 15,
	ORANGE_16_YNUM = 16,
	ORANGE_17_YNUM = 17,
	ORANGE_18_YNUM = 18,
	ORANGE_19_YNUM = 19,
	BLUE_1_YNUM = RED_1_YNUM + 1, // person register 2 room
	BLUE_2_YNUM = GREEN_1_YNUM + 1, // person register 2 room
};

enum {
	FIRST_LOCATION,
	SECOND_LOCATION,
	FULL_FIRST_SECTION_LOCATION,
	START_SECOND_SECTION_LOCATION,
	FULL_FIRST_BANK_LOCATION,
	FULL_ALL_BANK_LOCATION,
	RESTART_FIRST_BANK_LOCATION,
	// 2 unique data
	YELLOW_1_PREPARED_LOCATION,
	YELLOW_2_PREPARED_LOCATION,
	// 19 unique data
	ORANGE_1_PREPARED_LOCATION,
	ORANGE_2_PREPARED_LOCATION,
	ORANGE_3_PREPARED_LOCATION,
	ORANGE_4_PREPARED_LOCATION,
	ORANGE_5_PREPARED_LOCATION,
	ORANGE_6_PREPARED_LOCATION,
	ORANGE_7_PREPARED_LOCATION,
	ORANGE_8_PREPARED_LOCATION,
	ORANGE_9_PREPARED_LOCATION,
	ORANGE_10_PREPARED_LOCATION,
	ORANGE_11_PREPARED_LOCATION,
	ORANGE_12_PREPARED_LOCATION,
	ORANGE_13_PREPARED_LOCATION,
	ORANGE_14_PREPARED_LOCATION,
	ORANGE_15_PREPARED_LOCATION,
	ORANGE_16_PREPARED_LOCATION,
	ORANGE_17_PREPARED_LOCATION,
	ORANGE_18_PREPARED_LOCATION,
	ORANGE_19_PREPARED_LOCATION,
	// 1 unique data
	GREEN_1_PREPARED_LOCATION,
	// 2 unique data
	YELLOW_1_MOVED_LOCATION,
	YELLOW_2_MOVED_LOCATION,
	RED_NEW_2_LOCATION,
	// 19 unique data
	ORANGE_1_MOVED_LOCATION,
	ORANGE_2_MOVED_LOCATION,
	ORANGE_3_MOVED_LOCATION,
	ORANGE_4_MOVED_LOCATION,
	ORANGE_5_MOVED_LOCATION,
	ORANGE_6_MOVED_LOCATION,
	ORANGE_7_MOVED_LOCATION,
	ORANGE_8_MOVED_LOCATION,
	ORANGE_9_MOVED_LOCATION,
	ORANGE_10_MOVED_LOCATION,
	ORANGE_11_MOVED_LOCATION,
	ORANGE_12_MOVED_LOCATION,
	ORANGE_13_MOVED_LOCATION,
	ORANGE_14_MOVED_LOCATION,
	ORANGE_15_MOVED_LOCATION,
	ORANGE_16_MOVED_LOCATION,
	ORANGE_17_MOVED_LOCATION,
	ORANGE_18_MOVED_LOCATION,
	ORANGE_19_MOVED_LOCATION,
	RED_NEW_19_LOCATION,
	// 1 unique data
	GREEN_1_MOVED_LOCATION,
	RED_NEW_1_LOCATION,
};
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

typedef struct Location_ST
{
	UW bankIndex;
	UW secIndex;
	UW frmIndex;
} Location;
/******************************************************************************/
/*************************** Export Functions *********************************/
/******************************************************************************/
void TaskLearnDataInit(void);
TASK TaskLearnData(void);

#endif /* TASK_LEARN_DATA_H */
/*************************** The End ******************************************/
