/**
*	VA-300�v���O����
*
*	@file drv_eep.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/10
*	@brief  EEPROM��`���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_EEP_H_
#define	_DRV_EEP_H_

// �}�N����`
#define	EEP_IP_ADDR		0x40			///< IP�A�h���X
#define	EEP_NET_MASK	0x44			///< �T�u�l�b�g�}�X�N
#define	EEP_PORT_NO		0x48			///< �|�[�g�ԍ�
#define	EEP_TERM_NO		0x4A			///< �[���ԍ�
#define	EEP_CTRL_ADDR	0x50			///< �����IP�A�h���X
#define	EEP_CTRL_PORT	0x54			///< ����Ճ|�[�g�ԍ�

#define	lan_get_ip(n)		lan_get_4byte(EEP_IP_ADDR,n)	///< IP�A�h���X�̓Ǎ���
#define	lan_set_ip(n)		lan_set_4byte(EEP_IP_ADDR,n)	///< IP�A�h���X�̏�����
#define	lan_get_mask(n)		lan_get_4byte(EEP_NET_MASK,n)	///< �T�u�l�b�g�}�X�N�̓Ǎ���
#define	lan_set_mask(n)		lan_set_4byte(EEP_NET_MASK,n)	///< �T�u�l�b�g�}�X�N�̏�����
#define	lan_get_ctrlip(n)	lan_get_4byte(EEP_CTRL_ADDR,n)	///< �����IP�A�h���X�̓Ǎ���
#define	lan_set_ctrlip(n)	lan_set_4byte(EEP_CTRL_ADDR,n)	///< �����IP�A�h���X�̏�����

#define	lan_get_port(n)		lan_get_word(EEP_PORT_NO,n)		///< �|�[�g�ԍ��̓Ǎ���
#define	lan_set_port(n)		lan_set_word(EEP_PORT_NO,n)		///< �|�[�g�ԍ��̏�����
#define	lan_get_ctrlpt(n)	lan_get_word(EEP_CTRL_PORT,n)	///< ����Ճ|�[�g�ԍ��̓Ǎ���
#define	lan_set_ctrlpt(n)	lan_set_word(EEP_CTRL_PORT,n)	///< ����Ճ|�[�g�ԍ��̏�����
#define	lan_get_id(n)		lan_get_word(EEP_TERM_NO,n)		///< �@��ԍ��̓Ǎ���
#define	lan_set_id(n)		lan_set_word(EEP_TERM_NO,n)		///< �@��ԍ��̏�����

// EXTERN�錾�̒�`
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_EEP_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// �v���g�^�C�v�錾
EXTERN ER lan_get_4byte(UH addr, UB *pval);		///< 4�o�C�g�f�[�^�̓Ǐo��
EXTERN ER lan_set_4byte(UH addr, UB *pval);		///< 4�o�C�g�f�[�^�̏�����
EXTERN ER lan_get_word(UH addr, UH *pval);		///< 2�o�C�g�f�[�^�̓Ǐo��
EXTERN ER lan_set_word(UH addr, UH val);		///< 2�o�C�g�f�[�^�̏�����

#endif										/* end of _DRV_EEP_H_				*/
