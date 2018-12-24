/******************************************************************************/
/* File Name       : memInterface.c                                           */
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
#if FWK_CFG_LEARN_DATA_ENABLE

/******************************************************************************/
/*************************** Standard Header **********************************/
/******************************************************************************/
#include <string.h>
#include <stdio.h>

/******************************************************************************/
/*************************** Project Header ***********************************/
/******************************************************************************/
#include "learnData.h"
#include "id.h"

/******************************************************************************/
/*************************** Macro Definitions ********************************/
/******************************************************************************/
#define MI_FLASH_DELAY_TIME     5                   /* in miliseconds */

/******************************************************************************/
/*************************** Local Variables **********************************/
/******************************************************************************/
static UB miBuffer[MI_SECTOR_SIZE];

/******************************************************************************/
/*************************** Functions Prototype ******************************/
/******************************************************************************/


/******************************************************************************/
/*************************** Functions Definition *****************************/
/******************************************************************************/

/******************************************************************************/
/* Function Name: miInit                                                      */
/* Description  : This function will initial the drivers of the flash memory. */
/* Parameter    : None                                                        */
/* Return Value : BOOL - return the TRUE value when initials successful, else */
/*                to return the FALSE value.                                  */
/* Remarks      : None                                                        */
/******************************************************************************/
void miInit(void)
{
    FlInit( SEM_FL );
}

#if FWK_LDDATA_VERIFY_ENABLE
/******************************************************************************/
/* Function Name: miVerifyData                                                */
/* Description  : This function will verify data in flash memory with the     */
/*                bufPtr after write successful data in the bufPtr            */
/* Parameter    : Input: addr - is start address that needs to verify data in */
/*                       flash memory                                         */
/*                Input: buffer - is a pointer that stores data for writing   */
/*                Input: length - is size of data in the bufPtr.              */
/* Return Value : BOOL - return the TRUE value when verifies successful, else */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL miVerifyData(UW addr, UB* buffer, UW length)
{
    BOOL result;
    UW i, size;
    ER errCode;

    result = TRUE;
    for(i = 0; i < length; i += MI_SECTOR_SIZE)
    {
        size = MI_SECTOR_SIZE;
        if((length-i) < MI_SECTOR_SIZE) {
            size = (length-i);
        }
        
        errCode = FlRead(addr + i, (UH*)miBuffer, size/sizeof(UH));
        if(errCode == E_OK) {
            if(memcmp(buffer + i, miBuffer, size)!=0) {
                result = FALSE;
                break;
            }
        }else{
            result = FALSE;
            break;
        }
    }
    
    return result;
}
#endif /* FWK_LDDATA_VERIFY_ENABLE */

/******************************************************************************/
/* Function Name: miRemoveDataInSector                                        */
/* Description  : This function will remove data in a sector on flash memory  */
/* Parameter    : Input: sectorAddr - is address of a sector in flash memory. */
/* Return Value : BOOL - return the TRUE value when erases successful, else to*/
/*                return the FALSE value.                                     */
/* Remarks      : None                                                        */
/******************************************************************************/
BOOL miRemoveDataInSector(UW sectorAddr)
{
    BOOL result;
    volatile UW retryNumber;
    volatile ER errCode;
    
    result = FALSE;
    if((sectorAddr >= MI_FLASH_START) &&
       ((sectorAddr % MI_SECTOR_SIZE) == 0) &&
       (sectorAddr < (MI_FLASH_START + MI_FLASH_SIZE))) {
        retryNumber = 0;
        do {
            errCode = FlErase(sectorAddr);
            if(errCode != E_OK) {
                dly_tsk( 10/MSEC );
                retryNumber++;
            }else{
                memset(miBuffer, 0xFF, MI_SECTOR_SIZE);
                FlWrite(sectorAddr, (UH*)miBuffer, MI_SECTOR_SIZE/sizeof(UH));
            }
        }while((errCode != E_OK) && (retryNumber < LDATA_ERROR_RETRY_NBR));
        if((errCode == E_OK) && (retryNumber < LDATA_ERROR_RETRY_NBR)) {
            result = TRUE;
        }
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: miEraseSector                                               */
/* Description  : This function will erase a sector in the serial flash memory*/
/* Parameter    : Input: sectorAddr - is address of sector in flash memory.   */
/* Return Value : BOOL - return the TRUE value when erases successful, else to*/
/*                return the FALSE value.                                     */
/* Remarks      : None                                                        */
/******************************************************************************/
BOOL miEraseSector(UW sectorAddr)
{
    BOOL result;
    volatile UW retryNumber;
    volatile ER errCode;
    
    result = FALSE;
    if((sectorAddr >= MI_FLASH_START) &&
       ((sectorAddr % MI_SECTOR_SIZE) == 0) &&
       (sectorAddr < (MI_FLASH_START + MI_FLASH_SIZE))) {
        retryNumber = 0;
        do {
            dly_tsk( 10/MSEC );
            errCode = FlErase(sectorAddr);
            if(errCode != E_OK) {
                retryNumber++;
            }
        }while((errCode != E_OK) && (retryNumber < LDATA_ERROR_RETRY_NBR));
        if((errCode == E_OK) && (retryNumber < LDATA_ERROR_RETRY_NBR)) {
            result = TRUE;
        }
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: miWriteInSector                                             */
/* Description  : This function writes data in a sector in serial flash memory*/
/* Parameter    : Input: sectorAddr - is start address of a sector            */
/*                Input: offset - is offset in the sector.                    */
/*                       0 < offset < MI_SECTOR_SIZE                          */
/*                Input: buffer - is a pointer that stores data for writing   */
/*                Input: length - is size of data in the buffer.              */
/* Return Value : BOOL - return the TRUE value when writes successful, else   */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
BOOL miWriteInSector(UW sectorAddr, UW offset, UB* buffer, UW length)
{
    BOOL result;
    ER errCode;
    UW realSize, retryNumber;
    
    result = FALSE;
    if((sectorAddr >= MI_FLASH_START) &&
       ((sectorAddr % MI_SECTOR_SIZE) == 0) &&
       (sectorAddr < (MI_FLASH_START + MI_FLASH_SIZE))) {
        if((buffer != NULL) && (length > 0) && ((offset + length) <= MI_SECTOR_SIZE)) {
            result = TRUE;
        }
    }
    
    if(result) {
        retryNumber = 0;
        do{
            result = miEraseSector(sectorAddr);
            if(result) {
                FlWrite(sectorAddr + offset, (UH*)buffer, length/sizeof(UH));
            }
            
            if(!result) {
                retryNumber++;
                if(retryNumber < LDATA_ERROR_RETRY_NBR) {
                    dly_tsk( 20/MSEC );
                }
            }
        }while((!result) && (retryNumber < LDATA_ERROR_RETRY_NBR));
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: miWriteInSectorBkup                                         */
/* Description  : This function writes data in a sector into the serial flash */
/*                memory and to backup old data in this sector.               */
/* Parameter    : Input: sectorAddr - is start address of a sector            */
/*                Input: offset - is offset in the sector.                    */
/*                       0 < offset < MI_SECTOR_SIZE                          */
/*                Input: buffer - is a pointer that stores data for writing   */
/*                Input: length - is size of data in the buffer.              */
/* Return Value : BOOL - return the TRUE value when writes successful, else   */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL miWriteInSectorBkup(UW sectorAddr, UW offset, UB* buffer, UW length)
{
    BOOL result;
    ER errCode;
    volatile UW realSize, retryNumber;
    
    result = FALSE;
    if((sectorAddr >= MI_FLASH_START) &&
       ((sectorAddr % MI_SECTOR_SIZE) == 0) &&
       (sectorAddr < (MI_FLASH_START + MI_FLASH_SIZE))) {
        if((buffer != NULL) && (length > 0) && ((offset + length) <= MI_SECTOR_SIZE)) {
            result = TRUE;
        }
    }
    
    if(result) {
        result = (FlRead(sectorAddr, (UH*)miBuffer, MI_SECTOR_SIZE/sizeof(UH)) == E_OK);
        if(result) {
            memcpy(miBuffer + offset, buffer, length);
            retryNumber = 0;
            do{
                result = miEraseSector(sectorAddr);
                if(result) {
                    realSize = FlWrite(sectorAddr, (UH*)&miBuffer[0], MI_SECTOR_SIZE/sizeof(UH));
                }
                
                if(!result) {
                    retryNumber++;
                    if(retryNumber < LDATA_ERROR_RETRY_NBR) {
                        dly_tsk( 20/MSEC );
                    }
                }
            }while((!result) && (retryNumber < LDATA_ERROR_RETRY_NBR));
        }
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: miWriteSector                                               */
/* Description  : This function writes a sector data into serial flash memory */
/* Parameter    : Input: secAddr - is start address of a sector that is value */
/*                       to be multiple of the sector size.                   */
/*                Input: buffer - is a pointer that stores data for writing   */
/*                Input: length - is size of data in the bufPtr.              */
/* Return Value : BOOL - return the TRUE value when writes successful, else to*/
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL miWriteSector(UW secAddr, UB* buffer, UW length)
{
    BOOL result;
    ER errCode;
    UW retryNumber;
    
    result = FALSE;
    if ((secAddr >= MI_FLASH_START) &&
       ((secAddr % MI_SECTOR_SIZE) == 0) &&
       (secAddr < (MI_FLASH_START + MI_FLASH_SIZE))) {
        if((buffer != NULL) && (length > 0) && (length <= MI_SECTOR_SIZE)) {
            result = TRUE;
        }
    }
    
    if(result) {
        retryNumber = 0;
        do{
            result = miEraseSector(secAddr);
            if(result) {
                FlWrite(secAddr, (UH*)buffer, length/sizeof(UH));
            }
            
            if(!result) {
                retryNumber++;
                if(retryNumber < LDATA_ERROR_RETRY_NBR) {
                    dly_tsk( 20/MSEC );
                }
            }
        }while((!result) && (retryNumber < LDATA_ERROR_RETRY_NBR));
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: miWriteMultiSector                                          */
/* Description  : This function writes sectors data into serial flash memory  */
/* Parameter    : Input: addr - is start address of a sector that is value to */
/*                       be multiple of the sector size.                      */
/*                Input: buffer - is a pointer that stores data for writing   */
/*                Input: length - is size of data in the bufPtr.              */
/* Return Value : BOOL - return the TRUE value when writes successful, else   */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
static BOOL miWriteMultiSector(UW addr, UB* buffer, UW length)
{
    BOOL result;
    UW index, size;
    
    result = FALSE;
    if ((addr >= MI_FLASH_START) && ((addr % MI_SECTOR_SIZE) == 0)) {
        if( (buffer != NULL) && (length > 0) &&
            ((addr + length) <= (MI_FLASH_START + MI_FLASH_SIZE))) {
            result = TRUE;
        }
    }
    
    if(result) {
        for(index = 0; index < length; index += MI_SECTOR_SIZE)
        {
            size = MI_SECTOR_SIZE;
            if((length - index)  <MI_SECTOR_SIZE) {
                size = length - index;
            }
            
            if(size == MI_SECTOR_SIZE) {
                result = miWriteSector(addr + index, buffer + index, size);
            }else{
                result = (FlRead(addr + index,
                                 (UH*)miBuffer, MI_SECTOR_SIZE/sizeof(UH)) == E_OK);
                if(result) {
                    memcpy(miBuffer, buffer + index, size);
                    result = miWriteSector(addr + index,
                                           miBuffer, MI_SECTOR_SIZE);
                }
            }
            
            if(!result) {
                break;
            }
        }
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: miWriteRange                                                */
/* Description  : This function will write data at start address in the flash */
/*                memory.                                                     */
/* Parameter    : Input: addr - is start address that needs to write data into*/
/*                       the flash memory.                                    */
/*                Input: buffer - is a pointer that stores data for writing   */
/*                Input: length - is size of data in the bufPtr.              */
/* Return Value : BOOL - return the TRUE value when writes successful, else to*/
/*                return the FALSE value.                                     */
/* Remarks      : None                                                        */
/******************************************************************************/
BOOL miWriteRange(UW addr, UB* buffer, UW length)
{
    BOOL result;
    UW secAddr, offset, size;
    
    result = FALSE;
    if( (addr >= MI_FLASH_START) &&
        ((addr + length) <= (MI_FLASH_START + MI_FLASH_SIZE))) {
        if((buffer != NULL) && (length > 0)) {
            result = TRUE;
        }
    }
    
    if(result) {
        if((addr % MI_SECTOR_SIZE)==0) {
            result = miWriteMultiSector(addr, buffer, length);
        }else{
            secAddr = (addr/MI_SECTOR_SIZE) * MI_SECTOR_SIZE;
            offset = addr%MI_SECTOR_SIZE;
            
            size = MI_SECTOR_SIZE - offset;
            if((offset + length) <= MI_SECTOR_SIZE) {
                size = length;
            }
            result = miWriteInSectorBkup(secAddr, offset, buffer, size);
            if(result && (size < length)) {
                secAddr += MI_SECTOR_SIZE;
                result = miWriteMultiSector(secAddr,
                                            buffer + size, length - size);
            }
        }
    }
    
#if FWK_LDDATA_VERIFY_ENABLE
    if(result) {
        result = miVerifyData(addr, buffer, length);
    }
#endif

    return result;
}

/******************************************************************************/
/* Function Name: miReadRange                                                 */
/* Description  : This function will read data at start address from flash.   */
/* Parameter    : Input: addr - is start address that needs to read data from */
/*                       flash memory                                         */
/*                Input: buffer - is a pointer that stores data for reading   */
/*                Input: length - is size that reads data.                    */
/* Return Value : BOOL - return the TRUE value when reads successful, else to */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
BOOL miReadRange(UW addr, UB* buffer, UW length)
{
    BOOL result;
    ER errCode;
    
    result = FALSE;
    if( (addr >= MI_FLASH_START) &&
        ((addr + length) <= (MI_FLASH_START + MI_FLASH_SIZE))) {
        if((buffer != NULL) && (length > 0)) {
            result = TRUE;
        }
    }
    
    if(result) {
        errCode = FlRead(addr, (UH*)buffer, length/sizeof(UH));
        if(errCode != E_OK) {
            result = FALSE;
        }
    }
    
    return result;
}

/******************************************************************************/
/* Function Name: miReadHword                                                 */
/* Description  : This function will read half word data from flash memory.   */
/* Parameter    : Input: addr - is start address that needs to read data from */
/*                       flash memory. value of this is nulti half word.      */
/*                Input: buffer - is a pointer that stores data for reading   */
/*                Input: length - is size that reads data.                    */
/* Return Value : BOOL - return the TRUE value when reads successful, else to */
/*                return FALSE value                                          */
/* Remarks      : None                                                        */
/******************************************************************************/
BOOL miReadHword(UW addr, UH* halfWord)
{
    BOOL result;
    
    result = FALSE;
    if((addr >= MI_FLASH_START) && (addr < (MI_FLASH_START + MI_FLASH_SIZE))) {
        if((halfWord != NULL) && ((addr % sizeof(UH)) == 0)) {
            result = (FlRead(addr, halfWord, 1) == E_OK);
        }
    }
    
    return result;
}

#endif /* FWK_CFG_LEARN_DATA_ENABLE */
/*************************** The End ******************************************/