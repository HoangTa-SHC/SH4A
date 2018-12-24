//=============================================================================
/**
*	VA-300�v���O����
*
*	@file command.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/8/24
*	@brief  �ʐM�R�}���h��`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
//=============================================================================
#include <ctype.h>
#include <machine.h>
#include <string.h>
#include <stdarg.h>

#ifndef	_COMMAND_H_
#define	_COMMAND_H_

//=== ��`
// �^
#define	FMT_LEN_SIZE	2					///< �f�[�^��
#define	FMT_CMD_SIZE	3					///< �R�}���h�ԍ��T�C�Y
#define	FMT_BLKNO_SIZE	4					///< �u���b�N�ԍ��T�C�Y
#define	FMT_DATA_SIZE	2					///< �f�[�^�����̍Œ���̃f�[�^�T�C�Y(�������A���ۂ͉ϒ�)
typedef struct {							///< �R�}���h�t�H�[�}�b�g�\����
	UB		ubType;							///< �菇�^�C�v
	UB		ubLen[ FMT_LEN_SIZE ];			///< �f�[�^��
	UB		ubOrg;							///< ���M��
	char	cCmd[ FMT_CMD_SIZE ];			///< �R�}���h
	char	cBlkNo[ FMT_BLKNO_SIZE ];		///< �u���b�N�ԍ�
	char	cData[ FMT_DATA_SIZE ];			///< �f�[�^�T�C�Y
} ST_CMDFMT;

// �����R�[�h
#define CODE_EOT	0x04					///< EOT
#define CODE_ACK	0x06					///< ACK
#define CODE_CR		0x0D					///< CR
#define CODE_LF		0x0A					///< LF
#define CODE_NACK	0x15					///< NACK

// �w�b�_
enum HEADER_CODE {
	HC_ERR = -1,							///< �K�薳
	HC_A = '#',								///< �菇A
	HC_B,									///< �菇B
	HC_C,									///< �菇C
	HC_D,									///< �菇D
	HC_E,									///< �菇E
	
};

// ���M�����
enum HOST_TYPE {
	HOST_PC = '0',							///< PC����̑��M
	HOST_TERM1,								///< �[��1����̑��M
	HOST_TERM2,								///< �[��2����̑��M
	
};

/// �R�}���h
// (�ʐM�R�}���h)
// ���ʃR�}���h
#define	CMD_REQ 			000				///< �R�}���h�₢���킹(�[���������)
#define	CMD_RES_T			001				///< �����R�}���h(�[���������)
#define	CMD_RES_C			002				///< �����R�}���h(����Ձ��[��)
// �d��ON�Ə���������
#define	CMD_POW_ON			010				///< �d��ON�ʒm(����Ձ��[��)
#define	CMD_POW_OFF			011				///< �d��OFF�ʒm(����Ձ��[��)
#define	CMD_CMR_DEF			012				///< �J�����E�p�����[�^�̏����l���M(����Ձ��[��)
#define	CMD_LGT_DEF			013				///< LED���ʐ��l�̏����l���M(����Ձ��[��)
#define	CMD_PRM_DEF			014				///< �ݒ�f�[�^�̏����l���M(����Ձ��[��)
#define	CMD_CND_DEF			015				///< �B�������̏����l���M(����Ձ��[��)

// �����e�i���X�E���[�h
#define	CMD_MD_MNT_T		100				///< ���[�h�ؑ֒ʒm(�[���������)
#define	CMD_MD_MNT_C		101				///< ���[�h�ؑ֒ʒm(����Ձ��[��)
#define	CMD_LCD_REQ			102				///< LCD��ʕ\���f�[�^�]���J�n�v��(�[���������)
#define	CMD_LCD_WRT			103				///< LCD��ʕ\���f�[�^�̃u���b�N�]��(����Ձ��[��)
#define	CMD_MID_SET			104				///< �ێ�ID�o�^�E�ύX(�[���������)
#define	CMD_MID_REQ			105				///< �ێ�ID�Q�Ɨv��(�[���������)
#define	CMD_MID_SND			106				///< �ێ�ID���M(����Ձ��[��)
#define	CMD_PRM_INI			107				///< �ݒ�f�[�^�������v��(�[���������)
#define	CMD_PRM_SET			108				///< �ݒ�f�[�^�̓o�^�E�ύX(�[���������)
#define	CMD_PRM_REQ			109				///< �ݒ�f�[�^�̎Q�Ɨv��(�[���������)
#define	CMD_PRM_SND			110				///< �ݒ�f�[�^�̑��M(����Ձ��[��)
#define	CMD_RGST_INI		111				///< �o�^�f�[�^�������v��(�[���������)
#define	CMD_LOG_INI			112				///< �����f�[�^�������v��(�[���������)
#define	CMD_CND_INI			120				///< �摜�B�e�����������v��(�[���������)
#define	CMD_CND_DAT			121				///< �摜�B�e�����������f�[�^���M(����Ձ��[��)
#define	CMD_CND_TST			122				///< �摜�B�e�����m�F�J�n(�[���������)
#define	CMD_CND_SND			123				///< �摜�B�e�����ʒm(����Ձ��[��)
#define	CMD_PIC_SND			124				///< �B�e�摜���M(�[���������)
#define	CMD_CND_SET			125				///< �摜�B�e�����X�V(����Ձ��[��)

// �ʏ탂�[�h
#define	CMD_MD_NML_T		200				///< ���[�h�ؑ֒ʒm(�[���������)
#define	CMD_MD_NML_C		201				///< ���[�h�ؑ֒ʒm(����Ձ��[��)
#define	CMD_RGST_REQ		202				///< �o�^�f�[�^�̎Q�Ɨv��(�[���������)
#define	CMD_RGST_SND		203				///< �o�^�f�[�^�̑��M(����Ձ��[��)
#define	CMD_RGST_SET		204				///< �o�^�f�[�^�̓o�^(�[���������)
#define	CMD_JDG_SND			205				///< ���茋�ʒʒm(����Ձ��[��)
#define	CMD_CMR_RTRY		206				///< �w�f�[�^�̍ĎB�e�v��(����Ձ��[��)
#define	CMD_CMR_SET			207				///< �J�����E�p�����[�^�̕ύX�v��(����Ձ��[��)
#define	CMD_LGT_SET			208				///< LED���ʕύX�v��(����Ձ��[��)
#define	CMD_RGST_DEL		209				///< �o�^�f�[�^�̍폜�v��(�[���������)
#define	CMD_FIN_SND			210				///< �w�f�[�^�̑��M(�[���������)
#define	CMD_LCD_LOCK		230				///< �J���E���ʒm(����Ձ��[��)
#define	CMD_LCD_DOOR		231				///< �h�A�J�ʒm(����Ձ��[��)
#define	CMD_BUZ_REQ			232				///< �u�U�[ON/OFF�v��(����Ձ��[��)
#define	CMD_TST_REQ			250				///< ���Ȑf�f�J�n�v��
#define	CMD_TST_SND			251				///< ���Ȑf�f�摜(�q�X�g�O����)���M(�[���������)
#define	CMD_TST_RSLT		252				///< ���Ȑf�f���ʑ��M(����Ձ��[��)
#define	CMD_ERR_SND			260				///< �G���[����(�[���������)
#define	CMD_ERR_REQ			261				///< �G���[�\���v��(����Ձ��[��)
#define	CMD_ERR_CLR			262				///< �G���[�����v��(����Ձ��[��)
#define	CMD_ENO_SET			270				///< �ً}�ԍ��o�^�E�ύX(�[���������)
#define	CMD_ENO_REQ			271				///< �ً}�ԍ��Q�Ɨv��(�[���������)
#define	CMD_ENO_SND			272				///< �ً}�ԍ��f�[�^���M(����Ձ��[��)
#define	CMD_RND_REQ			273				///< �ً}�J���p�����v��(�[���������)
#define	CMD_RND_SND			274				///< �ً}�J���p�������M(����Ձ��[��)
#define	CMD_OPN_SND			275				///< �J���ԍ����͎�t(�[���������)
#define	CMD_ENO_CHK			276				///< �ً}�ԍ����͎�t(�[���������)
#define	CMD_EMG_RSLT		277				///< �ً}�J�����۔���ʒm(����Ձ��[��)
#define	CMD_EMP_REQ			280				///< �ދ���ԊJ�n�v��(�[���������)
#define	CMD_EMPID_SND		281				///< �ދ���ԊJ���̕ێ�ID���͒ʒm(�[���������)
#define	CMD_LCD_EMP			282				///< �ދ���ԊJ���ʒm(����Ձ��[��)
// ��d���[�h
#define	CMD_MD_BAT_C		301				///< ���[�h�ؑ֒ʒm(����Ձ��[��)
#define	CMD_POW_OFF_T		302				///< �d��OFF�v��(�[���������)
#define	CMD_POW_OFF_C		303				///< �d��OFF�v��(����Ձ��[��)
// ��펞�J�����[�h
#define	CMD_MD_PNC_C		401				///< ���[�h�ؑ֒ʒm(����Ձ��[��)
#define	CMD_LCD_PNC			402				///< ��펞�J���ʒm(����Ձ��[��)


/// �Œ�f�[�^
// �R�}���h���
#define	CMD_GEN_DAT			"  "			///< �X�y�[�X
#define	CMD_GEN_OK			"OK"			///< OK
#define	CMD_GEN_NG			"NG"			///< NG

// �����R�}���h
#define	CMD_RES_OK			"OK"			///< OK
#define	CMD_RES_NG			"NG"			///< NG
// �w�f�[�^�̔��茋��
#define	CMD_JDG_SND_OK		"OK"			///< OK
#define	CMD_JDG_SND_NG		"NG"			///< NG
// �J���E���ʒm
#define	CMD_LCD_LOCK_ON		"ON"			///< ON
#define	CMD_LCD_LOCK_OFF	"OFF"			///< OFF
// �h�A�J�ʒm
#define	CMD_LCD_DOOR_OPEN	"OPEN"			///< OPEN
#define	CMD_LCD_DOOR_CLOSE	"CLOSE"			///< CLOSE
// �u�U�[ON�EOFF�v��
#define	CMD_BUZ_REQ_ON		"ON"			///< ON
#define	CMD_BUZ_REQ_OFF		"OFF"			///< OFF
// ���Ȑf�f���ʑ��M
#define	CMD_TST_RSLT_OK		"OK"			///< OK
#define	CMD_TST_RSLT_NG		"NG"			///< NG
// �ً}�J�����۔���ʒm
#define	CMD_EMG_RSLT_OK		"OK"			///< OK
#define	CMD_EMG_RSLT_NG		"NG"			///< NG
// �ދ���ԊJ���ʒm
#define	CMD_LCD_EMP_OPEN	"OPEN"			///< OPEN
#define	CMD_LCD_EMP_CLOSE	"CLOSE"			///< CLOSE

#endif										// end of _COMMAND_H_
