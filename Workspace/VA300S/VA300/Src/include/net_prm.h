/**
*	VA-300�v���O����
*
*	@file net_prm.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  �l�b�g���[�N�p�����[�^��`���(���Č����痬�p)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_NET_PRM_H_
#define	_NET_PRM_H_

// �l�b�g���[�N�p�����[�^
// �⑫�FMAC�A�h���X���������܂�Ă��Ȃ��Ƃ��ɂ����Őݒ肳��Ă���MAC�A�h���X�̒l��
//       �g�p����܂��B�ʏ�ł̎g�p��NG�ł��B
//       IP�A�h���X�Ȃǂ̓f�t�H���g�ݒ胂�[�h�̂Ƃ��Ɏg�p�����l�ł��B
static const UB ini_mac_addr[] = { 0x00, 0x0C, 0x67, 0x00, 0xB0, 0x02};	///< ���p��MAC�A�h���X
																		// MAC�A�h���X��
																		// 00-0C-67-00-B0-00�`
																		// 00-0C-67-00-B0-0F�܂ł͊J���p
static const UB ini_ipaddr[]   = {  192,  168,   /*50*/ 1,  158};				///< IP�A�h���X
static const UB ini_mask[]     = {  255,  255,  255,    0};				///< �T�u�l�b�g�}�X�N

static const UH default_udp_port = 50000;								///< UDP�̃f�t�H���g�|�[�g
static const UH default_telnet_port = 23;								///< TELNET�̃f�t�H���g�|�[�g

// ���O�C���p�X���[�h�̐ݒ�
#define	LOGIN_ID	"VA300"			///< ���O�C��ID
#define	LOGIN_PASS	"VA300"			///< �p�X���[�h

#endif								// end of _NET_PRM_H_