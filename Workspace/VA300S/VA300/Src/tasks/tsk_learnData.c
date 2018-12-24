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

#define LDATA_FINGER_RED_IMG            0x11
#define LDATA_FINGER_BLUE_IMG           0x22
#define LDATA_FINGER_GREEN_IMG          0x33

/* number of frame in learning data area */
#define FRAME_NUM_IN_MAX_TEST           (4*19)  /* ((MI_BANK_SIZE/MI_SECTOR_SIZE) * 19) */
#define FRAME_NUM_IN_ADD_CASE           ((2*19) + 4)
#define FRAME_NUM_IN_ONLY_1DATA_CASE    (2*19)

/******************************************************************************/
/************************ Enumerations Definitions ****************************/
/******************************************************************************/
typedef enum {
    LDATA_ADD_CASE    = '0',
    LDATA_SEARCH_CASE = '1',
    LDATA_RING_CASE   = '2',
    LDATA_NOTIFY_CASE = '3',
    
    LDATA_UNKNOW_CASE
} ldataTestCaseType;

/******************************************************************************/
/*************************** Functions Prototype ******************************/
/******************************************************************************/
TASK TaskLearnData(void);
void taskLearnDataHandler(void);

/******************************************************************************/
/*************************** Constant Local Variables *************************/
/******************************************************************************/
#if TSK_LEARN_DATA_TEST_ENABLE
static const T_CTSK ctsk_learn_data = {TA_HLNG, NULL, (FP)TaskLearnData, TSK_PRI_LEARN_DATA,
                                       TSK_STACK_SIZE_LEARN_DATA, NULL, "Lean Data Task"};

static volatile SvLearnData ldRegFingerRed, ldRegFingerBlue, ldRegFingerGreen;
static volatile UH SearchLearDataResult[LDATA_REG_FIGURE_NBR_MAX][3];
static volatile UH* SearchLearDataResultPtr[LDATA_REG_FIGURE_NBR_MAX][3];
#endif

/******************************************************************************/
/*************************** Local Variables **********************************/
/******************************************************************************/
#if TSK_LEARN_DATA_TEST_ENABLE
static UW numberOfLearnData;
static ID s_idTskLearData;
#endif

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
#if TSK_LEARN_DATA_TEST_ENABLE
    UH index;

    /* create data for three figures with red, blue and green */
    memset(&ldRegFingerRed, 0, sizeof(SvLearnData));
    ldRegFingerRed.RegYnum = LDATA_REG_FINGER_RED_NUM;
    memset(&ldRegFingerRed.RegImg1[0], LDATA_FINGER_RED_IMG, LDATA_NORM_IMAGE_SIZE);
    memset(&ldRegFingerRed.RegImg2[0], LDATA_FINGER_RED_IMG, LDATA_NORM_IMAGE_SIZE);
    memset(&ldRegFingerRed.MiniImg[0][0], LDATA_FINGER_RED_IMG, LDATA_NORM_IMAGE_SIZE);
    memset(&ldRegFingerRed.MiniImg[1][0], LDATA_FINGER_RED_IMG, LDATA_MINI_IMAGE_SIZE);
    
    memset(&ldRegFingerBlue, 0, sizeof(SvLearnData));
    ldRegFingerBlue.RegYnum = LDATA_REG_FINGER_BLUE_NUM;
    memset(&ldRegFingerBlue.RegImg1[0], LDATA_FINGER_BLUE_IMG, LDATA_NORM_IMAGE_SIZE);
    memset(&ldRegFingerBlue.RegImg2[0], LDATA_FINGER_BLUE_IMG, LDATA_NORM_IMAGE_SIZE);
    memset(&ldRegFingerBlue.MiniImg[0][0], LDATA_FINGER_BLUE_IMG, LDATA_NORM_IMAGE_SIZE);
    memset(&ldRegFingerBlue.MiniImg[1][0], LDATA_FINGER_BLUE_IMG, LDATA_MINI_IMAGE_SIZE);
    
    memset(&ldRegFingerGreen, 0, sizeof(SvLearnData));
    ldRegFingerGreen.RegYnum = LDATA_REG_FINGER_GREEN_NUM;
    memset(&ldRegFingerGreen.RegImg1[0], LDATA_FINGER_GREEN_IMG, LDATA_NORM_IMAGE_SIZE);
    memset(&ldRegFingerGreen.RegImg2[0], LDATA_FINGER_GREEN_IMG, LDATA_NORM_IMAGE_SIZE);
    memset(&ldRegFingerGreen.MiniImg[0][0], LDATA_FINGER_GREEN_IMG, LDATA_NORM_IMAGE_SIZE);
    memset(&ldRegFingerGreen.MiniImg[1][0], LDATA_FINGER_GREEN_IMG, LDATA_MINI_IMAGE_SIZE);
    
    for(index=0; index<LDATA_REG_FIGURE_NBR_MAX; index++)
    {
        SearchLearDataResult[index][0] = 0;
        SearchLearDataResult[index][1] = 0;
        SearchLearDataResult[index][2] = 0;
        SearchLearDataResultPtr[index][0] = &SearchLearDataResult[index][0];
        SearchLearDataResultPtr[index][1] = &SearchLearDataResult[index][1];
        SearchLearDataResultPtr[index][2] = &SearchLearDataResult[index][2];
    }
#endif
}

#if TSK_LEARN_DATA_TEST_ENABLE
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
	ER ercd;
    UW index;
    int result;
    int randomRegFigureNum;
    ldataTestCaseType testCase;
    SvLearnData *learnDataPtr;
    SvLearnData learnData;
    
    TaskLearnDataInit();
    
    numberOfLearnData = 0;
    testCase = LDATA_ADD_CASE;      /* get character */
    if(InitBankArea(BANK1) == 0) {
        while(TRUE) {
            switch(testCase) {
                case LDATA_ADD_CASE:
                    randomRegFigureNum = (numberOfLearnData % (LDATA_REG_FINGER_NUM-1)) + 1;
                    if(randomRegFigureNum == LDATA_REG_FINGER_RED_NUM) {
                        learnDataPtr = &ldRegFingerRed;
                    } else if(randomRegFigureNum == LDATA_REG_FINGER_BLUE_NUM) {
                        learnDataPtr = &ldRegFingerBlue;
                    } else if(randomRegFigureNum == LDATA_REG_FINGER_GREEN_NUM) {
                        learnDataPtr = &ldRegFingerGreen;
                    }
                    
                    learnDataPtr->RegRnum = (UH)(numberOfLearnData % LDATA_REG_NBR_MAX);
                    
                    result = AddSvLearnImg(learnDataPtr);
                    if(result == 0) {
                        numberOfLearnData++;
                        if(numberOfLearnData > (UW)FRAME_NUM_IN_ADD_CASE) {
                            testCase = LDATA_SEARCH_CASE;
                        }
                    }else{
                        /* show test case fail */
                        while(TRUE) {dly_tsk( 20/MSEC );}
                    }
                break;
                case LDATA_SEARCH_CASE:
                    result = InitLearnInfo(BANK1, APARTMENT_TYPE);
                    if(result == 0) {
                        result = SearchLearnImg(LDATA_REG_NBR_MAX, SearchLearDataResultPtr);
                        if(result == 0) {
                            /* show test case pass and show least room */
                            testCase = LDATA_RING_CASE;
                        }
                    }
                    
                    /* add learn data of green to prepare for test case: only one data */
                    if(result == 0) {
                        learnDataPtr = &ldRegFingerGreen;
                        learnDataPtr->RegRnum = (UH)(numberOfLearnData % LDATA_REG_NBR_MAX);
                        result = AddSvLearnImg(learnDataPtr);
                        if(result == 0) {
                            numberOfLearnData++;
                        }
                    }
                    
                    if(result != 0) {
                        /* show test case fail */
                        while(TRUE) { dly_tsk( 20/MSEC ); }
                    }
                break;
                case LDATA_RING_CASE:
                    randomRegFigureNum = (numberOfLearnData % (LDATA_REG_FINGER_NUM-1)) + 1;
                    if(randomRegFigureNum == LDATA_REG_FINGER_RED_NUM) {
                        learnDataPtr = &ldRegFingerRed;
                    } else if(randomRegFigureNum == LDATA_REG_FINGER_BLUE_NUM) {
                        learnDataPtr = &ldRegFingerBlue;
                    } else if(randomRegFigureNum == LDATA_REG_FINGER_GREEN_NUM) {
                        learnDataPtr = &ldRegFingerGreen;
                    }
                    
                    learnDataPtr->RegRnum = (UH)(numberOfLearnData % LDATA_REG_NBR_MAX);
                    
                    result = AddSvLearnImg(learnDataPtr);
                    if(result == 0) {
                        numberOfLearnData++;
                        if(numberOfLearnData >= (UW)FRAME_NUM_IN_MAX_TEST) {
                            result = AddSvLearnImg(learnDataPtr);
                            if(result==0) {
                                numberOfLearnData++;
                                if(numberOfLearnData >= (UW)FRAME_NUM_IN_MAX_TEST) {
                                    testCase = LDATA_NOTIFY_CASE;
                                }
                            }
                        }
                    }
                    
                    if(result != 0) {
                        /* show test case fail */
                        while(TRUE) { dly_tsk( 20/MSEC ); }
                    }
                break;
                case LDATA_NOTIFY_CASE:
                    if((numberOfLearnData % FRAME_NUM_IN_MAX_TEST) < FRAME_NUM_IN_ONLY_1DATA_CASE) {
                        randomRegFigureNum = (numberOfLearnData % (LDATA_REG_FINGER_NUM-1)) + 1;
                        if(randomRegFigureNum == LDATA_REG_FINGER_RED_NUM) {
                            learnDataPtr = &ldRegFingerRed;
                        } else if(randomRegFigureNum == LDATA_REG_FINGER_BLUE_NUM) {
                            learnDataPtr = &ldRegFingerBlue;
                        } else if(randomRegFigureNum == LDATA_REG_FINGER_GREEN_NUM) {
                            learnDataPtr = &ldRegFingerGreen;
                        }
                        
                        learnDataPtr->RegRnum = (UH)(numberOfLearnData % LDATA_REG_NBR_MAX);
                        
                        result = AddSvLearnImg(learnDataPtr);
                        if(result==0) {
                            numberOfLearnData++;
                        }else{
                            /* show test case fail */
                            while(TRUE) {dly_tsk( 20/MSEC );}
                        }
                    } else {
                        /* add this learn data into flash, it happened notify the only one data */
                        ldRegFingerGreen.RegRnum = (UH)(rand() % LDATA_REG_NBR_MAX);
                        result = AddSvLearnImg(&ldRegFingerGreen);
                        if(result == 0) {
                            numberOfLearnData++;
                            testCase = LDATA_UNKNOW_CASE;
                            /* show test case pass */
                        }else{
                            /* show test case fail */
                            while(TRUE) {dly_tsk( 20/MSEC );}
                        }
                    }
                break;
                default:
                    while(TRUE) {dly_tsk( 20/MSEC );}
                break;
            }
            
            dly_tsk( 100/MSEC );
        }
    }

    while(TRUE)
    {
        dly_tsk( 100/MSEC );
    }
}
#endif

/*************************** The End ******************************************/