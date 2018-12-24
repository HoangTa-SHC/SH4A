/**
*	UDP�ʐM�v���O����
*
*	@file udp.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2008/04/09
*	@brief  NORTi��nonecho.c����ύX
*	@brief  FIFO_SZ�̎w��Ńo�b�t�@�T�C�Y�w��
*
*	Copyright (C) 2008, OYO Electric Corporation
*/
#include <stdio.h>
#include <string.h>
#include "kernel.h"
#include "nonet.h"
#include "udp.h"
#include "drv_fpga.h"
#include "drv_dsw.h"
#include "va300.h"

#define	_UDP_C_

#ifndef FIFO_SZ						// FIFO�̃T�C�Y�ݒ�
#define FIFO_SZ		(8192 * 2)		// 2^n�Ŏw��
//#define FIFO_SZ		2048			// 2^n�Ŏw��
#endif
#ifndef BUFSZ
#define BUFSZ		8192			// �ꎞ�o�b�t�@�T�C�Y�ݒ�
//#define BUFSZ		1024			// �ꎞ�o�b�t�@�T�C�Y�ݒ�
#endif

#define UDP_REMOTE_IP_ADDR		0xc0a8019f	///< �����IP�A�h���X�i192.168.1.159�j
#define UDP_REMOTE_PORT		50000	///< ������M�|�[�g�ԍ�

const T_CTSK c_udprcv_tsk = { TA_HLNG, NULL, udprcv_tsk, UDP_TSK_LVL, 512, NULL, "udprcv_tsk" };

// Local Data
static UH udprcv_buf[BUFSZ/2];		///< �ꎞ�o�b�t�@(UH and /2 for Word Aglinement)
static ID rcv_tskid;				///< ��M�^�X�N��ID
static ID udp_cepid;				///< �ʐM�[�_ID
static ID rxtid;					///< ��M�҂��^�X�NID
static T_IPEP udp_dstaddr;			///< UDP���M��A�h���X
static int udprcv_len;				///< ��M�f�[�^��

typedef struct t_buf{				///< FIFO�\����
	UH setp;						///< �i�[�|�C���^
	UH getp;						///< �擾�|�C���^
	UH cnt;							///< �i�[�f�[�^��
	UB buf[FIFO_SZ];				///< �o�b�t�@
}T_BUF;

static T_BUF udprcv;				///< ��M�o�b�t�@

extern UB default_ipaddr[];			// IP�A�h���X				

// �v���g�^�C�v�錾
static void init_buf(void);			// ��M�o�b�t�@�̏�����
static ER udprcv_cbk(ID, FN, VP);	// ��M�R�[���o�b�N�֐�
static void put_rxbuf(void);		// ��M�o�b�t�@�ւ̏�����
static int get_rxbuf(UB*);			// ��M�o�b�t�@����̎擾

/*==========================================================================*/
/* UDP��M�o�b�t�@�N���A                                                    */
/*==========================================================================*/

static void init_buf(void)
{
	memset( &udprcv, 0, sizeof udprcv);
}

/*==========================================================================*/
/* UDP Server Callback                                                      */
/*==========================================================================*/

static ER udprcv_cbk(ID cepid, FN fncd, VP parblk)
{
	switch(fncd)
	{
	case TFN_UDP_RCV_DAT:
		break;
	case TEV_UDP_RCV_DAT:
		udprcv_len = udp_rcv_dat(cepid, &udp_dstaddr, udprcv_buf, sizeof udprcv_buf, TMO_POL);
		put_rxbuf();					// �����ݒ��ŏ�������悤�ɕύX
		wup_tsk(rcv_tskid);
		break;
	}
	return E_OK;
}

/*==========================================================================*/
/**
 * UDP��M�^�X�N
 * @param cepid �ʐM�[�_ID
 */
/*==========================================================================*/

TASK udprcv_tsk(ID cepid)
{
	ID id;
	
	rcv_tskid = vget_tid();
	udp_cepid = cepid;

	for (;;)
	{
		/* Waiting for receive udp echo packet */
		/* It weke uped from udpecho_cbk */
		memset(udprcv_buf, 0, sizeof(udprcv_buf));
		slp_tsk();
		
		// ��M�҂�����
		if((id = rxtid) != 0) {
			rxtid = 0;
			wup_tsk((ID)id);
		}
	}
}

/*==========================================================================*/
/* ��M�o�b�t�@�֎�M����/�X�e�[�^�X�i�[�i�����֐��j                        */
/*==========================================================================*/

static void put_rxbuf(void)
{
	UB *p;
	int ln;

	p  = (UB*)udprcv_buf;	
	ln = udprcv_len;
	
	while( ln) {
		// �o�b�t�@���t�`�F�b�N
		if (udprcv.cnt == FIFO_SZ)
			return;
		
		// �o�b�t�@�֊i�[
		udprcv.buf[udprcv.setp] = *p;		// �o�b�t�@�֊i�[
		udprcv.cnt++;						// �o�b�t�@�f�[�^���{�P
		p++;								// �|�C���^�C���N�������g
		
		// �i�[�|�C���^���P�i�߂�
		udprcv.setp = (++udprcv.setp) & (FIFO_SZ - 1);
		
		ln--;
	}
}

/*==========================================================================*/
/* ��M�o�b�t�@����P�����擾�i�����֐��j                                   */
/*                                                                          */
/* �o�b�t�@��Ŏ擾�ł��Ȃ������ꍇ�́A-1 ��Ԃ��B                          */
/*==========================================================================*/

static int get_rxbuf(UB *c)
{
	int cnt;

	// ��M�o�b�t�@��`�F�b�N
	cnt = udprcv.cnt;
	if (--cnt == -1)
		return cnt;

	udprcv.cnt = (UH)cnt;				// ��M�o�b�t�@���f�[�^��-1
	*c = udprcv.buf[ udprcv.getp ];		// ��M�o�b�t�@����擾

	// �擾�|�C���^���P�i�߂�
	udprcv.getp = (++udprcv.getp) & (FIFO_SZ - 1);

	return 0;
}

/*==========================================================================*/
/**
 * UDP�ł̑��M
 * @param *s ���M�f�[�^
 * @param n ���M�f�[�^��
 */
/*==========================================================================*/

void put_udp(UB *s, UH n)
{
	ER ercd;
	
	udp_dstaddr.ipaddr = UDP_REMOTE_IP_ADDR;		// �����IP�A�h���X
	udp_dstaddr.portno = UDP_REMOTE_PORT;			// �����|�[�g�ԍ�

	ercd = udp_snd_dat(udp_cepid, &udp_dstaddr, s, n, TMO_FEVR);
}

/*==========================================================================*/
/**
 * UDP�ł̎�M
 * @param *c ��M�f�[�^�i�[�|�C���^
 * @param tmout ��M�^�C���A�E�g
 * @retval E_OK ��MOK
 * @retval E_TMOUT �^�C���A�E�g
 */
/*==========================================================================*/

ER get_udp(UB *c, TMO tmout)
{
	ER ercd;
	int sts;

	for (;;)
	{
		// ��M�o�b�t�@����P��������
		sts = get_rxbuf(c);
		if (sts != -1) {			// ��M�f�[�^�������ꍇ
			ercd = E_OK;
			return ercd;
		}

		// ��M�����ݑ҂�
		rxtid = vget_tid();
		ercd = tslp_tsk(tmout);
		rxtid = 0;
		vcan_wup();					// ������ wup_tsk ���ꂽ�ꍇ�̑΍�
		if (ercd)
			return ercd;			// �^�C���A�E�g�I��
	}
}

/*==========================================================================*/
/* UDP ��M�^�X�N Uninitialize                                              */
/*==========================================================================*/

void udp_ext(void)
{
	udp_can_cep(udp_cepid, TFN_TCP_ALL);
	udp_del_cep(udp_cepid);
	ter_tsk(rcv_tskid);
	del_tsk(rcv_tskid);
	rcv_tskid = udp_cepid = 0;
}

/*==========================================================================*/
/**
 * UDP�ʐM������
 * @param tskid �^�X�NID(0�̂Ƃ���ID����������)
 * @param cepid �ʐM�[�_ID(0�̂Ƃ���ID����������)
 * @param portno �|�[�g�ԍ�
 * @retval E_OK ����������
 */
/*==========================================================================*/

ER udp_ini(ID tskid, ID cepid, UH portno)
{
	ER ercd;
	static T_UDP_CCEP c_udp_cep;

	// UDP�ʐM�ݒ�
	c_udp_cep.cepatr = 0;						// UDP�ʐM�[�_����(0)
	c_udp_cep.myaddr.ipaddr = IPV4_ADDRANY;		// ��������IP�A�h���X
	c_udp_cep.myaddr.portno = portno;			// UDP�̃|�[�g�ԍ�
	c_udp_cep.callback = (FP)udprcv_cbk;		// �R�[���o�b�N�֐��̓o�^

	// ��M�o�b�t�@������
	init_buf();

	// Create UDP Recieve Task
	if(tskid == 0){/* ID auto (Add by Y.Y) */
		ercd = acre_tsk(&c_udprcv_tsk);
		if (ercd < 0)
			return ercd;
		tskid = ercd;
	}
	else{
		ercd = cre_tsk(tskid, &c_udprcv_tsk);
		if (ercd != E_OK)
			return ercd;
	}

	// Create UDP Commnunication End Point
	if(cepid == 0)
	{   ercd = cepid = udp_vcre_cep(&c_udp_cep);
		if(cepid <= 0)
			goto ERR;
	}
	else
	{   ercd = udp_cre_cep(cepid, &c_udp_cep);
		if (ercd != E_OK)
			goto ERR;
	}

	ercd = udp_set_opt(cepid, SET_UDP_RPKT_OPT, (VP)TRUE, sizeof(BOOL));
	if (ercd != E_OK) {
		goto ERR;
	}

	// Start Receive Server Task
	ercd = sta_tsk(tskid, cepid);
	if (ercd != E_OK)
		goto ERR;

	return E_OK;

ERR:
	udp_del_cep(cepid);
	del_tsk(tskid);
	return ercd;

}

/* end */
