/******************************************************************************/
/* File Name       : frameworkConf.h                                          */
/* Description     : This file is header to configuration framework layer in  */
/*                   firmware on TBSL1 platform.                              */
/* Author          : TanTrinh                                                 */
/* Created         : April 15, 2018                                           */
/* Modified by     : Tan Trinh                                                */
/* Date Modified   : April 15, 2018                                           */
/* Content Modified:                                                          */
/* Copyright (c) 2015 SH Consuting K.K.                                       */
/*  and SH Consulting VietNam. All Rights Reserved.                           */
/******************************************************************************/
#ifndef FRAMEWORK_CONF_H
#define FRAMEWORK_CONF_H

/******************************************************************************/
/*************************** Standard Header **********************************/
/******************************************************************************/
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/******************************************************************************/
/*************************** Project Header ***********************************/
/******************************************************************************/
#include "kernel.h"

/******************************************************************************/
/*************************** Macro Definitions ********************************/
/******************************************************************************/

/******************************************************************************/
/************* Group Definition for System Data Component *********************/
/******************************************************************************/
#define FWK_CFG_LEARN_DATA_ENABLE   TRUE
#if FWK_CFG_LEARN_DATA_ENABLE
#define FWK_LDDATA_SEM_ENABLE       FALSE
#define FWK_LDDATA_VERIFY_ENABLE    FALSE
#endif

#endif /* FRAMEWORK_CONF_H */
/*************************** The End ******************************************/