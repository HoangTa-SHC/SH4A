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
#define TSK_LEARN_DATA_TEST_ENABLE  FALSE
#endif

/******************************************************************************/
/************************ Enumerations Definitions ****************************/
/******************************************************************************/


/******************************************************************************/
/*************************** Structures Definitions ***************************/
/******************************************************************************/


/******************************************************************************/
/*************************** Export Functions *********************************/
/******************************************************************************/
void TaskLearnDataInit(void);
TASK TaskLearnData(void);

#endif /* TASK_LEARN_DATA_H */
/*************************** The End ******************************************/