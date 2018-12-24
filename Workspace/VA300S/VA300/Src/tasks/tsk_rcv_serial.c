/**
*	VA-300�v���O����
*
*	@file tsk_rcv_serial.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/10/04
*	@brief  �V���A����M�^�X�N
*
*	Copyright (C) 2013, Bionics Co.Ltd
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <kernel.h>
#include "sh7750.h"
#include "nosio.h"
#include "nonet.h"
#include "id.h"
#include "err_ctrl.h"

#include "va300.h"
#include "command.h"
#include "version.h"


// ��M�f�[�^�̈�
//#define	RCV_BUF_SIZE	1024 + 4
static UB cSioRcvBuf[ RCV_BUF_SIZE ];	// �V���A����M�f�[�^�o�b�t�@
static unsigned short usSioRcvCount;	// �V���A����M�f�[�^��

static UINT sio_mode;
static UINT comm_data_stat;
static UINT Rcv_chr_timer;
static UINT EOT_Wait_timer;
static UINT ACK_Wait_timer;
static UINT Rcv_chr_timer;

static UB send_Ack_buff[ 4 ];  // ACK���M�R�[�h�f�[�^�E�o�b�t�@
static UB send_Nack_buff[ 4 ]; // ACK���M�R�[�h�f�[�^�E�o�b�t�@
static UB send_EOT_buff[ 4 ];  // ACK���M�R�[�h�f�[�^�E�o�b�t�@
static UINT ENQ_sending_cnt;	 // ENQ�đ��J�E���^
static UINT sio_rcv_comm_len;	 // ��M���R�}���h�̎w�背���O�X
static UINT sio_rcv_block_num;	 // ��M���R�}���h�̃u���b�N�ԍ�

static unsigned short usSioBunkatuNum;	//20160930Miya PC����VA300S�𐧌䂷��

static ER Rcv_serial_main_proc_forPC( int ch ); //20160930Miya PC����VA300S�𐧌䂷�� // Serial Receive task�@���C������
static ER Rcv_serial_main_proc( int ch ); // Serial Receive task�@���C������
static int chk_rcv_cmddata( UB *Buf, int cmd_len  );	// ��M�R�}���h�̃`�F�b�N�T����͏����B
static int util_rcv_cmddata( UB *Buf );					// ��M�R�}���h��́i�R�}���h���̉�́j
static int chk_ENQ_code( UB *cSioRcvBuf, UINT usSioRcvCount );		//�@ENQ�R�[�h��͏���
static int chk_ack_nak_code( UB *cSioRcvBuf, UINT usSioRcvCount );	//�@Ack/Nak�R�[�h��͏���

void store_Test_cmddata( UB *Buf, int cmd_len );		// Test�R�}���h���f�[�^�̃X�g�A
void store_Alarm_data( UB *Buff, int cmd_len );			// �A���[�������E�����񍐃R�}���h���f�[�^�̃Z�b�g�B
void store_Time_data( UB *Buff, int cmd_len );			// �������킹�R�}���h���f�[�^�̃Z�b�g�B

int send_Comm_Ack( INT ch );
int send_Comm_Nack( INT ch );
int send_Comm_EOT( INT ch );
void util_i_to_char4( unsigned int i, char *buff );
void util_char4_to_i( char * source, unsigned int *i_dist );
void util_i_to_char( unsigned int i, char *buff );
void util_char_to_i( char * source, unsigned int *i_dist );

/*********************************************
 * Receive task
 *
 * @param ch �`�����l���ԍ�
 *********************************************/
TASK RcvTask(INT ch)
{
	ER ercd;

	/* initialize */
//	ini_sio(ch, (B *)"115200 B8 PN S1");		//�����j�^�v���O�����p
//	ini_sio(ch, (B *)"38400 B8 PN S1");		//�����j�^�v���O�����p
	ini_sio(ch, (B *)"57600 B8 PN S1");		//�����j�^�v���O�����p
	
	ctl_sio(ch, TSIO_RXE|TSIO_TXE|TSIO_RTSON);
	memset( cSioRcvBuf, 0, RCV_BUF_SIZE);		// ��M�o�b�t�@�N���A
	usSioRcvCount = 0;							// �V���A����M�Ł[�����N���A
	sio_mode = SIO_RCV_IDLE_MODE;

	for (;;)
	{

#if(PCCTRL == 1)	//20160930Miya PC����VA300S�𐧌䂷��
		ercd = Rcv_serial_main_proc_forPC( ch ); 	// Receive task�@���C������
#else
		ercd = Rcv_serial_main_proc( ch ); 	// Receive task�@���C������
#endif
		if ( ercd != E_OK ){
			nop();			// �G���[�����̋L�q�K�v�B
		}
	}

}



#if(PCCTRL == 1)	//20160930Miya PC����VA300S�𐧌䂷��
/*******************************************************************
* Receive task�@���C������
*
********************************************************************/
static ER Rcv_serial_main_proc_forPC( int ch ) // Receive task�@���C������
{
	unsigned char	code;
	ER ercd;
	int iercd;
	int ENQ_stat, ack_nak_stat;
	int rcv_nack_cnt, ACK_Wait_TimeOut;
	int th=0;
/**
enum SIO_MODE {
	SIO_RCV_IDLE_MODE,		// RS-232 �R�}���h��M/���M�A�C�h��
	SIO_RCV_ENQ_MODE,		// RS-232 ENQ��M�����
	SIO_RCV_WAIT_MODE,		// RS-232 ����Box����̃R�}���h��M�҂����
	SIO_RCV_CMD_MODE,		// RS-232 ����Box����̃R�}���h��M����ԁB
	SIO_SEND_MODE,			// �R�}���h���M���B
	SIO_SEND_ACK_WAIT		// �R�}���h���M�ς݂ŁAACK/NAK�̎�M�҂��A����ю�t���B
};
**/

	//20140125Miya nagaiBug�C���@
	//ACK_Wait_TimeOut�̏����l�ݒ肪�Ȃ����߁A�ʐM�ُ펞��ACK_Wait_TimeOut���s��l�ɂ��I���ł��Ȃ�
	ACK_Wait_TimeOut = 0;	//2014Miya add


	comm_data_stat = COMM_DATA_IDLE;

	for(;;){
		ercd = get_sio( ch, (UB*)&code, ( TMO )1000/MSEC /* TMO_FEVR */ );	// ��M�f�[�^�҂�
		
		 sio_rcv_TSK_wdt = FLG_ON;			// SIO��M�^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O
		
		if ( ercd == E_OK ){  
		//��Ԃ��Ƃ̏���
			switch ( sio_mode ){
		
				case SIO_RCV_IDLE_MODE:		// RS-232 �R�}���h��M/���M�A�C�h��
			
					rcv_nack_cnt = 0;
					ACK_Wait_TimeOut = 0;

					if ( code == '#' ) {		// ENQ�R�[�h����M
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;
						sio_mode = SIO_RCV_CMD_MODE;				// RS-232 ����Box����̃R�}���h��M����ԂցB	
						th = 5;					
					} else {
						Rcv_chr_timer = 100;		//  �L�����N�^�ETimeOut�@�J�E���^�Z�b�g�i1�b�j
						nop();						// �ǂݎ̂āB
					}
					break;
					
				case SIO_RCV_CMD_MODE:		// RS-232 �R�}���h��M��
				
					if ( usSioRcvCount <= 5 ){	// �R�}���h�E�����O�X�ƃu���b�N�ԍ��̊m�F
						cSioRcvBuf[ usSioRcvCount++ ] = code;		// ��M�R�}���h�E�f�[�^�̊i�[

						if ( usSioRcvCount == 4 ){					// ��M�R�}���h�E�����O�X�̎擾
							sio_rcv_comm_len = ( (UINT)cSioRcvBuf[ 2 ] * 256 ) + (UINT)code;
						}
						
						if ( usSioRcvCount == 6 ){
							sio_rcv_block_num = ( (UINT)cSioRcvBuf[ 4 ] * 256 ) + (UINT)code;	 // ��M���R�}���h�̃u���b�N�ԍ��̎擾
						}

					} else if ( ( usSioRcvCount <= sio_rcv_comm_len + 2 ) && ( usSioRcvCount < RCV_BUF_SIZE ) ){ // �R�}���h�R�[�h��CR, LF��M�R�[�h�̊i�[�B
						cSioRcvBuf[ usSioRcvCount++ ] = code;
											
						// �I�[�R�[�h��M�`�F�b�N
						if ( usSioRcvCount >= sio_rcv_comm_len + 3 ){
							if ( ( cSioRcvBuf[ sio_rcv_comm_len + 1 ] == CODE_CR )
							  && ( cSioRcvBuf[ sio_rcv_comm_len + 2 ] == CODE_LF ) ){
								  
								cSioRcvBuf[ usSioRcvCount++ ] = 0;		// ��M�o�b�t�@�̍Ō��NULL������B
								
								// ��M�R�}���h��Sum�`�F�b�N�ƁA��͏����B
								comm_data_stat = chk_rcv_cmddata( cSioRcvBuf, sio_rcv_comm_len );					
								dly_tsk( 10/MSEC );
								
											  
								if( comm_data_stat == RCV_TANMATU_INFO ){
									send_sio_TANMATU_INFO();
								}else if( comm_data_stat == RCV_REGDATA_DNLD_STA ){
									send_sio_STANDARD();
								}else if( comm_data_stat == RCV_REGDATA_DNLD_GET ){
									//usSioBunkatuNum = 10 * (unsigned short)cSioRcvBuf[8] + (unsigned short)cSioRcvBuf[9];
									send_sio_STANDARD();
								}else if( comm_data_stat == RCV_REGDATA_DNLD_END ){
									send_sio_STANDARD();
								}else if( comm_data_stat == RCV_REGDATA_UPLD_STA ){
									send_sio_REGDATA_UPLD_STA();
								}else if( comm_data_stat == RCV_REGDATA_UPLD_GET ){
									//usSioBunkatuNum = 10 * (unsigned short)cSioRcvBuf[8] + (unsigned short)cSioRcvBuf[9];
									send_sio_REGDATA_UPLD_GET();
								}else if( comm_data_stat == RCV_REGDATA_UPLD_END ){
									send_sio_STANDARD();
								}else if( comm_data_stat == RCV_REGPROC ){
									g_pcproc_f = 2;
								}else if( comm_data_stat == RCV_AUTHPROC ){
									g_pcproc_f = 1;
								} else {
									iercd = send_Comm_Nack( ch );					// Nak���M����
								}
					
								sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs
								comm_data_stat = COMM_DATA_IDLE;
							}
						}

					}
					break;
			
				case SIO_SEND_MODE:			// �R�}���h���M���B
				
					nop();					// ��M�f�[�^�͓ǂݎ̂āB
											// �R�}���h���M�I�����A�������[�`���ŁhSIO_SEND_ACK_WAIT�h�Ɉڍs�����邱�ƁB
					break;					
			
				case SIO_SEND_ACK_WAIT:		// �R�}���h���M�ς݂ŁAACK/NAK�̎�M��t���B
			
					if ( ( code == CODE_ACK ) || ( code == CODE_NACK ) ){		// ACK/NAK�R�[�h����M
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;

						if ( comm_data_stat == RCV_SHUTDOWN_OK ){	// ����Box����V���b�g�_�E���v����������M
							iercd = send_Comm_Ack( ch );					// Ack���M����
							MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						}

					
					} else if ( usSioRcvCount <= 2 ){	// ACK/NAK�R�[�h �Ȍ�̂Q�o�C�g����M���B
						cSioRcvBuf[ usSioRcvCount++ ] = code;
					
					} else if ( ACK_Wait_timer <= 0 ){  // ACK/NAK�@�҂��^�C���A�E�g�i ACK_Wait_timer�́A���M������Set����B�j
						if ( ACK_Wait_TimeOut++ < 3 ){
							sio_mode = SIO_SEND_MODE;	// ���M����t�����[�h�ֈڍs�B
						
							SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// �R�}���h�̍đ��B
													
							ACK_Wait_timer = ACK_WAIT_TIME; // Ack�҂��^�C���A�E�g���Ԃ��Đݒ�
							sio_mode = SIO_SEND_ACK_WAIT;	// Ack��M�҂������[�h�ֈڍs�B
						
							usSioRcvCount = 0;
							memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						
						} else {
							sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs�B	
							comm_data_stat = COMM_DATA_IDLE;
							ACK_Wait_TimeOut = 0;
							MdCngSubMode( SUB_MD_IDLE );
						}		
					
					} else {
						nop();
					}
			
					if ( usSioRcvCount >= 3 ){			// ACK/NAK�@�Ȍ�̂R�o�C�g����M�����B
								
						//�@Ack/Nak�R�[�h��͏���
						ack_nak_stat = chk_ack_nak_code( cSioRcvBuf, usSioRcvCount );

						if ( ack_nak_stat == 1 ){		// ACK��M��
							sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs�B
							comm_data_stat = COMM_DATA_IDLE;
							MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
						} else if ( ack_nak_stat == 0 ){	// NACK��M��
							if ( rcv_nack_cnt++ < 3 ){		// NACK�R���M�ŁA���M���f�B
								sio_mode = SIO_SEND_MODE;	// ���M����t�����[�h�ֈڍs�B
						
								SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// �R�}���h�̍đ��B
													
								ACK_Wait_timer = ACK_WAIT_TIME; // Ack�҂��^�C���A�E�g���Ԃ��Đݒ�
								sio_mode = SIO_SEND_ACK_WAIT;	// Ack��M�҂������[�h�ֈڍs�B
							
								usSioRcvCount = 0;
								memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
							
							} else {
								sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs�B
								comm_data_stat = COMM_DATA_IDLE;
								rcv_nack_cnt = 0;	
							}	
						} 
					}
					break;
				
				
				default:
					break;
			}
///*		
		} else if ( ercd == E_TMOUT ){	// ��M�^�C���A�E�g�̏ꍇ
		
			 if ( sio_mode == SIO_SEND_ACK_WAIT ){	// �R�}���h���M�ς݂ŁAACK/NAK�̎�M��t���B
				if ( ACK_Wait_timer <= 0 ){  		// ACK�@�҂��^�C���A�E�g
					if ( ACK_Wait_TimeOut++ < 3 ){
						sio_mode = SIO_SEND_MODE;	// ���M����t�����[�h�ֈڍs�B
						
						SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// �R�}���h�̍đ��B
													
						ACK_Wait_timer = ACK_WAIT_TIME; // Ack�҂��^�C���A�E�g���Ԃ��Đݒ�
						sio_mode = SIO_SEND_ACK_WAIT;	// Ack��M�҂������[�h�ֈڍs�B
						
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						
					} else {
						sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs�B	
						comm_data_stat = COMM_DATA_IDLE;
						ACK_Wait_TimeOut = 0;
					}
				}		
					
			}
//*/
		} else {
			nop();
		}
	}

}


#else
/*******************************************************************
* Receive task�@���C������
*
********************************************************************/
static ER Rcv_serial_main_proc( int ch ) // Receive task�@���C������
{
	char	code;
	ER ercd;
	int iercd;
	int ENQ_stat, ack_nak_stat;
	int rcv_nack_cnt, ACK_Wait_TimeOut;
/**
enum SIO_MODE {
	SIO_RCV_IDLE_MODE,		// RS-232 �R�}���h��M/���M�A�C�h��
	SIO_RCV_ENQ_MODE,		// RS-232 ENQ��M�����
	SIO_RCV_WAIT_MODE,		// RS-232 ����Box����̃R�}���h��M�҂����
	SIO_RCV_CMD_MODE,		// RS-232 ����Box����̃R�}���h��M����ԁB
	SIO_SEND_MODE,			// �R�}���h���M���B
	SIO_SEND_ACK_WAIT		// �R�}���h���M�ς݂ŁAACK/NAK�̎�M�҂��A����ю�t���B
};
**/

	//20140125Miya nagaiBug�C���@
	//ACK_Wait_TimeOut�̏����l�ݒ肪�Ȃ����߁A�ʐM�ُ펞��ACK_Wait_TimeOut���s��l�ɂ��I���ł��Ȃ�
	ACK_Wait_TimeOut = 0;	//2014Miya add


	comm_data_stat = COMM_DATA_IDLE;

	for(;;){
		ercd = get_sio( ch, (UB*)&code, ( TMO )1000/MSEC /* TMO_FEVR */ );	// ��M�f�[�^�҂�
		
		 sio_rcv_TSK_wdt = FLG_ON;			// SIO��M�^�X�N�@WDT�J�E���^���Z�b�g�E���N�G�X�g�E�t���O
		
		if ( ercd == E_OK ){  
		//��Ԃ��Ƃ̏���
			switch ( sio_mode ){
		
				case SIO_RCV_IDLE_MODE:		// RS-232 �R�}���h��M/���M�A�C�h��
			
					rcv_nack_cnt = 0;
					ACK_Wait_TimeOut = 0;

					if ( code == CODE_ENQ ) {		// ENQ�R�[�h����M
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;
					
						Rcv_chr_timer = 100;		//  �L�����N�^�ETimeOut�@�J�E���^�Z�b�g�i1�b�j
					
						sio_mode = SIO_RCV_ENQ_MODE;	// ENQ��M����ԃ��[�h�ֈڍs�B
						
					} else {
						
						Rcv_chr_timer = 100;		//  �L�����N�^�ETimeOut�@�J�E���^�Z�b�g�i1�b�j
						nop();						// ENQ�R�[�h��M�ȊO�́A�ǂݎ̂āB
					}
					break;
					
				case SIO_RCV_ENQ_MODE:		// RS-232 ENQ��M����ԃ��[�h
				
					if ( Rcv_chr_timer <= 0 ){						// �L�����N�^��M��1�b�^�C���A�E�g
						iercd = send_Comm_Nack( ch );				// Nak���M����	
						sio_mode = SIO_RCV_IDLE_MODE;				// �A�C�h���E���[�h�ֈڍs
						comm_data_stat = COMM_DATA_IDLE;
					
					} else if ( usSioRcvCount <= 2 ){
						cSioRcvBuf[ usSioRcvCount++ ] = code;		// ��M�R�[�h�̊i�[
						
					} else {
						nop();
					}
					
					if ( usSioRcvCount >= 3 ){		// ENQ�R�[�h��M�m�F
						ENQ_stat = chk_ENQ_code( cSioRcvBuf, usSioRcvCount );
						if ( ENQ_stat == 1 ){
							
							/***
							�����ɁA�[������̃R�}���h���M�̗L���`�F�b�N�������L�q�B
							***/
							
							send_Comm_EOT( ch );					// EOT���M�����i����Box�փR�}���h���M�����j
							sio_mode = SIO_RCV_WAIT_MODE;			// RS-232 ����Box����̃R�}���h��M�҂���ԂցB
							usSioRcvCount = 0;
						} else {
							iercd = send_Comm_Nack( ch );			// Nak���M����	
							sio_mode = SIO_RCV_IDLE_MODE;			// �A�C�h���E���[�h�ֈڍs
							comm_data_stat = COMM_DATA_IDLE;	
						}
					}
					Rcv_chr_timer = 100;		//  �L�����N�^�ETimeOut�@�J�E���^�Z�b�g�i1�b�j

					break;					
			
				case SIO_RCV_WAIT_MODE:		// RS-232 �R�}���h��M�҂����
				
					if ( Rcv_chr_timer <= 0 ){						// �L�����N�^��M��1�b�^�C���A�E�g
						iercd = send_Comm_Nack( ch );				// Nak���M����	
						sio_mode = SIO_RCV_IDLE_MODE;				// �A�C�h���E���[�h�ֈڍs
						comm_data_stat = COMM_DATA_IDLE;
					
					} else if ( ( code == '@' ) && ( usSioRcvCount == 0 ) ){
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;
						sio_mode = SIO_RCV_CMD_MODE;				// RS-232 ����Box����̃R�}���h��M����ԂցB						
						
					} else {
						nop();										// '@'�R�[�h�ȊO�́A�f�[�^�𖳎��B
					}
					Rcv_chr_timer = 100;		//  �L�����N�^�ETimeOut�@�J�E���^�Z�b�g�i1�b�j

					break;										
					
				case SIO_RCV_CMD_MODE:		// RS-232 �R�}���h��M��
				
					if ( usSioRcvCount <= 4 ){	// �R�}���h�E�����O�X�ƃu���b�N�ԍ��̊m�F
						cSioRcvBuf[ usSioRcvCount++ ] = code;		// ��M�R�}���h�E�f�[�^�̊i�[

						if ( usSioRcvCount == 3 ){					// ��M�R�}���h�E�����O�X�̎擾
							sio_rcv_comm_len = ( cSioRcvBuf[ 1 ] * 256 ) + code;
						}
						
						if ( usSioRcvCount == 5 ){
							sio_rcv_block_num = ( cSioRcvBuf[ 3 ] * 256 ) + code;	 // ��M���R�}���h�̃u���b�N�ԍ��̎擾
						}
					} else if ( ( usSioRcvCount <= sio_rcv_comm_len + 2 ) && ( usSioRcvCount < RCV_BUF_SIZE ) ){ // �R�}���h�R�[�h��CR, LF��M�R�[�h�̊i�[�B
						cSioRcvBuf[ usSioRcvCount++ ] = code;
											
						// �I�[�R�[�h��M�`�F�b�N
						if ( usSioRcvCount >= sio_rcv_comm_len + 3 ){
							if ( ( cSioRcvBuf[ sio_rcv_comm_len + 1 ] == CODE_CR )
							  && ( cSioRcvBuf[ sio_rcv_comm_len + 2 ] == CODE_LF ) ){
								  
								cSioRcvBuf[ usSioRcvCount++ ] = 0;		// ��M�o�b�t�@�̍Ō��NULL������B
								
								// ��M�R�}���h��Sum�`�F�b�N�ƁA��͏����B
								comm_data_stat = chk_rcv_cmddata( cSioRcvBuf, sio_rcv_comm_len );								  
								
								if ( comm_data_stat == RCV_TEST_COM ){				// Test�R�}���h����M			
									iercd = send_Comm_Ack( ch );					// Ack���M����
									
									store_Test_cmddata( cSioRcvBuf, sio_rcv_comm_len );	// Test�R�}���h���f�[�^�̃X�g�A�B
																
									MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
												
								} else if ( comm_data_stat == RCV_NINSHOU_DATA ){	// �F�؃f�[�^�̎�M
									iercd = send_Comm_Ack( ch );					// Ack���M����
						
								} else if ( comm_data_stat == RCV_TOUROKU_REQ ){	// ����Box����o�^�v������M
									iercd = send_Comm_Ack( ch );					// Ack���M����
							
								} else if ( comm_data_stat == RCV_NINSHOU_OK ){		// ����Box����F�؋�����M										
									iercd = send_Comm_Ack( ch );					// Ack���M����
							
								} else if ( comm_data_stat == RCV_TEIDEN_STAT ){	// ����Box�����d��Ԃ̉�������M
									iercd = send_Comm_Ack( ch );					// Ack���M����
									
									if ( ( cSioRcvBuf[ 7 ] == '0' ) && ( cSioRcvBuf[ 8 ] == '1' ) ){
										Pfail_sense_flg = PFAIL_SENSE_ON;			// ��d���m�t���O�@ON
									}	else {
										Pfail_sense_flg = PFAIL_SENSE_NO;			// ��d���m�t���O�@OFF
									}
							
								} else if ( comm_data_stat == RCV_SHUTDOWN_OK ){	// ����Box����V���b�g�_�E���v����������M
									iercd = send_Comm_Ack( ch );					// Ack���M����

									MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB

								} else if ( comm_data_stat == RCV_TEIDEN_EVENT ){	// ��d��ԃC�x���g�񍐂̎�M
									iercd = send_Comm_Ack( ch );					// Ack���M����
									
									if ( ( cSioRcvBuf[ 7 ] == '0' ) && ( cSioRcvBuf[ 8 ] == '1' ) ){
										Pfail_sense_flg = PFAIL_SENSE_ON;			// ��d���m�t���O�@ON
									}	else {
										Pfail_sense_flg = PFAIL_SENSE_NO;			// ��d���m�t���O�@OFF
									}
									
								} else if ( comm_data_stat == RCV_ALARM_STAT ){		// �x�񔭐��E�����C�x���g�񍐂̎�M
									iercd = send_Comm_Ack( ch );					// Ack���M����
									nop();
									store_Alarm_data( cSioRcvBuf, sio_rcv_comm_len );	// �A���[�������E�����񍐃R�}���h���f�[�^�̃Z�b�g�B
							
								} else if ( comm_data_stat == RCV_ADJUST_TIME ){	// �������킹�R�}���h�̎�M
									iercd = send_Comm_Ack( ch );					// Ack���M����
									
									store_Time_data( cSioRcvBuf, sio_rcv_comm_len );	// �������킹�R�}���h���f�[�^�̃Z�b�g�B
																
//									MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB							
								} else {
								
									iercd = send_Comm_Nack( ch );					// Nak���M����
								}
					
								sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs
								comm_data_stat = COMM_DATA_IDLE;
							}
						}

					}
					break;
			
				case SIO_SEND_MODE:			// �R�}���h���M���B
				
					nop();					// ��M�f�[�^�͓ǂݎ̂āB
											// �R�}���h���M�I�����A�������[�`���ŁhSIO_SEND_ACK_WAIT�h�Ɉڍs�����邱�ƁB
					break;					
			
				case SIO_SEND_ACK_WAIT:		// �R�}���h���M�ς݂ŁAACK/NAK�̎�M��t���B
			
					if ( ( code == CODE_ACK ) || ( code == CODE_NACK ) ){		// ACK/NAK�R�[�h����M
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						cSioRcvBuf[ usSioRcvCount++ ] = code;

						if ( comm_data_stat == RCV_SHUTDOWN_OK ){	// ����Box����V���b�g�_�E���v����������M
							iercd = send_Comm_Ack( ch );					// Ack���M����
							MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						}

					
					} else if ( usSioRcvCount <= 2 ){	// ACK/NAK�R�[�h �Ȍ�̂Q�o�C�g����M���B
						cSioRcvBuf[ usSioRcvCount++ ] = code;
					
					} else if ( ACK_Wait_timer <= 0 ){  // ACK/NAK�@�҂��^�C���A�E�g�i ACK_Wait_timer�́A���M������Set����B�j
						if ( ACK_Wait_TimeOut++ < 3 ){
							sio_mode = SIO_SEND_MODE;	// ���M����t�����[�h�ֈڍs�B
						
							SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// �R�}���h�̍đ��B
													
							ACK_Wait_timer = ACK_WAIT_TIME; // Ack�҂��^�C���A�E�g���Ԃ��Đݒ�
							sio_mode = SIO_SEND_ACK_WAIT;	// Ack��M�҂������[�h�ֈڍs�B
						
							usSioRcvCount = 0;
							memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						
						} else {
							sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs�B	
							comm_data_stat = COMM_DATA_IDLE;
							ACK_Wait_TimeOut = 0;
							MdCngSubMode( SUB_MD_IDLE );
						}		
					
					} else {
						nop();
					}
			
					if ( usSioRcvCount >= 3 ){			// ACK/NAK�@�Ȍ�̂R�o�C�g����M�����B
								
						//�@Ack/Nak�R�[�h��͏���
						ack_nak_stat = chk_ack_nak_code( cSioRcvBuf, usSioRcvCount );

						if ( ack_nak_stat == 1 ){		// ACK��M��
							sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs�B
							comm_data_stat = COMM_DATA_IDLE;
							MdCngSubMode( SUB_MD_IDLE );	//�@�T�u���[�h�E�X�e�[�^�X���A�h�A�C�h���h�ցB
						
						} else if ( ack_nak_stat == 0 ){	// NACK��M��
							if ( rcv_nack_cnt++ < 3 ){		// NACK�R���M�ŁA���M���f�B
								sio_mode = SIO_SEND_MODE;	// ���M����t�����[�h�ֈڍs�B
						
								SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// �R�}���h�̍đ��B
													
								ACK_Wait_timer = ACK_WAIT_TIME; // Ack�҂��^�C���A�E�g���Ԃ��Đݒ�
								sio_mode = SIO_SEND_ACK_WAIT;	// Ack��M�҂������[�h�ֈڍs�B
							
								usSioRcvCount = 0;
								memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
							
							} else {
								sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs�B
								comm_data_stat = COMM_DATA_IDLE;
								rcv_nack_cnt = 0;	
							}	
						} 
					}
					break;
				
				
				default:
					break;
			}
///*		
		} else if ( ercd == E_TMOUT ){	// ��M�^�C���A�E�g�̏ꍇ
		
			 if ( sio_mode == SIO_SEND_ACK_WAIT ){	// �R�}���h���M�ς݂ŁAACK/NAK�̎�M��t���B
				if ( ACK_Wait_timer <= 0 ){  		// ACK�@�҂��^�C���A�E�g
					if ( ACK_Wait_TimeOut++ < 3 ){
						sio_mode = SIO_SEND_MODE;	// ���M����t�����[�h�ֈڍs�B
						
						SendSerialBinaryData( send_sio_buff, sio_snd_comm_len + 3 );	// �R�}���h�̍đ��B
													
						ACK_Wait_timer = ACK_WAIT_TIME; // Ack�҂��^�C���A�E�g���Ԃ��Đݒ�
						sio_mode = SIO_SEND_ACK_WAIT;	// Ack��M�҂������[�h�ֈڍs�B
						
						usSioRcvCount = 0;
						memset( cSioRcvBuf, 0, RCV_BUF_SIZE );
						
					} else {
						sio_mode = SIO_RCV_IDLE_MODE;	// �A�C�h���E���[�h�ֈڍs�B	
						comm_data_stat = COMM_DATA_IDLE;
						ACK_Wait_TimeOut = 0;
					}
				}		
					
			}
//*/
		} else {
			nop();
		}
	}

}
#endif

/*******************************************************************
* ��M�R�}���h�̃`�F�b�N�T����͏����ƁA�R�}���h��́B
*	��Buf�@��M�R�}���h�E�o�b�t�@
*�@�@cmd_len�@��M�R�}���h�̃����O�X
********************************************************************/
static int chk_rcv_cmddata( UB *Buf, int cmd_len  )
{
	int ret = 0;
	UB tmp_str[ 5 ] = { 0, 0, 0, 0, 0 };
	UB tmp_sum[ 4 ] = { 0, 0, 0, 0 };		// Ckeck Sum�����߂�B
	UB ck_sum[ 4 ] = { 0, 0, 0, 0 };
	int i;

#if(PCCTRL == 1)	//20160930Miya PC����VA300S�𐧌䂷��
	if ( Buf[ 0 ] == '#' ){			// "#"�@�̈�v�𒲂ׂ�B

		// Ckeck Sum�����߂�B
		for ( i=0; i<cmd_len; i++ ){			// �R�}���h�̃o�C�g���̑��a�����߂�
			tmp_sum[ 0 ] = tmp_sum[ 0 ] + Buf[ i ];
		}
		tmp_sum[ 0 ] = tmp_sum[ 0 ] ^ 0xff;		// �`�F�b�N�T���𔽓]
		ck_sum[ 0 ] = tmp_sum[ 0 ];
		ck_sum[ 1 ] = '\0';
		tmp_str[ 0 ] = Buf[ cmd_len ];
		tmp_str[ 1 ] = 0;						// Ckeck Sum�𒲂ׂ�B

		if ( strcmp( ( char *)tmp_str, ( char *)ck_sum ) == 0 ){	// Ckeck Sum�̈�v

			ret = util_rcv_cmddata( Buf );		// ��M�R�}���h��́i�R�}���h���̉�́j
			comm_data_stat = ret;				// �R�}���h��M�X�e�[�^�X��Set

		}	else	{
			ret = ERR_CKSUM;			// Ckeck Sum�̕s��v
		}

	}	else	{					// "#"�Ŗ������B
		ret = ERR_FIRST_CODE;
	}
#else
	if ( Buf[ 0 ] == '@' ){			// "@"�@�̈�v�𒲂ׂ�B

		// Ckeck Sum�����߂�B
		for ( i=0; i<cmd_len; i++ ){			// �R�}���h�̃o�C�g���̑��a�����߂�
			tmp_sum[ 0 ] = tmp_sum[ 0 ] + Buf[ i ];
		}
		tmp_sum[ 0 ] = tmp_sum[ 0 ] ^ 0xff;		// �`�F�b�N�T���𔽓]
		ck_sum[ 0 ] = tmp_sum[ 0 ];
		ck_sum[ 1 ] = '\0';
		tmp_str[ 0 ] = Buf[ cmd_len ];
		tmp_str[ 1 ] = 0;						// Ckeck Sum�𒲂ׂ�B

		if ( strcmp( ( char *)tmp_str, ( char *)ck_sum ) == 0 ){	// Ckeck Sum�̈�v

			ret = util_rcv_cmddata( Buf );		// ��M�R�}���h��́i�R�}���h���̉�́j
			comm_data_stat = ret;				// �R�}���h��M�X�e�[�^�X��Set

		}	else	{
			ret = ERR_CKSUM;			// Ckeck Sum�̕s��v
		}

	}	else	{					// "@"�Ŗ������B
		ret = ERR_FIRST_CODE;
	}
#endif
	return ret;

}

/*******************************************************************
*
// ��M�R�}���h��́i�R�}���h���̉�́j
*
********************************************************************/
static int util_rcv_cmddata( UB *Buf )
{
	int ret = 0;
//	int block_num;
	UB tmp_str[ 5 ] = { 0, 0, 0, 0, 0 };
	int datlen, st, sz;
	int tm;
	
//	block_num = ( Buf[ 4 ] * 256 ) + Buf[ 5 ];	 // ��M���R�}���h�̃u���b�N�ԍ��̎擾
//	if ( block_num != 0 ) return;


#if(PCCTRL == 1)
	if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '0' ) ){				// �[�����擾 //20160930Miya
		ret = RCV_TANMATU_INFO;

	}else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '1' ) ){		// �o�^�f�[�^�_�E�����[�h�J�n //20160930Miya
		ret = RCV_REGDATA_DNLD_STA;
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '2' ) ){		// �o�^�f�[�^�_�E�����[�h�� //20160930Miya
		ret = RCV_REGDATA_DNLD_GET;
		usSioBunkatuNum = 10 * (unsigned short)Buf[4] + (unsigned short)Buf[5];

		datlen = 2 * 80 * 40 + 20 * 10;
		st = 1000 * usSioBunkatuNum;
		if(st + 1000 > datlen){
			sz = datlen - st;
		}else{
			sz = 1000;
		}
		memcpy(&IfImageBuf[st], &Buf[8], sz);
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '3' ) ){		// �o�^�f�[�^�_�E�����[�h�I�� //20160930Miya
		ret = RCV_REGDATA_DNLD_END;
		memcpy(&RegImgBuf[0][0][0][0], &IfImageBuf[0], 80 * 40);
		memcpy(&RegImgBuf[0][0][1][0], &IfImageBuf[80*40], 80 * 40);
		memcpy(&g_RegBloodVesselTagData[0].MinAuthImg[0][0], &IfImageBuf[2*80*40], 20 * 10);
		memcpy(&RegImgBuf[0][1][0][0], &IfImageBuf[0], 80 * 40);
		memcpy(&RegImgBuf[0][1][1][0], &IfImageBuf[80*40], 80 * 40);
		memcpy(&g_RegBloodVesselTagData[0].MinAuthImg[1][0], &IfImageBuf[2*80*40], 20 * 10);
		AddRegImgFromRegImg(1, 0);		//20160312Miya �ɏ����xUP �o�^�E�w�K
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '4' ) ){		// �o�^�f�[�^�A�b�v���[�h�J�n //20160930Miya
		ret = RCV_REGDATA_UPLD_STA;
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '5' ) ){		// �o�^�f�[�^�A�b�v���[�h�� //20160930Miya
		ret = RCV_REGDATA_UPLD_GET;
		usSioBunkatuNum = 10 * (unsigned short)Buf[8] + (unsigned short)Buf[9];
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '6' ) ){		// �o�^�f�[�^�A�b�v���[�h�I�� //20160930Miya
		ret = RCV_REGDATA_UPLD_END;
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '7' ) ){		// �o�^���� //20160930Miya
		ret = RCV_REGPROC;
	
	} else if ( ( Buf[ 6 ] == 'A' ) && ( Buf[ 7 ] == '8' ) ){		// �F�ؑ��� //20160930Miya
		ret = RCV_AUTHPROC;
		tm = 1000 * (int)(Buf[26]-0x30) + 100 * (int)(Buf[27]-0x30) + 10 * (int)(Buf[28]-0x30) + (int)(Buf[29]-0x30);
		if(tm > 300 || tm == 0)	tm = 300;
		g_pc_authtime = tm;
		
		g_pc_authnum = Buf[31];
		if(g_pc_authnum == 3)		g_pc_authnum = 1;
		else if(g_pc_authnum == 2)	g_pc_authnum = 2;
		else						g_pc_authnum = 3;
	
	} else {
			ret = KEY_UNNOWN_REQ;		// �s���Ȗ��߂̎�M
	}

#else
	if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '0' ) ){				// �e�X�g�R�}���h�̎�M
		ret = RCV_TEST_COM;
							
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '1' ) ){		// �F�؃f�[�^�̎�M
		ret = RCV_NINSHOU_DATA;
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '2' ) ){		// ����Box����o�^�v������						
		ret = RCV_TOUROKU_REQ;
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '3' ) ){		// ����Box����F�؋�����M				
		ret = RCV_NINSHOU_OK;		
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '4' ) ){		// ����Box�����d��Ԃ̉�������M						
		ret = RCV_TEIDEN_STAT;	
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '5' ) ){		// ����Box����V���b�g�_�E���v����������M						
		ret = RCV_SHUTDOWN_OK;	
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '6' ) ){		// ��d��ԃC�x���g�񍐂̎�M						
		ret = RCV_TEIDEN_EVENT;	

	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '7' ) ){		// �x�񔭐��E�����C�x���g�񍐂̎�M						
		ret = RCV_ALARM_STAT;	
						
	} else if ( ( Buf[ 5 ] == '5' ) && ( Buf[ 6 ] == '8' ) ){		// �������킹�R�}���h�̎�M		
		ret = RCV_ADJUST_TIME;	
						
	} else {
			ret = KEY_UNNOWN_REQ;		// �s���Ȗ��߂̎�M
	}
#endif
	return ret;
}

/*******************************************************************
// Test�R�}���h���f�[�^�̃X�g�A
********************************************************************/
void store_Test_cmddata( UB *Buf, int cmd_len )
{

	// ��d��Ԓʒm		
	if ( ( Buf[ 7 ] == '0' ) && ( Buf[ 8 ] == '1' ) ){
		Pfail_sense_flg = PFAIL_SENSE_ON;			// ��d���m�t���O�@ON
	}	else {
		Pfail_sense_flg = PFAIL_SENSE_NO;			// ��d���m�t���O�@OFF
	}
	
	// Version�ԍ��ʒm
	KeyIO_board_soft_VER[ 0 ] = Buf[ 9 ];
	KeyIO_board_soft_VER[ 1 ] = Buf[ 10 ];
	KeyIO_board_soft_VER[ 2 ] = Buf[ 11 ];
	KeyIO_board_soft_VER[ 3 ] = Buf[ 12 ];
	
	// DIP SW�̐ݒ���e�ʒm
	dip_sw_data[ 0 ] = Buf[ 13 ] - '0';	// Dip SW��� Bit0,�@0�FOFF�A1�FON�A�@��������Bit0,1,2,3
	dip_sw_data[ 1 ] = Buf[ 14 ] - '0';	// Dip SW��� Bit1
	dip_sw_data[ 2 ] = Buf[ 15 ] - '0';	// Dip SW��� Bit2
	dip_sw_data[ 3 ] = Buf[ 16 ] - '0';	// Dip SW��� Bit3

}


/*******************************************************************
// �A���[�������E�����񍐃R�}���h���f�[�^�̃Z�b�g�B
********************************************************************/
void store_Alarm_data( UB *Buff, int cmd_len )
{
	UINT alm_Kubun, cnt;
	UINT alm_no;
	UB Fig1, Fig2, Fig3;
	
	alm_Kubun = Buff[ 7 ] - '0';		// �A���[���̔����敪�@-0�F�����A=1�F����
	if ( alm_Kubun != 0 ) alm_Kubun = 1;
	
	Fig1 = Buff[ 9 ] - '0';				// �A���[���ԍ�
	if ( ( Fig1 < 0 ) && ( Fig1 >= 10 ) ) Fig1 = 0;
	Fig2 = Buff[ 10 ] - '0';
	if ( ( Fig2 < 0 ) && ( Fig2 >= 10 ) ) Fig2 = 0;
	Fig3 = Buff[ 11 ] - '0';
	if ( ( Fig3 < 0 ) && ( Fig3 >= 10 ) ) Fig3 = 0;
	alm_no = ( Fig1 * 100 ) + ( Fig2 * 10 ) + Fig3;
	
	cnt = g_MainteLog.err_wcnt;			// �A���[���o�b�t�@�ւ̃A���[����񏑍���
	g_MainteLog.err_buff[ cnt ][ 0 ] = alm_no;		// �A���[���ԍ�
	g_MainteLog.err_buff[ cnt ][ 1 ] = count_1hour;	// �����E�����@��
	g_MainteLog.err_buff[ cnt ][ 2 ] = count_1min;	// �����E�����@��
	g_MainteLog.err_buff[ cnt ][ 3 ] = count_1sec;	// �����E�����@�b
	g_MainteLog.err_buff[ cnt ][ 4 ] = alm_Kubun;	// �A���[���̔����敪
			
	++g_MainteLog.err_wcnt;				//  �A���[���o�b�t�@�E�����݃|�C���^�[�@�C���N�������g
	g_MainteLog.err_wcnt = g_MainteLog.err_wcnt & 0x7F;
	
	
	DBG_send_cnt++;
			
}

/*******************************************************************
// �������킹�R�}���h���f�[�^�̃Z�b�g�B
********************************************************************/
void store_Time_data( UB *Buff, int cmd_len )
{
	UB fig1, fig2;
	int tmp_hour, tmp_min, tmp_sec;

	fig1 = Buff[ 7 ] - '0';				// ��
	fig2 = Buff[ 8 ] - '0';
	tmp_hour = fig1 * 10 + fig2;
	if ( ( tmp_hour < 0 ) || ( tmp_hour >= 24 ) ) return;

	fig1 = Buff[ 9 ] - '0';				// ��
	fig2 = Buff[ 10 ] - '0';
	tmp_min = fig1 * 10 + fig2;
	if ( ( tmp_min < 0 ) || ( tmp_min >= 60 ) ) return;

	fig1 = Buff[ 11 ] - '0';			// �b
	fig2 = Buff[ 12 ] - '0';
	tmp_sec = fig1 * 10 + fig2;
	if ( ( tmp_sec < 0 ) || ( tmp_sec >= 60 ) ) return;

	count_1hour = tmp_hour;		// ���̃X�g�A
	count_1min = tmp_min;		// ���̃X�g�A
	count_1sec = tmp_sec;		// �b�̃X�g�A
	
	g_MainteLog.now_min = count_1hour;		// "��"��LCD��ʕ\���p�������ɔ��f
	g_MainteLog.now_hour = count_1min;		// "��"���@�@��
	g_MainteLog.now_sec = count_1sec;		// "�b"���@�@��		

}


//--------------------------------------------
//�@ENQ�R�[�h��͏���
//--------------------------------------------
static int chk_ENQ_code( UB *cSioRcvBuf, UINT usSioRcvCount )
{
	int ret = -1;
	char tmp_cmd[ 4 ] = { 0 };
		
	tmp_cmd[ 0 ] = cSioRcvBuf[ usSioRcvCount - 3 ];		// ��M�f�[�^�RByte���擾
	tmp_cmd[ 1 ] = cSioRcvBuf[ usSioRcvCount - 2 ];
	tmp_cmd[ 2 ] = cSioRcvBuf[ usSioRcvCount - 1 ];

	if ( tmp_cmd[ 0 ] == CODE_ENQ ){					// ENQ�R�[�h�@�̈�v�𒲂ׂ�B

		if ( ( tmp_cmd[ 1 ] == 0x0d ) && ( tmp_cmd[ 2 ] == 0x0a ) ){			// �I�[�R�[�h�̊m�F�B
		
			ret = 1;
		}
	}
	
	return ret;
}


//--------------------------------------------
//�@Ack/Nak�R�[�h��͏���
//--------------------------------------------
static int chk_ack_nak_code( UB *cSioRcvBuf, UINT usSioRcvCount )
{
	int ret = -1;
	char tmp_cmd[ 4 ] = { 0 };
		
	tmp_cmd[ 0 ] = cSioRcvBuf[ usSioRcvCount - 3 ];		// ��M�f�[�^�RByte���擾
	tmp_cmd[ 1 ] = cSioRcvBuf[ usSioRcvCount - 2 ];
	tmp_cmd[ 2 ] = cSioRcvBuf[ usSioRcvCount - 1 ];

	if ( tmp_cmd[ 0 ] == 0x06 || tmp_cmd[ 0 ] == 0x15 ){	// Ack/Nack�R�[�h�@�̈�v�𒲂ׂ�B

		if ( ( tmp_cmd[ 1 ] == 0x0d ) && ( tmp_cmd[ 2 ] == 0x0a ) ){			// �I�[�R�[�h�̊m�F�B
			// Ack/Nack��M���m�F�B
			if ( tmp_cmd[ 0 ] == 0x06 && sio_mode == SIO_SEND_ACK_WAIT ){
				// Ack��M�Ȃ�A���[�h���u����M������ԁv�ցB
				ret = 1;
			}	else if ( tmp_cmd[ 0 ] == 0x15 && sio_mode == SIO_SEND_ACK_WAIT ){
				// Nack��M�Ȃ�A���[�h���uNack��M�ς݁B�R��܂ł͍đ��K�v�B�v�ցB
				ret = 0;
			}	else	{
				nop();
			}
		}
	}
	
	return ret;
}


/*--------------------------------------*/
/*	Ack���M����				�@�@�@	  �@*/
/*--------------------------------------*/
int send_Comm_Ack( INT ch )
{
	int i = 1;

	// Ack�R�}���h�̏����B	
	send_Ack_buff[ 0 ] = CODE_ACK;
	send_Ack_buff[ 1 ] = CODE_CR;
	send_Ack_buff[ 2 ] = CODE_LF;

	// ���M�����B
	SendSerialBinaryData( send_Ack_buff, 3 );

	return i;
}

/*--------------------------------------*/
/*	Nack���M����				�@�@�@�@*/
/*--------------------------------------*/
int send_Comm_Nack( INT ch )
{
	int i = 1;

	// Nack�R�}���h�̏����B	
	send_Nack_buff[ 0 ] = CODE_NACK;
	send_Nack_buff[ 1 ] = CODE_CR;
	send_Nack_buff[ 2 ] = CODE_LF;

	// ���M�����B
	SendSerialBinaryData( send_Nack_buff, 3 );

	return i;
}


/*---------------------------------------*/
/*	EOT���M����				�@�@	 �@�@*/
/*---------------------------------------*/
int send_Comm_EOT( INT ch )
{
	int i = 1;

	// EOT�R�}���h�̏����B	
	send_EOT_buff[ 0 ] = CODE_EOT;
	send_EOT_buff[ 1 ] = CODE_CR;
	send_EOT_buff[ 2 ] = CODE_LF;

	// ���M�����B
	SendSerialBinaryData( send_EOT_buff, 3 );

	EOT_Wait_timer = EOT_WAIT_TIME;		// EOT�҂��^�C���A�E�g���ԁi�P�b�jSet�B

	return 1;
}


/*-----------------------------------------------*/
/*	intger�������@char�S��(�E�l��)�ɕϊ�����	 */
/*-----------------------------------------------*/
void util_i_to_char4( unsigned int i, char *buff )
{
	char fig1, fig2, fig3, fig4;
	unsigned int ii;
	
	ii = i;

	fig1 = ii / 1000;
	ii = ii - ( fig1 * 1000 );

	fig2 = ii / 100;
	ii = ii - ( fig2 * 100 );

	fig3 = ii / 10;
	ii = ii - ( fig3 * 10 );
		
	fig4 = ii;

	buff[ 0 ] = fig1 + 0x30;
	buff[ 1 ] = fig2 + 0x30;
	buff[ 2 ] = fig3 + 0x30;
	buff[ 3 ] = fig4 + 0x30;
	buff[ 4 ] = 0x0;

}


/*-----------------------------------------------*/
/*	char�S��(�E�l��)���@intger�����ɕϊ�����	 */
/*-----------------------------------------------*/
void util_char4_to_i( char * source, unsigned int *i_dist )
{
	int tmp[ 4 ] = { -1, -1, -1, -1 };
	int i;

	for ( i=0; i<4; i++ ){
		if ( source[ i ] >= '0' && source[ i ] <= '9' ){
			tmp[ i ] = source[ i ] - '0';
		}
	}
	
	*i_dist = 0;

	if ( tmp[ 0 ] != -1 ){
		*i_dist = tmp[ 0 ] * 1000;
	}
	if ( tmp[ 1 ] != -1 ){
		*i_dist = *i_dist + ( tmp[ 1 ] * 100 );
	}
	if ( tmp[ 2 ] != -1 ){
		*i_dist = *i_dist + ( tmp[ 2 ] * 10 );
	}
	if ( tmp[ 3 ] != -1 ){
		*i_dist = *i_dist + tmp[ 3 ];
	}

}

/*-----------------------------------------------*/
/*	intger�������@char�T���ɕϊ�����(���l��)	 */
/*-----------------------------------------------*/
void util_i_to_char( unsigned int i, char * buff )
{
	char fig1, fig2, fig3, fig4, fig5;

	if ( i ){

		fig5 = i / 10000;
		i = i - ( fig5 * 10000 );

		fig4 = i / 1000;
		i = i - ( fig4 * 1000 );

		fig3 = i / 100;
		i = i - ( fig3 * 100 );

		fig2 = i / 10;

		fig1 = i - ( fig2 * 10 );

		if ( fig5 > 0 ){
			buff[ 0 ] = fig5 + 0x30;
			buff[ 1 ] = fig4 + 0x30;
			buff[ 2 ] = fig3 + 0x30;
			buff[ 3 ] = fig2 + 0x30;
			buff[ 4 ] = fig1 + 0x30;
			buff[ 5 ] = 0x0;

		}	else if ( fig4 > 0 ){
			buff[ 0 ] = fig4 + 0x30;
			buff[ 1 ] = fig3 + 0x30;
			buff[ 2 ] = fig2 + 0x30;
			buff[ 3 ] = fig1 + 0x30;
			buff[ 4 ] = 0x0;

		}	else if ( fig3 > 0 ){
			buff[ 0 ] = fig3 + 0x30;
			buff[ 1 ] = fig2 + 0x30;
			buff[ 2 ] = fig1 + 0x30;
			buff[ 3 ] = 0x0;

		}	else if ( fig2 > 0 ){
			buff[ 0 ] = fig2 + 0x30;
			buff[ 1 ] = fig1 + 0x30;
			buff[ 2 ] = 0x0;

		}	else if ( fig1 > 0 ){
			buff[ 0 ] = fig1 + 0x30;
			buff[ 1 ] = 0x0;

		}	else	{
			buff[ 0 ] = 0x30;
			buff[ 1 ] = 0x0;
		}

	}	else	{
		buff[ 0 ] = 0x30;
		buff[ 1 ] = 0x0;
	}	

}


/*----------------------------------------------*/
/*	char�T��(���l��)���@intger�����ɕϊ�����	*/
/*----------------------------------------------*/
void util_char_to_i( char * source, unsigned int *i_dist )
{
	int tmp[ 5 ] = { -1, -1, -1, -1, -1 };
	int i;

	for ( i=0; i<5; i++ ){
		if ( source[ i ] >= '0' && source[ i ] <= '9' ){
			tmp[ i ] = source[ i ] - '0';
		}
	}

	if ( tmp[ 4 ] != -1 ){
		*i_dist = ( tmp[ 0 ] * 10000 ) + ( tmp[ 1 ] * 1000 ) + ( tmp[ 2 ] * 100 )
			 + ( tmp[ 3 ] * 10 ) + tmp[ 4 ];

	} else if ( tmp[ 3 ] != -1 ){
		*i_dist = ( tmp[ 0 ] * 1000 ) + ( tmp[ 1 ] * 100 ) + ( tmp[ 2 ] * 10 ) + tmp[ 3 ];

	} else if ( tmp[ 2 ] != -1 ){
		*i_dist = ( tmp[ 0 ] * 100 ) + ( tmp[ 1 ] * 10 ) + tmp[ 2 ];

	} else if ( tmp[ 1 ] != -1 ){
		*i_dist = ( tmp[ 0 ] * 10 ) + tmp[ 1 ];

	} else if ( tmp[ 0 ] != -1 ){
		*i_dist = tmp[ 0 ];

	} else {
		*i_dist = 0;
	}
}


