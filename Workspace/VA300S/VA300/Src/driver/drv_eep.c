//=============================================================================
/**
 *
 * VA-300プログラム
 * <<EEPROM関連モジュール>>
 *
 *	@brief EEPROM関連の機能。他案件から流用
 *	
 *	@file drv_eep.c
 *	
 *	@date	2012/09/10
 *	@version 1.00 新規作成
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "lan9118.h"
#include "drv_eep.h"

// プロトタイプ宣言
static ER lan_get_eep_4byte(UH addr, UB *pval);	///< 4バイトのEEPROMデータ読込み
static ER lan_get_eep_word(UH addr, UH *pval);	///< 2バイトのEEPROMデータ読込み

//=============================================================================
/**
 * Get 4 byte data which is stored in EEPROM.
 *
 * @param addr EEPROMアドレス
 * @param pval データ格納先
 * @retval E_OK 成功
 */
//=============================================================================
ER lan_get_4byte(UH addr, UB *pval)
{
	ER ercd = -1;
	int i;
	UB ubTmp[ 4 ];
	BOOL bFlg;
	const int nSize = sizeof ubTmp;
	
	bFlg = TRUE;
	while( 1 ) {
		ercd = lan_get_eep_4byte(addr, pval);
		if (ercd != E_OK) {
			return ercd;
		}
		ercd = lan_get_eep_4byte(addr, ubTmp);
		if (ercd != E_OK) {
			return ercd;
		}

		for( i = 0;i < nSize;i++) {
			if (ubTmp[ i ] != pval[ i ]) {
				bFlg = FALSE;
				break;
			}
		}
		if (bFlg == TRUE) {
			break;
		}
	}

	return ercd;
}

//=============================================================================
/**
 * Get 4 byte data which is stored in EEPROM.
 *
 * @param addr EEPROMアドレス
 * @param pval データ格納先
 * @retval E_OK 成功
 */
//=============================================================================
static ER lan_get_eep_4byte(UH addr, UB *pval)
{
	ER ercd = -1;
	int i;
	
	for( i = 0;i < 4;i++) {
		ercd = lan_get_eep((addr + i), &pval[i]);
		if (ercd != E_OK) {
			return ercd;
		}
	}

	return ercd;
}

//=============================================================================
/**
 * Set and store 4 byte data in EEPROM.
 *
 * @param addr EEPROMアドレス
 * @param pval データ格納先
 * @retval E_OK 成功
 */
//=============================================================================
ER lan_set_4byte(UH addr, UB *pval)
{
	ER ercd = -1;
	int i;
	
	for( i = 0;i < 4;i++) {
		ercd = lan_set_eep((addr + i), pval[i]);
		if (ercd != E_OK) {
			return ercd;
		}
	}

	return ercd;
}

//=============================================================================
/**
 * Get Word Data which is stored in EEPROM.
 *
 * @param addr EEPROMのアドレス
 * @param pval 読込みデータ格納先
 * @retval E_OK 成功
 */
//=============================================================================
ER lan_get_word(UH addr, UH *pval)
{
	ER ercd = -1;
	UH	uhTmp;
	
	while( 1 ) {
		ercd = lan_get_eep_word(addr, pval);
		if (ercd != E_OK) {
			return ercd;
		}
		ercd = lan_get_eep_word(addr, &uhTmp);
		if (ercd != E_OK) {
			return ercd;
		}
		if (*pval == uhTmp) {
			break;
		}
	}

	return ercd;
}

//=============================================================================
/**
 * Get Word Data which is stored in EEPROM.
 *
 * @param addr EEPROMのアドレス
 * @param pval 読込みデータ格納先
 * @retval E_OK 成功
 */
//=============================================================================
static ER lan_get_eep_word(UH addr, UH *pval)
{
	ER ercd = -1;
	UB dat;
	
	ercd = lan_get_eep(addr, &dat);
	if (ercd != E_OK) {
		return ercd;
	}
	*pval = ((UH)dat << 8);
	ercd = lan_get_eep((addr + 1), &dat);
	if (ercd != E_OK) {
		return ercd;
	}
	*pval |= dat;

	return ercd;
}

//=============================================================================
/**
 * Set and store Word Data in EEPROM.
 *
 * @param addr EEPROMのアドレス
 * @param val ポート番号
 * @retval E_OK 成功
 */
//=============================================================================
ER lan_set_word(UH addr, UH val)
{
	ER ercd = -1;

	ercd = lan_set_eep(addr, (val >> 8));
	if (ercd != E_OK) {
		return ercd;
	}
	ercd = lan_set_eep((addr + 1), (val & 0xFF));

	return ercd;
}
