/**
*	VA-300�v���O����
*
*	@file sub_set.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/29
*	@brief  �����e�i���X���[�h�̃T�u���[�h(�ݒ�)
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

// �ϐ���`
static enum MODE_STATUS s_eMachineMode;		// ��ԕϐ�
static enum SUBMODE_STATUS s_eNormalSubMode;// �T�u���

// �v���g�^�C�v�錾
	//���[�h����

	//���[�h�����i�T�u�j
static void MdMainteModeLanCmd(ST_COMMAND *cmdmsg);	// �����e�i���X���[�h��LAN�R�}���h����
static void MdMainteModeMon(ST_COMMAND *cmdmsg);	// �����e�i���X���[�h�̃��j�^�R�}���h����


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
/* �����e�i���X���[�h���̐ݒ胂�[�h											*/
/*==========================================================================*/
void MdMainteSetMode(void)
{
	switch() {
	case :
	
	case :
	
	default:
	
		break;
	}
}

/*==========================================================================
	�����e�i���X���[�h�̏���
	�ETST_CMD����������
============================================================================*/
static void MdMainteModeLanCmd(ST_COMMAND *cmdmsg) 
{
	switch (cmdmsg->command) {
	case CMD_READY:						// 
		break;

	case CMD_RESET:
		MdChangeStatus(MD_INITIALIZE);	//���������[�h
		CmmSendAck();
		break;	

	default:
		CmmSendNack(ERR_COM_COMMAND);	//�R�}���h�ُ�
		break;
	}//switch (msg->command) {
}

/*==========================================================================
	�����e�i���X���[�h�̏���
	�E���j�^�R�}���h����������
============================================================================*/
static void MdMainteModeMon(ST_COMMAND *cmdmsg) 
{
	switch (cmdmsg->command) {
	case CMD_READY:						// 

		break;
	case CMD_RESET:
		MdChangeStatus(MD_INITIALIZE);	//���������[�h
		break;	
	default:
		PrgErrSet();
		break;
	}//switch (msg->command) {
}
