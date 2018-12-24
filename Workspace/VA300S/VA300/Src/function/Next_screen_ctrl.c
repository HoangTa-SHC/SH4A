/**
	VA-300�v���O����
	@file Next_screen_ctrl.c
	@version 1.00
	
	@author Bionics Co.Ltd�@T.N
	@date 2012/07/31
	@brief LCD��ʐ��䃁�C���֐�
*/

#include "id.h"
#include "udp.h"
#include "net_prm.h"
#include "va300.h"
#include "err_ctrl.h"
#include "drv_eep.h"
#include "drv_led.h"

ER NextScrn_Control_mantion( void );	// ���̉�ʃR���g���[���i�}���V�����E��L���d�l�̏ꍇ�j
ER NextScrn_Control_office( void );		// ���̉�ʃR���g���[���i�P�΂P�d�l�̏ꍇ�j
ER NextScrn_Control( void );			// ���̉�ʃR���g���[���i���ʎd�l�̏ꍇ�j
static UB Chk_shutdown_ok( void );		// �F�ؑ��쒆���ǂ������`�F�b�N����
static ER Pfail_shutdown( void );		// ��d���[�h�A��d���m�ʒm��M�̏ꍇ�́A�V���b�g�_�E�����s

int dbg_Auth_8cnt;

/*==========================================================================*/
/**
 *	���̉�ʕ\���v���t���O�Z�b�g�i�P�΂P�d�l�̏ꍇ�j
 */
/*==========================================================================*/
ER NextScrn_Control_office( void )
{
	ER ercd;
	UB S_No;
	ER_UINT i;
	ER_UINT rcv_length;
	UB msg[ 128 ];	
	
	S_No = GetScreenNo();
	
	switch ( S_No ) {
		
		
	//�@�����o�^���[�h
		case LCD_SCREEN401:		// �����o�^ or �����e�i���X�{�^���I�����

			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
						
			if ( ( msg[ 0 ] == LCD_INIT_INPUT ) && ( rcv_length >= 1 ) ){	// 	�����o�^�{�^���������ꂽ
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN402 );	// �ӔC�Ҕԍ��@�I����ʂցB	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN402 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else {		// �����e�i���X�E�{�^�������͖�������B
				nop();	
			}
			break;
			
		case LCD_SCREEN402:		// �����o�^����ID���͉�ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ���[�U�[ID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN403 );	// ���O���͉�ʂցB	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN403 );		// ��ʔԍ��@<-�@���̉��
				}
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN403:		// �����@���͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// ����
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN404 );	// 	�w��ʉ�ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN404 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN403;
				befor_scrn_for_ctrl = LCD_SCREEN403;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN411 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN411 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;
			

		case LCD_SCREEN404:		// �w��ʑI�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// �w���
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				yb_touroku_data.kubun[ 0 ] = '0';		// �ӔC�ҁE��ʎҋ敪�ɁA�ē҂�Set�B
				yb_touroku_data.kubun[ 1 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN405 );	// 	�u�w���Z�b�g����..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN405 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN404;
				befor_scrn_for_ctrl = LCD_SCREEN404;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN411 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN411 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;			
		
		case LCD_SCREEN405:		// �u�w���Z�b�g����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���mON�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN406:		// �u�w�𔲂��ĉ�����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���mOFF�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN407:		// �u������x�w���Z�b�g����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN408:		// �o�^��������ʁB
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN410 );	// �u�o�^�𑱂��܂����v��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN410 );		// ��ʔԍ��@<-�@���̉��
			}		
			break;		

		case LCD_SCREEN409:		// �o�^���s�~��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN404 );	// ID�ԍ����͉�ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN404 );		// ��ʔԍ��@<-�@���̉��
			}
			break;		

		case LCD_SCREEN410:		// �o�^���s�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN402 );	// �ӔC�Ҕԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN402 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN500 );		// ��ʔԍ��@<-�@���̉��
						MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
#if ( VA300S == 0 )
						ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
						if ( ercd != E_OK ){
							nop();		// �G���[�����̋L�q	
						}
#endif
					}
				} else {
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN410 );	// �u�o�^�𑱂��܂����v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN410 );		// ��ʔԍ��@<-�@���̉��
					}		
				}					
			}
			break;		

		case LCD_SCREEN411:		// ���~�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN401 );	// �����o�^���j���[��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN401 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}
				}					
			}			
			break;		


		
	// �ʏ탂�[�h				
		case LCD_SCREEN500:		// �u�����N��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN501 );	// �ʏ탂�[�h�̑ҋ@���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN501 );		// ��ʔԍ��@<-�@���̉��
				}
			}	
			break;
		
		case LCD_SCREEN501:		// �ʏ탂�[�h�̑ҋ@���(ID�ԍ����͉��)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ���[�U�[ID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
#if ( VA300S == 0 )						
				send_ID_No_check_req_Wait_Ack_Retry();	// PC�ցAID�ԍ��⍇�킹�𑗐M�B	
#endif										
				// ��ʈڍs�́AID�ԍ��⍇�킹�i�R�}���h214�j��M�������ŁA��M��������OK/NG�ɏ]���Ď��s����B
				
			} else if ( msg[ 0 ] == LCD_MENU ){
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN520 );	// �ʏ탂�[�h�̃��j���[�ҋ@��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN520 );		// ��ʔԍ��@<-�@���̉��
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
					}	else	{
						MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
					}
				}

			} else {
				nop();	
			}
			break;
		
		case LCD_SCREEN502:		// �ʏ탂�[�h�̑ҋ@��ʂ̃u�����N�\��
			break;				// LCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN503:		// �u�w���Z�b�g����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���mON�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN504:		// �F�؊�������ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );	
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN501 );	// �ʏ탂�[�h�̑ҋ@��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN501 );		// ��ʔԍ��@<-�@���̉��
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
				}	else	{
					MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
				}
			}
			break;
		
		case LCD_SCREEN505:		// �F�؊����~��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN501 );	// �ʏ탂�[�h�̃��j���[��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN501 );		// ��ʔԍ��@<-�@���̉��
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
				}	else	{
					MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
				}
			}		
			break;


		case LCD_SCREEN506:		// �uID�ԍ����o�^����Ă��܂���v��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			if ( msg[ 0 ] == LCD_OK ){
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN501 );	// �ʏ탂�[�h�̃��j���[�ҋ@��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN501 );		// ��ʔԍ��@<-�@���̉��
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
				}	else	{
					MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
				}
				}
			}
			break;

	// �ʏ탂�[�h�E�o�^
		case LCD_SCREEN520:		// �ʏ탂�[�h�E�o�^�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN521 );	// �ʏ탂�[�h�̃��j���[���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN521 );		// ��ʔԍ��@<-�@���̉��
				}
			}			
			break;

		case LCD_SCREEN521:		// �ʏ탂�[�h�̃��j���[���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_TOUROKU ){				// �o�^�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN522 );	// �ʏ탂�[�h�o�^�E������ID���͉�ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN522 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_SAKUJYO ){		// �폜�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN543 );	// �ʏ탂�[�h�폜��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN543 );		// ��ʔԍ��@<-�@���̉��
					}													
				} else if ( msg[ 0 ] == LCD_MAINTE ){		// �����e�i���X�E�{�^������

#if ( VA300S == 0 )						
					send_donguru_chk_Wait_Ack_Retry();	// PC�ցA�h���O���̗L���m�F�𑗐M�B						
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN200 );	// �����e�i���X���[�h��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN200 );		// ��ʔԍ��@<-�@���̉��
					} else {
						nop();			// �G���[�����̋L�q
					}
#endif					
					// �����e�i���X���[�h��ʈڍs�́A�R�}���h002��M�������ŁAOK��M�����ꍇ�Ɏ��s����B

				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �I���E�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// �u���~���܂����H�v��ʂցB
					if ( ercd == E_OK ){
						befor_scrn_no = FPTN_LCD_SCREEN521;
						befor_scrn_for_ctrl = LCD_SCREEN521;
						ChgScreenNo( LCD_SCREEN538 );		// ��ʔԍ��@<-�@���̉��
					}					
				} else {
					
				}
			}
			break;
			
		case LCD_SCREEN522:		// �o�^���̌�����ID���͉�ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ���[�U�[ID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
#if ( VA300S == 0 ) 				
				send_ID_Authority_check_req_Wait_Ack_Retry();	// PC�ցAID�����⍇�킹�𑗐M�B	
#endif										
				// ��ʈڍs�́AID�����⍇�킹�i�R�}���h215�j��M�������ŁA��M��������OK/NG/CN�ɏ]���Ď��s����B

			} else if ( msg[ 0 ] == LCD_CANCEL ){			// ���~�{�^������
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// �u���~���܂����H�v��ʂցB
				if ( ercd == E_OK ){
					befor_scrn_no = FPTN_LCD_SCREEN522;
					befor_scrn_for_ctrl = LCD_SCREEN522;
					ChgScreenNo( LCD_SCREEN538 );		// ��ʔԍ��@<-�@���̉��
				}					
			} else {
				nop();	
			}
			break;
			
			
		case LCD_SCREEN523:		// �u�w���Z�b�g����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���mON�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN524:		// �F�؊�������ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );	
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN526 );	// �o�^���[�h��ID�ԍ����͉�ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN526 );		// ��ʔԍ��@<-�@���̉��
				MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
			}
			break;
		
		case LCD_SCREEN525:		// �F�؊����~��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN522 );	// �o�^���̌�����ID���͉�ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN522 );		// ��ʔԍ��@<-�@���̉��
				MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
			}		
			break;

		case LCD_SCREEN526:		// �o�^�̏ꍇ�̐ӔC�ґI�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KANTOKU ){				// �ē҃{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN527 );	// ������ID���͉�ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN527 );		// ��ʔԍ��@<-�@���̉��
						
						yb_touroku_data.kubun[ 0 ] = '0';		// �u�ӔC�ҋ敪�v�Ɋē�Set�B
						yb_touroku_data.kubun[ 1 ] = ',';
					}

				} else if ( msg[ 0 ] == LCD_KANRI ){		// �Ǘ��҃{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN527 );	// ������ID���͉�ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN527 );		// ��ʔԍ��@<-�@���̉��
						
						yb_touroku_data.kubun[ 0 ] = '1';		// �u�ӔC�ҋ敪�v�ɊǗ��҂�Set�B
						yb_touroku_data.kubun[ 1 ] = ',';
					}
									
				} else if ( msg[ 0 ] == LCD_IPPAN ){		// ��ʎ҃{�^������				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN527 );	// ������ID���͉�ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN527 );		// ��ʔԍ��@<-�@���̉��
						
						yb_touroku_data.kubun[ 0 ] = '2';		// �u�ӔC�ҋ敪�v�Ɉ�ʎ҂�Set�B
						yb_touroku_data.kubun[ 1 ] = ',';
					}

				} else if ( msg[ 0 ] == LCD_CANCEL ){		// �I���E�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// �u���~���܂����H�v��ʂցB
					if ( ercd == E_OK ){
						befor_scrn_no = FPTN_LCD_SCREEN526;
						befor_scrn_for_ctrl = LCD_SCREEN526;
						ChgScreenNo( LCD_SCREEN538 );		// ��ʔԍ��@<-�@���̉��
					}					
				} else {
					nop();
				}
			}
			break;

		case LCD_SCREEN527:		// �o�^�p��ID���͉�ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ���[�U�[ID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
#if ( VA300S == 0 )				
				send_ID_No_check_req_Wait_Ack_Retry();	// PC�ցAID�ԍ��⍇�킹�𑗐M�B	
#endif										
				// ��ʈڍs�́AID�ԍ��⍇�킹�i�R�}���h214�j��M�������ŁA��M��������OK/NG�ɏ]���Ď��s����B

			} else if ( msg[ 0 ] == LCD_CANCEL ){			// ���~�{�^������
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// �u���~���܂����H�v��ʂցB
				if ( ercd == E_OK ){
					befor_scrn_no = FPTN_LCD_SCREEN527;
					befor_scrn_for_ctrl = LCD_SCREEN527;
					ChgScreenNo( LCD_SCREEN538 );		// ��ʔԍ��@<-�@���̉��
				}					
			} else {
				nop();	
			}
			break;
			
			
		case LCD_SCREEN528:		// ���O���͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// ����
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN529 );	// 	�w��ʉ�ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN529 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN528;
				befor_scrn_for_ctrl = LCD_SCREEN528;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN538 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN529:		// �w��ʑI�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// �w���
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN530 );	// 	�u�w���Z�b�g����..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN530 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN529;
				befor_scrn_for_ctrl = LCD_SCREEN529;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN538 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN538 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN530:		// �u�w���Z�b�g����...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN531:		// �u�w�𔲂��ĉ������v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
				
		case LCD_SCREEN532:		// �u������x�A�w��...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN533:		// �o�^���������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN526);	// �ӔC�ҁE��ʎ҂̓o�^�ԍ��I����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN526 );		// ��ʔԍ��@<-�@���̉��
				}
			}	
			break;
			
		case LCD_SCREEN534:		// �o�^���s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN529);	// �w��ʑI����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN529 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
		
		case LCD_SCREEN535:		// �uID�ԍ����o�^����Ă��܂��v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){				// �m�F�{�^�����������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN527);	// �o�^ID�ԍ����͉�ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN527 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		
		case LCD_SCREEN536:		// �uID�ԍ����o�^����Ă��܂���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){				// �m�F�{�^�����������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN522 );	// �o�^ID�ԍ����͉�ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN522 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		
		case LCD_SCREEN537:		// �u���삷�錠��������܂���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){					// �m�F�{�^�����������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN522 );	// ����ID�ԍ����͉�ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN522 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
			
		case LCD_SCREEN538:		// ���~�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN500 );	// ��ʔԍ��@<-�@���̉��
					}
					
					yb_touroku_data.kubun[ 0 ] = '2';		// �u�ӔC�ҋ敪�v�Ɉ�ʎ҂�Set�B�i�f�t�H���g�j
					yb_touroku_data.kubun[ 1 ] = ',';
						
				} else if ( msg[ 0 ] == LCD_NO ){		// ������
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// ���̉�ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}

				}					
			}			
			break;

			
	// �ʏ탂�[�h�E�폜
		case LCD_SCREEN543:		// �o�^���̌�����ID���͉�ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ���[�U�[ID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
#if ( VA300S == 0 )				
				send_ID_Authority_check_req_Wait_Ack_Retry();	// PC�ցAID�����⍇�킹�𑗐M�B	
#endif										
				// ��ʈڍs�́AID�����⍇�킹�i�R�}���h215�j��M�������ŁA��M��������OK/NG/CN�ɏ]���Ď��s����B

			} else if ( msg[ 0 ] == LCD_CANCEL ){			// ���~�{�^������
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN551 );	// �u���~���܂����H�v��ʂցB
				if ( ercd == E_OK ){
					befor_scrn_no = FPTN_LCD_SCREEN543;
					befor_scrn_for_ctrl = LCD_SCREEN543;
					ChgScreenNo( LCD_SCREEN551 );		// ��ʔԍ��@<-�@���̉��
				}					
			} else {
				nop();	
			}
			break;

			
		case LCD_SCREEN544:		// �u�w���Z�b�g����..�v��ʁB
			break;				// �폜�̐��̃Z���T�[���mON�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN545:		// �F�؊�������ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );	
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN547 );	// �o�^���[�h��ID�ԍ����͉�ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN547 );		// ��ʔԍ��@<-�@���̉��
				MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
			}
			break;
		
		case LCD_SCREEN546:		// �F�؊����~��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN543 );	// �o�^���̌�����ID���͉�ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN543 );		// ��ʔԍ��@<-�@���̉��
				MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
			}		
			break;

		case LCD_SCREEN547:		// �폜�p��ID���͉�ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ���[�U�[ID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
#if ( VA300S == 0 )				
				send_ID_Authority_check_req_Wait_Ack_Retry();	// PC�ցAID�����⍇�킹�𑗐M�B	
#endif										
				// ��ʈڍs�́AID�����⍇�킹�i�R�}���h215�j��M�������ŁA��M��������OK/NG�ɏ]���Ď��s����B

			} else if ( msg[ 0 ] == LCD_CANCEL ){			// ���~�{�^������
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN551 );	// �u���~���܂����H�v��ʂցB
				if ( ercd == E_OK ){
					befor_scrn_no = FPTN_LCD_SCREEN547;
					befor_scrn_for_ctrl = LCD_SCREEN547;
					ChgScreenNo( LCD_SCREEN551 );		// ��ʔԍ��@<-�@���̉��
				}					
			} else {
				nop();	
			}
			break;
	
		
		case LCD_SCREEN548:		// �u�폜���Ă���낵���ł����H�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN547 );	// �폜�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN547 );		// ��ʔԍ��@<-�@���̉��
#if ( VA300S == 0 )
						send_touroku_delete_Wait_Ack_Retry();	// PC�ցA�w�o�^���P���̍폜�v�����M�B
#endif
					}

				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN547 );	// ���~�́u�͂��v�u�������v�I�����
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN547 );		// ��ʔԍ��@<-�@���̉��
					}
				}	
/**/				
			}
			break;
		
		case LCD_SCREEN549:		// �uID�ԍ����o�^����Ă��܂���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){					// �m�F�{�^�����������ꂽ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �o�^ID�ԍ����͉�ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		
		case LCD_SCREEN550:		// �u���삷�錠��������܂���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_OK ){					// �m�F�{�^�����������ꂽ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// ����ID�ԍ����͉�ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
			
		case LCD_SCREEN551:		// ���~�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN500 );	// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){		// ������
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// ���̉�ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}

				}					
			}			
			break;

			
		default:
			break;
	}	
}

/*==========================================================================*/
/**
 *	���̉�ʕ\���v���t���O�Z�b�g�i�}���V�����E��L���d�l�̏ꍇ�j
 */
/*==========================================================================*/
ER NextScrn_Control_mantion( void )
{
	ER ercd;
	UB S_No;
	ER_UINT i;
	ER_UINT rcv_length;
	UB msg[ 128 ];	
	int cnt0, cnt1, cnt2;
	UB ubtmp;
	unsigned short	tp_num;
	UB rtn;

	S_No = GetScreenNo();
	//memset(msg, 0, sizeof(msg));
	
	switch ( S_No ) {
		
		
	//�@�����o�^���[�h
		case LCD_SCREEN1:		// �����o�^����ID���͉�ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) && ( rcv_length >= 5 ) ){
				
				yb_touroku_data.user_id[ 0 ] = msg[ 1 ];	// ���[�U�[ID
				yb_touroku_data.user_id[ 1 ] = msg[ 2 ];
				yb_touroku_data.user_id[ 2 ] = msg[ 3 ];
				yb_touroku_data.user_id[ 3 ] = msg[ 4 ];
				yb_touroku_data.user_id[ 4 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// �����o�^/�����e�i���X�̑I����ʂցB	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN2 );		// ��ʔԍ��@<-�@���̉��
				}
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN2:		// �����o�^ or �����e�i���X�{�^���I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
						
			if ( ( msg[ 0 ] == LCD_INIT_INPUT ) && ( rcv_length >= 1 ) ){	// 	�����o�^�{�^���������ꂽ
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN3 );	// �ӔC�Ҕԍ��@�I����ʂցB	
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN3 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_MAINTE ) && ( rcv_length >= 1 ) ){// 	�����e�i���X�E�{�^���������ꂽ

				if( g_RegUserInfoData.RegSts == 0 ){	//�o�^�f�[�^�Ȃ�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN201 );	// �����e�i���X��ʁE�ӔC�Ҕԍ��@�I����ʂցB	
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN201 );		// ��ʔԍ��@<-�@���̉��
						MdCngMode( MD_MAINTE );				// ���u���[�h�������e�i���X�E���[�h��
#if ( VA300S == 0 )
						ercd = SndCmdCngMode( (UINT)MD_MAINTE );	// PC��	�����e�i���X�E���[�h�ؑւ��ʒm�𑗐M
						if ( ercd != E_OK ){
							nop();		// �G���[�����̋L�q	
						}
#endif
					}
				}else{
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// �����o�^/�����e�i���X�̑I����ʂցB	
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN2 );		// ��ʔԍ��@<-�@���̉��
					}
				}					

			} else {
				nop();	
			}
			break;

		case LCD_SCREEN3:		// �ӔC�Ҏw�ԍ��@�I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// �ӔC�Ҏw�ԍ�
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN4 );	// �����@���͉�ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN4 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN3;
				befor_scrn_for_ctrl = LCD_SCREEN3;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN4:		// �����@���͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// ����
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN5 );	// 	�w��ʉ�ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN5 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN4;
				befor_scrn_for_ctrl = LCD_SCREEN4;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;

		case LCD_SCREEN5:		// �w��ʑI�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// �w���
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN6 );	// 	�u�w���Z�b�g����..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN6 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN5;
				befor_scrn_for_ctrl = LCD_SCREEN5;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN12 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN12 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;			
		
		case LCD_SCREEN6:		// �u�w���Z�b�g����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���mON�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN7:		// �u�w�𔲂��ĉ�����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���mOFF�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN8:		// �u������x�w���Z�b�g����..�v��ʁB
			break;				// �o�^�̐��̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

		case LCD_SCREEN9:		// �o�^��������ʁB
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// �u�o�^�𑱂��܂����v��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN11 );		// ��ʔԍ��@<-�@���̉��
			}		
			break;		

		case LCD_SCREEN10:		// �o�^���s�~��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN5 );	// �w�I����ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN5 );			// ��ʔԍ��@<-�@���̉��
			}
//			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// �u�o�^�𑱂��܂����v��ʂցB
//			if ( ercd == E_OK ){
//				ChgScreenNo( LCD_SCREEN11 );		// ��ʔԍ��@<-�@���̉��
//			}
			break;		

		case LCD_SCREEN11:		// �o�^���s�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN3 );	// �ӔC�Ҕԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN3 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
						MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
#if ( VA300S == 0 )
						ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
						if ( ercd != E_OK ){
							nop();		// �G���[�����̋L�q	
						}
#else
						//�t���b�V���ɓo�^�f�[�^��ۑ�����
						dly_tsk( 1000/MSEC );
						ercd = SaveBkAuthDataFl();
						ercd = SaveRegImgFlArea(0);
#endif
					}
				} else {
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN11 );	// �u�o�^�𑱂��܂����v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN11 );		// ��ʔԍ��@<-�@���̉��
					}		
				}					
			}
			break;		

		case LCD_SCREEN12:		// ���~�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN2 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN2 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}
				}					
			}			
			break;		



	// �ʏ탂�[�h				
		case LCD_SCREEN100:		// �u�����N��ʁB
		
			g_MainteLvl = 0;	//20160711Miya �f���@
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			send_sio_BPWR(FLG_OFF);//20160905Miya B-PWR����
				
			if ( rcv_length >= 0 ){
				if(g_TechMenuData.SysSpec == 0){	//20160108Miya FinKeyS
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�̑ҋ@���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
					}
				}else{
					g_PasswordOpen.sw = FLG_ON;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// �ʏ탂�[�h�̑ҋ@���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}	
			break;
		
		case LCD_SCREEN101:		// �ʏ탂�[�h�̃��j���[���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_TOUROKU ){		// �o�^�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN120 );	// �ʏ탂�[�h�o�^��ʂցB
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )						
						g_RegFlg = 0;
#endif
						ChgScreenNo( LCD_SCREEN120 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_SAKUJYO ){	// �폜�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN140 );	// �ʏ탂�[�h�폜��ʂցB
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )						
						g_RegFlg = 0;
#endif
						ChgScreenNo( LCD_SCREEN140 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_KINKYUU_SETTEI ){	// "�ً}�ԍ��ݒ�"�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN160 );	// �ً}�ԍ��ݒ�E������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN160 );		// ��ʔԍ��@<-�@���̉��
					}					
				} else if ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU ){	//  "�ً}����"�{�^���������ꂽ"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN170 );	// �ً}�J���E������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN170 );		// ��ʔԍ��@<-�@���̉��
					}													
				} else if ( msg[ 0 ] == LCD_MAINTE ){	// �����e�i���X�E�{�^������
#if ( VA300S == 0 )						
					send_donguru_chk_Wait_Ack_Retry();	// PC�ցA�h���O���̗L���m�F�𑗐M�B						
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN200 );	// �����e�i���X���[�h��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN200 );		// ��ʔԍ��@<-�@���̉��
					} else {
						nop();			// �G���[�����̋L�q
					}
#endif						
					// �����e�i���X���[�h��ʈڍs�́A�R�}���h002��M�������ŁAOK��M�����ꍇ�Ɏ��s����B

				} else if ( msg[ 0 ] == LCD_PASSWORD_OPEN ){//20140925Miya password_open	//  "�p�X���[�h�J���{�^���������ꂽ"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN108 );	// �ً}�J���E������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN108 );		// ��ʔԍ��@<-�@���̉��
					}													
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){//20140925Miya add err	//  �G���[�������o����
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN109 );	// �G���[�\����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN109 );		// ��ʔԍ��@<-�@���̉��
					}													
				}

#if(PCCTRL == 1)	//20160930Miya PC����VA300S�𐧌䂷��
				if ( msg[ 0 ] == LCD_BACK ){		// �o�^�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�o�^��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
					}
				}
#endif						

			}
			break;
		
		case LCD_SCREEN102:		// �ʏ탂�[�h�̑ҋ@��ʂ̃u�����N�\��
			break;				// LCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		
		case LCD_SCREEN103:		// �F�؊�������ʁB	
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
					
//#if ( VA300S == 1 || VA300S == 2 )						
#if ( (VA300S == 1 || VA300S == 2) && PCCTRL == 0 )		//20160930Miya PC����VA300S�𐧌䂷��					
			//�t���b�V���ɓo�^�f�[�^��ۑ�����
			if( g_AuthOkCnt >= 50 ){
			//if( g_AuthOkCnt >= 5 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN110 );	// �ʏ탂�[�h�̑ҋ@��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN110 );		// ��ʔԍ��@<-�@���̉��
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
					}	else	{
						MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
					}
				}
				break;
				//	g_AuthOkCnt = 0;
				//	ercd = SaveBkAuthDataFl();
				//	ercd = SaveRegImgFlArea( 0 );
				//	ercd = SaveRegImgFlArea( 10 );
			}
#endif

#if(AUTHTEST >= 1)	//20160613Miya
			if(g_sv_okcnt == 8){
				ReadTestRegImgArea(0, 0, 0, 0);
				SaveTestRegImgFlArea(0);
				g_sv_okcnt = 0;
			}
#endif

			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�̑ҋ@��ʂցB

			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
				}	else	{
					MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
				}
			}
			
#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
			send_sio_AUTHPROC(0);
#endif

			break;
		
		case LCD_SCREEN104:		// �F�؊����~��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

#if(AUTHTEST >= 1)	//20160715Miya
			if(g_sv_okcnt == 8){
				ReadTestRegImgArea(0, 0, 0, 0);
				SaveTestRegImgFlArea(0);
				g_sv_okcnt = 0;
			}
#endif
			
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�̃��j���[��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
				}	else	{
					MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
				}
			}

#if(PCCTRL)	//20160930Miya PC����VA300S�𐧌䂷��
			send_sio_AUTHPROC(1);
#endif

			break;
		
		//20140925Miya password_open
		case LCD_SCREEN108:		// �J���p�p�X���[�h���͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KEYIN_PASSWORD ){		// ����{�^��
					if( rcv_length >= 5 ){
						g_InPasswordOpen.password[ 0 ] = msg[ 1 ];		// �p�X���[�h�W���ԍ�
						g_InPasswordOpen.password[ 1 ] = msg[ 2 ];
						g_InPasswordOpen.password[ 2 ] = msg[ 3 ];
						g_InPasswordOpen.password[ 3 ] = msg[ 4 ];
						g_InPasswordOpen.password[ 4 ] = msg[ 5 ];
						g_InPasswordOpen.password[ 5 ] = msg[ 6 ];
						g_InPasswordOpen.password[ 6 ] = msg[ 7 ];
						g_InPasswordOpen.password[ 7 ] = msg[ 8 ];
						g_InPasswordOpen.password[ 8 ] = 0;
						
						dly_tsk((500/MSEC));	//500msec�̃E�G�C�g
						ercd = ChekPasswordOpenKey();
						if( ercd == E_OK ){
							g_AuthType = 1;	//20160120Miya
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN103 );	// �J��OK�́���ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN103 );		// ��ʔԍ��@<-�@���̉��
								dly_tsk( 500/MSEC );
								send_sio_Ninshou(1, 1, 0);				//�����R�}���h���s
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN104 );	// �J��OK�́~��ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN104 );		// ��ʔԍ��@<-�@���̉��
							}
						}
					}
				}else if(msg[ 0 ] == LCD_MENU){
					if ( rcv_length >= 0 ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN180 );	// 
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN180 );		// ��ʔԍ��@<-�@���̉��
						}
					}
				}else if(msg[ 0 ] == LCD_CANCEL){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �ʏ탂�[�h�̑ҋ@��ʂցB

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
						}	else	{
							MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
						}
					}
				}else if(msg[ 0 ] == LCD_MAINTE){
#if ( VA300S == 0 )						
					send_donguru_chk_Wait_Ack_Retry();	// PC�ցA�h���O���̗L���m�F�𑗐M�B						
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X���[�h��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					} else {
						nop();			// �G���[�����̋L�q
					}

					MdCngMode( MD_MAINTE );				// ���u���[�h�������e�i���X���[�h��
					ercd = set_flg( ID_FLG_MAIN, FPTN_SEND_REQ_MAINTE_CMD );	// ���C��Task�ցA�����e�i���X���[�h�ؑւ��ʒm���M���˗��B
					if ( ercd != E_OK ){
						nop();		// �G���[�����̋L�q	
					}
#endif						
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){//20140925Miya add err	//  �G���[�����o����
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN109 );	// �G���[�\����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN109 );		// ��ʔԍ��@<-�@���̉��
					}													
				}

			}
			break;
		case LCD_SCREEN109:		// �u�G���[�\���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// �u�߂�v�������ꂽ
				
					//++g_MainteLog.err_rcnt;
					//g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;

					//�G���[��10�ȏ゠��ꍇ
					cnt1 = (int)g_MainteLog.err_wcnt;
					cnt2 = (int)g_MainteLog.err_rcnt;
					if( cnt1 != cnt2 ){
						if( cnt1 > 0x7F || cnt2 > 0x7F ){
							g_MainteLog.err_wcnt = 0;
							g_MainteLog.err_rcnt = 0;
						}else{
							if( cnt1 > cnt2 ){
								cnt0 = cnt1 - cnt2;
							}else{
								cnt0 = 128 - cnt2;
								cnt0 = cnt0 + cnt1;
							}
							if( cnt0 > 10 ){
								cnt2 = cnt1 - 10;
								if( cnt2 < 0 ){
									cnt2 = 128 + cnt2;
								}
							}
							g_MainteLog.err_rcnt = (unsigned short)cnt2;
						}
					}

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN101 );	// �����e�i���X�E���j���[���(���)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN101 );		// ��ʔԍ��@<-�@���̉��
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
						}	else	{
							MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
						}
					}
				}
			}
			break;
		case LCD_SCREEN110:		// �u���΂炭���܂����������v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR����

			if ( rcv_length >= 1 ){
				g_Diagnosis_start = 1;
			}
			break;
	// �ʏ탂�[�h�E�o�^
		case LCD_SCREEN120:		// �ʏ탂�[�h�E�o�^�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR����

#if(PCCTRL == 1) //20160930Miya PC����VA300S�𐧌䂷��
			if ( rcv_length >= 0 ){
				IrLedOnOffSet(1, irDuty2, irDuty3, irDuty4, irDuty5);	//20160312Miya �ɏ����xUP
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN126 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN126 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
#endif

#if(AUTHTEST >= 1)	//20160613Miya
			if ( rcv_length >= 0 ){
				IrLedOnOffSet(1, irDuty2, irDuty3, irDuty4, irDuty5);	//20160312Miya �ɏ����xUP
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
				}
			}
#else
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN121 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN121 );		// ��ʔԍ��@<-�@���̉��
				}
			}
#endif
			break;
			
		case LCD_SCREEN121:		// �o�^���E�ӔC�҂̔F�؉��
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN122:		// �ӔC�҂̔F�ؐ��������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// �ӔC�ҁE��ʎ҂̓o�^�ԍ��I����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN123:		// �ӔC�҂̔F�؎��s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�̃u�����N���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN124:		// �ӔC�ҁE��ʎ҂̓o�^�ԍ��I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// �ӔC�Ҏw�ԍ�
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN125 );	// �����@���͉�ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN125 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN124;
				befor_scrn_for_ctrl = LCD_SCREEN124;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN133 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN133 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}	
			break;
			
		case LCD_SCREEN125:		// ���O���͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_NAME ) && ( rcv_length >= 25 ) ){	

				for(i = 0 ; i < 24 ; i++ ){					// ����
					yb_touroku_data.name[ i ] = msg[ 1 + i ];
				}
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN126 );	// 	�w��ʉ�ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN126 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN125;
				befor_scrn_for_ctrl = LCD_SCREEN125;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN126:		// �w��ʑI�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( ( msg[ 0 ] == LCD_YUBI_SHUBETU ) && ( rcv_length >= 3 ) ){	
			
				yb_touroku_data.yubi_no[ 0 ] = msg[ 1 ];	// �w���
				yb_touroku_data.yubi_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_no[ 2 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN127 );	// 	�u�w���Z�b�g����..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN127 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				befor_scrn_no = FPTN_LCD_SCREEN126;
				befor_scrn_for_ctrl = LCD_SCREEN126;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN132 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN132 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}
			break;
			
		case LCD_SCREEN127:		// �u�w���Z�b�g����...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN128:		// �u�w�𔲂��ĉ������v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN129:		// �u������x�A�w��...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN130:		// �o�^���������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
#if(PCCTRL == 1) //20160930Miya PC����VA300S�𐧌䂷��
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ���[�U�[ID�ԍ��I����ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
/*
					if(g_RegFlg > 0){
						dly_tsk( 1000/MSEC );
						ercd = SaveBkAuthDataFl();
						if(g_RegFlg == 1 || g_RegFlg == 3){
							//�t���b�V���ɓo�^�f�[�^��ۑ�����
							ercd = SaveRegImgFlArea(0);
						}
						if(g_RegFlg == 2 || g_RegFlg == 3){
							//�t���b�V���ɓo�^�f�[�^��ۑ�����
							ercd = SaveRegImgFlArea(10);
						}
					}
*/
				}
				
				send_sio_REGPROC(0);
			}
#else
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// �ӔC�ҁE��ʎ҂̓o�^�ԍ��I����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
				}
			}	
#endif
			break;
			
		case LCD_SCREEN131:		// �o�^���s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
#if(PCCTRL == 1) //20160930Miya PC����VA300S�𐧌䂷��
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ���[�U�[ID�ԍ��I����ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
				send_sio_REGPROC(1);
			}
#else
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN126);	// �w��ʑI����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN126 );		// ��ʔԍ��@<-�@���̉��
				}
//				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124);	// �ӔC�ҁE��ʎ҂̓o�^�ԍ��I����ʂ�
//				if ( ercd == E_OK ){
//					ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
//				}
			}
				
#endif
			break;

		case LCD_SCREEN132:		// ���~�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
#if(PCCTRL == 1) //20160930Miya PC����VA300S�𐧌䂷��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
					send_sio_REGPROC(1);
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
					}
#endif
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					//���Ή��@20130510 Miya
//					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN124 );	// ���[�U�[ID�ԍ��I����ʂցB
//					if ( ercd == E_OK ){
//						ChgScreenNo( LCD_SCREEN124 );		// ��ʔԍ��@<-�@���̉��
//					}

					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}

				}					
			}			
			break;
			
		case LCD_SCREEN133:		// ���~�́u�͂��v�u�������v�I����ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��

#if ( VA300S == 1 || VA300S == 2 )
						if(g_RegFlg > 0){
							dly_tsk( 1000/MSEC );
							ercd = SaveBkAuthDataFl();
							if(g_RegFlg == 1 || g_RegFlg == 3){
								//�t���b�V���ɓo�^�f�[�^��ۑ�����
								ercd = SaveRegImgFlArea(0);
							}
							if(g_RegFlg == 2 || g_RegFlg == 3){
								//�t���b�V���ɓo�^�f�[�^��ۑ�����
								ercd = SaveRegImgFlArea(10);
							}
						}
#endif

					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );		// ��ʔԍ��@<-�@���̉��
					}

				}					
			}			
			break;
			
		
	// �ʏ탂�[�h�E�폜
		case LCD_SCREEN140:		// �ʏ탂�[�h�E�폜�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR����

			//20130620_Miya �f���p�ɍ폜��ʑJ�ڕύX
/*
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN145 );	// �ʏ탂�[�h�폜���E�ӔC�҂̔F�؉��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN145 );		// ��ʔԍ��@<-�@���̉��
				}
			}
*/

/*�@20130620_Miya �f���p�ɍ폜��ʑJ�ڕύX�@�� */
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN141 );	// �ʏ탂�[�h�폜���E�ӔC�҂̔F�؉��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN141 );		// ��ʔԍ��@<-�@���̉��
				}
			}
/**/

			break;
			
		case LCD_SCREEN141:		// �폜���E�ӔC�҂̔F�؉��
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN142:		// �ӔC�҂̔F�ؐ��������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144);	// �ӔC�ҁE��ʎ҂̍폜�ԍ��I����ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN144 );		// ��ʔԍ��@<-�@���̉��
				}
			}	
			break;
			
		case LCD_SCREEN143:		// �ӔC�҂̔F�؎��s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100);	// �ʏ탂�[�h�u�����N��ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN144:		// �ӔC�ҁE��ʎ҂̍폜�ԍ��I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// �폜���̎w�ԍ�
				yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				yb_touroku_data.yubi_seq_no[ 3 ] = ',';
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN145 );	// �u�폜���Ă���낵���ł����v��ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN145 );		// ��ʔԍ��@<-�@���̉��
				}
				
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN146 );	// 	�u���~���Ă�..�v��ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN146 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}	
			break;
			
		case LCD_SCREEN145:		// �u�폜���Ă���낵���ł����v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );


			if ( rcv_length >= 1 ){
/*
				//20130620_Miya �f���p�ɍ폜��ʑJ�ڕύX
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
//					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �폜�ԍ��I����ʂցB
//					if ( ercd == E_OK ){
//						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
						//send_touroku_delete_Wait_Ack_Retry();	// PC�ցA�w�o�^���P���̍폜�v�����M�B
						send_touroku_init_Wait_Ack_Retry();		// PC�ցA�o�^�f�[�^�̏������v�����M�B
					if ( ercd == E_OK ){
						req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
					}
						if ( ercd == E_OK ){
							req_restart = 1;		// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
						}
//					}

				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ���~�́u�͂��v�u�������v�I�����
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
				}					
*/				
				
/*�@20130620_Miya �f���p�ɍ폜��ʑJ�ڕύX�@�� */
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// �폜�ԍ��I����ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// ��ʔԍ��@<-�@���̉��
#if ( VA300S == 0 )
						send_touroku_delete_Wait_Ack_Retry();	// PC�ցA�w�o�^���P���̍폜�v�����M�B
#else
						rtn = DelImgAuthPara();
						if(rtn == 0){	//20161031Miya Ver2204
							send_sio_Touroku_Del();				// VA300s �������ցA�w�o�^���P���̍폜��񍐁B
							MdCngSubMode( SUB_MD_DELETE );		// �T�u���[�h�폜��
						}else{
							set_reg_param_for_Lcd();	//20161031Miya Ver2204
						}
#endif
					}

				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// ���~�́u�͂��v�u�������v�I�����
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// ��ʔԍ��@<-�@���̉��
					}
				}	
/**/				
			}
			break;
			
		case LCD_SCREEN146:		// ���~�́u�͂��v�u�������v�I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){			// �͂�
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// ���[�U�[ID�ԍ��I����ʂցB
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )
						if(g_RegFlg > 0){
							dly_tsk( 1000/MSEC );
							ercd = SaveBkAuthDataFl();
							if(g_RegFlg == 1 || g_RegFlg == 3){
								//�t���b�V���ɓo�^�f�[�^��ۑ�����
								ercd = SaveRegImgFlArea(0);
							}
							if(g_RegFlg == 2 || g_RegFlg == 3){
								//�t���b�V���ɓo�^�f�[�^��ۑ�����
								ercd = SaveRegImgFlArea(10);
							}
						}
#endif
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_NO ){	// ������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN144 );	// �ʏ��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN144 );		// ��ʔԍ��@<-�@���̉��
					}
				}					
			}	
			break;

	// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�
		case LCD_SCREEN160:		// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR����
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN161 );	// �u�ӔC�҂̎w��...�v���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN161 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;

		case LCD_SCREEN161:		// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�E�u�ӔC�҂̎w��...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN162:		// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�E�ӔC�҂̔F�ؐ��������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN164 );	// �ً}�J���ԍ��̐ݒ���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN164 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN163:		// �ʏ탂�[�h�E�ً}�J���ԍ��ݒ�E�ӔC�҂̔F�؎��s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E�������
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;

		case LCD_SCREEN164:		// �ʏ탂�[�h�E�ً}�J���ԍ��̐ݒ���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){			
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_BANGOU ) ) && ( rcv_length >= 5 ) ){
				
					kinkyuu_touroku_no[ 0 ] = msg[ 1 ];		// �ً}�J���ԍ�
					kinkyuu_touroku_no[ 1 ] = msg[ 2 ];
					kinkyuu_touroku_no[ 2 ] = msg[ 3 ];
					kinkyuu_touroku_no[ 3 ] = msg[ 4 ];
					kinkyuu_touroku_no[ 4 ] = 0;
#if ( VA300S == 0 )				
					send_kinkyuu_touroku_Wait_Ack_Retry();	// PC�ցA�ً}�J���ԍ��ʒm���M�B
#else
					g_RegUserInfoData.KinkyuNum[0] = kinkyuu_touroku_no[0];
					g_RegUserInfoData.KinkyuNum[1] = kinkyuu_touroku_no[1];
					g_RegUserInfoData.KinkyuNum[2] = kinkyuu_touroku_no[2];
					g_RegUserInfoData.KinkyuNum[3] = kinkyuu_touroku_no[3];
					//�t���b�V���ɓo�^�f�[�^��ۑ�����
					dly_tsk( 500/MSEC );
					ercd = SaveBkAuthDataFl();

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
						MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h�ցi�s�v�̂͂������O�̂��߁j
					}

#endif								
					// ����ʁi�ʏ탂�[�h�E������ʁj�ւ̑J�ڂ́A�ً}�J���ԍ��ʒm���M��OK���ʂ���M��A���̎�M�R�}���h�����Ŏ��s����B	

				} else if ( msg[ 0 ] == LCD_CANCEL ){		// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN165 );	// �ʏ탂�[�h������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN165 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;
			
		case LCD_SCREEN165:		// �ʏ탂�[�h�E�ً}�J���ԍ��u���~���Ă���낵���ł����H�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// �u�͂��v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E�������
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// �u�������v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN164 );	// �ʏ탂�[�h�E�ً}�J���ԍ��̐ݒ���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN164 );		// ��ʔԍ��@<-�@���̉��
					}					
				}
			}
			break;			

			
	// �ʏ탂�[�h�E�ً}�J��
		case LCD_SCREEN170:		// �ʏ탂�[�h�E�ً}�J���̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR����

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN171 );	// �u�R�[���Z���^�[�ɘA������...�v���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN171 );		// ��ʔԍ��@<-�@���̉��
				}					
			}
			break;

		case LCD_SCREEN171:		// �ʏ탂�[�h�E�ً}�J���́u�R�[���Z���^�[�ɘA������...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				
				if ( msg[ 0 ] == LCD_NEXT ){			// �u�y�[�W�v�{�^���������ꂽ
					if(g_TechMenuData.SysSpec == 2){	//20160120Miya FinKeyS
						MakeOpenKeyNum();
						befor_scrn_no = FPTN_LCD_SCREEN171;
						befor_scrn_for_ctrl = LCD_SCREEN171;
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN173 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN173 );		// ��ʔԍ��@<-�@���̉��
						}	
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN172 );	// �uID�ԍ�����͂���...�v���
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN172 );	// ��ʔԍ��@<-�@���̉��
						}
					}
					
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN171;
					befor_scrn_for_ctrl = LCD_SCREEN171;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );	// ��ʔԍ��@<-�@���̉��
					}	
				}					
			}
			break;

		case LCD_SCREEN172:		// �ʏ탂�[�h�E�ً}�J���́uID�ԍ�����͂���...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) ) && ( rcv_length >= 5 ) ){
					if ( ( yb_touroku_data.user_id[ 0 ] == msg[ 1 ] )
					  && ( yb_touroku_data.user_id[ 1 ] == msg[ 2 ] )
					  && ( yb_touroku_data.user_id[ 2 ] == msg[ 3 ] )
					  && ( yb_touroku_data.user_id[ 3 ] == msg[ 4 ] ) ){	// ID�ԍ�����v�������B
#if ( VA300S == 0 ) 						
						send_kinkyuu_8keta_Wait_Ack_Retry();	// PC�ցA�ً}�W���ԍ��f�[�^�v�����M�AAck�ENack�҂��ƃ��g���C�t��
#else
						MakeOpenKeyNum();
						befor_scrn_no = FPTN_LCD_SCREEN172;
						befor_scrn_for_ctrl = LCD_SCREEN172;
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN173 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN173 );		// ��ʔԍ��@<-�@���̉��
						}	
#endif						
						// ����ʂւ̑J�ڂ́A�ً}�W���ԍ��f�[�^�v���̉����ŁA�ԍ��ʒm���󂯎�������Ɏ�M�������ōs���B
						
					} else {	// �o�^�ς݃��[�U�[ID�ԍ��ƁA���͂����ԍ����s��v�̎��A
						
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN177 );	// �F�؎��s�~���
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN177 );		// ��ʔԍ��@<-�@���̉��
						}						
					}
					 
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN172;
					befor_scrn_for_ctrl = LCD_SCREEN172;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;

		case LCD_SCREEN173:		// �ʏ탂�[�h�E�ً}�J���́u�ԍ����R�[���Z���^�[��...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_NEXT ){			// �u�y�[�W�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN174 );	// �u�W���ԍ�����͂���...�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN174 );	// ��ʔԍ��@<-�@���̉��
					}			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN173;
					befor_scrn_for_ctrl = LCD_SCREEN173;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;			


		case LCD_SCREEN174:		// �ʏ탂�[�h�E�ً}�J���́u�W���ԍ�����͂��ĉ�����...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU_BANGOU ) ) && ( rcv_length >= 9 ) ){								
					kinkyuu_kaijyo_no[ 0 ] = msg[ 1 ];		// �ً}�W���ԍ�
					kinkyuu_kaijyo_no[ 1 ] = msg[ 2 ];
					kinkyuu_kaijyo_no[ 2 ] = msg[ 3 ];
					kinkyuu_kaijyo_no[ 3 ] = msg[ 4 ];
					kinkyuu_kaijyo_no[ 4 ] = msg[ 5 ];
					kinkyuu_kaijyo_no[ 5 ] = msg[ 6 ];
					kinkyuu_kaijyo_no[ 6 ] = msg[ 7 ];
					kinkyuu_kaijyo_no[ 7 ] = msg[ 8 ];
					kinkyuu_kaijyo_no[ 8 ] = 0;
#if ( VA300S == 0 ) 
					send_kinkyuu_kaijyou_Wait_Ack_Retry();	// PC�ցA�ً}�J���ԍ����M�AAck�ENack�҂��ƃ��g���C�t��
#else
					g_UseProcNum.InOpenCode[0] = kinkyuu_kaijyo_no[0];
					g_UseProcNum.InOpenCode[1] = kinkyuu_kaijyo_no[1];
					g_UseProcNum.InOpenCode[2] = kinkyuu_kaijyo_no[2];
					g_UseProcNum.InOpenCode[3] = kinkyuu_kaijyo_no[3];
					g_UseProcNum.InOpenCode[4] = kinkyuu_kaijyo_no[4];
					g_UseProcNum.InOpenCode[5] = kinkyuu_kaijyo_no[5];
					g_UseProcNum.InOpenCode[6] = kinkyuu_kaijyo_no[6];
					g_UseProcNum.InOpenCode[7] = kinkyuu_kaijyo_no[7];

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN175 );	// �ً}�ԍ����͉�ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN175 );		// ��ʔԍ��@<-�@���̉��
					}

#endif								
					// ����ʁi�ً}�ԍ����͉�ʁj�ւ̑J�ڂ́A�ً}�J���ԍ��W�����M��OK���ʂ���M��A���̎�M�R�}���h�����Ŏ��s����B			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN174;
					befor_scrn_for_ctrl = LCD_SCREEN174;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;


		case LCD_SCREEN175:		// �ʏ탂�[�h�E�u�ً}�ԍ�����͂��ĉ�����...�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_KINKYUU_BANGOU ) ) && ( rcv_length >= 5 ) ){								
					kinkyuu_input_no[ 0 ] = msg[ 1 ];		// ���͂��ꂽ�ً}�ԍ�
					kinkyuu_input_no[ 1 ] = msg[ 2 ];
					kinkyuu_input_no[ 2 ] = msg[ 3 ];
					kinkyuu_input_no[ 3 ] = msg[ 4 ];
					kinkyuu_input_no[ 4 ] = 0;
#if ( VA300S == 0 )					
					send_kinkyuu_input_Wait_Ack_Retry();	// PC�ցA�ً}�ԍ��̑Ó����₢���킹�m�F�v�����M�AAck�ENack�҂��ƃ��g���C�t��
#endif								
//#else
#if ( VA300S == 1 )	//20140905Miya				
					g_UseProcNum.InKinkyuNum[0] = kinkyuu_input_no[0]; 
					g_UseProcNum.InKinkyuNum[1] = kinkyuu_input_no[1]; 
					g_UseProcNum.InKinkyuNum[2] = kinkyuu_input_no[2]; 
					g_UseProcNum.InKinkyuNum[3] = kinkyuu_input_no[3]; 

					ercd = ChekKinkyuKaijyouKey();
					if( ercd == E_OK ){
						g_AuthType = 2;	//20160120Miya
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN176 );	// �J��OK�́���ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN176 );		// ��ʔԍ��@<-�@���̉��
							dly_tsk( 500/MSEC );
							send_sio_Ninshou(1, 2, 0);				//�����R�}���h���s
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN177 );	// �J��OK�́~��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN177 );		// ��ʔԍ��@<-�@���̉��
						}
						
					}
#endif								
					// ����ʁiOK/NG��ʁj�ւ̑J�ڂ́A�ً}�ԍ��Ó����m�F�v�����M��OK���ʂ���M��A���̎�M�R�}���h�����Ŏ��s����B			
				
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					befor_scrn_no = FPTN_LCD_SCREEN175;
					befor_scrn_for_ctrl = LCD_SCREEN175;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN178 );	// �ʏ탂�[�h�E�ً}�J���u���~���܂����H�v���
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN178 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;

		case LCD_SCREEN176:		// �ʏ탂�[�h�E�ً}�J���̔F�ؐ��������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E������ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN177:		// �ʏ탂�[�h�E�ً}�J���̔F�؎��s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E������ʂցB
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;

			
		case LCD_SCREEN178:		// �ʏ탂�[�h�E�ً}�J�����u���~���܂����H�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// �u�͂��v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E�������
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// �u�������v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �O��ʂ֖߂�
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );	// ��ʔԍ��@<-�@���̉��
					}					
				}
			}
			break;

		case LCD_SCREEN180:		// �ʏ탂�[�h�E�p�X���[�h�J���ݒ�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR����
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN181 );	// �u�ӔC�҂̎w��...�v���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN181 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;

		case LCD_SCREEN181:		// �ʏ탂�[�h�E�p�X���[�h�J���ݒ�E�u�ӔC�҂̎w��...�v���
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		case LCD_SCREEN182:		// �ʏ탂�[�h�E�p�X���[�h�J���ݒ�E�ӔC�҂̔F�ؐ��������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// �p�X���[�h�J���̐ݒ���
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN184 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN183:		// �ʏ탂�[�h�E�p�X���[�h�J���ݒ�E�ӔC�҂̔F�؎��s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E�������
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
		case LCD_SCREEN184:		// �J���p�p�X���[�h�ύX�E���j���[���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_PASS_HENKOU_REQ ){						// �p�X���[�h�ύX�{�^��
					//if(g_TechMenuData.SysSpec == 0){	//20160108Miya FinKeyS
					if(g_PasswordOpen2.family_sw == 0){	//20160120Miya FinKeyS
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN185 );		// �u�p�X���[�h�ύX�\���v��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN185 );		// ��ʔԍ��@<-�@���̉��
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );		// �u�p�X���[�h�ύX�\���v��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN188 );		// ��ʔԍ��@<-�@���̉��
						}
					}
				}else if ( msg[ 0 ] == LCD_PASS_SETTEI_HENKOU_REQ ){		// �p�X���[�h�J���ݒ�{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN186 );		// �u�p�X���[�h�J���ݒ�\���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN186 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_NOUSE ){					// ���g�p�{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// ��ʔԍ��@<-�@���̉��
					}
				}else if ( msg[ 0 ] == LCD_MAINTE_END ){					// �I���{�^��
					//20160108Miya FinKeyS
					if( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// �ʏ탂�[�h�E�������
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN620 );		// ��ʔԍ��@<-�@���̉��
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E�������
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
						}
					}
				}
			}
			break;

		case LCD_SCREEN185:		// �J���p�p�X���[�h�ύX���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KEYIN_PASSWORD ){		// ����{�^��
					if( rcv_length >= 5 ){
						//if(g_TechMenuData.SysSpec == 0){	//20160108Miya FinKeyS
						if(g_PasswordOpen2.family_sw == 0){	//20160120Miya FinKeyS
							g_PasswordOpen.password[ 0 ] = msg[ 1 ];		// �p�X���[�h�W���ԍ�
							g_PasswordOpen.password[ 1 ] = msg[ 2 ];
							g_PasswordOpen.password[ 2 ] = msg[ 3 ];
							g_PasswordOpen.password[ 3 ] = msg[ 4 ];
							g_PasswordOpen.password[ 4 ] = msg[ 5 ];
							g_PasswordOpen.password[ 5 ] = msg[ 6 ];
							g_PasswordOpen.password[ 6 ] = msg[ 7 ];
							g_PasswordOpen.password[ 7 ] = msg[ 8 ];
							g_PasswordOpen.password[ 8 ] = 0;
						}else{
							g_PasswordOpen.password[ 0 ] = msg[ 1 ];		// �p�X���[�h�W���ԍ�
							g_PasswordOpen.password[ 1 ] = msg[ 2 ];
							g_PasswordOpen.password[ 2 ] = msg[ 3 ];
							g_PasswordOpen.password[ 3 ] = msg[ 4 ];
							g_PasswordOpen.password[ 4 ] = msg[ 5 ];
							g_PasswordOpen.password[ 5 ] = msg[ 6 ];
							g_PasswordOpen.password[ 6 ] = msg[ 7 ];
							g_PasswordOpen.password[ 7 ] = msg[ 8 ];
							g_PasswordOpen.password[ 8 ] = 0;

							if(g_PasswordOpen2.family_sw == 1){
								g_PasswordOpen2.keta[g_PasswordOpen2.num] = g_PasswordOpen.keta;
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 0 ] = msg[ 1 ];		// �p�X���[�h�W���ԍ�
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 1 ] = msg[ 2 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 2 ] = msg[ 3 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 3 ] = msg[ 4 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 4 ] = msg[ 5 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 5 ] = msg[ 6 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 6 ] = msg[ 7 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 7 ] = msg[ 8 ];
								g_PasswordOpen2.password[g_PasswordOpen2.num][ 8 ] = 0;
							}
						}


						//�t���b�V���ɓo�^�f�[�^��ۑ�����
						//dly_tsk( 500/MSEC );
						ercd = SaveBkAuthDataFl();
					}

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// �ʏ탂�[�h�̑ҋ@��ʂցB

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// ��ʔԍ��@<-�@���̉��
					}

				}else if(msg[ 0 ] == LCD_MENU){

				}else if(msg[ 0 ] == LCD_CANCEL){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// �ʏ탂�[�h�̑ҋ@��ʂցB

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// ��ʔԍ��@<-�@���̉��
					}
				}else if(msg[ 0 ] == LCD_MAINTE){
				}

			}
			break;
		case LCD_SCREEN186:		// �u�p�X���[�h�J���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){				// �u�߂�v�������ꂽ
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( ( msg[ 0 ] == LCD_ENTER ) && ( rcv_length >= 4 ) ){	// �u�m��v�������ꂽ
					if( msg[ 1 ] == 1 )	g_PasswordOpen.hide_num = FLG_ON;
					else				g_PasswordOpen.hide_num = FLG_OFF;

					if( msg[ 2 ] == 1 )	g_PasswordOpen.kigou_inp = FLG_ON;
					else				g_PasswordOpen.kigou_inp = FLG_OFF;

					if( msg[ 3 ] == 1 )	g_PasswordOpen.random_key = FLG_ON;
					else				g_PasswordOpen.random_key = FLG_OFF;

					if( msg[ 4 ] == 1 )	g_PasswordOpen2.family_sw = FLG_ON;
					else				g_PasswordOpen2.family_sw = FLG_OFF;

					for(i = 0 ; i < 10 ; i++ ){
						g_key_arry[i] = i;	//�p�X���[�h�J���p�L�[�z�񏉊���
					}

					//�t���b�V���ɓo�^�f�[�^��ۑ�����
					//dly_tsk( 500/MSEC );
					ercd = SaveBkAuthDataFl();
			
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN184 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN187:		// �ʏ탂�[�h�E�J���p�p�X���[�h�ύX���u���~���܂����H�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){					// �u�͂��v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E�������
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( msg[ 0 ] == LCD_NO ){			// �u�������v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, ( FLGPTN )befor_scrn_no );	// �O��ʂ֖߂�
					if ( ercd == E_OK ){
						ChgScreenNo( ( UB )befor_scrn_for_ctrl );	// ��ʔԍ��@<-�@���̉��
					}					
				}
			}
			break;
//20160108Miya FinKryS ->
		case LCD_SCREEN188:		// �ӔC�ҁE��ʎ҂̍폜�ԍ��I�����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
		
			if ( ( msg[ 0 ] == LCD_YUBI_ID ) && ( rcv_length >= 4 ) ){	
			
				//yb_touroku_data.yubi_seq_no[ 0 ] = msg[ 1 ];	// �폜���̎w�ԍ�
				//yb_touroku_data.yubi_seq_no[ 1 ] = msg[ 2 ];
				//yb_touroku_data.yubi_seq_no[ 2 ] = msg[ 3 ];
				//yb_touroku_data.yubi_seq_no[ 3 ] = ',';

				ubtmp = msg[1] - 0x30;
				tp_num = 100 * (unsigned short)ubtmp;
				ubtmp = msg[2] - 0x30;
				tp_num += (10 * (unsigned short)ubtmp);
				ubtmp = msg[3] - 0x30;
				tp_num += (unsigned short)ubtmp;
				
				if(tp_num > 0){
					tp_num -= 1;
					g_PasswordOpen2.num = (short)tp_num;
				
					if(g_RegBloodVesselTagData[tp_num].RegInfoFlg == 1){	//�o�^����
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN185 );
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN185 );		// ��ʔԍ��@<-�@���̉��
						}
					}else{
						yb_touroku_data20[tp_num+1].yubi_seq_no[ 0 ] = '0';
						yb_touroku_data20[tp_num+1].yubi_seq_no[ 1 ] = '0';
						yb_touroku_data20[tp_num+1].yubi_seq_no[ 2 ] = '0';
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN188 );		// ��ʔԍ��@<-�@���̉��
						}
					}
				}else{
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN188 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN188 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			} else if ( ( msg[ 0 ] == LCD_CANCEL ) && ( rcv_length >= 1 ) ){
			
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN184 );	// ��ʔԍ��@<-�@���̉��
				}	
								
			} else {
				nop();	
			}	
			break;
//20160108Miya FinKryS <-

//20160108Miya FinKryS ->
//�ʏ탂�[�h2 FinKeyS�p
		case LCD_SCREEN601:		// �ʏ탂�[�h�̃��j���[���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			ode_oru_sw = 0;
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_ODEKAKE ){							// ���ł����{�^������
					ode_oru_sw = 1;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN610 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN610 );					// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_ORUSUBAN ){					// ������ԃ{�^������
					ode_oru_sw = 2;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN611 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN611 );					// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_SETTEI ){					// �ݒ�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN612 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN612 );					// ��ʔԍ��@<-�@���̉��
					}					
				} else if ( msg[ 0 ] == LCD_KINKYUU_KAIJYOU ){			//  "�ً}����"�{�^���������ꂽ"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN170 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN170 );					// ��ʔԍ��@<-�@���̉��
					}													
				} else if ( msg[ 0 ] == LCD_PASSWORD_OPEN ){			//  "�p�X���[�h�J���{�^���������ꂽ"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN608 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN608 );					// ��ʔԍ��@<-�@���̉��
					}													
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){					// �G���[�����o����
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN609 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN609 );					// ��ʔԍ��@<-�@���̉��
					}													
				}
			}
			break;
		
		case LCD_SCREEN602:		// �ʏ탂�[�h�̑ҋ@��ʂ̃u�����N�\��
			break;				// LCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B
			
		
		case LCD_SCREEN603:		// �F�؊�������ʁB	
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
					
#if ( VA300S == 1 || VA300S == 2 )						
			//�t���b�V���ɓo�^�f�[�^��ۑ�����
			if( g_AuthOkCnt >= 50 && g_AuthType == 0 && ode_oru_sw == 0){	//20160120Miya
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN613 );	// ���΂炭���҂�������
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN613 );		// ��ʔԍ��@<-�@���̉��
					if ( Pfail_mode_count == 0 ){ 
						MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
					}	else	{
						MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
					}
				}
				break;
			}
#endif

			ode_oru_sw = 0;	//20160120Miya
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// �ʏ탂�[�h�̑ҋ@��ʂցB

			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN601 );		// ��ʔԍ��@<-�@���̉��
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
				}	else	{
					MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
				}
			}
			break;
		
		case LCD_SCREEN604:		// �F�؊����~��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			ode_oru_sw = 0;	//20160120Miya
				
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// �ʏ탂�[�h�̃��j���[��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN601 );		// ��ʔԍ��@<-�@���̉��
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
				}	else	{
					MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
				}
			}
			break;
		
		//20140925Miya password_open
		case LCD_SCREEN608:		// �J���p�p�X���[�h���͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KEYIN_PASSWORD ){		// ����{�^��
					if( rcv_length >= 5 ){
						g_InPasswordOpen.password[ 0 ] = msg[ 1 ];		// �p�X���[�h�W���ԍ�
						g_InPasswordOpen.password[ 1 ] = msg[ 2 ];
						g_InPasswordOpen.password[ 2 ] = msg[ 3 ];
						g_InPasswordOpen.password[ 3 ] = msg[ 4 ];
						g_InPasswordOpen.password[ 4 ] = msg[ 5 ];
						g_InPasswordOpen.password[ 5 ] = msg[ 6 ];
						g_InPasswordOpen.password[ 6 ] = msg[ 7 ];
						g_InPasswordOpen.password[ 7 ] = msg[ 8 ];
						g_InPasswordOpen.password[ 8 ] = 0;
						
						dly_tsk((500/MSEC));	//500msec�̃E�G�C�g
						ercd = ChekPasswordOpenKey();
						if( ercd == E_OK ){
							g_AuthType = 1;	//20160120Miya
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN603 );	// �J��OK�́���ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN603 );		// ��ʔԍ��@<-�@���̉��
								dly_tsk( 500/MSEC );
								send_sio_Ninshou(1, 1, ode_oru_sw);
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN604 );	// �J��OK�́~��ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN604 );		// ��ʔԍ��@<-�@���̉��
							}
						}
					}
				}else if(msg[ 0 ] == LCD_MENU){
					if ( rcv_length >= 0 ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN180 );	// 
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN180 );		// ��ʔԍ��@<-�@���̉��
						}
					}
				}else if(msg[ 0 ] == LCD_CANCEL){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// �ʏ탂�[�h�̑ҋ@��ʂցB

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );		// ��ʔԍ��@<-�@���̉��
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
						}	else	{
							MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
						}
					}
				}else if(msg[ 0 ] == LCD_MAINTE){
#if ( VA300S == 0 )						
					send_donguru_chk_Wait_Ack_Retry();	// PC�ցA�h���O���̗L���m�F�𑗐M�B						
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X���[�h��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					} else {
						nop();			// �G���[�����̋L�q
					}

					MdCngMode( MD_MAINTE );				// ���u���[�h�������e�i���X���[�h��
					ercd = set_flg( ID_FLG_MAIN, FPTN_SEND_REQ_MAINTE_CMD );	// ���C��Task�ցA�����e�i���X���[�h�ؑւ��ʒm���M���˗��B
					if ( ercd != E_OK ){
						nop();		// �G���[�����̋L�q	
					}
#endif						
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){					//  �G���[�����o����
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN609 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN609 );					// ��ʔԍ��@<-�@���̉��
					}													
				}

			}
			break;
		case LCD_SCREEN609:		// �u�G���[�\���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// �u�߂�v�������ꂽ
				
					//++g_MainteLog.err_rcnt;
					//g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;

					//�G���[��10�ȏ゠��ꍇ
					cnt1 = (int)g_MainteLog.err_wcnt;
					cnt2 = (int)g_MainteLog.err_rcnt;
					if( cnt1 != cnt2 ){
						if( cnt1 > 0x7F || cnt2 > 0x7F ){
							g_MainteLog.err_wcnt = 0;
							g_MainteLog.err_rcnt = 0;
						}else{
							if( cnt1 > cnt2 ){
								cnt0 = cnt1 - cnt2;
							}else{
								cnt0 = 128 - cnt2;
								cnt0 = cnt0 + cnt1;
							}
							if( cnt0 > 10 ){
								cnt2 = cnt1 - 10;
								if( cnt2 < 0 ){
									cnt2 = 128 + cnt2;
								}
							}
							g_MainteLog.err_rcnt = (unsigned short)cnt2;
						}
					}

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	/// �ʏ탂�[�h�̑ҋ@��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );		// ��ʔԍ��@<-�@���̉��
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
						}	else	{
							MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
						}
					}
				}
			}
			break;
		case LCD_SCREEN610:		// ���ł����ݒ���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				
				if ( msg[ 0 ] == LCD_PASSWORD_OPEN ){					//  "�p�X���[�h�J���{�^���������ꂽ"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN608 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN608 );					// ��ʔԍ��@<-�@���̉��
					}													
				} else if ( msg[ 0 ] == LCD_CANCEL ){					// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// �ʏ탂�[�h�̑ҋ@��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );					// ��ʔԍ��@<-�@���̉��
					}
				}					
			}
			break;
		case LCD_SCREEN611:		// ������Ԑݒ���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				
				if ( msg[ 0 ] == LCD_PASSWORD_OPEN ){					//  "�p�X���[�h�J���{�^���������ꂽ"
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN608 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN608 );					// ��ʔԍ��@<-�@���̉��
					}													
				} else if ( msg[ 0 ] == LCD_CANCEL ){					// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );	// �ʏ탂�[�h�̑ҋ@��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );					// ��ʔԍ��@<-�@���̉��
					}
				}					
			}
			break;
		case LCD_SCREEN612:		// �ʏ탂�[�h�̐ݒ���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_TOUROKU ){							// �o�^�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN120 );	// �ʏ탂�[�h�o�^��ʂցB
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )						
						g_RegFlg = 0;
#endif
						ChgScreenNo( LCD_SCREEN120 );					// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_SAKUJYO ){					// �폜�{�^������
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN140 );	// �ʏ탂�[�h�폜��ʂցB
					if ( ercd == E_OK ){
#if ( VA300S == 1 || VA300S == 2 )						
						g_RegFlg = 0;
#endif
						ChgScreenNo( LCD_SCREEN140 );					// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_KINKYUU_SETTEI ){			// "�ً}�ԍ��ݒ�"�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN160 );	// �ً}�ԍ��ݒ�E������ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN160 );					// ��ʔԍ��@<-�@���̉��
					}					
				} else if ( msg[ 0 ] == LCD_BACK ){						// �߂�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN601 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN601 );					// ��ʔԍ��@<-�@���̉��
					}													
				} else if ( msg[ 0 ] == LCD_NOUSE ){					// "���g�p"�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN612 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN612 );					// ���̉�ʂɖ߂�
					}													
				}
			}
			break;
		case LCD_SCREEN613:		// �u���΂炭���܂����������v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				g_Diagnosis_start = 1;
			}
			break;

	// �H�����[�h
		case LCD_SCREEN620:		// �u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_OFF);//20160905Miya B-PWR����

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	// 
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN621 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN621:		// 
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_KEYIN_PASSWORD ){		// ����{�^��
					if( rcv_length >= 5 ){
						g_InPasswordOpen.password[ 0 ] = msg[ 1 ];		// �p�X���[�h�W���ԍ�
						g_InPasswordOpen.password[ 1 ] = msg[ 2 ];
						g_InPasswordOpen.password[ 2 ] = msg[ 3 ];
						g_InPasswordOpen.password[ 3 ] = msg[ 4 ];
						g_InPasswordOpen.password[ 4 ] = msg[ 5 ];
						g_InPasswordOpen.password[ 5 ] = msg[ 6 ];
						g_InPasswordOpen.password[ 6 ] = msg[ 7 ];
						g_InPasswordOpen.password[ 7 ] = msg[ 8 ];
						g_InPasswordOpen.password[ 8 ] = 0;
						
						dly_tsk((500/MSEC));	//500msec�̃E�G�C�g
						ercd = ChekPasswordOpenKey();
						if( ercd == E_OK ){
							g_AuthType = 1;	//20160120Miya
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN623 );	// �J��OK�́���ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN623 );		// ��ʔԍ��@<-�@���̉��
								dly_tsk( 500/MSEC );
								send_sio_Ninshou(1, 1, ode_oru_sw);
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN624 );	// �J��OK�́~��ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN624 );		// ��ʔԍ��@<-�@���̉��
							}
						}
					}
				}else if(msg[ 0 ] == LCD_MENU){
					if( rcv_length >= 5 ){
						g_InPasswordOpen.password[ 0 ] = msg[ 1 ];		// �p�X���[�h�W���ԍ�
						g_InPasswordOpen.password[ 1 ] = msg[ 2 ];
						g_InPasswordOpen.password[ 2 ] = msg[ 3 ];
						g_InPasswordOpen.password[ 3 ] = msg[ 4 ];
						g_InPasswordOpen.password[ 4 ] = msg[ 5 ];
						g_InPasswordOpen.password[ 5 ] = msg[ 6 ];
						g_InPasswordOpen.password[ 6 ] = msg[ 7 ];
						g_InPasswordOpen.password[ 7 ] = msg[ 8 ];
						g_InPasswordOpen.password[ 8 ] = 0;
						
						dly_tsk((500/MSEC));	//500msec�̃E�G�C�g
						ercd = ChekPasswordOpenKey();
						if( ercd == E_OK ){
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// 
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN184 );		// ��ʔԍ��@<-�@���̉��
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN624 );	// �J��OK�́~��ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN624 );		// ��ʔԍ��@<-�@���̉��
							}
						}
					}else{
						if(g_PasswordOpen.keta == 0){
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN184 );	// 
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN184 );		// ��ʔԍ��@<-�@���̉��
							}
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN624 );	// �J��OK�́~��ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN624 );		// ��ʔԍ��@<-�@���̉��
							}
						}
					}
				}else if(msg[ 0 ] == LCD_CANCEL){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	// �ʏ탂�[�h�̑ҋ@��ʂցB

					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN621 );		// ��ʔԍ��@<-�@���̉��
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
						}	else	{
							MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
						}
					}
				}else if(msg[ 0 ] == LCD_MAINTE){
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X���[�h��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					} else {
						nop();			// �G���[�����̋L�q
					}

					MdCngMode( MD_MAINTE );				// ���u���[�h�������e�i���X���[�h��
					ercd = set_flg( ID_FLG_MAIN, FPTN_SEND_REQ_MAINTE_CMD );	// ���C��Task�ցA�����e�i���X���[�h�ؑւ��ʒm���M���˗��B
					if ( ercd != E_OK ){
						nop();		// �G���[�����̋L�q	
					}
				} else if ( msg[ 0 ] == LCD_ERR_REQ ){					//  �G���[�����o����
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN625 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN625 );					// ��ʔԍ��@<-�@���̉��
					}													
				}

			}
			break;
		case LCD_SCREEN623:		// �F�؊�������ʁB	
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
					
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	// �ʏ탂�[�h�̑ҋ@��ʂցB

			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN621 );		// ��ʔԍ��@<-�@���̉��
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
				}	else	{
					MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
				}
			}
			break;
		
		case LCD_SCREEN624:		// �F�؊����~��ʁB
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
				
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	// �ʏ탂�[�h�̃��j���[��ʂցB
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN621 );		// ��ʔԍ��@<-�@���̉��
				if ( Pfail_mode_count == 0 ){ 
					MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
				}	else	{
					MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
				}
			}
			break;

		case LCD_SCREEN625:		// �u�G���[�\���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// �u�߂�v�������ꂽ
				
					//�G���[��10�ȏ゠��ꍇ
					cnt1 = (int)g_MainteLog.err_wcnt;
					cnt2 = (int)g_MainteLog.err_rcnt;
					if( cnt1 != cnt2 ){
						if( cnt1 > 0x7F || cnt2 > 0x7F ){
							g_MainteLog.err_wcnt = 0;
							g_MainteLog.err_rcnt = 0;
						}else{
							if( cnt1 > cnt2 ){
								cnt0 = cnt1 - cnt2;
							}else{
								cnt0 = 128 - cnt2;
								cnt0 = cnt0 + cnt1;
							}
							if( cnt0 > 10 ){
								cnt2 = cnt1 - 10;
								if( cnt2 < 0 ){
									cnt2 = 128 + cnt2;
								}
							}
							g_MainteLog.err_rcnt = (unsigned short)cnt2;
						}
					}

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN621 );	/// �ʏ탂�[�h�̑ҋ@��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN621 );		// ��ʔԍ��@<-�@���̉��
						if ( Pfail_mode_count == 0 ){ 
							MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h�ցi�K�v�������O�̂��߁j
						}	else	{
							MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h�ցi�K�v�������O�̂��߁j
						}
					}
				}
			}
			break;

		default:
			break;
	}
}


/*==========================================================================*/
/**
 *	���̉�ʕ\���v���t���O�Z�b�g�i���ʎd�l�̏ꍇ�j
 */
/*==========================================================================*/
ER NextScrn_Control( void )
{
	ER ercd;
	UB S_No;
	ER_UINT i;
	ER_UINT rcv_length;
	UB msg[ 128 ];
	UB tmp;	
	unsigned short dat;
	
	S_No = GetScreenNo();
	
	switch ( S_No ) {
		

	// �����e�i���X���
		case LCD_SCREEN200:		// �����e�i���X�E���[�h�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			send_sio_BPWR(FLG_ON);//20160905Miya B-PWR����

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN201 );	// �����e�i���X�E���[�h��ID�ԍ����͉��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN201 );		// ��ʔԍ��@<-�@���̉��
				}					
			}
			break;

		case LCD_SCREEN201:		// �����e�i���X��ʂ̃p�X���[�h�ԍ����͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){			
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_USER_ID ) ) && ( rcv_length >= 5 ) ){
				
					mainte_password[ 0 ] = msg[ 1 ];	// �p�X���[�h�ԍ�
					mainte_password[ 1 ] = msg[ 2 ];
					mainte_password[ 2 ] = msg[ 3 ];
					mainte_password[ 3 ] = msg[ 4 ];
					mainte_password[ 4 ] = 0;
#if ( VA300S == 0 )				
					send_password_chk_Wait_Ack_Retry();	// PC�ցA�p�X���[�h�̈�v�m�F�v�����M�B
#else

#if(KOUJYOUCHK == 1 || AUTHTEST >= 1)
					if(msg[1] == '9' &&  msg[2] == '9' && msg[3] == '9' && msg[4] == '9'){
						mainte_password[ 0 ] = '3';	// �p�X���[�h�ԍ�
						mainte_password[ 1 ] = '2';
						mainte_password[ 2 ] = '2';
						mainte_password[ 3 ] = '7';
					}
#endif

					if( mainte_password[0] == '3' && mainte_password[1] == '2' && mainte_password[2] == '2' && mainte_password[3] == '7' ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_MAINTE );				// ���u���[�h�������e�i���X���[�h��
							ercd = set_flg( ID_FLG_MAIN, FPTN_SEND_REQ_MAINTE_CMD );	// ���C��Task�ցA�����e�i���X���[�h�ؑւ��ʒm���M���˗��B
							if ( ercd != E_OK ){
								nop();		// �G���[�����̋L�q	
							}
						}
					}else{
						if ( GetSysSpec() == SYS_SPEC_MANTION ){	// �}���V������L���d�l
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h�E������ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
							} else {
								nop();			// �G���[�����̋L�q
							}
						} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){//�@�P�΂P�d�l�i�I�t�B�X�d�l�j
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// �ʏ탂�[�h�E������ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN500 );		// ��ʔԍ��@<-�@���̉��
							} else {
								nop();		// �G���[�����̋L�q
							}
						} else if ( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// �H����ʂցB
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN620 );		// ��ʔԍ��@<-�@���̉��
							} else {
								nop();		// �G���[�����̋L�q
							}
						}
					}	
#endif								
					// �����e�i���X�E���j���[��ʂւ̑J�ڂ́A�p�X���[�h�̈�v�m�F�v�����M�̌��ʂ�OK���������A���̎�M�R�}���h�����Ŏ��s����B�B	

				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ

					if ( GetSysSpec() == SYS_SPEC_MANTION ){ 	// �}���V�����E��L���d�l�̏ꍇ
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h������ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
#if ( VA300S == 0 ) 
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
#endif	
							}
					} else if ( GetSysSpec() == SYS_SPEC_OFFICE ){//�@�P�΂P�d�l�i�I�t�B�X�d�l�j
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// �ʏ탂�[�h������ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN500 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
#if ( VA300S == 0 ) 
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
#endif	
						}
					} else if ( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// �H����ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN620 );		// ��ʔԍ��@<-�@���̉��
						} else {
							nop();		// �G���[�����̋L�q
						}
					}
				}
			}
			break;
			
		case LCD_SCREEN202:		// �����e�i���X�E���j���[���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){

//20140925Miya cng mainte
				if ( msg[ 0 ] == LCD_JYOUHOU_REQ ){	// ���{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN220 );	// �u�o�[�W�����\���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN220 );		// ��ʔԍ��@<-�@���̉��
					}			
				} else if ( msg[ 0 ] == LCD_SETTEI_HENKOU_REQ ){			// �h�ݒ�ύX�h�{�^���������ꂽ
					if(g_MainteLvl == 1){	//20160711Miya �f���@
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
						}
						break;
					}
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN240 );	// �u�ݒ�ύX�v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN240 );		// ��ʔԍ��@<-�@���̉��
					}
				} else if ( msg[ 0 ] == LCD_SINDAN_REQ ){			// �h�f�f�h�{�^���������ꂽ
					if(g_MainteLvl == 1){	//20160711Miya �f���@
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
						}
						break;
					}
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN203 );	// �u���΂炭���܂����������v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN203 );		// ��ʔԍ��@<-�@���̉��
					}			
				} else if ( msg[ 0 ] == LCD_SYOKI_SETTEI_REQ ){			// �h�����ݒ�h�{�^���������ꂽ
					//20160112Miya FinKeyS
					if( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
						dip_sw_data[0] = 1;
					}
				
					if(dip_sw_data[0] == 1){
						//g_cmr_dbgcnt = 0;	//20160610Miya
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN260 );	// �u�Z�p�����e���j���[�v��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN260 );		// ��ʔԍ��@<-�@���̉��
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �u�Z�p�����e���j���[�v��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
						}
					}
/*
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN206 );	// �������ڍs�u�폜���Ă���낵���ł����H�v��ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN206 );		// ��ʔԍ��@<-�@���̉��
					}
*/
				} else if ( msg[ 0 ] == LCD_MAINTE_END ){	// �I���{�^��

					if ( GetSysSpec() == SYS_SPEC_MANTION ){ 	// �}���V�����E��L���d�l�̏ꍇ
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
#if ( VA300S == 0 ) 
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
#endif
						}
					}else if( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN620 );	// �ʏ탂�[�h��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN620 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_INITIAL );
						}
					} else {									// �P�΂P�F�؎d�l�̏ꍇ
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// �ʏ탂�[�h��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN500 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
#if ( VA300S == 0 )
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
#endif
						}						
					}
				}


/*
				if ( msg[ 0 ] == LCD_MAINTE_SHOKIKA_REQ ){	// �������{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN206 );	// �������ڍs�u�폜���Ă���낵���ł����H�v��ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN206 );		// ��ʔԍ��@<-�@���̉��
					}
					
				} else if ( msg[ 0 ] == LCD_JYOUHOU_REQ ){			// �h���h�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN207 );	// �u�o�[�W�����\���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN207 );		// ��ʔԍ��@<-�@���̉��
					}			
					
				} else if ( msg[ 0 ] == LCD_SPEC_CHG_REQ ){			// ,			// �h�d�l�ؑցh�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN208 );	// �u�d�l�ؑցv��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN208 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( msg[ 0 ] == LCD_FULL_PIC_SEND_REQ ){	// �t���摜���M�{�^��
#if ( VA300S == 2 ) 
					DebugSendCmd_210();
#else
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN203 );	// �u�w���Z�b�g����...�v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN203 );		// ��ʔԍ��@<-�@���̉��
					}			
#endif			
				} else if ( msg[ 0 ] == LCD_MAINTE_END ){	// �I���{�^��
					if ( GetSysSpec() == SYS_SPEC_MANTION ){ 	// �}���V�����E��L���d�l�̏ꍇ
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN100 );	// �ʏ탂�[�h��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN100 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
#if ( VA300S == 0 ) 
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
#endif
						}
					} else {									// �P�΂P�F�؎d�l�̏ꍇ
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN500 );	// �ʏ탂�[�h��ʂցB
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN500 );		// ��ʔԍ��@<-�@���̉��
							MdCngMode( MD_NORMAL );				// ���u���[�h��ʏ탂�[�h��
#if ( VA300S == 0 )
							ercd = SndCmdCngMode( (UINT)MD_NORMAL );	// PC��	�ʏ탂�[�h�ؑւ��ʒm�𑗐M
#endif
						}						
					}
				}
*/		
			break;
			
		case LCD_SCREEN203:		// �u���΂炭���܂����������v���
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				g_Diagnosis_start = 1;
			}
			break;				// ���̃Z���T�[���m�҂��BLCD�^�X�N�́A�����ւ�change Req�͏o���Ȃ��͂��B

			
		case LCD_SCREEN204:		// �t���摜���M�E�w�摜�̎擾���������
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				g_Diagnosis_start = 0;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[��ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN205:		// �t���摜���M�E�w�摜�̎擾���s�~���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[��ʂ�
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
			
		case LCD_SCREEN206:		// �������ڍs�u�폜���Ă���낵���ł����H�v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// �u�͂��v�������ꂽ
				
#if ( VA300S == 0 )
					send_touroku_init_Wait_Ack_Retry();		// PC�ցA�o�^�f�[�^�̏������v�����M�B
					if ( ercd == E_OK ){
						req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
					}
#else
					ercd = InitBkAuthData();
					send_sio_Touroku_AllDel();		// VA300S����Box�փV���A���œo�^�f�[�^�������i�ꊇ�폜�j�R�}���h(05)�𑗐M�B
					MdCngSubMode( SUB_MD_ALLDEL );		// �ꊇ�폜���M���iVA300���j
					//ercd = InitRegImgArea();
					//FpgaSetWord(FPGA_RECONF, 0x01, 0x01);
					while ( MdGetSubMode() != SUB_MD_IDLE ){	// ACK��M����܂őҋ@�B
						dly_tsk( 25/MSEC );			
					}
					req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
#endif
										
				} else if ( msg[ 0 ] == LCD_NO ){		// �u�������v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[�I�����
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					}					
				}
			}
			break;
			
		case LCD_SCREEN207:		// �u�o�[�W�����\���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){			// �u�߂�v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[��ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
			
		case LCD_SCREEN208:		// �d�l�ؑւ����
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){				// �u�߂�v�������ꂽ
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );	// �����e�i���X�E���j���[��ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( ( msg[ 0 ] == LCD_ENTER ) && ( rcv_length >= 3 ) ){	// �u�m��v�������ꂽ
				
					if ( ( msg[ 1 ] >= 0 ) && ( msg[ 1 ] <= 2 ) ){	// va300.h "enum SYS_SPEC"���Q�Ƃ̂��ƁB
						if ( ( msg[ 2 ] != SYS_SPEC_DEMO ) ){
							tmp = msg[ 1 ];					// DEMO���[�h�łȂ��ꍇ
						} else {
							tmp = msg[ 1 ] + 3;				// DEMO���[�h�̏ꍇ
						}
						ercd = lan_set_eep( EEP_SYSTEM_SPEC, tmp );	// �ݒ�d�l����EEPROM�ւ̏����݁B
						if (ercd != E_OK) {
							// �G���[�����̋L�q�B
						}

#if ( VA300S == 0 )
						send_touroku_init_Wait_Ack_Retry();		// PC�ցA�o�^�f�[�^�̏������v�����M�B
						if ( ercd == E_OK ){
							req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
						}
#else
						ercd = InitBkAuthData();
						req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
#endif
					}				
				}
			}
			break;
						
		}		 

//20140925Miya cng mainte
		case LCD_SCREEN220:		// �����e�i���X�E���[�h�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// �����e�i���X�E���[�h��ID�ԍ����͉��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN221 );		// ��ʔԍ��@<-�@���̉��
				}					
			}
			break;
		case LCD_SCREEN221:		// �����e�i���X�E���j���[���(���)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_VERSION_REQ ){							// �o�[�W�����{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN222 );		// �u�o�[�W�����\���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN222 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_ERROR_REREKI_REQ ){				// �G���[�����{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN223 );		// �u�G���[����\���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN223 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_NINSYOU_JYOUKYOU_REQ ){			// �F�؏󋵃{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN224 );		// �u�F�؏󋵕\���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN224 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_JIKOKU_HYOUJI_REQ ){					// ���g�p�{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN225 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN225 );		// ��ʔԍ��@<-�@���̉��
					}
				}else if ( msg[ 0 ] == LCD_MAINTE_END ){					// �I���{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );		// �����e�i���X�E���j���[��ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN222:		// �u�o�[�W�����\���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// �u�߂�v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// �����e�i���X�E���j���[���(���)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN221 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN223:		// �u�G���[����\���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// �u�߂�v�������ꂽ
					g_MainteLog.cmr_seq_err_f = 0;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// �����e�i���X�E���j���[���(���)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN221 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN224:		// �u�F�؏󋵕\���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// �u�߂�v�������ꂽ
/*				
					if(dbg_cap_flg == 0){
						dbg_cap_flg = 1;
					}else if(dbg_cap_flg == 1){
						dbg_cap_flg = 2;
					}else if(dbg_cap_flg == 2){
						dbg_cap_flg = 3;
					}else if(dbg_cap_flg == 3){
						dbg_cap_flg = 4;
					}else if(dbg_cap_flg == 4){
						dbg_cap_flg = 1;
					}
*/

#if(AUTHTEST >= 1)	//20160613Miya
					if(g_sv_okcnt > 0){
						ReadTestRegImgArea(0, 0, 0, 0);
						SaveTestRegImgFlArea(0);
						g_sv_okcnt = 0;
					}
#endif

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// �����e�i���X�E���j���[���(���)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN221 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN225:		// �u�����󋵕\���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// �u�߂�v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN221 );	// �����e�i���X�E���j���[���(���)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN221 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN240:		// �����e�i���X�E���[�h�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���[�h��ID�ԍ����͉��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
				}					
			}
			break;
		case LCD_SCREEN241:		// �����e�i���X�E���j���[���(�ݒ�ύX)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_PASS_KAIJYOU_REQ ){					// �p�X���[�h�J���{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN242 );		// �u�p�X���[�h�J���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN242 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_CALLCEN_TEL_REQ ){				// �R�[���Z���^�[TEL�{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN243 );		// �u�R�[���Z���^�[TEL�\���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN243 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_JIKOKU_AWASE_REQ ){				// �������킹�{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN246 );		// �u�������킹�\���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN246 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_ITI_TYOUSEI_REQ ){				// LCD�ʒu�����{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN248 );		// �uLCD�ʒu�����\���v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN248 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_MAINTE_END ){					// �I���{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );		// �����e�i���X�E���j���[��ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN242:		// �u�p�X���[�h�J���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){				// �u�߂�v�������ꂽ
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( ( msg[ 0 ] == LCD_ENTER ) && ( rcv_length >= 5 ) ){	// �u�m��v�������ꂽ
					if( msg[ 1 ] == 1 )	g_PasswordOpen.hide_num = FLG_ON;
					else				g_PasswordOpen.hide_num = FLG_OFF;

					if( msg[ 2 ] == 1 )	g_PasswordOpen.kigou_inp = FLG_ON;
					else				g_PasswordOpen.kigou_inp = FLG_OFF;

					if( msg[ 3 ] == 1 )	g_PasswordOpen.random_key = FLG_ON;
					else				g_PasswordOpen.random_key = FLG_OFF;

					if( msg[ 4 ] == 1 )	g_PasswordOpen2.family_sw = FLG_ON;		//20160112Miya FinKeyS
					else				g_PasswordOpen2.family_sw = FLG_OFF;

					if( msg[ 5 ] == 1 )	g_PasswordOpen.sw = FLG_ON;
					else				g_PasswordOpen.sw = FLG_OFF;

					for(i = 0 ; i < 10 ; i++ ){
						g_key_arry[i] = i;	//�p�X���[�h�J���p�L�[�z�񏉊���
					}
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN249 );	// ���΂炭���܂�����������ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN249 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN243:		// �R�[���Z���^�[�d�b�ԍ��ύX(�s�O�ǔ�)���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_CALLCEN_TEL_REQ ) ) && ( rcv_length >= 1 ) ){								
					g_RegUserInfoData.MainteTelNum[ 0 ] = msg[ 1 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 1 ] = msg[ 2 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 2 ] = msg[ 3 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 3 ] = msg[ 4 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 4 ] = 0x20;
					g_RegUserInfoData.MainteTelNum[ 5 ] = 0x20;
					g_RegUserInfoData.MainteTelNum[ 6 ] = 0x20;
					g_RegUserInfoData.MainteTelNum[ 7 ] = 0x20;
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN244 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN244 );		// ��ʔԍ��@<-�@���̉��
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;
		case LCD_SCREEN244:		// �R�[���Z���^�[�d�b�ԍ��ύX(�s���ǔ�)���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_CALLCEN_TEL_REQ ) ) && ( rcv_length >= 1 ) ){								
					g_RegUserInfoData.MainteTelNum[ 8 ] = msg[ 1 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 9 ] = msg[ 2 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 10 ] = msg[ 3 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 11 ] = msg[ 4 ];		// ���͂��ꂽ�ً}�ԍ�
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN245 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN245 );		// ��ʔԍ��@<-�@���̉��
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;
		case LCD_SCREEN245:		// �R�[���Z���^�[�d�b�ԍ��ύX(�����Ҕԍ�)���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_CALLCEN_TEL_REQ ) ) && ( rcv_length >= 1 ) ){								
					g_RegUserInfoData.MainteTelNum[ 12 ] = msg[ 1 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 13 ] = msg[ 2 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 14 ] = msg[ 3 ];		// ���͂��ꂽ�ً}�ԍ�
					g_RegUserInfoData.MainteTelNum[ 15 ] = msg[ 4 ];		// ���͂��ꂽ�ً}�ԍ�

					for( i = 0 ; i < 16 ; i++ ){
						kinkyuu_tel_no[i] = g_RegUserInfoData.MainteTelNum[i];
					}
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN249 );	// ���΂炭���܂������������
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN249 );		// ��ʔԍ��@<-�@���̉��
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;
		case LCD_SCREEN246:		// ���ݎ������͉��
//20161031Miya Ver2204 LCDADJ ->		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			if ( rcv_length >= 1 ){	
				if ( msg[ 0 ] == LCD_JIKOKU_AWASE_REQ ){
					LcdPosAdj(1);
					SaveBkDataNoClearFl();								
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
//20161031Miya Ver2204 LCDADJ <-		

/*
			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_JIKOKU_AWASE_REQ ) ) && ( rcv_length >= 1 ) ){								
					dat = 10 * ((unsigned short)msg[ 1 ] - 0x30);
					dat = dat + ((unsigned short)msg[ 2 ] - 0x30);
					g_MainteLog.now_hour = dat;
					dat = 10 * ((unsigned short)msg[ 3 ] - 0x30);
					dat = dat + ((unsigned short)msg[ 4 ] - 0x30);
					g_MainteLog.now_min = dat;
					g_MainteLog.now_sec = 0;

					count_1hour = g_MainteLog.now_hour;		// "��"��LCD��ʕ\���p����������X�g�A
					count_1min = g_MainteLog.now_min;		// "��"���@�@��
					count_1sec = g_MainteLog.now_sec;		// "�b"���@�@��

#if ( VA300S == 1 )
					send_sio_init_time();		// VA300S����Box�փV���A���Ŏ����̏����ݒ�R�}���h(10)�𑗐M�B
//					MdCngSubMode( SUB_MD_INIT_TIME );			// �����̏����ݒ�R�}���h�iVA300���j
//					while ( MdGetSubMode() != SUB_MD_IDLE ){	// ACK��M����܂őҋ@�B
//						dly_tsk( 25/MSEC );		
//					}
#endif

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN247 );	// �����e�i���X�E���j���[���(�ݒ�ύX)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN247 );		// ��ʔԍ��@<-�@���̉��
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
*/
			break;

		case LCD_SCREEN247:		// ���Ȑf�f�J�n�������͉��
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){	
				if ( ( ( msg[ 0 ] == LCD_NEXT ) || ( msg[ 0 ] == LCD_JIKOKU_AWASE_REQ ) ) && ( rcv_length >= 1 ) ){								
					dat = 10 * ((unsigned short)msg[ 1 ] - 0x30);
					dat = dat + ((unsigned short)msg[ 2 ] - 0x30);
					g_MainteLog.st_hour = dat;
					dat = 10 * ((unsigned short)msg[ 3 ] - 0x30);
					dat = dat + ((unsigned short)msg[ 4 ] - 0x30);
					g_MainteLog.st_min = dat;
					g_MainteLog.st_sec = 0;

					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN249 );	// ���΂炭���܂������������
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN249 );		// ��ʔԍ��@<-�@���̉��
					}	
				} else if ( msg[ 0 ] == LCD_CANCEL ){	// �u���~�v�{�^���������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
					}	
				}
			}
			break;

		case LCD_SCREEN248:		// �uLCD�ʒu�����\���v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_BACK ){							// �u�߂�v�������ꂽ

#if ( VA300S == 1 )
					//20141120 �����{���R�}���h���s
					send_sio_force_lock_close();
#endif
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;

		case LCD_SCREEN249:		// �u���΂炭���܂����������v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				//�t���b�V���ɓo�^�f�[�^��ۑ�����
				//dly_tsk( 500/MSEC );
				ercd = SaveBkAuthDataFl();
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN241 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN241 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
		case LCD_SCREEN260:		// �����e�i���X�E���[�h�̃u�����N���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 0 ){
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���[�h��ID�ԍ����͉��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
				}					
			}
			break;
		case LCD_SCREEN261:		// �����e�i���X�E���j���[���(�Z�p���[�h)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			
			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_MAINTE_SHOKIKA_REQ ){					// �������{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN262 );		// �u�������v��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN262 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_SPEC_CHG_REQ ){				// �d�l�ؑփ{�^��
					if(g_MainteLvl == 1){	//20160711Miya �f���@
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
						}
						break;
					}
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN263 );		// �u�d�l�ؑցv��ʂցB
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN263 );		// ��ʔԍ��@<-�@���̉��
					}			
				}else if ( msg[ 0 ] == LCD_IMAGE_KAKUNIN_REQ ){				// �摜�m�F�{�^��
					if(g_MainteLvl == 1){	//20160711Miya �f���@
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
						}
						break;
					}
//#if(AUTHTEST >= 1)	//20160613Miya
#if(AUTHTEST >= 2)	//20160902Miya FPGA������ forDebug
					if( g_TechMenuData.DebugHyouji == FLG_OFF ){
						if(g_sv_okcnt0 > 0){
							dbg_Auth_8cnt = 0;
							//g_sv_okcnt = 0;
							ReadTestRegImgArea(0, 1, 1, 0);
						}else{
							ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );		// �����e�i���X�E���j���[��ʂ�
							if ( ercd == E_OK ){
								ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
							}
							break;
						}
					}
#endif
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN264 );		// �u�摜�m�F�\���v��ʂցB
					if ( ercd == E_OK ){
						dbg_Auth_hcnt = 0;	//20141014Miya
						ChgScreenNo( LCD_SCREEN264 );		// ��ʔԍ��@<-�@���̉��
					}
				}else if ( msg[ 0 ] == LCD_NOUSE ){					// ���g�p�{�^��
					if(g_MainteLvl == 1){	//20160711Miya �f���@
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );
						if ( ercd == E_OK ){
							dbg_Auth_hcnt = 0;	//20141014Miya
							ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
						}
						break;
					}
					g_TestCap_start = 0;
#if ( NEWCMR == 1 )	//20160613Miya
					g_cmr_dbgcnt = 0;
#endif
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN266 );
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN266 );		// ��ʔԍ��@<-�@���̉��
					}
				}else if ( msg[ 0 ] == LCD_MAINTE_END ){					// �I���{�^��
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN202 );		// �����e�i���X�E���j���[��ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN202 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN262:		// �����e�i���X�E���j���[���(�Z�p���[�h)
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				if ( msg[ 0 ] == LCD_YES ){				// �u�͂��v�������ꂽ
				
#if ( VA300S == 0 )
					send_touroku_init_Wait_Ack_Retry();		// PC�ցA�o�^�f�[�^�̏������v�����M�B
					if ( ercd == E_OK ){
						req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
					}
#else

					if( GetSysSpec() == SYS_SPEC_KOUJIS || g_TechMenuData.SysSpec == 2 ){
						ercd = lan_set_eep( EEP_SYSTEM_SPEC, SYS_SPEC_SMT );	// �ݒ�d�l����EEPROM�ւ̏����݁B
					}
					ercd = InitBkAuthData();
					send_sio_Touroku_AllDel();		// VA300S����Box�փV���A���œo�^�f�[�^�������i�ꊇ�폜�j�R�}���h(05)�𑗐M�B
					MdCngSubMode( SUB_MD_ALLDEL );		// �ꊇ�폜���M���iVA300���j
					//ercd = InitRegImgArea();
					//FpgaSetWord(FPGA_RECONF, 0x01, 0x01);
					while ( MdGetSubMode() != SUB_MD_IDLE ){	// ACK��M����܂őҋ@�B
						dly_tsk( 25/MSEC );		
					}
					req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
#endif
										
				} else if ( msg[ 0 ] == LCD_NO ){		// �u�������v�������ꂽ
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���j���[�I�����
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
					}					
				}else{
#if(KOUJYOUCHK)
		g_TechMenuData.SysSpec = 2;
		g_PasswordOpen.sw = 1;
		g_PasswordOpen2.family_sw = 1;
#endif
					if(g_TechMenuData.SysSpec == 2){	//20160112Miya FinKeyS
						ercd = lan_set_eep( EEP_SYSTEM_SPEC, SYS_SPEC_KOUJIS );	// �ݒ�d�l����EEPROM�ւ̏����݁B

						ercd = InitBkAuthData();
						send_sio_Touroku_AllDel();		// VA300S����Box�փV���A���œo�^�f�[�^�������i�ꊇ�폜�j�R�}���h(05)�𑗐M�B
						MdCngSubMode( SUB_MD_ALLDEL );		// �ꊇ�폜���M���iVA300���j
						//ercd = InitRegImgArea();
						//FpgaSetWord(FPGA_RECONF, 0x01, 0x01);
						while ( MdGetSubMode() != SUB_MD_IDLE ){	// ACK��M����܂őҋ@�B
							dly_tsk( 25/MSEC );		
						}
						req_restart = 1;			// �p���[�I���v���Z�X�̍ċN���v���t���O�̃Z�b�g�B
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN262 );	// �����e�i���X�E���j���[�I�����
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN262 );		// ��ʔԍ��@<-�@���̉��
						}
					}					
				}
			}
		case LCD_SCREEN263:		// �����e�i���X�E���j���[���(�Z�p���[�h)
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				
				if ( msg[ 0 ] == LCD_BACK ){				// �u�߂�v�������ꂽ
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���j���[���(�Z�p���[�h)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
					}
										
				} else if ( ( msg[ 0 ] == LCD_ENTER ) && ( rcv_length >= 5 ) ){	// �u�m��v�������ꂽ
					//if( msg[ 1 ] == 1 )	g_TechMenuData.SysSpec = FLG_ON;
					//else				g_TechMenuData.SysSpec = FLG_OFF;

					if( msg[ 1 ] == 0 ){
						g_TechMenuData.SysSpec = 0;
					}else if( msg[ 1 ] == 1 ){
						//g_TechMenuData.SysSpec = 1;
						g_TechMenuData.SysSpec = 0;
					}else if( msg[ 1 ] == 2 ){
						g_TechMenuData.SysSpec = 2;
						g_PasswordOpen.sw = 1;
						g_PasswordOpen2.family_sw = 1;
					}

					if( msg[ 2 ] == 1 )	g_TechMenuData.DemoSw = FLG_ON;
					else				g_TechMenuData.DemoSw = FLG_OFF;

					if( msg[ 3 ] == 1 ){
						g_TechMenuData.HijyouRemocon = FLG_ON;
#if ( VA300S == 1 )
						send_sio_WakeUp();					// Test�R�}���h���M�i�����R���ݒ��񑗐M�j
#endif
					} else	{
						g_TechMenuData.HijyouRemocon = FLG_OFF;
#if ( VA300S == 1 )
						send_sio_WakeUp();					// Test�R�}���h���M�i�����R���ݒ��񑗐M�j
#endif
					}

					if( msg[ 4 ] == 1 )	g_TechMenuData.DebugHyouji = FLG_ON;
					else				g_TechMenuData.DebugHyouji = FLG_OFF;
//20161031Miya Ver2204 ->
					if( msg[ 5 ] == 2 ){
						g_BkDataNoClear.LiveCnt = 0;	//20161031Miya Ver2204
					}

					if( msg[ 6 ] == 0 )			{g_BkDataNoClear.LedPosi = 0;}
					else if( msg[ 6 ] == 1 )	{g_BkDataNoClear.LedPosi = 1;}
					else						{g_BkDataNoClear.LedPosi = 0;}
//20161031Miya Ver2204 <-

					for(i = 0 ; i < 10 ; i++ ){
						g_key_arry[i] = i;	//�p�X���[�h�J���p�L�[�z�񏉊���
					}
				
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN265);	// ���΂炭���܂�����������ʂ�
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN265 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
			break;
		case LCD_SCREEN264:		// �����e�i���X�E���j���[���(�Z�p���[�h)
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
#if(AUTHTEST >= 1)	//20160613Miya
			if ( rcv_length >= 1 ){
				if( g_TechMenuData.DebugHyouji == FLG_ON ){
#if ( VA300S == 2 ) 
					DebugSendCmd_210();
#endif
					dbg_Auth_hcnt++;
					if(dbg_Auth_hcnt >= g_RegUserInfoData.TotalNum ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN264 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN264 );		// ��ʔԍ��@<-�@���̉��
						}
					}
				}else{
#if ( VA300S == 2 ) 
					DebugSendCmd_210();
#endif
					dbg_Auth_hcnt++;
					if(	dbg_Auth_hcnt == 8 ){
						dbg_Auth_hcnt = 0;
						++dbg_Auth_8cnt;
						if(dbg_Auth_8cnt < 4){
							ReadTestRegImgArea(0, 1, 1, dbg_Auth_8cnt);
						}else if(dbg_Auth_8cnt < 8){
							ReadTestRegImgArea(0, 1, 2, (dbg_Auth_8cnt - 4));
						}else if(dbg_Auth_8cnt < 12){
							ReadTestRegImgArea(0, 1, 3, (dbg_Auth_8cnt - 8));
						}else{
							ReadTestRegImgArea(0, 1, 4, (dbg_Auth_8cnt - 12));
						}
					}
					
					if(g_sv_okcnt0 <= 8 * dbg_Auth_8cnt + dbg_Auth_hcnt){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN264 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN264 );		// ��ʔԍ��@<-�@���̉��
						}
					}
				}
			}
#else
			if ( rcv_length >= 1 ){
				if( g_TechMenuData.DebugHyouji == FLG_ON ){
#if ( VA300S == 2 ) 
					DebugSendCmd_210();
#endif
					dbg_Auth_hcnt++;
					if(dbg_Auth_hcnt >= g_RegUserInfoData.TotalNum ){
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
						}
					}else{
						ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN264 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
						if ( ercd == E_OK ){
							ChgScreenNo( LCD_SCREEN264 );		// ��ʔԍ��@<-�@���̉��
						}
					}
				}else{
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
					}
				}
			}
#endif
			break;
		 
		case LCD_SCREEN265:		// �u���΂炭���܂����������v���
		
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				//�t���b�V���ɓo�^�f�[�^��ۑ�����
				ercd = SaveBkAuthDataFl();
				ercd = SaveBkDataNoClearFl();	//20161031Miya Ver2204 LCDADJ
				
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
				}
			}
			break;
		case LCD_SCREEN266:		// �����e�i���X�E���j���[���(�Z�p���[�h)
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );
			g_TestCap_start = 1;

			break;
		 
		case LCD_SCREEN267:		// �����e�i���X�E���j���[���(�Z�p���[�h)
			rcv_length = rcv_mbf_Lcd( MBF_LCD_DATA, ( VP )msg );

			if ( rcv_length >= 1 ){
				//20160601Miya forDebug
#if ( VA300S == 2 ) 
				DebugSendCmd_210();
#endif

#if ( NEWCMR == 1 )	//20150613Miya
				if(g_cmr_dbgcnt > 0){
					g_TestCap_start = 0;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN266 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN266 );		// ��ʔԍ��@<-�@���̉��
					}
				}else{
					g_TestCap_start = 0;
					ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
					if ( ercd == E_OK ){
						ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
					}
				}
#else
				g_TestCap_start = 0;
				ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN261 );	// �����e�i���X�E���j���[���(�ݒ�ύX)��
				if ( ercd == E_OK ){
					ChgScreenNo( LCD_SCREEN261 );		// ��ʔԍ��@<-�@���̉��
				}
#endif				
				
			}
			break;
		 
		default:
			break;
	}
}

/*==========================================================================*/
/**
 *	�F�ؑ��쒆���ǂ������`�F�b�N����
 */
/*==========================================================================*/
static UB Chk_shutdown_ok( void )
{
	UB ret;
	
	if ( ( ( GetScreenNo() >= LCD_SCREEN100 ) && ( GetScreenNo() == LCD_SCREEN104 ) )	// �}���V����(��L��)�d�l/�E�F�ؑ��쒆�̏ꍇ
	  || ( ( GetScreenNo() >= LCD_SCREEN170 ) && ( GetScreenNo() <= LCD_SCREEN178 )	)	// �ً}�J����
	  || ( ( GetScreenNo() == LCD_SCREEN500 ) && ( GetScreenNo() <= LCD_SCREEN506 )	)  	// �}���V����(�P�΂P)�d�l�E�F�ؑ��쒆�̏ꍇ
	  || ( MdGetMode() == MD_CAP ) ){
		  
		ret = SHUTDOWN_NO;
		
	}	else	{
		ret = SHUTDOWN_OK;
	}
	
	return ret;
}

/*==========================================================================*/
/**
 *	��d���[�h�A��d���m�ʒm��M�̏ꍇ�́A�V���b�g�_�E�����s
 */
/*==========================================================================*/
static ER Pfail_shutdown( void )
{
	ER ercd;
	
	ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN999 );	// �u�V���b�g�_�E�����܂��v��ʂ̕\����v��
	if ( ercd == E_OK ){
		ChgScreenNo( LCD_SCREEN999 );		// ��ʔԍ��@<-�@���̉��
	}
	
#if ( VA300S == 1 || VA300S == 2 )
	//ercd = SaveBkAuthDataFl();
	//ercd = SaveRegImgFlArea(0);
	//ercd = SaveRegImgFlArea(10);

	LedOut(LED_OK, LED_ON);
	dly_tsk( 5000/MSEC );
	LedOut(LED_ERR, LED_ON);
	send_sio_ShutDown();					// SIO�o�R�ŁAVA-300���փV���b�g�_�E���v���̑��M

#else
	send_shutdown_req_Wait_Ack_Retry(); 	// �V���b�g�_�E���v���̑��M
	
	slp_tsk();
	
#endif	


	return ercd;
}