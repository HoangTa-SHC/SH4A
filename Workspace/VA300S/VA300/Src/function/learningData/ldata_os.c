/******************************************************************************/
/* File Name       : ldata_os.c                                               */
/* Description     : This file is source code of the learn data component that*/
/*                   will interact between kernel and this component.         */
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
#include "frameworkConf.h"
#if FWK_LDDATA_SEM_ENABLE
#include "kernel.h"

/******************************************************************************/
/*************************** Local Variables **********************************/
/******************************************************************************/
static ID ldataosSemId;
const T_CSEM ldataCsem = {TA_TFIFO, 0, 1};

/******************************************************************************/
/*************************** Functions Definition *****************************/
/******************************************************************************/

/******************************************************************************/
/* Function Name: ldataosInit                                                 */
/* Description  : This function will interact between learn data component and*/
/*                the NORTi kernel to synchronous in multiple accessing.      */
/* Parameter    : None                                                        */
/* Return Value : None                                                        */
/* Remarks      : None                                                        */
/******************************************************************************/
void ldataosInit(void)
{
    ER_ID ercd;
    
    ercd = acre_sem(&ldataCsem);
    if(ercd > 0) {
        ldataosSemId = ercd;
    }
}

/******************************************************************************/
/* Function Name: ldataosGetSemaphore                                         */
/* Description  : This function will wait and get the ready semaphore in the  */
/*                NORTi kernel.                                               */
/* Parameter    : None                                                        */
/* Return Value : None                                                        */
/* Remarks      : None                                                        */
/******************************************************************************/
void ldataosGetSemaphore(void)
{
    wai_sem(ldataosSemId);
}

/******************************************************************************/
/* Function Name: ldataosReleaseSemaphore                                     */
/* Description  : This function will realse semaphore in the NORTi kernel     */
/* Parameter    : None                                                        */
/* Return Value : None                                                        */
/* Remarks      : None                                                        */
/******************************************************************************/
void ldataosReleaseSemaphore(void)
{
    sig_sem(ldataosSemId);
}

#endif /* FWK_LDDATA_SEM_ENABLE */
/*************************** The End ******************************************/