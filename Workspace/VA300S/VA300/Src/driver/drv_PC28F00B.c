/**
*	VA-300�v���O����
*
*	@file drv_PC28F00B.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/13
*	@brief  Micron��PC28F00BM29EWHA�p�h���C�o
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_PC28F00B_C_
#include <kernel.h>
#include <string.h>
#include <machine.h>
#include "drv_PC28F00B.h"
#include "drv_tim.h"

#pragma section FL

// �������ڑ����@�ňقȂ�
#define	FL_ADDR1	(FL_ADDR + 0xAAAA)
#define	FL_ADDR2	(FL_ADDR + 0x5554)

// �t���b�V���u�[�g�̈��`
enum FLASH_BOOT {						///< �u�[�g�̈��`
	FLASH_BOOT_BOTTOM = 0,				///< BOTTOM�u�[�g�̈�
	FLASH_BOOT_MIDDLE,					///< ��ʗ̈�
	FLASH_BOOT_TOP,						///< TOP�u�[�g�̈�
	
	FLASH_BOOT_MAX
};

// �t���b�V���������u���b�N��`
typedef struct {						///< �u���b�N���\����
	UW Size;							///< 1�u���b�N�̃T�C�Y
	UW Count;							///< �u���b�N��
}ST_BLK;

// �t���b�V���������}�b�v��`
#define	FLASH_BOTTOM_SIZE 	0x20000		///< �{�g���̈��1�u���b�N�̃T�C�Y
#define	FLASH_BOTTOM_CNT	2048		///< �{�g���̈�̃u���b�N��
#define	FLASH_MIDDLE_SIZE 	0			///< �ʏ�̈��1�u���b�N�̃T�C�Y
#define	FLASH_MIDDLE_CNT	0			///< �ʏ�̈�̃u���b�N��
#define	FLASH_TOP_SIZE 		0			///< �g�b�v�̈��1�u���b�N�̃T�C�Y
#define	FLASH_TOP_CNT		0			///< �g�b�v�̈�̃u���b�N��

static const struct stFlashMap{
	ST_BLK Boot[ FLASH_BOOT_MAX ];		///< �u�[�g�̈��`
} s_stFlashMap = {
	{ FLASH_BOTTOM_SIZE, FLASH_BOTTOM_CNT,
	  FLASH_MIDDLE_SIZE, FLASH_MIDDLE_CNT,
	  FLASH_TOP_SIZE,    FLASH_TOP_CNT}
};

static const int BlkSize = FLASH_BOTTOM_CNT + FLASH_MIDDLE_CNT + FLASH_TOP_CNT;
static const UW s_uwFlashSize = FLASH_BOTTOM_SIZE * FLASH_BOTTOM_CNT
								+ FLASH_MIDDLE_SIZE * FLASH_MIDDLE_CNT
								+ FLASH_TOP_SIZE * FLASH_TOP_CNT;

#define	USER_AREA_END		(0x400000 / 0x20000 - 1)	// ���ۂɂ�USER_AREA_END-1�u���b�N�܂ŏ����E�����݂����

// �v���g�^�C�v�錾
static ER fdErase(UW uwAddr);				///< 1�u���b�N����
static ER fdWriteWord(volatile UH*, UH);	///< 1���[�h������

static void SecErase(volatile UH *);		///< �Z�N�^����
static void WordPrg(volatile UH *, UH);		///< PROGRAM�R�}���h
static void ReadReset(volatile UH *);		///< ���[�h���Z�b�g
static void IdRead(void);					///< ID�Ǐo��
static UW fdGetErsAddr(UW);					///< �����u���b�N�̃A�h���X(����)�����߂�
static UW Offset(UH);						///< �I�t�Z�b�g�Z�o
static UW fdBlkSize(UH);					///< �u���b�N�T�C�Y�Z�o
static int fdVerify(UH volatile *, UH);		///< �x���t�@�C

//=============================================================================
/**
 * �t���b�V���������̃u���b�N����
 * @param uwAddr �����A�h���X(�u���b�N�擪)
 * @return �G���[�R�[�h
 */
//=============================================================================
ER FdErase(UW uwAddr)
{
	ER ercd;
	UINT psw;
	
	psw = vdis_psw();				// �����݋֎~
	ercd = fdErase(uwAddr);			// ����
	vset_psw(psw);					// �����݋֎~����

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
UW FdWrite(UW uwFp, UH *puhBp, UW n)
{
	UH	*puhAddr, *puhStart;
	UH	uhData;
	ER	ercd;
	UINT imask;
	int i, j;
	UB ledon;
	
	i = 0;
	j = 0;
	ledon = 0;
	
	if( !(uwFp & 0x01)) {						// �A�h���X�`�F�b�N
		puhAddr = (UH*)(FL_ADDR + (uwFp & (FL_AREA_SZ - 1)));
		puhStart = puhAddr;

		while( n ) {
			n--;
			if (puhAddr < (UH*)(FL_ADDR + FL_AREA_SZ)) {
				uhData = *puhBp++;
				if (uhData != 0xFFFF) {
					imask = get_imask();			// �����݋֎~
					set_imask(15);
					ercd = fdWriteWord(puhAddr, uhData);
					set_imask(imask);				// �����݋֎~����
					if (ercd == E_OK) {
						i += 2;
						j += 1;
						if( j >= 6400 ){	//12800 == �摜1��
							j = 0;
							if(ledon == 1){
								LedOut(0x04, 0x00);
								ledon = 0;
							}else{
								LedOut(0x04, 0x01);
								ledon = 1;
							}
						}
					} else {
						break;
					}
				}
				puhAddr++;
				if ((UW)puhAddr & 0xFFF) {
					continue;
				} else {
					ReadReset(puhStart);
					dly_tsk( 10/MSEC);				// �����݊����҂�
				}
			} else {								// �����G���[
				
			}
		}
		ReadReset(puhStart);
	} else {										// �����G���[
		
	}
	return i;
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
ER FdRead(UW uwFp, UH *puhBp, UW n)
{
	UH	*puhAddr;
	
	if( !(uwFp & 0x01)) {						// �A�h���X�`�F�b�N
		puhAddr = (UH*)(FL_ADDR + (uwFp & (FL_AREA_SZ - 1)));
		ReadReset(puhAddr);						// ���[�h���Z�b�g

		while( n ) {
			n--;
			if (puhAddr < (UH*)(FL_ADDR + FL_AREA_SZ)) {
				*puhBp = *puhAddr;
				puhBp++;
				puhAddr++;
				if ((UW)puhAddr & 0xFFF) {
					continue;
				} else {
					dly_tsk( 10/MSEC);			// �����݊����҂�
				}
			} else {							// �����G���[
				return E_PAR;
			}
		}
	} else {									// �����G���[
		return E_PAR;
	}
	return E_OK;
}

//=============================================================================
/**
 * �t���b�V���������̃u���b�N����
 * @param uwAddr �����A�h���X(�u���b�N�擪)
 * @return �G���[�R�[�h
 */
//=============================================================================
static ER fdErase(UW uwAddr)
{
	UW	uwErsAddr;
	int	iRetry, iStat;
	UW	uwTmout;
	
	iRetry = 0;									// ���g���C�񐔏�����
	
	// �t���b�V���������̏����R�}���h
	if (uwAddr < s_uwFlashSize) {
		uwErsAddr = FL_ADDR + fdGetErsAddr( uwAddr );
		dly_tsk( 10/MSEC);
		ReadReset((UH*)uwErsAddr );				// ���[�h���Z�b�g
		dly_tsk( 10/MSEC);
		SecErase((UH*)uwErsAddr );				// �����R�}���h���s
	} else {									// �w��A�h���X�G���[
		return E_PAR;
	}
	
	// �^�C�}�ݒ�
	uwTmout = ERS_TMOUT;
	
	// ���������҂�
	while( uwTmout ) {
		// �u���b�N�̏�����Ԋm�F
		iStat = fdVerify((UH*)uwErsAddr, 0xFFFF);
		if (iStat == 1) {
			ReadReset((UH*)uwErsAddr );
			return E_OK;
		} else if (iStat) {
			if (iRetry >= 2) {
				break;
			} else {
				iRetry++;						// ���g���C�񐔃C���N�������g
				SecErase((UH*)uwErsAddr);		// �����R�}���h���s
				uwTmout = ERS_TMOUT;			// �^�C�}�ݒ�
			}
		}
		uwTmout--;
	}
	ReadReset((UH*)uwErsAddr );					// ���[�h���Z�b�g
	
	return E_TMOUT;
}

//-----------------------------------------------------------------------------
// 1���[�h������
//-----------------------------------------------------------------------------
static ER fdWriteWord(volatile UH* puhAddr, UH uhData)
{
	int iStat;
	UW	uwTmout;

	WordPrg( puhAddr, uhData);		// �����݃R�}���h���s
	
	uwTmout = PRG_TMOUT;			// �^�C�}�ݒ�
	
	while( (iStat = fdVerify(puhAddr, uhData)) == 0) {
		if( uwTmout) {
//			dly_tsk( 1/MSEC);
			uwTmout--;
			continue;
		}
		// �����݃^�C���A�E�g
		ReadReset( puhAddr );
		return E_TMOUT;
	}
	
	if (iStat == 1) {				// �����ݐ���
		return E_OK;
	}
	// �����ݎ��s
	return E_OBJ;
}

//=============================================================================
// [�@�\] �s�R�}���h�̎��s
// [����] �Ȃ�
// [�ߒl] �Ȃ�
// [�⑫] ���ɂȂ�
//=============================================================================
void CmdTest(void)
{
	*((UH*)(FL_ADDR + 0x00AAAA)) = 0xAA;		// �f�[�^��������
	*((UH*)(FL_ADDR + 0x005554)) = 0x55;
	*((UH*)(FL_ADDR + 0x00AAAA)) = 0xA0;
	*((UH*)(FL_ADDR + 0x000000)) = 0x1234;
}

//-----------------------------------------------------------------------------
// �Z�N�^�����J�n
//-----------------------------------------------------------------------------
static void SecErase(volatile UH *SecAddr)
{
	UINT psw;
	int i;
	
//	psw = vdis_psw();							// �����݋֎~

//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR1) = 0xAA;			// �������݃T�C�N�� UNLOCK
//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR2) = 0x55;
//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR1) = 0x80;			// �Z�b�g�A�b�v�R�}���h
//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR1) = 0xAA;			// �������݃T�C�N�� UNLOCK
//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR2) = 0x55;
//	for(i=0;i<100;i++)	{ nop(); }
	*SecAddr = 0x30;							// �Z�N�^��������
//	for(i=0;i<100;i++)	{ nop(); }
	
//	vset_psw( psw );							// �����݋֎~��߂�
}

//-----------------------------------------------------------------------------
// PROGRAM�R�}���h
//-----------------------------------------------------------------------------
static void WordPrg(volatile UH *pPrgAddr, UH uhData)
{
	*((volatile UH*)FL_ADDR1) = 0xAA;			// �f�[�^��������
	*((volatile UH*)FL_ADDR2) = 0x55;
	*((volatile UH*)FL_ADDR1) = 0xA0;
	*pPrgAddr = uhData;
}

//-----------------------------------------------------------------------------
// ���[�h���Z�b�g
//-----------------------------------------------------------------------------
static void ReadReset(volatile UH *pSecAddr)
{
	*pSecAddr = 0xFF;							// ���[�h���Z�b�g����
}

//-----------------------------------------------------------------------------
// ID READ�R�}���h
//-----------------------------------------------------------------------------
static void IdRead(void)
{
	*((volatile UH*)FL_ADDR1) = 0xAA;				// ID Read�R�}���h
	*((volatile UH*)FL_ADDR2) = 0x55;
	*((volatile UH*)FL_ADDR1) = 0x90;
}

//-----------------------------------------------------------------------------
// �����u���b�N�̃A�h���X(����)�����߂�
//-----------------------------------------------------------------------------
static UW fdGetErsAddr(UW uwAddr)
{
	UW ulBlk;
	
	for(ulBlk = 0;ulBlk < (BlkSize - 1);ulBlk++) {
		if (uwAddr < Offset( ulBlk + 1 )) {
			return Offset( ulBlk );
		}
	}
	
	return 0;
}

//-----------------------------------------------------------------------------
// �I�t�Z�b�g�����߂�
//-----------------------------------------------------------------------------
static UW Offset(UH blkno)
{
	UW	ofs;
	
	ofs = 0;									// �I�t�Z�b�g������
	if( blkno-- < 1) return 0;					// �I�t�Z�b�g0�̌���
	do { ofs += fdBlkSize( blkno ); } while( blkno--);
	return ofs;
}

//-----------------------------------------------------------------------------
// �u���b�N�T�C�Y�����߂�
//-----------------------------------------------------------------------------
static UW fdBlkSize(UH blkno)
{
	int iBoot;
	
	for (iBoot = FLASH_BOOT_BOTTOM;iBoot < FLASH_BOOT_MAX;iBoot++) {
		if (blkno < s_stFlashMap.Boot[ iBoot ].Count) {
			return s_stFlashMap.Boot[ iBoot ].Size;
		}
	}
	
	return 0;
}

//-----------------------------------------------------------------------------
// �t���b�V���q�n�l����/�����ݏI������
//-----------------------------------------------------------------------------
static int fdVerify(UH volatile *pAddr, UH data)
{
	UB	flg;

	flg = *pAddr;									// �V�[�P���X�t���O�Q��
	if (!((flg ^ data) & 0x80)) return 1;			// ����/���� ����
	if (! (flg & 0x20))			return 0;			// ����/���� ����
	flg = *pAddr;									// �V�[�P���X�t���O�Č���
	if (!((flg ^ data) & 0x80)) return 1;			// ����/���� ����
	return 999;										// ����/���� ���s
}

//-----------------------------------------------------------------------------
// �t���b�V���q�n�l�T�C�Y
//-----------------------------------------------------------------------------
UW FdFlAllSize(void)
{
	return s_uwFlashSize;
}

#pragma section
