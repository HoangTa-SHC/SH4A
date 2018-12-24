//=============================================================================
/**
 *
 * VA-300�v���O����
 * <<EEPROM�֘A���W���[��>>
 *
 *	@brief EEPROM�֘A�̋@�\�B���Č����痬�p
 *	
 *	@file drv_eep.c
 *	
 *	@date	2012/09/10
 *	@version 1.00 �V�K�쐬
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#include "kernel.h"
#include "lan9118.h"
#include "drv_eep.h"

// �v���g�^�C�v�錾
static ER lan_get_eep_4byte(UH addr, UB *pval);	///< 4�o�C�g��EEPROM�f�[�^�Ǎ���
static ER lan_get_eep_word(UH addr, UH *pval);	///< 2�o�C�g��EEPROM�f�[�^�Ǎ���

//=============================================================================
/**
 * Get 4 byte data which is stored in EEPROM.
 *
 * @param addr EEPROM�A�h���X
 * @param pval �f�[�^�i�[��
 * @retval E_OK ����
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
 * @param addr EEPROM�A�h���X
 * @param pval �f�[�^�i�[��
 * @retval E_OK ����
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
 * @param addr EEPROM�A�h���X
 * @param pval �f�[�^�i�[��
 * @retval E_OK ����
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
 * @param addr EEPROM�̃A�h���X
 * @param pval �Ǎ��݃f�[�^�i�[��
 * @retval E_OK ����
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
 * @param addr EEPROM�̃A�h���X
 * @param pval �Ǎ��݃f�[�^�i�[��
 * @retval E_OK ����
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
 * @param addr EEPROM�̃A�h���X
 * @param val �|�[�g�ԍ�
 * @retval E_OK ����
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
