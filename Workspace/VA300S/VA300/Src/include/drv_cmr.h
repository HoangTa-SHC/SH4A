/**
*	VA-300 センサー部プログラム
*
*	@file drv_cmr.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/24
*	@brief  カメラ定義情報
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_DRV_CMR_H_
#define	_DRV_CMR_H_

// 定義
enum CMR_CMD_DEF {						///< カメラコマンド
	CMR_CMD_REQUEST_RESPONSE = 0x10,	///< Request Response
	CMR_CMD_SET_ID           = 0x19,	///< Set ID
	CMR_CMD_GET_PARAMETERS   = 0x20,	///< Get Parameters
	CMR_CMD_LOAD_PARAMETERS  = 0x27,	///< Load Parameters
};

enum CAP_MODE {							///< キャプチャーモード
	CAP_MODE_0 = CMR_MOD_0,				///< モード0
	CAP_MODE_1 = CMR_MOD_1,				///< モード1
	CAP_MODE_2 = CMR_MOD_2,				///< モード2
	CAP_MODE_3 = CMR_MOD_3,				///< モード3
	CAP_MODE_4 = CMR_MOD_4,				///< モード4
};

enum CAP_BANK {							///< キャプチャー領域のバンク指定
	CAP_BANK_0 = CMR_BANK_0,			///< BANK0
	CAP_BANK_1 = CMR_BANK_1,			///< BANK1
	CAP_BANK_2 = CMR_BANK_2,			///< BANK2
	CAP_BANK_3 = CMR_BANK_3,			///< BANK3
};

enum TRIM_BANK {						///< トリミング画像の格納先
	TRIM_BANK_0 = CMR_BANK_0,			///< BANK0
	TRIM_BANK_1 = CMR_BANK_1,			///< BANK1
	TRIM_BANK_2 = CMR_BANK_2,			///< BANK2
	TRIM_BANK_3 = CMR_BANK_3,			///< BANK3
};

enum RSZ_BANK {							///< 圧縮画像の格納先
	RSZ_BANK_0 = CMR_BANK_0,			///< BANK0
	RSZ_BANK_1 = CMR_BANK_1,			///< BANK1
	RSZ_BANK_2 = CMR_BANK_2,			///< BANK2
	RSZ_BANK_3 = CMR_BANK_3,			///< BANK3
};

enum RSZ_MODE {							///< 圧縮画像の縮小率
	RSZ_MODE_0 = CMR_BANK_0,			///< 辺1/2
	RSZ_MODE_1 = CMR_BANK_1,			///< 辺1/4
	RSZ_MODE_2 = CMR_BANK_2,			///< 辺1/8
};

// 取得画像サイズの定義
#define	GET_IMG_SIZE_X	640				///< 取得画像Xサイズ
#define	GET_IMG_SIZE_Y	360				///< 取得画像Yサイズ

// マクロ定義
// NC社・NCM03-Vカメラ関連	/* Added T.N 2016.3.8 */
#define	NCmrAddesWrite( n )		fpga_outw( NCMR_SERAL_ADDR, n )		///< I2C送信カメラ・アドレスの書込み
#define	NCmrSendDataWrite( n )	fpga_outw( NCMR_SERAL_WDATA, n )	///< I2C送信カメラ・データの書込み
#define	IsNCmrSendEnd()  		( fpga_inw( NCMR_SERAL_CTRL ) & NCMR_CRL_SND )	///< NCameraI2C送信中の確認　=0:終了、=1:送信中
#define	NCmrSendStart()			fpga_setw( NCMR_SERAL_CTRL, NCMR_CRL_SND )		///< NCameraI2C送信開始要求
#define	NCmrRcvDataRead()		fpga_inw( NCMR_SERAL_RDATA & 0xffff )			///< I2C受信カメラ・データの読込み
#define	IsNCmrReadEnd()  		( fpga_inw( NCMR_SERAL_CTRL ) & NCMR_CRL_RCV )	///< NCameraI2C受信中の確認　=0:終了、=1:送信中
#define	NCmrReadStart()			fpga_setw( NCMR_SERAL_CTRL, NCMR_CRL_RCV )		///< NCameraI2C受信開始要求

//#define NCmrCapStart()			fpga_outw( NCMR_CAP_CTR, ( fpga_inw( NCMR_CAP_CTR ) | NCMR_CAP_STA ) )	///< キャプチャ開始要求
//#define NCmrCapStart()			fpga_setw( NCMR_CAP_CTR, NCMR_CAP_STA ) 	///< キャプチャ開始要求
#define IsNCmrCap()				( fpga_inw( NCMR_CAP_CTR ) & NCMR_CAP_STA )		///< キャプチャ画像処理状態　=0:終了、=1:処理中
#define NCmrCapAreaSel( n )		fpga_outw( NCMR_CAP_CTR, ( fpga_inw( NCMR_CAP_CTR ) & ~( NCMR_CAP_SEL ) ) | ( ( n << 1 ) & NCMR_CAP_SEL ) )	///< 画像格納エリア種別　=0:トリミングエリア、=1:キャプチャエリア(全画面640x480)
#define NCmrCapBnkSel( n )		fpga_outw( NCMR_CAP_CTR, ( fpga_inw( NCMR_CAP_CTR ) & ~( NCMR_CAP_BNK ) ) | ( ( n << 8 ) & NCMR_CAP_BNK ) )
																	///< 画像格納バンク選択  =0:CapturBank0 又は TrimArea0
																	///< 					 =1:CapturBank1 又は TrimArea1
																	///< 					 =2:CapturBank2 又は TrimArea2
																	///< 					 =3:CapturBank3 又は TrimArea3
#define NCmrCapWaitSet( n )		fpga_outw( NCMR_CAP_CTR, ( fpga_inw( NCMR_CAP_CTR ) & ~( NCMR_CAP_WAIT ) ) | ( ( n << 12 ) & NCMR_CAP_WAIT ) )
																	///< 画像取込み時の先頭廃棄フレーム数:0～7

// NCM03-Vカメラ関連 end.	/* Added END */

// GCT社・カメラ関連
#define	CmrPrmAesSet(n)		fpga_outw(CMR_PRM_AES, n)		///< パラメータAES設定
#define	CmrPrmShutterSet(n)	fpga_outw(CMR_PRM_SHT, n)		///< パラメータFix Shutter Control設定
#define	CmrWaitCrl(n)		fpga_outw(CMR_MOD_WAIT, n)		///< 画像取り込み時Mode4のWait設定
#define	CmrWakeUpWaitCrl(n)		fpga_outw(CMR_WAKEUP_WAIT, n)	///< カメラWakeUp時のWait設定.... 追加　永井　2013.4.12
#define	CmrBaudrateSet(n)	fpga_outw(CMR_BAU, n)			///< ボーレートの設定
#define	CmrHistogramGet(b,n)	fpga_inl((CMR_HGM + b * 0x400 + n * 4))		///< ヒストグラムデータ
#define	CmrCmdPktSend()		fpga_setw(CMR_CRL, CMR_CRL_ACT)	///< コマンド送信
#define	CmrCmdPktSize(n)	fpga_outw(CMR_CSZ, n)			///< 送信コマンドサイズの設定
#define	CmrRecvPktSize(n)	fpga_outw(CMR_RSZ, n)			///< 受信パケットサイズの設定
#define	CmrSendDataWrite(n,v)	fpga_outw((CMR_CMD + (n << 1)), v)	///< 送信データの書込み
#define	CmrRecvDataRead(n)	fpga_inw((CMR_RES + (n << 1)))	///< 受信データの読込み
#define	CmrCapMode(n)		fpga_outw(CMR_CAP, ((fpga_inw(CMR_CAP) & ~(CMR_CAP_MOD  * 0x07)) | (n * CMR_CAP_MOD)))	///< キャプチャーモード設定
#define	CmrCapBank(n)		fpga_outw(CMR_CAP, ((fpga_inw(CMR_CAP) & ~(CMR_CAP_BANK * 0x03)) | (n * CMR_CAP_BANK)))	///< キャプチャーバンク設定
#define	IsCmrCapErr()		((fpga_inw(CMR_CAP) & CMR_CAP_ERR) == CMR_CAP_ERR)	///< キャプチャーエラー発生中？
#define	CmrCapErrClr()		fpga_clrw(CMR_CAP, CMR_CAP_ERR)	///< キャプチャーエラークリア
#define	IsCmrCapStart()		((fpga_inw(CMR_CAP) & CMR_CAP_CRL) == CMR_CAP_CRL)	///< キャプチャー実行中？
#define	CmrCapStart()		fpga_setw(CMR_CAP, CMR_CAP_CRL)	///< キャプチャー開始
// GCT社・カメラ関連 end.

#define	CmrTrimOrg(n)		fpga_outw(TRIM_BNK, ((fpga_inw(TRIM_BNK) & ~(TRIM_BNK_ORG * 0x03)) | (n * TRIM_BNK_ORG)))	///< トリミングの画像選択
#define	CmrTrimDist(n)		fpga_outw(TRIM_BNK, ((fpga_inw(TRIM_BNK) & ~(TRIM_BNK_DST * 0x03)) | (n * TRIM_BNK_DST)))	///< トリミング画像格納先選択
#define	CmrTrimStartX(n)	fpga_outw(TRIM_ST_X, n)			///< 画像トリミング開始X座標
#define	CmrTrimStartY(n)	fpga_outw(TRIM_ST_Y, n)			///< 画像トリミング開始Y座標
#define	CmrTrimSizeX(n)		fpga_outw(TRIM_SZ_X, n)			///< 画像トリミングXサイズ
#define	CmrTrimSizeY(n)		fpga_outw(TRIM_SZ_Y, n)			///< 画像トリミングYサイズ
#define	IsCmrTrimStart()	(fpga_inw(TRIM_CRL) & TRIM_CRL_ST)	///< 画像トリミング実行中
#define	CmrTrimStart()		fpga_setw(TRIM_CRL, TRIM_CRL_ST)	///< 画像トリミング開始
#define	CmrTrimSizeXGet()	fpga_inw(TRIM_SZ_X)				///< 画像トリミングXサイズ取得
#define	CmrTrimSizeYGet()	fpga_inw(TRIM_SZ_Y)				///< 画像トリミングYサイズ取得

#define	CmrResizeMode(n)	fpga_outw(RSZ_CRL, ((fpga_inw(RSZ_CRL) & ~(RSZ_CRL_MOD * 0x03)) | (n * RSZ_CRL_MOD)))	///< リサイズの画像選択
#define	CmrResizeOrg(n)		fpga_outw(RSZ_BNK, ((fpga_inw(RSZ_BNK) & ~(RSZ_BNK_ORG * 0x03)) | (n * RSZ_BNK_ORG)))	///< リサイズの画像選択
#define	CmrResizeDist(n)	fpga_outw(RSZ_BNK, ((fpga_inw(RSZ_BNK) & ~(RSZ_BNK_DST * 0x03)) | (n * RSZ_BNK_DST)))	///< リサイズの画像選択
#define	CmrResizeSizeX(n)	fpga_outw(RSZ_SZ_X, n)			///< リサイズする画像のXサイズ
#define	CmrResizeSizeY(n)	fpga_outw(RSZ_SZ_Y, n)			///< リサイズする画像のYサイズ
#define	IsCmrResizeStart()	(fpga_inw(RSZ_CRL) & RSZ_CRL_ST)	///< 画像リサイズ実行中
#define	CmrResizeStart()	fpga_setw(RSZ_CRL, RSZ_CRL_ST)	///< 画像リサイズ開始

// EXTERN宣言の定義
#ifdef EXTERN
#undef EXTERN
#endif

#if defined(_DRV_CMR_C_)
#define	EXTERN
#else
#define	EXTERN extern
#endif

// 画像取得チェック用バッファ
EXTERN UB g_ubCapTest[ CAP_X_SIZE * CAP_Y_SIZE ];	///< 画像取得チェック用バッファ

// プロトタイプ宣言
EXTERN ER CmrInit(ID idSem, ID idRcvTsk);		///< カメラ関連初期化
EXTERN ER CmrPktSend(UB *pData, int iSendSize, int iRcvSize);	///< カメラ通信I/F 送信
EXTERN ER CmrPktRecv(UB *pData, int iRcvSize, TMO tmout);	///< カメラ通信I/F 受信
EXTERN ER CmrCapture(enum CAP_MODE eMode, enum CAP_BANK eBank, TMO tmout);	///< カメラ画像のキャプチャー
EXTERN ER CmrCapture2(enum CAP_MODE eMode, enum CAP_BANK eBank, int iStartX, int iStartY, int iSizeX, int iSizeY, TMO tmout);
EXTERN ER CmrCapGet(enum CAP_BANK eBank, int iX, int iY, int iXsize, int iYsize, UB *p);	///< カメラ画像の取得
EXTERN ER CmrTrim(enum CAP_BANK eOrg, enum TRIM_BANK eDist, int iStartX, int iStartY, int iSizeX, int iSizeY, TMO tmout); ///< 画像のトリミング
EXTERN ER CmrTrimGet(enum TRIM_BANK eBank, long lStart, long lSize, UB *p);	///< トリミング画像の取得
EXTERN ER CmrResize(enum TRIM_BANK eOrg, enum RSZ_BANK eDist, enum RSZ_MODE eMode, int iSizeX, int iSizeY, TMO tmout);		///< 画像圧縮
EXTERN ER CmrResizeGet(enum RSZ_BANK eBank, long lStart, long lSize, UB *p);
EXTERN ER CmrCmdTest( void );
EXTERN ER CmrMemWrt(enum CAP_BANK eBank, int iX, int iY, int iXsize, int iYsize, UB *p);
#endif										/* end of _DRV_CMR_H_				*/
