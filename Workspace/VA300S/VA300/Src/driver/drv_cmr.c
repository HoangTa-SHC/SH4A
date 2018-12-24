/**
*	VA-300プログラム
*
*	@file drv_cmr.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/24
*	@brief  カメラ制御ドライバ
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

// 定義
#define	CMR_CAP_IP	10					///< カメラ画像取り込み完了の割込みレベル
#define	CMR_CAP_INT	INT_IRL10			///< カメラ画像取り込み完了の割込み番号
#define	CMR_SND_IP	6					///< カメラコマンド送信完了の割込みレベル
#define	CMR_SND_INT	INT_IRL6			///< カメラコマンド送信完了の割込み番号
#define	CMR_RCV_IP	5					///< カメラコマンド受信完了の割込みレベル
#define	CMR_RCV_INT	INT_IRL5			///< カメラコマンド受信完了の割込み番号

#define	enable_cap_int()	fpga_setw(INT_CRL, INT_CRL_CAP)		///< キャプチャー割込み
#define	clear_cap_int()		fpga_clrw(INT_CRL, INT_CRL_CAP)		///< キャプチャー割込みクリア＆不許可
#define	enable_snd_int()	fpga_setw(INT_CRL, INT_CRL_CMR_CMD)	///< コマンド送信割込み
#define	clear_snd_int()		fpga_clrw(INT_CRL, INT_CRL_CMR_CMD)	///< コマンド送信割込みクリア＆不許可
#define	enable_rcv_int()	fpga_setw(INT_CRL, INT_CRL_CMR_RES)	///< コマンド受信割込み
#define	clear_rcv_int()		fpga_clrw(INT_CRL, INT_CRL_CMR_RES)	///< コマンド受信割込みクリア＆不許可
#define	cap_line_get(b,x,y,n,p)	FpgaCapLineGet(b,x,y,n,p)	///< 取込み画像の1ライン取得
#define	cap_dat_wrt(b,x,y,n,p)	FpgaWrtDat(b,x,y,n,p)	///Miya

// 変数定義
static ID s_idCapTsk;					///< キャプチャーのタスクID
static ID s_idSendTsk;					///< コマンド送信タスクID
static ID s_idRecvTsk;					///< 応答受信タスクID
static ID s_idSem;						///< セマフォID

// プロトタイプ宣言
static INTHDR cmrCaptureInt(void);		///< カメラ画像取り込み完了割込み
static void cmr_capture_int(void);		///< カメラ画像取り込み完了割込み本体
#if ( NEWCMR == 0 )
static INTHDR cmrSendInt(void);			///< コマンド送信完了割込み
static void cmr_send_int(void);			///< コマンド送信完了割込み本体
static INTHDR cmrRecvInt(void);			///< コマンド受信完了割込み
static void cmr_recv_int(void);			///< コマンド受信完了割込み本体
static ER cmrCapture(enum CAP_MODE eMode, enum CAP_BANK eBank);	///< カメラ画像キャプチャー(本体)
static ER cmrCaptureWait(TMO tmout);	///< カメラ画像取り込み待ち
#else
ER NcmrCapture( enum CAP_MODE eMode, enum CAP_BANK eBank );	///< カメラ画像キャプチャー(本体)
ER NcmrCaptureWait( TMO tmout );	///< カメラ画像取り込み待ち
#endif

static void cmrPktSend(UB *pData, int iSendSize, int iRcvSize);	///< カメラ通信I/F 送信(本体)
static BOOL cmrPktRecv(UB *pData, int iRcvSize);	///< カメラ通信I/F 受信(本体)
static void cmrCapLineGet(enum CAP_BANK eBank, int iX, int iY, int iSize, UB *p);	///< カメラ画像を指定バイト数取得する(本体)
static void cmrCapGet(enum CAP_BANK eBank, int iX, int iY, int iXsize, int iYsize, UB *p);	///< カメラ画像を取得する(本体)
static void cmrMemWrt(enum CAP_BANK eBank, int iX, int iY, int iSize, UB *p);

static unsigned short cmrGenCRC(unsigned char *lpBuf, unsigned char sizeOfBuf);	///< CRC16を求める
static ER cmrTrim(enum CAP_BANK eOrg, enum TRIM_BANK eDist, int iStartX, int iStartY, int iSizeX, int iSizeY, TMO tmout);	///< 画像のトリミング
static void cmrTrimGet(enum CAP_BANK eBank, long lStart, long lSize, UB *p);	///< トリミング画像の取得
static ER cmrResize(enum TRIM_BANK eOrg, enum RSZ_BANK eDist, enum RSZ_MODE eMode, int iSizeX, int iSizeY, TMO tmout);	///< 画像の圧縮
static void cmrResizeGet(enum RSZ_BANK eBank, long lStart, long lSize, UB *p);	///< 圧縮画像の取得

#if ( NEWCMR == 1 )
static ER NCmrI2C_Send( UB *pAddr, UB *pData, int iSendSize );		// カメラ・コマンドのI2C送信
static int NcmrI2C_Send( UB *pAddr, UB *pData, int iSendSize );		// カメラ・コマンドのI2C送信(本体)
static ER NCmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize );		// カメラ・コマンドのI2C受信
static int NcmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize );		// カメラ・コマンドのI2C受信(本体)
static void NCmrCapStart( void );	///< キャプチャ開始要求

#endif

// 割込み定義
const T_DINH dinh_cmr_cap = { TA_HLNG, cmrCaptureInt, CMR_CAP_IP};	///< 画像取り込み
#if ( NEWCMR == 0 )
const T_DINH dinh_cmr_snd = { TA_HLNG, cmrSendInt, CMR_SND_IP};		///< コマンド送信
const T_DINH dinh_cmr_rcv = { TA_HLNG, cmrRecvInt, CMR_RCV_IP};		///< 応答受信
#endif
// セマフォ定義
const T_CSEM csem_cmr     = { TA_TFIFO, 1, 1, (B *)"sem_cmr" };		///< カメラのセマフォ

/*==========================================================================*/
/**
 * カメラ関連初期化
 * 
 * @param idSem カメラ用セマフォ
 * @param idRcvTsk 応答受信完了割込み時に起床するタスクID
 */
/*==========================================================================*/
ER CmrInit(ID idSem, ID idRcvTsk)
{
	ER ercd;
	UW psw;
	
	ercd = E_OK;
	
	// 割込み時に起床するタスクの設定
	if (idRcvTsk <= 0) {
		return E_PAR;					// パラメータエラー
	}
	
	// セマフォの生成
	if (idSem <= 0) {
		return E_PAR;					// パラメータエラー
	}
	ercd = cre_sem(idSem, &csem_cmr);	// セマフォ生成
	if (ercd != E_OK) {
		return ercd;
	}
	
	// スタティック変数の設定
	s_idRecvTsk = idRcvTsk;
	s_idSem     = idSem;

	psw = vdis_psw();
	
	// ベクタ登録
	ercd = def_inh(CMR_CAP_INT, &dinh_cmr_cap);		// 画像取り込み完了割込み設定
	if (ercd == E_OK) {
		enable_cap_int();							// 割込み設定(ハードウェア側)
	}

#if ( NEWCMR == 0 )
	if (ercd == E_OK) {
		ercd = def_inh(CMR_RCV_INT, &dinh_cmr_rcv);	// 応答受信完了割込み設定
		if (ercd == E_OK) {
			enable_rcv_int();						// 割込み設定(ハードウェア側)
		}
	}
#endif

	vset_psw(psw);
	
	return ercd;
}

#pragma interrupt(cmrCaptureInt)
/*==========================================================================*/
/**
 * 画像取込み完了割込みハンドラ
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
 * 画像取込み完了割込みハンドラ(本体)
 *
 */
/*==========================================================================*/
static void cmr_capture_int(void)
{
	clear_cap_int();					// キャプチャー割込みクリア＆不許可
	if (s_idCapTsk) {
		iwup_tsk(s_idCapTsk);
	}
	enable_cap_int();					// キャプチャー割込みクリア＆不許可
}

#if ( NEWCMR == 0 )
#pragma interrupt(cmrSendInt)
/*==========================================================================*/
/**
 * コマンド送信割込みハンドラ
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
 * コマンド送信割込みハンドラ(本体)
 *
 */
/*==========================================================================*/
static void cmr_send_int(void)
{
	clear_snd_int();					// コマンド送信割込みクリア＆不許可
	if (s_idSendTsk) {
		iwup_tsk(s_idSendTsk);
	}
	enable_snd_int();					// コマンド送信割込み
}

#pragma interrupt(cmrRecvInt)
/*==========================================================================*/
/**
 * 受信完了割込みハンドラ
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
 * 受信完了割込みハンドラ(本体)
 *
 */
/*==========================================================================*/
static void cmr_recv_int(void)
{
	clear_rcv_int();					// コマンド受信割込みクリア＆不許可
	if (s_idRecvTsk) {
		iwup_tsk(s_idRecvTsk);
	}
	enable_rcv_int();					// コマンド受信割込み
}
#endif

/*==========================================================================*/
/**
 * カメラ画像のキャプチャー
 *
 *	@param eMode 画像取り込みシーケンスモード
 *	@param eBank 取り込む領域
 *	@param tmout タイムアウト時間
 *	@return OSのエラーコード
 */
/*==========================================================================*/
ER CmrCapture(enum CAP_MODE eMode, enum CAP_BANK eBank, TMO tmout)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	
	if (ercd == E_OK) {
#if ( NEWCMR == 0 )
		ercd = cmrCapture(eMode, eBank);	// 画像取り込み(GCT社カメラの場合)
		if (ercd == E_OK) {
			ercd = cmrCaptureWait(tmout);	// 取り込み待ち(GCT社カメラの場合)
		}
#else
		ercd = NcmrCapture(eMode, eBank);	// 画像取り込み(NC社カメラNCM03-Vの場合)
		if (ercd == E_OK) {
			ercd = NcmrCaptureWait(tmout);	// 取り込み待ち(NC社カメラNCM03-Vの場合)
		}
#endif
		sig_sem(s_idSem);
	}
	
	return ercd;
}


/*==========================================================================*/
/**
 * カメラ画像のキャプチャー
 * Bionics改造(撮影～トリミング実施)
 *
 *	@param eMode 画像取り込みシーケンスモード
 *	@param eBank 取り込む領域
 *	@param iStartX 開始X座標
 *	@param iStartY 開始Y座標
 *	@param iSizeX Xサイズ
 *	@param iSizeY Yサイズ
 *	@param tmout タイムアウト時間
 *	@return OSのエラーコード
 
 //20140829 Miya FPGA Bio
 //20160309 modified T.N For NC社カメラNCM03-V
 */
/*==========================================================================*/
ER CmrCapture2(enum CAP_MODE eMode, enum CAP_BANK eBank, int iStartX, int iStartY, int iSizeX, int iSizeY, TMO tmout)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		CmrTrimStartX( iStartX );				// トリミング開始X座標
		CmrTrimStartY( iStartY );				// トリミング開始Y座標
		CmrTrimSizeX( iSizeX );					// トリミングXサイズ
		CmrTrimSizeY( iSizeY );					// トリミングYサイズ

#if ( NEWCMR == 0 )
		ercd = cmrCapture(eMode, eBank);	// 画像取り込み(GCT社カメラの場合)
		if (ercd == E_OK) {
			ercd = cmrCaptureWait(tmout);	// 取り込み待ち(GCT社カメラの場合)
		}
#else
		ercd = NcmrCapture(eMode, eBank);	// 画像取り込み(NC社カメラNCM03-Vの場合)
		if (ercd == E_OK) {
			ercd = NcmrCaptureWait(tmout);	// 取り込み待ち(NC社カメラNCM03-Vの場合)
		}
#endif
		sig_sem(s_idSem);
	}
	
	return ercd;
}

#if ( NEWCMR == 0 )
/*==========================================================================*/
/**
 * カメラ画像のキャプチャー(本体)
 *
 *	@param eMode 画像取り込みシーケンスモード
 *	@param eBank 取り込む領域
 *	@retval E_OK 取り込みOK
 *	@retval E_PAR パラメーラエラー
 *	@retval E_OBJ キャプチャー実行中エラー
 */
/*==========================================================================*/
static ER cmrCapture(enum CAP_MODE eMode, enum CAP_BANK eBank)
{
	ER ercd;
	
	ercd = E_OK;
	
	CmrCapErrClr();						// エラークリア
	
	if (eMode > CAP_MODE_4) {			// 実装エラー
		ercd = E_PAR;
	}
	
	s_idCapTsk = vget_tid();			// 自タスクを起床タスクに登録
	
	if (IsCmrCapStart()) {				// キャプチャー実行中
		ercd = E_OBJ;
	}
	
	if (ercd == E_OK) {
		vcan_wup();						// 起床要求のクリア
		CmrCapBank(eBank);				// キャプチャーする領域の設定
		CmrCapMode(eMode);				// キャプチャーモードの設定
		CmrCapStart();					// キャプチャー開始
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * カメラ画像のキャプチャー待ち
 *
 *	@param tmout タイムアウト時間
 *	@retval E_OK 取り込みOK
 *	@retval E_TMOUT タイムアウトエラー
 *	@retval E_SYS キャプチャー実行エラー
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
	vcan_wup();							// 起床要求のクリア
	
	return ercd;
}

#else
/*==========================================================================*/
/**
 * カメラ画像のキャプチャー(本体)
 *
 *	@param eMode 画像取り込みシーケンスモード
 *	@param eBank 取り込む領域
 *	@retval E_OK 取り込みOK
 *	@retval E_PAR パラメーラエラー
 *	@retval E_OBJ キャプチャー実行中エラー
 */
/*==========================================================================*/
static ER NcmrCapture( enum CAP_MODE eMode, enum CAP_BANK eBank )
{
	ER ercd;
	
	ercd = E_OK;
	
	if ( eMode >= CAP_MODE_2 ) {		// 実装エラー
		eMode = CAP_MODE_1;				// 生画像撮影（640x480)）
//		ercd = E_PAR;
	}
	
	s_idCapTsk = vget_tid();			// 自タスクを起床タスクに登録
	
	if ( IsNCmrCap() ){					// キャプチャー実行中
		ercd = E_OBJ;
	}
	
	if ( ercd == E_OK ){
		vcan_wup();						// 起床要求のクリア
		NCmrCapAreaSel( eMode );		// キャプチャーモードの設定
		NCmrCapBnkSel( eBank );			// キャプチャーする領域の設定
		NCmrCapWaitSet( 3 );			//	画像取込み時の先頭廃棄フレーム数:0～7
		NCmrCapStart();					// キャプチャー開始
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
	
	fpga_outw( NCMR_CAP_CTR, hbuf2 );	// キャプチャ開始要求
	
	hbuf3 = fpga_inw( NCMR_CAP_CTR );
	Lcnt = 100000;
	while ( IsNCmrCap() ){  			// キャプチャ終了の確認　=0:終了、=1:送信中
		Lcnt--;
		if ( Lcnt <= 0 ) break;
	}
	hbuf4 = fpga_inw( NCMR_CAP_CTR );

}

/*==========================================================================*/
/**
 * カメラ画像のキャプチャー待ち
 *
 *	@param tmout タイムアウト時間
 *	@retval E_OK 取り込みOK
 *	@retval E_TMOUT タイムアウトエラー
 *	@retval E_SYS キャプチャー実行エラー
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
	vcan_wup();							// 起床要求のクリア
	
	return ercd;
}


/*==========================================================================*/
/**
 * カメラ・コマンドのI2C送信
 *
 *	@param pAddr 送信アドレス文字列へのポインタ
 *	@param pData 送信データ文字列へのポインタ
 *	@param iSendSize 送信データサイズ
 *	@return OSのエラーコード
 */
/*==========================================================================*/
ER NCmrI2C_Send( UB *pAddr, UB *pData, int iSendSize )
{
	ER ercd;
	int icnt;
	
	ercd = twai_sem( s_idSem, 1000/MSEC );
	if ( ercd == E_OK ) {
		vcan_wup();											// 起床要求のクリア
		icnt = NcmrI2C_Send( pAddr, pData, iSendSize );		// コマンド送信
		if ( iSendSize != icnt ){
			ercd = E_OBJ;				// データ送信未了
		}
		sig_sem( s_idSem );
	}
	
	return ercd;
}


/*==========================================================================*/
/**
 * カメラ・コマンドのI2C送信(本体)
 *
 *	@param pAddr 送信アドレス文字列へのポインタ
 *	@param pData コマンド文字列へのポインタ
 *	@param iSendSize 送信データサイズ
 *	戻り値 送信成功データサイズ
 */
/*==========================================================================*/
static int NcmrI2C_Send( UB *pAddr, UB *pData, int iSendSize )
{
	ER ercd = E_OK;
	int i;
	long Lcnt;

	// データのI2C送信
	Lcnt = 100000;
	while ( IsNCmrSendEnd() ){  		//	I2C送信中の確認　=0:終了、=1:送信中
		Lcnt--;
		if ( Lcnt <= 0 ){
			return 0;	
		}
	}
		
	for ( i=0; i<iSendSize; i++ ){
		NCmrAddesWrite( pAddr[ i ] );	// I2C送信書込みアドレスをSet。
		NCmrSendDataWrite( pData[ i ] );// I2C送信カメラ・データの書込み
		NCmrSendStart();				// I2C送信開始要求
		Lcnt = 100000;
		while ( IsNCmrSendEnd() ){  	// I2C送信中の確認　=0:終了、=1:送信中
			Lcnt--;
			if ( Lcnt <= 0 ) break;
		}
	}
	return i;
}

/*==========================================================================*/
/**
 * カメラ・コマンドのI2C受信
 *
 *	@param pAddr 受信アドレス文字列へのポインタ
 *	@param pData 受信データ文字列へのポインタ
 *	@param iSendSize 受信データサイズ
 *	@return OSのエラーコード
 */
/*==========================================================================*/
static ER NCmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize )
{
	ER ercd;
	int icnt;
	
	ercd = twai_sem( s_idSem, 1000/MSEC );
	if ( ercd == E_OK ) {
		vcan_wup();											// 起床要求のクリア
		icnt = NcmrI2C_Rcv( pAddr, pData, iRcvSize );		// コマンド受信
		if ( iRcvSize != icnt ){
			ercd = E_OBJ;				// データ受信未了
		}
		sig_sem( s_idSem );
	}
	
	return ercd;
}


/*==========================================================================*/
/**
 * カメラ・コマンドのI2C受信(本体)
 *
 *	@param pAddr 受信アドレス文字列へのポインタ
 *	@param pData 受信データ文字列へのポインタ
 *	@param iSendSize 受信データサイズ
 *	戻り値 送信成功データサイズ
 */
/*==========================================================================*/
static int NcmrI2C_Rcv( UB *pAddr, UB *pData, int iRcvSize )
{
	ER ercd = E_OK;
	int i;
	UH hbuf;
	long Lcnt;

	// データのI2C送信
	Lcnt = 100000;
	while ( IsNCmrReadEnd() ){  		//	I2C送信中の確認　=0:終了、=1:送信中
		Lcnt--;
		nop();
		if ( Lcnt <= 0 ){
			return 0;	
		}
	}
		
	for ( i=0; i<iRcvSize; i++ ){
		NCmrAddesWrite( pAddr[ i ] );	// I2C受信アドレスをSet。
		NCmrReadStart();				// I2C受信開始要求
		Lcnt = 100000;
		while ( IsNCmrReadEnd() ){  	// I2C受信中の確認　=0:終了、=1:送信中
			Lcnt--;
			nop();
			if ( Lcnt <= 0 ) break;
		}
		hbuf = NCmrRcvDataRead();		// I2C受信カメラ・データの読込み
		pData[ i ] = ( UB )hbuf;
	}
	return i;
}

#endif

/*==========================================================================*/
/**
 * コマンド送信
 *
 *	@param pData コマンド文字列へのポインタ
 *	@param iSendSize 送信データサイズ
 *	@param iRcvSize 受信データサイズ
 *	@return OSのエラーコード
 */
/*==========================================================================*/
ER CmrPktSend(UB *pData, int iSendSize, int iRcvSize)
{
	ER ercd;
	
	ercd = twai_sem(s_idSem, 1000/MSEC);
	if (ercd == E_OK) {
		vcan_wup();								// 起床要求のクリア
		cmrPktSend(pData, iSendSize, iRcvSize);	// コマンド送信
		sig_sem(s_idSem);
	}
	
	return ercd;
}

/*==========================================================================*/
/**
 * コマンド送信(本体)
 *
 *	@param pData コマンド文字列へのポインタ
 *	@param iSendSize 送信データサイズ
 *	@param iRcvSize 受信データサイズ
 */
/*==========================================================================*/
static void cmrPktSend(UB *pData, int iSendSize, int iRcvSize)
{
	UH uhCrc;
	int iCnt;
	
	uhCrc = cmrGenCRC(pData, iSendSize);
	
	iCnt = 0;

	// 送信データの書込み
	while(iSendSize) {
		CmrSendDataWrite(iCnt, *pData);
		pData++;
		iCnt++;
		iSendSize--;
	}
	// CRC付加
	CmrSendDataWrite(iCnt, (uhCrc & 0xFF));			iCnt++;
	CmrSendDataWrite(iCnt, ((uhCrc >> 8)& 0xFF));	iCnt++;
	
	CmrCmdPktSize(iCnt);				// 送信データサイズ設定
	CmrRecvPktSize((iRcvSize + 2));		// 受信データサイズ設定
	
	CmrCmdPktSend();					// コマンド送信
}

/*==========================================================================*/
/**
 * 応答受信
 *
 *	@param pData 応答文字列格納先へのポインタ
 *	@param iRcvSize 受信データサイズ
 *	@param tmout タイムアウト
 *	@return OSのエラーコード
 */
/*==========================================================================*/
ER CmrPktRecv(UB *pData, int iRcvSize, TMO tmout)
{
	ER ercd;
	
	ercd = tslp_tsk( tmout );
	if (ercd != E_OK) {
		return ercd;
	}
	
	vcan_wup();							// 起床要求のクリア
	
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
 * 応答送信(本体)
 *
 *	@param pData コマンド文字列へのポインタ
 *	@param iRcvSize 受信データサイズ
 */
/*==========================================================================*/
static BOOL cmrPktRecv(UB *pData, int iRcvSize)
{
	UH uhCrc, uhCrcRcv;
	int iCnt;
	UB *p, ubTmp;
	
	p = pData;
	
	iCnt = 0;

	// 受信データの書込み
	while(iRcvSize) {
		*p = CmrRecvDataRead(iCnt);
		p++;
		iCnt++;
		iRcvSize--;
	}
	// CRCチェック
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
 * カメラ画像を指定バイト数取得する(1ライン内)
 *
 *	@param eBank キャプチャー領域
 *	@param iX X座標(8で割り切れる値)
 *	@param iY Y座標
 *	@param iSize 指定ビット数(8で割り切れるサイズ)
 *	@param p 格納先アドレス
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
 * カメラ画像を指定バイト数取得する(本体)
 *
 *	@param eBank キャプチャー領域
 *	@param iX X座標(8で割り切れる値)
 *	@param iY Y座標
 *	@param iSize 指定ビット数(8で割り切れるサイズ)
 *	@param p 格納先アドレス
 */
/*==========================================================================*/
static void cmrCapLineGet(enum CAP_BANK eBank, int iX, int iY, int iSize, UB *p)
{
	cap_line_get(eBank, iX, iY, iSize, p);
}

/*==========================================================================*/
/**
 * カメラ画像を取得する
 *
 *	@param eBank キャプチャー領域
 *	@param iX X座標(8で割り切れる値)
 *	@param iY Y座標
 *	@param iXsize 指定Xビット数(8で割り切れるサイズ)
 *	@param iYsize 指定Yビット数
 *	@param p 格納先アドレス
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
 * カメラ画像を取得する(本体)
 *
 *	@param eBank キャプチャー領域
 *	@param iX X座標(8で割り切れる値)
 *	@param iY Y座標
 *	@param iXsize 指定Xビット数(8で割り切れるサイズ)
 *	@param iYsize 指定Yビット数
 *	@param p 格納先アドレス
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
 * CRC16を求める
 *
 *	@param lpBuf バッファへのポインタ
 *	@param sizeOfBuf バッファサイズ
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
 * 画像トリミングの実行
 *
 *	@param eOrg キャプチャー画像
 *	@param eDist トリミング画像格納先
 *	@param iStartX 開始X座標
 *	@param iStartY 開始Y座標
 *	@param iSizeX Xサイズ
 *	@param iSizeY Yサイズ
 *	@param tmout タイムアウト時間
 *	@return OSのエラーコード
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
 * 画像トリミングの実行(本体)
 *
 *	@param eOrg キャプチャー画像
 *	@param eDist トリミング画像格納先
 *	@param iStartX 開始X座標
 *	@param iStartY 開始Y座標
 *	@param iSizeX Xサイズ
 *	@param iSizeY Yサイズ
 *	@param tmout タイムアウト時間
 *	@return OSのエラーコード
 */
/*==========================================================================*/
static ER cmrTrim(enum CAP_BANK eOrg, enum TRIM_BANK eDist, int iStartX, int iStartY,
			int iSizeX, int iSizeY, TMO tmout)
{
	// パラメータチェック
	if (eOrg > CAP_BANK_3) {
		return E_PAR;
	}
	if (eDist > TRIM_BANK_3) {
		return E_PAR;
	}
	// 実行中チェック
	if (IsCmrTrimStart()) {
		return E_OBJ;
	}
	CmrTrimOrg( eOrg );						// キャプチャー画像指定
	CmrTrimDist( eDist );					// トリミング画像保存領域
#if 1
	CmrTrimStartX( iStartX );				// トリミング開始X座標
	CmrTrimStartY( iStartY );				// トリミング開始Y座標
	CmrTrimSizeX( iSizeX );					// トリミングXサイズ
	CmrTrimSizeY( iSizeY );					// トリミングYサイズ
#else
	fpga_outw( TRIM_ST_X, iStartX );		///< 画像トリミング開始X座標
	fpga_outw( TRIM_ST_Y, iStartY );		///< 画像トリミング開始Y座標
	fpga_outw( TRIM_SZ_X, iSizeX );			///< 画像トリミングXサイズ
	fpga_outw( TRIM_SZ_Y, iSizeY );			///< 画像トリミングYサイズ
#endif	
	
	CmrTrimStart();							// トリミング開始
	nop();
	
	if (tmout != TMO_FEVR) {
		while(IsCmrTrimStart()) {					// トリミング完了まで待つ
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
 * トリミング画像を取得する
 *
 *	@param eBank トリミング画像領域
 *	@param lStart 取得開始位置
 *	@param lSize 取得データサイズ
 *	@param p 格納先アドレス
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
 * トリミング画像を取得する(本体)
 *
 *	@param eBank トリミング画像領域
 *	@param lStart 取得開始位置
 *	@param lSize 取得データサイズ
 *	@param p 格納先アドレス
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
 * 画像圧縮の実行
 *
 *	@param eOrg トリミング画像
 *	@param eDist 圧縮画像格納先
 *	@param eMode 圧縮モード
 *	@param iSizeX リサイズする画像のXサイズ
 *	@param iSizeY リサイズする画像のYサイズ
 *	@param tmout タイムアウト時間
 *	@return OSのエラーコード
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
 * 画像圧縮の実行(本体)
 *
 *	@param eOrg トリミング画像
 *	@param eDist 圧縮画像格納先
 *	@param eMode 圧縮モード
 *	@param iSizeX リサイズする画像のXサイズ
 *	@param iSizeY リサイズする画像のYサイズ
 *	@param tmout タイムアウト時間
 *	@return OSのエラーコード
 */
/*==========================================================================*/
static ER cmrResize(enum TRIM_BANK eOrg, enum RSZ_BANK eDist, enum RSZ_MODE eMode,
				int iSizeX, int iSizeY, TMO tmout)
{
	volatile int iX, iY;
	
	// パラメータチェック
	if (eOrg > TRIM_BANK_3) {
		return E_PAR;
	}
	if (eDist > RSZ_BANK_3) {
		return E_PAR;
	}
	// 実行中チェック
	if (IsCmrResizeStart()) {
		return E_OBJ;
	}
	
	iX = iSizeX;
	iY = iSizeY;
	
	CmrResizeOrg( eOrg );					// トリミング画像指定
	CmrResizeDist( eDist );					// 圧縮画像保存領域
	CmrResizeSizeX( iX );					// 圧縮画像のXサイズ
	CmrResizeSizeY( iY );					// 圧縮画像のYサイズ
	CmrResizeMode( eMode );					// 圧縮モードの設定
	
	CmrResizeStart();						// 圧縮開始
	
	if (tmout != TMO_FEVR) {
		while(IsCmrResizeStart()) {			// 圧縮完了まで待つ
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
 * 圧縮画像を取得する
 *
 *	@param eBank トリミング画像領域
 *	@param lStart 取得開始位置
 *	@param lSize 取得データサイズ
 *	@param p 格納先アドレス
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
 * 圧縮画像を取得する(本体)
 *
 *	@param eBank 圧縮画像領域
 *	@param lStart 取得開始位置
 *	@param lSize 取得データサイズ
 *	@param p 格納先アドレス
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
 * カメラ画像を取得する
 *
 *	@param eBank キャプチャー領域
 *	@param iX X座標(8で割り切れる値)
 *	@param iY Y座標
 *	@param iXsize 指定Xビット数(8で割り切れるサイズ)
 *	@param iYsize 指定Yビット数
 *	@param p 格納先アドレス
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
 * カメラ画像を取得する(本体)
 *
 *	@param eBank キャプチャー領域
 *	@param iX X座標(8で割り切れる値)
 *	@param iY Y座標
 *	@param iXsize 指定Xビット数(8で割り切れるサイズ)
 *	@param iYsize 指定Yビット数
 *	@param p 格納先アドレス
 */
/*==========================================================================*/
static void cmrMemWrt(enum CAP_BANK eBank, int iX, int iY, int iSize, UB *p)
{
	cap_dat_wrt(eBank, iX, iY, iSize, p);
	
}




