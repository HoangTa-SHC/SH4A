/**
*	VA-300�v���O����
*
*	@file sub_aut.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/27
*	@brief  �F�؏���
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>

#include "kernel.h"
#include "id.h"
#include "message.h"
#include "err_ctrl.h"
#include "command.h"

#include "drv_7seg.h"
#include "drv_led.h"
#include "drv_dsw.h"
#include "drv_rtc.h"

// ��`
enum MD_AUT_STATUS {						///< �F�؃X�e�[�^�X
	MD_AUT_INP_NUMBER,						///< ���L���̂Ƃ��̔ԍ����͑҂�
	MD_AUT_INP_WAIT,						///< �F�؂̓��͑҂�
	MD_AUT_SND_DAT_ACK_WAIT,				///< �w�f�[�^���MACK��M�҂�
	MD_AUT_JDG_WAIT,						///< �����M�҂�
	MD_AUT_PRM_WAIT,						///< �p�����[�^�̕ύX�v���҂�
	MD_AUT_CMR_RTRY,						///< �ĎB�e�v���҂�
	MD_AUT_EOT_WAIT,						///< EOT��M�҂�
	
	MD_AUT_RES_WAIT,						///< ���X�|���X�҂�
};

// �ϐ���`
static enum MD_AUT_STATUS s_eAutMode;		// �F�؃��[�h���
static int s_iBlkNo;						// �u���b�N�ԍ�
static int s_iRetry;						// ���L���̏ꍇ�̃��g���C��

// �v���g�^�C�v�錾
static void mdAutChangeStatus(enum MD_AUT_STATUS eNextMode);	// �F�؃��[�h�̏�ԑJ��

	//���[�h�����i�T�u�j
static void mdNormalAutModeInputWaitLanCmd(T_MDCMDMSG *pCmdMsg);// ���͑҂�(LAN�R�}���h)
static void mdNormalAutModeInputWaitIo(T_MDCMDMSG *pCmdMsg);	// ���͑҂�(I/O)
static void mdNormalAutModeInputWaitMon(T_MDCMDMSG *pCmdMsg);	// ���͑҂�(���j�^)
static void mdNormalAutModeSndDatAckWait(T_MDCMDMSG *pCmdMsg);	// ACK�҂����
static void mdNormalAutModeResWait(T_MDCMDMSG *pCmdMsg);		// �����R�}���h�҂����
static void mdNormalAutModeSndBlkAckWait(T_MDCMDMSG *pCmdMsg);	// �u���b�N�]��ACK�҂����
static void mdNormalAutModeJdgWait(T_MDCMDMSG *pCmdMsg);		// ����҂����
static void mdNormalAutModeParamWait(T_MDCMDMSG *pCmdMsg);		// �p�����[�^�ύX�҂����
static void mdNormalAutModeRetryWait(T_MDCMDMSG *pCmdMsg);		// �ĎB�e�v���҂����
static void mdNormalAutModeEotWait(T_MDCMDMSG *pCmdMsg);		// EOT��M�҂����

/*==========================================================================*/
/**
 *	�F�؃��[�h�̏�ԏ�����
 */
/*==========================================================================*/
void MdNormalAutInit(void)
{
	if (IsTypeCommon() == TRUE) {		// ���L�������̂Ƃ�
		s_iRetry = ;
		mdAutChangeStatus(MD_AUT_INP_NUMBER);
	} else {							// ��L�^�C�v
		s_iRetry = 1;
		mdAutChangeStatus(MD_AUT_INP_WAIT);
	}
}

/*==========================================================================*/
/**
 *	�F�؃��[�h�̏�ԑJ��
 *	@param eNextMode ���̏��
 */
/*==========================================================================*/
static void mdAutChangeStatus(enum MD_AUT_STATUS eNextMode)
{
	//�ޏꏈ��
	switch (s_eAutMode) {
	case MD_AUT_INP_WAIT:

		break;
	case MD_AUT_RES_WAIT:				//

		break;
	case MD_AUT_SND_DAT_ACK_WAIT:
	
		break;
	}//switch
	
	s_eAutMode = eNextMode;				//���̃��[�h�ݒ�
	
	//���ꏈ��
	switch (s_eAutMode) {
	case MD_AUT_INP_WAIT:				//
		break;		

	case MD_AUT_RES_WAIT:
		break;
		
	case MD_AUT_SND_DAT_ACK_WAIT:		//

		break;
	}
}

/*==========================================================================*/
/* �ʏ탂�[�h���̃T�u���[�h													*/
/*==========================================================================*/
/**
 *	�F�؃��[�h
 */
void MdNormalAutMode(T_MDCMDMSG *pCmdMsg)
{
	switch( s_eAutMode ) {
	case MD_AUT_INP_NUMBER:						// �L�[���͑҂����
		mdNormalAutModeInputNumber(pCmdMsg);
		break;
	
	case MD_AUT_INP_WAIT:						// ���͑҂����
		mdNormalAutModeInputWait(pCmdMsg);
		break;
	
	case MD_AUT_SND_DAT_ACK_WAIT:				// �w�f�[�^��ACK��M�҂�
		mdNormalAutModeSndDatAckWait(pCmdMsg);
		break;
	
	case MD_AUT_RES_WAIT:						// �����R�}���h�҂�
		mdNormalAutModeResWait(pCmdMsg);
		break;
	
	case MD_CHK_SND_BLK_ACK_WAIT:				// �w�f�[�^��ACK��M�҂�
		mdNormalAutModeSndBlkAckWait(pCmdMsg);
		break;
		
	case MD_AUT_JDG_WAIT:						// �w�f�[�^�̔����M�҂����
		mdNormalAutModeJdgWait(pCmdMsg);
		break;
		
	case MD_AUT_PRM_WAIT:						// �p�����[�^�ύX�҂����
		mdNormalAutModeParamWait(pCmdMsg);
		break;
		
	case MD_AUT_CMR_RTRY:						// �ĎB�e�v���҂�
		mdNormalAutModeRetryWait(pCmdMsg);
		break;
		
	case MD_AUT_EOT_WAIT:						// EOT�҂�
		mdNormalAutModeEotWait(pCmdMsg);
		break;
		
	default:									// �����G���[
	
		break;
	}
}

/**
 *	�F�؃��[�h��ID���͑҂����
 */
static void mdNormalAutModeInputNumber(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		mdNormalAutModeInputWaitLanCmd(pCmdMsg);
		break;
	
	case TSK_IO:
		mdNormalAutModeInputNumberIo(pCmdMsg);
		break;
		
	case TSK_CMD_MON:
		mdNormalAutModeMon(pCmdMsg);
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/*==========================================================================
	���͑҂��̏���
	�EI/O�^�X�N����̗v������������
============================================================================*/
static void mdNormalAutModeInputNumberIo(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case MD_CMD_KEY_ON:
	
		break;
		
	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/**
 *	�F�؃��[�h�̑҂����
 */
static void mdNormalAutModeInputWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		mdNormalAutModeInputWaitLanCmd(pCmdMsg);
		break;
	
	case TSK_IO:
		mdNormalAutModeInputWaitIo(pCmdMsg);
		break;
		
	case TSK_CMD_MON:
		mdNormalAutModeInputWaitMon(pCmdMsg);
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/*==========================================================================
	���͑҂��̏���
	�ELAN�R�}���h����̗v������������
============================================================================*/
static void mdNormalAutModeInputWaitLanCmd(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case MD_CMD_POL_REQ:
		LanReqSend();					// �₢���킹�R�}���h���s
		break;
		
	default:
		CmmSendNak();					//�R�}���h�ُ�
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/*==========================================================================
	���͑҂��̏���
	�EI/O�^�X�N����̗v������������
============================================================================*/
static void mdNormalAutModeInputWaitIo(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {
	case MD_CMD_FIN_SET:
			// �B�e����
		LanFingerDataSend();							// �w�摜�f�[�^�̑��M�R�}���h
		mdAutChangeStatus(MD_AUT_SND_DAT_ACK_WAIT);		// �w�f�[�^��ACK��M�҂���
		break;
		
	case MD_CMD_KEY_ON:
	
		break;
		
	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/*==========================================================================
	���͑҂��̏���
	�E���j�^�R�}���h����̗v������������
============================================================================*/
static void mdNormalAutModeInputWaitMon(T_MDCMDMSG *pCmdMsg) 
{
	switch (pCmdMsg->uhCmd) {

		break;	

	default:
		break;
	}//switch (pCmdMsg->uhCmd) {
}

/**
 *	�F�؃��[�h��Ack�҂����
 */
static void mdNormalAutModeSndDatAckWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case MD_CMD_ACK_RCV:
			mdAutChangeStatus(MD_AUT_RES_WAIT);		// �����҂���Ԃ�
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// ���͑҂���Ԃ�
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	�F�؃��[�h�̉����R�}���h�҂����
 */
static void mdNormalAutModeResWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case CMD_RES_C:
			if (!memcmp(pCmdMsg->cData, CMD_RES_OK, sizeof CMD_RES_OK)) {		// OK�̂Ƃ�
				s_iBlkNo = ;		// �u���b�N�ԍ���ݒ�
				CmmSendFinData( s_iBlkNo );				// �摜�f�[�^�𑗐M
				mdAutChangeStatus(MD_AUT_RES_WAIT);		// �����҂���Ԃ�
			} else if (!memcmp(pCmdMsg->cData, CMD_RES_NG, sizeof CMD_RES_NG)) {	// NG�̂Ƃ�
				// LCD�ɔF�؂ł��Ȃ��\��
				
				MdNormalAutInit();						// ���͑҂���Ԃ�
			} else {					// �ُ�f�[�^
				CmmSendNak();			// 
			}
			break;
			
		case MD_CMD_EOT_RCV:
			MdNormalAutInit();							// ���͑҂���Ԃ�
			break;

		default:						// ���肦�Ȃ��͂�
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	�F�؃��[�h��Ack�҂����
 */
static void mdNormalAutModeSndBlkAckWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case MD_CMD_ACK_RCV:
			if ( s_iBlkNo ) {						// ���M�f�[�^����Ȃ�
				CmmSendFinData( s_iBlkNo );			// �摜�f�[�^�𑗐M
			} else {								// �ŏI�u���b�N���M����������
				mdAutChangeStatus(MD_AUT_JDG_WAIT);	// �w�f�[�^�̔����M�҂���Ԃ�
			}
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// ���͑҂���Ԃ�
			break;
			
		case MD_CMD_EOT_RCV:
			MdNormalAutInit();						// ���͑҂���Ԃ�
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	�F�؃��[�h�̔���҂����
 */
static void mdNormalAutModeJdgWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case CMD_JDG_SND:
			if (!memcmp(pCmdMsg->cData, CMD_JDG_SND_OK, sizeof CMD_JDG_SND_OK)) {	// OK�̂Ƃ�
				CmmAckSend();									// ACK���M
				();// �F��OK�\��
				mdAutChangeStatus(MD_AUT_EOT_WAIT);				// EOT��M�҂�
			} else if (!memcmp(pCmdMsg->cData, CMD_JDG_SND_NG, sizeof CMD_JDG_SND_NG)) {	// NG�̂Ƃ�
				CmmAckSend();									// ACK���M
				mdAutChangeStatus(MD_AUT_PRM_WAIT);				// EOT��M�҂�
			} else {
				CmmNakSend();									// NAK���M
			}
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();									// ���͑҂���Ԃ�
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	�F�؃��[�h�̃p�����[�^�ύX�҂����
 */
static void mdNormalAutModeParamWait(T_MDCMDMSG *pCmdMsg)
{
	UH uhVal;

	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case CMD_CMR_SET:							// �J�����E�p�����[�^�̕ύX�v��
					// �J�����p�����[�^�̕ύX�w��
			CmmAckSend();							// ACK���M
			LanResSend(PR_TYPE_D, CMD_RES_OK);		// �����R�}���h���M
			mdAutChangeStatus(MD_AUT_CMR_RTRY);		// �ĎB�e�v���҂���
			break;
		
		case CMD_LGT_SET:							// LED���ʕύX�v��
			uhVal = pCmdMsg->cData[ 0 ];
			uhVal <<= 8;
			uhVal |= (UH)pCmdMsg->cData[ 1 ];
			if (uhVal <= 255) {
								// LED���ʕύX
				CmmAckSend();						// ACK���M
				LanResSend(PR_TYPE_D, CMD_RES_OK);	// �����R�}���h���M
				mdAutChangeStatus(MD_AUT_CMR_RTRY);	// �ĎB�e�v���҂���
			} else {
				CmmNakSend();						// NAK���M
			}
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// ���͑҂���Ԃ�
			break;
			
		case MD_CMD_EOT_RCV:
			mMdNormalAutInit();						// ���͑҂���Ԃ�
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	�F�؃��[�h�̍ĎB�e�v���҂�
 */
static void mdNormalAutModeRetryWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case CMD_CMR_RTRY:							// �w�f�[�^�̍ĎB�e�v��
					// �ĎB�e�w��
			CmmAckSend();							// ACK���M
			mdAutChangeStatus(MD_AUT_EOT_WAIT);		// EOT��M�҂���
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// ���͑҂���Ԃ�
			break;

		case MD_CMD_EOT_RCV:
			MdNormalAutInit();						// ���͑҂���Ԃ�
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

/**
 *	�F�؃��[�h��EOT�҂����
 */
static void mdNormalAutModeEotWait(T_MDCMDMSG *pCmdMsg)
{
	switch(pCmdMsg->idOrg) {
	case TSK_CMD_LAN:
		switch (pCmdMsg->uhCmd) {
		case MD_CMD_EOT_RCV:
			MdNormalAutInit();						// ���͑҂���Ԃ�
			break;
		
		case MD_CMD_SND_FAIL:
			MdNormalAutInit();						// ���͑҂���Ԃ�
			break;
		}								// switch (pCmdMsg->uhCmd)
		break;
	
	case TSK_IO:
		break;
		
	case TSK_CMD_MON:
		break;
	
	}//switch (pCmdMsg->idOrg)
}

