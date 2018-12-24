/**
*	VA-300sプログラム
*
*	@file tsk_ninshou.c
*	@version 1.00
*
*	@author Bionics Co.Ltd T.Nagai
*	@date   2013/12/20
*	@brief  VA-300s 認証処理メインタスク
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

#define HI_KANWA	0	//高得点時減点緩和(FAR危険性あり)
#define X_TRIM_SW	0	//X軸トリミング(速度影響あり)

#define MAX_SIZE_X	160
#define MAX_SIZE_Y	80
#define REC_SIZE_Y	140
#define AUTH_SIZE_X	80
#define AUTH_SIZE_Y	40
#define MAXLIMIT	10000
#define DEV_NUM		4		//分割数=4 
#define X_SEARCH	3		//相関サイズ(X)
#define Y_SEARCH	5		//相関サイズ(Y)
//#define LOW_PART_TH 100
#define LOW_PART_TH 200		//20140423Miya 閾値100→200変更(シミュレータにあわせる)
#define TRIM_ON		1
#define TRIM_X		120
#define TRIM_Y		60
#define MINAUTH_ON	1
#define MIN_SIZE_X	20
#define MIN_SIZE_Y	10

#define LO_END		0		//20160909Miya 認証アップ
//#define HI_END		219		//20160909Miya 認証アップ
#define HI_END		255		//20160909Miya 認証アップ

#define ERR_NONE		0
#define ERR_CAMERA		1	//カメラ異常
#define ERR_MEIRYOU		2	//明瞭異常(明るい)
#define ERR_MEIRYOU2	3	//明瞭異常(暗い)
#define ERR_MEIRYOU3	4
#define ERR_SLIT		5	//スリットあり

#define XSFT_NUM		5	//20140910Miya XSFT (3 or 5を設定、0の場合はOFF)
#define XSFT0			2	//20140910Miya XSFT (XSFT_NUM=3:1 XSFT_NUM=5:2)

#define	AUTH_BASE		0xB8000000				///< 認証画像データのベースアドレス(CS6領域)
#define	AUTHINP_BASE	0xBC000000				///< 認証画像データのベースアドレス(CS7領域)

#define IMG_HDRBUF		0x0C000000

#define IMG_CAPBUF1		0x0C000000
#define IMG_CAPBUF2		0x0C001900
#define IMG_REGBUF		0x0C004000

#define NC_SOUKAN_SGM	813		//20160711Miya NewCmr	
#define NC_SOUKAN_AVE	873		//20160711Miya NewCmr
#define NC_LBP_SGM		8275	//20160711Miya NewCmr	
#define NC_LBP_AVE		8622	//20160711Miya NewCmr

static UB g_RegFlg;
static int g_AuthOkCnt;	//認証OK回数　50回で学習画像保存

static RegUserInfoData			g_RegUserInfoData;				//ユーザ情報
static RegBloodVesselTagData	g_RegBloodVesselTagData[20];	//登録血流画像タグ情報
static UseProcNum 				g_UseProcNum;					//ユーザー使用番号
static PasswordOpen				g_PasswordOpen;					//パスワード開錠
static PasswordOpen				g_InPasswordOpen;				//パスワード開錠
static TechMenuData				g_TechMenuData;					//技術メンテ
static AuthLog					g_AuthLog;						//認証ログ		//20140925Miya add log
static MainteLog				g_MainteLog;					//メンテログ	//20140925Miya add log
static PasswordOpen2			g_PasswordOpen2;				//20160108Miya FinKeyS //パスワード開錠

static struct{
	RegUserInfoData			BkRegUserInfoData;				//ユーザ情報
	RegBloodVesselTagData	BkRegBloodVesselTagData[20];	//登録血流画像タグ情報
	PasswordOpen			BkPasswordOpen;					//パスワード開錠
	TechMenuData			BkTechMenuData;					//技術メンテ	//20140925Miya add Mainte
	AuthLog					BkAuthLog;						//認証ログ		//20140925Miya add log
	MainteLog				BkMainteLog;					//メンテログ	//20140925Miya add log
	PasswordOpen2			BkPasswordOpen2;				//20160108Miya FinKeyS	//パスワード開錠
}g_BkAuthData;

static CapImgData				g_CapImgWorkData;			//画像キャプチャー条件パラメータ実処理
static ImgAndAuthProcSetData	g_ImgAndAuthProcSetData;	//画像＆認証処理パラメータ
static StsImgAndAuthProc		g_StsImgAndAuthProc;		//画像＆認証処理ステータス情報

static RegImgDataAdd			g_RegImgDataAdd[20];		//20160312Miya 極小精度UP 登録画像(R1,LBP,R1極小)

static short g_ImgAve[3];
static int normalize_array[MAX_SIZE_Y][MAX_SIZE_X];

static UB CapImgBuf[2][2][AUTH_SIZE_X*AUTH_SIZE_Y];		//[登録数][ソーベル][認証サイズ]
static UB RegImgBuf[20][2][2][AUTH_SIZE_X*AUTH_SIZE_Y];	//[登録数][学習][ソーベル][認証サイズ]

static UB RegMinImgBuf[20][2][MIN_SIZE_X*MIN_SIZE_Y];	//[登録数][学習][極小認証サイズ]
static double score_buf[2][20];
static double score_buf_cont[2][20];	//20160312Miya 極小精度UP
static double score_buf_yubi[2][20];	//20160312Miya 極小精度UP
static double score_buf_cont2[2][20];	//20160312Miya 極小精度UP
static int auth_turn_num[10];	//20151118Miya 極小認証見直し
static int auth_turn_learn[10];	//20151118Miya 極小認証見直し

volatile static int match_pos[2][DEV_NUM][2];	//[SOBEL][分割][x,y]=[2][4][2]
volatile static int match_score[2][DEV_NUM][2*Y_SEARCH+1][2*X_SEARCH+1];	//[SOBEL][分割][サーチ範囲y][サーチ範囲x]=[2][4][11][7]
volatile static int match_soukan[2][DEV_NUM];	//[SOBEL][分割]=[2][4]
volatile static int m_matrix3[2][DEV_NUM][Y_SEARCH][X_SEARCH];  // 3x3 認証値

//static	UB indata[TRIM_Y][TRIM_X];
//static	UB outdata[TRIM_Y][TRIM_X];
static	UB indata[60][150];		//20140910Miya XSFT
static	UB outdata[60][150];	//20140910Miya XSFT

static int lbp_level[20][2];			//20140905Miya lbp追加 //[登録数][学習]
static double lbp_score[20][2][5];		//20140905Miya lbp追加 //[登録数][学習][分割]
static int lbp_hist_data[2][5][256];	//20140905Miya lbp追加 [比較][分割][データ]
static unsigned short g_contrast_sa[2][16];		//20160312Miya 極小精度UP 16分割コントラスト差分データ [R1/R2][16分割]

static UB g_XsftMiniBuf[5][ 20 * 10 ];	//20140910Miya XSFT

static int GammaLut[256];	//20160810Miya

static UB g_CapMini[ 20 * 10 ];	//20170706Miya FPGA高速化


//20150531Miya
/*
static struct{
	int 	zure_sum_x;		//ずれ量SUM値
	double	ttl_dat_x;		//累計SUM値
	int 	zure_sum_y;		//ずれ量SUM値
	double 	ttl_dat_y;		//累計SUM値
	int		pm_zure_y;		//ずれが+方向、-方向にばらついているか 0:ばらつきなし 1:ばらつきあり
	int		parallel_y;		//平行ずれがMAXかどうか 0:MAXでない 1:MAX
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
static UB TstAuthImgBuf[8][100*40];	//[OK/NG][8個][シフト前サイズ]
unsigned short g_cmr_err;
unsigned short g_imgsv_f;
#endif

TASK NinshouTask( void );
void SendNinshouData( char *data, int cnt );	// データのタスク間送信（汎用・認証処理専用）

void Ninshou_204( UB *data );
void Ninshou_210( UB *data );
static UB Ninshou_ok_proc( void );
static UB Ninshou_ng_proc( void );

static void InitImgAuthPara( void );
static UB InitFlBkAuthArea( void );
static void InitBkAuthDataChar(int num);
static UB InitBkAuthData( void );
static UB SaveBkAuthDataTmpArea( void );
static UB SaveBkAuthDataFl( void );	//画像処理&認証処理バックアップデータ保存
static UB ReadBkAuthData( void );	//画像処理&認証処理バックアップデータ読込

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
static void local_binary_pattern(int sizeX,int sizeY);	//20140905Miya lbp追加
static void make_lbp_hist(int szx, int szy, int num);	//20140905Miya lbp追加

static UB DoAuthProc( UB num );
//static void ChkMaxPara( int *auth_num, int *auth_learn );	//20151118Miya 極小認証見直し
static int ChkMaxPara( int auth_num );	//20151118Miya 極小認証見直し
static int ChkRegNum( UB proc, int num );
static UB ChkScoreLbp( int id_B, int authlow, double sv_score1, double sv_score2, double gentenA, double gentenB, int lbp_lvl );
static UB ChkScoreNewCmr( int id_B, double sv_score1, double sv_score2, double gentenA, double gentenB ); //20160711Miya NewCmr
static UB ChkScore( double score1, double score2 );
static double auto_matching( UB proc, int cap_num, int sv_num, int sbl, int *num );
static double two_matcher7v( int sizex, int sizey, int cap_num, int sv_num, int stx, int edx, int offsetx, int offsety, int sbl, int learn_num);
static double contrast_soukan(int regnum, int learn, int sw);	//20160312Miya 極小精度UP
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
static int two_matcher_lbp(int t_num, int g_num);	//20140905Miya lbp追加
static double lbp_hist_compare(int b_num);			//20140905Miya lbp追加
static int lbp_score_judge( double dat, double dat1, double dat2, double dat3, double dat4 );	//20140905Miya lbp追加
static void make_contrast(int sw);	//20160312Miya 極小精度UP 16分割コントラスト差分作成

static UB SaveRegImgTmpArea( UB proc );
static UB SaveRegImgFlArea( int num );
static UB ReadRegImgArea( int num );
static UB AddRegImgFromRegImg( int sw, int num );	//20160312Miya 極小精度UP
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

//20160902Miya FPGA高速化
static int WrtImgToRam(int ts, int sbl);
static int WrtMiniImgToRam(UB num, int learn, int sbl);
static int Cal4Ave(int ts, int sbl, int *rt1, int *rt2, int *rt3, int *rt4);
void MakeTestImg(void);
static void AutoMachingFPGA(UB num, int proc, double *scr1, double *scr2);

#if(CMRSENS == 1)	//20170424Miya 	カメラ感度対応
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
		ercd = rcv_mbx(MBX_SND_NINSHOU, &msg);	// 認証用データの受信待ち
		if ( ercd == E_OK ){

			dbg_nin_flg = 1;

			// 認証メイン処理
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
		
    	} else {							/* コーディングエラー */
	    	ErrCodeSet( ercd );
    	}
    }
}


//=============================================================================
/**
 *	データのタスク間送信（汎用・認証処理専用）
 *	@param	data　送信データ
 *	@param	cnt　送信バイト数
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
 *	データのタスク間送信（汎用・認証処理専用）
 *	@param	data　送信データ
 *	@param	cnt　送信バイト数
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
 *	データのタスク間送信（汎用・認証処理専用）
 *	@param	data　送信データ
 *	@param	cnt　送信バイト数
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
//	認証OK判定の受信処理
/*==========================================================================*/
static UB Ninshou_ok_proc( void )
{
	UB ercd = 0xff;
		
	// 初期登録モードの場合
	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN405 ) ){	// 認証時「指をセットして下さい..」

		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TSタスクへ画面が「指を抜いて下さい」または、失敗画面になるのを通知。
		if ( ercd != E_OK ){
			nop();			// エラー処理の記述
		}						
	}
	
	if ( GetScreenNo() == LCD_SCREEN6 ){ 	// 登録時の場合。
	
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN7 );	// 	登録「指を抜いて..」画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN7 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_INITIAL );			// 装置モードを初期登録モードへ
		
	} else if ( GetScreenNo() == LCD_SCREEN8 ){	// 認証時「もう一度指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN9 );	// 登録完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN9 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_INITIAL );			// 装置モードを初期登録モードへ
		
#if ( VA300S == 1 || VA300S == 2 )
		send_sio_Touroku();					// 登録データ１件分の錠制御基板への送信。
		MdCngSubMode( SUB_MD_TOUROKU );		// サブモードを、登録データ送信中へ。					
#endif		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	} else if ( GetScreenNo() == LCD_SCREEN405 ){	// 登録時の場合。（１対１認証仕様）
	
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN406 );	// 	登録「指を抜いて..」画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN406 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_INITIAL );			// 装置モードを初期登録モードへ		

	} else if ( GetScreenNo() == LCD_SCREEN407 ){	// 認証時「もう一度指をセットして下さい..」（１対１認証仕様）

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN408 );	// 登録完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN408 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_INITIAL );			// 装置モードを初期登録モードへ
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
	}
	
	// 通常モードの場合	
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN105 ) ){ // 認証の場合。
			
		g_AuthCnt = 0;	//20140423Miya del 認証リトライ追加
#if(AUTHTEST >= 1)	//20150320Miya
		if(g_sv_okcnt < 8)	g_sv_okcnt++;
#endif
#if(AUTHTEST == 3)	//20160810Miya
		g_sv_okcnt = 8;
#endif
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN103 );	// 通常モード（認証）認証完了画面へ。	
		if ( ercd == E_OK ){
			g_AuthType = 0;	//20160120Miya
			ChgScreenNo( LCD_SCREEN103 );			// 画面番号　<-　次の画面
#if ( VA300S == 1 )	//20140905Miya
			send_sio_Ninshou(1, 0, 0);	// 認証OK送信(指認証)
#endif
			MdCngSubMode( SUB_MD_NINSHOU );			// 認証完了送信中（VA300ｓ）
		}
		if ( Pfail_mode_count == 0 ){
			if(dbg_cap_flg == 1){	//20160711Miya NewCmr
				dbg_cap_flg = 2;
			}

			MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
		}	else	{
			MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ
		}
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	//20160108Miya FinKeyS	
	}else if ( ( GetScreenNo() == LCD_SCREEN601 ) || ( GetScreenNo() == LCD_SCREEN602 ) || ( GetScreenNo() == LCD_SCREEN605 ) ){ // 認証の場合。
			
		g_AuthCnt = 0;	//20140423Miya del 認証リトライ追加
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN603 );	// 通常モード（認証）認証完了画面へ。	
		if ( ercd == E_OK ){
			g_AuthType = 0;	//20160120Miya
			ChgScreenNo( LCD_SCREEN603 );			// 画面番号　<-　次の画面
#if ( VA300S == 1 )	//20140905Miya				
			send_sio_Ninshou(1, 0, ode_oru_sw);	// 認証OK送信(指認証)
#endif
			MdCngSubMode( SUB_MD_NINSHOU );			// 認証完了送信中（VA300ｓ）
		}
		if ( Pfail_mode_count == 0 ){ 
			if(dbg_cap_flg == 1){	//20160711Miya NewCmr
				dbg_cap_flg = 2;
			}

			MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
		}	else	{
			MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ
		}
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	//20160108Miya FinKeyS	
	}else if ( ( GetScreenNo() == LCD_SCREEN610 ) || ( GetScreenNo() == LCD_SCREEN611 ) ){ // 認証の場合。
			
		g_AuthCnt = 0;	//20140423Miya del 認証リトライ追加
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN603 );	// 通常モード（認証）認証完了画面へ。	
		if ( ercd == E_OK ){
			g_AuthType = 0;	//20160120Miya
			ChgScreenNo( LCD_SCREEN603 );			// 画面番号　<-　次の画面
#if ( VA300S == 1 )	//20140905Miya
			send_sio_Ninshou(1, 0, ode_oru_sw);					// 認証OK送信(シリアル)
#endif
			MdCngSubMode( SUB_MD_NINSHOU );			// 認証完了送信中（VA300ｓ）
		}
		if ( Pfail_mode_count == 0 ){ 
			MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
		}	else	{
			MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ
		}
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
	} else if ( GetScreenNo() == LCD_SCREEN503 ){ // 認証の場合。（１対１認証仕様）
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN504 );	// 通常モード（認証）認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN504 );			// 画面番号　<-　次の画面
		}
		if ( Pfail_mode_count == 0 ){ 
			MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
		}	else	{
			MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ
		}
					
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	} else if ( GetScreenNo() == LCD_SCREEN121 ){ // 通常モード（登録・責任者の確認）の場合。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN122 );	// 登録時の責任者認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN122 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	} else if ( GetScreenNo() == LCD_SCREEN523 ){ // 通常モード（登録・責任者の確認）の場合。（１対１認証仕様）
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN524 );	// 登録時の責任者認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN524 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	}
	
	if ( ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN530 ) ){	// 通常モード（登録）「指をセットして...」画面
			
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TSタスクへ画面が「指を抜いて下さい」または、失敗画面になるのを通知。
		if ( ercd != E_OK ){
			nop();			// エラー処理の記述
		}			
	}
	
	if ( GetScreenNo() == LCD_SCREEN127 ){	// 通常モード（登録）の場合。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN128 );	// 「指を抜いて下さい」画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN128 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ			

	} else if ( GetScreenNo() == LCD_SCREEN129 ){	// 通常モード（登録）「もう一度指をセットして...」画面
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN130 );	// 認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN130 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
#if ( VA300S == 1 || VA300S == 2 )
		send_sio_Touroku();					// 登録データ１件分の錠制御基板への送信。
		MdCngSubMode( SUB_MD_TOUROKU );		// サブモードを、登録データ送信中へ。					
#endif				
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	} else if ( GetScreenNo() == LCD_SCREEN530 ){	// 通常モード（登録）の場合。（１対１認証仕様）
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN531 );	// 「指を抜いて下さい」画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN531 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ			

	} else if ( GetScreenNo() == LCD_SCREEN532 ){	// 通常モード（登録）「もう一度指をセットして...」画面（１対１認証仕様）
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN533 );	// 認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN533 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	} else if ( GetScreenNo() == LCD_SCREEN141 ){	// 削除時の責任者認証完了画面へ。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN142 );	// 認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN142 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	} else if ( GetScreenNo() == LCD_SCREEN544 ){	// 削除時の責任者認証完了画面へ。（１対１認証仕様）
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN545 );	// 認証完了画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN545 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	} else if ( GetScreenNo() == LCD_SCREEN161 ){	// 緊急開錠番号登録時の責任者認証完了画面へ。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN162 );	// 認証完了○画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN162 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	} else if ( GetScreenNo() == LCD_SCREEN181 ){	// パスワード開錠設定時の責任者認証完了画面へ。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN182 );	// 認証完了○画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN182 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
	}
	
	// メンテナンス・モードの場合
	if ( GetScreenNo() == LCD_SCREEN203 ){	// メンテナンス・フル画像送信時「指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN204 );	// 	撮影○画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN204 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_MAINTE );			// 装置モードをメンテナンスモードへ
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
	}
	
	return ercd;

}

/*==========================================================================*/
//	認証NG判定の受信処理
/*==========================================================================*/
static UB Ninshou_ng_proc( void )
{
	UB ercd = 0xff;
							
	// 初期登録モードの場合
	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN405 ) ){
		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TSタスクへ画面が「指を抜いて下さい」または、失敗画面になるのを通知。
		if ( ercd != E_OK ){
			nop();			// エラー処理の記述
		}
	}

	if ( ( GetScreenNo() == LCD_SCREEN6 ) || ( GetScreenNo() == LCD_SCREEN8 ) ){	// 初期モード（登録）「指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN10 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN10 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_INITIAL );			// 装置モードを初期登録モードへ

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	} else 	if ( ( GetScreenNo() == LCD_SCREEN405 ) || ( GetScreenNo() == LCD_SCREEN407 ) ){	// 初期モード（登録）(１対１認証仕様)「指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN409 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN409 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_INITIAL );			// 装置モードを初期登録モードへ

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}
	
	// 通常モードの場合	
	if ( ( GetScreenNo() == LCD_SCREEN101 ) || ( GetScreenNo() == LCD_SCREEN102 ) || ( GetScreenNo() == LCD_SCREEN105 ) ){

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			

		//20140423Miya del 認証リトライ追加 ->
		//ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN104 );	// 認証失敗画面へ。	
		//if ( ercd == E_OK ){
		//	ChgScreenNo( LCD_SCREEN104 );			// 画面番号　<-　次の画面
		//}
		//20140423Miya del 認証リトライ追加 <-

		//20140423Miya 認証リトライ追加 ->
		if( g_AuthCnt >= 1 && g_AuthCnt <= 2){
		//if(0){
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN105 );	// 認証失敗画面へ。	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN105 );			// 画面番号　<-　次の画面
				g_AuthCnt++;
			}
		}else{
#if(AUTHTEST >= 1)	//20160715Miya
			if(g_sv_okcnt < 8)	g_sv_okcnt++;
#endif

			g_AuthCnt = 0;	//20140423Miya del 認証リトライ追加
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN104 );	// 認証失敗画面へ。	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN104 );			// 画面番号　<-　次の画面
			}
		}
		//20140423Miya 認証リトライ追加 <-

		if ( Pfail_mode_count == 0 ){ 
			if(dbg_cap_flg == 1){	//20160711Miya NewCmr
				dbg_cap_flg = 2;
			}
			MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
		}	else	{
			MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ
		}

		//reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			
			
	//20160108miya FinKeyS
	}else if ( ( GetScreenNo() == LCD_SCREEN601 ) || ( GetScreenNo() == LCD_SCREEN602 ) || ( GetScreenNo() == LCD_SCREEN605 ) ){

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			

		if( g_AuthCnt >= 1 && g_AuthCnt <= 2){
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN605 );	// 認証失敗画面へ。	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN605 );			// 画面番号　<-　次の画面
				g_AuthCnt++;
			}
		}else{
			g_AuthCnt = 0;	//20140423Miya del 認証リトライ追加
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN604 );	// 認証失敗画面へ。	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN604 );			// 画面番号　<-　次の画面
			}
		}

		if ( Pfail_mode_count == 0 ){ 
			if(dbg_cap_flg == 1){	//20160711Miya NewCmr
				dbg_cap_flg = 2;
			}

			MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
		}	else	{
			MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ
		}

		//reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			
			
	//20160108miya FinKeyS
	}else if ( ( GetScreenNo() == LCD_SCREEN610 ) || ( GetScreenNo() == LCD_SCREEN611 ) ){

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			

		if( g_AuthCnt >= 1 && g_AuthCnt <= 2){
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN605 );	// 認証失敗画面へ。	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN605 );			// 画面番号　<-　次の画面
				g_AuthCnt++;
			}
		}else{
			g_AuthCnt = 0;	//20140423Miya del 認証リトライ追加
			ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN604 );	// 認証失敗画面へ。	
			if ( ercd == E_OK ){
				ChgScreenNo( LCD_SCREEN604 );			// 画面番号　<-　次の画面
			}
		}

		if ( Pfail_mode_count == 0 ){ 
			MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
		}	else	{
			MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ
		}

		//reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			
	} else if ( GetScreenNo() == LCD_SCREEN503 ){ // 通常モード（認証）(１対１認証仕様)の場合。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN505 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN505 );			// 画面番号　<-　次の画面
		}
		if ( Pfail_mode_count == 0 ){ 
			MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ
		}	else	{
			MdCngMode( MD_PFAIL );			// 装置モードを	停電モードへ
		}

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN121 ){	// 通常モード（登録・責任者確認）の場合。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN123 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN123 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	} else 	if ( GetScreenNo() == LCD_SCREEN523 ){	// 通常モード（登録・責任者確認）（１対１認証仕様）の場合。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN525 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN525 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}

	if ( ( GetScreenNo() == LCD_SCREEN127) || ( GetScreenNo() == LCD_SCREEN530 ) ){	

		ercd = set_flg( ID_FLG_TS, FPTN_WAIT_CHG_SCRN );// TSタスクへ画面が「指を抜いて下さい」または、失敗画面になるのを通知。
		if ( ercd != E_OK ){
			nop();			// エラー処理の記述
		}
	}
				
	if ( ( GetScreenNo() == LCD_SCREEN127 ) || ( GetScreenNo() == LCD_SCREEN129 ) ){	// 通常モード（登録）の場合。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN131 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN131 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			
			
	} else 	if ( ( GetScreenNo() == LCD_SCREEN530 ) || ( GetScreenNo() == LCD_SCREEN532 ) ){	// 通常モード（登録）（１対１認証仕様）の場合。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN534 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN534 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	

		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）			
			
	}
		
	if ( GetScreenNo() == LCD_SCREEN141 ){	// 通常モード（削除）の場合。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN143 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN143 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	} else 	if ( GetScreenNo() == LCD_SCREEN544 ){	// 通常モード（削除）（１対１認証仕様）の場合。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN546 );	// 認証失敗画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN546 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
			
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
			
	}
		
		
	if ( GetScreenNo() == LCD_SCREEN161 ){	// 緊急開錠番号登録時の責任者認証失敗画面へ。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN163 );	// 認証失敗×画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN163 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}

	if ( GetScreenNo() == LCD_SCREEN181 ){	// パスワード開錠設定時の責任者認証失敗画面へ。
			
		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN183 );	// 認証失敗×画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN183 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_NORMAL );			// 装置モードを通常モードへ	
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）

	}

	// メンテナンス・モードの場合
	if ( GetScreenNo() == LCD_SCREEN203 ){	// メンテナンス・フル画像送信「指をセットして下さい..」

		ercd = set_flg( ID_FLG_LCD, FPTN_LCD_SCREEN205 );	// 	撮影×画面へ。	
		if ( ercd == E_OK ){
			ChgScreenNo( LCD_SCREEN205 );			// 画面番号　<-　次の画面
		}
		MdCngMode( MD_MAINTE );			// 装置モードをメンテナンス・モードへ
		
		reload_CAP_Param();			// Gain. FixShutter,IR_LEDのパラメータを、初期要求値に戻す。（CAPのRetry時に変わった可能性がある為。）
	}
	
	return ercd;
}



/*==========================================================================*/
//	画像処理&認証処理パラメータ設定(初期値)
/*==========================================================================*/
static void InitImgAuthPara( void )
{
	int i;
	
	/*画像キャプチャー条件パラメータ*/
	g_CapImgWorkData.PictureSizeX 	= 1280;		//カメラ撮影サイズ（X)
	g_CapImgWorkData.PictureSizeY 	= 720;		//カメラ撮影サイズ（Y)
	g_CapImgWorkData.TrimStPosX 	= 200;		//トリミング開始座標（X)
	g_CapImgWorkData.TrimStPosY 	= 140;		//トリミング開始座標（Y)
	g_CapImgWorkData.TrimSizeX 		= 640;		//トリミング画像サイズ（X)
	g_CapImgWorkData.TrimSizeY 		= 560;		//トリミング画像サイズ（Y)
	g_CapImgWorkData.ResizeMode 	= 2;		//画像圧縮率　0：辺1/2　1：辺1/4　2：辺1/8　3：辺1/1（等倍）
	g_CapImgWorkData.CapSizeX 		= 160;		//キャプチャーサイズ（X)
	g_CapImgWorkData.CapSizeY 		= 140;		//キャプチャーサイズ（Y)
	g_CapImgWorkData.DataLoadFlg 	= 1;		//パラメータのロード先 0：未調整　1：調整済み　2：INI強制

	/*画像＆認証処理パラメータ*/
	g_ImgAndAuthProcSetData.Proc  					= 0;	//登録状況確認用フラグ　0：未登録　1：登録済み　2?：学習画像あり　0xFF：削除
	g_ImgAndAuthProcSetData.InpSizeX  				= 160;	//入力サイズ（X)
	g_ImgAndAuthProcSetData.InpSizeY  				= 80;	//入力サイズ（Y)
	//g_ImgAndAuthProcSetData.IrLedPos[8]  			= ;		//画像分割領域を設定 入力サイズ（X)基準
															//[0]：LED1-LED2間　[1]：LED2-LED3間　[2]：LED3-LED4間
#if ( NEWALGO > 0 )
	g_ImgAndAuthProcSetData.AuthLvlPara 			= 250;	//20140423miya 減点緩和 //認証アルゴの閾値
#else
	//g_ImgAndAuthProcSetData.AuthLvlPara 			= 350;	//認証アルゴの閾値
	g_ImgAndAuthProcSetData.AuthLvlPara 			= 320;	//20150531Miya 位置減点緩和(NEWALGO=1) //認証アルゴの閾値
#endif

#if ( NEWCMR == 1 )
	//g_ImgAndAuthProcSetData.AuthLvlPara 			= 300;	//20160711Miya NewCmr
	g_ImgAndAuthProcSetData.AuthLvlPara 			= 280;	//20160810Miya NewCmr
#endif

	g_ImgAndAuthProcSetData.LearningPara 			= 1;	//学習機能　０：OFF　１：簡易学習　２：通常学習
	g_ImgAndAuthProcSetData.LvlCameraErrHi 			= 239;	//ヒストグラム輝度平均値の判定(カメラエラー上限)
	g_ImgAndAuthProcSetData.LvlCameraErrLo 			= 16;	//ヒストグラム輝度平均値の判定(カメラエラー下限)
	g_ImgAndAuthProcSetData.LvlHistAveHi 			= 159;	//ヒストグラム輝度平均値の判定(高感度画像しきい値)
	g_ImgAndAuthProcSetData.LvlHistAveLo 			= 50;	//ヒストグラム輝度平均値の判定(低感度画像しきい値)
	g_ImgAndAuthProcSetData.LvlMeiryouLoAll 		= 50;	//細線化明瞭度の下限（全体）
	g_ImgAndAuthProcSetData.LvlMeiryouLoPart 		= 15;	//細線化明瞭度の下限（部分）
	g_ImgAndAuthProcSetData.LvlMeiryouHiAll 		= 550;	//細線化明瞭度の上限（全体）
	g_ImgAndAuthProcSetData.LvlMeiryouHiPart 		= 120;	//細線化明瞭度の上限（部分）
	g_ImgAndAuthProcSetData.ThLowPart1 				= 250;	//認証スコアー減点閾値1(250点以下)
	g_ImgAndAuthProcSetData.ThLowPart2 				= 200;	//認証スコアー減点閾値2(200点以下)
	g_ImgAndAuthProcSetData.WeightLowPart1 			= 50;	//認証スコアー減点係数1(1/100値を使用する)
	g_ImgAndAuthProcSetData.WeightLowPart2 			= 0;	//認証スコアー減点係数1(1/100値を使用する)
	g_ImgAndAuthProcSetData.LvlBrightLo 			= 100;	//明るさ判定（暗レベル）閾値
	g_ImgAndAuthProcSetData.LvlBrightLo2 			= 70;	//明るさ判定（暗レベル）画素数
	g_ImgAndAuthProcSetData.LvlBrightHi 			= 200;	//明るさ判定（明レベル）閾値
	g_ImgAndAuthProcSetData.LvlBrightHiNum 			= 1000;	//明るさ判定（明レベル）画素数
	g_ImgAndAuthProcSetData.LvlFingerTop 			= 20;	//指先コントラスト判定閾値(20)
	g_ImgAndAuthProcSetData.LvlSlitCheck 			= 20;	//スリット判定端部レベル(20)
	g_ImgAndAuthProcSetData.NumSlitCheck 			= 20;	//スリット判定端部画素数(20)
	g_ImgAndAuthProcSetData.LvlSlitSensCheck 		= 60;	//スリット判定生体センサー検知レベル(60)
	g_ImgAndAuthProcSetData.StSlitSensCheck 		= 30;	//スリット判定生体センサー検知開始画素(30)
	g_ImgAndAuthProcSetData.EdSlitSensCheck 		= 60;	//スリット判定生体センサー検知終了画素(60)
	g_ImgAndAuthProcSetData.NgImageChkFlg 			= 3;	//画像異常判定(0:なし 1:あり 2:メモリ異常のみ 3:指抜きのみ)
	g_ImgAndAuthProcSetData.WeightXzure 			= 20;	//X方向ずれ減点重み係数(20->0.2)
	g_ImgAndAuthProcSetData.WeightYzure 			= 30;	//Y方向ずれ減点重み係数(30->0.3)
	g_ImgAndAuthProcSetData.WeightYmuki 			= 20;	//Y方向ずれ減点向き係数(20->0.2)
	g_ImgAndAuthProcSetData.WeightYhei 				= 20;	//Y方向ずれ平行ずれ係数(20->0.2)
	g_ImgAndAuthProcSetData.WeightScore400 			= 90;	//認証スコアー400以下減点重み係数(90->0.9)
	g_ImgAndAuthProcSetData.WeightScore300 			= 90;	//認証スコアー300以下減点重み係数(90->0.9)
	g_ImgAndAuthProcSetData.WeightScore200 			= 75;	//認証スコアー200以下減点重み係数(75->0.75)
	g_ImgAndAuthProcSetData.WeightScore200a 		= 25;	//認証スコアー200以下減点係数(25->0.0025)
	g_ImgAndAuthProcSetData.WeightScore200b 		= 25;	//認証スコアー200以下減点切片(25->0.25)
	g_ImgAndAuthProcSetData.WeightXzureSHvy 		= 8;	//スーパーヘビー級X方向ずれ減点重み係数(8->0.08)
	g_ImgAndAuthProcSetData.WeightYzureSHvy 		= 8;	//スーパーヘビー級Y方向ずれ減点重み係数(8->0.08)
	g_ImgAndAuthProcSetData.WeightYmukiSHvy 		= 0;	//スーパーヘビー級Y方向ずれ減点向き係数(100->1)
	g_ImgAndAuthProcSetData.WeightYheiSHvy 			= 20;	//スーパーヘビー級Y方向ずれ平行ずれ係数(20->0.2)
	g_ImgAndAuthProcSetData.WeightScore400SHvy 		= 90;	//スーパーヘビー級認証スコアー400以下減点重み係数(90->0.9)
	g_ImgAndAuthProcSetData.WeightScore300SHvy 		= 90;	//スーパーヘビー級認証スコアー300以下減点重み係数(90->0.9)
	g_ImgAndAuthProcSetData.WeightScore200SHvy 		= 75;	//スーパーヘビー級認証スコアー200以下減点重み係数(75->0.75)
	g_ImgAndAuthProcSetData.WeightScore200aSHvy 	= 25;	//スーパーヘビー級認証スコアー200以下減点係数(25->0.0025)
	g_ImgAndAuthProcSetData.WeightScore200bSHvy 	= 25;	//スーパーヘビー級認証スコアー200以下減点切片(25->0.25)

	/*画像＆認証処理ステータス情報*/
	g_StsImgAndAuthProc.AuthResult 		= 0;		//画像処理＆認証処理の結果 0：正常　1：Retry　2?：異常
	g_StsImgAndAuthProc.RetruCnt 		= 0;		//画像処理＆認証処理のリトライ回数
	g_StsImgAndAuthProc.CmrGain 		= 0;		//カメラゲインの設定 0：なし　1：+補正　-1：-補正
	g_StsImgAndAuthProc.L_ImgSSvalue 	= 0;		//シャッター速度の設定 0：なし　1：+補正　-1：-補正
	g_StsImgAndAuthProc.N_ImgSSvalue 	= 0;		//シャッター速度の設定 0：なし　1：+補正　-1：-補正
	g_StsImgAndAuthProc.H_ImgSSvalue 	= 0;		//シャッター速度の設定 0：なし　1：+補正　-1：-補正
	//g_StsImgAndAuthProc.IrLedSw[8] 		= 0;	//LED点灯設定 0：変更無し　1：点灯　-1：消灯
													//[0]：LED1　[1]：LED2　[2]：LED3　[3]：LED4"
	g_StsImgAndAuthProc.StsErr 			= 0;		//エラー状況把握 0：正常 1?：異常

	g_AuthCnt = 0;	//20140423Miya 認証リトライ回数

	ProcGamma(1.8);	//20160810Miya

/*
	if(g_DipSwCode != 0){
		if( g_DipSwCode == 0x30 ){
			g_ImgAndAuthProcSetData.AuthLvlPara = 300;	//認証アルゴの閾値
		}
	}
*/

}


/*==========================================================================*/
//	画像処理&認証処理バックアップデータ初期化
/*==========================================================================*/
static UB InitFlBkAuthArea( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;

	uwaddr = ADDR_REGAUTHDATA;

	//フラッシュ保存セクション初期化	
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	return(ercd);
}

//20160108Miya FinKeyS
/*==========================================================================*/
//	画像処理&認証処理バックアップデータ初期化(キャラクターコードをスペースに)
//  0:オール 1:名前 2:パスワード 3:パスワード20個
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
//	画像処理&認証処理バックアップデータ初期化
/*==========================================================================*/
static UB InitBkAuthData( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;

	uwaddr = ADDR_REGAUTHDATA;
	//uwSizeIn = sizeof(RegUserInfoData) + sizeof(RegBloodVesselTagData) * 20 + sizeof(PasswordOpen) + sizeof(TechMenuData) + sizeof(AuthLog) + sizeof(MainteLog);	//20140925Miya password_open
	uwSizeIn = sizeof(RegUserInfoData) + sizeof(RegBloodVesselTagData) * 20 + sizeof(PasswordOpen) + sizeof(TechMenuData) + sizeof(AuthLog) + sizeof(MainteLog) + sizeof(PasswordOpen2);	//20160108Miya FinKeyS

	//フラッシュ保存セクション初期化	
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

	//フラッシュ保存セクション書き込み	
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
//	画像処理&認証処理バックアップデータ保存(テンポラリー)
/*==========================================================================*/
static UB SaveBkAuthDataTmpArea( void )
{
	UB	ercd=E_OK;
	volatile UW	uwaddr, uwSize, uwSizeIn;
	unsigned short reg_num;
	int i;

	reg_num = g_RegUserInfoData.RegNum;
	//登録状況確認　0：未登録　1：登録済み
	g_RegUserInfoData.RegSts = 1;
	//ユーザ指登録本数
	if( g_RegBloodVesselTagData[reg_num].RegInfoFlg != 1 ){	//上書き対応(登録済みの場合、合計数はアップしない)
		g_RegUserInfoData.TotalNum += 1;
	}
	
	//登録状況　0：未登録　1：登録済み　2?：学習画像あり　0xFF：削除
	g_RegBloodVesselTagData[reg_num].RegInfoFlg = 1;
	//登録時の撮影感度 0:感度up(ｽｰﾊﾟｰﾍﾋﾞｰ) 1:感度up(ﾍﾋﾞｰ) 2:標準 3:感度down(女性・子供)
	g_RegBloodVesselTagData[reg_num].RegImageSens = g_RegUserInfoData.RegImageSens;
	//登録時4分割の濃度値AVE
	g_RegBloodVesselTagData[reg_num].RegHiDensityDev[0] = g_RegUserInfoData.RegHiDensityDev[0];
	g_RegBloodVesselTagData[reg_num].RegHiDensityDev[1] = g_RegUserInfoData.RegHiDensityDev[1];
	g_RegBloodVesselTagData[reg_num].RegHiDensityDev[2] = g_RegUserInfoData.RegHiDensityDev[2];
	g_RegBloodVesselTagData[reg_num].RegHiDensityDev[3] = g_RegUserInfoData.RegHiDensityDev[3];
	//H/M/Lの濃度値　0:H 1:M 2:L
	g_RegBloodVesselTagData[reg_num].RegDensityAve[0] = g_RegUserInfoData.RegDensityAve[0];
	g_RegBloodVesselTagData[reg_num].RegDensityAve[1] = g_RegUserInfoData.RegDensityAve[1];
	g_RegBloodVesselTagData[reg_num].RegDensityAve[2] = g_RegUserInfoData.RegDensityAve[2];
	//登録時指太さ(未対応) 0:ｽｰﾊﾟｰﾍﾋﾞｰ 1:ﾍﾋﾞｰ 2:標準 3:細い 4:極細
	g_RegBloodVesselTagData[reg_num].RegFingerSize = g_RegUserInfoData.RegFingerSize;

	//極小画像
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
//	画像処理&認証処理バックアップデータ保存(フラッシュ)
/*==========================================================================*/
static UB SaveBkAuthDataFl( void )
{
	UB	ercd=E_OK;
	volatile UW	uwaddr, uwSize, uwSizeIn;


	//フラッシュ保存セクション初期化	
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
	
	//フラッシュ保存セクション初期化	
	//ercd = FlErase(uwaddr);
	//if( ercd != E_OK ){
	//	return(ercd);
	//}

	//フラッシュ保存セクション書き込み	
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
//	画像処理&認証処理バックアップデータ読込
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

	//フラッシュ保存セクション読み込み	
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

			//フラッシュ保存セクション書き込み	
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

		//フラッシュ保存セクション書き込み	
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
		if(g_BkAuthData.BkPasswordOpen2.family_sw == -1){	//20160105Miya FinKeyS バージョンアップ後
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
//	初期化しないエリア(LED位置調整、LCD調整)バックアップデータ保存(フラッシュ)
/*==========================================================================*/
static UB SaveBkDataNoClearFl( void )
{
	UB	ercd=E_OK;
	volatile UW	uwaddr, uwSize, uwSizeIn;

	//フラッシュ保存セクション初期化	
	uwaddr = ADDR_REGAUTHDATA2;
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	memset( &Flbuf[0], 0xffff, 0x10000 );

	uwSizeIn = sizeof(BkDataNoClear);
	memcpy( &Flbuf[0], &g_BkDataNoClear.LedPosiSetFlg, uwSizeIn );

	//フラッシュ保存セクション書き込み	
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
//	初期化しないエリア(LED位置調整、LCD調整)バックアップデータ読込(フラッシュ)
/*==========================================================================*/
static UB ReadBkDataNoClearFl( void )
{
	UB	ercd=E_OK;
	volatile UW	uwaddr, uwSize, uwSizeIn;
	volatile unsigned long chk_size;
	volatile UB chk=1;

	uwaddr = ADDR_REGAUTHDATA2;
	uwSizeIn = sizeof(BkDataNoClear);

	//フラッシュ保存セクション読み込み	
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
//	画像処理&認証処理パラメータ設定
/*==========================================================================*/
static UB SetImgAuthPara( UB *data )
{
	UB rtn = 0;
	UB *recbuf;
	UB ubtmp;
	unsigned short i, tp_num, reg_num;

	recbuf = data;

#if(PCCTRL == 1) //20160930Miya PCからVA300Sを制御する
	//登録ＩＤ
	reg_num = 0;
	g_RegUserInfoData.RegNum = reg_num;
	g_RegBloodVesselTagData[reg_num].RegNum = reg_num;
	//1:登録1回目 2:登録2回目 3:認証時
	ubtmp = *(recbuf + 11) - 0x30;
	g_RegBloodVesselTagData[reg_num].CapNum = (unsigned short)ubtmp;
	//棟番号　"00"～"99"
	g_RegUserInfoData.BlockNum = 0;
	g_RegBloodVesselTagData[reg_num].BlockNum = 0;
	//ユーザーＩＤ(部屋番号)
	//登録者レベル　"0"：監督者　"1"：管理者　"2"：一般者
	g_RegBloodVesselTagData[reg_num].Level = 2;
	//登録指
	ubtmp = *(recbuf + 27) - 0x30;
	tp_num = 10 * (unsigned short)ubtmp;
	ubtmp = *(recbuf + 28) - 0x30;
	tp_num += (unsigned short)ubtmp;
	g_RegBloodVesselTagData[reg_num].RegFinger = tp_num;
	//登録者名前
	for( i = 0 ; i < 24 ; i++){
		g_RegBloodVesselTagData[reg_num].Name[i] = 0x20;
	}
#else
	//登録ＩＤ
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


	//1:登録1回目 2:登録2回目 3:認証時
	ubtmp = *(recbuf + 11) - 0x30;
	g_RegBloodVesselTagData[reg_num].CapNum = (unsigned short)ubtmp;

	//棟番号　"00"～"99"
	ubtmp = *(recbuf + 13) - 0x30;
	tp_num = 10 * (unsigned short)ubtmp;
	ubtmp = *(recbuf + 14) - 0x30;
	tp_num += (unsigned short)ubtmp;
	g_RegUserInfoData.BlockNum = tp_num;
	g_RegBloodVesselTagData[reg_num].BlockNum = tp_num;

	//ユーザーＩＤ(部屋番号)
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

	//登録者レベル　"0"：監督者　"1"：管理者　"2"：一般者
	ubtmp = *(recbuf + 25) - 0x30;
	g_RegBloodVesselTagData[reg_num].Level = (unsigned short)ubtmp;

	//登録指
	ubtmp = *(recbuf + 27) - 0x30;
	tp_num = 10 * (unsigned short)ubtmp;
	ubtmp = *(recbuf + 28) - 0x30;
	tp_num += (unsigned short)ubtmp;
	g_RegBloodVesselTagData[reg_num].RegFinger = tp_num;

	//登録者名前
	for( i = 0 ; i < 24 ; i++){
		ubtmp = *(recbuf + (30+i));
		g_RegBloodVesselTagData[reg_num].Name[i] = ubtmp;
	}
#endif	
	return(rtn);
}


/*==========================================================================*/
//	画像処理&認証処理パラメータ削除
/*==========================================================================*/
static UB DelImgAuthPara( void )
{
	UB rtn = 0;
	UB ubtmp;
	unsigned short i, tp_num, reg_num, kanri_num;


	if( g_RegUserInfoData.TotalNum == 1 ){	//最後の1人の場合は削除NG
		rtn = 1;
		return(rtn);
	}

	kanri_num = 0;
	for(i = 0 ; i < 4 ; i++){
		if( g_RegBloodVesselTagData[i].RegInfoFlg == 1 ){	//登録ありの場合
			kanri_num += 1;
		}
	}

	//if(tp_num <= 1){	//管理者1名しか残っていないので削除禁止
	//	rtn = 1;
	//	return(rtn);
	//}

	//削除ＩＤ
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

	if( kanri_num <= 1 && reg_num < 4 ){ //管理者1名しか残っていないので削除禁止
		rtn = 1;
		return(rtn);
	}


	if( g_RegBloodVesselTagData[reg_num].RegInfoFlg == 0 ){	//未登録の場合
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
//	画像判定処理
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
//	画像判定処理(High画像)
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
//	画像判定処理(Mid画像)
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
//	画像判定処理(Low画像)
/*==========================================================================*/
static UB LowImgHantei( void )
{
	UB rtn = 0, sens = 2;
	
	return(rtn);
}



/*==========================================================================*/
//	画像判定処理(スリット)
/*==========================================================================*/
static UB SlitImgHantei( int *st )
{
	UB rtn = 0, sens = 2;

	return(rtn);
}



/*==========================================================================*/
//	画像判定処理(トリミング)
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
//	画像処理
/*==========================================================================*/
static UB DoImgProc( UB num )
{
	UB rtn = 0;

	return(rtn);
}

///////////////////////////////////
// ガンマ補正  20160603Miya
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
//	HDR合成処理
/*==========================================================================*/
static void HdrGouseiProc(void)
{

}

/*==========================================================================*/
//	縮小処理
/*==========================================================================*/
static void ImgResize4(int num)
{
	
}


//////////////////////////////////////
// Bicubic　縮小
///////////////////////////////////////
static void dat_bicubicS(int sizeX,int sizeY,int out_X,int out_Y)
{
}


/*==========================================================================*/
//	X軸トリミング処理
/*==========================================================================*/
static void TrimXsft(int num)
{
}

/*==========================================================================*/
//	右上ソーベル処理
/*==========================================================================*/
static void sobel_R1_Filter(int sizeX,int sizeY)
{
}


/*==========================================================================*/
//	右下ソーベル処理
/*==========================================================================*/
static void sobel_R2_Filter(int sizeX,int sizeY)
{
}


static void medianFilter7G(int num, int matX,int matY,int sizeX,int sizeY)
{
}

//20140905Miya LBP追加
////////////////////////////////////////////
// LBP(ローカルバイナリーパターン)画像作成
////////////////////////////////////////////
void local_binary_pattern(int sizeX,int sizeY)
{

}

//20140905Miya LBP追加
////////////////////////////////////////////
// LBP(ローカルバイナリーパターン)ヒストグラム作成
////////////////////////////////////////////
static void make_lbp_hist(int szx, int szy, int num)
{
}

//20160312Miya 極小精度UP
////////////////////////////////////////////
// 16 分割コントラスト配列を作成する
////////////////////////////////////////////
static void make_contrast(int sw)
{
}




#if(FPGA_HI == 1)	//20170706Miya 400Finger
/*==========================================================================*/
//	認証処理
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

	if( num == 0 || num == 3 ){	//0:通常認証 3:登録用認証
			memset( &score_buf[0][0], 0, sizeof(double) * 2 * 20 );
			memset( &auth_turn_num[0], 0, sizeof(int) * 10);	//20151118Miya 極小認証見直し
			memset( &auth_turn_learn[0], 0, sizeof(int) * 10);	//20151118Miya 極小認証見直し
			//memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );		//キャプチャー画像->テンプレート0番
			//memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );		//キャプチャー画像->テンプレート0番
			memset( &score_buf_cont[0][0], 0, sizeof(double) * 2 * 20 );	//20160312Miya 極小精度UP
			memset( &score_buf_cont2[0][0], 0, sizeof(double) * 2 * 20 );	//20161031Miya Ver2204

			total = (int)g_RegUserInfoData.TotalNum;
			th_low = (double)g_ImgAndAuthProcSetData.ThLowPart2;	//200

			//20160312Miya 極小精度UP 撮影画像のコントラスト差分算出 
			TrimXsft(XSFT_NUM - XSFT0);	//センタートリミング
			sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
			sobel_R2_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
			make_contrast(0);
			make_contrast(1);

			//20160312Miya 極小精度UP 極小判定　R2だけ　->　R1＆R2
			//R2極小
			memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );		//キャプチャー画像->テンプレート0番
			memcpy( &g_CapMini[0], &CapImgBuf[0][r2][0], MIN_SIZE_X * MIN_SIZE_Y );	//20161115Miya FPGA高速化 forDebug

			medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//極小画像にメディアをかける
			ImgResize4(1);										//リサイズ
			memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );		//キャプチャー画像->テンプレート0番

			g_RegUserInfoData.lbp_pls = 0;	//20140905Miya lbp追加

			//20140905Miya lbp追加 ->
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
			memcpy( &CapImgBuf[0][r1][0], &g_RegBloodVesselTagData[dbg_dbg1].MinAuthImg[r1][0], MIN_SIZE_X * MIN_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
			memcpy( &CapImgBuf[0][r2][0], &g_RegBloodVesselTagData[dbg_dbg1].MinAuthImg[r2][0], MIN_SIZE_X * MIN_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
#endif

			AutoMachingFPGA(num, 1, &score1, &score2);	//20160902Miya FPGA高速化 forDebug
#if( FREEZTEST )
			if(g_FPGA_ErrFlg != 0)	return 1;
#endif

			if( max_score > 0.0 ){

				if(num == 3 ){	//責任者認証
					auth_total = ChkMaxPara( 0 );	//20151118Miya 極小認証見直し
					if(auth_total == 0){
						s_CapResult = CAP_JUDGE_NG;
						return(1);
					}
					if(auth_total == 99){
						s_CapResult = CAP_JUDGE_NG;
						return(99);	//20151118Miya 同画像再撮影
					}
				}else{
#if(AUTHTEST == 0)
					auth_total = ChkMaxPara( g_AuthCnt );	//20151118Miya 極小認証見直し
#else
					auth_total = ChkMaxPara( 1 );	//20151118Miya 極小認証見直し
#endif
					if(auth_total == 0){
						s_CapResult = CAP_JUDGE_NG;
						return(1);
					}
					if(auth_total == 99){
						s_CapResult = CAP_JUDGE_NG;
						return(99);	//20151118Miya 同画像再撮影
					}
				}					

				//20140905Miya lbp追加 見直し、CapImgBuf[0][*][*]は、ループの外に変更				
				//memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//キャプチャー画像->テンプレート0番
				//memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR2Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//キャプチャー画像->テンプレート0番
				//for( i = 0 ; i < 4 ; i++ ){
				//20140423Miya 認証リトライ追加 認証時リトライ回数リセット
				//auth_total = 1;	//20160902Miya FPGA高速化 forDebug
				for( i = 0 ; i < auth_total ; i++ ){
					auth_num_in = auth_turn_num[i];			//20151118Miya 極小認証見直し
					auth_learn_in = auth_turn_learn[i];		//20151118Miya 極小認証見直し

#if( FREEZTEST == 0 )
					//20140910Miya XSFT ->
					if( XSFT_NUM ){
						stx = 0;
						edx = stx + MIN_SIZE_X;
						offsetx = 0;
						offsety = 0;
						max_score_xsft = 0.0;
						max_xsft = 0;
						memcpy( &CapImgBuf[1][r1][0], &g_RegBloodVesselTagData[auth_num_in].MinAuthImg[auth_learn_in][0], MIN_SIZE_X * MIN_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
						for(xsft = 0 ; xsft < XSFT_NUM ; xsft++){
							memcpy( &CapImgBuf[0][r1][0], &g_XsftMiniBuf[xsft][0], MIN_SIZE_X * MIN_SIZE_Y );		//キャプチャー画像->テンプレート0番
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
						local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);	//20140905Miya LBP追加
						make_lbp_hist(AUTH_SIZE_X, AUTH_SIZE_Y, 0);		//20140905Miya LBP追加
					}
					g_PosGenten[0].x_scr1 = score1;
					//20140910Miya XSFT <-

					memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//キャプチャー画像->テンプレート0番
					memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR2Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//キャプチャー画像->テンプレート0番
#else
					memcpy( &CapImgBuf[0][r1][0], &g_RegImgDataAdd[dbg_dbg1].RegR1Img[0][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
					memcpy( &CapImgBuf[0][r2][0], &RegImgBuf[dbg_dbg1][0][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
#endif


					g_PosGenten[0].auth_num_in = auth_num_in;
					g_PosGenten[0].auth_learn_in = auth_learn_in;
					//20140905Miya lbp追加
					//memcpy( &g_ubResizeBuf[0], &RegImgBuf[auth_num_in][auth_learn_in][r1][0], AUTH_SIZE_X * AUTH_SIZE_Y );
					//local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
					//20160312Miya 極小精度UP 起動時・登録時にLBP作成
					memcpy( &g_ubLbpBuf[0], &g_RegImgDataAdd[auth_num_in].RegLbpImg[auth_learn_in][0], AUTH_SIZE_X * AUTH_SIZE_Y );
					make_lbp_hist(AUTH_SIZE_X, AUTH_SIZE_Y, 1);
					lvl = two_matcher_lbp(auth_num_in, 0);
#if(NEWCMR == 0)//20160711Miya NewCmr
					//20160312Miya 極小精度UP 登録認証時、LBP_LVL=0にしない
					if(num == 3 && lvl == 0)	lvl = 1;
					lbp_level[auth_num_in][auth_learn_in] = lvl;
					if( lvl == 99 || lvl == 15 ){
						//rtn = 1;
						rtn = 99;	//20151118Miya 同画像再撮影
						break;
					}
#else
					lvl = 1;
#endif

					if( lvl > 0 ){	//LBP_lvlが0は認証しない
						memcpy( &CapImgBuf[1][r1][0], &g_RegImgDataAdd[auth_num_in].RegR1Img[auth_learn_in][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
						memcpy( &CapImgBuf[1][r2][0], &RegImgBuf[auth_num_in][auth_learn_in][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
//#if( FREEZTEST )
						AutoMachingFPGA(0, 0, &score1, &score2);	//20160902Miya FPGA高速化
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
						//ChkMaxPara( &auth_num_in, &auth_learn_in );	//20151118Miya 極小認証見直し
#else
						rtn = ChkScoreNewCmr(auth_num_in, score1, score2, genten1, genten2); //20160711Miya NewCmr
						if( rtn == 0 ){
							g_RegUserInfoData.xsft = max_xsft;
							break;
						}
#endif
						
					}else{
						//score_buf[auth_learn_in][auth_num_in] = 0.0;	//20151118Miya 極小認証見直し
						//ChkMaxPara( &auth_num_in, &auth_learn_in );
						rtn = 1;
					}
				}
				g_RegUserInfoData.RegNum = auth_num_in;
				g_RegUserInfoData.r1 = (unsigned short)score1;
				g_RegUserInfoData.r2 = (unsigned short)score2;
				g_RegUserInfoData.lbp_lvl = lbp_level[auth_num_in][auth_learn_in];

			}else{
				rtn = 1;	//認証NG
			}
	}else{			//登録認証 num=2
		proc = 1;
		cap_num = 0;	//キャプチャーした画像番号
		sv_num = 1;		//登録されている画像番号
		
		memcpy( &CapImgBuf[0][r1][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//キャプチャー画像->テンプレート0番
		memcpy( &CapImgBuf[0][r2][0], &g_ubSobelR2Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );		//キャプチャー画像->テンプレート0番

		memcpy( &CapImgBuf[1][r1][0], &g_ubSobelR1SvBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
		score1 = auto_matching( proc, cap_num, sv_num, r1, &auth_num );

		memcpy( &CapImgBuf[1][r2][0], &g_ubSobelR2SvBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
		score2 = auto_matching( proc, cap_num, sv_num, r2, &auth_num );

		//20150531Miya 減点なし
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
	
	if( proc == 3 ){	//登録用認証
		lmt = 4;		//責任指4本
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

	//20140423Miya 認証リトライ回数
	//if( g_AuthCnt >= 2 ){
	//	dat = (unsigned short)((double)g_ImgAndAuthProcSetData.AuthLvlPara * 0.914 + 0.5);
	//}else{
	//	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	//}
	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	
	//th = 2.0 * (double)g_ImgAndAuthProcSetData.AuthLvlPara * (double)g_ImgAndAuthProcSetData.AuthLvlPara;
	th = 2.0 * (double)dat * (double)dat;

	if( total_score > th ){
		rtn = 0;	//認証OK
	}else{
		rtn = 1;	//認証NG
	}
	
	return(rtn);
}


//20140905Miya lbp追加
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
	// 両方とも暗い（太い）指の場合 スーパーヘビー級
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
	// 両方とも暗い（太い）指の場合 スーパーヘビー級
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

	//20140423Miya 認証リトライ回数
	//if( g_AuthCnt >= 2 ){
	//	dat = (unsigned short)((double)g_ImgAndAuthProcSetData.AuthLvlPara * 0.914 + 0.5);
	//}else{
	//	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	//}
	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	
	//th = 2.0 * (double)g_ImgAndAuthProcSetData.AuthLvlPara * (double)g_ImgAndAuthProcSetData.AuthLvlPara;
	th = 2.0 * (double)dat * (double)dat;


	if( lbp_lvl >= 1 && lbp_lvl <= 6 )	//LBP LEVEL=1(1～2), 2(3～6)
	{
		if( total_score > th ){
			rtn = 0;	//認証OK
		}else{
			rtn = 1;	//認証NG
		}
	}
	else if( lbp_lvl >= 7 && lbp_lvl <= 10 )	//LBP LEVEL=3(7～10)
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
			rtn = 0;	//認証OK
		}else{
			if((sv_score1*sv_score1 + sv_score2*sv_score2) > 2.0*450 && gentenAB >= th_genten){	//lbp減点緩和(小)
				rtn = 0;	//認証OK
				g_RegUserInfoData.lbp_pls = 1;
			}else{
				rtn = 1;	//認証NG
			}
		}
	}else{	//LBP LEVEL=4(11～12),5(13～15)
		if( total_score > th ){
			rtn = 0;	//認証OK
		}else{
			if((sv_score1*sv_score1 + sv_score2*sv_score2) > 2.0*450){	//lbp減点緩和(大)
				rtn = 0;	//認証OK
				g_RegUserInfoData.lbp_pls = 1;
			}else{
				rtn = 1;	//認証NG
			}
		}
	}

	
	return(rtn);
}

/*==========================================================================*/
//20160711Miya
//	認証判定処理(NewCmr)
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

	score1 = 0.5 * sv_score1 * genten0;	//NewCmrではSCRが高く出る傾向があるので1/2にする
	score2 = 0.5 * sv_score2 * genten0;
	
	g_PosGenten[0].gen1 = genten1;
	g_PosGenten[0].gen2 = genten2;
	total_score = score1 * score1 + score2 * score2;
	
	dat = g_ImgAndAuthProcSetData.AuthLvlPara;
	th = 2.0 * (double)dat * (double)dat;
	//thG = 2.0 * (double)(dat - 50) * (double)(dat - 50);
	thG = 2.0 * 220.0 * 220.0;	//20160810Miya
	
	if( total_score > th ){
		rtn = 0;	//認証OK
	}else{
		if( total_score > thG ){
			total_scoreG = sv_score1 * sv_score1 + sv_score2 * sv_score2;
			//th = 2.0 * 400.0 * 400.0;
			th = 2.0 * 390.0 * 390.0;	//20160810Miya
			if(total_scoreG > th)	rtn = 0;	//認証OK
			else 					rtn = 1;	//認証NG
		}else{
			rtn = 1;	//認証NG
		}
	}

	return(rtn);
}

/*==========================================================================*/
//	2画像の認証処理
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
	dev_sizex = sizex / DEV_NUM;	//4分割 80 / 4 = 20

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
				//スコアー算出
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
//	エリア内認証演算
/*==========================================================================*/
static double two_matcher7v( int sizex, int sizey, int cap_num, int sv_num, int stx, int edx, int offsetx, int offsety, int sbl, int learn_num)
{
	volatile double score;

	score = -1.0;
	return( score );
}

//20160312Miya 極小精度UP
/*==========================================================================*/
//	R1/R2コントラスト差分の類似度を求める regnum:登録番号 learn;登録/学習 sw:R1/R2
/*==========================================================================*/
static double contrast_soukan(int regnum, int learn, int sw)
{
	return 0.0;
}

/*==========================================================================*/
//	認証減点処理
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
//	認証減点処理(NewCmr)
/*==========================================================================*/
static void genten_seq_NewCmr(int id_B, double *gentenA, double *gentenB)
{
	*gentenA=0.0;
	*gentenB=0.0;
	return;	
}



///////////////////////////////////////
//  7GS0 宮内方式　配列計算
///////////////////////////////////////
static void matrix3_calc()
{
}


// 配列計算その２
static void matrix3_calc2(int r,int k)
{
}


//20140423Miya FAR対策
static int matrix3_low_check(void)
{
	return(0);
}



///////////////////////////////////////
// 7GS0 Ｘ方向のずれのチェック
///////////////////////////////////////
static double x_ofst_check(int r)
{
	return 0.0;
}
///////////////////////////////////////
// 7GS0 Y方向のずれのチェック
///////////////////////////////////////
static double y_ofst_check(int r)
{
	return 0.0;
}

///////////////////////////////////////
// 認証制御関数 補助関数
// 高速sqrt関数
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
// LBP類似度計算
// int t_num	登録NUM
// int g_num	学習NUM
///////////////////////////////////////
static int two_matcher_lbp(int t_num, int g_num)
{
	return(1);
}


//20140905 miya
///////////////////////////////////////
// LBPヒストグラム比較
///////////////////////////////////////
static double lbp_hist_compare(int b_num)
{
	return(0.0);
}

//20140905 miya
///////////////////////////////////////
// LBPレベル判定
///////////////////////////////////////
static int lbp_score_judge( double dat, double dat1, double dat2, double dat3, double dat4 )
{
	return(0);
}




/*==========================================================================*/
//	認証画像のFLASH保存
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

	if( proc == 0 ){	//認証時　学習データのみ更新
		//極小画像
		//for( i = 0 ; i < MIN_SIZE_X * MIN_SIZE_Y ; i++ ){
		//	g_RegBloodVesselTagData[num].MinAuthImg[1][i] = g_ubSobelR3Buf[i];
		//}
		memcpy(&g_RegBloodVesselTagData[num].MinAuthImg[1][0], &g_ubSobelR3DbgBuf[0], MIN_SIZE_X * MIN_SIZE_Y);
		g_RegBloodVesselTagData[num].RegFingerSize = g_RegUserInfoData.RegFingerSize;	//20160120Miya 極小判定の重みにかかわる

		//学習用バッファに保存
		//memcpy( &RegImgBuf[num][1][0][0], &CapImgBuf[0][0][0], size );
		if( XSFT_NUM ){
			TrimXsft(XSFT_NUM - g_RegUserInfoData.xsft);
			memcpy( &RegImgBuf[num][1][0][0], &g_ubResizeBuf[0], size );		//20140905Miya lbp追加 R1エリアにHDRを保存 R1は認証時再計算
		}else{
			memcpy( &RegImgBuf[num][1][0][0], &g_ubResizeSvBuf[0], size );		//20140905Miya lbp追加 R1エリアにHDRを保存 R1は認証時再計算
		}
		memcpy( &RegImgBuf[num][1][1][0], &CapImgBuf[0][1][0], size );
		AddRegImgFromRegImg(2, num);		//20160312Miya 極小精度UP 学習のみ
	}else{
		//登録用バッファに保存
		//memcpy( &RegImgBuf[num][0][0][0], &CapImgBuf[0][0][0], size );
		memcpy( &RegImgBuf[num][0][0][0], &g_ubResizeSvBuf[0], size );		//20140905Miya lbp追加 R1エリアにHDRを保存 R1は認証時再計算
		memcpy( &RegImgBuf[num][0][1][0], &CapImgBuf[0][1][0], size );

		//学習用バッファに保存
		//memcpy( &RegImgBuf[num][1][0][0], &CapImgBuf[1][0][0], size );
		memcpy( &RegImgBuf[num][1][0][0], &g_ubResizeSvBuf2[0], size );		//20140905Miya lbp追加 R1エリアにHDRを保存 R1は認証時再計算
		memcpy( &RegImgBuf[num][1][1][0], &CapImgBuf[1][1][0], size );
		AddRegImgFromRegImg(1, num);		//20160312Miya 極小精度UP 登録・学習
	}
}


/*==========================================================================*/
//	認証画像のFLASH保存
/*==========================================================================*/
static UB SaveRegImgFlArea( int num )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;
	
	uwSizeIn = AUTH_SIZE_X * AUTH_SIZE_Y * 2 * 2 * 10;	//10個分

	if( num < 10 ){	//セクション1(1～10)
		uwaddr = ADDR_REGIMG1;
		memcpy( &Flbuf[0], &RegImgBuf[0][0][0][0], uwSizeIn );
	}else{			//セクション2(11～20)
		uwaddr = ADDR_REGIMG2;
		memcpy( &Flbuf[0], &RegImgBuf[10][0][0][0], uwSizeIn );
	}

	//フラッシュ保存セクション初期化	
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	//フラッシュ保存セクション書き込み	
	uwSizeIn = uwSizeIn / 2;							//word単位につきサイズ半減
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

	return(ercd);
}


/*==========================================================================*/
//	認証画像のFLASH読み出し
/*==========================================================================*/
static UB ReadRegImgArea( int num )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;

//テンプレート初期化
	memset( &RegImgBuf[0][0][0][0], 0, sizeof(RegImgBuf) );
	memset( &Flbuf[0], 0, 0x10000 );

	uwSizeIn = AUTH_SIZE_X * AUTH_SIZE_Y * 2 * 2 * 10;	//10個分
	uwSizeIn = uwSizeIn / 2;							//word単位につきサイズ半減

//セクション1(1～10)
	uwaddr = ADDR_REGIMG1;
	//フラッシュ保存セクション読み込み	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	//テンプレートにコピー　0番地～
	memcpy( &RegImgBuf[0][0][0][0], &Flbuf[0], 2*uwSizeIn );


//セクション2(10～20)
	uwaddr = ADDR_REGIMG2;
	//フラッシュ保存セクション読み込み	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	//テンプレートにコピー　10番地～
	memcpy( &RegImgBuf[10][0][0][0], &Flbuf[0], 2*uwSizeIn );

	return(ercd);
}

/*==========================================================================*/
//20160312Miya 極小精度UP
//	FLASH読み出した認証画像(HDR画像)からR1、LBP、R1極小作成 (SW 0:オール 1:登録・学習(NUM指定) 2:学習のみ(NUM指定))
/*==========================================================================*/
static UB AddRegImgFromRegImg( int sw, int num )
{
	UB	ercd=E_OK;
	int i;
	int tou=0, gaku=1, hdr=0, r1=0, r2=1;

	if(sw == 0){
		for(i = 0 ; i < 20 ; i++){
			if( g_RegBloodVesselTagData[i].RegInfoFlg == 1 ){
				//登録エリア
				memcpy( &g_ubResizeBuf[0], &RegImgBuf[i][tou][hdr][0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//LBP
				local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
				memcpy( &g_RegImgDataAdd[i].RegLbpImg[tou][0], &g_ubLbpBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//R1
				sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
				memcpy( &g_RegImgDataAdd[i].RegR1Img[tou][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//R1 16分割コントラスト差分作成
				make_contrast(r1);
				memcpy( &g_RegImgDataAdd[i].RegContrast[tou][r1][0], &g_contrast_sa[r1][0], sizeof(short)*16 );
				//R1極小
				medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//極小画像にメディアをかける
				ImgResize4(1);										//リサイズ
				memcpy( &g_RegImgDataAdd[i].RegMiniR1Img[tou][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );
				//R2 16分割コントラスト差分作成
				memcpy( &g_ubSobelR2Buf[0], &RegImgBuf[i][tou][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
				make_contrast(r2);
				memcpy( &g_RegImgDataAdd[i].RegContrast[tou][r2][0], &g_contrast_sa[r2][0], sizeof(short)*16 );
			
				//学習エリア
				memcpy( &g_ubResizeBuf[0], &RegImgBuf[i][gaku][hdr][0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//LBP
				local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
				memcpy( &g_RegImgDataAdd[i].RegLbpImg[gaku][0], &g_ubLbpBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//R1
				sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
				memcpy( &g_RegImgDataAdd[i].RegR1Img[gaku][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
				//R1 16分割コントラスト差分作成
				make_contrast(r1);
				memcpy( &g_RegImgDataAdd[i].RegContrast[gaku][r1][0], &g_contrast_sa[r1][0], sizeof(short)*16 );
				//R1極小
				medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//極小画像にメディアをかける
				ImgResize4(1);										//リサイズ
				memcpy( &g_RegImgDataAdd[i].RegMiniR1Img[gaku][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );
				//R2 16分割コントラスト差分作成
				memcpy( &g_ubSobelR2Buf[0], &RegImgBuf[i][gaku][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
				make_contrast(r2);
				memcpy( &g_RegImgDataAdd[i].RegContrast[gaku][r2][0], &g_contrast_sa[r2][0], sizeof(short)*16 );
			}
		}
	}else{
		i = num;	//NUM指定
		if(sw == 1){
			//登録エリア
			memcpy( &g_ubResizeBuf[0], &RegImgBuf[i][tou][hdr][0], AUTH_SIZE_X * AUTH_SIZE_Y );
			//LBP
			local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
			memcpy( &g_RegImgDataAdd[i].RegLbpImg[tou][0], &g_ubLbpBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
			//R1
			sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
			memcpy( &g_RegImgDataAdd[i].RegR1Img[tou][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
			//R1 16分割コントラスト差分作成
			make_contrast(r1);
			memcpy( &g_RegImgDataAdd[i].RegContrast[tou][r1][0], &g_contrast_sa[r1][0], sizeof(int)*16 );
			//R1極小
			medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//極小画像にメディアをかける
			ImgResize4(1);										//リサイズ
			memcpy( &g_RegImgDataAdd[i].RegMiniR1Img[tou][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );
			//R2 16分割コントラスト差分作成
			memcpy( &g_ubSobelR2Buf[0], &RegImgBuf[i][tou][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
			make_contrast(r2);
			memcpy( &g_RegImgDataAdd[i].RegContrast[tou][r2][0], &g_contrast_sa[r2][0], sizeof(short)*16 );
		}

		//学習エリア
		memcpy( &g_ubResizeBuf[0], &RegImgBuf[i][gaku][hdr][0], AUTH_SIZE_X * AUTH_SIZE_Y );
		//LBP
		local_binary_pattern(AUTH_SIZE_X, AUTH_SIZE_Y);
		memcpy( &g_RegImgDataAdd[i].RegLbpImg[gaku][0], &g_ubLbpBuf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
		//R1
		sobel_R1_Filter(AUTH_SIZE_X, AUTH_SIZE_Y);
		memcpy( &g_RegImgDataAdd[i].RegR1Img[gaku][0], &g_ubSobelR1Buf[0], AUTH_SIZE_X * AUTH_SIZE_Y );
		//R1 16分割コントラスト差分作成
		make_contrast(r1);
		memcpy( &g_RegImgDataAdd[i].RegContrast[gaku][r1][0], &g_contrast_sa[r1][0], sizeof(short)*16 );
		//R1極小
		medianFilter7G(1, 3, 3, AUTH_SIZE_X, AUTH_SIZE_Y);	//極小画像にメディアをかける
		ImgResize4(1);										//リサイズ
		memcpy( &g_RegImgDataAdd[i].RegMiniR1Img[gaku][0], &g_ubSobelR3Buf[0], MIN_SIZE_X * MIN_SIZE_Y );
		//R2 16分割コントラスト差分作成
		memcpy( &g_ubSobelR2Buf[0], &RegImgBuf[i][gaku][r2][0], AUTH_SIZE_X * AUTH_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
		make_contrast(r2);
		memcpy( &g_RegImgDataAdd[i].RegContrast[gaku][r2][0], &g_contrast_sa[r2][0], sizeof(short)*16 );
	}

	return(ercd);
}

/*==========================================================================*/
//	認証画像のFLASH初期化
/*==========================================================================*/
static UB InitRegImgArea( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn;
	int i;
	//UH	uhData[ 8 ];
	//unsigned long i, j, cnt;


//テンプレート初期化
	memset( &RegImgBuf[0][0][0][0], 0, sizeof(RegImgBuf) );
	memset( &Flbuf[0], 0, 0x10000 );

//セクション1(1～10)
	uwaddr = ADDR_REGIMG1;
	//フラッシュ保存セクション初期化

	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	
	//フラッシュ用退避バッファにコピー
	uwSizeIn = AUTH_SIZE_X * AUTH_SIZE_Y * 2 * 2 * 10;
	memcpy( &Flbuf[0], &RegImgBuf[0][0][0][0], uwSizeIn );
	uwSizeIn = uwSizeIn / 2;
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

//セクション2(11～20)
	uwaddr = ADDR_REGIMG2;
	//フラッシュ保存セクション初期化

	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	//フラッシュ用退避バッファにコピー
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
//OKセクション
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

		//フラシュ初期化
		ercd = E_OK;//FlErase(uwaddr);
		if( ercd != E_OK ){
			return(ercd);
		}

		//保存エリアに0設定
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
//	テスト用認証画像のFLASH保存 
/*==========================================================================*/
static UB InitTestRegImgFlArea( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn, ofst;
	short i, lt, cnt;

	g_sv_okcnt = 0;
//OKセクション
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

		//フラシュ初期化
		ercd = E_OK;//FlErase(uwaddr);
		if( ercd != E_OK ){
			return(ercd);
		}

		//保存エリアに0設定
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
//	テスト用認証画像のFLASH保存 
/*==========================================================================*/
static UB SaveTestRegImgFlArea( unsigned short ok_ng_f )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn, ofst;
	short i, lt;

	return(0);	//20180406Miya ForSH

	if(ok_ng_f == 0){	//OKエリア
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
			//20160810Miya ループ対応
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

	//フラッシュ保存セクション初期化	
	ercd = E_OK;//FlErase(uwaddr);
	if( ercd != E_OK ){
		return(ercd);
	}

	//フラッシュ保存セクション書き込み	
	uwSizeIn = uwSizeIn / 2;							//word単位につきサイズ半減
	uwSize = E_OK;//FlWrite(uwaddr, &Flbuf[0], uwSizeIn);
	if( uwSize != ((uwSizeIn << 1) & 0xffff) ){
		ercd = 1;
		return(ercd);
	}

	return(ercd);
}

/*==========================================================================*/
//	テスト用認証画像のFLASH読み出し
/*==========================================================================*/
static UB ReadTestRegImgArea( unsigned short ok_ng_f, short cpy_f, short num, short num10 )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn, ofst;

	return(0);	//20180406Miya ForSH

//テンプレート初期化
	memset( &Flbuf[0], 0, 0x10000 );

	uwSizeIn = sizeof(short) + 100 * 40 * 32;
	uwSizeIn = uwSizeIn / 2;							//word単位につきサイズ半減

	if( cpy_f == 1 ){	//データ取得
		if(ok_ng_f == 0){	//OKエリア
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
	}else{				//フラッシュ書き込み前に読み込む
		if(ok_ng_f == 0){	//OKエリア
			if( g_sv_okcnt0 < 32 ){
				uwaddr = ADDR_OKIMG1;
			}else if( g_sv_okcnt0 < 64 ){
				uwaddr = ADDR_OKIMG2;
			}else if( g_sv_okcnt0 < 96 ){
				uwaddr = ADDR_OKIMG3;
			}else if( g_sv_okcnt0 < 128 ){	//20160810Miya ループ対応
				uwaddr = ADDR_OKIMG4;
			}else{
				uwaddr = ADDR_OKIMG1;
			}		
		}
	}

	//フラッシュ保存セクション読み込み	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}

	if( cpy_f == 1 ){	//データ取得
		ofst = ofst / 2;
		if(ok_ng_f == 0){	//OKエリア
			memcpy( &TstAuthImgBuf[0][0], &Flbuf[ofst], 100 * 40 * 8 );
		}
	}


	return(ercd);
}

/*==========================================================================*/
//	テスト用認証画像カウンターのFLASH読み出し
/*==========================================================================*/
static UB ReadTestRegImgCnt( void )
{
	UB	ercd=E_OK;
	UW	uwaddr, uwSize, uwSizeIn, ofst;

//テンプレート初期化
	memset( &Flbuf[0], 0, 0x10000 );

	uwSizeIn = sizeof(short);
	uwSizeIn = uwSizeIn / 2;							//word単位につきサイズ半減

	uwaddr = ADDR_OKIMG1;
	//フラッシュ保存セクション読み込み	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	memcpy( &g_sv_okcnt1, &Flbuf[0], sizeof(short) );

	uwaddr = ADDR_OKIMG2;
	//フラッシュ保存セクション読み込み	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	memcpy( &g_sv_okcnt2, &Flbuf[0], sizeof(short) );

	uwaddr = ADDR_OKIMG3;
	//フラッシュ保存セクション読み込み	
	ercd = FlRead(uwaddr, &Flbuf[0], uwSizeIn);
	if (ercd != E_OK) {
		return(ercd);
	}
	memcpy( &g_sv_okcnt3, &Flbuf[0], sizeof(short) );

	uwaddr = ADDR_OKIMG4;
	//フラッシュ保存セクション読み込み	
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
		case 0:	//オール1の位
			rnum1 = (sum1 % 10) * 1000 + (sum2 % 10) * 100 + (sum3 % 10) * 10 + (sum4 % 10);
			break;
		case 1:	//オール10の位
			rnum1 = 0;
			rnum1 += 1000 * GetKeta(sum1, 10);
			rnum1 += 100 * GetKeta(sum2, 10);
			rnum1 += 10 * GetKeta(sum3, 10);
			rnum1 += GetKeta(sum4, 10);
			break;
		case 2:	//オール100の位
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
		case 1:	//オール1の位
			rnum2 = (sum1 % 10) * 1000 + (sum2 % 10) * 100 + (sum3 % 10) * 10 + (sum4 % 10);
			break;
		case 2:	//オール10の位
			rnum2 = 0;
			rnum2 += 1000 * GetKeta(sum1, 10);
			rnum2 += 100 * GetKeta(sum2, 10);
			rnum2 += 10 * GetKeta(sum3, 10);
			rnum2 += GetKeta(sum4, 10);
			break;
		case 3:	//オール100の位
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

	if( (num0[0] == num0[1]) && (num0[2] == num0[3]) && (num0[0] == num0[2]) ){	//4文字同じ場合(4文字同じことはありえないようにする)
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

	if( (num0[4] == num0[5]) && (num0[6] == num0[7]) && (num0[4] == num0[6]) ){	//4文字同じ場合(4文字同じことはありえないようにする)
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

	if( s_CapResult == CAP_JUDGE_RT ){//最撮影(暗い)
		irDuty2 = 255;		
		irDuty3 = 255;
		irDuty4 = 255;
		irDuty5 = 0;
		cmrGain = ini_cmrGain + 2;
		g_CmrParaSet = 1;
		ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_GAIN );	// カメラタスクに、直接のゲイン設定値を設定を依頼（FPGAを経由しない）
	}
	if( s_CapResult == CAP_JUDGE_RT2 ){//最撮影(明るい)
		irDuty2 = 255;		
		irDuty3 = 255;
		irDuty4 = 128;
		irDuty5 = 0;
		cmrFixShutter1 = 5;
		cmrFixShutter2 = 6;
		cmrFixShutter3 = 7;
		g_CmrParaSet = 1;
		ercd = set_flg( ID_FLG_CAMERA, FPTN_SETREQ_SHUT1 );	// カメラタスクに、直接の露出３設定値を設定を依頼（FPGAを経由しない）
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
		ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP204 );			// カメラ撮影+登録処理（コマンド204）へ。
	}else{
		ercd = set_flg( ID_FLG_CAMERA, FPTN_START_CAP211 );			// 認証用撮影処理（コマンド210）へ。
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
	
	if(dat1 == 0x10){		//あ
		if(dat2 < 5){
			dat0 = 0xB1;
			dat0 = dat0 + dat2;
		}else if(dat2 < 10){
			dat0 = 0xA7;
			dat0 = dat0 + (dat2 - 5);
		}
		k = 1;
	}else if(dat1 == 0x20){	//か
		dat0 = 0xB6;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x30){	//さ
		dat0 = 0xBB;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x40){	//た
		if(dat2 < 5){
			dat0 = 0xC0;
			dat0 = dat0 + dat2;
		}else if(dat2 == 5){
			dat0 = 0xAF;
		}
		k = 1;
	}else if(dat1 == 0x50){	//な
		dat0 = 0xC5;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x60){	//は
		dat0 = 0xCA;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x70){	//ま
		dat0 = 0xCF;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0x80){	//や
		if(dat2 < 3){
			dat0 = 0xD4;
			dat0 = dat0 + dat2;
		}else if(dat2 < 6){
			dat0 = 0xAC;
			dat0 = dat0 + (dat2 - 5);
		}
		k = 1;
	}else if(dat1 == 0x90){	//ら
		dat0 = 0xD7;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 1;
	}else if(dat1 == 0xA0){	//わ
		if(dat2 == 0){
			dat0 = 0xDC;
		}else if(dat2 == 1){
			dat0 = 0xA6;
		}else if(dat2 == 2){
			dat0 = 0xDD;
		}
		k = 1;
	}else if(dat1 == 0xB0){	//が
		dat0 = 0xB6;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 2;
	}else if(dat1 == 0xC0){	//ざ
		dat0 = 0xBB;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 2;
	}else if(dat1 == 0xD0){	//だ
		if(dat2 < 5){
			dat0 = 0xC0;
			dat0 = dat0 + dat2;
		}
		k = 2;
	}else if(dat1 == 0xE0){	//ば
		dat0 = 0xCA;
		if(dat2 < 5){
			dat0 = dat0 + dat2;
		}
		k = 2;
	}else if(dat1 == 0xF0){	//ぱ
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
//20160902Miya FPGA高速化 ->
///////////////////////////////////////////////////////////////////////////
// ts 0:撮影画像 1:登録画像 sbl 0:R1 1:R2
static int WrtImgToRam(int ts, int sbl)
{
	int x, y, i, cnt, rtn, bun;
	volatile unsigned long	pixdata, pix, addr;
	double sum, ave;
	
	rtn = 0;

	if(ts == 0){	//固定側(撮影画像)
		if(sbl == 0)	addr = 0xC00000;	//R1
		else			addr = 0xE00000;	//R2
	}else{			//ずらし側(登録画像)
		if(sbl == 0)	addr = 0xD00000;	//R1
		else			addr = 0xF00000;	//R2
	}

	//メモリに画像を4分割して書込む
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
				pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[登録][R1][画素]
				sum += (double)pix;
				pixdata = pixdata | pix;		
				//2pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[登録][R1][画素]
				sum += (double)pix;
				pixdata = pixdata | pix;		
				//3pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[登録][R1][画素]
				sum += (double)pix;
				pixdata = pixdata | pix;		
				//4pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[登録][R1][画素]
				sum += (double)pix;
				pixdata = pixdata | pix;		
	
				*(volatile unsigned long *)(CAP_BASE + addr + 4 * (unsigned long)i++) = pixdata;
			}
		}
	}
	
	
/*	メモリに画像を一面書込む
	for(y = 0 ; y < AUTH_SIZE_Y ; y++){			//AUTH_SIZE_Y = 80
		for(x = 0 ; x < AUTH_SIZE_X / 4 ; x++){		//AUTH_SIZE_X = 40
			pixdata = 0;
			//1pix
			pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[登録][R1][画素]
			sum += (double)pix;
			pixdata = pixdata | pix;		
			//2pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[登録][R1][画素]
			sum += (double)pix;
			pixdata = pixdata | pix;		
			//3pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[登録][R1][画素]
			sum += (double)pix;
			pixdata = pixdata | pix;		
			//4pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[ts][sbl][cnt++];	//[登録][R1][画素]
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
	addr1 = 0xC00000;	//ずらし側(登録画像)
	addr2 = 0xD00000;	//固定側(撮影画像)

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
			memcpy( &CapImgBuf[1][sbl][0], &g_RegBloodVesselTagData[proc_num].MinAuthImg[learn][0], MIN_SIZE_X * MIN_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
		else			//R1
			memcpy( &CapImgBuf[1][sbl][0], &g_RegImgDataAdd[proc_num].RegMiniR1Img[learn1][0], MIN_SIZE_X * MIN_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
#else	//20170315Miya 400Finger
		if(sbl == 1)	//R2
			memcpy( &CapImgBuf[1][sbl][0], &g_RegBloodVesselTagData[proc_num].MinAuthImg[sbl][0], MIN_SIZE_X * MIN_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
		else			//R1
			memcpy( &CapImgBuf[1][sbl][0], &g_RegBloodVesselTagData[proc_num].MinAuthImg[sbl][0], MIN_SIZE_X * MIN_SIZE_Y );	//保存画像(登録1回目)->テンプレート1番
#endif
		cnt1 = 0;		
		for(y = 0 ; y < MIN_SIZE_Y ; y++){			//AUTH_SIZE_Y = 10
			for(x = 0 ; x < MIN_SIZE_X / 4 ; x++){		//AUTH_SIZE_X = 20
				pixdata = 0;
				//1pix
				pix = (unsigned long)CapImgBuf[1][sbl][cnt1++];	//[登録][R1][画素]
				pixdata = pixdata | pix;		
				//2pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[1][sbl][cnt1++];	//[登録][R1][画素]
				pixdata = pixdata | pix;		
				//3pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[1][sbl][cnt1++];	//[登録][R1][画素]
				pixdata = pixdata | pix;		
				//4pix
				pixdata = (pixdata << 8);
				pix = (unsigned long)CapImgBuf[1][sbl][cnt1++];	//[登録][R1][画素]
				pixdata = pixdata | pix;		
	
				*(volatile unsigned long *)(CAP_BASE + addr1 + 4 * (unsigned long)cnt2++) = pixdata;
			}
		}
	}

	cnt2 = 0;
	cnt1 = 0;		
	//memcpy( &CapImgBuf[0][sbl][0], &g_CapMini[0], MIN_SIZE_X * MIN_SIZE_Y );		//キャプチャー画像->テンプレート0番
	for(y = 0 ; y < MIN_SIZE_Y ; y++){			//AUTH_SIZE_Y = 10
		for(x = 0 ; x < MIN_SIZE_X / 4 ; x++){		//AUTH_SIZE_X = 20
			pixdata = 0;
			//1pix
			pix = (unsigned long)CapImgBuf[0][sbl][cnt1++];	//[登録][R1][画素]
			pixdata = pixdata | pix;		
			//2pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[0][sbl][cnt1++];	//[登録][R1][画素]
			pixdata = pixdata | pix;		
			//3pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[0][sbl][cnt1++];	//[登録][R1][画素]
			pixdata = pixdata | pix;		
			//4pix
			pixdata = (pixdata << 8);
			pix = (unsigned long)CapImgBuf[0][sbl][cnt1++];	//[登録][R1][画素]
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
			CapImgBuf[0][0][cnt] = 128;		//撮影　R1
			CapImgBuf[0][1][cnt] = 128;		//撮影　R2
			CapImgBuf[1][0][cnt] = 128;		//登録　R1
			CapImgBuf[1][1][cnt] = 128;		//登録　R2
			++cnt;
		}
	}

	for(i = 0 ; i < 4 ; i++){
		for(y = 0 ; y < 40 ; y++){
			for(x = 0 ; x < 20 ; x++){
				if( x >= 5 && x < 15 && y >= 10 && y < 30){
					adr = y * 80 + 20 * i + x;
					//CapImgBuf[0][0][adr] = 64;		//撮影　R1
					//CapImgBuf[0][1][adr] = 20;		//撮影　R2
					//CapImgBuf[0][1][adr] = 64;		//撮影　R2
					CapImgBuf[0][0][adr] = 200;		//撮影　R1
					CapImgBuf[0][1][adr] = 200;		//撮影　R2
					if(i==1 || i==2){
						CapImgBuf[0][0][adr] = 220;		//撮影　R1
						CapImgBuf[0][1][adr] = 220;		//撮影　R2
					}
				}					
				if( x >= 5 && x < 15 && y >= 10 && y < 30){
					adr = y * 80 + 20 * i + x;
					//CapImgBuf[1][0][adr] = 64;		//登録　R1
					//CapImgBuf[1][1][adr] = 20;		//登録　R2
					//CapImgBuf[1][1][adr] = 64;		//登録　R2
					CapImgBuf[1][0][adr] = 200;		//登録　R1
					CapImgBuf[1][1][adr] = 200;		//登録　R2
					if(i==1 || i==2){
						CapImgBuf[1][0][adr] = 220;		//登録　R1
						CapImgBuf[1][1][adr] = 220;		//登録　R2
					}
				}					

				if( x >= 5 && x < 15 && y >= 20 && y < 30){
					adr = y * 80 + 20 * i + x;
					//CapImgBuf[0][0][adr] = 64;		//撮影　R1
					//CapImgBuf[0][1][adr] = 20;		//撮影　R2
					//CapImgBuf[0][1][adr] = 64;		//撮影　R2
					CapImgBuf[0][0][adr] = 64;		//撮影　R1
					CapImgBuf[0][1][adr] = 64;		//撮影　R2
				}					
				if( x >= 5 && x < 15 && y >= 20 && y < 30){
					adr = y * 80 + 20 * i + x;
					//CapImgBuf[1][0][adr] = 64;		//登録　R1
					//CapImgBuf[1][1][adr] = 20;		//登録　R2
					//CapImgBuf[1][1][adr] = 64;		//登録　R2
					CapImgBuf[1][0][adr] = 32;		//登録　R1
					CapImgBuf[1][1][adr] = 32;		//登録　R2
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
//20160902Miya FPGA高速化 <-
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//20170424Miya カメラ感度対応 ->
///////////////////////////////////////////////////////////////////////////
#if(CMRSENS)
int RegYubiSideChk(void)
{
	return(0);
}

#endif



#endif