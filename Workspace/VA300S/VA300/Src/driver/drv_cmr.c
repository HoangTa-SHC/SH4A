/**
*	VA-300�v���O����
*
*	@file drv_cmr.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/24
*	@brief  �J��������h���C�o
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_CMR_C_
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "drv_cmr.h"
#include "va300.h"

// ��`
#define	CMR_CAP_IP	10					///< �J�����摜��荞�݊����̊����݃��x��
#define	CMR_CAP_INT	INT_IRL10			///< �J�����摜��荞�݊����̊����ݔԍ�
#define	CMR_SND_IP	6					///< �J�����R�}���h���M�����̊����݃��x��
#define	CMR_SND_INT	INT_IRL6			///< �J�����R�}���h���M�����̊����ݔԍ�
#define	CMR_RCV_IP	5					///< �J�����R�}���h��M�����̊����݃��x��
#define	CMR_RCV_INT	INT_IRL5			///< �J�����R�}���h��M�����̊����ݔԍ�

#define	enable_cap_int()	fpga_setw(INT_CRL, INT_CRL_CAP)		///< �L���v�`���[������
#define	clear_cap_int()		fpga_clrw(INT_CRL, INT_CRL_CAP)		///< �L���v�`���[�����݃N���A���s����
#define	enable_snd_int()	fpga_setw(INT_CRL, INT_CRL_CMR_CMD)	///< �R�}���h���M������
#define	clear_snd_int()		fpga_clrw(INT_CRL, INT_CRL_CMR_CMD)	///< �R�}���h���M�����݃N���A���s����
#define	enable_rcv_int()	fpga_setw(INT_CRL, INT_CRL_CMR_RES)	///< �R�}���h��M������
#define	clear_rcv_int()		fpga_clrw(INT_CRL, INT_CRL_CMR_RES)	///< �R�}���h��M�����݃N���A���s����
#define	cap_line_get(b,x,y,n,p)	FpgaCapLineGet(b,x,y,n,p)	///< �捞�݉摜��1���C���擾
#define	cap_dat_wrt(b,x,y,n,p)	FpgaWrtDat(b,x,y,n,p)	///Miya

// �ϐ���`
static ID s_idCapTsk;					///< �L���v�`���[�̃^�X�NID
static ID s_idSendTsk;					///< �R�}���h���M�^�X�NID
static ID s_idRecvTsk;					///< ������M�^�X�NID
static ID s_idSem;						///< �Z�}�t�HID

// �v���g�^�C�v�錾
static INTHDR cmrCaptureInt(void);		///< �J�����摜��荞�݊���������
static void cmr_capture_int(void);		///< �J�����摜��荞�݊��������ݖ{��
#if ( NEWCMR == 0 )
static INTHDR cmrSendInt(void);			///< �R�}���h���M����������
static void cmr_send_int(void);			///< �R�}���h���M���������ݖ{��
static INTHDR cmrRecvInt(void);			///< �R�}���h��M����������
static void cmr_recv_int(void);			///< �R�}���h��M���������ݖ{��
static ER cmrCapture(enum CAP_MODE eMode, enum CAP_BANK eBank);	///< �J�����摜�L���v�`���[(�{��)
static ER cmrCaptureWait(TMO tmout);	///< �J�����摜��荞�ݑ҂�
#else
ER NcmrCapture( enum CAP_MODE eMode, enum CAP_BANK eBank );	///< �J�����摜�L���v�`���[(�{��)
ER NcmrCaptureWait( TMO tmout );	///< �J�����摜��荞�ݑ҂�
#endif

static void cmrPktSend(UB *pData, int iSendSize, int iRcvSize);	///< �J�����ʐMI/F ���M(�{��)
static BOOL cmrPktRecv(UB *pData, int iRcvSize);	///< �J�����ʐMI/F ��M(�{��)
static void cmrCapLineGet(enum CAP_BANK eBank, int iX, int iY, int iSize, UB *p);	///< �J�����摜���w��o�C�g���擾����(�{��)
static void cmrCapGet(enum CAP_BANK eBank, int iX, int iY, int iXsize, int iYsize, UB *p);	///< �J�����摜���擾����(�{��)
static void cmrMemWrt(enum CAP_BANK eBank, int iX, int iY, int iSize, UB *p);

static unsigned short cmrGenCRC(unsigned char *lpBuf, unsigned char sizeOfBuf);	///< CRC16�����߂�
static ER cmrTrim(enum CAP_BANK eOrg, enum TRIM_BANK eDist, int iStartX, int iStartY, int iSizeX, int iSizeY, TMO tmout);	///< �摜�̃g���~���O
static void cmrTrimGet(enum CAP_BANK eBank, long lStart, long lSize, UB *p);	///< �g���~���O�摜�̎擾
static ER cmrResize(enum TRIM_BANK eOrg, enum RSZ_BANK eDist, enum RSZ_MODE eMode, int iSizeX, int iSizeY, TMO tmout);	///< �摜�̈��k
static void cmrResizeGet(enum RSZ_BANK eBank, long lStart, long lSize, UB *p);	///< ���k�摜�̎擾

#if ( NEWCMR == 1 )
static ER NCmrI2C_Send( UB *pAddr, UB *pData, int iSendSize );		// �J�����E�R�}���h��I2C���M
static int NcmrI2C_Send( UB *pAddr, UB *pData, int iSendSize );		// �J�����E�R�}���h��I2C���M(�{��)
static ER NCmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize );		// �J�����E�R�}���h��I2C��M
static int NcmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize );		// �J�����E�R�}���h��I2C��M(�{��)
static void NCmrCapStart( void );	///< �L���v�`���J�n�v��

#endif

// �����ݒ�`
const T_DINH dinh_cmr_cap = { TA_HLNG, cmrCaptureInt, CMR_CAP_IP};	///< �摜��荞��
#if ( NEWCMR == 0 )
const T_DINH dinh_cmr_snd = { TA_HLNG, cmrSendInt, CMR_SND_IP};		///< �R�}���h���M
const T_DINH dinh_cmr_rcv = { TA_HLNG, cmrRecvInt, CMR_RCV_IP};		///< ������M
#endif
// �Z�}�t�H��`
const T_CSEM csem_cmr     = { TA_TFIFO, 1, 1, (B *)"sem_cmr" };		///< �J�����̃Z�}�t�H

/*==========================================================================*/
/**
 * �J�����֘A������
 * 
 * @param idSem �J�����p�Z�}�t�H
 * @param idRcvTsk ������M���������ݎ��ɋN������^�X�NID
 */
/*==========================================================================*/
ER CmrInit(ID idSem, ID idRcvTsk)
{
	ER ercd;
	UW psw;
	
	ercd = E_OK;
	
	// �����ݎ��ɋN������^�X�N�̐ݒ�
	if (idRcvTsk <= 0) {
		return E_PAR;					// �p�����[�^�G���[
	}
	
	// �Z�}�t�H�̐���
	if (idSem <= 0) {
		return E_PAR;					// �p�����[�^�G���[
	}
	ercd = cre_sem(idSem, &csem_cmr);	// �Z�}�t�H����
	if (ercd != E_OK) {
		return ercd;
	}
	
	// �X�^�e�B�b�N�ϐ��̐ݒ�
	s_idRecvTsk = idRcvTsk;
	s_idSem     = idSem;

	psw = vdis_psw();
	
	// �x�N�^�o�^
	ercd = def_inh(CMR_CAP_INT, &dinh_cmr_cap);		// �摜��荞�݊��������ݐݒ�
	if (ercd == E_OK) {
		enable_cap_int();							// �����ݐݒ�(�n�[�h�E�F�A��)
	}

#if ( NEWCMR == 0 )
	if (ercd == E_OK) {
		ercd = def_inh(CMR_RCV_INT, &dinh_cmr_rcv);	// ������M���������ݐݒ�
		if (ercd == E_OK) {
			enable_rcv_int();						// �����ݐݒ�(�n�[�h�E�F�A��)
		}
	}
#endif

	vset_psw(psw);
	
	return ercd;
}

#pragma interrupt(cmrCaptureInt)
/*==========================================================================*/
/**
 * �摜�捞�݊��������݃n���h��
 *
 */
/*==========================================================================*/
static INTHDR cmrCaptureInt(void)
{
	ent_int();
	cmr_capture_int();
	ret_int();
}

/*==========================================================================*/
/**
 * �摜�捞�݊��������݃n���h��(�{��)
 *
 */
/*==========================================================================*/
static void cmr_capture_int(void)
{
	clear_cap_int();					// �L���v�`���[�����݃N���A���s����
	if (s_idCapTsk) {
		iwup_tsk(s_idCapTsk);
	}
	enable_cap_int();					// �L���v�`���[�����݃N���A���s����
}

#if ( NEWCMR == 0 )
#pragma interrupt(cmrSendInt)
/*==========================================================================*/
/**
 * �R�}���h���M�����݃n���h��
 *
 */
/*==========================================================================*/
static INTHDR cmrSendInt(void)
{
	ent_int();
	cmr_send_int();
	ret_int();
}

/*==========================================================================*/
/**
 * �R�}���h���M�����݃n���h��(�{��)
 *
 */
/*==========================================================================*/
static void cmr_send_int(void)
{
	clear_snd_int();					// �R�}���h���M�����݃N���A���s����
	if (s_idSendTsk) {
		iwup_tsk(s_idSendTsk);
	}
	enable_snd_int();					// �R�}���h���M������
}

#pragma interrupt(cmrRecvInt)
/*==========================================================================*/
/**
 * ��M���������݃n���h��
 *
 */
/*==========================================================================*/
static INTHDR cmrRecvInt(void)
{
	ent_int();
	cmr_recv_int();
	ret_int();
}

/*==========================================================================*/
/**
 * ��M���������݃n���h��(�{��)
 *
 */
/*==========================================================================*/
static void cmr_recv_int(void)
{
	clear_rcv_int();					// �R�}���h��M�����݃N���A���s����
	if (s_idRecvTsk) {
		iwup_tsk(s_idRecvTsk);
	}
	enable_rcv_int();					// �R�}���h��M������
}
#endif

/*==========================================================================*/
/**
 * �J�����摜�̃L���v�`���[
 *
 *	@param eMode �摜��荞�݃V�[�P���X���[�h
 *	@param eBank ��荞�ޗ̈�
 *	@param tmout �^�C���A�E�g����
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
ER CmrCapture(enum CAP_MODE eMode, enum CAP_BANK eBank, TMO tmout)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	
	if (ercd == E_OK) {
#if ( NEWCMR == 0 )
		ercd = cmrCapture(eMode, eBank);	// �摜��荞��(GCT�ЃJ�����̏ꍇ)
		if (ercd == E_OK) {
			ercd = cmrCaptureWait(tmout);	// ��荞�ݑ҂�(GCT�ЃJ�����̏ꍇ)
		}
#else
		ercd = NcmrCapture(eMode, eBank);	// �摜��荞��(NC�ЃJ����NCM03-V�̏ꍇ)
		if (ercd == E_OK) {
			ercd = NcmrCaptureWait(tmout);	// ��荞�ݑ҂�(NC�ЃJ����NCM03-V�̏ꍇ)
		}
#endif
		sig_sem(s_idSem);
	}
	
	return ercd;
}


/*==========================================================================*/
/**
 * �J�����摜�̃L���v�`���[
 * Bionics����(�B�e�`�g���~���O���{)
 *
 *	@param eMode �摜��荞�݃V�[�P���X���[�h
 *	@param eBank ��荞�ޗ̈�
 *	@param iStartX �J�nX���W
 *	@param iStartY �J�nY���W
 *	@param iSizeX X�T�C�Y
 *	@param iSizeY Y�T�C�Y
 *	@param tmout �^�C���A�E�g����
 *	@return OS�̃G���[�R�[�h
 
 //20140829 Miya FPGA Bio
 //20160309 modified T.N For NC�ЃJ����NCM03-V
 */
/*==========================================================================*/
ER CmrCapture2(enum CAP_MODE eMode, enum CAP_BANK eBank, int iStartX, int iStartY, int iSizeX, int iSizeY, TMO tmout)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		CmrTrimStartX( iStartX );				// �g���~���O�J�nX���W
		CmrTrimStartY( iStartY );				// �g���~���O�J�nY���W
		CmrTrimSizeX( iSizeX );					// �g���~���OX�T�C�Y
		CmrTrimSizeY( iSizeY );					// �g���~���OY�T�C�Y

#if ( NEWCMR == 0 )
		ercd = cmrCapture(eMode, eBank);	// �摜��荞��(GCT�ЃJ�����̏ꍇ)
		if (ercd == E_OK) {
			ercd = cmrCaptureWait(tmout);	// ��荞�ݑ҂�(GCT�ЃJ�����̏ꍇ)
		}
#else
		ercd = NcmrCapture(eMode, eBank);	// �摜��荞��(NC�ЃJ����NCM03-V�̏ꍇ)
		if (ercd == E_OK) {
			ercd = NcmrCaptureWait(tmout);	// ��荞�ݑ҂�(NC�ЃJ����NCM03-V�̏ꍇ)
		}
#endif
		sig_sem(s_idSem);
	}
	
	return ercd;
}

#if ( NEWCMR == 0 )
/*==========================================================================*/
/**
 * �J�����摜�̃L���v�`���[(�{��)
 *
 *	@param eMode �摜��荞�݃V�[�P���X���[�h
 *	@param eBank ��荞�ޗ̈�
 *	@retval E_OK ��荞��OK
 *	@retval E_PAR �p�����[���G���[
 *	@retval E_OBJ �L���v�`���[���s���G���[
 */
/*==========================================================================*/
static ER cmrCapture(enum CAP_MODE eMode, enum CAP_BANK eBank)
{
	ER ercd;
	
	ercd = E_OK;
	
	CmrCapErrClr();						// �G���[�N���A
	
	if (eMode > CAP_MODE_4) {			// �����G���[
		ercd = E_PAR;
	}
	
	s_idCapTsk = vget_tid();			// ���^�X�N���N���^�X�N�ɓo�^
	
	if (IsCmrCapStart()) {				// �L���v�`���[���s��
		ercd = E_OBJ;
	}
	
	if (ercd == E_OK) {
		vcan_wup();						// �N���v���̃N���A
		CmrCapBank(eBank);				// �L���v�`���[����̈�̐ݒ�
		CmrCapMode(eMode);				// �L���v�`���[���[�h�̐ݒ�
		CmrCapStart();					// �L���v�`���[�J�n
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * �J�����摜�̃L���v�`���[�҂�
 *
 *	@param tmout �^�C���A�E�g����
 *	@retval E_OK ��荞��OK
 *	@retval E_TMOUT �^�C���A�E�g�G���[
 *	@retval E_SYS �L���v�`���[���s�G���[
 */
/*==========================================================================*/
static ER cmrCaptureWait(TMO tmout)
{
	ER ercd;
	
	ercd = tslp_tsk( tmout );
	if (ercd == E_OK) {
		if (IsCmrCapErr()) {
			ercd = E_SYS;
		}
	}
	vcan_wup();							// �N���v���̃N���A
	
	return ercd;
}

#else
/*==========================================================================*/
/**
 * �J�����摜�̃L���v�`���[(�{��)
 *
 *	@param eMode �摜��荞�݃V�[�P���X���[�h
 *	@param eBank ��荞�ޗ̈�
 *	@retval E_OK ��荞��OK
 *	@retval E_PAR �p�����[���G���[
 *	@retval E_OBJ �L���v�`���[���s���G���[
 */
/*==========================================================================*/
static ER NcmrCapture( enum CAP_MODE eMode, enum CAP_BANK eBank )
{
	ER ercd;
	
	ercd = E_OK;
	
	if ( eMode >= CAP_MODE_2 ) {		// �����G���[
		eMode = CAP_MODE_1;				// ���摜�B�e�i640x480)�j
//		ercd = E_PAR;
	}
	
	s_idCapTsk = vget_tid();			// ���^�X�N���N���^�X�N�ɓo�^
	
	if ( IsNCmrCap() ){					// �L���v�`���[���s��
		ercd = E_OBJ;
	}
	
	if ( ercd == E_OK ){
		vcan_wup();						// �N���v���̃N���A
		NCmrCapAreaSel( eMode );		// �L���v�`���[���[�h�̐ݒ�
		NCmrCapBnkSel( eBank );			// �L���v�`���[����̈�̐ݒ�
		NCmrCapWaitSet( 3 );			//	�摜�捞�ݎ��̐擪�p���t���[����:0�`7
		NCmrCapStart();					// �L���v�`���[�J�n
	}
	
	return ercd;
}

	UH hbuf1, hbuf2, hbuf3, hbuf4;
void NCmrCapStart( void ){
	long Lcnt;
	
	hbuf1 = 0;
	hbuf2 = 0;
	hbuf3 = 0;
	hbuf4 = 0;
	
	hbuf1 = fpga_inw( NCMR_CAP_CTR );
	hbuf2 = hbuf1 | NCMR_CAP_STA;
	
	fpga_outw( NCMR_CAP_CTR, hbuf2 );	// �L���v�`���J�n�v��
	
	hbuf3 = fpga_inw( NCMR_CAP_CTR );
	Lcnt = 100000;
	while ( IsNCmrCap() ){  			// �L���v�`���I���̊m�F�@=0:�I���A=1:���M��
		Lcnt--;
		if ( Lcnt <= 0 ) break;
	}
	hbuf4 = fpga_inw( NCMR_CAP_CTR );

}

/*==========================================================================*/
/**
 * �J�����摜�̃L���v�`���[�҂�
 *
 *	@param tmout �^�C���A�E�g����
 *	@retval E_OK ��荞��OK
 *	@retval E_TMOUT �^�C���A�E�g�G���[
 *	@retval E_SYS �L���v�`���[���s�G���[
 */
/*==========================================================================*/
ER NcmrCaptureWait( TMO tmout )
{
	ER ercd;
	
	ercd = tslp_tsk( tmout );
	if (ercd == E_OK) {
		nop();
//		if ( IsCmrCapErr() ) {
//			ercd = E_SYS;
//		}
	}
	vcan_wup();							// �N���v���̃N���A
	
	return ercd;
}


/*==========================================================================*/
/**
 * �J�����E�R�}���h��I2C���M
 *
 *	@param pAddr ���M�A�h���X������ւ̃|�C���^
 *	@param pData ���M�f�[�^������ւ̃|�C���^
 *	@param iSendSize ���M�f�[�^�T�C�Y
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
ER NCmrI2C_Send( UB *pAddr, UB *pData, int iSendSize )
{
	ER ercd;
	int icnt;
	
	ercd = twai_sem( s_idSem, 1000/MSEC );
	if ( ercd == E_OK ) {
		vcan_wup();											// �N���v���̃N���A
		icnt = NcmrI2C_Send( pAddr, pData, iSendSize );		// �R�}���h���M
		if ( iSendSize != icnt ){
			ercd = E_OBJ;				// �f�[�^���M����
		}
		sig_sem( s_idSem );
	}
	
	return ercd;
}


/*==========================================================================*/
/**
 * �J�����E�R�}���h��I2C���M(�{��)
 *
 *	@param pAddr ���M�A�h���X������ւ̃|�C���^
 *	@param pData �R�}���h������ւ̃|�C���^
 *	@param iSendSize ���M�f�[�^�T�C�Y
 *	�߂�l ���M�����f�[�^�T�C�Y
 */
/*==========================================================================*/
static int NcmrI2C_Send( UB *pAddr, UB *pData, int iSendSize )
{
	ER ercd = E_OK;
	int i;
	long Lcnt;

	// �f�[�^��I2C���M
	Lcnt = 100000;
	while ( IsNCmrSendEnd() ){  		//	I2C���M���̊m�F�@=0:�I���A=1:���M��
		Lcnt--;
		if ( Lcnt <= 0 ){
			return 0;	
		}
	}
		
	for ( i=0; i<iSendSize; i++ ){
		NCmrAddesWrite( pAddr[ i ] );	// I2C���M�����݃A�h���X��Set�B
		NCmrSendDataWrite( pData[ i ] );// I2C���M�J�����E�f�[�^�̏�����
		NCmrSendStart();				// I2C���M�J�n�v��
		Lcnt = 100000;
		while ( IsNCmrSendEnd() ){  	// I2C���M���̊m�F�@=0:�I���A=1:���M��
			Lcnt--;
			if ( Lcnt <= 0 ) break;
		}
	}
	return i;
}

/*==========================================================================*/
/**
 * �J�����E�R�}���h��I2C��M
 *
 *	@param pAddr ��M�A�h���X������ւ̃|�C���^
 *	@param pData ��M�f�[�^������ւ̃|�C���^
 *	@param iSendSize ��M�f�[�^�T�C�Y
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
static ER NCmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize )
{
	ER ercd;
	int icnt;
	
	ercd = twai_sem( s_idSem, 1000/MSEC );
	if ( ercd == E_OK ) {
		vcan_wup();											// �N���v���̃N���A
		icnt = NcmrI2C_Rcv( pAddr, pData, iRcvSize );		// �R�}���h��M
		if ( iRcvSize != icnt ){
			ercd = E_OBJ;				// �f�[�^��M����
		}
		sig_sem( s_idSem );
	}
	
	return ercd;
}


/*==========================================================================*/
/**
 * �J�����E�R�}���h��I2C��M(�{��)
 *
 *	@param pAddr ��M�A�h���X������ւ̃|�C���^
 *	@param pData ��M�f�[�^������ւ̃|�C���^
 *	@param iSendSize ��M�f�[�^�T�C�Y
 *	�߂�l ���M�����f�[�^�T�C�Y
 */
/*==========================================================================*/
static int NcmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize )
{
	ER ercd = E_OK;
	int i;
	UH hbuf;
	long Lcnt;

	// �f�[�^��I2C���M
	Lcnt = 100000;
	while ( IsNCmrReadEnd() ){  		//	I2C���M���̊m�F�@=0:�I���A=1:���M��
		Lcnt--;
		nop();
		if ( Lcnt <= 0 ){
			return 0;	
		}
	}
		
	for ( i=0; i<iRcvSize; i++ ){
		NCmrAddesWrite( pAddr[ i ] );	// I2C��M�A�h���X��Set�B
		NCmrReadStart();				// I2C��M�J�n�v��
		Lcnt = 100000;
		while ( IsNCmrReadEnd() ){  	// I2C��M���̊m�F�@=0:�I���A=1:���M��
			Lcnt--;
			nop();
			if ( Lcnt <= 0 ) break;
		}
		hbuf = NCmrRcvDataRead();		// I2C��M�J�����E�f�[�^�̓Ǎ���
		pData[ i ] = ( UB )hbuf;
	}
	return i;
}

#endif

/*==========================================================================*/
/**
 * �R�}���h���M
 *
 *	@param pData �R�}���h������ւ̃|�C���^
 *	@param iSendSize ���M�f�[�^�T�C�Y
 *	@param iRcvSize ��M�f�[�^�T�C�Y
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
ER CmrPktSend(UB *pData, int iSendSize, int iRcvSize)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		vcan_wup();								// �N���v���̃N���A
		cmrPktSend(pData, iSendSize, iRcvSize);	// �R�}���h���M
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * �R�}���h���M(�{��)
 *
 *	@param pData �R�}���h������ւ̃|�C���^
 *	@param iSendSize ���M�f�[�^�T�C�Y
 *	@param iRcvSize ��M�f�[�^�T�C�Y
 */
/*==========================================================================*/
static void cmrPktSend(UB *pData, int iSendSize, int iRcvSize)
{
	UH uhCrc;
	int iCnt;
	
	uhCrc = cmrGenCRC(pData, iSendSize);
	
	iCnt = 0;

	// ���M�f�[�^�̏�����
	while(iSendSize) {
		CmrSendDataWrite(iCnt, *pData);
		pData++;
		iCnt++;
		iSendSize--;
	}
	// CRC�t��
	CmrSendDataWrite(iCnt, (uhCrc & 0xFF));			iCnt++;
	CmrSendDataWrite(iCnt, ((uhCrc >> 8)& 0xFF));	iCnt++;
	
	CmrCmdPktSize(iCnt);				// ���M�f�[�^�T�C�Y�ݒ�
	CmrRecvPktSize((iRcvSize + 2));		// ��M�f�[�^�T�C�Y�ݒ�
	
	CmrCmdPktSend();					// �R�}���h���M
}

/*==========================================================================*/
/**
 * ������M
 *
 *	@param pData ����������i�[��ւ̃|�C���^
 *	@param iRcvSize ��M�f�[�^�T�C�Y
 *	@param tmout �^�C���A�E�g
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
ER CmrPktRecv(UB *pData, int iRcvSize, TMO tmout)
{
	ER ercd;
	
	ercd = tslp_tsk( tmout );
	if (ercd != E_OK) {
		return ercd;
	}
	
	vcan_wup();							// �N���v���̃N���A
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		if (cmrPktRecv(pData, iRcvSize) != TRUE) {
			ercd = E_SYS;
		}
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * �������M(�{��)
 *
 *	@param pData �R�}���h������ւ̃|�C���^
 *	@param iRcvSize ��M�f�[�^�T�C�Y
 */
/*==========================================================================*/
static BOOL cmrPktRecv(UB *pData, int iRcvSize)
{
	UH uhCrc, uhCrcRcv;
	int iCnt;
	UB *p, ubTmp;
	
	p = pData;
	
	iCnt = 0;

	// ��M�f�[�^�̏�����
	while(iRcvSize) {
		*p = CmrRecvDataRead(iCnt);
		p++;
		iCnt++;
		iRcvSize--;
	}
	// CRC�`�F�b�N
	uhCrc = cmrGenCRC(pData, iCnt);
	ubTmp = CmrRecvDataRead(iCnt);
	uhCrcRcv = ubTmp;
	iCnt++;
	ubTmp = CmrRecvDataRead(iCnt); 
	uhCrcRcv |= ((UH)ubTmp << 8);
	
	if (uhCrc == uhCrcRcv) {
		if (*pData & 0x80) {
			return TRUE;
		}
	} 
	return FALSE;
}

/*==========================================================================*/
/**
 * �J�����摜���w��o�C�g���擾����(1���C����)
 *
 *	@param eBank �L���v�`���[�̈�
 *	@param iX X���W(8�Ŋ���؂��l)
 *	@param iY Y���W
 *	@param iSize �w��r�b�g��(8�Ŋ���؂��T�C�Y)
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
ER CmrCapLineGet(enum CAP_BANK eBank, int iX, int iY, int iSize, UB *p)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		cmrCapLineGet(eBank, iX, iY, iSize, p);
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * �J�����摜���w��o�C�g���擾����(�{��)
 *
 *	@param eBank �L���v�`���[�̈�
 *	@param iX X���W(8�Ŋ���؂��l)
 *	@param iY Y���W
 *	@param iSize �w��r�b�g��(8�Ŋ���؂��T�C�Y)
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
static void cmrCapLineGet(enum CAP_BANK eBank, int iX, int iY, int iSize, UB *p)
{
	cap_line_get(eBank, iX, iY, iSize, p);
}

/*==========================================================================*/
/**
 * �J�����摜���擾����
 *
 *	@param eBank �L���v�`���[�̈�
 *	@param iX X���W(8�Ŋ���؂��l)
 *	@param iY Y���W
 *	@param iXsize �w��X�r�b�g��(8�Ŋ���؂��T�C�Y)
 *	@param iYsize �w��Y�r�b�g��
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
ER CmrCapGet(enum CAP_BANK eBank, int iX, int iY, int iXsize, int iYsize, UB *p)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		cmrCapGet(eBank, iX, iY, iXsize, iYsize, p);
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * �J�����摜���擾����(�{��)
 *
 *	@param eBank �L���v�`���[�̈�
 *	@param iX X���W(8�Ŋ���؂��l)
 *	@param iY Y���W
 *	@param iXsize �w��X�r�b�g��(8�Ŋ���؂��T�C�Y)
 *	@param iYsize �w��Y�r�b�g��
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
static void cmrCapGet(enum CAP_BANK eBank, int iX, int iY, int iXsize, int iYsize, UB *p)
{
	int iCount;
	
	for(iCount = 0;iCount < iYsize;iCount++) {
		cap_line_get(eBank, iX, iY, iXsize, p);
		iY++;
		p += iXsize;
	}
}

/*==========================================================================*/
/**
 * CRC16�����߂�
 *
 *	@param lpBuf �o�b�t�@�ւ̃|�C���^
 *	@param sizeOfBuf �o�b�t�@�T�C�Y
 *	@return CRC16
 */
/*==========================================================================*/
static unsigned short cmrGenCRC(unsigned char *lpBuf, unsigned char sizeOfBuf)
{
	unsigned char i, j;
	unsigned char flg_carry;
	unsigned char tmp_rotate;
	unsigned char buf;
	unsigned short crc16;
	
	crc16 = 0;
	
	for(i = 0;i < sizeOfBuf;i++) {

		buf = lpBuf[ i ];

		for(j = 0;j < 8;j++) {

			flg_carry = 0;
			if (buf & 0x80) {
				flg_carry = 1;
			}

			buf <<= 1;

			tmp_rotate = (crc16 >> 15) & 0x01;
			crc16 <<= 1;
			
			if (tmp_rotate) {
				crc16 ^= 0x1021;
			}
			
			crc16 ^= (unsigned short)flg_carry;
		}
	}
	return crc16;
}

/*==========================================================================*/
/**
 * �摜�g���~���O�̎��s
 *
 *	@param eOrg �L���v�`���[�摜
 *	@param eDist �g���~���O�摜�i�[��
 *	@param iStartX �J�nX���W
 *	@param iStartY �J�nY���W
 *	@param iSizeX X�T�C�Y
 *	@param iSizeY Y�T�C�Y
 *	@param tmout �^�C���A�E�g����
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
ER CmrTrim(enum CAP_BANK eOrg, enum TRIM_BANK eDist, int iStartX, int iStartY,
			int iSizeX, int iSizeY, TMO tmout)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		ercd = cmrTrim(eOrg, eDist, iStartX, iStartY, iSizeX, iSizeY, tmout);
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * �摜�g���~���O�̎��s(�{��)
 *
 *	@param eOrg �L���v�`���[�摜
 *	@param eDist �g���~���O�摜�i�[��
 *	@param iStartX �J�nX���W
 *	@param iStartY �J�nY���W
 *	@param iSizeX X�T�C�Y
 *	@param iSizeY Y�T�C�Y
 *	@param tmout �^�C���A�E�g����
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
static ER cmrTrim(enum CAP_BANK eOrg, enum TRIM_BANK eDist, int iStartX, int iStartY,
			int iSizeX, int iSizeY, TMO tmout)
{
	// �p�����[�^�`�F�b�N
	if (eOrg > CAP_BANK_3) {
		return E_PAR;
	}
	if (eDist > TRIM_BANK_3) {
		return E_PAR;
	}
	// ���s���`�F�b�N
	if (IsCmrTrimStart()) {
		return E_OBJ;
	}
	CmrTrimOrg( eOrg );						// �L���v�`���[�摜�w��
	CmrTrimDist( eDist );					// �g���~���O�摜�ۑ��̈�
#if 1
	CmrTrimStartX( iStartX );				// �g���~���O�J�nX���W
	CmrTrimStartY( iStartY );				// �g���~���O�J�nY���W
	CmrTrimSizeX( iSizeX );					// �g���~���OX�T�C�Y
	CmrTrimSizeY( iSizeY );					// �g���~���OY�T�C�Y
#else
	fpga_outw( TRIM_ST_X, iStartX );		///< �摜�g���~���O�J�nX���W
	fpga_outw( TRIM_ST_Y, iStartY );		///< �摜�g���~���O�J�nY���W
	fpga_outw( TRIM_SZ_X, iSizeX );			///< �摜�g���~���OX�T�C�Y
	fpga_outw( TRIM_SZ_Y, iSizeY );			///< �摜�g���~���OY�T�C�Y
#endif	
	
	CmrTrimStart();							// �g���~���O�J�n
	nop();
	
	if (tmout != TMO_FEVR) {
		while(IsCmrTrimStart()) {					// �g���~���O�����܂ő҂�
			if (tmout > 0) {
				tmout -= 10;
				dly_tsk( 10/MSEC );
			} else {
				return E_TMOUT;
			}
		}
	} else {
		while(IsCmrTrimStart()) {
			dly_tsk( 10/MSEC );
		}
	}
	return E_OK;
}

/*==========================================================================*/
/**
 * �g���~���O�摜���擾����
 *
 *	@param eBank �g���~���O�摜�̈�
 *	@param lStart �擾�J�n�ʒu
 *	@param lSize �擾�f�[�^�T�C�Y
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
ER CmrTrimGet(enum TRIM_BANK eBank, long lStart, long lSize, UB *p)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		cmrTrimGet(eBank, lStart, lSize, p);
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * �g���~���O�摜���擾����(�{��)
 *
 *	@param eBank �g���~���O�摜�̈�
 *	@param lStart �擾�J�n�ʒu
 *	@param lSize �擾�f�[�^�T�C�Y
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
static void cmrTrimGet(enum CAP_BANK eBank, long lStart, long lSize, UB *p)
{
	for(;lSize > 0;lSize--, lStart++, p++) {
		*p = trim_dat(eBank, lStart);
	}
}

/*==========================================================================*/
/**
 * �摜���k�̎��s
 *
 *	@param eOrg �g���~���O�摜
 *	@param eDist ���k�摜�i�[��
 *	@param eMode ���k���[�h
 *	@param iSizeX ���T�C�Y����摜��X�T�C�Y
 *	@param iSizeY ���T�C�Y����摜��Y�T�C�Y
 *	@param tmout �^�C���A�E�g����
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
ER CmrResize(enum TRIM_BANK eOrg, enum RSZ_BANK eDist, enum RSZ_MODE eMode, 
			int iSizeX, int iSizeY, TMO tmout)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		ercd = cmrResize(eOrg, eDist, eMode, iSizeX, iSizeY, tmout);
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * �摜���k�̎��s(�{��)
 *
 *	@param eOrg �g���~���O�摜
 *	@param eDist ���k�摜�i�[��
 *	@param eMode ���k���[�h
 *	@param iSizeX ���T�C�Y����摜��X�T�C�Y
 *	@param iSizeY ���T�C�Y����摜��Y�T�C�Y
 *	@param tmout �^�C���A�E�g����
 *	@return OS�̃G���[�R�[�h
 */
/*==========================================================================*/
static ER cmrResize(enum TRIM_BANK eOrg, enum RSZ_BANK eDist, enum RSZ_MODE eMode,
				int iSizeX, int iSizeY, TMO tmout)
{
	volatile int iX, iY;
	
	// �p�����[�^�`�F�b�N
	if (eOrg > TRIM_BANK_3) {
		return E_PAR;
	}
	if (eDist > RSZ_BANK_3) {
		return E_PAR;
	}
	// ���s���`�F�b�N
	if (IsCmrResizeStart()) {
		return E_OBJ;
	}
	
	iX = iSizeX;
	iY = iSizeY;
	
	CmrResizeOrg( eOrg );					// �g���~���O�摜�w��
	CmrResizeDist( eDist );					// ���k�摜�ۑ��̈�
	CmrResizeSizeX( iX );					// ���k�摜��X�T�C�Y
	CmrResizeSizeY( iY );					// ���k�摜��Y�T�C�Y
	CmrResizeMode( eMode );					// ���k���[�h�̐ݒ�
	
	CmrResizeStart();						// ���k�J�n
	
	if (tmout != TMO_FEVR) {
		while(IsCmrResizeStart()) {			// ���k�����܂ő҂�
			if (tmout > 0) {
				tmout -= 10;
				dly_tsk( 10/MSEC );
			} else {
				return E_TMOUT;
			}
		}
	} else {
		while(IsCmrResizeStart()) {
			dly_tsk( 10/MSEC );
		}
	}
	return E_OK;
}

/*==========================================================================*/
/**
 * ���k�摜���擾����
 *
 *	@param eBank �g���~���O�摜�̈�
 *	@param lStart �擾�J�n�ʒu
 *	@param lSize �擾�f�[�^�T�C�Y
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
ER CmrResizeGet(enum RSZ_BANK eBank, long lStart, long lSize, UB *p)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		cmrResizeGet(eBank, lStart, lSize, p);
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * ���k�摜���擾����(�{��)
 *
 *	@param eBank ���k�摜�̈�
 *	@param lStart �擾�J�n�ʒu
 *	@param lSize �擾�f�[�^�T�C�Y
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
static void cmrResizeGet(enum RSZ_BANK eBank, long lStart, long lSize, UB *p)
{
	for(;lSize > 0;lSize--, lStart++, p++) {
		*p = rsz_dat(eBank, lStart);
	}
}






/*==========================================================================*/
/**
 * �J�����摜���擾����
 *
 *	@param eBank �L���v�`���[�̈�
 *	@param iX X���W(8�Ŋ���؂��l)
 *	@param iY Y���W
 *	@param iXsize �w��X�r�b�g��(8�Ŋ���؂��T�C�Y)
 *	@param iYsize �w��Y�r�b�g��
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
ER CmrMemWrt(enum CAP_BANK eBank, int iX, int iY, int iXsize, int iYsize, UB *p)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		cmrMemWrt(eBank, iX, iY, iXsize*iYsize, p);
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * �J�����摜���擾����(�{��)
 *
 *	@param eBank �L���v�`���[�̈�
 *	@param iX X���W(8�Ŋ���؂��l)
 *	@param iY Y���W
 *	@param iXsize �w��X�r�b�g��(8�Ŋ���؂��T�C�Y)
 *	@param iYsize �w��Y�r�b�g��
 *	@param p �i�[��A�h���X
 */
/*==========================================================================*/
static void cmrMemWrt(enum CAP_BANK eBank, int iX, int iY, int iSize, UB *p)
{
	cap_dat_wrt(eBank, iX, iY, iSize, p);
	
}




