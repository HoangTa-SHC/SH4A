//=============================================================================
/**
*	VA-300�v���O����
*
*	@file drv_flash.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  �t���b�V��������������
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
//=============================================================================
#include <kernel.h>
#include <string.h>
#include "drv_flash.h"

#pragma section FL

// �t���b�V���������}�b�v��`
#define	USER_AREA_END		(0x400000 / 0x20000 - 1)	///< ���ۂɂ�USER_AREA_END-1�u���b�N�܂ŏ����E�����݂����

// �ϐ���`
static ID s_IdSem;							///< �t���b�V���������̃Z�}�t�H

// �v���g�^�C�v�錾
static ER flErase(UW uwAddr);				///< �t���b�V���������̏���
static UW flWrite(UW uwFp, UH *puhBp, UW n);///< �t���b�V���������̏�����
static ER flRead(UW uwFp, UH *puhBp, UW n);	///< �t���b�V���������̓Ǎ���

#pragma section
// �Z�}�t�H��`(�Z�N�V������ʏ�ɂ��Ă���)
const T_CSEM csem_fl = { TA_TFIFO, 1, 1, (B *)"sem_fl" };

/*==========================================================================*/
/**
 * �t���b�V���������h���C�o������
 * @param id �t���b�V���������p�Z�}�t�HID
 */
/*==========================================================================*/
void FlInit(ID id)
{
	ER ercd; 

	s_IdSem = 0;
	
	// �Z�}�t�H�̐���
	if (id > 0) {
		ercd = cre_sem(id, &csem_fl);	
		if (ercd == E_OK) {
			s_IdSem = id;
		}
	} else {
		ercd = acre_sem(&csem_fl);
		if (ercd > 0) {
			s_IdSem = ercd;
		}
	}
}

#pragma section FL
//=============================================================================
/**
 * �t���b�V���������̃u���b�N����
 * @param uwAddr �����A�h���X(�u���b�N�擪)
 * @return �G���[�R�[�h
 */
//=============================================================================
ER FlErase(UW uwAddr)
{
	ER ercd;
	
	ercd = twai_sem(s_IdSem, (100/MSEC));	// �Z�}�t�H�̎擾
	
	if (ercd == E_OK) {
		ercd = flErase(uwAddr);				// ����
		
		sig_sem(s_IdSem);					// �Z�}�t�H�ԋp
	}

	return ercd;
}

//=============================================================================
/**
 * �t���b�V���������̃u���b�N����(�{��)
 * @param uwAddr �����A�h���X(�u���b�N�擪)
 * @return �G���[�R�[�h
 */
//=============================================================================
static ER flErase(UW uwAddr)
{
	ER ercd;
	UH uhBank;
	
	uhBank = FlBankCalc(uwAddr);
	// �A�N�Z�X�E�B���h�E�ؑ�
	FlBankSet( uhBank );
	
	ercd = FdErase((uwAddr & (FL_AREA_SZ - 1)));
	
	
	return ercd;
}


//=============================================================================
/**
 * �t���b�V���������̏�����
 * @param uwFp �t���b�V���������̃|�C���^
 * @param puhBp �f�[�^�i�[�o�b�t�@�̃|�C���^
 * @param n �����݂����f�[�^��
 * @return �����ݐ����f�[�^��
 */
//=============================================================================
UW FlWrite(UW uwFp, UH *puhBp, UW n)
{
	ER	ercd;
	UW uwSize;
	
	uwSize = 0;
	
	ercd = twai_sem(s_IdSem, (100/MSEC));	// �Z�}�t�H�̎擾
	
	if (ercd == E_OK) {
		uwSize = flWrite(uwFp, puhBp, n);
		sig_sem(s_IdSem);					// �Z�}�t�H�ԋp
	}
	
	return uwSize;
}

//=============================================================================
/**
 * �t���b�V���������̏�����(�{��)
 * @param uwFp �t���b�V���������̃|�C���^
 * @param puhBp �f�[�^�i�[�o�b�t�@�̃|�C���^
 * @param n �����݂����f�[�^��
 * @return �����ݐ����f�[�^��
 */
//=============================================================================
static UW flWrite(UW uwFp, UH *puhBp, UW n)
{
	UW uwSize;
	UW uwCnt;
	UH uhBank;
	UH uwEndSize;
	
	uwEndSize = 0;						// �����݊��������N���A
	
	while(uwFp < FdFlAllSize()) {
		uhBank = FlBankCalc(uwFp);
		uwCnt = n;						// �܂��͎w��񐔂ɐݒ�
		// �����A�N�Z�X�E�B���h�E��
		if (uhBank != FlBankCalc((uwFp + 2 * n - 1))) {
			// ����łȂ��Ƃ��̓A�N�Z�X�E�B���h�E�ŏI�܂ł̉񐔂ɐݒ�
			uwCnt = (FL_AREA_SZ - (uwFp & (FL_AREA_SZ - 1))) / 2;
		}
		uwSize = uwCnt << 1;
		
		// �A�N�Z�X�E�B���h�E�ؑ�
		FlBankSet( uhBank );
		
		// ������
		if (FdWrite((uwFp & (FL_AREA_SZ - 1)), puhBp, uwCnt) == uwSize) {
			uwFp      += uwSize;
			puhBp     += uwCnt;
			uwEndSize += uwSize;
			n -= uwCnt;
			if (n > 0) {
				continue;
			}
		}
		break;
	}
	
	return uwEndSize;
}

//=============================================================================
/**
 * �t���b�V���������̓Ǎ���
 * @param uwFp �t���b�V���������̃|�C���^
 * @param puhBp �f�[�^�i�[�o�b�t�@�̃|�C���^
 * @param n �Ǎ��݂����f�[�^��
 * @return �G���[�R�[�h
 */
//=============================================================================
ER FlRead(UW uwFp, UH *puhBp, UW n)
{
	ER ercd;
	
	ercd = twai_sem(s_IdSem, (100/MSEC));	// �Z�}�t�H�̎擾
	
	if (ercd == E_OK) {
		ercd = flRead(uwFp, puhBp, n);
		sig_sem(s_IdSem);					// �Z�}�t�H�ԋp
	}

	return ercd;
}

//=============================================================================
/**
 * �t���b�V���������̓Ǎ���(�{��)
 * @param uwFp �t���b�V���������̃|�C���^
 * @param puhBp �f�[�^�i�[�o�b�t�@�̃|�C���^
 * @param n �Ǎ��݂����f�[�^��
 * @return �G���[�R�[�h
 */
//=============================================================================
static ER flRead(UW uwFp, UH *puhBp, UW n)
{
	UW uwCnt;
	UH uhBank;
	ER ercd;
	
	ercd = E_OK;
	
	while(uwFp < FdFlAllSize()) {
		uhBank = FlBankCalc(uwFp);
		uwCnt = n;				// �܂��͎w��񐔂ɐݒ�
		// �����A�N�Z�X�E�B���h�E��
		if (uhBank != FlBankCalc((uwFp + 2 * n - 1))) {
			// ����łȂ��Ƃ��̓A�N�Z�X�E�B���h�E�ŏI�܂ł̉񐔂ɐݒ�
			uwCnt = (FL_AREA_SZ - (uwFp & (FL_AREA_SZ - 1))) / 2;
		}
		// �A�N�Z�X�E�B���h�E�ؑ�
		FlBankSet( uhBank );
		
		// ������
		ercd = FdRead((uwFp & (FL_AREA_SZ - 1)), puhBp, uwCnt);
		if (ercd == E_OK) {
			uwFp  += (uwCnt << 1);	// �Ǎ��݃A�h���X�C���N�������g
			puhBp += uwCnt;			// �����݃A�h���X�C���N�������g
			n -= uwCnt;				// �c��f�[�^�����Z
			if (n > 0) {			// �c��f�[�^������Ȃ珈���𑱂���
				continue;
			}
		}
		break;
	}
	return ercd;
}
