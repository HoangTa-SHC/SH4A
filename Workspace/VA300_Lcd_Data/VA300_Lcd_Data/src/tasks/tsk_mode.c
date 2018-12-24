/**
*	VA-300�v���O����
*
*	@file tsk_mode.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/27
*	@brief  ���[�h����^�X�N(ELM-10���痬�p)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>

#include "kernel.h"
#include "id.h"
#include "err_ctrl.h"
#include "command.h"

#include "drv_7seg.h"
#include "drv_led.h"
#include "drv_dsw.h"
#include "drv_rtc.h"

#include "va300.h"

// ��`
enum MD_LAN_CMD_STATUS {
	MD_LAN_CMD_NONE,						///< �R�}���h�����M���
	MD_LAN_CMD_ACK_WAIT,					///< ACK�҂�
	MD_LAN_CMD_RES_WAIT,					///< �����R�}���h�҂����

};

// �ϐ���`
static enum MODE_STATUS s_eMachineMode;		// ��ԕϐ�
static enum SUBMODE_STATUS s_eMainteSubMode;// �����e�i���X���[�h�̃T�u���
static enum SUBMODE_STATUS s_eNormalSubMode;// �ʏ탂�[�h�̃T�u���
static enum MD_LAN_CMD_STATUS s_eLanCmdSubMode;// LAN�R�}���h���[�h�̃T�u���

// �v���g�^�C�v�錾
	//���[�h����
static void PowerOnMode(void);				// �p���[�I�����[�h
static void MainteMode(void);				// �����e�i���X���[�h
static void NormalMode(void);				// �ʏ탂�[�h
static void PowerFailMode(void);			// ��d�����샂�[�h
static void PanicOpenMode(void);			// ��펞�J�����[�h
static void SelfTestMode(void);				// �Z���t�e�X�g���
static void ErrorMode(void);				// �G���[�������

	//���[�h�����i�T�u�j
static void MdPowerOnModeLanCmd(T_MDCMDMSG *cmdmsg);// �p���[�I�����[�h��LAN�R�}���h����
static void MdPowerOnModeMon(T_MDCMDMSG *cmdmsg);	// �p���[�I�����[�h�̃��j�^�R�}���h����
static void MdMainteModeLanCmd(T_MDCMDMSG *cmdmsg);	// �����e�i���X���[�h��LAN�R�}���h����
static void MdMainteModeMon(T_MDCMDMSG *cmdmsg);	// �����e�i���X���[�h�̃��j�^�R�}���h����
static void MdNormalModeLanCmd(T_MDCMDMSG *cmdmsg);	// �ʏ탂�[�h��LAN�R�}���h����
static void MdNormalModeIo(T_MDCMDMSG *cmdmsg);		// �ʏ탂�[�h��I/O����
static void mdNormalModeMon(T_MDCMDMSG *cmdmsg);	// �ʏ탂�[�h�̃��j�^�R�}���h����
static void MdNormalRegMode(T_MDCMDMSG *pCmdMsg);	// �o�^���[�h
static void MdNormalDelMode(T_MDCMDMSG *pCmdMsg);	// �폜���[�h
static void mdPowerFailModeLan(T_MDCMDMSG *cmdmsg);	// ��d�����샂�[�h��LAN�R�}���h����
static void mdPowerFailModeIo(T_MDCMDMSG *cmdmsg);	// ��d�����샂�[�h��I/O����
static void mdPanicOpenModeLan(T_MDCMDMSG *cmdmsg);	// �p�j�b�N�I�[�v�����[�h��LAN�R�}���h����
static void mdPanicOpenModeIo(T_MDCMDMSG *cmdmsg);	// �p�j�b�N�I�[�v�����[�h��I/O����
static void mdPanicOpenMon(T_MDCMDMSG *cmdmsg);		// �p�j�b�N�I�[�v�����[�h�̃��j�^�R�}���h����

static void MdChangeStatus(enum MODE_STATUS nextMode);	//���[�h�J��

/*==========================================================================*/
/**
 *	�}�V����Ԃ�Ԃ��i���[�h�Q�Ɓj
 *	@return �}�V���̏��
 */
/*==========================================================================*/
enum MODE_STATUS MdGetMachineMode(void)
{
	return s_eMachineMode;
}

/*==========================================================================*/
/**
 *	���[�h�J��
 *	@param eNextMode ���̏��
 */
/*==========================================================================*/
static void MdChangeStatus(enum MODE_STATUS eNextMode)
{
	//�ޏꏈ��
	switch (s_eMachineMode) {
	case MD_POWER_ON:

		break;
	case MD_NORMAL:						//�F�؃��[�h

		break;
	}//switch
	
	s_eMachineMode = eNextMode;			//���̃��[�h�ݒ�
	
	//���ꏈ��
	switch (s_eMachineMode) {
	case MD_INITIALIZE:					//������
		break;		

	case MD_MAINTENANCE:
		break;
		
	case MD_NORMAL:						//�F�؃��[�h

		break;
	}
	
	//LED�\��
	LedOut(LED_ERR,  LED_OFF);
	switch (s_eMachineMode) {
	case MD_PANIC:
		LedOut(LED_ERR,  LED_ON);
		break;
	}
}


/*==========================================================================*/
/**
 * ���[�h�Ǘ��^�X�N
 *
 */
/*==========================================================================*/
TASK ModeTask(void)
{
	s_eMachineMode = MD_POWER_ON;
	
	for(;;) {
		switch(s_eMachineMode) {
		
		case MD_POWER_ON:			// �d��ON�i�����l)
			PowerOnMode();
			break;
			
		case MD_MAINTENANCE:		// �����e�i���X���[�h
			MainteMode();
			break;
			
		case MD_NORMAL:				// �ʏ탂�[�h
			NormalMode();
			break;
			
		case MD_POWER_FAIL:			// ��d�����샂�[�h
			PowerFailMode();
			break;
			
		case MD_PANIC:				// ��펞�J�����[�h
			PanicOpenMode();
			break;
			
		case MD_SELFTEST:			// �Z���t�e�X�g���
			SelfTestMode();
			break;
			
		case MD_ERROR:				// �G���[���
			ErrorMode();
			break;
		
		default:					// ���肦�Ȃ�
			PrgErrSet();
			break;
		}
	}
}

/*==========================================================================*/
/* �p���[�I�����[�h�i�����ʉ�)												*/
/*==========================================================================*/
static void PowerOnMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;

	// ����������
	MdChangeStatus(MD_POWER_ON);
	
	
	
	// LCD�����\��
	
	
	
	while(s_eMachineMode == MD_POWER_ON) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				MdPowerOnModeLanCmd(msg);
				break;
				
			case TSK_IO:				// �Ԉ����I/O���͂������Ƃ��̏���
				break;
				
			case TSK_CMD_MON:
				MdPowerOnModeMon(msg);
				break;
				
			default:					// ���肦�Ȃ�
				PrgErrSet();
				break;
			}
			rel_mpf(MPF_COM, (VP)msg);
		}
	}
}

/*==========================================================================
	�p���[�I�����[�h��LAN�R�}���h����
============================================================================*/
static void MdPowerOnModeLanCmd(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case CMD_MD_MNT_C:					// �����e�i���X���[�h�ֈڍs�v��
		MdChangeStatus(MD_MAINTENANCE);	// �����e�i���X���[�h�ֈڍs
		break;
	
	case CMD_MD_BAT_C:					// ��d���[�h�ֈڍs�v��
		MdChangeStatus(MD_POWER_FAIL);	// ��d���[�h�ֈڍs
		break;

	case MD_CMD_POL_REQ:				// ����Ղւ̃|�[�����O�v��
		
		break;
		
	case MD_CMD_RCV:					// �R�}���h��M�ʒm
		break;
		
	default:
		break;
	}//switch (msg->command) {
}

/*==========================================================================
	�p���[�I�����[�h�̃��j�^�R�}���h����
============================================================================*/
static void MdPowerOnModeMon(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case CMD_MD_MNT_C:					// �����e�i���X���[�h�ֈڍs�v��
		MdChangeStatus(MD_MAINTENANCE);	// �����e�i���X���[�h�ֈڍs
		break;
	
	case CMD_MD_BAT_C:					// ��d���[�h�ֈڍs�v��
		MdChangeStatus(MD_POWER_FAIL);	// ��d���[�h�ֈڍs
		break;

	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/*==========================================================================*/
/* �����e�i���X���[�h														*/
/*==========================================================================*/
static void MainteMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;

	s_eMainteSubMode = SUB_MD_SETTING;		

	while(s_eMachineMode == MD_MAINTENANCE) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(s_eMainteSubMode) {
			case SUB_MD_SETTING:		// �ݒ胂�[�h
				MdMainteSetMode(msg);
				break;
				
			case SUB_MD_BLK:			// �u���b�N�]�����[�h
				MdMainteBlkMode(msg);
				break;
				
			case SUB_MD_INIT_REGIST:	// �����o�^���[�h
				MdMainteInitRegistMode(msg);
				break;
				
			default:					// ���肦�Ȃ�
				PrgErrSet();
				break;
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/*==========================================================================*/
/* �ʏ탂�[�h																*/
/*==========================================================================*/
/*===========================================================================
	�F�؊J�n�w�߂�����ƁA�F�؂��J�n����
=============================================================================*/
static void NormalMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	// �T�u���[�h��F�؃��[�h��
	s_eNormalSubMode = SUB_MD_AUTHENTICATE;
	
	while(s_eMachineMode == MD_NORMAL) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(s_eNormalSubMode) {
			case SUB_MD_AUTHENTICATE:			// �F�؃��[�h
				MdNormalAutMode(msg);
				break;
				
			case SUB_MD_REGIST:					// �o�^���[�h
				MdNormalRegMode(msg);
				break;
				
			case SUB_MD_DELETE:					// �폜���[�h
				MdNormalDelMode(msg);
				break;
				
			default:
				
				break;
			}//switch (msg->idOrg)
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/**
 *	�o�^���[�h
 */
static void MdNormalRegMode(T_MDCMDMSG *pCmdMsg)
{
	
	
	
}

/**
 *	�폜���[�h
 */
static void MdNormalDelMode(T_MDCMDMSG *pCmdMsg)
{
	
	
	
}

/*==========================================================================*/
/* ��d���샂�[�h															*/
/*==========================================================================*/
static void PowerFailMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	while(s_eMachineMode == MD_POWER_FAIL) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				mdPowerFailModeLan(msg);
				break;
			case TSK_IO:
				mdPowerFailModeIo(msg);
				break;
				
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}//while(iMachineMode == MD_RUN) {
}

/*==========================================================================*/
/*	��d�����샂�[�h��LAN�R�}���h����										*/
/*==========================================================================*/
static void mdPowerFailModeLan(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	default:
		break;
	}//switch
}

/*==========================================================================*/
/*	��d�����샂�[�h��I/O����												*/
/*==========================================================================*/
static void mdPowerFailModeIo(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	default:
		break;
	}//switch
}

/*==========================================================================*/
/* ��펞�J�����[�h															*/
/*==========================================================================*/
static void PanicOpenMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	while(s_eMachineMode == MD_PANIC) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				mdPanicOpenModeLan(msg);
				break;
			case TSK_IO:
				break;
							
			case TSK_CMD_MON:
				mdPanicOpenModeMon(msg);
				break;
			
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/*==========================================================================
	�p�j�b�N�I�[�v�����[�h��LAN�R�}���h����
============================================================================*/
static void mdPanicOpenModeLan(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {

	default:
		break;
	}//switch
}

/*==========================================================================
	�p�j�b�N�I�[�v�����[�h��I/O����
============================================================================*/
static void mdPanicOpenModeIo(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {

	default:
		break;
	}//switch
}

/*==========================================================================
	�p�j�b�N�I�[�v�����[�h�̃��j�^�R�}���h����
============================================================================*/
static void mdPanicOpenMon(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {

	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/*==========================================================================*/
/* �Z���t�e�X�g���[�h														*/
/*==========================================================================*/
static void SelfTestMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	while(s_eMachineMode == MD_SELFTEST) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				
				break;
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/*==========================================================================*/
/* �G���[�������[�h													*/
/*==========================================================================*/
static void ErrorMode(void)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	
	while(s_eMachineMode == MD_ERROR) {
		ercd = rcv_mbx(MBX_MODE, &msg);
		if (ercd == E_OK) {
			switch(msg->idOrg) {
			case TSK_CMD_LAN:
				
				break;
			}
			rel_mpf(MPF_COM, (VP)msg);
		} else {
			ErrCodeSet(ercd);
		}
	}
}

/*==========================================================================
	LAN�R�}���h����
============================================================================*/
static void mdLanCmdProc(T_MDCMDMSG *pCmdMsg) 
{
	switch (s_eLanCmdSubMode) {
	case MD_LAN_CMD_NONE:
		
		break;
		
	case MD_LAN_CMD_ACK_WAIT:
		mdLanCmdProcAckWait(pCmdMsg);
		break;
		
	case MD_LAN_CMD_RES_WAIT:
	
		break;
	
	default:
		break;
	}//switch
}

/**
 * �R�}���h�ɑ΂���ACK��M�҂�
 * 
 * @param @CmdMsg �R�}���h
 */
static void mdLanCmdProcAckWait(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case MD_CMD_ACK_RCV:
		
		break;
	
	default:
		CmmSendNak();					// NAK���M
		break;
	}
}

/**
 * ���[�h�^�X�N�ւ̃��b�Z�[�W���M
 *
 * @param ���M���^�X�NID
 * @param �R�}���h
 * @return �G���[�R�[�h
 */
ER MdSendMsg(ID idOrg, UH uhCmd, UH uhBlkno, B *p, int iCount)
{
	ER			ercd;
	T_MDCMDMSG	*msg;
	int			iSize;
	
	ercd = tget_mpf(MPF_COM, &msg, (100/MSEC));
	if (ercd != E_OK) {
		ErrCodeSet(ercd);		// �G���[����
		return ercd;
	}
	msg->idOrg   = idOrg;				// ���M���^�X�NID
	msg->uhCmd   = uhCmd;				// �R�}���h
	msg->uhBlkNo = uhBlkno;				// �u���b�N�ԍ�
	iSize = iCount;
	if (iSize > sizeof msg->cData) {
		iSize = sizeof msg->cData;
	}
	memcpy(&(msg->cData), p, iSize);
	msg->cnt     = iSize;				// �f�[�^��
	
	//���b�Z�[�W���M
	ercd = snd_mbx(MBX_MODE, (T_MSG*)msg);
	
	return ercd;
}
