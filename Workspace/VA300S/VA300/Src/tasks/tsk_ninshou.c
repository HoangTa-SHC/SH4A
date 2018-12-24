/**
*	VA-300s�v���O����
*
*	@file tsk_ninshou.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/12/20
*	@brief  VA-300s �F�؏������C���^�X�N
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
#include "drv_fpga.h"
#include "drv_cmr.h"

#include "va300.h"
#include "command.h"

#if ( VA300S == 1 || VA300S == 2 )	

#define HI_KANWA	0	//�����_�����_�ɘa(FAR�댯������)
#define X_TRIM_SW	0	//X���g���~���O(���x�e������)

#define MAX_SIZE_X	160
#define MAX_SIZE_Y	80
#define REC_SIZE_Y	140
#define AUTH_SIZE_X	80
#define AUTH_SIZE_Y	40
#define MAXLIMIT	10000
#define DEV_NUM		4		//������=4 
#define X_SEARCH	3		//���փT�C�Y(X)
#define Y_SEARCH	5		//���փT�C�Y(Y)
//#define LOW_PART_TH 100
#define LOW_PART_TH 200		//20140423Miya 臒l100��200�ύX(�V�~�����[�^�ɂ��킹��)
#define TRIM_ON		1
#define TRIM_X		120
#define TRIM_Y		60
#define MINAUTH_ON	1
#define MIN_SIZE_X	20
#define MIN_SIZE_Y	10

#define LO_END		0		//20160909Miya �F�؃A�b�v
//#define HI_END		219		//20160909Miya �F�؃A�b�v
#define HI_END		255		//20160909Miya �F�؃A�b�v

#define ERR_NONE		0
#define ERR_CAMERA		1	//�J�����ُ�
#define ERR_MEIRYOU		2	//���Ĉُ�(���邢)
#define ERR_MEIRYOU2	3	//���Ĉُ�(�Â�)
#define ERR_MEIRYOU3	4
#define ERR_SLIT		5	//�X���b�g����

#define XSFT_NUM		5	//20140910Miya XSFT (3 or 5��ݒ�A0�̏ꍇ��OFF)
#define XSFT0			2	//20140910Miya XSFT (XSFT_NUM=3:1 XSFT_NUM=5:2)

#define	AUTH_BASE		0xB8000000				///< �F�؉摜�f�[�^�̃x�[�X�A�h���X(CS6�̈�)
#define	AUTHINP_BASE	0xBC000000				///< �F�؉摜�f�[�^�̃x�[�X�A�h���X(CS7�̈�)

#define IMG_HDRBUF		0x0C000000

#define IMG_CAPBUF1		0x0C000000
#define IMG_CAPBUF2		0x0C001900
#define IMG_REGBUF		0x0C004000

#define NC_SOUKAN_SGM	813		//20160711Miya NewCmr	
#define NC_SOUKAN_AVE	873		//20160711Miya NewCmr
#define NC_LBP_SGM		8275	//20160711Miya NewCmr	
#define NC_LBP_AVE		8622	//20160711Miya NewCmr

static UB g_RegFlg;
static int g_AuthOkCnt;	//�F��OK�񐔁@50��Ŋw�K�摜�ۑ�

static RegUserInfoData			g_RegUserInfoData;				//���[�U���
static RegBloodVesselTagData	g_RegBloodVesselTagData[20];	//�o�^�����摜�^�O���
static UseProcNum 				g_UseProcNum;					//���[�U�[�g�p�ԍ�
static PasswordOpen				g_PasswordOpen;					//�p�X���[�h�J��
static PasswordOpen				g_InPasswordOpen;				//�p�X���[�h�J��
static TechMenuData				g_TechMenuData;					//�Z�p�����e
static AuthLog					g_AuthLog;						//�F�؃��O		//20140925Miya add log
static MainteLog				g_MainteLog;					//�����e���O	//20140925Miya add log
static PasswordOpen2			g_PasswordOpen2;				//20160108Miya FinKeyS //�p�X���[�h�J��

static struct{
	RegUserInfoData			BkRegUserInfoData;				//���[�U���
	RegBloodVesselTagData	BkRegBloodVesselTagData[20];	//�o�^�����摜�^�O���
	PasswordOpen			BkPasswordOpen;					//�p�X���[�h�J��
	TechMenuData			BkTechMenuData;					//�Z�p�����e	//20140925Miya add Mainte
	AuthLog					BkAuthLog;						//�F�؃��O		//20140925Miya add log
	MainteLog				BkMainteLog;					//�����e���O	//20140925Miya add log
	PasswordOpen2			BkPasswordOpen2;				//20160108Miya FinKeyS	//�p�X���[�h�J��
}g_BkAuthData;

static CapImgData				g_CapImgWorkData;			//�摜�L���v�`���[�����p�����[�^������
static ImgAndAuthProcSetData	g_ImgAndAuthProcSetData;	//�摜���F�؏����p�����[�^
static StsImgAndAuthProc		g_StsImgAndAuthProc;		//�摜���F�؏����X�e�[�^�X���

static RegImgDataAdd			g_RegImgDataAdd[20];		//20160312Miya �ɏ����xUP �o�^�摜(R1,LBP,R1�ɏ�)

static short g_ImgAve[3];
static int normalize_array[MAX_SIZE_Y][MAX_SIZE_X];

static UB CapImgBuf[2][2][AUTH_SIZE_X*AUTH_SIZE_Y];		//[�o�^��][�\�[�x��][�F�؃T�C�Y]
static UB RegImgBuf[20][2][2][AUTH_SIZE_X*AUTH_SIZE_Y];	//[�o�^��][�w�K][�\�[�x��][�F�؃T�C�Y]

static UB RegMinImgBuf[20][2][MIN_SIZE_X*MIN_SIZE_Y];	//[�o�^��][�w�K][�ɏ��F�؃T�C�Y]
static double score_buf[2][20];
static double score_buf_cont[2][20];	//20160312Miya �ɏ����xUP
static double score_buf_yubi[2][20];	//20160312Miya �ɏ����xUP
static double score_buf_cont2[2][20];	//20160312Miya �ɏ����xUP
static int auth_turn_num[10];	//20151118Miya �ɏ��F�،�����
static int auth_turn_learn[10];	//20151118Miya �ɏ��F�،�����

volatile static int match_pos[2][DEV_NUM][2];	//[SOBEL][����][x,y]=[2][4][2]
volatile static int match_score[2][DEV_NUM][2*Y_SEARCH+1][2*X_SEARCH+1];	//[SOBEL][����][�T�[�`�͈�y][�T�[�`�͈�x]=[2][4][11][7]
volatile static int match_soukan[2][DEV_NUM];	//[SOBEL][����]=[2][4]
volatile static int m_matrix3[2][DEV_NUM][Y_SEARCH][X_SEARCH];  // 3x3 �F�ؒl

//static	UB indata[TRIM_Y][TRIM_X];
//static	UB outdata[TRIM_Y][TRIM_X];
static	UB indata[60][150];		//20140910Miya XSFT
static	UB outdata[60][150];	//20140910Miya XSFT

static int lbp_level[20][2];			//20140905Miya lbp�ǉ� //[�o�^��][�w�K]
static double lbp_score[20][2][5];		//20140905Miya lbp�ǉ� //[�o�^��][�w�K][����]
static int lbp_hist_data[2][5][256];	//20140905Miya lbp�ǉ� [��r][����][�f�[�^]
static unsigned short g_contrast_sa[2][16];		//20160312Miya �ɏ����xUP 16�����R���g���X�g�����f�[�^ [R1/R2][16����]

static UB g_XsftMiniBuf[5][ 20 * 10 ];	//20140910Miya XSFT

static int GammaLut[256];	//20160810Miya

static UB g_CapMini[ 20 * 10 ];	//20170706Miya FPGA������


//20150531Miya
/*
static struct{
	int 	zure_sum_x;		//�����SUM�l
	double	ttl_dat_x;		//�݌vSUM�l
	int 	zure_sum_y;		//�����SUM�l
	double 	ttl_dat_y;		//�݌vSUM�l
	int		pm_zure_y;		//���ꂪ+�����A-�����ɂ΂���Ă��邩 0:�΂���Ȃ� 1:�΂������
	int		parallel_y;		//���s���ꂪMAX���ǂ��� 0:MAX�łȂ� 1:MAX
	int		auth_num_in;
	int		auth_learn_in;
	int		low_f;
	int		x_scr1;
	int		scr1;
	int		scr2;
	double	gen1;
	double	gen2;
}g_PosGenten[2];
*/
PosGenten g_PosGenten[2];

#if(AUTHTEST >= 1)	//20160613Miya
unsigned short g_sv_okcnt;
unsigned short g_sv_okcnt0;
unsigned short g_sv_okcnt1;
unsigned short g_sv_okcnt2;
unsigned short g_sv_okcnt3;
unsigned short g_sv_okcnt4;
static UB TstAuthImgBuf[8][100*40];	//[OK/NG][8��][�V�t�g�O�T�C�Y]
unsigned short g_cmr_err;
unsigned short g_imgsv_f;
#endif

TASK NinshouTask( void );
void SendNinshouData( char *data, int cnt );	// �f�[�^�̃^�X�N�ԑ��M�i�ėp�E�F�؏�����p�j

void Ninshou_204( UB *data );
void Ninshou_210( UB *data );
static UB Ninshou_ok_proc( void );
static UB Ninshou_ng_proc( void );

static void InitImgAuthPara( void );
static UB InitFlBkAuthArea( void );
static void InitBkAuthDataChar(int num);
static UB InitBkAuthData( void );
static UB SaveBkAuthDataTmpArea( void );
static UB SaveBkAuthDataFl( void );	//�摜����&�F�؏����o�b�N�A�b�v�f�[�^�ۑ�
static UB ReadBkAuthData( void );	//�摜����&�F�؏����o�b�N�A�b�v�f�[�^�Ǎ�

static UB SaveBkDataNoClearFl( void );	//20161031Miya Ver2204 LCDADJ
static UB ReadBkDataNoClearFl( void );	//20161031Miya Ver2204 LCDADJ

static UB SetImgAuthPara( UB *data );
static UB DelImgAuthPara( void );

static UB ImgHantei( UB num );
static UB HiImgHantei( UB hantei );
static UB MidImgHantei( UB hantei );
static UB LowImgHantei( void );
static UB SlitImgHantei( int *st );
//static UB SlitImgHantei( int reg_f, int *st );
static void ImgTriming( int st_y );

static UB DoImgProc( UB num );
static void ProcGamma(double par);
static void HdrGouseiProc(void);
static void ImgResize4(int num);
static void dat_bicubicS(int sizeX,int sizeY,int out_X,int out_Y);
static void TrimXsft(int num);
static void sobel_R1_Filter(int sizeX,int sizeY);
static void sobel_R2_Filter(int sizeX,int sizeY);
static void medianFilter7G(int num, int matX,int matY,int sizeX,int sizeY);
static void local_binary_pattern(int sizeX,int sizeY);	//20140905Miya lbp�ǉ�
static void make_lbp_hist(int szx, int szy, int num);	//20140905Miya lbp�ǉ�

static UB DoAuthProc( UB num );
//static void ChkMaxPara( int *auth_num, int *auth_learn );	//20151118Miya �ɏ��F�،�����
static int ChkMaxPara( int auth_num );	//20151118Miya �ɏ��F�،�����
static int ChkRegNum( UB proc, int num );
static UB ChkScoreLbp( int id_B, int authlow, double sv_score1, double sv_score2, double gentenA, double gentenB, int lbp_lvl );
static UB ChkScoreNewCmr( int id_B, double sv_score1, double sv_score2, double gentenA, double gentenB ); //20160711Miya NewCmr
static UB ChkScore( double score1, double score2 );
static double auto_matching( UB proc, int cap_num, int sv_num, int sbl, int *num );
static double two_matcher7v( int sizex, int sizey, int cap_num, int sv_num, int stx, int edx, int offsetx, int offsety, int sbl, int learn_num);
static double contrast_soukan(int regnum, int learn, int sw);	//20160312Miya �ɏ����xUP
//static double two_matcher7vHi( UB proc, int sizex, int sizey, int cap_num, int sv_num, int stx, int edx, int offsetx, int offsety, int sbl, int learn_num);
//static void genten_seq(int id_B, double *gentenA, double *gentenB);
static void genten_seq(int id_B, int authlow, int lbplvl, double *gentenA, double *gentenB); //20150531Miya
static void genten_seq_NewCmr(int id_B, double *gentenA, double *gentenB);	//20160711Miya NewCmr
static void matrix3_calc();
static void matrix3_calc2(int r,int k);
static int matrix3_low_check(void);
static double x_ofst_check(int r);
static double y_ofst_check(int r);
static float fastsqrt(float val);
static int two_matcher_lbp(int t_num, int g_num);	//20140905Miya lbp�ǉ�
static double lbp_hist_compare(int b_num);			//20140905Miya lbp�ǉ�
static int lbp_score_judge( double dat, double dat1, double dat2, double dat3, double dat4 );	//20140905Miya lbp�ǉ�
static void make_contrast(int sw);	//20160312Miya �ɏ����xUP 16�����R���g���X�g�����쐬

static UB SaveRegImgTmpArea( UB proc );
static UB SaveRegImgFlArea( int num );
static UB ReadRegImgArea( int num );
static UB AddRegImgFromRegImg( int sw, int num );	//20160312Miya �ɏ����xUP
static UB InitRegImgArea( void );

#if(AUTHTEST >= 1)	//20160613Miya
static UB InitTestRegImgFlArea( void );
static UB SaveTestRegImgFlArea( unsigned short ok_ng_f );
static UB ReadTestRegImgArea( unsigned short ok_ng_f, short cpy_f, short num, short num10 );
static UB ReadTestRegImgCnt( void );
#endif

static void MakeOpenKeyNum(void);
static UB ChekKinkyuKaijyouKey( void );
static unsigned short GetKeta( int indat, int keta );
static unsigned int CalcLineSum( int num, int ln, int sobel, int line, int lmt );
static UB ChekPasswordOpenKey( void );	//20140925Miya password_open
static UB CngNameCharaCode( unsigned char code, int *num );	//20160108Miya FinKeyS

static void SetReCap( void );

UB MemCheck( unsigned long offset );
void FpgaTest( void );

/*
extern const unsigned short NiTest_R1_00[];
extern const unsigned short NiTest_R2_00[];

extern const unsigned short NiTest_R1_01[];
extern const unsigned short NiTest_R2_01[];
*/

//20160902Miya FPGA������
static int WrtImgToRam(int ts, int sbl);
static int WrtMiniImgToRam(UB num, int learn, int sbl);
static int Cal4Ave(int ts, int sbl, int *rt1, int *rt2, int *rt3, int *rt4);
void MakeTestImg(void);
static void AutoMachingFPGA(UB num, int proc, double *scr1, double *scr2);

#if(CMRSENS == 1)	//20170424Miya 	�J�������x�Ή�
int RegYubiSideChk(void);
#endif

/*******************************
 * Ninshou Task
 *
 * @param  NON
 *******************************/
TASK NinshouTask( viod )
{
	ER	ercd;
	T_COMMSG *msg;
	UINT i, idcode;
	UB recbuf[1024];

	//InitImgAuthPara();

	for (;;)
	{
		ercd = rcv_mbx(MBX_SND_NINSHOU, &msg);	// �F�ؗp�f�[�^�̎�M�҂�
		if ( ercd == E_OK ){

			dbg_nin_flg = 1;

			// �F�؃��C������
			nop();

			for (i = 0;i < msg->cnt;i++) {
				recbuf[i] = msg->buf[i];
			}
		
			/* Release memory block */
			rel_mpf(MPF_SND_NINSHOU, msg);

			idcode = 100 * ((UINT)recbuf[4] - 0x30);
			idcode += 10 * ((UINT)recbuf[5] - 0x30);
			idcode += (UINT)recbuf[6] - 0x30;
			
			switch(idcode){
				case 204:
					Ninshou_204(recbuf);
					break;
				case 210:
					Ninshou_210(recbuf);
					break;
				default:
					ercd = idcode;
					break;
			}

			dbg_nin_flg = 0;
		
    	} else {							/* �R�[�f�B���O�G���[ */
	    	ErrCodeSet( ercd );
    	}
    }
}


//=============================================================================
/**
 *	�f�[�^�̃^�X�N�ԑ��M�i�ėp�E�F�؏�����p�j
 *	@param	data�@���M�f�[�^
 *	@param	cnt�@���M�o�C�g��
 */
//=============================================================================
void SendNinshouData( char *data, int cnt )
{
	T_COMMSG *msg;
	ER		ercd;
	
	ercd = tget_mpf( MPF_SND_NINSHOU, ( VP* )&msg, ( 10/MSEC ) );
	if( ercd == E_OK ) {
		
		memset( &(msg->buf), 0, sizeof( msg->buf ) );
	
		msg->cnt    = cnt;
		msg->msgpri = 1;
		memcpy( &( msg->buf ), data, cnt );
	
		snd_mbx( MBX_SND_NINSHOU, ( T_MSG* )msg );
	}
	
}


//=============================================================================
/**
 *	�f�[�^�̃^�X�N�ԑ��M�i�ėp�E�F�؏�����p�j
 *	@param	data�@���M�f�[�^
 *	@param	cnt�@���M�o�C�g��
 */
//=============================================================================
void Ninshou_204( UB *data )
{
	UB ercd = 0;
	UB *recbuf;
	UB touroku_num;
	UB rtn1, rtn2, rtn3;
	UB ubtmp;
	unsigned short reg_id;

//20180406Miya ForSH
	ercd = Ninshou_ng_proc();
	g_CapTimes = 0;
	return;
}

//=============================================================================
/**
 *	�f�[�^�̃^�X�N�ԑ��M�i�ėp�E�F�؏�����p�j
 *	@param	data�@���M�f�[�^
 *	@param	cnt�@���M�o�C�g��
 */
//=============================================================================
void Ninshou_210( UB *data )
{
	UB ercd = 0;
	UB *recbuf;
	UB auth_type, touroku_num;
	UB rtn1, rtn2, rtn3;
	UB ubtmp;
	unsigned short reg_id;
	int i, reg_num;
	short cnt;

//20180406Miya ForSH
	ercd = Ninshou_ng_proc();
	g_CapTimes = 0;
	return;
}



/*==========================================================================*/
//	�F��OK����̎�M����
/*==========================================================================*/
static UB Ninshou_ok_proc( void )
{
	UB ercd = 0xff;
		
	// �����o�^���[�h�̏ꍇ
	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN405 ) ){	// �F�؎��u�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TS�^�X�N�։�ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�ʒm�B
		if ( ercd != E_OK ){
			nop();			// �G���[�����̋L�q
		}						
	}
	
	if ( GetScreenNo() == LCD_SCREEN6 ){ 	// �o�^���̏ꍇ�B
	
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN7 );	// 	�o�^�u�w�𔲂���..�v��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN7 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_INITIAL );			// ���u���[�h�������o�^���[�h��
		
	} else if ( GetScreenNo() == LCD_SCREEN8 ){	// �F�؎��u������x�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN9 );	// �o�^������ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN9 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_INITIAL );			// ���u���[�h�������o�^���[�h��
		
#if ( VA300S == 1 || VA300S == 2 )
		send_sio_Touroku();					// �o�^�f�[�^�P�����̏������ւ̑��M�B
		MdCngSubMode( SUB_MD_TOUROKU );		// �T�u���[�h���A�o�^�f�[�^���M���ցB					
#endif		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	} else if ( GetScreenNo() == LCD_SCREEN405 ){	// �o�^���̏ꍇ�B�i�P�΂P�F�؎d�l�j
	
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN406 );	// 	�o�^�u�w�𔲂���..�v��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN406 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_INITIAL );			// ���u���[�h�������o�^���[�h��		

	} else if ( GetScreenNo() == LCD_SCREEN407 ){	// �F�؎��u������x�w���Z�b�g���ĉ�����..�v�i�P�΂P�F�؎d�l�j

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN408 );	// �o�^������ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN408 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_INITIAL );			// ���u���[�h�������o�^���[�h��
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
	}
	
	// �ʏ탂�[�h�̏ꍇ	
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN105 ) ){ // �F�؂̏ꍇ�B
			
		g_AuthCnt = 0;	//20140423Miya del �F�؃��g���C�ǉ�
#if(AUTHTEST >= 1)	//20150320Miya
		if(g_sv_okcnt < 8)	g_sv_okcnt++;
#endif
#if(AUTHTEST == 3)	//20160810Miya
		g_sv_okcnt = 8;
#endif
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN103 );	// �ʏ탂�[�h�i�F�؁j�F�؊�����ʂցB	
		if ( ercd == E_OK ){
			g_AuthType = 0;	//20160120Miya
			ChgScreenNo( LCD_SCREEN103 );			// ��ʔԍ��@<-�@���̉��
#if ( VA300S == 1 )	//20140905Miya
			send_sio_Ninshou(1, 0, 0);	// �F��OK���M(�w�F��)
#endif
			MdCngSubMode( SUB_MD_NINSHOU );			// �F�؊������M���iVA300���j
		}
		if ( Pfail_mode_count == 0 ){
			if(dbg_cap_flg == 1){	//20160711Miya NewCmr
				dbg_cap_flg = 2;
			}

			MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
		}	else	{
			MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h��
		}
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	//20160108Miya FinKeyS	
	}else if ( ( GetScreenNo() == LCD_SCREEN601 ) || ( GetScreenNo() == LCD_SCREEN602 ) || ( GetScreenNo() == LCD_SCREEN605 ) ){ // �F�؂̏ꍇ�B
			
		g_AuthCnt = 0;	//20140423Miya del �F�؃��g���C�ǉ�
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN603 );	// �ʏ탂�[�h�i�F�؁j�F�؊�����ʂցB	
		if ( ercd == E_OK ){
			g_AuthType = 0;	//20160120Miya
			ChgScreenNo( LCD_SCREEN603 );			// ��ʔԍ��@<-�@���̉��
#if ( VA300S == 1 )	//20140905Miya				
			send_sio_Ninshou(1, 0, ode_oru_sw);	// �F��OK���M(�w�F��)
#endif
			MdCngSubMode( SUB_MD_NINSHOU );			// �F�؊������M���iVA300���j
		}
		if ( Pfail_mode_count == 0 ){ 
			if(dbg_cap_flg == 1){	//20160711Miya NewCmr
				dbg_cap_flg = 2;
			}

			MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
		}	else	{
			MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h��
		}
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	//20160108Miya FinKeyS	
	}else if ( ( GetScreenNo() == LCD_SCREEN610 ) || ( GetScreenNo() == LCD_SCREEN611 ) ){ // �F�؂̏ꍇ�B
			
		g_AuthCnt = 0;	//20140423Miya del �F�؃��g���C�ǉ�
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN603 );	// �ʏ탂�[�h�i�F�؁j�F�؊�����ʂցB	
		if ( ercd == E_OK ){
			g_AuthType = 0;	//20160120Miya
			ChgScreenNo( LCD_SCREEN603 );			// ��ʔԍ��@<-�@���̉��
#if ( VA300S == 1 )	//20140905Miya
			send_sio_Ninshou(1, 0, ode_oru_sw);					// �F��OK���M(�V���A��)
#endif
			MdCngSubMode( SUB_MD_NINSHOU );			// �F�؊������M���iVA300���j
		}
		if ( Pfail_mode_count == 0 ){ 
			MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
		}	else	{
			MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h��
		}
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
	} else if ( GetScreenNo() == LCD_SCREEN503 ){ // �F�؂̏ꍇ�B�i�P�΂P�F�؎d�l�j
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN504 );	// �ʏ탂�[�h�i�F�؁j�F�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN504 );			// ��ʔԍ��@<-�@���̉��
		}
		if ( Pfail_mode_count == 0 ){ 
			MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
		}	else	{
			MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h��
		}
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	} else if ( GetScreenNo() == LCD_SCREEN121 ){ // �ʏ탂�[�h�i�o�^�E�ӔC�҂̊m�F�j�̏ꍇ�B
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN122 );	// �o�^���̐ӔC�ҔF�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN122 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	} else if ( GetScreenNo() == LCD_SCREEN523 ){ // �ʏ탂�[�h�i�o�^�E�ӔC�҂̊m�F�j�̏ꍇ�B�i�P�΂P�F�؎d�l�j
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN524 );	// �o�^���̐ӔC�ҔF�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN524 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	}
	
	if ( ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN530 ) ){	// �ʏ탂�[�h�i�o�^�j�u�w���Z�b�g����...�v���
			
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TS�^�X�N�։�ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�ʒm�B
		if ( ercd != E_OK ){
			nop();			// �G���[�����̋L�q
		}			
	}
	
	if ( GetScreenNo() == LCD_SCREEN127 ){	// �ʏ탂�[�h�i�o�^�j�̏ꍇ�B
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN128 );	// �u�w�𔲂��ĉ������v��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN128 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��			

	} else if ( GetScreenNo() == LCD_SCREEN129 ){	// �ʏ탂�[�h�i�o�^�j�u������x�w���Z�b�g����...�v���
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN130 );	// �F�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN130 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
#if ( VA300S == 1 || VA300S == 2 )
		send_sio_Touroku();					// �o�^�f�[�^�P�����̏������ւ̑��M�B
		MdCngSubMode( SUB_MD_TOUROKU );		// �T�u���[�h���A�o�^�f�[�^���M���ցB					
#endif				
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	} else if ( GetScreenNo() == LCD_SCREEN530 ){	// �ʏ탂�[�h�i�o�^�j�̏ꍇ�B�i�P�΂P�F�؎d�l�j
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN531 );	// �u�w�𔲂��ĉ������v��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN531 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��			

	} else if ( GetScreenNo() == LCD_SCREEN532 ){	// �ʏ탂�[�h�i�o�^�j�u������x�w���Z�b�g����...�v��ʁi�P�΂P�F�؎d�l�j
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN533 );	// �F�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN533 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	} else if ( GetScreenNo() == LCD_SCREEN141 ){	// �폜���̐ӔC�ҔF�؊�����ʂցB
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN142 );	// �F�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN142 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	} else if ( GetScreenNo() == LCD_SCREEN544 ){	// �폜���̐ӔC�ҔF�؊�����ʂցB�i�P�΂P�F�؎d�l�j
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN545 );	// �F�؊�����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN545 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	} else if ( GetScreenNo() == LCD_SCREEN161 ){	// �ً}�J���ԍ��o�^���̐ӔC�ҔF�؊�����ʂցB
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN162 );	// �F�؊�������ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN162 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	} else if ( GetScreenNo() == LCD_SCREEN181 ){	// �p�X���[�h�J���ݒ莞�̐ӔC�ҔF�؊�����ʂցB
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN182 );	// �F�؊�������ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN182 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
	}
	
	// �����e�i���X�E���[�h�̏ꍇ
	if ( GetScreenNo() == LCD_SCREEN203 ){	// �����e�i���X�E�t���摜���M���u�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN204 );	// 	�B�e����ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN204 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_MAINTE );			// ���u���[�h�������e�i���X���[�h��
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
	}
	
	return ercd;

}

/*==========================================================================*/
//	�F��NG����̎�M����
/*==========================================================================*/
static UB Ninshou_ng_proc( void )
{
	UB ercd = 0xff;
							
	// �����o�^���[�h�̏ꍇ
	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN405 ) ){
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TS�^�X�N�։�ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�ʒm�B
		if ( ercd != E_OK ){
			nop();			// �G���[�����̋L�q
		}
	}

	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN8 ) ){	// �������[�h�i�o�^�j�u�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN10 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN10 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_INITIAL );			// ���u���[�h�������o�^���[�h��

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	} else 	if ( ( GetScreenNo() == LCD_SCREEN405 ) || ( GetScreenNo() == LCD_SCREEN407 ) ){	// �������[�h�i�o�^�j(�P�΂P�F�؎d�l)�u�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN409 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN409 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_INITIAL );			// ���u���[�h�������o�^���[�h��

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}
	
	// �ʏ탂�[�h�̏ꍇ	
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN105 ) ){

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			

		//20140423Miya del �F�؃��g���C�ǉ� ->
		//ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN104 );	// �F�؎��s��ʂցB	
		//if ( ercd == E_OK ){
		//	ChgScreenNo( LCD_SCREEN104 );			// ��ʔԍ��@<-�@���̉��
		//}
		//20140423Miya del �F�؃��g���C�ǉ� <-

		//20140423Miya �F�؃��g���C�ǉ� ->
		if( g_AuthCnt >= 1 && g_AuthCnt <= 2){
		//if(0){
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN105 );	// �F�؎��s��ʂցB	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN105 );			// ��ʔԍ��@<-�@���̉��
				g_AuthCnt++;
			}
		}else{
#if(AUTHTEST >= 1)	//20160715Miya
			if(g_sv_okcnt < 8)	g_sv_okcnt++;
#endif

			g_AuthCnt = 0;	//20140423Miya del �F�؃��g���C�ǉ�
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN104 );	// �F�؎��s��ʂցB	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN104 );			// ��ʔԍ��@<-�@���̉��
			}
		}
		//20140423Miya �F�؃��g���C�ǉ� <-

		if ( Pfail_mode_count == 0 ){ 
			if(dbg_cap_flg == 1){	//20160711Miya NewCmr
				dbg_cap_flg = 2;
			}
			MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
		}	else	{
			MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h��
		}

		//reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			
			
	//20160108miya FinKeyS
	}else if ( ( GetScreenNo() == LCD_SCREEN601 ) || ( GetScreenNo() == LCD_SCREEN602 ) || ( GetScreenNo() == LCD_SCREEN605 ) ){

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			

		if( g_AuthCnt >= 1 && g_AuthCnt <= 2){
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN605 );	// �F�؎��s��ʂցB	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN605 );			// ��ʔԍ��@<-�@���̉��
				g_AuthCnt++;
			}
		}else{
			g_AuthCnt = 0;	//20140423Miya del �F�؃��g���C�ǉ�
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN604 );	// �F�؎��s��ʂցB	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN604 );			// ��ʔԍ��@<-�@���̉��
			}
		}

		if ( Pfail_mode_count == 0 ){ 
			if(dbg_cap_flg == 1){	//20160711Miya NewCmr
				dbg_cap_flg = 2;
			}

			MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
		}	else	{
			MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h��
		}

		//reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			
			
	//20160108miya FinKeyS
	}else if ( ( GetScreenNo() == LCD_SCREEN610 ) || ( GetScreenNo() == LCD_SCREEN611 ) ){

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			

		if( g_AuthCnt >= 1 && g_AuthCnt <= 2){
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN605 );	// �F�؎��s��ʂցB	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN605 );			// ��ʔԍ��@<-�@���̉��
				g_AuthCnt++;
			}
		}else{
			g_AuthCnt = 0;	//20140423Miya del �F�؃��g���C�ǉ�
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN604 );	// �F�؎��s��ʂցB	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN604 );			// ��ʔԍ��@<-�@���̉��
			}
		}

		if ( Pfail_mode_count == 0 ){ 
			MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
		}	else	{
			MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h��
		}

		//reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			
	} else if ( GetScreenNo() == LCD_SCREEN503 ){ // �ʏ탂�[�h�i�F�؁j(�P�΂P�F�؎d�l)�̏ꍇ�B
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN505 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN505 );			// ��ʔԍ��@<-�@���̉��
		}
		if ( Pfail_mode_count == 0 ){ 
			MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��
		}	else	{
			MdCngMode( MD_PFAIL );			// ���u���[�h��	��d���[�h��
		}

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN121 ){	// �ʏ탂�[�h�i�o�^�E�ӔC�Ҋm�F�j�̏ꍇ�B
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN123 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN123 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	} else 	if ( GetScreenNo() == LCD_SCREEN523 ){	// �ʏ탂�[�h�i�o�^�E�ӔC�Ҋm�F�j�i�P�΂P�F�؎d�l�j�̏ꍇ�B
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN525 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN525 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}

	if ( ( GetScreenNo() == LCD_SCREEN127) || ( GetScreenNo() == LCD_SCREEN530 ) ){	

		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TS�^�X�N�։�ʂ��u�w�𔲂��ĉ������v�܂��́A���s��ʂɂȂ�̂�ʒm�B
		if ( ercd != E_OK ){
			nop();			// �G���[�����̋L�q
		}
	}
				
	if ( ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN129 ) ){	// �ʏ탂�[�h�i�o�^�j�̏ꍇ�B
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN131 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN131 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			
			
	} else 	if ( ( GetScreenNo() == LCD_SCREEN530 ) || ( GetScreenNo() == LCD_SCREEN532 ) ){	// �ʏ탂�[�h�i�o�^�j�i�P�΂P�F�؎d�l�j�̏ꍇ�B
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN534 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN534 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	

		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j			
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN141 ){	// �ʏ탂�[�h�i�폜�j�̏ꍇ�B
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN143 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN143 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	} else 	if ( GetScreenNo() == LCD_SCREEN544 ){	// �ʏ탂�[�h�i�폜�j�i�P�΂P�F�؎d�l�j�̏ꍇ�B
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN546 );	// �F�؎��s��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN546 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
			
	}
		
		
	if ( GetScreenNo() == LCD_SCREEN161 ){	// �ً}�J���ԍ��o�^���̐ӔC�ҔF�؎��s��ʂցB
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN163 );	// �F�؎��s�~��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN163 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}

	if ( GetScreenNo() == LCD_SCREEN181 ){	// �p�X���[�h�J���ݒ莞�̐ӔC�ҔF�؎��s��ʂցB
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN183 );	// �F�؎��s�~��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN183 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_NORMAL );			// ���u���[�h��ʏ탂�[�h��	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j

	}

	// �����e�i���X�E���[�h�̏ꍇ
	if ( GetScreenNo() == LCD_SCREEN203 ){	// �����e�i���X�E�t���摜���M�u�w���Z�b�g���ĉ�����..�v

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN205 );	// 	�B�e�~��ʂցB	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN205 );			// ��ʔԍ��@<-�@���̉��
		}
		MdCngMode( MD_MAINTE );			// ���u���[�h�������e�i���X�E���[�h��
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LED�̃p�����[�^���A�����v���l�ɖ߂��B�iCAP��Retry���ɕς�����\��������ׁB�j
	}
	
	return ercd;
}



/*==========================================================================*/
//	�摜����&�F�؏����p�����[�^�ݒ�(�����l)
/*==========================================================================*/
static void InitImgAuthPara( void )
{
	int i;
	
	/*�摜�L���v�`���[�����p�����[�^*/
	g_CapImgWorkData.PictureSizeX 	= 1280;		//�J�����B�e�T�C�Y�iX)
	g_CapImgWorkData.PictureSizeY 	= 720;		//�J�����B�e�T�C�Y�iY)
	g_CapImgWorkData.TrimStPosX 	= 200;		//�g���~���O�J�n���W�iX)
	g_CapImgWorkData.TrimStPosY 	= 140;		//�g���~���O�J�n���W�iY)
	g_CapImgWorkData.TrimSizeX 		= 640;		//�g���~���O�摜�T�C�Y�iX)
	g_CapImgWorkData.TrimSizeY 		= 560;		//�g���~���O�摜�T�C�Y�iY)
	g_CapImgWorkData.ResizeMode 	= 2;		//�摜���k���@0�F��1/2�@1�F��1/4�@2�F��1/8�@3�F��1/1�i���{�j
	g_CapImgWorkData.CapSizeX 		= 160;		//�L���v�`���[�T�C�Y�iX)
	g_CapImgWorkData.CapSizeY 		= 140;		//�L���v�`���[�T�C�Y�iY)
	g_CapImgWorkData.DataLoadFlg 	= 1;		//�p�����[�^�̃��[�h�� 0�F�������@1�F�����ς݁@2�FINI����

	/*�摜���F�؏����p�����[�^*/
	g_ImgAndAuthProcSetData.Proc  					= 0;	//�o�^�󋵊m�F�p�t���O�@0�F���o�^�@1�F�o�^�ς݁@2?�F�w�K�摜����@0xFF�F�폜
	g_ImgAndAuthProcSetData.InpSizeX  				= 160;	//���̓T�C�Y�iX)
	g_ImgAndAuthProcSetData.InpSizeY  				= 80;	//���̓T�C�Y�iY)
	//g_ImgAndAuthProcSetData.IrLedPos[8]  			= ;		//�摜�����̈��ݒ� ���̓T�C�Y�iX)�
															//[0]�FLED1-LED2�ԁ@[1]�FLED2-LED3�ԁ@[2]�FLED3-LED4��
#if ( NEWALGO > 0 )
	g_ImgAndAuthProcSetData.AuthLvlPara 			= 250;	//20140423miya ���_�ɘa //�F�؃A���S��臒l
#else
	//g_ImgAndAuthProcSetData.AuthLvlPara 			= 350;	//�F�؃A���S��臒l
	g_ImgAndAuthProcSetData.AuthLvlPara 			= 320;	//20150531Miya �ʒu���_�ɘa(NEWALGO=1) //�F�؃A���S��臒l
#endif

#if ( NEWCMR == 1 )
	//g_ImgAndAuthProcSetData.AuthLvlPara 			= 300;	//20160711Miya NewCmr
	g_ImgAndAuthProcSetData.AuthLvlPara 			= 280;	//20160810Miya NewCmr
#endif

	g_ImgAndAuthProcSetData.LearningPara 			= 1;	//�w�K�@�\�@�O�FOFF�@�P�F�ȈՊw�K�@�Q�F�ʏ�w�K
	g_ImgAndAuthProcSetData.LvlCameraErrHi 			= 239;	//�q�X�g�O�����P�x���ϒl�̔���(�J�����G���[���)
	g_ImgAndAuthProcSetData.LvlCameraErrLo 			= 16;	//�q�X�g�O�����P�x���ϒl�̔���(�J�����G���[����)
	g_ImgAndAuthProcSetData.LvlHistAveHi 			= 159;	//�q�X�g�O�����P�x���ϒl�̔���(�����x�摜�������l)
	g_ImgAndAuthProcSetData.LvlHistAveLo 			= 50;	//�q�X�g�O�����P�x���ϒl�̔���(�ኴ�x�摜�������l)
	g_ImgAndAuthProcSetData.LvlMeiryouLoAll 		= 50;	//�א������ēx�̉����i�S�́j
	g_ImgAndAuthProcSetData.LvlMeiryouLoPart 		= 15;	//�א������ēx�̉����i�����j
	g_ImgAndAuthProcSetData.LvlMeiryouHiAll 		= 550;	//�א������ēx�̏���i�S�́j
	g_ImgAndAuthProcSetData.LvlMeiryouHiPart 		= 120;	//�א������ēx�̏���i�����j
	g_ImgAndAuthProcSetData.ThLowPart1 				= 250;	//�F�؃X�R�A�[���_臒l1(250�_�ȉ�)
	g_ImgAndAuthProcSetData.ThLowPart2 				= 200;	//�F�؃X�R�A�[���_臒l2(200�_�ȉ�)
	g_ImgAndAuthProcSetData.WeightLowPart1 			= 50;	//�F�؃X�R�A�[���_�W��1(1/100�l���g�p����)
	g_ImgAndAuthProcSetData.WeightLowPart2 			= 0;	//�F�؃X�R�A�[���_�W��1(1/100�l���g�p����)
	g_ImgAndAuthProcSetData.LvlBrightLo 			= 100;	//���邳����i�Ã��x���j臒l
	g_ImgAndAuthProcSetData.LvlBrightLo2 			= 70;	//���邳����i�Ã��x���j��f��
	g_ImgAndAuthProcSetData.LvlBrightHi 			= 200;	//���邳����i�����x���j臒l
	g_ImgAndAuthProcSetData.LvlBrightHiNum 			= 1000;	//���邳����i�����x���j��f��
	g_ImgAndAuthProcSetData.LvlFingerTop 			= 20;	//�w��R���g���X�g����臒l(20)
	g_ImgAndAuthProcSetData.LvlSlitCheck 			= 20;	//�X���b�g����[�����x��(20)
	g_ImgAndAuthProcSetData.NumSlitCheck 			= 20;	//�X���b�g����[����f��(20)
	g_ImgAndAuthProcSetData.LvlSlitSensCheck 		= 60;	//�X���b�g���萶�̃Z���T�[���m���x��(60)
	g_ImgAndAuthProcSetData.StSlitSensCheck 		= 30;	//�X���b�g���萶�̃Z���T�[���m�J�n��f(30)
	g_ImgAndAuthProcSetData.EdSlitSensCheck 		= 60;	//�X���b�g���萶�̃Z���T�[���m�I����f(60)
	g_ImgAndAuthProcSetData.NgImageChkFlg 			= 3;	//�摜�ُ픻��(0:�Ȃ� 1:���� 2:�������ُ�̂� 3:�w�����̂�)
	g_ImgAndAuthProcSetData.WeightXzure 			= 20;	//X�������ꌸ�_�d�݌W��(20->0.2)
	g_ImgAndAuthProcSetData.WeightYzure 			= 30;	//Y�������ꌸ�_�d�݌W��(30->0.3)
	g_ImgAndAuthProcSetData.WeightYmuki 			= 20;	//Y�������ꌸ�_�����W��(20->0.2)
	g_ImgAndAuthProcSetData.WeightYhei 				= 20;	//Y�������ꕽ�s����W��(20->0.2)
	g_ImgAndAuthProcSetData.WeightScore400 			= 90;	//�F�؃X�R�A�[400�ȉ����_�d�݌W��(90->0.9)
	g_ImgAndAuthProcSetData.WeightScore300 			= 90;	//�F�؃X�R�A�[300�ȉ����_�d�݌W��(90->0.9)
	g_ImgAndAuthProcSetData.WeightScore200 			= 75;	//�F�؃X�R�A�[200�ȉ����_�d�݌W��(75->0.75)
	g_ImgAndAuthProcSetData.WeightScore200a 		= 25;	//�F�؃X�R�A�[200�ȉ����_�W��(25->0.0025)
	g_ImgAndAuthProcSetData.WeightScore200b 		= 25;	//�F�؃X�R�A�[200�ȉ����_�ؕ�(25->0.25)
	g_ImgAndAuthProcSetData.WeightXzureSHvy 		= 8;	//�X�[�p�[�w�r�[��X�������ꌸ�_�d�݌W��(8->0.08)
	g_ImgAndAuthProcSetData.WeightYzureSHvy 		= 8;	//�X�[�p�[�w�r�[��Y�������ꌸ�_�d�݌W��(8->0.08)
	g_ImgAndAuthProcSetData.WeightYmukiSHvy 		= 0;	//�X�[�p�[�w�r�[��Y�������ꌸ�_�����W��(100->1)
	g_ImgAndAuthProcSetData.WeightYheiSHvy 			= 20;	//�X�[�p�[�w�r�[��Y�������ꕽ�s����W��(20->0.2)
	g_ImgAndAuthProcSetData.WeightScore400SHvy 		= 90;	//�X�[�p�[�w�r�[���F�؃X�R�A�[400�ȉ����_�d�݌W��(90->0.9)
	g_ImgAndAuthProcSetData.WeightScore300SHvy 		= 90;	//�X�[�p�[�w�r�[���F�؃X�R�A�[300�ȉ����_�d�݌W��(90->0.9)
	g_ImgAndAuthProcSetData.WeightScore200SHvy 		= 75;	//�X�[�p�[�w�r�[���F�؃X�R�A�[200�ȉ����_�d�݌W��(75->0.75)
	g_ImgAndAuthProcSetData.WeightScore200aSHvy 	= 25;	//�X�[�p�[�w�r�[���F�؃X�R�A�[200�ȉ����_�W��(25->0.0025)
	g_ImgAndAuthProcSetData.WeightScore200bSHvy 	= 25;	//�X�[�p�[�w�r�[���F�؃X�R�A�[200�ȉ����_�ؕ�(25->0.25)

	/*�摜���F�؏����X�e�[�^�X���*/
	g_StsImgAndAuthProc.AuthResult 		= 0;		//�摜�������F�؏����̌��� 0�F����@1�FRetry�@2?�F�ُ�
	g_StsImgAndAuthProc.RetruCnt 		= 0;		//�摜�������F�؏����̃��g���C��
	g_StsImgAndAuthProc.CmrGain 		= 0;		//�J�����Q�C���̐ݒ� 0�F�Ȃ��@1�F+�␳�@-1�F-�␳
	g_StsImgAndAuthProc.L_ImgSSvalue 	= 0;		//�V���b�^�[���x�̐ݒ� 0�F�Ȃ��@1�F+�␳�@-1�F-�␳
	g_StsImgAndAuthProc.N_ImgSSvalue 	= 0;		//�V���b�^�[���x�̐ݒ� 0�F�Ȃ��@1�F+�␳�@-1�F-�␳
	g_StsImgAndAuthProc.H_ImgSSvalue 	= 0;		//�V���b�^�[���x�̐ݒ� 0�F�Ȃ��@1�F+�␳�@-1�F-�␳
	//g_StsImgAndAuthProc.IrLedSw[8] 		= 0;	//LED�_���ݒ� 0�F�ύX�����@1�F�_���@-1�F����
													//[0]�FLED1�@[1]�FLED2�@[2]�FLED3�@[3]�FLED4"
	g_StsImgAndAuthProc.StsErr 			= 0;		//�G���[�󋵔c�� 0�F���� 1?�F�ُ�

	g_AuthCnt = 0;	//20140423Miya �F�؃��g���C��

	ProcGamma(1.8);	//20160810Miya

/*
	if(g_DipSwCode != 0){
		if( g_DipSwCode == 0x30 ){
			g_ImgAndAuthProcSetData.AuthLvlPara = 300;	//�F�؃A���S��臒l
		}
	}
*/

}


/*==========================================================================*/
//	�摜����&�F�؏����o�b�N�A�b�v�f�[�^������
/*==========================================================================*/
static UB InitFlBkAuthArea( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;

	uwaddr = ADDR_REGAUTHDATA;

	//�t���b�V���ۑ��Z�N�V����������	
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	return(ercd);
}

//20160108Miya FinKeyS
/*==========================================================================*/
//	�摜����&�F�؏����o�b�N�A�b�v�f�[�^������(�L�����N�^�[�R�[�h���X�y�[�X��)
//  0:�I�[�� 1:���O 2:�p�X���[�h 3:�p�X���[�h20��
/*==========================================================================*/
static void InitBkAuthDataChar(int num)
{
	int i, j;
	
	if(num == 0 || num == 1){
		for(i = 0 ; i < 20 ; i++){
			for(j = 0 ; j < 24 ; j++){
				g_BkAuthData.BkRegBloodVesselTagData[i].Name[j] = 0x20;
			}
		}
	}
/*	
	if(num == 0 || num == 2){
		for(j = 0 ; j < 10 ; j++){
			g_BkAuthData.BkPasswordOpen.password[j] = 0x20;
		}
	}
	
	if(num == 0 || num == 3){
		for(i = 0 ; i < 20 ; i++){
			for(j = 0 ; j < 10 ; j++){
				g_BkAuthData.BkPasswordOpen2.password[i][j] = 0x20;
			}
		}
	}
*/
}

/*==========================================================================*/
//	�摜����&�F�؏����o�b�N�A�b�v�f�[�^������
/*==========================================================================*/
static UB InitBkAuthData( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;

	uwaddr = ADDR_REGAUTHDATA;
	//uwSizeIn = sizeof(RegUserInfoData) + sizeof(RegBloodVesselTagData) * 20 + sizeof(PasswordOpen) + sizeof(TechMenuData) + sizeof(AuthLog) + sizeof(MainteLog);	//20140925Miya password_open
	uwSizeIn = sizeof(RegUserInfoData) + sizeof(RegBloodVesselTagData) * 20 + sizeof(PasswordOpen) + sizeof(TechMenuData) + sizeof(AuthLog) + sizeof(MainteLog) + sizeof(PasswordOpen2);	//20160108Miya FinKeyS

	//�t���b�V���ۑ��Z�N�V����������	
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	memset( &Flbuf[0], 0, 0x10000 );
	memcpy( &g_BkAuthData.BkRegUserInfoData.RegSts, &Flbuf[0], uwSizeIn );
	InitBkAuthDataChar(0);	//20160108Miya FinKeyS

	memcpy( &g_RegUserInfoData.RegSts, &g_BkAuthData.BkRegUserInfoData.RegSts, sizeof(RegUserInfoData) );
	memcpy( &g_RegBloodVesselTagData[0].CapNum, &g_BkAuthData.BkRegBloodVesselTagData[0].CapNum, sizeof(RegBloodVesselTagData)*20 );
	memcpy( &g_PasswordOpen.sw, &g_BkAuthData.BkPasswordOpen.sw, sizeof(PasswordOpen) );	//20140925Miya password_open
	memcpy( &g_TechMenuData.SysSpec, &g_BkAuthData.BkTechMenuData.SysSpec, sizeof(TechMenuData) );	//20140925Miya password_open
	memcpy( &g_AuthLog.ok_cnt, &g_BkAuthData.BkAuthLog.ok_cnt, sizeof(AuthLog) );	//20140925Miya add log
	memcpy( &g_MainteLog.err_wcnt, &g_BkAuthData.BkMainteLog.err_wcnt, sizeof(MainteLog) );	//20140925Miya add log
	memcpy( &g_PasswordOpen2.family_sw, &g_BkAuthData.BkPasswordOpen2.family_sw, sizeof(PasswordOpen2) );	//20160108Miya FinKeyS

	//�t���b�V���ۑ��Z�N�V������������	
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	dbg_flwsize = uwSize;
	dbg_flwsizeIn = uwSizeIn;
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

	return(ercd);
}


/*==========================================================================*/
//	�摜����&�F�؏����o�b�N�A�b�v�f�[�^�ۑ�(�e���|�����[)
/*==========================================================================*/
static UB SaveBkAuthDataTmpArea( void )
{
	UB	ercd=E_OK;
	volatile UW	uwaddr, uwSize, uwSizeIn;
	unsigned short reg_num;
	int i;

	reg_num = g_RegUserInfoData.RegNum;
	//�o�^�󋵊m�F�@0�F���o�^�@1�F�o�^�ς�
	g_RegUserInfoData.RegSts = 1;
	//���[�U�w�o�^�{��
	if( g_RegBloodVesselTagData[reg_num].RegInfoFlg != 1 ){	//�㏑���Ή�(�o�^�ς݂̏ꍇ�A���v���̓A�b�v���Ȃ�)
		g_RegUserInfoData.TotalNum += 1;
	}
	
	//�o�^�󋵁@0�F���o�^�@1�F�o�^�ς݁@2?�F�w�K�摜����@0xFF�F�폜
	g_RegBloodVesselTagData[reg_num].RegInfoFlg = 1;
	//�o�^���̎B�e���x 0:���xup(���߰��ް) 1:���xup(��ް) 2:�W�� 3:���xdown(�����E�q��)
	g_RegBloodVesselTagData[reg_num].RegImageSens = g_RegUserInfoData.RegImageSens;
	//�o�^��4�����̔Z�x�lAVE
	g_RegBloodVesselTagData[reg_num].RegHiDensityDev[0] = g_RegUserInfoData.RegHiDensityDev[0];
	g_RegBloodVesselTagData[reg_num].RegHiDensityDev[1] = g_RegUserInfoData.RegHiDensityDev[1];
	g_RegBloodVesselTagData[reg_num].RegHiDensityDev[2] = g_RegUserInfoData.RegHiDensityDev[2];
	g_RegBloodVesselTagData[reg_num].RegHiDensityDev[3] = g_RegUserInfoData.RegHiDensityDev[3];
	//H/M/L�̔Z�x�l�@0:H 1:M 2:L
	g_RegBloodVesselTagData[reg_num].RegDensityAve[0] = g_RegUserInfoData.RegDensityAve[0];
	g_RegBloodVesselTagData[reg_num].RegDensityAve[1] = g_RegUserInfoData.RegDensityAve[1];
	g_RegBloodVesselTagData[reg_num].RegDensityAve[2] = g_RegUserInfoData.RegDensityAve[2];
	//�o�^���w����(���Ή�) 0:���߰��ް 1:��ް 2:�W�� 3:�ׂ� 4:�ɍ�
	g_RegBloodVesselTagData[reg_num].RegFingerSize = g_RegUserInfoData.RegFingerSize;

	//�ɏ��摜
	//if(MINAUTH_ON == 1){
		for( i = 0 ; i < MIN_SIZE_X * MIN_SIZE_Y ; i++ ){
			g_RegBloodVesselTagData[reg_num].MinAuthImg[0][i] = g_ubSobelR3Buf[i];
			g_RegBloodVesselTagData[reg_num].MinAuthImg[1][i] = g_ubSobelR3SvBuf[i];
		}
	//}

	uwSize = sizeof(RegUserInfoData);
	memcpy( &g_BkAuthData.BkRegUserInfoData.RegSts, &g_RegUserInfoData.RegSts, uwSize );

	uwSize = sizeof(RegBloodVesselTagData)*20;
	memcpy( &g_BkAuthData.BkRegBloodVesselTagData[0].CapNum, &g_RegBloodVesselTagData[0].CapNum, uwSize );

	//20140925Miya password_open
	uwSize = sizeof(PasswordOpen);
	memcpy( &g_BkAuthData.BkPasswordOpen.sw, &g_PasswordOpen.sw, uwSize );

	//20140925Miya add Mainte
	uwSize = sizeof(TechMenuData);
	memcpy( &g_BkAuthData.BkTechMenuData.SysSpec, &g_TechMenuData.SysSpec, uwSize );

	//20140925Miya add log
	uwSize = sizeof(AuthLog);
	memcpy( &g_BkAuthData.BkAuthLog.ok_cnt, &g_AuthLog.ok_cnt, uwSize );

	//20140925Miya add log
	uwSize = sizeof(MainteLog);
	memcpy( &g_BkAuthData.BkMainteLog.err_wcnt, &g_MainteLog.err_wcnt, uwSize );

	//20160108Miya FinKeyS
	uwSize = sizeof(PasswordOpen2);
	memcpy( &g_BkAuthData.BkPasswordOpen2.family_sw, &g_PasswordOpen2.family_sw, uwSize );

	if( reg_num < 10 ){
		if( g_RegFlg == 0 ){
			g_RegFlg = 1;
		}else if( g_RegFlg == 2 ){
			g_RegFlg = 3;
		}
	}else{
		if( g_RegFlg == 0 ){
			g_RegFlg = 2;
		}else if( g_RegFlg == 1 ){
			g_RegFlg = 3;
		}
	}
	return(ercd);
}



/*==========================================================================*/
//	�摜����&�F�؏����o�b�N�A�b�v�f�[�^�ۑ�(�t���b�V��)
/*==========================================================================*/
static UB SaveBkAuthDataFl( void )
{
	UB	ercd=E_OK;
	volatile UW	uwaddr, uwSize, uwSizeIn;


	//�t���b�V���ۑ��Z�N�V����������	
	uwaddr = ADDR_REGAUTHDATA;
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	//g_RegBloodVesselTagData[19].end_code = 0x1234;
	g_MainteLog.end_code = 0x1234;
	g_PasswordOpen2.dummy[0] = 'E';
	g_PasswordOpen2.dummy[1] = 'N';
	g_PasswordOpen2.dummy[2] = 'D';
	g_PasswordOpen2.dummy[3] = 0xA5;

	memset( &Flbuf[0], 0xffff, 0x10000 );

	uwSize = sizeof(RegUserInfoData);
	memcpy( &g_BkAuthData.BkRegUserInfoData.RegSts, &g_RegUserInfoData.RegSts, uwSize );

	uwSize = sizeof(RegBloodVesselTagData)*20;
	memcpy( &g_BkAuthData.BkRegBloodVesselTagData[0].CapNum, &g_RegBloodVesselTagData[0].CapNum, uwSize );

	//20140925Miya password_open
	uwSize = sizeof(PasswordOpen);
	memcpy( &g_BkAuthData.BkPasswordOpen.sw, &g_PasswordOpen.sw, uwSize );

	//20140925Miya add Mainte
	uwSize = sizeof(TechMenuData);
	memcpy( &g_BkAuthData.BkTechMenuData.SysSpec, &g_TechMenuData.SysSpec, uwSize );

	//20140925Miya add log
	uwSize = sizeof(AuthLog);
	memcpy( &g_BkAuthData.BkAuthLog.ok_cnt, &g_AuthLog.ok_cnt, uwSize );

	//20140925Miya add log
	uwSize = sizeof(MainteLog);
	memcpy( &g_BkAuthData.BkMainteLog.err_wcnt, &g_MainteLog.err_wcnt, uwSize );

	//20160108Miya FinKeyS
	uwSize = sizeof(PasswordOpen2);
	memcpy( &g_BkAuthData.BkPasswordOpen2.family_sw, &g_PasswordOpen2.family_sw, uwSize );
	
	//uwSizeIn = sizeof(RegUserInfoData) + sizeof(RegBloodVesselTagData) * 20 + sizeof(PasswordOpen) + sizeof(TechMenuData) + sizeof(AuthLog) + sizeof(MainteLog);	//20140925Miya password_open
	uwSizeIn = sizeof(RegUserInfoData) + sizeof(RegBloodVesselTagData) * 20 + sizeof(PasswordOpen) + sizeof(TechMenuData) + sizeof(AuthLog) + sizeof(MainteLog) + sizeof(PasswordOpen2);	//20160108Miya FinKeyS
	memcpy( &Flbuf[0], &g_BkAuthData.BkRegUserInfoData.RegSts, uwSizeIn );
	//uwaddr = ADDR_REGAUTHDATA;
	
	//�t���b�V���ۑ��Z�N�V����������	
	//ercd = FlErase(uwaddr);
	//if( ercd != E_OK ){
	//	return(ercd);
	//}

	//�t���b�V���ۑ��Z�N�V������������	
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	dbg_flwsize = uwSize;
	dbg_flwsizeIn = uwSizeIn;
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

	return(ercd);
}


/*==========================================================================*/
//	�摜����&�F�؏����o�b�N�A�b�v�f�[�^�Ǎ�
/*==========================================================================*/
static UB ReadBkAuthData( void )
{
	UB	ercd=E_OK;
	volatile UW	uwaddr, uwSize, uwSizeIn;
	volatile unsigned long chk_size;
	volatile UB chk=1;

	chk_size = sizeof(int);
	chk_size = sizeof(long);
	chk_size = sizeof(double);
	chk_size = sizeof(UW);
	chk_size = sizeof(RegUserInfoData);
	chk_size = sizeof(RegBloodVesselTagData) * 20;
	chk_size = sizeof(PasswordOpen);
	chk_size = sizeof(TechMenuData);
	chk_size = sizeof(AuthLog);
	chk_size = sizeof(MainteLog);
	chk_size = sizeof(PasswordOpen2);

	uwaddr = ADDR_REGAUTHDATA;
	chk_size = sizeof(RegBloodVesselTagData);
	//uwSizeIn = sizeof(RegUserInfoData) + sizeof(RegBloodVesselTagData) * 20 + sizeof(PasswordOpen) + sizeof(TechMenuData) + sizeof(AuthLog) + sizeof(MainteLog);	//20140925Miya password_open
	uwSizeIn = sizeof(RegUserInfoData) + sizeof(RegBloodVesselTagData) * 20 + sizeof(PasswordOpen) + sizeof(TechMenuData) + sizeof(AuthLog) + sizeof(MainteLog) + sizeof(PasswordOpen2);	//20160108Miya FinKeyS

	//�t���b�V���ۑ��Z�N�V�����ǂݍ���	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}

	//22160112Miya FinKeyS
	if( GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS ){
		if(Flbuf[0] == 0xffff){
			memset( &Flbuf[0], 0, 0x10000 );
			memcpy( &g_BkAuthData.BkRegUserInfoData.RegSts, &Flbuf[0], uwSizeIn );
			InitBkAuthDataChar(0);	//20160108Miya FinKeyS
			g_BkAuthData.BkPasswordOpen.sw = 1;

			memcpy( &g_RegUserInfoData.RegSts, &g_BkAuthData.BkRegUserInfoData.RegSts, sizeof(RegUserInfoData) );
			memcpy( &g_RegBloodVesselTagData[0].CapNum, &g_BkAuthData.BkRegBloodVesselTagData[0].CapNum, sizeof(RegBloodVesselTagData)*20 );
			memcpy( &g_PasswordOpen.sw, &g_BkAuthData.BkPasswordOpen.sw, sizeof(PasswordOpen) );	//20140925Miya password_open
			memcpy( &g_TechMenuData.SysSpec, &g_BkAuthData.BkTechMenuData.SysSpec, sizeof(TechMenuData) );	//20140925Miya add log
			memcpy( &g_AuthLog.ok_cnt, &g_BkAuthData.BkAuthLog.ok_cnt, sizeof(AuthLog) );	//20140925Miya add log
			memcpy( &g_MainteLog.err_wcnt, &g_BkAuthData.BkMainteLog.err_wcnt, sizeof(MainteLog) );	//20140925Miya add log
			memcpy( &g_PasswordOpen2.family_sw, &g_BkAuthData.BkPasswordOpen2.family_sw, sizeof(PasswordOpen2) );	//20160108Miya FinKeyS

			//�t���b�V���ۑ��Z�N�V������������	
			uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
			if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
				ercd = 1;
				return(ercd);
			}
		}else{
			memcpy( &g_BkAuthData.BkRegUserInfoData.RegSts, &Flbuf[0], uwSizeIn );
			memcpy( &g_RegUserInfoData.RegSts, &g_BkAuthData.BkRegUserInfoData.RegSts, sizeof(RegUserInfoData) );
			memcpy( &g_RegBloodVesselTagData[0].CapNum, &g_BkAuthData.BkRegBloodVesselTagData[0].CapNum, sizeof(RegBloodVesselTagData)*20 );
			memcpy( &g_PasswordOpen.sw, &g_BkAuthData.BkPasswordOpen.sw, sizeof(PasswordOpen) );	//20140925Miya password_open
			memcpy( &g_TechMenuData.SysSpec, &g_BkAuthData.BkTechMenuData.SysSpec, sizeof(TechMenuData) );	//20140925Miya add log
			memcpy( &g_AuthLog.ok_cnt, &g_BkAuthData.BkAuthLog.ok_cnt, sizeof(AuthLog) );	//20140925Miya add log
			memcpy( &g_MainteLog.err_wcnt, &g_BkAuthData.BkMainteLog.err_wcnt, sizeof(MainteLog) );	//20140925Miya add log
			memcpy( &g_PasswordOpen2.family_sw, &g_BkAuthData.BkPasswordOpen2.family_sw, sizeof(PasswordOpen2) );	//20160108Miya FinKeyS

			g_BkAuthData.BkPasswordOpen.sw = 1;
			g_PasswordOpen.sw = 1;
		}

		return(ercd);
	}
	
	if( Flbuf[0] == 0xffff ){
		//InitBkAuthData();
		memset( &Flbuf[0], 0, 0x10000 );
		memcpy( &g_BkAuthData.BkRegUserInfoData.RegSts, &Flbuf[0], uwSizeIn );
		InitBkAuthDataChar(0);	//20160108Miya FinKeyS

		memcpy( &g_RegUserInfoData.RegSts, &g_BkAuthData.BkRegUserInfoData.RegSts, sizeof(RegUserInfoData) );
		memcpy( &g_RegBloodVesselTagData[0].CapNum, &g_BkAuthData.BkRegBloodVesselTagData[0].CapNum, sizeof(RegBloodVesselTagData)*20 );
		memcpy( &g_PasswordOpen.sw, &g_BkAuthData.BkPasswordOpen.sw, sizeof(PasswordOpen) );	//20140925Miya password_open
		memcpy( &g_TechMenuData.SysSpec, &g_BkAuthData.BkTechMenuData.SysSpec, sizeof(TechMenuData) );	//20140925Miya add log
		memcpy( &g_AuthLog.ok_cnt, &g_BkAuthData.BkAuthLog.ok_cnt, sizeof(AuthLog) );	//20140925Miya add log
		memcpy( &g_MainteLog.err_wcnt, &g_BkAuthData.BkMainteLog.err_wcnt, sizeof(MainteLog) );	//20140925Miya add log
		memcpy( &g_PasswordOpen2.family_sw, &g_BkAuthData.BkPasswordOpen2.family_sw, sizeof(PasswordOpen2) );	//20160108Miya FinKeyS

		//�t���b�V���ۑ��Z�N�V������������	
		uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
		if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
			ercd = 1;
			return(ercd);
		}
	}else{
		memcpy( &g_BkAuthData.BkRegUserInfoData.RegSts, &Flbuf[0], uwSizeIn );

		memcpy( &g_RegUserInfoData.RegSts, &g_BkAuthData.BkRegUserInfoData.RegSts, sizeof(RegUserInfoData) );
		memcpy( &g_RegBloodVesselTagData[0].CapNum, &g_BkAuthData.BkRegBloodVesselTagData[0].CapNum, sizeof(RegBloodVesselTagData)*20 );
		memcpy( &g_PasswordOpen.sw, &g_BkAuthData.BkPasswordOpen.sw, sizeof(PasswordOpen) );	//20140925Miya password_open
		memcpy( &g_TechMenuData.SysSpec, &g_BkAuthData.BkTechMenuData.SysSpec, sizeof(TechMenuData) );	//20140925Miya add log
		memcpy( &g_AuthLog.ok_cnt, &g_BkAuthData.BkAuthLog.ok_cnt, sizeof(AuthLog) );	//20140925Miya add log
		memcpy( &g_MainteLog.err_wcnt, &g_BkAuthData.BkMainteLog.err_wcnt, sizeof(MainteLog) );	//20140925Miya add log
		if(g_BkAuthData.BkPasswordOpen2.family_sw == -1){	//20160105Miya FinKeyS �o�[�W�����A�b�v��
			memset(&g_BkAuthData.BkPasswordOpen2.family_sw, 0, sizeof(PasswordOpen2));
			//InitBkAuthDataChar(3);	//20160108Miya FinKeyS
		}
		memcpy( &g_PasswordOpen2.family_sw, &g_BkAuthData.BkPasswordOpen2.family_sw, sizeof(PasswordOpen2) );	//20160108Miya FinKeyS

		//if(g_RegBloodVesselTagData[19].end_code != 0x1234){
		if(g_MainteLog.end_code != 0x1234){
			ercd = 1;
		}

	}

	return(ercd);
}


//20161031Miya Ver2204 ->
/*==========================================================================*/
//	���������Ȃ��G���A(LED�ʒu�����ALCD����)�o�b�N�A�b�v�f�[�^�ۑ�(�t���b�V��)
/*==========================================================================*/
static UB SaveBkDataNoClearFl( void )
{
	UB	ercd=E_OK;
	volatile UW	uwaddr, uwSize, uwSizeIn;

	//�t���b�V���ۑ��Z�N�V����������	
	uwaddr = ADDR_REGAUTHDATA2;
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	memset( &Flbuf[0], 0xffff, 0x10000 );

	uwSizeIn = sizeof(BkDataNoClear);
	memcpy( &Flbuf[0], &g_BkDataNoClear.LedPosiSetFlg, uwSizeIn );

	//�t���b�V���ۑ��Z�N�V������������	
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	dbg_flwsize = uwSize;
	dbg_flwsizeIn = uwSizeIn;
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

	return(ercd);
}

/*==========================================================================*/
//	���������Ȃ��G���A(LED�ʒu�����ALCD����)�o�b�N�A�b�v�f�[�^�Ǎ�(�t���b�V��)
/*==========================================================================*/
static UB ReadBkDataNoClearFl( void )
{
	UB	ercd=E_OK;
	volatile UW	uwaddr, uwSize, uwSizeIn;
	volatile unsigned long chk_size;
	volatile UB chk=1;

	uwaddr = ADDR_REGAUTHDATA2;
	uwSizeIn = sizeof(BkDataNoClear);

	//�t���b�V���ۑ��Z�N�V�����ǂݍ���	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}

	if( Flbuf[0] == 0xffff ){
		memset(&g_BkDataNoClear.LedPosiSetFlg, 0x00, uwSizeIn);
		SaveBkDataNoClearFl();
	}else{
		memcpy( &g_BkDataNoClear.LedPosiSetFlg, &Flbuf[0], uwSizeIn );
	}

	if(g_BkDataNoClear.InitRetryCnt < 0 || g_BkDataNoClear.InitRetryCnt > 5){
		g_BkDataNoClear.InitRetryCnt = 0;
	}
	if(g_BkDataNoClear.LiveCnt < 0 || g_BkDataNoClear.LiveCnt > 999){
		g_BkDataNoClear.LiveCnt = 0;
	}

	if(g_BkDataNoClear.LiveCnt > 999){
		g_BkDataNoClear.LiveCnt = 1;
	}else{
		g_BkDataNoClear.LiveCnt += 1;
	}
	

}
//20161031Miya Ver2204 <-

/*==========================================================================*/
//	�摜����&�F�؏����p�����[�^�ݒ�
/*==========================================================================*/
static UB SetImgAuthPara( UB *data )
{
	UB rtn = 0;
	UB *recbuf;
	UB ubtmp;
	unsigned short i, tp_num, reg_num;

	recbuf = data;

#if(PCCTRL == 1) //20160930Miya PC����VA300S�𐧌䂷��
	//�o�^�h�c
	reg_num = 0;
	g_RegUserInfoData.RegNum = reg_num;
	g_RegBloodVesselTagData[reg_num].RegNum = reg_num;
	//1:�o�^1��� 2:�o�^2��� 3:�F�؎�
	ubtmp = *(recbuf + 11) - 0x30;
	g_RegBloodVesselTagData[reg_num].CapNum = (unsigned short)ubtmp;
	//���ԍ��@"00"�`"99"
	g_RegUserInfoData.BlockNum = 0;
	g_RegBloodVesselTagData[reg_num].BlockNum = 0;
	//���[�U�[�h�c(�����ԍ�)
	//�o�^�҃��x���@"0"�F�ēҁ@"1"�F�Ǘ��ҁ@"2"�F��ʎ�
	g_RegBloodVesselTagData[reg_num].Level = 2;
	//�o�^�w
	ubtmp = *(recbuf + 27) - 0x30;
	tp_num = 10 * (unsigned short)ubtmp;
	ubtmp = *(recbuf + 28) - 0x30;
	tp_num += (unsigned short)ubtmp;
	g_RegBloodVesselTagData[reg_num].RegFinger = tp_num;
	//�o�^�Җ��O
	for( i = 0 ; i < 24 ; i++){
		g_RegBloodVesselTagData[reg_num].Name[i] = 0x20;
	}
#else
	//�o�^�h�c
	ubtmp = *(recbuf + 21) - 0x30;
	tp_num = 100 * (unsigned short)ubtmp;
	ubtmp = *(recbuf + 22) - 0x30;
	tp_num += (10 * (unsigned short)ubtmp);
	ubtmp = *(recbuf + 23) - 0x30;
	tp_num += (unsigned short)ubtmp;
	reg_num = tp_num;
	if( reg_num > 0 ){
		reg_num -= 1;
	}
	
	g_RegUserInfoData.RegNum = reg_num;
	g_RegBloodVesselTagData[reg_num].RegNum = reg_num;


	//1:�o�^1��� 2:�o�^2��� 3:�F�؎�
	ubtmp = *(recbuf + 11) - 0x30;
	g_RegBloodVesselTagData[reg_num].CapNum = (unsigned short)ubtmp;

	//���ԍ��@"00"�`"99"
	ubtmp = *(recbuf + 13) - 0x30;
	tp_num = 10 * (unsigned short)ubtmp;
	ubtmp = *(recbuf + 14) - 0x30;
	tp_num += (unsigned short)ubtmp;
	g_RegUserInfoData.BlockNum = tp_num;
	g_RegBloodVesselTagData[reg_num].BlockNum = tp_num;

	//���[�U�[�h�c(�����ԍ�)
	ubtmp = *(recbuf + 16) - 0x30;
	tp_num = 1000 * (unsigned short)ubtmp;
	ubtmp = *(recbuf + 17) - 0x30;
	tp_num += (100 * (unsigned short)ubtmp);
	ubtmp = *(recbuf + 18) - 0x30;
	tp_num += (10 * (unsigned short)ubtmp);
	ubtmp = *(recbuf + 19) - 0x30;
	tp_num += (unsigned short)ubtmp;
	g_RegUserInfoData.UserId = tp_num;
	g_RegBloodVesselTagData[reg_num].UserId = tp_num;

	//�o�^�҃��x���@"0"�F�ēҁ@"1"�F�Ǘ��ҁ@"2"�F��ʎ�
	ubtmp = *(recbuf + 25) - 0x30;
	g_RegBloodVesselTagData[reg_num].Level = (unsigned short)ubtmp;

	//�o�^�w
	ubtmp = *(recbuf + 27) - 0x30;
	tp_num = 10 * (unsigned short)ubtmp;
	ubtmp = *(recbuf + 28) - 0x30;
	tp_num += (unsigned short)ubtmp;
	g_RegBloodVesselTagData[reg_num].RegFinger = tp_num;

	//�o�^�Җ��O
	for( i = 0 ; i < 24 ; i++){
		ubtmp = *(recbuf + (30+i));
		g_RegBloodVesselTagData[reg_num].Name[i] = ubtmp;
	}
#endif	
	return(rtn);
}


/*==========================================================================*/
//	�摜����&�F�؏����p�����[�^�폜
/*==========================================================================*/
static UB DelImgAuthPara( void )
{
	UB rtn = 0;
	UB ubtmp;
	unsigned short i, tp_num, reg_num, kanri_num;


	if( g_RegUserInfoData.TotalNum == 1 ){	//�Ō��1�l�̏ꍇ�͍폜NG
		rtn = 1;
		return(rtn);
	}

	kanri_num = 0;
	for(i = 0 ; i < 4 ; i++){
		if( g_RegBloodVesselTagData[i].RegInfoFlg == 1 ){	//�o�^����̏ꍇ
			kanri_num += 1;
		}
	}

	//if(tp_num <= 1){	//�Ǘ���1�������c���Ă��Ȃ��̂ō폜�֎~
	//	rtn = 1;
	//	return(rtn);
	//}

	//�폜�h�c
	ubtmp = yb_touroku_data.yubi_seq_no[ 0 ] - 0x30;
	tp_num = 100 * (unsigned short)ubtmp;
	ubtmp = yb_touroku_data.yubi_seq_no[ 1 ] - 0x30;
	tp_num += (10 * (unsigned short)ubtmp);
	ubtmp = yb_touroku_data.yubi_seq_no[ 2 ] - 0x30;
	tp_num += (unsigned short)ubtmp;
	reg_num = tp_num;
	if( reg_num > 0 ){
		reg_num -= 1;
	}

	if( kanri_num <= 1 && reg_num < 4 ){ //�Ǘ���1�������c���Ă��Ȃ��̂ō폜�֎~
		rtn = 1;
		return(rtn);
	}


	if( g_RegBloodVesselTagData[reg_num].RegInfoFlg == 0 ){	//���o�^�̏ꍇ
		rtn = 1;
		return(rtn);
	}
	
	g_RegBloodVesselTagData[reg_num].RegInfoFlg = 0;
	g_RegBloodVesselTagData[reg_num].RegNum = 0;
	g_RegUserInfoData.TotalNum -= 1;

	if( reg_num < 10 ){
		if( g_RegFlg == 0 ){
			g_RegFlg = 1;
		}else if( g_RegFlg == 2 ){
			g_RegFlg = 3;
		}
	}else{
		if( g_RegFlg == 0 ){
			g_RegFlg = 2;
		}else if( g_RegFlg == 1 ){
			g_RegFlg = 3;
		}
	}

	return(rtn);
}


/*==========================================================================*/
//	�摜���菈��
/*==========================================================================*/
static UB ImgHantei( UB num )
{
	UB rtn = 0;
	UB rtn1, rtn2, rtn3, rtn4;
	int x, y, st_y;
	UB dat;
	unsigned char *h_img, *n_img, *l_img;
	unsigned long cnt, bun_cnt;
	double ave, sum;
	UB hantei;

	return(rtn);

}


/*==========================================================================*/
//	�摜���菈��(High�摜)
/*==========================================================================*/
static UB HiImgHantei( UB hantei )
{
	UB rtn = 0, sens = 2;
	volatile int x, y;
	unsigned char *h_img;
	short width, height;
	unsigned long cnt, h100cnt, bun_cnt;
	volatile int dat, hmax, hmin;
	int hmax0, hmax1, hmax2, hmax3, hmax4;
	int hini0, hmin1, hmin2, hmin3, hmin4;
	double have0, have1, have2, have3, have4;

	return(rtn);

}


/*==========================================================================*/
//	�摜���菈��(Mid�摜)
/*==========================================================================*/
static UB MidImgHantei( UB hantei )
{
	UB rtn = 0, sens = 2;
	int x, y;
	unsigned char *n_img;
	short width, height;
	unsigned long cnt, n100cnt, bun_cnt;
	int dat, nmax, nmin;
	double have, sum;

	return(rtn);
}


/*==========================================================================*/
//	�摜���菈��(Low�摜)
/*==========================================================================*/
static UB LowImgHantei( void )
{
	UB rtn = 0, sens = 2;
	
	return(rtn);
}



/*==========================================================================*/
//	�摜���菈��(�X���b�g)
/*==========================================================================*/
static UB SlitImgHantei( int *st )
{
	UB rtn = 0, sens = 2;

	return(rtn);
}



/*==========================================================================*/
//	�摜���菈��(�g���~���O)
/*==========================================================================*/
static void ImgTriming( int st_y )
{
	UB dat;
	int x, y;
	unsigned char *h_img, *n_img, *l_img;
	unsigned long cnt, bun_cnt;

	h_img = &g_ubCapBuf[0];
	n_img = &g_ubCapBuf[( iReSizeX * iReSizeY )];
	l_img = &g_ubCapBuf[( iReSizeX * iReSizeY )*2];

	cnt = 0;
	bun_cnt = st_y * MAX_SIZE_X;
	for(y = 0 ; y < MAX_SIZE_Y ; y++){
		for(x = 0 ; x < MAX_SIZE_X ; x++){
			dat = *(l_img + bun_cnt);
			*(l_img + cnt) = dat;
			dat = *(n_img + bun_cnt);
			*(n_img + cnt) = dat;
			dat = *(h_img + bun_cnt);
			*(h_img + cnt) = dat;
			++cnt;
			++bun_cnt;
		}
	}
}


/*==========================================================================*/
//	�摜����
/*==========================================================================*/
static UB DoImgProc( UB num )
{
	UB rtn = 0;

	return(rtn);
}

///////////////////////////////////
// �K���}�␳  20160603Miya
///////////////////////////////////
static void ProcGamma(double par)
{
	int rtn = 0;
	int x, y;
	int sum, den;
	double kei, dat;
	//int GammaLut[256];

	kei = 1.0 / par;
	for (x = 0; x < 256; x++){
		dat = ((double)x / 255.0);
		dat = pow(dat, kei) * 255;
		GammaLut[x] = (int)(dat + 0.5);
	}

}

/*==========================================================================*/
//	HDR��������
/*==========================================================================*/
static void HdrGouseiProc(void)
{

}

/*==========================================================================*/
//	�k������
/*==========================================================================*/
static void ImgResize4(int num)
{
	
}


//////////////////////////////////////
// Bicubic�@�k��
///////////////////////////////////////
static void dat_bicubicS(int sizeX,int sizeY,int out_X,int out_Y)
{
}


/*==========================================================================*/
//	X���g���~���O����
/*==========================================================================*/
static void TrimXsft(int num)
{
}

/*==========================================================================*/
//	�E��\�[�x������
/*==========================================================================*/
static void sobel_R1_Filter(int sizeX,int sizeY)
{
}


/*==========================================================================*/
//	�E���\�[�x������
/*==========================================================================*/
static void sobel_R2_Filter(int sizeX,int sizeY)
{
}


static void medianFilter7G(int num, int matX,int matY,int sizeX,int sizeY)
{
}

//20140905Miya LBP�ǉ�
////////////////////////////////////////////
// LBP(���[�J���o�C�i���[�p�^�[��)�摜�쐬
////////////////////////////////////////////
void local_binary_pattern(int sizeX,int sizeY)
{

}

//20140905Miya LBP�ǉ�
////////////////////////////////////////////
// LBP(���[�J���o�C�i���[�p�^�[��)�q�X�g�O�����쐬
////////////////////////////////////////////
static void make_lbp_hist(int szx, int szy, int num)
{
}

//20160312Miya �ɏ����xUP
////////////////////////////////////////////
// 16 �����R���g���X�g�z����쐬����
////////////////////////////////////////////
static void make_contrast(int sw)
{
}




#if(FPGA_HI == 1)	//20170706Miya 400Finger
/*==========================================================================*/
//	�F�؏���
/*==========================================================================*/
static UB DoAuthProc( UB num )
{
	UB rtn = 0;
	UB proc;
	int cap_num=0, sv_num=0;
	int learn1, learn2, r1, r2;
	volatile double score1, max_score, max_score1, max_score2;
	volatile double score2;
	volatile double genten1=1.0;
	volatile double genten2=1.0;
	volatile double total_score;
	volatile double th, th_low;
	volatile int auth_num, auth_learn, total, auth_total, yubi_size;
	volatile int auth_num_in, auth_learn_in, st, proc_num;
	int x, y, i, stx, edx, offsetx, offsety, svx, svy;
	volatile double yubi_rot;
	volatile int lvl;
	volatile int xsft, max_xsft;
	volatile double max_score_xsft;
	volatile int AuthLvlLow;	//20150531Miya
	volatile double cont_scr, cont_scr1, cont_scr2;

	//FpgaTest();
	
	learn1 = 0;
	learn2 = 1;
	r1 = 0;
	r2 = 1;
	auth_total = 0;

	if( num == 0 || num == 3 ){	//0:�ʏ�F�� 3:�o�^�p�F��
			memset( &score_buf[0][0], 0, sizeof(double) * 2 * 20 );
			memset( &auth_turn_num[0], 0, sizeof(int) * 10);	//20151118Miya �ɏ��F�،�����
			memset( &auth_turn_learn[0], 0, sizeof(int) * 10);	//20151118Miya �ɏ��F�،�����
			//memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
			//memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
			memset( &score_buf_cont[0][0], 0, sizeof(double) * 2 * 20 );	//20160312Miya �ɏ����xUP
			memset( &score_buf_cont2[0][0], 0, sizeof(double) * 2 * 20 );	//20161031Miya Ver2204

			total = (int)g_RegUserInfoData.TotalNum;
			th_low = (double)g_ImgAndAuthProcSetData.ThLowPart2;	//200

			//20160312Miya �ɏ����xUP �B�e�摜�̃R���g���X�g�����Z�o 
			TrimXsft(XSFT_NUM - XSFT0);	//�Z���^�[�g���~���O
			sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
			sobel_R2_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
			make_contrast(0);
			make_contrast(1);

			//20160312Miya �ɏ����xUP �ɏ�����@R2�����@->�@R1��R2
			//R2�ɏ�
			memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
			memcpy( &g_CapMini[0], &CapImgBuf[0][r2][0], MIN_SIZE_X * MIN_SIZE_Y );	//20161115Miya FPGA������ forDebug

			medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//�ɏ��摜�Ƀ��f�B�A��������
			ImgResize4(1);										//���T�C�Y
			memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��

			g_RegUserInfoData.lbp_pls = 0;	//20140905Miya lbp�ǉ�

			//20140905Miya lbp�ǉ� ->
			for( i = 0 ; i < 20 ; i++ ){
				lbp_score[i][0][0] = 0.0;	
				lbp_score[i][1][0] = 0.0;	
				lbp_score[i][0][1] = 0.0;	
				lbp_score[i][1][1] = 0.0;	
				lbp_score[i][0][2] = 0.0;	
				lbp_score[i][1][2] = 0.0;	
				lbp_score[i][0][3] = 0.0;	
				lbp_score[i][1][3] = 0.0;	
				lbp_score[i][0][4] = 0.0;	
				lbp_score[i][1][4] = 0.0;	
			}
			proc_num = 0;
			max_score = 0.0;
			st = 0;

			max_score = 1.0;

#if( FREEZTEST )
			memcpy( &CapImgBuf[0][r1][0], &g_RegBloodVesselTagData[dbg_dbg1].MinAuthImg[r1][0], MIN_SIZE_X * MIN_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
			memcpy( &CapImgBuf[0][r2][0], &g_RegBloodVesselTagData[dbg_dbg1].MinAuthImg[r2][0], MIN_SIZE_X * MIN_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
#endif

			AutoMachingFPGA(num, 1, &score1, &score2);	//20160902Miya FPGA������ forDebug
#if( FREEZTEST )
			if(g_FPGA_ErrFlg != 0)	return 1;
#endif

			if( max_score > 0.0 ){

				if(num == 3 ){	//�ӔC�ҔF��
					auth_total = ChkMaxPara( 0 );	//20151118Miya �ɏ��F�،�����
					if(auth_total == 0){
						s_CapResult = CAP_JUDGE_NG;
						return(1);
					}
					if(auth_total == 99){
						s_CapResult = CAP_JUDGE_NG;
						return(99);	//20151118Miya ���摜�ĎB�e
					}
				}else{
#if(AUTHTEST == 0)
					auth_total = ChkMaxPara( g_AuthCnt );	//20151118Miya �ɏ��F�،�����
#else
					auth_total = ChkMaxPara( 1 );	//20151118Miya �ɏ��F�،�����
#endif
					if(auth_total == 0){
						s_CapResult = CAP_JUDGE_NG;
						return(1);
					}
					if(auth_total == 99){
						s_CapResult = CAP_JUDGE_NG;
						return(99);	//20151118Miya ���摜�ĎB�e
					}
				}					

				//20140905Miya lbp�ǉ� �������ACapImgBuf[0][*][*]�́A���[�v�̊O�ɕύX				
				//memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
				//memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR2Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
				//for( i = 0 ; i < 4 ; i++ ){
				//20140423Miya �F�؃��g���C�ǉ� �F�؎����g���C�񐔃��Z�b�g
				//auth_total = 1;	//20160902Miya FPGA������ forDebug
				for( i = 0 ; i < auth_total ; i++ ){
					auth_num_in = auth_turn_num[i];			//20151118Miya �ɏ��F�،�����
					auth_learn_in = auth_turn_learn[i];		//20151118Miya �ɏ��F�،�����

#if( FREEZTEST == 0 )
					//20140910Miya XSFT ->
					if( XSFT_NUM ){
						stx = 0;
						edx = stx + MIN_SIZE_X;
						offsetx = 0;
						offsety = 0;
						max_score_xsft = 0.0;
						max_xsft = 0;
						memcpy( &CapImgBuf[1][r1][0], &g_RegBloodVesselTagData[auth_num_in].MinAuthImg[auth_learn_in][0], MIN_SIZE_X * MIN_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
						for(xsft = 0 ; xsft < XSFT_NUM ; xsft++){
							memcpy( &CapImgBuf[0][r1][0], &g_XsftMiniBuf[xsft][0], MIN_SIZE_X * MIN_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
							score1 = two_matcher7v( MIN_SIZE_X, MIN_SIZE_Y, cap_num, sv_num, stx, edx, offsetx, offsety, r1, learn1 );
							if( max_score_xsft < score1 ){
								max_score_xsft = score1;
								max_xsft = xsft;
							}
						}
						if( max_score_xsft == 0.0 ){
							max_xsft = XSFT0;
						}
						TrimXsft(XSFT_NUM - max_xsft);
						sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
						sobel_R2_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
						local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);	//20140905Miya LBP�ǉ�
						make_lbp_hist(AUTH_SIZE_X, AUTH_SIZE_Y, 0);		//20140905Miya LBP�ǉ�
					}
					g_PosGenten[0].x_scr1 = score1;
					//20140910Miya XSFT <-

					memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
					memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR2Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
#else
					memcpy( &CapImgBuf[0][r1][0], &g_RegImgDataAdd[dbg_dbg1].RegR1Img[0][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
					memcpy( &CapImgBuf[0][r2][0], &RegImgBuf[dbg_dbg1][0][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
#endif


					g_PosGenten[0].auth_num_in = auth_num_in;
					g_PosGenten[0].auth_learn_in = auth_learn_in;
					//20140905Miya lbp�ǉ�
					//memcpy( &g_ubResizeBuf[0], &RegImgBuf[auth_num_in][auth_learn_in][r1][0], AUTH_SIZE_X * AUTH_SIZE_Y );
					//local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
					//20160312Miya �ɏ����xUP �N�����E�o�^����LBP�쐬
					memcpy( &g_ubLbpBuf[0], &g_RegImgDataAdd[auth_num_in].RegLbpImg[auth_learn_in][0], AUTH_SIZE_X * AUTH_SIZE_Y );
					make_lbp_hist(AUTH_SIZE_X, AUTH_SIZE_Y, 1);
					lvl = two_matcher_lbp(auth_num_in, 0);
#if(NEWCMR == 0)//20160711Miya NewCmr
					//20160312Miya �ɏ����xUP �o�^�F�؎��ALBP_LVL=0�ɂ��Ȃ�
					if(num == 3 && lvl == 0)	lvl = 1;
					lbp_level[auth_num_in][auth_learn_in] = lvl;
					if( lvl == 99 || lvl == 15 ){
						//rtn = 1;
						rtn = 99;	//20151118Miya ���摜�ĎB�e
						break;
					}
#else
					lvl = 1;
#endif

					if( lvl > 0 ){	//LBP_lvl��0�͔F�؂��Ȃ�
						memcpy( &CapImgBuf[1][r1][0], &g_RegImgDataAdd[auth_num_in].RegR1Img[auth_learn_in][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
						memcpy( &CapImgBuf[1][r2][0], &RegImgBuf[auth_num_in][auth_learn_in][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
//#if( FREEZTEST )
						AutoMachingFPGA(0, 0, &score1, &score2);	//20160902Miya FPGA������
//#endif
						if(g_FPGA_ErrFlg != 0)	return 1;
						if( score1 < th_low || score2 < th_low ){
							score1 = score2 = 0.0;
						}else{
							AuthLvlLow = 0;
							g_PosGenten[0].scr1 = score1;
							g_PosGenten[0].scr2 = score2;
							if(score1*score1+score2*score2 < 500 * 500 * 2){
								AuthLvlLow = 1;
							}
							g_PosGenten[0].low_f = AuthLvlLow;
#if(NEWCMR == 0)//20160711Miya NewCmr
							genten_seq(auth_num_in, AuthLvlLow, lvl, &genten1, &genten2);	//20150531Miya
#else
							genten_seq_NewCmr(auth_num_in, &genten1, &genten2);	//20160711Miya NewCmr
#endif
			
						}


#if(NEWCMR == 0)//20160711Miya NewCmr
						if(num == 3 && max_score > 600.0 && i <= 1 && dip_sw_data[0] == 1){	//20151118Miya
							rtn = ChkScore(score1, score2);
						}else{
							g_PosGenten[0].gen1 = genten1;
							g_PosGenten[0].gen2 = genten2;
							rtn = ChkScoreLbp(auth_num_in, AuthLvlLow, score1, score2, genten1, genten2, lvl);
						}
						if( rtn == 0 ){
							g_RegUserInfoData.xsft = max_xsft;
							break;
						}
						//score_buf[auth_learn_in][auth_num_in] = 0.0;
						//ChkMaxPara( &auth_num_in, &auth_learn_in );	//20151118Miya �ɏ��F�،�����
#else
						rtn = ChkScoreNewCmr(auth_num_in, score1, score2, genten1, genten2); //20160711Miya NewCmr
						if( rtn == 0 ){
							g_RegUserInfoData.xsft = max_xsft;
							break;
						}
#endif
						
					}else{
						//score_buf[auth_learn_in][auth_num_in] = 0.0;	//20151118Miya �ɏ��F�،�����
						//ChkMaxPara( &auth_num_in, &auth_learn_in );
						rtn = 1;
					}
				}
				g_RegUserInfoData.RegNum = auth_num_in;
				g_RegUserInfoData.r1 = (unsigned short)score1;
				g_RegUserInfoData.r2 = (unsigned short)score2;
				g_RegUserInfoData.lbp_lvl = lbp_level[auth_num_in][auth_learn_in];

			}else{
				rtn = 1;	//�F��NG
			}
	}else{			//�o�^�F�� num=2
		proc = 1;
		cap_num = 0;	//�L���v�`���[�����摜�ԍ�
		sv_num = 1;		//�o�^����Ă���摜�ԍ�
		
		memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
		memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR2Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��

		memcpy( &CapImgBuf[1][r1][0], &g_ubSobelR1SvBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
		score1 = auto_matching( proc, cap_num, sv_num, r1, &auth_num );

		memcpy( &CapImgBuf[1][r2][0], &g_ubSobelR2SvBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
		score2 = auto_matching( proc, cap_num, sv_num, r2, &auth_num );

		//20150531Miya ���_�Ȃ�
		//genten_seq(sv_num, &genten1, &genten2);
		//score1 = score1 * genten1;
		//score2 = score2 * genten2;
		rtn = ChkScore(score1, score2);
	}

	
	if( rtn == 0 ){
		s_CapResult = CAP_JUDGE_OK;
	}else{
		s_CapResult = CAP_JUDGE_NG;
	}

	//s_CapResult = CAP_JUDGE_OK;
	//s_CapResult = CAP_JUDGE_NG;
	//s_CapResult = CAP_JUDGE_E1;
	//s_CapResult = CAP_JUDGE_E2;
	//rtn = 1;
	return(rtn);
}
#endif


#if(FPGA_HI == 1)	//20170220Miya
static int ChkMaxPara( int auth_num )
{
	
	return(0);	
}
#endif


static int ChkRegNum( UB proc, int num )
{
	int i, lmt, rtn;
	
	if( proc == 3 ){	//�o�^�p�F��
		lmt = 4;		//�ӔC�w4�{
	}else{
		lmt = 20;
	}
	
	rtn = 0xff;
	for( i = num ; i < lmt ; i++ ){
		if( g_RegBloodVesselTagData[i].RegInfoFlg == 1 ){
			rtn = (int)g_RegBloodVesselTagData[i].RegNum;
			break;
		}
	}

	return( rtn );
}

static UB ChkScore( double score1, double score2 )
{
	UB rtn;
	double th, total_score;
	unsigned short dat;

	if( score1 < 400.0 && score2 < 400.0 ){
		score1 *= 0.9;
		score2 *= 0.9;
	}
	if( score1 < 300.0 ){
		score1 *= 0.9;
	}
	if( score2 < 300.0 ){
		score2 *= 0.9;
	}
	if( score1 < 200.0 || score2 < 200.0 ){
		score1 *= 0.75;
		score2 *= 0.75;
	}

	total_score = score1 * score1 + score2 * score2;

	//20140423Miya �F�؃��g���C��
	//if( g_AuthCnt >= 2 ){
	//	dat = (unsigned short)((double)g_ImgAndAuthProcSetData.AuthLvlPara * 0.914 + 0.5);
	//}else{
	//	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	//}
	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	
	//th = 2.0 * (double)g_ImgAndAuthProcSetData.AuthLvlPara * (double)g_ImgAndAuthProcSetData.AuthLvlPara;
	th = 2.0 * (double)dat * (double)dat;

	if( total_score > th ){
		rtn = 0;	//�F��OK
	}else{
		rtn = 1;	//�F��NG
	}
	
	return(rtn);
}


//20140905Miya lbp�ǉ�
//static UB ChkScoreLbp( int id_B, double sv_score1, double sv_score2, double gentenA, double gentenB, int lbp_lvl )
static UB ChkScoreLbp( int id_B, int authlow, double sv_score1, double sv_score2, double gentenA, double gentenB, int lbp_lvl )
{
	UB rtn;
	volatile double th, total_score;
	volatile double score1, score2, genten1, genten2, gentenAB, th_genten;
	unsigned short dat;

	genten1 = gentenA;
	genten2 = gentenB;

	score1 = sv_score1 * genten1;
	score2 = sv_score2 * genten2;


#if ( NEWALGO > 0 )
	///////////////////////////////////////////////////////////
	// �����Ƃ��Â��i�����j�w�̏ꍇ �X�[�p�[�w�r�[��
	if(	g_RegUserInfoData.RegImageSens==0 && 
		g_RegBloodVesselTagData[id_B].RegImageSens==0)
	{
		if( score1 < 250.0 ){
			score1 *= 0.9;
			genten1 *= 0.9;
		}
		if( score2 < 250.0 ){
			score2 *= 0.9;
			genten2 *= 0.9;
		}
		if( score1 < 200.0 || score2 < 200.0 ){
			score1 *= 0.75;
			score2 *= 0.75;
			genten1 *= 0.75;
			genten2 *= 0.75;
		}
	}else{
		if(authlow == 1){
			if( score1 < 350.0 && score2 < 350.0 ){
				score1 *= 0.9;
				score2 *= 0.9;
				genten1 *= 0.9;
				genten2 *= 0.9;
			}
			if( score1 < 300.0 ){
				score1 *= 0.9;
				genten1 *= 0.9;
			}
			if( score2 < 300.0 ){
				score2 *= 0.9;
				genten2 *= 0.9;
			}
			if( score1 < 250.0 || score2 < 250.0 ){
				score1 *= 0.75;
				score2 *= 0.75;
				genten1 *= 0.75;
				genten2 *= 0.75;
			}
			if( score1 < 200.0 || score2 < 200.0 ){
				score1 *= 0.5;
				score2 *= 0.5;
				genten1 *= 0.5;
				genten2 *= 0.5;
			}
		}else{
			if( score1 < 300.0 ){
				score1 *= 0.9;
				genten1 *= 0.9;
			}
			if( score2 < 300.0 ){
				score2 *= 0.9;
				genten2 *= 0.9;
			}
			if( score1 < 250.0 || score2 < 250.0 ){
				score1 *= 0.75;
				score2 *= 0.75;
				genten1 *= 0.75;
				genten2 *= 0.75;
			}
			if( score1 < 200.0 || score2 < 200.0 ){
				score1 *= 0.5;
				score2 *= 0.5;
				genten1 *= 0.5;
				genten2 *= 0.5;
			}
		}
	}
#else
	///////////////////////////////////////////////////////////
	// �����Ƃ��Â��i�����j�w�̏ꍇ �X�[�p�[�w�r�[��
	if(	g_RegUserInfoData.RegImageSens==0 && 
		g_RegBloodVesselTagData[id_B].RegImageSens==0)
	{
		if( score1 < 300.0 && score2 < 300.0 ){
			score1 *= 0.9;
			score2 *= 0.9;
			genten1 *= 0.9;
			genten2 *= 0.9;
		}
		if( score1 < 250.0 ){
			score1 *= 0.9;
			genten1 *= 0.9;
		}
		if( score2 < 250.0 ){
			score2 *= 0.9;
			genten2 *= 0.9;
		}
		if( score1 < 200.0 || score2 < 200.0 ){
			score1 *= 0.75;
			score2 *= 0.75;
			genten1 *= 0.75;
			genten2 *= 0.75;
		}
	}else{
		if( score1 < 400.0 && score2 < 400.0 ){
			score1 *= 0.9;
			score2 *= 0.9;
			genten1 *= 0.9;
			genten2 *= 0.9;
		}
		if( score1 < 300.0 ){
			score1 *= 0.9;
			genten1 *= 0.9;
		}
		if( score2 < 300.0 ){
			score2 *= 0.9;
			genten2 *= 0.9;
		}
		if( score1 < 200.0 || score2 < 200.0 ){
			score1 *= 0.75;
			score2 *= 0.75;
			genten1 *= 0.75;
			genten2 *= 0.75;
		}
	}
#endif



	g_PosGenten[0].gen1 = genten1;
	g_PosGenten[0].gen2 = genten2;
	total_score = score1 * score1 + score2 * score2;

	//20140423Miya �F�؃��g���C��
	//if( g_AuthCnt >= 2 ){
	//	dat = (unsigned short)((double)g_ImgAndAuthProcSetData.AuthLvlPara * 0.914 + 0.5);
	//}else{
	//	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	//}
	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	
	//th = 2.0 * (double)g_ImgAndAuthProcSetData.AuthLvlPara * (double)g_ImgAndAuthProcSetData.AuthLvlPara;
	th = 2.0 * (double)dat * (double)dat;


	if( lbp_lvl >= 1 && lbp_lvl <= 6 )	//LBP LEVEL=1(1�`2), 2(3�`6)
	{
		if( total_score > th ){
			rtn = 0;	//�F��OK
		}else{
			rtn = 1;	//�F��NG
		}
	}
	else if( lbp_lvl >= 7 && lbp_lvl <= 10 )	//LBP LEVEL=3(7�`10)
	{
		gentenAB = genten1 * genten2;
		th_genten = 0.1;
		
#if ( NEWALGO > 0 )
		th_genten = 0.16 + (double)(10 - lbp_lvl) * 0.025;
#else
		if( lbp_lvl == 7 )	{th_genten = 0.2;}
		if( lbp_lvl == 8 )	{th_genten = 0.15;}
		if( lbp_lvl == 9 )	{th_genten = 0.125;}
		if( lbp_lvl == 10 )	{th_genten = 0.1;}
#endif
		if( total_score > th ){
			rtn = 0;	//�F��OK
		}else{
			if((sv_score1*sv_score1 + sv_score2*sv_score2) > 2.0*450 && gentenAB >= th_genten){	//lbp���_�ɘa(��)
				rtn = 0;	//�F��OK
				g_RegUserInfoData.lbp_pls = 1;
			}else{
				rtn = 1;	//�F��NG
			}
		}
	}else{	//LBP LEVEL=4(11�`12),5(13�`15)
		if( total_score > th ){
			rtn = 0;	//�F��OK
		}else{
			if((sv_score1*sv_score1 + sv_score2*sv_score2) > 2.0*450){	//lbp���_�ɘa(��)
				rtn = 0;	//�F��OK
				g_RegUserInfoData.lbp_pls = 1;
			}else{
				rtn = 1;	//�F��NG
			}
		}
	}

	
	return(rtn);
}

/*==========================================================================*/
//20160711Miya
//	�F�ؔ��菈��(NewCmr)
/*==========================================================================*/
static UB ChkScoreNewCmr( int id_B, double sv_score1, double sv_score2, double gentenA, double gentenB )
{
	UB rtn;
	volatile double th, thG, total_score, total_scoreG;
	volatile double score1, score2, genten0, genten1, genten2, gentenAB, th_genten;
	unsigned short dat;

	genten1 = gentenA;
	genten2 = gentenB;
	genten0 = genten1 * genten2;
	if(genten0 >= 1.4)	genten0 = 1.4;

	score1 = 0.5 * sv_score1 * genten0;	//NewCmr�ł�SCR�������o��X��������̂�1/2�ɂ���
	score2 = 0.5 * sv_score2 * genten0;
	
	g_PosGenten[0].gen1 = genten1;
	g_PosGenten[0].gen2 = genten2;
	total_score = score1 * score1 + score2 * score2;
	
	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	th = 2.0 * (double)dat * (double)dat;
	//thG = 2.0 * (double)(dat - 50) * (double)(dat - 50);
	thG = 2.0 * 220.0 * 220.0;	//20160810Miya
	
	if( total_score > th ){
		rtn = 0;	//�F��OK
	}else{
		if( total_score > thG ){
			total_scoreG = sv_score1 * sv_score1 + sv_score2 * sv_score2;
			//th = 2.0 * 400.0 * 400.0;
			th = 2.0 * 390.0 * 390.0;	//20160810Miya
			if(total_scoreG > th)	rtn = 0;	//�F��OK
			else 					rtn = 1;	//�F��NG
		}else{
			rtn = 1;	//�F��NG
		}
	}

	return(rtn);
}

/*==========================================================================*/
//	2�摜�̔F�؏���
/*==========================================================================*/
static double auto_matching( UB proc, int cap_num, int sv_num, int sbl, int *num )
{
	UB rtn = 0;
	int i, j, k, x, y;
	int sizex, sizey, dev_sizex;
	int stx, edx, offsetx, offsety, learn_num;
	int min1;
	volatile double score, max1, ave1, score2;
	
	sizex = AUTH_SIZE_X;	//80
	sizey = AUTH_SIZE_Y;	//40
	dev_sizex = sizex / DEV_NUM;	//4���� 80 / 4 = 20

	learn_num = 0;
	min1 = 999;
	ave1 = 0.0;

	for(k = 0 ; k < DEV_NUM ; k++){
		max1 = -100.0;
		match_pos[sbl][k][0] = 0;
		match_pos[sbl][k][1] = 0;
		for( y = -Y_SEARCH ; y <= Y_SEARCH ; y++ ){		
			for( x = -X_SEARCH ; x <= X_SEARCH ; x++ ){
				stx = k * dev_sizex;
				edx = stx + dev_sizex;
				offsetx = x;
				offsety = y;
				//�X�R�A�[�Z�o
				score = two_matcher7v( sizex, sizey, cap_num, sv_num, stx, edx, offsetx, offsety, sbl, learn_num );
//				score = two_matcher7vHi( proc, sizex, sizey, cap_num, sv_num, stx, edx, offsetx, offsety, sbl, learn_num );

//				if( score != score2 ){
//					nop();
//				}

				match_score[sbl][k][y + Y_SEARCH][x + X_SEARCH] = score;
				if( score >= max1 ){
					max1 = score;
					match_pos[sbl][k][0] = x;
					match_pos[sbl][k][1] = y;
				}
			}
		}
		if( max1 < 0.0 ){
			match_soukan[sbl][k] = -1;
		}else{
			match_soukan[sbl][k] = (int)max1;
		}
		if( (int)max1 <= min1 ){
			min1 = (int)max1;
		}
		ave1 += match_soukan[sbl][k];
		
	}

	ave1 = ave1 / 4.0;

	*num = 0;
	return(ave1);
}


/*==========================================================================*/
//	�G���A���F�؉��Z
/*==========================================================================*/
static double two_matcher7v( int sizex, int sizey, int cap_num, int sv_num, int stx, int edx, int offsetx, int offsety, int sbl, int learn_num)
{
	volatile double score;

	score = -1.0;
	return( score );
}

//20160312Miya �ɏ����xUP
/*==========================================================================*/
//	R1/R2�R���g���X�g�����̗ގ��x�����߂� regnum:�o�^�ԍ� learn;�o�^/�w�K sw:R1/R2
/*==========================================================================*/
static double contrast_soukan(int regnum, int learn, int sw)
{
	return 0.0;
}

/*==========================================================================*/
//	�F�،��_����
/*==========================================================================*/
//static void genten_seq(int id_B, double *gentenA, double *gentenB) 
static void genten_seq(int id_B, int authlow, int lbplvl, double *gentenA, double *gentenB) //20150531Miya
{
	*gentenA=0.0;
	*gentenB=0.0;
	return;	
}

/*==========================================================================*/
//20160711Miya
//	�F�،��_����(NewCmr)
/*==========================================================================*/
static void genten_seq_NewCmr(int id_B, double *gentenA, double *gentenB)
{
	*gentenA=0.0;
	*gentenB=0.0;
	return;	
}



///////////////////////////////////////
//  7GS0 �{�������@�z��v�Z
///////////////////////////////////////
static void matrix3_calc()
{
}


// �z��v�Z���̂Q
static void matrix3_calc2(int r,int k)
{
}


//20140423Miya FAR�΍�
static int matrix3_low_check(void)
{
	return(0);
}



///////////////////////////////////////
// 7GS0 �w�����̂���̃`�F�b�N
///////////////////////////////////////
static double x_ofst_check(int r)
{
	return 0.0;
}
///////////////////////////////////////
// 7GS0 Y�����̂���̃`�F�b�N
///////////////////////////////////////
static double y_ofst_check(int r)
{
	return 0.0;
}

///////////////////////////////////////
// �F�ؐ���֐� �⏕�֐�
// ����sqrt�֐�
///////////////////////////////////////
static float fastsqrt(float val)
{
union	{	int tmp;	float val;	} u;
	u.val=val;
	u.tmp -=1<<23;
	u.tmp >>=1;
	u.tmp +=1<<29;
	return u.val;
}


//20140905 miya
///////////////////////////////////////
// LBP�ގ��x�v�Z
// int t_num	�o�^NUM
// int g_num	�w�KNUM
///////////////////////////////////////
static int two_matcher_lbp(int t_num, int g_num)
{
	return(1);
}


//20140905 miya
///////////////////////////////////////
// LBP�q�X�g�O������r
///////////////////////////////////////
static double lbp_hist_compare(int b_num)
{
	return(0.0);
}

//20140905 miya
///////////////////////////////////////
// LBP���x������
///////////////////////////////////////
static int lbp_score_judge( double dat, double dat1, double dat2, double dat3, double dat4 )
{
	return(0);
}




/*==========================================================================*/
//	�F�؉摜��FLASH�ۑ�
/*==========================================================================*/
static UB SaveRegImgTmpArea( UB proc )
{
	volatile unsigned short num, learn1, learn2, r1, r2;
	volatile unsigned int	size;
	int cnt, x, y;
	UB *img;

	num = g_RegUserInfoData.RegNum;
	learn1 = 0;
	learn2 = 1;
	r1 = 0;
	r2 = 1;
	size = AUTH_SIZE_X * AUTH_SIZE_Y;

	if( proc == 0 ){	//�F�؎��@�w�K�f�[�^�̂ݍX�V
		//�ɏ��摜
		//for( i = 0 ; i < MIN_SIZE_X * MIN_SIZE_Y ; i++ ){
		//	g_RegBloodVesselTagData[num].MinAuthImg[1][i] = g_ubSobelR3Buf[i];
		//}
		memcpy(&g_RegBloodVesselTagData[num].MinAuthImg[1][0], &g_ubSobelR3DbgBuf[0], MIN_SIZE_X * MIN_SIZE_Y);
		g_RegBloodVesselTagData[num].RegFingerSize = g_RegUserInfoData.RegFingerSize;	//20160120Miya �ɏ�����̏d�݂ɂ������

		//�w�K�p�o�b�t�@�ɕۑ�
		//memcpy( &RegImgBuf[num][1][0][0], &CapImgBuf[0][0][0], size );
		if( XSFT_NUM ){
			TrimXsft(XSFT_NUM - g_RegUserInfoData.xsft);
			memcpy( &RegImgBuf[num][1][0][0], &g_ubResizeBuf[0], size );		//20140905Miya lbp�ǉ� R1�G���A��HDR��ۑ� R1�͔F�؎��Čv�Z
		}else{
			memcpy( &RegImgBuf[num][1][0][0], &g_ubResizeSvBuf[0], size );		//20140905Miya lbp�ǉ� R1�G���A��HDR��ۑ� R1�͔F�؎��Čv�Z
		}
		memcpy( &RegImgBuf[num][1][1][0], &CapImgBuf[0][1][0], size );
		AddRegImgFromRegImg(2, num);		//20160312Miya �ɏ����xUP �w�K�̂�
	}else{
		//�o�^�p�o�b�t�@�ɕۑ�
		//memcpy( &RegImgBuf[num][0][0][0], &CapImgBuf[0][0][0], size );
		memcpy( &RegImgBuf[num][0][0][0], &g_ubResizeSvBuf[0], size );		//20140905Miya lbp�ǉ� R1�G���A��HDR��ۑ� R1�͔F�؎��Čv�Z
		memcpy( &RegImgBuf[num][0][1][0], &CapImgBuf[0][1][0], size );

		//�w�K�p�o�b�t�@�ɕۑ�
		//memcpy( &RegImgBuf[num][1][0][0], &CapImgBuf[1][0][0], size );
		memcpy( &RegImgBuf[num][1][0][0], &g_ubResizeSvBuf2[0], size );		//20140905Miya lbp�ǉ� R1�G���A��HDR��ۑ� R1�͔F�؎��Čv�Z
		memcpy( &RegImgBuf[num][1][1][0], &CapImgBuf[1][1][0], size );
		AddRegImgFromRegImg(1, num);		//20160312Miya �ɏ����xUP �o�^�E�w�K
	}
}


/*==========================================================================*/
//	�F�؉摜��FLASH�ۑ�
/*==========================================================================*/
static UB SaveRegImgFlArea( int num )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;
	
	uwSizeIn = AUTH_SIZE_X * AUTH_SIZE_Y * 2 * 2 * 10;	//10��

	if( num < 10 ){	//�Z�N�V����1(1�`10)
		uwaddr = ADDR_REGIMG1;
		memcpy( &Flbuf[0], &RegImgBuf[0][0][0][0], uwSizeIn );
	}else{			//�Z�N�V����2(11�`20)
		uwaddr = ADDR_REGIMG2;
		memcpy( &Flbuf[0], &RegImgBuf[10][0][0][0], uwSizeIn );
	}

	//�t���b�V���ۑ��Z�N�V����������	
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	//�t���b�V���ۑ��Z�N�V������������	
	uwSizeIn = uwSizeIn / 2;							//word�P�ʂɂ��T�C�Y����
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

	return(ercd);
}


/*==========================================================================*/
//	�F�؉摜��FLASH�ǂݏo��
/*==========================================================================*/
static UB ReadRegImgArea( int num )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;

//�e���v���[�g������
	memset( &RegImgBuf[0][0][0][0], 0, sizeof(RegImgBuf) );
	memset( &Flbuf[0], 0, 0x10000 );

	uwSizeIn = AUTH_SIZE_X * AUTH_SIZE_Y * 2 * 2 * 10;	//10��
	uwSizeIn = uwSizeIn / 2;							//word�P�ʂɂ��T�C�Y����

//�Z�N�V����1(1�`10)
	uwaddr = ADDR_REGIMG1;
	//�t���b�V���ۑ��Z�N�V�����ǂݍ���	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	//�e���v���[�g�ɃR�s�[�@0�Ԓn�`
	memcpy( &RegImgBuf[0][0][0][0], &Flbuf[0], 2*uwSizeIn );


//�Z�N�V����2(10�`20)
	uwaddr = ADDR_REGIMG2;
	//�t���b�V���ۑ��Z�N�V�����ǂݍ���	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	//�e���v���[�g�ɃR�s�[�@10�Ԓn�`
	memcpy( &RegImgBuf[10][0][0][0], &Flbuf[0], 2*uwSizeIn );

	return(ercd);
}

/*==========================================================================*/
//20160312Miya �ɏ����xUP
//	FLASH�ǂݏo�����F�؉摜(HDR�摜)����R1�ALBP�AR1�ɏ��쐬 (SW 0:�I�[�� 1:�o�^�E�w�K(NUM�w��) 2:�w�K�̂�(NUM�w��))
/*==========================================================================*/
static UB AddRegImgFromRegImg( int sw, int num )
{
	UB	ercd=E_OK;
	int i;
	int tou=0, gaku=1, hdr=0, r1=0, r2=1;

	if(sw == 0){
		for(i = 0 ; i < 20 ; i++){
			if( g_RegBloodVesselTagData[i].RegInfoFlg == 1 ){
				//�o�^�G���A
				memcpy( &g_ubResizeBuf[0], &RegImgBuf[i][tou][hdr][0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//LBP
				local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
				memcpy( &g_RegImgDataAdd[i].RegLbpImg[tou][0], &g_ubLbpBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//R1
				sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
				memcpy( &g_RegImgDataAdd[i].RegR1Img[tou][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//R1 16�����R���g���X�g�����쐬
				make_contrast(r1);
				memcpy( &g_RegImgDataAdd[i].RegContrast[tou][r1][0], &g_contrast_sa[r1][0], sizeof(short)*16 );
				//R1�ɏ�
				medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//�ɏ��摜�Ƀ��f�B�A��������
				ImgResize4(1);										//���T�C�Y
				memcpy( &g_RegImgDataAdd[i].RegMiniR1Img[tou][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );
				//R2 16�����R���g���X�g�����쐬
				memcpy( &g_ubSobelR2Buf[0], &RegImgBuf[i][tou][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
				make_contrast(r2);
				memcpy( &g_RegImgDataAdd[i].RegContrast[tou][r2][0], &g_contrast_sa[r2][0], sizeof(short)*16 );
			
				//�w�K�G���A
				memcpy( &g_ubResizeBuf[0], &RegImgBuf[i][gaku][hdr][0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//LBP
				local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
				memcpy( &g_RegImgDataAdd[i].RegLbpImg[gaku][0], &g_ubLbpBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//R1
				sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
				memcpy( &g_RegImgDataAdd[i].RegR1Img[gaku][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//R1 16�����R���g���X�g�����쐬
				make_contrast(r1);
				memcpy( &g_RegImgDataAdd[i].RegContrast[gaku][r1][0], &g_contrast_sa[r1][0], sizeof(short)*16 );
				//R1�ɏ�
				medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//�ɏ��摜�Ƀ��f�B�A��������
				ImgResize4(1);										//���T�C�Y
				memcpy( &g_RegImgDataAdd[i].RegMiniR1Img[gaku][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );
				//R2 16�����R���g���X�g�����쐬
				memcpy( &g_ubSobelR2Buf[0], &RegImgBuf[i][gaku][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
				make_contrast(r2);
				memcpy( &g_RegImgDataAdd[i].RegContrast[gaku][r2][0], &g_contrast_sa[r2][0], sizeof(short)*16 );
			}
		}
	}else{
		i = num;	//NUM�w��
		if(sw == 1){
			//�o�^�G���A
			memcpy( &g_ubResizeBuf[0], &RegImgBuf[i][tou][hdr][0], AUTH_SIZE_X * AUTH_SIZE_Y );
			//LBP
			local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
			memcpy( &g_RegImgDataAdd[i].RegLbpImg[tou][0], &g_ubLbpBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
			//R1
			sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
			memcpy( &g_RegImgDataAdd[i].RegR1Img[tou][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
			//R1 16�����R���g���X�g�����쐬
			make_contrast(r1);
			memcpy( &g_RegImgDataAdd[i].RegContrast[tou][r1][0], &g_contrast_sa[r1][0], sizeof(int)*16 );
			//R1�ɏ�
			medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//�ɏ��摜�Ƀ��f�B�A��������
			ImgResize4(1);										//���T�C�Y
			memcpy( &g_RegImgDataAdd[i].RegMiniR1Img[tou][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );
			//R2 16�����R���g���X�g�����쐬
			memcpy( &g_ubSobelR2Buf[0], &RegImgBuf[i][tou][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
			make_contrast(r2);
			memcpy( &g_RegImgDataAdd[i].RegContrast[tou][r2][0], &g_contrast_sa[r2][0], sizeof(short)*16 );
		}

		//�w�K�G���A
		memcpy( &g_ubResizeBuf[0], &RegImgBuf[i][gaku][hdr][0], AUTH_SIZE_X * AUTH_SIZE_Y );
		//LBP
		local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
		memcpy( &g_RegImgDataAdd[i].RegLbpImg[gaku][0], &g_ubLbpBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
		//R1
		sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
		memcpy( &g_RegImgDataAdd[i].RegR1Img[gaku][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
		//R1 16�����R���g���X�g�����쐬
		make_contrast(r1);
		memcpy( &g_RegImgDataAdd[i].RegContrast[gaku][r1][0], &g_contrast_sa[r1][0], sizeof(short)*16 );
		//R1�ɏ�
		medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//�ɏ��摜�Ƀ��f�B�A��������
		ImgResize4(1);										//���T�C�Y
		memcpy( &g_RegImgDataAdd[i].RegMiniR1Img[gaku][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );
		//R2 16�����R���g���X�g�����쐬
		memcpy( &g_ubSobelR2Buf[0], &RegImgBuf[i][gaku][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
		make_contrast(r2);
		memcpy( &g_RegImgDataAdd[i].RegContrast[gaku][r2][0], &g_contrast_sa[r2][0], sizeof(short)*16 );
	}

	return(ercd);
}

/*==========================================================================*/
//	�F�؉摜��FLASH������
/*==========================================================================*/
static UB InitRegImgArea( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;
	int i;
	//UH	uhData[ 8 ];
	//unsigned long i, j, cnt;


//�e���v���[�g������
	memset( &RegImgBuf[0][0][0][0], 0, sizeof(RegImgBuf) );
	memset( &Flbuf[0], 0, 0x10000 );

//�Z�N�V����1(1�`10)
	uwaddr = ADDR_REGIMG1;
	//�t���b�V���ۑ��Z�N�V����������

	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	
	//�t���b�V���p�ޔ��o�b�t�@�ɃR�s�[
	uwSizeIn = AUTH_SIZE_X * AUTH_SIZE_Y * 2 * 2 * 10;
	memcpy( &Flbuf[0], &RegImgBuf[0][0][0][0], uwSizeIn );
	uwSizeIn = uwSizeIn / 2;
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

//�Z�N�V����2(11�`20)
	uwaddr = ADDR_REGIMG2;
	//�t���b�V���ۑ��Z�N�V����������

	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	//�t���b�V���p�ޔ��o�b�t�@�ɃR�s�[
	uwSizeIn = AUTH_SIZE_X * AUTH_SIZE_Y * 2 * 2 * 10;
	memcpy( &Flbuf[0], &RegImgBuf[10][0][0][0], uwSizeIn );
	uwSizeIn = uwSizeIn / 2;
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

#if(AUTHTEST >= 1)	//20150613Miya
	g_sv_okcnt = 0;
//OK�Z�N�V����
	for( i = 0 ; i < 4 ; i++ ){
		if( i == 0 ){
			uwaddr = ADDR_OKIMG1;
			g_sv_okcnt1 = 0;
		}else if( i == 1 ){
			uwaddr = ADDR_OKIMG2;
			g_sv_okcnt2 = 0;
		}else if( i == 2 ){
			uwaddr = ADDR_OKIMG3;
			g_sv_okcnt3 = 0;
		}else{
			uwaddr = ADDR_OKIMG4;
			g_sv_okcnt4 = 0;
		}

		//�t���V��������
		ercd = E_OK;//FlErase(uwaddr);
		if( ercd != E_OK ){
			return(ercd);
		}

		//�ۑ��G���A��0�ݒ�
		uwSizeIn = sizeof(short) + 100 * 40 * 32;
		memset( &Flbuf[0], 0x00, uwSizeIn );
		uwSizeIn = uwSizeIn / 2;
		uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
		if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
			ercd = 1;
			return(ercd);
		}
	}
	g_sv_okcnt0 = g_sv_okcnt1 + g_sv_okcnt2 + g_sv_okcnt3 + g_sv_okcnt4;

#endif


	return(ercd);

}

#if(AUTHTEST >= 1)	//20160613Miya
/*==========================================================================*/
//	�e�X�g�p�F�؉摜��FLASH�ۑ� 
/*==========================================================================*/
static UB InitTestRegImgFlArea( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn, ofst;
	short i, lt, cnt;

	g_sv_okcnt = 0;
//OK�Z�N�V����
	for( i = 0 ; i < 4 ; i++ ){
		if( i == 0 ){
			uwaddr = ADDR_OKIMG1;
			g_sv_okcnt1 = 0;
		}else if( i == 1 ){
			uwaddr = ADDR_OKIMG2;
			g_sv_okcnt2 = 0;
		}else if( i == 2 ){
			uwaddr = ADDR_OKIMG3;
			g_sv_okcnt3 = 0;
		}else{
			uwaddr = ADDR_OKIMG4;
			g_sv_okcnt4 = 0;
		}

		//�t���V��������
		ercd = E_OK;//FlErase(uwaddr);
		if( ercd != E_OK ){
			return(ercd);
		}

		//�ۑ��G���A��0�ݒ�
		uwSizeIn = sizeof(short) + 100 * 40 * 32;
		memset( &Flbuf[0], 0x00, uwSizeIn );
		uwSizeIn = uwSizeIn / 2;
		uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
		if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
			ercd = 1;
			return(ercd);
		}
	}
	g_sv_okcnt0 = g_sv_okcnt1 + g_sv_okcnt2 + g_sv_okcnt3 + g_sv_okcnt4;

	return(ercd);
}

/*==========================================================================*/
//	�e�X�g�p�F�؉摜��FLASH�ۑ� 
/*==========================================================================*/
static UB SaveTestRegImgFlArea( unsigned short ok_ng_f )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn, ofst;
	short i, lt;

	return(0);	//20180406Miya ForSH

	if(ok_ng_f == 0){	//OK�G���A
		if(g_sv_okcnt < 8){
			for(i = 0 ; i < 8 ; i++){
				memset( &TstAuthImgBuf[g_sv_okcnt][0], 0xff, 100 * 40 );
				++g_sv_okcnt;
				if(g_sv_okcnt >= 8) break;
			}
		}	
		if( g_sv_okcnt0 < 32 ){
			uwaddr = ADDR_OKIMG1;
			ofst = sizeof(short) + 100 * 40 * g_sv_okcnt1;
			g_sv_okcnt1 += g_sv_okcnt;
			memcpy( &Flbuf[0], &g_sv_okcnt1, sizeof(short) );
		}else if( g_sv_okcnt0 < 64 ){
			uwaddr = ADDR_OKIMG2;
			ofst = sizeof(short) + 100 * 40 * g_sv_okcnt2;
			g_sv_okcnt2 += g_sv_okcnt;
			memcpy( &Flbuf[0], &g_sv_okcnt2, sizeof(short) );
		}else if( g_sv_okcnt0 < 96 ){
			uwaddr = ADDR_OKIMG3;
			ofst = sizeof(short) + 100 * 40 * g_sv_okcnt3;
			g_sv_okcnt3 += g_sv_okcnt;
			memcpy( &Flbuf[0], &g_sv_okcnt3, sizeof(short) );
		}else if( g_sv_okcnt0 < 128 ){
			uwaddr = ADDR_OKIMG4;
			ofst = sizeof(short) + 100 * 40 * g_sv_okcnt4;
			g_sv_okcnt4 += g_sv_okcnt;
			memcpy( &Flbuf[0], &g_sv_okcnt4, sizeof(short) );
		}else{
			//20160810Miya ���[�v�Ή�
			g_sv_okcnt1 = g_sv_okcnt2 = g_sv_okcnt3 = g_sv_okcnt4 = 0;
			uwaddr = ADDR_OKIMG1;
			ofst = sizeof(short) + 100 * 40 * g_sv_okcnt1;
			g_sv_okcnt1 += g_sv_okcnt;
			memcpy( &Flbuf[0], &g_sv_okcnt1, sizeof(short) );
			//return(0);
		}
		ofst = ofst / 2;
		g_sv_okcnt0 = g_sv_okcnt1 + g_sv_okcnt2 + g_sv_okcnt3 + g_sv_okcnt4;
		memcpy( &Flbuf[ofst], &TstAuthImgBuf[0][0], 100 * 40 * 8 );
	}
	uwSizeIn = sizeof(short) + 100 * 40 * 32;

	//�t���b�V���ۑ��Z�N�V����������	
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	//�t���b�V���ۑ��Z�N�V������������	
	uwSizeIn = uwSizeIn / 2;							//word�P�ʂɂ��T�C�Y����
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

	return(ercd);
}

/*==========================================================================*/
//	�e�X�g�p�F�؉摜��FLASH�ǂݏo��
/*==========================================================================*/
static UB ReadTestRegImgArea( unsigned short ok_ng_f, short cpy_f, short num, short num10 )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn, ofst;

	return(0);	//20180406Miya ForSH

//�e���v���[�g������
	memset( &Flbuf[0], 0, 0x10000 );

	uwSizeIn = sizeof(short) + 100 * 40 * 32;
	uwSizeIn = uwSizeIn / 2;							//word�P�ʂɂ��T�C�Y����

	if( cpy_f == 1 ){	//�f�[�^�擾
		if(ok_ng_f == 0){	//OK�G���A
			memset( &TstAuthImgBuf[0][0], 0, 100 * 40 * 8 );
			if( num == 1 ){
				uwaddr = ADDR_OKIMG1;
			}else if( num == 2 ){
				uwaddr = ADDR_OKIMG2;
			}else if( num == 3 ){
				uwaddr = ADDR_OKIMG3;
			}else{
				uwaddr = ADDR_OKIMG4;
			}		
		}
		ofst = sizeof(short) + 100 * 40 * 8 * num10;
	}else{				//�t���b�V���������ݑO�ɓǂݍ���
		if(ok_ng_f == 0){	//OK�G���A
			if( g_sv_okcnt0 < 32 ){
				uwaddr = ADDR_OKIMG1;
			}else if( g_sv_okcnt0 < 64 ){
				uwaddr = ADDR_OKIMG2;
			}else if( g_sv_okcnt0 < 96 ){
				uwaddr = ADDR_OKIMG3;
			}else if( g_sv_okcnt0 < 128 ){	//20160810Miya ���[�v�Ή�
				uwaddr = ADDR_OKIMG4;
			}else{
				uwaddr = ADDR_OKIMG1;
			}		
		}
	}

	//�t���b�V���ۑ��Z�N�V�����ǂݍ���	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}

	if( cpy_f == 1 ){	//�f�[�^�擾
		ofst = ofst / 2;
		if(ok_ng_f == 0){	//OK�G���A
			memcpy( &TstAuthImgBuf[0][0], &Flbuf[ofst], 100 * 40 * 8 );
		}
	}


	return(ercd);
}

/*==========================================================================*/
//	�e�X�g�p�F�؉摜�J�E���^�[��FLASH�ǂݏo��
/*==========================================================================*/
static UB ReadTestRegImgCnt( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn, ofst;

//�e���v���[�g������
	memset( &Flbuf[0], 0, 0x10000 );

	uwSizeIn = sizeof(short);
	uwSizeIn = uwSizeIn / 2;							//word�P�ʂɂ��T�C�Y����

	uwaddr = ADDR_OKIMG1;
	//�t���b�V���ۑ��Z�N�V�����ǂݍ���	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	memcpy( &g_sv_okcnt1, &Flbuf[0], sizeof(short) );

	uwaddr = ADDR_OKIMG2;
	//�t���b�V���ۑ��Z�N�V�����ǂݍ���	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	memcpy( &g_sv_okcnt2, &Flbuf[0], sizeof(short) );

	uwaddr = ADDR_OKIMG3;
	//�t���b�V���ۑ��Z�N�V�����ǂݍ���	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	memcpy( &g_sv_okcnt3, &Flbuf[0], sizeof(short) );

	uwaddr = ADDR_OKIMG4;
	//�t���b�V���ۑ��Z�N�V�����ǂݍ���	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	memcpy( &g_sv_okcnt4, &Flbuf[0], sizeof(short) );

	return(ercd);
}
#endif

static void MakeOpenKeyNum(void)
{
	volatile unsigned int seed, rnum1, rnum2, dat, para;
	volatile int num1[4], num2[4], num0[8];
	int i;
	volatile unsigned short num, reg_num, line;
	volatile unsigned short para0, para1, para2, para3, para4;
	volatile unsigned short sum1, sum2, sum3, sum4;

	int jyunretu[24][4] = {
			{1,2,3,4}, {1,2,4,3}, {1,3,2,4}, {1,3,4,2}, {1,4,2,3}, {1,4,3,2},
			{2,1,3,4}, {2,1,4,3}, {2,3,1,4}, {2,3,4,1}, {2,4,1,3}, {2,4,3,1},
			{3,1,2,4}, {3,1,4,2}, {3,2,1,4}, {3,2,4,1}, {3,4,1,2}, {3,4,2,1},
			{4,1,2,3}, {4,1,3,2}, {4,2,1,3}, {4,2,3,1}, {4,3,1,2}, {4,3,2,1}
	};

	num = g_RegUserInfoData.kinkyu_times;
	++g_RegUserInfoData.kinkyu_times;
	g_RegUserInfoData.kinkyu_times = g_RegUserInfoData.kinkyu_times & 0xff;

	SaveBkAuthDataFl();

	para0 = num % 4 + num % 3;

	switch( para0 )
	{
		case 0:
			para1 = 1;
			para2 = 0;
			para3 = 1;
			para4 = AUTH_SIZE_X;
			break;
		case 1:
			para1 = 1;
			para2 = 1;
			para3 = 0;
			para4 = AUTH_SIZE_X;
			break;
		case 2:
			para1 = 1;
			para2 = 0;
			para3 = 0;
			para4 = AUTH_SIZE_X;
			break;
		case 3:
			para1 = 1;
			para2 = 1;
			para3 = 1;
			para4 = AUTH_SIZE_X;
			break;
		case 4:
			para1 = 1;
			para2 = 0;
			para3 = 1;
			para4 = AUTH_SIZE_Y;
			break;
		case 5:
			para1 = 1;
			para2 = 1;
			para3 = 0;
			para4 = AUTH_SIZE_Y;
			break;
		default:
			break;
	}

	for( i = 0 ; i < 20 ; i++ ){
		if( g_RegBloodVesselTagData[i].RegInfoFlg == 1 ){
			reg_num = i;
			break;
		}
	}

	line = num % 40;
	sum1 = CalcLineSum( (int)reg_num, (int)para1, (int)para2, (int)line, (int)para4 );
	line = 10 + (sum1 % 10);
	sum2 = CalcLineSum( (int)reg_num, (int)para1, (int)para2, (int)line, (int)para4 );
	line = 20 + (sum2 % 10);
	sum3 = CalcLineSum( (int)reg_num, (int)para1, (int)para2, (int)line, (int)para4 );
	line = 30 + (sum3 % 10);
	sum4 = CalcLineSum( (int)reg_num, (int)para1, (int)para2, (int)line, (int)para4 );


	para0 = num % 4;
	switch( para0 )
	{
		case 0:	//�I�[��1�̈�
			rnum1 = (sum1 % 10) * 1000 + (sum2 % 10) * 100 + (sum3 % 10) * 10 + (sum4 % 10);
			break;
		case 1:	//�I�[��10�̈�
			rnum1 = 0;
			rnum1 += 1000 * GetKeta(sum1, 10);
			rnum1 += 100 * GetKeta(sum2, 10);
			rnum1 += 10 * GetKeta(sum3, 10);
			rnum1 += GetKeta(sum4, 10);
			break;
		case 2:	//�I�[��100�̈�
			rnum1 = 0;
			rnum1 += 1000 * GetKeta(sum1, 100);
			rnum1 += 100 * GetKeta(sum2, 100);
			rnum1 += 10 * GetKeta(sum3, 100);
			rnum1 += GetKeta(sum4, 100);
			break;
		case 3:	//1,10,100,1000
			rnum1 = 0;
			rnum1 += 1000 * (sum1 % 10);
			rnum1 += 100 * GetKeta(sum2, 10);
			rnum1 += 10 * GetKeta(sum3, 100);
			rnum1 += GetKeta(sum4, 1000);
			break;
		default:
			rnum1 = (sum1 % 10) * 1000 + (sum2 % 10) * 100 + (sum3 % 10) * 10 + (sum4 % 10);
			break;
	}


	line = 30 + (sum4 % 10) + (num % 3);
	if(line >= 40){ line -= 40; }
	sum1 = CalcLineSum( (int)reg_num, (int)para1, (int)para3, (int)line, (int)para4 );
	line = 20 + (sum1 % 10);
	sum2 = CalcLineSum( (int)reg_num, (int)para1, (int)para3, (int)line, (int)para4 );
	line = 10 + (sum2 % 10);
	sum3 = CalcLineSum( (int)reg_num, (int)para1, (int)para3, (int)line, (int)para4 );
	line = sum3 % 10;
	sum4 = CalcLineSum( (int)reg_num, (int)para1, (int)para3, (int)line, (int)para4 );

	switch( para0 )
	{
		case 0:	//1,10,100,1000
			rnum2 = 0;
			rnum2 += 1000 * (sum1 % 10);
			rnum2 += 100 * GetKeta(sum2, 10);
			rnum2 += 10 * GetKeta(sum3, 100);
			rnum2 += GetKeta(sum4, 1000);
			break;
		case 1:	//�I�[��1�̈�
			rnum2 = (sum1 % 10) * 1000 + (sum2 % 10) * 100 + (sum3 % 10) * 10 + (sum4 % 10);
			break;
		case 2:	//�I�[��10�̈�
			rnum2 = 0;
			rnum2 += 1000 * GetKeta(sum1, 10);
			rnum2 += 100 * GetKeta(sum2, 10);
			rnum2 += 10 * GetKeta(sum3, 10);
			rnum2 += GetKeta(sum4, 10);
			break;
		case 3:	//�I�[��100�̈�
			rnum2 = 0;
			rnum2 += 1000 * GetKeta(sum1, 100);
			rnum2 += 100 * GetKeta(sum2, 100);
			rnum2 += 10 * GetKeta(sum3, 100);
			rnum2 += GetKeta(sum4, 100);
			break;
		default:
			rnum2 = (sum1 % 10) * 1000 + (sum2 % 10) * 100 + (sum3 % 10) * 10 + (sum4 % 10);
			break;
	}

	switch( para0 )
	{
		case 0:
			para = rnum1 % 24;
			break;
		case 1:
			para = sum1 % 24;
			break;
		case 2:
			para = sum2 % 24;
			break;
		case 3:
			para = sum3 % 24;
			break;
		case 4:
			para = sum4 % 24;
			break;
		case 5:
			para = rnum2 % 24;
			break;
		default:
			break;
	}

	dat = rnum1 / 1000;
	num1[jyunretu[para][0]-1] = dat;
	g_UseProcNum.OpenKeyNum[0] = dat + 0x30;
	rnum1 = rnum1 - dat * 1000;
	dat = rnum1 / 100;
	num1[jyunretu[para][1]-1] = dat;
	g_UseProcNum.OpenKeyNum[1] = dat + 0x30;
	rnum1 = rnum1 - dat * 100;
	dat = rnum1 / 10;
	num1[jyunretu[para][2]-1] = dat;
	g_UseProcNum.OpenKeyNum[2] = dat + 0x30;
	rnum1 = rnum1 - dat * 10;
	dat = rnum1;
	num1[jyunretu[para][3]-1] = dat;
	g_UseProcNum.OpenKeyNum[3] = dat + 0x30;

	dat = rnum2 / 1000;
	num2[jyunretu[para][0]-1] = dat;
	g_UseProcNum.OpenKeyNum[4] = dat + 0x30;
	rnum2 = rnum2 - dat * 1000;
	dat = rnum2 / 100;
	num2[jyunretu[para][1]-1] = dat;
	g_UseProcNum.OpenKeyNum[5] = dat + 0x30;
	rnum2 = rnum2 - dat * 100;
	dat = rnum2 / 10;
	num2[jyunretu[para][2]-1] = dat;
	g_UseProcNum.OpenKeyNum[6] = dat + 0x30;
	rnum2 = rnum2 - dat * 10;
	dat = rnum2;
	num2[jyunretu[para][3]-1] = dat;
	g_UseProcNum.OpenKeyNum[7] = dat + 0x30;

	if((para % 2) == 0){
		memcpy(&num0[0], &num1[0], sizeof(unsigned int)*4 );
		memcpy(&num0[4], &num2[0], sizeof(unsigned int)*4 );
	}else{
		memcpy(&num0[0], &num2[0], sizeof(unsigned int)*4 );
		memcpy(&num0[4], &num1[0], sizeof(unsigned int)*4 );
	}

	if( (num0[0] == num0[1]) && (num0[2] == num0[3]) && (num0[0] == num0[2]) ){	//4���������ꍇ(4�����������Ƃ͂��肦�Ȃ��悤�ɂ���)
		if(num0[1] == 9){
			num0[1] = 0;
		}else{
			num0[1] += 1;
		}
		if(num0[2] == 0){
			num0[2] = 9;
		}else{
			num0[1] -= 1;
		}
	}

	if( (num0[4] == num0[5]) && (num0[6] == num0[7]) && (num0[4] == num0[6]) ){	//4���������ꍇ(4�����������Ƃ͂��肦�Ȃ��悤�ɂ���)
		if(num0[4] == 9){
			num0[4] = 0;
		}else{
			num0[4] += 1;
		}
		if(num0[6] == 0){
			num0[6] = 9;
		}else{
			num0[6] -= 1;
		}
	}

	for(i = 0 ; i < 8 ; i++){
		if(num0[i] + 0x30 > 0x39 || num0[i] + 0x30 < 0x30 ){
			nop();
		}
		g_UseProcNum.OpenKeyNum[i] = num0[i] + 0x30;
		kinkyuu_hyouji_no[i] = num0[i] + 0x30;
	}
	kinkyuu_hyouji_no[8] = 0;

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

	dat = 0;
	for(i = 0 ; i < 8 ; i++){
		dat += (num0[i] + i + 2 * i);
	}
	para = dat % 24;

	if( (dat % 2 == 0) ){
		for(i = 0 ; i < 4 ; i++){
			num1[i] = g_UseProcNum.OpenKeyNum[jyunretu[para][i]-1] - 0x30;
			num2[i] = g_UseProcNum.OpenKeyNum[jyunretu[para][i]+3] - 0x30;
		}
	}else{
		for(i = 0 ; i < 4 ; i++){
			num2[i] = g_UseProcNum.OpenKeyNum[jyunretu[para][i]-1] - 0x30;
			num1[i] = g_UseProcNum.OpenKeyNum[jyunretu[para][i]+3] - 0x30;
		}
	}

	for(i = 0 ; i < 4 ; i++){
		if(jyunretu[para][i] == 1){
			dat = num0[i] + num1[i] + para;
		}else if(jyunretu[para][i] == 2){
			dat = num0[i] * num1[i] + para;
		}else if(jyunretu[para][i] == 3){
			dat = num1[i] * num1[i] + para;
		}else{
			if(num0[i] >= num1[i]){
				dat = num0[i] - num1[i] + para;
			}else{
				dat = num1[i] - num0[i] + para;
			}
		}
		num1[i] = dat % 10;
	}

	for(i = 0 ; i < 4 ; i++){
		dat += (num1[i] + i + 2 * i);
	}
	para = dat % 24;

	for(i = 0 ; i < 4 ; i++){
		if(jyunretu[para][i] == 1){
			dat = num0[4+i] + num2[i] + para;
		}else if(jyunretu[para][i] == 2){
			dat = num0[4+i] * num2[i] + para;
		}else if(jyunretu[para][i] == 3){
			dat = num2[i] * num2[i] + para;
		}else{
			if(num0[4+i] >= num1[i]){
				dat = num0[4+i] - num2[i] + para;
			}else{
				dat = num2[i] - num0[4+i] + para;
			}
		}
		num2[i] = dat % 10;
	}

	memcpy(&num0[0], &num1[0], sizeof(unsigned int)*4 );
	memcpy(&num0[4], &num2[0], sizeof(unsigned int)*4 );

	for(i = 0 ; i < 8 ; i++){
		g_UseProcNum.CalOpenCode[i] = num0[i] + 0x30;
	}

}

//20140925Miya password_open
static UB ChekKinkyuKaijyouKey( void )
{
	UB rtn=E_OK;
	int i;
	
	for(i = 0 ; i < 4 ; i++ ){
		if( g_RegUserInfoData.KinkyuNum[i] != g_UseProcNum.InKinkyuNum[i] ){
			rtn = 1;
			return(rtn);
		}
	}
	
	for(i = 0 ; i < 8 ; i++ ){
		if( g_UseProcNum.CalOpenCode[i] != g_UseProcNum.InOpenCode[i] ){
			rtn = 1;
			return(rtn);
		}
	}
	
	return(rtn);
}


static unsigned short GetKeta( int indat, int keta )
{
	int outdat;
	
	if(keta == 10){
		outdat = indat % 100;
		outdat = outdat / 10;
	}
	if(keta == 100){
		outdat = indat % 1000;
		outdat = outdat / 100;
	}
	if(keta == 1000){
		outdat = indat % 10000;
		outdat = outdat / 1000;
	}
	
	return(outdat);
}


static unsigned int CalcLineSum( int num, int ln, int sobel, int line, int lmt )
{
	int i, cnt;
	unsigned short sum;
	
	sum = 0;
	for( i = 0 ; i < lmt ; i++ ){
		sum += RegImgBuf[num][ln][sobel][line*AUTH_SIZE_X+i];
	}
	
	return(sum);	
}	


static void SetReCap( void )
{
	UB ercd = 0;
	unsigned short touroku_num;
	
	touroku_num = g_RegUserInfoData.CapNum;

	if( s_CapResult == CAP_JUDGE_RT ){//�ŎB�e(�Â�)
		irDuty2 = 255;		
		irDuty3 = 255;
		irDuty4 = 255;
		irDuty5 = 0;
		cmrGain = ini_cmrGain + 2;
		g_CmrParaSet = 1;
		ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// �J�����^�X�N�ɁA���ڂ̃Q�C���ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j
	}
	if( s_CapResult == CAP_JUDGE_RT2 ){//�ŎB�e(���邢)
		irDuty2 = 255;		
		irDuty3 = 255;
		irDuty4 = 128;
		irDuty5 = 0;
		cmrFixShutter1 = 5;
		cmrFixShutter2 = 6;
		cmrFixShutter3 = 7;
		g_CmrParaSet = 1;
		ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// �J�����^�X�N�ɁA���ڂ̘I�o�R�ݒ�l��ݒ���˗��iFPGA���o�R���Ȃ��j
	}


	while(1){
		if(	g_CmrParaSet == 0 ){
			//dly_tsk( 500/MSEC );
			break;
		}
		dly_tsk( 100/MSEC );
	}

	g_CapTimes = 2;	//20131210Miya add
	if( touroku_num == 1 || touroku_num == 2 ){
		ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// �J�����B�e+�o�^�����i�R�}���h204�j�ցB
	}else{
		ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP211 );			// �F�ؗp�B�e�����i�R�}���h210�j�ցB
	}
}


static UB ChekPasswordOpenKey( void )
{
	UB rtn=E_OK;
	int i, lmt, cnt, chk;

	//if(g_TechMenuData.SysSpec == 0){	//20160108Miya FinKeyS
	if(g_PasswordOpen2.family_sw == 0){	//20160120Miya FinKeyS
		lmt = g_InPasswordOpen.keta;
	
		if( g_PasswordOpen.keta != lmt ){
			rtn = 1;
			return(rtn);
		}
	
		for(i = 0 ; i < lmt ; i++ ){
			if( g_PasswordOpen.password[i] != g_InPasswordOpen.password[i] ){
				rtn = 1;
				return(rtn);
			}
		}
	}else{
		rtn = 1;
		for(cnt = 0 ; cnt < 20 ; cnt++){
			if(g_RegBloodVesselTagData[cnt].RegInfoFlg == 1){
				lmt = (int)g_InPasswordOpen.keta;
				
				if(g_PasswordOpen2.keta[cnt] == lmt){
					chk = 0;
					for(i = 0 ; i < lmt ; i++ ){
						if( g_PasswordOpen2.password[cnt][i] != g_InPasswordOpen.password[i] ){
							chk = 1;
							break;
						}
					}
					if(chk == 0){
						g_PasswordOpen2.num = cnt;
						rtn = E_OK;
						break;
					}
				}
			}
		}
	}
	
	return(rtn);
}

//20160108Miya FinKeyS
static UB CngNameCharaCode( unsigned char code, int *num )
{
	int i, k;
	unsigned char dat0, dat1, dat2;
	
	k = 1;
	dat0 = 0x20;
	dat1 = code & 0xF0;
	dat2 = code & 0x0F;
	
	if(dat1 == 0x10){		//��
		if(dat2 < 5){
			dat0 = 0xB1;
			dat0 = dat0 + dat2;
		}else if(dat2 < 10){
			dat0 = 0xA7;
			dat0 = dat0 + (dat2 - 5);
		}
		k = 1;
	}else if(dat1 == 0x20){	//��
		dat0 = 0xB6;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x30){	//��
		dat0 = 0xBB;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x40){	//��
		if(dat2 < 5){
			dat0 = 0xC0;
			dat0 = dat0 + dat2;
		}else if(dat2 == 5){
			dat0 = 0xAF;
		}
		k = 1;
	}else if(dat1 == 0x50){	//��
		dat0 = 0xC5;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x60){	//��
		dat0 = 0xCA;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x70){	//��
		dat0 = 0xCF;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x80){	//��
		if(dat2 < 3){
			dat0 = 0xD4;
			dat0 = dat0 + dat2;
		}else if(dat2 < 6){
			dat0 = 0xAC;
			dat0 = dat0 + (dat2 - 5);
		}
		k = 1;
	}else if(dat1 == 0x90){	//��
		dat0 = 0xD7;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0xA0){	//��
		if(dat2 == 0){
			dat0 = 0xDC;
		}else if(dat2 == 1){
			dat0 = 0xA6;
		}else if(dat2 == 2){
			dat0 = 0xDD;
		}
		k = 1;
	}else if(dat1 == 0xB0){	//��
		dat0 = 0xB6;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 2;
	}else if(dat1 == 0xC0){	//��
		dat0 = 0xBB;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 2;
	}else if(dat1 == 0xD0){	//��
		if(dat2 < 5){
			dat0 = 0xC0;
			dat0 = dat0 + dat2;
		}
		k = 2;
	}else if(dat1 == 0xE0){	//��
		dat0 = 0xCA;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 2;
	}else if(dat1 == 0xF0){	//��
		dat0 = 0xCA;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 3;
	}
	
	*num = k;
	return(dat0);
}


UB MemCheck( unsigned long offset )
{
	UB ercd = 0;
	volatile int i, cnt;
	volatile unsigned long dat;


	for(i = 0 ; i < 1280 ; i++ ){
		Flbuf[i] = (i & 0xFF);
	}
	
	
	//ercd = CmrMemWrt( CAP_BANK_0, 0, 0, 80, 40, &tst_buf[0] );
	
	cnt = 0;
	for(i = 0 ; i < 256 ; i++ ){
		*(volatile unsigned long *)(CAP_BASE + 4 * (unsigned long)i + offset) = (unsigned long)Flbuf[i];

		dat = *(volatile unsigned long *)(CAP_BASE + 4 * (unsigned long)i + offset);

		if( Flbuf[i] != (UH)dat ){
			++cnt;
			//break;
		}
	}

/*	
	for(i = 0 ; i < 32 ; i++ ){
		chk_buf2[i] = chk_buf[i];
	}
*/

	ercd = (UB)cnt & 0xff;
	return ercd;
}

///////////////////////////////////////////////////////////////////////////
//20160902Miya FPGA������ ->
///////////////////////////////////////////////////////////////////////////
// ts 0:�B�e�摜 1:�o�^�摜 sbl 0:R1 1:R2
static int WrtImgToRam(int ts, int sbl)
{
	int x, y, i, cnt, rtn, bun;
	volatile unsigned long	pixdata, pix, addr;
	double sum, ave;
	
	rtn = 0;

	if(ts == 0){	//�Œ葤(�B�e�摜)
		if(sbl == 0)	addr = 0xC00000;	//R1
		else			addr = 0xE00000;	//R2
	}else{			//���炵��(�o�^�摜)
		if(sbl == 0)	addr = 0xD00000;	//R1
		else			addr = 0xF00000;	//R2
	}

	//�������ɉ摜��4�������ď�����
	i = cnt = 0;
	sum = 0.0;
	for(bun = 0 ; bun < 4 ; bun++){
		//cnt = 0;
		for(y = 0 ; y < AUTH_SIZE_Y ; y++){			//AUTH_SIZE_Y = 80
			cnt = AUTH_SIZE_X * y + 20 * bun;
			//cnt = AUTH_SIZE_X * y;
			for(x = 0 ; x < 20 / 4 ; x++){		//AUTH_SIZE_X = 40
				pixdata = 0;
				//1pix
				pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[�o�^][R1][��f]
				sum += (double)pix;
				pixdata = pixdata | pix;		
				//2pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[�o�^][R1][��f]
				sum += (double)pix;
				pixdata = pixdata | pix;		
				//3pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[�o�^][R1][��f]
				sum += (double)pix;
				pixdata = pixdata | pix;		
				//4pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[�o�^][R1][��f]
				sum += (double)pix;
				pixdata = pixdata | pix;		
	
				*(volatile unsigned long *)(CAP_BASE + addr + 4 * (unsigned long)i++) = pixdata;
			}
		}
	}
	
	
/*	�������ɉ摜����ʏ�����
	for(y = 0 ; y < AUTH_SIZE_Y ; y++){			//AUTH_SIZE_Y = 80
		for(x = 0 ; x < AUTH_SIZE_X / 4 ; x++){		//AUTH_SIZE_X = 40
			pixdata = 0;
			//1pix
			pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[�o�^][R1][��f]
			sum += (double)pix;
			pixdata = pixdata | pix;		
			//2pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[�o�^][R1][��f]
			sum += (double)pix;
			pixdata = pixdata | pix;		
			//3pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[�o�^][R1][��f]
			sum += (double)pix;
			pixdata = pixdata | pix;		
			//4pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[�o�^][R1][��f]
			sum += (double)pix;
			pixdata = pixdata | pix;		
	
			*(volatile unsigned long *)(CAP_BASE + addr + 4 * (unsigned long)i++) = pixdata;
		}
	}
*/
	
	ave = sum / (double)cnt;
	rtn = (int)ave;
	
	return(rtn);
}

static int WrtMiniImgToRam(UB num, int learn, int sbl)
{
	int x, y, i, cnt1, cnt2, rtn, total, proc_num, st;
	int learn1, learn2, r1, r2;
	volatile unsigned long	pixdata, pix, addr1, addr2;
	
	rtn = 0;
	learn1 = 0;
	learn2 = 1;
	r1 = 0;
	r2 = 1;

	total = (int)g_RegUserInfoData.TotalNum;
	addr1 = 0xC00000;	//���炵��(�o�^�摜)
	addr2 = 0xD00000;	//�Œ葤(�B�e�摜)

	proc_num = 0;
	st = 0;
	cnt2 = 0;
	for(i = 0 ; i < total ; i++){
		proc_num = ChkRegNum(num, st);
		if( proc_num == 0xff ){
			break;
		}else{
			st = proc_num + 1;
		}

#if(COMMON == 0)	//20170315Miya 400Finger
		if(sbl == 1)	//R2
			memcpy( &CapImgBuf[1][sbl][0], &g_RegBloodVesselTagData[proc_num].MinAuthImg[learn][0], MIN_SIZE_X * MIN_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
		else			//R1
			memcpy( &CapImgBuf[1][sbl][0], &g_RegImgDataAdd[proc_num].RegMiniR1Img[learn1][0], MIN_SIZE_X * MIN_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
#else	//20170315Miya 400Finger
		if(sbl == 1)	//R2
			memcpy( &CapImgBuf[1][sbl][0], &g_RegBloodVesselTagData[proc_num].MinAuthImg[sbl][0], MIN_SIZE_X * MIN_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
		else			//R1
			memcpy( &CapImgBuf[1][sbl][0], &g_RegBloodVesselTagData[proc_num].MinAuthImg[sbl][0], MIN_SIZE_X * MIN_SIZE_Y );	//�ۑ��摜(�o�^1���)->�e���v���[�g1��
#endif
		cnt1 = 0;		
		for(y = 0 ; y < MIN_SIZE_Y ; y++){			//AUTH_SIZE_Y = 10
			for(x = 0 ; x < MIN_SIZE_X / 4 ; x++){		//AUTH_SIZE_X = 20
				pixdata = 0;
				//1pix
				pix = (unsigned long)CapImgBuf[1][sbl][cnt1++];	//[�o�^][R1][��f]
				pixdata = pixdata | pix;		
				//2pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[1][sbl][cnt1++];	//[�o�^][R1][��f]
				pixdata = pixdata | pix;		
				//3pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[1][sbl][cnt1++];	//[�o�^][R1][��f]
				pixdata = pixdata | pix;		
				//4pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[1][sbl][cnt1++];	//[�o�^][R1][��f]
				pixdata = pixdata | pix;		
	
				*(volatile unsigned long *)(CAP_BASE + addr1 + 4 * (unsigned long)cnt2++) = pixdata;
			}
		}
	}

	cnt2 = 0;
	cnt1 = 0;		
	//memcpy( &CapImgBuf[0][sbl][0], &g_CapMini[0], MIN_SIZE_X * MIN_SIZE_Y );		//�L���v�`���[�摜->�e���v���[�g0��
	for(y = 0 ; y < MIN_SIZE_Y ; y++){			//AUTH_SIZE_Y = 10
		for(x = 0 ; x < MIN_SIZE_X / 4 ; x++){		//AUTH_SIZE_X = 20
			pixdata = 0;
			//1pix
			pix = (unsigned long)CapImgBuf[0][sbl][cnt1++];	//[�o�^][R1][��f]
			pixdata = pixdata | pix;		
			//2pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[0][sbl][cnt1++];	//[�o�^][R1][��f]
			pixdata = pixdata | pix;		
			//3pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[0][sbl][cnt1++];	//[�o�^][R1][��f]
			pixdata = pixdata | pix;		
			//4pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[0][sbl][cnt1++];	//[�o�^][R1][��f]
			pixdata = pixdata | pix;		
	
			*(volatile unsigned long *)(CAP_BASE + addr2 + 4 * (unsigned long)cnt2++) = pixdata;
		}
	}
	
	return(total);
}

static int Cal4Ave(int ts, int sbl, int *rt1, int *rt2, int *rt3, int *rt4)
{
	int x, y, i, cnt, rtn, ofst;
	double sum;
	int ave1, ave2, ave3, ave4;

	for(i = 0 ; i < 4 ; i++){
		cnt = 0;
		sum = 0.0;
		for(y = 0 ; y < AUTH_SIZE_Y ; y++){			//AUTH_SIZE_Y = 80
			ofst = y * AUTH_SIZE_Y + 20 * i;
			for(x = 0 ; x < AUTH_SIZE_X / 4 ; x++){		//AUTH_SIZE_X = 40
				sum += (double)CapImgBuf[ts][sbl][ofst + x];
				++cnt;
			}
		}
		if(i == 0)			ave1 = (int)(sum / (double)cnt);
		else if(i == 1)		ave2 = (int)(sum / (double)cnt);
		else if(i == 2)		ave3 = (int)(sum / (double)cnt);
		else if(i == 3)		ave4 = (int)(sum / (double)cnt);
	}

	*rt1 = ave1;
	*rt2 = ave2;
	*rt3 = ave3;
	*rt4 = ave4;

}

void MakeTestImg(void)
{
	int i, x, y, cnt, adr;	
	
	cnt = 0;
	for(y = 0 ; y < 40 ; y++){
		for(x = 0 ; x < 80 ; x++){
			CapImgBuf[0][0][cnt] = 128;		//�B�e�@R1
			CapImgBuf[0][1][cnt] = 128;		//�B�e�@R2
			CapImgBuf[1][0][cnt] = 128;		//�o�^�@R1
			CapImgBuf[1][1][cnt] = 128;		//�o�^�@R2
			++cnt;
		}
	}

	for(i = 0 ; i < 4 ; i++){
		for(y = 0 ; y < 40 ; y++){
			for(x = 0 ; x < 20 ; x++){
				if( x >= 5 && x < 15 && y >= 10 && y < 30){
					adr = y * 80 + 20 * i + x;
					//CapImgBuf[0][0][adr] = 64;		//�B�e�@R1
					//CapImgBuf[0][1][adr] = 20;		//�B�e�@R2
					//CapImgBuf[0][1][adr] = 64;		//�B�e�@R2
					CapImgBuf[0][0][adr] = 200;		//�B�e�@R1
					CapImgBuf[0][1][adr] = 200;		//�B�e�@R2
					if(i==1 || i==2){
						CapImgBuf[0][0][adr] = 220;		//�B�e�@R1
						CapImgBuf[0][1][adr] = 220;		//�B�e�@R2
					}
				}					
				if( x >= 5 && x < 15 && y >= 10 && y < 30){
					adr = y * 80 + 20 * i + x;
					//CapImgBuf[1][0][adr] = 64;		//�o�^�@R1
					//CapImgBuf[1][1][adr] = 20;		//�o�^�@R2
					//CapImgBuf[1][1][adr] = 64;		//�o�^�@R2
					CapImgBuf[1][0][adr] = 200;		//�o�^�@R1
					CapImgBuf[1][1][adr] = 200;		//�o�^�@R2
					if(i==1 || i==2){
						CapImgBuf[1][0][adr] = 220;		//�o�^�@R1
						CapImgBuf[1][1][adr] = 220;		//�o�^�@R2
					}
				}					

				if( x >= 5 && x < 15 && y >= 20 && y < 30){
					adr = y * 80 + 20 * i + x;
					//CapImgBuf[0][0][adr] = 64;		//�B�e�@R1
					//CapImgBuf[0][1][adr] = 20;		//�B�e�@R2
					//CapImgBuf[0][1][adr] = 64;		//�B�e�@R2
					CapImgBuf[0][0][adr] = 64;		//�B�e�@R1
					CapImgBuf[0][1][adr] = 64;		//�B�e�@R2
				}					
				if( x >= 5 && x < 15 && y >= 20 && y < 30){
					adr = y * 80 + 20 * i + x;
					//CapImgBuf[1][0][adr] = 64;		//�o�^�@R1
					//CapImgBuf[1][1][adr] = 20;		//�o�^�@R2
					//CapImgBuf[1][1][adr] = 64;		//�o�^�@R2
					CapImgBuf[1][0][adr] = 32;		//�o�^�@R1
					CapImgBuf[1][1][adr] = 32;		//�o�^�@R2
				}					
			}
		}
	}

}



static void AutoMachingFPGA(UB num, int proc, double *scr1, double *scr2)
{
	*scr1 = 0.0;
	*scr2 = 0.0;
}
///////////////////////////////////////////////////////////////////////////
//20160902Miya FPGA������ <-
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//20170424Miya �J�������x�Ή� ->
///////////////////////////////////////////////////////////////////////////
#if(CMRSENS)
int RegYubiSideChk(void)
{
	return(0);
}

#endif



#endif