/**
*	VA-300テストプログラム
*
*	@file tsk_lcd.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/28
*	@brief  LCD表示タスク
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "version.h"

#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"

#include "va300.h"
#include "drv_buz.h"

//#define LCDDEBUG	//LCDデバッグ用宣言
#define LCDX	480
#define LCDY	272

#define LCDBACKLIGHTON	1
#define LCDBACKLIGHTOFF	0

#define INITMODE10KEYGMNBUF	1
#define GMN10KEYCOADE 10
#define GMNMKEYCOADE 11
#define GMNERRMESCODE 20
#define GMNKEYHEADCODE 30

#define MOJICODEAGYOU	1
#define MOJICODEKAGYOU	2
#define MOJICODESAGYOU	3
#define MOJICODETAGYOU	4
#define MOJICODENAGYOU	5
#define MOJICODEHAGYOU	6
#define MOJICODEMAGYOU	7
#define MOJICODEYAGYOU	8
#define MOJICODERAGYOU	9
#define MOJICODEWAGYOU	10
#define MOJICODEGAGYOU	11
#define MOJICODEZAGYOU	12
#define MOJICODEDAGYOU	13
#define MOJICODEBAGYOU	14
#define MOJICODEPAGYOU	15
#define MOJICODECAN		0xff
#define MOJICODEDEL		0xfe
#define MOJICODERTN		0xfd
#define MOJICODECSR		0xfc
#define MOJIKODEDAKUTN	0xfb


#define KEY1	0
#define KEY2	1
#define KEY3	2
#define KEYCAN	3
#define KEY4	4
#define KEY5	5
#define KEY6	6
#define KEYDEL	7
#define KEY7	8
#define KEY8	9
#define KEY9	10
#define KEYRTN	11
#define KEY0	12
#define KEYSP1	13
#define KEYKIGOU	13
#define KEYSP2	14
#define KEYCSR	14
#define KEYSP3	15


enum sousa{
	GMN_NOSOUSA,	//0:何もしない
	GMN_MENUSOUSA,	//1:メニュー選択
	GMN_YNSOUSA,	//2:Yes/No
	GMN_SELSOUSA,	//3:要素選択
	GMN_KEYSOUSA,	//4:キー操作
	GMN_WAITSOUSA,	//5:タイマー操作
	GMN_DEFSOUSA	//6:その他
};




// 変数定義
static ID s_idTsk;

// プロトタイプ宣言
static TASK LcdTask( void );		///< LCD表示タスク


// タスクの定義
const T_CTSK ctsk_lcd = { TA_HLNG, NULL, LcdTask, 7, 2048, NULL, (B *)"lcd task" };//

UB LcdProcMain(int buf_num, int gmn_num, UINT *size, UB *msg);
UB TouchKeyProc(int buf_num, int num, UINT *msize, UB *msg);
UB KeyInputNumSousa( int gmn, int buf, int *keta, int btn );
UB KeyInputMojiSousa( int buf, int *hit, int *keta, int btn );
UB KeyPageSousa( int *page, int btn );
void SetVerNum(int buf_num);
void SetErrMes(int buf_num, int mes_num);
void SetKeyHeadMes(int buf_num, int mes_num);
void SetKinkyuNum1(int buf_num, int keta, int key);
void SetKinkyuNum2(int buf_num, int keta, int key);
void SetKeyNum(int page, int keta, int key);
void SetKeyMoji(int buf_num, int moji, int hit, int keta, int key);
void SetInfo(int page);
void SetGmnToLcdBuff(int buf_num, int gmn_num, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
UH GetPixDataFromInputGmn(int buf, int sub_num1, int sub_num2, unsigned long cnt);


UB WaitKeyProc(int sousa, int tflg, UB *msg);
void SetGamen01(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGamen02(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
UH GetPixDataFromGamen01(int buf, unsigned long cnt);
UH GetPixDataFromGamen02(int buf, unsigned long cnt);
void SetGmnToLcdBuff(int buf_num, int gmn_num, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGmnToLcdBuff02(int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGmnToLcdBuff03(int offset, int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
UINT DbgSendMessageToMain(int num, int sousa, UB *msg);

extern const unsigned short BtnPosType1[4][2];
extern const unsigned short BtnPosType2[10][2];
extern const unsigned short BtnPosType3[14][2];
extern const unsigned short BtnPosType4[14][2];
extern const unsigned short BtnPosType5[4][2];
extern const unsigned short BtnPosType6[32][2];
extern const unsigned short BtnPosType7[4][2];
extern const unsigned short InpPosKey[16][2];
extern const unsigned short InpPos1[8][2];
extern const unsigned short InpPos2[8][2];
extern const unsigned short InpPos3[4][2];

extern const unsigned short LcdGmn000[];
extern const unsigned short LcdGmn001[];
extern const unsigned short LcdGmn002[];
extern const unsigned short LcdGmn101[];
extern const unsigned short LcdGmn102[];
extern const unsigned short LcdGmn103[];
extern const unsigned short LcdGmn104[];
extern const unsigned short LcdGmn105[];
extern const unsigned short LcdGmn106[];
extern const unsigned short LcdGmn107[];
extern const unsigned short LcdGmn108[];
extern const unsigned short LcdGmn109[];
extern const unsigned short LcdGmn201[];
extern const unsigned short LcdGmn202[];
extern const unsigned short LcdGmn203[];
extern const unsigned short LcdGmn204[];
extern const unsigned short LcdGmn205[];
extern const unsigned short LcdGmn206[];
extern const unsigned short LcdGmn207[];
extern const unsigned short LcdGmn208[];
extern const unsigned short LcdGmn209[];
extern const unsigned short LcdGmn301[];
extern const unsigned short LcdGmn302[];
extern const unsigned short LcdGmn303[];
extern const unsigned short LcdGmnDataNum[10][2560];
extern const unsigned short LcdGmnDataAgyou[10][2560];
extern const unsigned short LcdGmnDataKAgyou[5][2560];
extern const unsigned short LcdGmnDataSAgyou[5][2560];
extern const unsigned short LcdGmnDataTAgyou[6][2560];
extern const unsigned short LcdGmnDataNAgyou[5][2560];
extern const unsigned short LcdGmnDataHAgyou[5][2560];
extern const unsigned short LcdGmnDataMAgyou[5][2560];
extern const unsigned short LcdGmnDataYAgyou[6][2560];
extern const unsigned short LcdGmnDataRAgyou[5][2560];
extern const unsigned short LcdGmnDataWAgyou[6][2560];
extern const unsigned short LcdGmnDataGAgyou[5][2560];
extern const unsigned short LcdGmnDataZAgyou[5][2560];
extern const unsigned short LcdGmnDataDAgyou[5][2560];
extern const unsigned short LcdGmnDataBAgyou[5][2560];
extern const unsigned short LcdGmnDataPAgyou[5][2560];
extern const unsigned short LcdBoardMes01[];
extern const unsigned short LcdBoardMes02[];
extern const unsigned short LcdBoardMes03[];

extern const unsigned short MesPosition[2][2];
extern const unsigned short KeyHeadMesPosition[2][2];
extern const unsigned short LcdMes001[];
extern const unsigned short kinkyuNumPos1[16][2];
extern const unsigned short kinkyuNumPos2[16][2];


extern const unsigned short InpGmnMigi[];
extern const unsigned short InpGmnHidari[];
extern const unsigned short InpGmnYubi01[];
extern const unsigned short InpGmnYubi02[];
extern const unsigned short InpGmnYubi03[];
extern const unsigned short InpGmnYubi04[];
extern const unsigned short InpGmnYubi05[];

extern struct{
	unsigned int LcdMsgSize;
	UB LcdMsgBuff[1024];
}g_LcdmsgData;

//extern static UB	s_CapResult;		// 指認証の結果



int sv_reg_num;
UB	sv_keyindat[8];
int sv_mkey;
UB	init_flg;	//初期駆動フラグ
UB sv_yubi_seq_no[4];
UB sv_yubi_no[3];

//extern T_YBDATA yb_touroku_data;	// 指登録情報（１指分）


/*==========================================================================*/
/**
 * LCD表示タスク初期化
 *
 * @param idTsk タスクID
 * @retval E_OK 正常起動
 */
/*==========================================================================*/
ER LcdTaskInit(ID idTsk)
{
	ER ercd;
	
	// タスクの生成
	if (idTsk > 0) {
		ercd = cre_tsk(idTsk, &ctsk_lcd);
		if (ercd == E_OK) {
			s_idTsk = idTsk;
		}
	} else {
		ercd = acre_tsk(&ctsk_lcd);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// タスクの起動
	ercd = sta_tsk(s_idTsk, 0);
	
	return ercd;
}

/*==========================================================================*/
/**
 * LCD表示タスク
 * 
 */
/*==========================================================================*/
static TASK LcdTask( void )
{
	FLGPTN	flgptn;
	ER		ercd;
	UB	sts_key;
	UB	perm_enb, perm_r, perm_g, perm_b;
	UH	st_x, st_y, ed_x, ed_y;
	int buf_num, gmn_num;
	
	int cnt, num;
	int posx, posy;
	UB	gmn_snd_flg;	//画面転送確認フラグ
	
	/** メッセージ・バッファ用の宣言  **/
	static UB *msg; 		// <-- サイズは120くらいまでの範囲で適宜変えて下さい。
	static UINT msg_size;


	// LCDの機器Initialize は、ここに記入。
	perm_enb = 0;	//透過色設定 0:未使用 1:使用
	perm_r = 0;		//R透過色
	perm_g = 0;		//G透過色
	perm_b = 0;		//B透過色
	st_x = 0;
	st_y = 0;
	ed_x = 479;
	ed_y = 271;
	gmn_snd_flg = 0;

	msg = &g_LcdmsgData.LcdMsgBuff[0];
	init_flg = 0;
	
	
	// 通常処理開始
	for(;;) {
		// メインタスクからのの受信待ち
		ercd = wai_flg( ID_FLG_LCD, ( FPTN_LCD_INIT				// LCD初期画面表示要求(メイン→LCD、ノーマルモード移行の時)
									| FPTN_LCD_SCREEN1			// 画面１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN2			// 画面２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN3			// 画面３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN4			// 画面４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN5			// 画面５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN6			// 画面６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN7			// 画面７表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN8			// 画面８表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN9			// 画面９表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN10			// 画面１０表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN11			// 画面１１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN12			// 画面１２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN100 		// 画面１００表示要求(メイン→LCD)　通常モード
									| FPTN_LCD_SCREEN101 		// 画面１０１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN102 		// 画面１０２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN103 		// 画面１０３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN104 		// 画面１０４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN120		// 画面１２０表示要求(メイン→LCD)　通常モード（登録）
									| FPTN_LCD_SCREEN121		// 画面１２１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN122		// 画面１２２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN123		// 画面１２３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN124		// 画面１２４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN125		// 画面１２５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN126		// 画面１２６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN127		// 画面１２７表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN128		// 画面１２８表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN129		// 画面１２９表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN130		// 画面１３０表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN131		// 画面１３１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN132		// 画面１３２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN140		// 画面１４０表示要求(メイン→LCD)　通常モード（削除）
									| FPTN_LCD_SCREEN141		// 画面１４１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN142		// 画面１４２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN143		// 画面１４３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN144		// 画面１４４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN145		// 画面１４５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN146		// 画面１４６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN160		// 画面１４０表示要求(メイン→LCD)　通常モード（削除）
									| FPTN_LCD_SCREEN161		// 画面１４１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN162		// 画面１４２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN163		// 画面１４３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN164		// 画面１４４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN165		// 画面１４５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN180		// 画面１２０表示要求(メイン→LCD)　通常モード（登録）
									| FPTN_LCD_SCREEN181		// 画面１２１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN182		// 画面１２２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN183		// 画面１２３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN184		// 画面１２４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN185		// 画面１２５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN186		// 画面１２６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN187		// 画面１２７表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN188		// 画面１２８表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN200		// 画面２００表示要求(メイン→LCD)　メンテナンス・モード
									| FPTN_LCD_SCREEN201		// 画面２０１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN202		// 画面２０２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN203		// 画面２０３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN204		// 画面２０４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN205 ), TWF_ORW, &flgptn );	// 画面２０５表示要求(メイン→LCD)
		if ( ercd != E_OK ){
			break;
		}
														
		// 以下に、Switch文などで受信内容に沿った処理を記述
		// この時、処理内容の最初に、
		//		ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_INIT );		// フラグのクリア
		// 		if ( ercd != E_OK ){
		//			break;
		//		}
		//  などで、受信したフラグ毎に、クリア処理を行う。
		//  wai_flg() で使用するフラグパターンは、id.h　を参照のこと。
		// 


		switch(flgptn)
		{
// イニシャライズ ------------------------------------------------------------------------------------//
			case FPTN_LCD_INIT:
				gmn_num = 0;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_INIT );
				if ( ercd != E_OK ){
					break;
				}

				LcdcBackLightOff();
				SetGmnToLcdBuff(0, 0, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcBackLightOn();
				LcdcDisplayModeSet(0, 0);
				init_flg = 1;

				break;
// 初期モード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN1:
				gmn_num = 1;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN1 );
				if ( ercd != E_OK ){
					break;
				}

				if( init_flg == 1 ){//初期駆動の場合、画面バッファを設定する
					for(cnt = 1 ; cnt < 13 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, gmn_num + cnt -1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					init_flg = 0;
				}else{
					cnt = 1;	//画面1のみ再設定(キ－入力印字のため)
					SetGmnToLcdBuff(cnt, gmn_num + cnt -1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}					

				break;
			case FPTN_LCD_SCREEN2:
				gmn_num = 2;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN2 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN3:
				gmn_num = 3;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN3 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN4:
				gmn_num = 4;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN4 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN5:
				gmn_num = 5;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN5 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN6:
				gmn_num = 6;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN6 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN7:
				gmn_num = 7;
				buf_num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN7 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN8:
				gmn_num = 8;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN8 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN9:
				gmn_num = 9;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN9 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN10:
				gmn_num = 10;
				buf_num = 10;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN10 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN11:
				gmn_num = 11;
				buf_num = 11;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN11 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN12:
				gmn_num = 12;
				buf_num = 12;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN12 );
				if ( ercd != E_OK ){
					break;
				}
				break;
// 通常モード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN100:
				gmn_num = 100;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN100 );
				if ( ercd != E_OK ){
					break;
				}

				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(0, 0);
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt < 5 ; cnt++){	//画面を画面バッファに転送する
					//SetGamen01(cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					SetGmnToLcdBuff(cnt, gmn_num + cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				//chg_pri(TSK_SELF, TPRI_INI);

				break;
			case FPTN_LCD_SCREEN101:
				gmn_num = 101;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN101 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN102:
				gmn_num = 102;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN102 );
				if ( ercd != E_OK ){
					break;
				}
				LcdcBackLightOff();
				break;
			case FPTN_LCD_SCREEN103:
				gmn_num = 103;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN103 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN104:
				gmn_num = 104;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN104 );
				if ( ercd != E_OK ){
					break;
				}
				break;
// 通常モード(登録) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN120:
				gmn_num = 120;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN120 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 2 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//SetGmnToLcdBuff(cnt, (gmn_num + cnt + 1), 0, 0, 0, 0, 0, 0, 479, 271);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
				gmn_snd_flg = 0;

				break;
			case FPTN_LCD_SCREEN121:
				gmn_num = 121;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN121 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN122:
				gmn_num = 122;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN122 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN123:
				gmn_num = 123;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN123 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN124:
				gmn_num = 124;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN124 );
				if ( ercd != E_OK ){
					break;
				}
			
				if( gmn_snd_flg == 0 ){
					//chg_pri(TSK_SELF, TMIN_TPRI);
					for(cnt = 3 ; cnt <= 9 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, 120 + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					SetGmnToLcdBuff(15, 120 + 15 + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//chg_pri(TSK_SELF, TPRI_INI);
					//gmn_snd_flg = 1;
				}

				break;
			case FPTN_LCD_SCREEN125:
				gmn_num = 129;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN125 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN126:
				gmn_num = 130;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN126 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN127:
				gmn_num = 131;
				buf_num = 10;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN127 );
				if ( ercd != E_OK ){
					break;
				}

				if( gmn_snd_flg == 0 ){
					chg_pri(TSK_SELF, TMIN_TPRI);
					for(cnt = 10 ; cnt <= 14 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, 120 + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					chg_pri(TSK_SELF, TPRI_INI);
					gmn_snd_flg = 1;
				}

				break;
			case FPTN_LCD_SCREEN128:
				gmn_num = 132;
				buf_num = 11;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN128 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN129:
				gmn_num = 133;
				buf_num = 12;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN129 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN130:
				gmn_num = 134;
				buf_num = 13;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN130 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN131:
				gmn_num = 135;
				buf_num = 14;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN131 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN132:
				gmn_num = 136;
				buf_num = 15;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN132 );
				if ( ercd != E_OK ){
					break;
				}
				break;
// 通常モード(削除) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN140:
				gmn_num = 140;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN140 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);

				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 2 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
				gmn_snd_flg = 0;

				break;
			case FPTN_LCD_SCREEN141:
				gmn_num = 141;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN141 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN142:
				gmn_num = 142;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN142 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN143:
				gmn_num = 143;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN143 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN144:
				gmn_num = 144;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN144 );
				if ( ercd != E_OK ){
					break;
				}

				if( gmn_snd_flg == 0 ){
					//chg_pri(TSK_SELF, TMIN_TPRI);
					for(cnt = 3 ; cnt <= 9 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, 140 + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					SetGmnToLcdBuff(15, 120 + 15 + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//chg_pri(TSK_SELF, TPRI_INI);
					gmn_snd_flg = 1;
				}
				break;
			case FPTN_LCD_SCREEN145:
				gmn_num = 149;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN145 );
				if ( ercd != E_OK ){
					break;
				}
				//SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//デモのみ
				break;
			case FPTN_LCD_SCREEN146:
				gmn_num = 150;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN146 );
				if ( ercd != E_OK ){
					break;
				}
				break;
// 通常モード(緊急解錠番号設定) ------------------------------------------------------------------------------------//

			case FPTN_LCD_SCREEN160:
				gmn_num = 160;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN160 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
/*
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 4 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
*/
				gmn_snd_flg = 0;

				break;
			case FPTN_LCD_SCREEN161:
				gmn_num = 161;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN161 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN162:
				gmn_num = 162;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN162 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN163:
				gmn_num = 163;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN163 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN164:
				gmn_num = 164;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN164 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 1);
				break;
			case FPTN_LCD_SCREEN165:
				gmn_num = 165;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN165 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;

// 通常モード(緊急解錠) ------------------------------------------------------------------------------------//

			case FPTN_LCD_SCREEN180:
				gmn_num = 180;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN180 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
/*
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 7 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
*/
				gmn_snd_flg = 0;

				break;
			case FPTN_LCD_SCREEN181:
				gmn_num = 181;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN181 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				for(cnt = 0 ; cnt < 8 ; cnt++){
					if(kinkyuu_tel_no[cnt] == 0x20){
						num = 10;
					}else{
						num = kinkyuu_tel_no[cnt] - 0x30;
						if(num < 0 || num >9 ){
							num = 10;
						}
					}
					
					SetKinkyuNum1(buf_num, cnt+1, num);
				}
				for(cnt = 0 ; cnt < 8 ; cnt++){
					if(kinkyuu_tel_no[cnt+8] == 0x20){
						num = 10;
					}else{
						num = kinkyuu_tel_no[cnt+8] - 0x30;
						if(num < 0 || num >9 ){
							num = 10;
						}
					}
					
					SetKinkyuNum2(buf_num, cnt+1, num);
				}
				
				break;
			case FPTN_LCD_SCREEN182:
				gmn_num = 182;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN182 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN183:
				gmn_num = 183;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN183 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);

				for(cnt = 0 ; cnt < 8 ; cnt++){
					if(kinkyuu_hyouji_no[cnt] == 0x20){
						num = 10;
					}else{
						num = kinkyuu_hyouji_no[cnt] - 0x30;
						if(num < 0 || num >9 ){
							num = 10;
						}
					}
					
					SetKinkyuNum1(buf_num, cnt+1, num);
				}

				break;
			case FPTN_LCD_SCREEN184:
				gmn_num = 184;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN184 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 2);
				break;
			case FPTN_LCD_SCREEN185:
				gmn_num = 185;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN185 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 1);
				break;
			case FPTN_LCD_SCREEN186:
				gmn_num = 186;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN186 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN187:
				gmn_num = 187;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN187 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN188:
				gmn_num = 188;
				buf_num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN188 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;

// メンテナンスモード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN200:
				gmn_num = 200;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN200 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
/*
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 2 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//SetGmnToLcdBuff(cnt, (gmn_num + cnt + 1), 0, 0, 0, 0, 0, 0, 479, 271);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
				gmn_snd_flg = 0;
*/

				break;
			case FPTN_LCD_SCREEN201:
				gmn_num = 201;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN201 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 3);
				SetVerNum(buf_num);
				break;
			case FPTN_LCD_SCREEN202:
				gmn_num = 202;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN202 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN203:
				gmn_num = 203;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN203 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN204:
				gmn_num = 204;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN204 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN205:
				gmn_num = 205;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN205 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN206:
				gmn_num = 206;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN206 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			default:
				break;
		}

		if( gmn_num != 0 ){
			memset( msg, 0x20, sizeof(msg) );
			sts_key = LcdProcMain(buf_num, gmn_num, &msg_size, msg);
			g_LcdmsgData.LcdMsgSize = msg_size;
			
			if(sts_key == 1){	//キー押下あり
				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}

				//////// メッセージ送信 ////////
				//msg_size = DbgSendMessageToMain(num, 0, msg);
				//ercd = snd_mbf( MBF_LCD_DATA, ( VP )&msg, msg_size );
				//if(ercd != E_OK){
				//	nop();
					//エラ－処理要
				//}
			}					
		}
			
	}

	PrgErrSet();
	slp_tsk();					//ここに来るのは、システム・エラー
}


UB LcdProcMain(int buf_num, int gmn_num, UINT *size, UB *msg)
{
	UB	sts_key;
	int i, cnt;
	int posx, posy;
	UINT msg_size;
	UB *buf;

	buf = msg;

	//////// 画面表示 ////////
	//if(gmn_num == 1 || gmn_num == 100 || gmn_num == 120 || gmn_num == 140 || gmn_num == 200 ){
	if( gmn_num == 100 || gmn_num == 120 || gmn_num == 140 || gmn_num == 200 ){
		nop();
	}else{
		if(gmn_num == 3 || gmn_num == 124 || gmn_num == 144 ){
			SetInfo(buf_num);
		}
		if(gmn_num == 10 || gmn_num == 104 || gmn_num == 123 || gmn_num == 135 || gmn_num == 143 ){
			if(s_CapResult == CAP_JUDGE_E1){	//エラーメッセージ表示ありの場合
				SetErrMes(buf_num, 1);
			}else{
				SetErrMes(buf_num, 0);
			}
		}
		if(gmn_num == 4 || gmn_num == 129 ){	//ひらがなキ－
			for(i=0;i< 8;i++){
				SetKeyMoji(buf_num, MOJICODEDEL, 0, i+1, 0);
			}
		}
		
		LcdcDisplayModeSet(buf_num, 0);
	}

	///////// キー入力待ち ////////
	sts_key = TouchKeyProc(buf_num, gmn_num, &msg_size, buf);
	*size = msg_size;
	return(sts_key);
}


UB TouchKeyProc(int buf_num, int num, UINT *msize, UB *msg)
{
	UB	rtn=0;
	UB	rtnsts=0;
	UB	lcd_sw=LCDBACKLIGHTON;
	UB	proc;	//0:なし 1:複数ボタン選択(中止あり) 2:YES/NOボタン 3:複数ボタン選択(中止あり) 4:キーボード入力 5:タイマー待ち 6:強制キーあり
	ER	ercd;
	unsigned short pos[32][2];
	int x1[16], x2[16], y1[16], y2[16];
	int posx, posy;
	int i, j, btn_num, btn_press, dat;
	int cnt;
	int tcnt=0;
	int page, keta, hit;
	UB *buf;

	buf = msg;
	memset( x1, 0, sizeof(int) * 16 );
	memset( x2, 0, sizeof(int) * 16 );
	memset( y1, 0, sizeof(int) * 16 );
	memset( y2, 0, sizeof(int) * 16 );
	btn_num = 0;
	btn_press = 0;

	switch(num)
	{
		case 1:
		case 164:
		case 182:
		case 184:
		case 185:
		case 201:
			proc = GMN_KEYSOUSA;
			btn_num = 16;
			keta = 1;
			memcpy( &pos[0][0], &BtnPosType6[0][0], sizeof(short) * btn_num * 4 );
			memset( &sv_keyindat[0], 0x20, 8);
			break;
		case 4:
		case 129:
			proc = GMN_KEYSOUSA;
			btn_num = 16;
			keta = 1;
			hit = 0;
			sv_mkey = 0xff;
			memcpy( &pos[0][0], &BtnPosType6[0][0], sizeof(short) * btn_num * 4 );
			memset( &sv_keyindat[0], 0, 8);
			break;
//		case 129:
//			proc = GMN_DEFSOUSA;
//			break;
		case 2:
			proc = GMN_MENUSOUSA;
			//btn_num = 2;
			btn_num = 1;	//メンテボタン無効
			memcpy( &pos[0][0], &BtnPosType1[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 3:
//20130610_Miya 画像データ採取実施フラグ
//#ifdef GETIMG20130611
//			proc = GMN_DEFSOUSA;
//			btn_press = 1;
//#else
			proc = GMN_SELSOUSA;
			//btn_num = 7;
			btn_num = 5;	//←→ボタン無効
			memcpy( &pos[0][0], &BtnPosType3[0][0], sizeof(short) * btn_num * 4 );
//#endif
			break;
		case 5:
		case 130:
			proc = GMN_SELSOUSA;
			btn_num = 7;
			memcpy( &pos[0][0], &BtnPosType4[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 7:
			BuzTplSet(0x87, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();
			proc = GMN_NOSOUSA;
			break;	
		case 9:
		case 134:
//20130610_Miya 画像データ採取実施フラグ
#ifdef GETIMG20130611
			BuzTplSet(0x87, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();
#endif
			proc = GMN_WAITSOUSA;
			break;
		case 10:
		case 135:
//20130610_Miya 画像データ採取実施フラグ
#ifdef GETIMG20130611
			BuzTplSet(0xB0, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();
#endif
			if(s_CapResult == CAP_JUDGE_E1){	//エラーメッセージ表示ありの場合
				dly_tsk((2500/MSEC));	//タイマー2.5sec(時間追加)
				//エラーメッセージ表示クリアー
			}
			proc = GMN_WAITSOUSA;
			break;
		case 103:
		case 122:
		case 142:
		case 162:
		case 186:
		case 204:
			BuzTplSet(0x87, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();
			proc = GMN_WAITSOUSA;
			break;
		case 104:
		case 123:
		case 143:
		case 163:
		case 187:
		case 205:
			BuzTplSet(0xB0, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();
			if(s_CapResult == CAP_JUDGE_E1){	//エラーメッセージ表示ありの場合
				dly_tsk((2500/MSEC));	//タイマー2.5sec(時間追加)
				//エラーメッセージ表示クリアー
			}
			proc = GMN_WAITSOUSA;
			break;
		case 11:
		case 12:
		case 136:
		case 149:
		case 150:
		case 165:
		case 188:
		case 206:
			proc = GMN_YNSOUSA;
			btn_num = 2;
			memcpy( &pos[0][0], &BtnPosType5[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 100:
		case 120:
		case 140:
		case 160:
		case 180:
		case 200:
			proc = GMN_DEFSOUSA;
			break;
		case 101:
			proc = GMN_MENUSOUSA;
			lcd_sw = LCDBACKLIGHTON;
			btn_num = 5;
			//btn_num = 2;	//緊急番号設定、緊急解錠ボタン無効
			memcpy( &pos[0][0], &BtnPosType2[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 102:
			proc = GMN_MENUSOUSA;
			lcd_sw = LCDBACKLIGHTOFF;
			btn_num = 5;
			//btn_num = 3;	//緊急番号設定、緊急解錠ボタン無効
			memcpy( &pos[0][0], &BtnPosType2[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 124:
		case 144:
			proc = GMN_SELSOUSA;
			page = 3;
			btn_num = 7;
			//btn_num = 6;	//←ボタン無効
			memcpy( &pos[0][0], &BtnPosType3[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 125:
		case 126:
		case 127:
		case 128:
		case 145:
		case 146:
		case 147:
		case 148:
			proc = GMN_SELSOUSA;
			btn_num = 7;
			memcpy( &pos[0][0], &BtnPosType3[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 181:
		case 183:
			proc = GMN_SELSOUSA;
			btn_num = 2;
			memcpy( &pos[0][0], &BtnPosType7[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 202:
			proc = GMN_MENUSOUSA;
			btn_num = 5;
			memcpy( &pos[0][0], &BtnPosType2[0][0], sizeof(short) * btn_num * 4 );
			break;
		default:
			proc = GMN_NOSOUSA;
			break;	
	}


	if(proc == GMN_NOSOUSA ){
		rtn = 0;
		return(rtn);
	}
	else
	{
		if( proc == GMN_MENUSOUSA || proc == GMN_YNSOUSA || proc == GMN_SELSOUSA || proc == GMN_KEYSOUSA )
		{
			while(1){
				if( num == 101 || num == 102 ){
					ercd = TplPosGet(&posx, &posy, (500/MSEC));	//キー待ち(500msec)
					if(tcnt < 120 ){	//60sec判定(60sec以内の場合)
						++tcnt;
					}else{
						if(lcd_sw == LCDBACKLIGHTON){
							LcdcBackLightOff();
							LcdcDisplayModeSet(2, 0);
							lcd_sw = LCDBACKLIGHTOFF;
							ercd = E_TMOUT;		//キー操作に反応しないようにするため
						}
						if(	ercd == E_OK ){
							BuzTplSet(0x87, 1);
							dly_tsk((50/MSEC));	//50msecのウエイト(タッチキーの反応解除用)
							BuzTplOff();
							dly_tsk((250/MSEC));	//250msecのウエイト(タッチキーの反応解除用)
							LcdcDisplayModeSet(1, 0);
							LcdcBackLightOn();
							lcd_sw = LCDBACKLIGHTON;
							tcnt = 0;
							ercd = E_TMOUT;		//キー操作に反応しないようにするため
						}
					}
				}else{
					ercd = TplPosGet(&posx, &posy, TMO_FEVR);	//キー待ち(押されるまで)
				}

				if(	ercd == E_OK ){
					//dly_tsk((500/MSEC));	//500msecのウエイト(タッチキーの反応解除用)
					for(i = 0, j = 0 ; i < btn_num ; i++, j+=2 ){
						x1[i] = pos[j][0];
						y1[i] = pos[j][1];
						x2[i] = pos[j+1][0];
						y2[i] = pos[j+1][1];
						if( (posx > x1[i] && posx < x2[i]) && (posy > y1[i] && posy < y2[i]) ){
							btn_press = i;
					
							BuzTplSet(0x87, 1);
							dly_tsk((50/MSEC));	//50msecのウエイト(タッチキーの反応解除用)
							BuzTplOff();
							dly_tsk((200/MSEC));	//200msecのウエイト(タッチキーの反応解除用)
							if( (num >= 124 && num <= 128) || (num >= 144 && num <= 148)){
								rtn = KeyPageSousa( &page, btn_press );
								if( rtn == 1 ){
									break;	//CHG_REQ要求
								}
							}else{
								if( proc == GMN_KEYSOUSA ){
									if( num == 1 || num == 201 || num == 164 || num == 182 || num == 184 || num == 185 ){	//テンキ－操作の場合
										rtn = KeyInputNumSousa( num, buf_num, &keta, btn_press );
										if( btn_press == KEYCAN && num != 1 ){
											rtn = 1;
										}
									}else if( num == 4 || num == 129 ){	//ひらがなキ－
										rtn = KeyInputMojiSousa( buf_num, &hit, &keta, btn_press );
									}
									if( rtn == 1 ){
										break;	//CHG_REQ要求
									}
								}else{
									if( num == 202 && (btn_press == 1 || btn_press == 2 || btn_press == 3) ){//メンテメニュー未対応ボタン
										nop();
									}else{
										rtn = 1;	//CHG_REQ要求
									}
									break;
								}
							}
						}
					}
					if(rtn == 1){
						break;
					}else{
						//BuzTplSet(0xB0, 1);
						//dly_tsk((50/MSEC));	//50msecのウエイト(タッチキーの反応解除用)
						//BuzTplOff();
						dly_tsk((100/MSEC));	//100msecのウエイト(タッチキーの反応解除用)
					}
				}

				if( num == 101 || num == 102 ){
					rtnsts = MdGetMode();		//指挿入チェック
					if( rtnsts == MD_CAP ){		//指挿入ありの場合
						rtn = 2;	//CHG_REQなし
						LcdcBackLightOn();
						return(rtn);
						//break;
					}
				}
			}
		}
		else
		{
			if( proc == GMN_WAITSOUSA ){
				rtn = 1;
				if( num != 122 ){//登録時の画面バッファ転送時間対策(画面122の場合ウエイトしない)
					dly_tsk((2500/MSEC));	//タイマー2.5sec
				}
			}else {	//GMN_DEFSOUSA
				rtn = 1;
			}
		}
	}

	cnt = 0;
	switch(num)
	{
		case 1:
			*(buf + cnt++) = ( UB )LCD_USER_ID;
			*(buf + cnt++) = sv_keyindat[0];
			*(buf + cnt++) = sv_keyindat[1];
			*(buf + cnt++) = sv_keyindat[2];
			*(buf + cnt++) = sv_keyindat[3];
			//*(buf + cnt++) = '0';
			//*(buf + cnt++) = '9';
			//*(buf + cnt++) = '8';
			//*(buf + cnt++) = '7';
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 2:
			if(btn_press == 0){	//初期登録
				*(buf + cnt++) = ( UB )LCD_INIT_INPUT;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{	//メンテナンス
				*(buf + cnt++) = ( UB )LCD_INIT_INPUT;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 3:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else{
				dat = btn_press;
				sv_reg_num = dat;
				dat += 0x30;
				*(buf + cnt++) = ( UB )LCD_YUBI_ID;
				*(buf + cnt++) = '0';
				*(buf + cnt++) = '0';
				*(buf + cnt++) = (UB)dat;
				*(buf + cnt++) = 0;
				
				sv_yubi_seq_no[0] = '0';
				sv_yubi_seq_no[1] = '0';
				sv_yubi_seq_no[2] = (UB)dat;
				//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
				//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '0';
				//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
			}
			*msize = cnt;
			break;
		case 4:

			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				for(i = 0 ; i < 24 ; i++ ){
					*(buf + i) = 0x20;
				}
				*(buf + 24) = 0;
				
				*(buf + cnt++) = ( UB )LCD_NAME;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];
				*(buf + cnt++) = sv_keyindat[4];
				*(buf + cnt++) = sv_keyindat[5];
				*(buf + cnt++) = sv_keyindat[6];
				*(buf + cnt++) = sv_keyindat[7];
				*msize = 26;
			
				//memcpy(&yb_touroku_data20[sv_reg_num].name[0], &sv_keyindat[0], 8);
			}
			
			break;
		case 5:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
				if(btn_press <= 3){
					*(buf + cnt++) = '1';	//左手
					dat = btn_press + 1 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '1';
					sv_yubi_no[1] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = '1';
					//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = (UB)dat;
				}else{
					*(buf + cnt++) = '0';	//右手
					dat = btn_press - 2 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '0';
					sv_yubi_no[1] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = '0';
					//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = (UB)dat;
				}
				
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 9:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = sv_yubi_seq_no[0];
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = sv_yubi_seq_no[1];
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = sv_yubi_seq_no[2];
			yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = sv_yubi_no[0];
			yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = sv_yubi_no[1];
			memcpy(&yb_touroku_data20[sv_reg_num].name[0], &sv_keyindat[0], 8);
			break;
		case 10:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = 0x30;
			//memset(&yb_touroku_data20[sv_reg_num].name[0], 0, 8);
			break;
		case 11:
		case 12:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
//20130610_Miya 画像データ採取実施フラグ
#ifdef GETIMG20130611
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
#else
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
#endif
			}
			*msize = cnt;
			break;
		case 100:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 101:
		case 102:
			if(btn_press == 0){			//登録
				*(buf + cnt++) = ( UB )LCD_TOUROKU;
				*(buf + cnt++) = 0;
			}else if(btn_press == 1){	//削除
				*(buf + cnt++) = ( UB )LCD_SAKUJYO;
				*(buf + cnt++) = 0;
			}else if(btn_press == 2){	//緊急解錠番号
				*(buf + cnt++) = ( UB )LCD_KINKYUU_SETTEI;
				*(buf + cnt++) = 0;
			}else if(btn_press == 3){	//緊急解錠
				*(buf + cnt++) = ( UB )LCD_KINKYUU_KAIJYOU;
				*(buf + cnt++) = 0;
			}else if(btn_press == 4){	//メンテ
				*(buf + cnt++) = ( UB )LCD_MAINTE;
				*(buf + cnt++) = 0;
			}

			*msize = cnt;
			break;
		case 103:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 104:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 120:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 122:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 123:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 124:
		case 125:
		case 126:
		case 127:
		case 128:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else if(btn_press == 5){
				nop();
			}else if(btn_press == 6){
				nop();
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_ID;
				*(buf + cnt++) = '0';
				dat = btn_press + 4 * (page - 3);
				sv_reg_num = dat;
				if(dat >= 10){
					if(dat >= 20){
						*(buf + cnt++) = '2';
						dat = dat - 20 + 0x30;
						*(buf + cnt++) = (UB)dat;
						sv_yubi_seq_no[0] = '0';
						sv_yubi_seq_no[1] = '2';
						sv_yubi_seq_no[2] = (UB)dat;
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '2';
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}else{
						*(buf + cnt++) = '1';
						dat = dat - 10 + 0x30;
						*(buf + cnt++) = (UB)dat;
						sv_yubi_seq_no[0] = '0';
						sv_yubi_seq_no[1] = '1';
						sv_yubi_seq_no[2] = (UB)dat;
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '1';
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}
				}else{
					*(buf + cnt++) = '0';
					dat = dat + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_seq_no[0] = '0';
					sv_yubi_seq_no[1] = '0';
					sv_yubi_seq_no[2] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
					//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '0';
					//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
				}
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 129:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				for(i = 0 ; i < 24 ; i++ ){
					*(buf + i) = 0x20;
				}
				*(buf + 24) = 0;
				
				*(buf + cnt++) = ( UB )LCD_NAME;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];
				*(buf + cnt++) = sv_keyindat[4];
				*(buf + cnt++) = sv_keyindat[5];
				*(buf + cnt++) = sv_keyindat[6];
				*(buf + cnt++) = sv_keyindat[7];
				*msize = 26;
				//memcpy(&yb_touroku_data20[sv_reg_num].name[0], &sv_keyindat[0], 8);
			}
			break;
		case 130:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
				if(btn_press <= 3){
					*(buf + cnt++) = '1';	//左手
					dat = btn_press + 1 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '1';
					sv_yubi_no[1] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = '1';
					//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = (UB)dat;
				}else{
					*(buf + cnt++) = '0';	//右手
					dat = btn_press - 2 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '0';
					sv_yubi_no[1] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = '0';
					//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = (UB)dat;
				}
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 134:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = sv_yubi_seq_no[0];
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = sv_yubi_seq_no[1];
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = sv_yubi_seq_no[2];
			yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = sv_yubi_no[0];
			yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = sv_yubi_no[1];
			memcpy(&yb_touroku_data20[sv_reg_num].name[0], &sv_keyindat[0], 8);
			break;
		case 135:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = 0x30;
			//memset(&yb_touroku_data20[sv_reg_num].name[0], 0, 8);
			break;
		case 136:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = 0x30;
			//memset(&yb_touroku_data20[sv_reg_num].name[0], 0, 8);
			break;
		case 140:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 142:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 143:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 144:
		case 145:
		case 146:
		case 147:
		case 148:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else if(btn_press == 5){
				nop();
			}else if(btn_press == 6){
				nop();
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_ID;
				*(buf + cnt++) = '0';
				dat = btn_press + 4 * (page - 3);
				sv_reg_num = dat;
				if(dat >= 10){
					if(dat >= 20){
						*(buf + cnt++) = '2';
						dat = dat - 20 + 0x30;
						*(buf + cnt++) = (UB)dat;
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '2';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}else{
						*(buf + cnt++) = '1';
						dat = dat - 10 + 0x30;
						*(buf + cnt++) = (UB)dat;
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '1';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}
				}else{
					*(buf + cnt++) = '0';
					dat = dat + 0x30;
					*(buf + cnt++) = (UB)dat;
					yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
					yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '0';
					yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
				}
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
/*
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else if(btn_press == 5){
				nop();
			}else if(btn_press == 6){
				nop();
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_ID;
				*(buf + cnt++) = '0';
				dat = btn_press + 4 * (page - 3);
				if(dat >= 10){
					if(dat >= 20){
						*(buf + cnt++) = '2';
						dat = dat - 20 + 0x30;
						*(buf + cnt++) = (UB)dat;
					}else{
						*(buf + cnt++) = '1';
						dat = dat - 10 + 0x30;
						*(buf + cnt++) = (UB)dat;
					}
				}else{
					*(buf + cnt++) = '0';
					*(buf + cnt++) = (UB)dat;
				}
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
*/
		case 149:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = 0x30;
				yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = 0x30;
				yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = 0x30;
				yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = 0x30;
				yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = 0x30;
				memset(&yb_touroku_data20[sv_reg_num].name[0], 0, 8);
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 150:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 160:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 162:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 163:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 164:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_KINKYUU_BANGOU;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 165:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 180:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 181:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
			}else{	//→
				*(buf + cnt++) = ( UB )LCD_NEXT;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 182:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_USER_ID;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 183:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
			}else{	//→
				*(buf + cnt++) = ( UB )LCD_NEXT;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 184:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_KINKYUU_KAIJYOU_BANGOU;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];
				*(buf + cnt++) = sv_keyindat[4];
				*(buf + cnt++) = sv_keyindat[5];
				*(buf + cnt++) = sv_keyindat[6];
				*(buf + cnt++) = sv_keyindat[7];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 185:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_KINKYUU_BANGOU;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 186:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 187:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 188:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 200:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 201:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_USER_ID;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 202:
			if(btn_press == 0){			//初期化
				*(buf + cnt++) = ( UB )LCD_MAINTE_SHOKIKA_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 3){	//フル画像
				*(buf + cnt++) = ( UB )LCD_FULL_PIC_SEND_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 4){	//終了
				*(buf + cnt++) = ( UB )LCD_MAINTE_END;
				*(buf + cnt++) = 0;
			}else{
				
			}
			*msize = cnt;
			break;
		case 204:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 205:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 206:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		default:
			proc = 0;
			break;	
	}

	return(rtn);
}


UB KeyInputNumSousa( int gmn, int buf, int *keta, int btn )
{
	UB rtn=0;
	int num, key;
	int lmt;
	
	num = *keta;
	
	if(gmn == 184 ){
		lmt = 8;
	}else{
		lmt = 4;
	}

	switch( btn )
	{
		case KEY1:
			key = 1;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY2:
			key = 2;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY3:
			key = 3;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY4:
			key = 4;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY5:
			key = 5;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY6:
			key = 6;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY7:
			key = 7;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY8:
			key = 8;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY9:
			key = 9;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY0:
			key = 0;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEYCAN:
			break;
		case KEYDEL:
			key = 10;
			if( num > 1 ){
				sv_keyindat[num-1] = 0x20;
				--num;
				SetKeyNum(buf, num, key);
			}
			break;
		case KEYRTN:
			if( num > lmt ){
				rtn = 1;
			}
			break;
		default:
			break;
	}
	
	*keta = num;
	return(rtn);	
}


//ひらがなキーボード操作
UB KeyInputMojiSousa( int buf, int *hit, int *keta, int btn )
{
	UB rtn=0, code;
	int num, key, cnt;
	
	num = *keta;
	cnt = *hit;

	switch( btn )
	{
		case KEY1:
			key = MOJICODEAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 10){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY2:
			key = MOJICODEKAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEKAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY3:
			key = MOJICODESAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODESAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY4:
			key = MOJICODETAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 6){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODETAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY5:
			key = MOJICODENAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODENAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY6:
			key = MOJICODEHAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEHAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY7:
			key = MOJICODEMAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEMAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY8:
			key = MOJICODEYAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 6){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEYAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY9:
			key = MOJICODERAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODERAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY0:
			key = 0;
			if(sv_mkey == MOJICODEKAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEGAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEGAGYOU;
				}
			}else if(sv_mkey == MOJICODESAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEZAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEZAGYOU;
				}
			}else if(sv_mkey == MOJICODETAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEDAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEDAGYOU;
				}
			}else if(sv_mkey == MOJICODEHAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEBAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEBAGYOU;
				}
			}else if(sv_mkey == MOJICODEBAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEPAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEPAGYOU;
				}
			}else if(sv_mkey == MOJICODEGAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEKAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEKAGYOU;
				}
			}else if(sv_mkey == MOJICODEZAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODESAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODESAGYOU;
				}
			}else if(sv_mkey == MOJICODEDAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODETAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODETAGYOU;
				}
			}else if(sv_mkey == MOJICODEPAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEHAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEHAGYOU;
				}
			}			
			
			break;
		case KEYKIGOU:
			key = MOJICODEWAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 6){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEWAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEYCAN:
			key = 0xff;
			rtn = 1;
			break;
		case KEYDEL:
			key = 0xfe;
			cnt = 0;
			if( num > 1 ){
				//sv_keyindat[num-1] = 0x20;
				--num;
				sv_keyindat[num-1] = 0;
				SetKeyMoji(buf, MOJICODEDEL, cnt, num, key);
			}
			sv_mkey = key;
			break;
		case KEYRTN:
			key = 0xfd;
			if( num > 1 ){
				rtn = 1;
			}
			break;
		case KEYCSR:
			key = 0xfc;
			if( cnt >0 ){
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODECSR, cnt, num, key);
					cnt = 0;
					sv_mkey = 0xff;
				}
			}
			break;
		default:
			sv_mkey = 0xff;
			break;
	}
	
	*keta = num;
	*hit = cnt;
	return(rtn);	
}




UB KeyPageSousa( int *page, int btn )
{
	UB rtn=0;
	int num;
	
	num = *page;

	if( btn == 5 ){			//→ボタン
		num += 1;
		if( num > 7 ){
			num = 3;
		}
		SetInfo(num);
		LcdcDisplayModeSet(num, 0);
	}else if( btn == 6 ){		//←ボタン
		if(num > 3){
			num -= 1;
			SetInfo(num);
			LcdcDisplayModeSet(num, 0);
		}
	}else{
		rtn = 1;	//CHG_REQ要求
	}
	
	*page = num;
	
	return(rtn);
}


void SetVerNum(int buf_num)
{
	int x1, x2, y1, y2;
	int num, dat;
	int i;
	unsigned short VerPos[6][2] =
					{
						{8, 87}, {47, 118},		//1ケタ
						{8, 39}, {47, 70},		//2ケタ
						{8,  7}, {47, 38},		//3ケタ
					};

	for(i = 0; i < 3; i++){

		num = 2 * i;

		x1 = VerPos[num][0];
		y1 = VerPos[num][1];
		x2 = VerPos[num+1][0];
		y2 = VerPos[num+1][1];

		if( i == 0 ){
			dat = VER_MAJOR - 0x30;
		}
		if( i == 1 ){
			dat = VER_MINOR - 0x30;
		}
		if( i == 2 ){
			dat = VER_REVISION - 0x30;
		}
		
		SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, dat, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
	}
}


void SetErrMes(int buf_num, int mes_num)
{
	int x1, x2, y1, y2;
	int num;
	
	x1 = MesPosition[0][0];
	y1 = MesPosition[0][1];
	x2 = MesPosition[1][0];
	y2 = MesPosition[1][1];

	SetGmnToLcdBuff02(buf_num, GMNERRMESCODE, mes_num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}

void SetKeyHeadMes(int buf_num, int mes_num)
{
	int x1, x2, y1, y2;
	int num;
	
	x1 = KeyHeadMesPosition[0][0];
	y1 = KeyHeadMesPosition[0][1];
	x2 = KeyHeadMesPosition[1][0];
	y2 = KeyHeadMesPosition[1][1];

	SetGmnToLcdBuff02(buf_num, GMNKEYHEADCODE, mes_num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}

void SetKinkyuNum1(int buf_num, int keta, int key)
{
	int x1, x2, y1, y2;
	int num;
	
	num = 2 * (keta - 1);
	
	x1 = kinkyuNumPos1[num][0];
	y1 = kinkyuNumPos1[num][1];
	x2 = kinkyuNumPos1[num+1][0];
	y2 = kinkyuNumPos1[num+1][1];
	SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, key, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}

void SetKinkyuNum2(int buf_num, int keta, int key)
{
	int x1, x2, y1, y2;
	int num;
	
	num = 2 * (keta - 1);
	
	x1 = kinkyuNumPos2[num][0];
	y1 = kinkyuNumPos2[num][1];
	x2 = kinkyuNumPos2[num+1][0];
	y2 = kinkyuNumPos2[num+1][1];
	SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, key, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}


void SetKeyNum(int buf_num, int keta, int key)
{
	int x1, x2, y1, y2;
	int num;
	
	num = 2 * (keta - 1);
	
	x1 = InpPosKey[num][0];
	y1 = InpPosKey[num][1];
	x2 = InpPosKey[num+1][0];
	y2 = InpPosKey[num+1][1];
	SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, key, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}

void SetKeyMoji(int buf_num, int moji, int hit, int keta, int key)
{
	UB code;
	int x1, x2, y1, y2;
	int num;


	if(moji >= 0 && moji <= 0x0E ){
		code = (UB)moji << 4;
		code = code | ((UB)hit - 1);
		sv_keyindat[keta-1] = code;
	}

	num = 2 * (keta - 1);
	
	x1 = InpPosKey[num][0];
	y1 = InpPosKey[num][1];
	x2 = InpPosKey[num+1][0];
	y2 = InpPosKey[num+1][1];

	SetGmnToLcdBuff02(buf_num, GMNMKEYCOADE, moji, hit-1, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}

void SetInfo(int page)
{
	int num, inp_num;
	int i, j;
	int x1, x2, y1, y2;
	int moji, hit;
	
	
	num = 4 * (page - 3) + 1;

	for(i = 0, j = 0 ; i < 4 ; i++, j+=2 ){
		if( yb_touroku_data20[num].yubi_seq_no[0] == 0x30 && yb_touroku_data20[num].yubi_seq_no[1] == 0x30 && yb_touroku_data20[num].yubi_seq_no[2] == 0x30){
			x1 = InpPos1[j][0];
			y1 = InpPos1[j][1];
			x2 = InpPos1[j+1][0];
			y2 = InpPos1[j+1][1];		
			inp_num = 7;	//消去
			SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);

			x1 = InpPos2[j][0];
			y1 = InpPos2[j][1];
			x2 = InpPos2[j+1][0];
			y2 = InpPos2[j+1][1];
			inp_num = 7;	//消去
			SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		}else{
			x1 = InpPos1[j][0];
			y1 = InpPos1[j][1];
			x2 = InpPos1[j+1][0];
			y2 = InpPos1[j+1][1];		
			inp_num = yb_touroku_data20[num].yubi_no[ 0 ] - 0x30;
			if( inp_num == 0 || inp_num == 1 ){
				SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
			}

			x1 = InpPos2[j][0];
			y1 = InpPos2[j][1];
			x2 = InpPos2[j+1][0];
			y2 = InpPos2[j+1][1];
			inp_num = yb_touroku_data20[num].yubi_no[ 1 ] - 0x30;
			if( inp_num >= 1 && inp_num <= 5 ){
				inp_num += 1;
				SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
			}
		}
		num++;
	}
	
	num = 4 * (page - 3) + 1;
	for(i = 0 ; i < 4 ; i++){
		if( yb_touroku_data20[num].yubi_seq_no[0] == 0x30 && yb_touroku_data20[num].yubi_seq_no[1] == 0x30 && yb_touroku_data20[num].yubi_seq_no[2] == 0x30){
			for( j = 0 ; j < 8 ; j++ ){
				x1 = InpPos3[i][0];
				y1 = InpPos3[i][1] - 32 * j;
				x2 = x1 + 31;
				y2 = y1 + 31;
				
				hit = 0;
				SetGmnToLcdBuff03(4, page, GMNMKEYCOADE, MOJICODEDEL, hit, 0, 0, 0, 0, x1, y1, x2, y2);	//消去
			}	
		}else{
			for( j = 0 ; j < 8 ; j++ ){
				x1 = InpPos3[i][0];
				y1 = InpPos3[i][1] - 32 * j;
				x2 = x1 + 31;
				y2 = y1 + 31;
				
				moji = yb_touroku_data20[num].name[j];
				if(moji != 0){
					hit = moji & 0x0F;
					moji = (moji & 0xF0) >> 4;
					//SetGmnToLcdBuff02(page, GMNMKEYCOADE, moji, hit, 0, 0, 0, 0, x1, y1, x2, y2);
					SetGmnToLcdBuff03(4, page, GMNMKEYCOADE, moji, hit, 0, 0, 0, 0, x1, y1, x2, y2);
				}else{
					hit = 0;
					SetGmnToLcdBuff03(4, page, GMNMKEYCOADE, MOJICODEDEL, hit, 0, 0, 0, 0, x1, y1, x2, y2);	//消去
				}					
			}	
		}
		num++;
	}
}




void SetGmnToLcdBuff(int buf_num, int gmn_num, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	num = buf_num;
	LcdcDisplayWindowSet(num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	num = gmn_num;
	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		val1 = GetPixDataFromGamen01(num, cnt);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();	
}


void SetGmnToLcdBuff02(int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	//num = buf;
	LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		val1 = GetPixDataFromInputGmn(gmn_num, sub_num1, sub_num2, cnt);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();
}

void SetGmnToLcdBuff03(int offset, int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	int st, imgx, imgy, ln_size, ln_num;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	//num = buf;
	LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	imgx = 40;
	imgy = 32;
	ln_size = 2 * (x2 - x1 + 1);
	ln_num = y2 - y1 + 1;

	for( y = 0 ; y < ln_num ; y++ ){
		st = y * imgx * 2 + 2 * offset;
		for(x = 0 ; x < ln_size ; x++ ){
			val1 = GetPixDataFromInputGmn(gmn_num, sub_num1, sub_num2, st);
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
			++st;
		}
	}
	
	LcdcDisplayWindowFinish();
}



UH GetPixDataFromInputGmn(int buf, int sub_num1, int sub_num2, unsigned long cnt)
{
	UH val;
	
	switch(buf){
		case 0:
			val = InpGmnMigi[cnt];
			break;
		case 1:
			val = InpGmnHidari[cnt];
			break;
		case 2:
			val = InpGmnYubi01[cnt];
			break;
		case 3:
			val = InpGmnYubi02[cnt];
			break;
		case 4:
			val = InpGmnYubi03[cnt];
			break;
		case 5:
			val = InpGmnYubi04[cnt];
			break;
		case 6:
			val = InpGmnYubi05[cnt];
			break;
		case 7:
			if( (cnt % 2) == 0 ){
				val = 0x0034;
			}else{
				val = 0x9ae8;
			}
			break;
		case GMN10KEYCOADE:
			if(sub_num1 == 10){
				if( (cnt % 2) == 0 ){
					val = 0x0034;
				}else{
					val = 0x9ae8;
				}
			}else{
				val = LcdGmnDataNum[sub_num1][cnt];
			}
			break;
		case GMNMKEYCOADE:	
			if(sub_num1 == MOJICODEDEL){
				if( (cnt % 2) == 0 ){
					val = 0x0034;
				}else{
					val = 0x9ae8;
				}
				break;
			}
			if(sub_num1 == MOJICODECSR){
				if( (cnt % 2) == 0 ){
					val = 0x00ff;
				}else{
					val = 0xffff;
				}
				break;
			}
		
			if(sub_num1 == MOJICODEAGYOU){
				val = LcdGmnDataAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEKAGYOU ){
				val = LcdGmnDataKAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODESAGYOU ){
				val = LcdGmnDataSAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODETAGYOU ){
				val = LcdGmnDataTAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODENAGYOU ){
				val = LcdGmnDataNAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEHAGYOU ){
				val = LcdGmnDataHAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEMAGYOU ){
				val = LcdGmnDataMAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEYAGYOU ){
				val = LcdGmnDataYAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODERAGYOU ){
				val = LcdGmnDataRAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEWAGYOU ){
				val = LcdGmnDataWAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEGAGYOU ){
				val = LcdGmnDataGAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEZAGYOU ){
				val = LcdGmnDataZAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEDAGYOU ){
				val = LcdGmnDataDAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEBAGYOU ){
				val = LcdGmnDataBAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEPAGYOU ){
				val = LcdGmnDataPAgyou[sub_num2][cnt];
			}

			break;
		case GMNERRMESCODE:
			if(sub_num1 == 0){
				if( (cnt % 2) == 0 ){
					val = 0x0067;
				}else{
					val = 0x6767;
				}
			}else{
				val = LcdMes001[cnt];
			}
			break;
		case GMNKEYHEADCODE:
			if(sub_num1 == 1){
				val = LcdBoardMes01[cnt];
			}else if(sub_num1 == 2){
				val = LcdBoardMes02[cnt];
			}else if(sub_num1 == 3){
				val = LcdBoardMes03[cnt];
			}		
		
			break;
		default:
			val = 0x00;
			break;
	}
	
	return(val);
}


UB WaitKeyProc(int sousa, int tflg, UB *msg)
{
	UB	rtn=0;
	UB	rtnsts;
	ER	ercd;
	int posx, posy;
	int x1, x2, y1, y2;
	int tcnt;
	int dbg_cnt;
	UB *buf;

	buf = msg;
	tcnt = 0;
	dbg_cnt = 0;
	while(1){
		ercd = TplPosGet(&posx, &posy, (500/MSEC));	//キー待ち500msec
		if(	ercd == E_OK ){
			dly_tsk((500/MSEC));	//500msecのウエイト(タッチキーの反応解除用)
			rtn = 1;	//CHG_REQ要求

			if(tflg == 2){	//画面Sleep状態の場合(LCDバックライトをOFFにしている)
				LcdcBackLightOn();
				tflg = 1;
				tcnt = 0;
			}else{
				if( sousa == 99 ){
					x1 = 208;
					x2 = 272;
					y1 = 104;
					y2 = 167;
					if( (posx > x1 && posx < x2) && (posy > y1 && posy < y2) ){
						break;
					}
				}else{
					break;
				}
			}
			
		}

		if(tflg == 1){
			++tcnt;
			if(tcnt >= 120){	//画面Sleep状態にする
				LcdcBackLightOff();
				tflg = 2;
				tcnt = 0;
			}
		}

		rtnsts = MdGetMode();
		if( rtnsts == MD_CAP ){
			rtn = 2;	//CHG_REQなし
			LcdcBackLightOn();
			break;
		}
	}

	return(rtn);
}


void SetGamen01(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	num = buf;
	LcdcDisplayWindowSet(num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		val1 = GetPixDataFromGamen01(num, cnt);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();
}


UH GetPixDataFromGamen01(int buf, unsigned long cnt)
{
	UH val;

	switch(buf){
		case 0:
			val = LcdGmn000[cnt];
			break;
		case 1:
			val = LcdGmn001[cnt];
			break;
		case 2:
			val = LcdGmn101[cnt];
			break;
		case 3:
			val = LcdGmn102[cnt];
			break;
		case 4:
			val = LcdGmn002[cnt];
			break;
		case 5:
			val = LcdGmn107[cnt];
			break;
		case 6:
			val = LcdGmn201[cnt];
			break;
		case 7:
			val = LcdGmn202[cnt];
			break;
		case 8:
			val = LcdGmn203[cnt];
			break;
		case 9:
			val = LcdGmn204[cnt];
			break;
		case 10:
			val = LcdGmn206[cnt];
			break;
		case 11:
			val = LcdGmn301[cnt];
			break;
		case 12:
			val = LcdGmn302[cnt];
			break;
		case 100:
			val = 0x00;
			break;
		case 101:
			val = LcdGmn108[cnt];
			break;
		case 102:
			val = 0x00;
			break;
		case 103:
			val = LcdGmn205[cnt];
			break;
		case 104:
			val = LcdGmn206[cnt];
			break;
		case 120:
			val = 0x00;
			break;
		case 121:
			val = LcdGmn207[cnt];
			break;
		case 122:
			val = LcdGmn205[cnt];
			break;
		case 123:
			val = LcdGmn206[cnt];
			break;
		case 124:
			val = LcdGmn102[cnt];
			break;
		case 125:
			val = LcdGmn103[cnt];
			break;
		case 126:
			val = LcdGmn104[cnt];
			break;
		case 127:
			val = LcdGmn105[cnt];
			break;
		case 128:
			val = LcdGmn106[cnt];
			break;
		case 129:
			val = LcdGmn002[cnt];
			break;
		case 130:
			val = LcdGmn107[cnt];
			break;
		case 131:
			val = LcdGmn201[cnt];
			break;
		case 132:
			val = LcdGmn202[cnt];
			break;
		case 133:
			val = LcdGmn203[cnt];
			break;
		case 134:
			val = LcdGmn204[cnt];
			break;
		case 135:
			val = LcdGmn206[cnt];
			break;
		case 136:
			val = LcdGmn302[cnt];
			break;
		case 140:
			val = 0x00;
			break;
		case 141:
			val = LcdGmn207[cnt];
			break;
		case 142:
			val = LcdGmn205[cnt];
			break;
		case 143:
			val = LcdGmn206[cnt];
			break;
		case 144:
			val = LcdGmn102[cnt];
			break;
		case 145:
			val = LcdGmn103[cnt];
			break;
		case 146:
			val = LcdGmn104[cnt];
			break;
		case 147:
			val = LcdGmn105[cnt];
			break;
		case 148:
			val = LcdGmn106[cnt];
			break;
		case 149:
			val = LcdGmn303[cnt];
			break;
		case 150:
			val = LcdGmn302[cnt];
			break;
		case 160:
			val = 0x00;
			break;
		case 161:
			val = LcdGmn207[cnt];
			break;
		case 162:
			val = LcdGmn205[cnt];
			break;
		case 163:
			val = LcdGmn206[cnt];
			break;
		case 164:
			val = LcdGmn001[cnt];
			break;
		case 165:
			val = LcdGmn302[cnt];
			break;
		case 180:
			val = 0x00;
			break;
		case 181:
			val = LcdGmn208[cnt];
			break;
		case 182:
			val = LcdGmn001[cnt];
			break;
		case 183:
			val = LcdGmn209[cnt];
			break;
		case 184:
			val = LcdGmn001[cnt];
			break;
		case 185:
			val = LcdGmn001[cnt];
			break;
		case 186:
			val = LcdGmn205[cnt];
			break;
		case 187:
			val = LcdGmn206[cnt];
			break;
		case 188:
			val = LcdGmn302[cnt];
			break;
		case 200:
			val = 0x00;
			break;
		case 201:
			val = LcdGmn001[cnt];
			break;
		case 202:
			val = LcdGmn109[cnt];
			break;
		case 203:
			val = LcdGmn207[cnt];
			break;
		case 204:
			val = LcdGmn205[cnt];
			break;
		case 205:
			val = LcdGmn206[cnt];
			break;
		case 206:
			val = LcdGmn303[cnt];
			break;
		default:
			val = 0x00;
			break;
	}
	return(val);
}



