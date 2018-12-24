/**
*	VA-300プログラム
*
*	@file drv_fpga.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/8/31
*	@brief  FPGA関連定義情報(作成中)
*
*	Copyright (C) 2011, OYO Electric Corporation
*/
#ifndef	_DRV_FPGA_H_
#define	_DRV_FPGA_H_

/// @name 型定義
//@{
typedef unsigned short FPGA_U16;				// 
typedef unsigned long  FPGA_U32;				// 

enum BIT_CRL {									///< ビット制御
	BIT_CRL_OFF,								///< ビットOFF
	BIT_CRL_ON									///< ビットON
};

enum IO_ON_OFF {								///< I/O のON/OFF定義
	IO_OFF,										///< OFF
	IO_ON										///< ON
};

enum CMR_BAU {									///< カメラ通信I/Fボーレート
	CMR_BAU4800  = 0x00,						///< 4800bps
	CMR_BAU9600,								///< 9600bps
	CMR_BAU19200,								///< 19200bps
	CMR_BAU38400,								///< 38400bps
	CMR_BAU57600,								///< 57600bps
	CMR_BAU115200,								///< 115200bps
	CMR_BAU2400  = 0x0F,						///< 2400bps
};

//@}
/// @name 定義
//@{
#define	CAP_X_SIZE		1280					///< Xサイズ
#define	CAP_Y_SIZE		720						///< Yサイズ
//@}
/// @name レジスタ定義
//@{
#define	FPGA_BASE		0xB0000000				///< FPGAレジスタのベースアドレス
#define	CAP_BASE		0xB4000000				///< 画像データのベースアドレス

// レジスタ直接アクセス
#define	fpga_inw(n)		(*(volatile FPGA_U16 *)(FPGA_BASE + (n)))			///< FPGAレジスタの16bit読出し
#define	fpga_outw(n,c)	(*(volatile FPGA_U16 *)(FPGA_BASE + (n)) = (c))		///< FPGAレジスタの16bit書込み
#define	fpga_inl(n)		(*(volatile FPGA_U32 *)(FPGA_BASE + (n)))			///< FPGAレジスタの32bit読出し
#define	fpga_outl(n,c)	(*(volatile FPGA_U32 *)(FPGA_BASE + (n)) = (c))		///< FPGAレジスタの32bit書込み
#define	fpga_setw(n,c)	(*(volatile FPGA_U16 *)(FPGA_BASE + (n)) |= (c))	///< FPGAレジスタのbitセット(16bit)
#define	fpga_clrw(n,c)	(*(volatile FPGA_U16 *)(FPGA_BASE + (n)) &= ~(c))	///< FPGAレジスタのbitクリア(16bit)
#define	fpga_setl(n,c)	(*(volatile FPGA_U32 *)(FPGA_BASE + (n)) |= (c))	///< FPGAレジスタのbitセット(32bit)
#define	fpga_clrl(n,c)	(*(volatile FPGA_U32 *)(FPGA_BASE + (n)) &= ~(c))	///< FPGAレジスタのbitクリア(32bit)
#define	cap_dat(b,n)	(*(volatile UB *)(CAP_BASE + (b) * 0x00100000 + (n)))	///< 画像データ読出し
#define	trim_dat(b,n)	(cap_dat((b+4),n))		///< トリミング画像データ読出し
#define	rsz_dat(b,n)	(cap_dat((b+8),n))		///< 圧縮画像データ読出し
//#define	wrt_dat(b,n,c)	(*(volatile UB *)(CAP_BASE + (b) * 0x00100000 + (n)) = (c))	///< 画像データ読出し
#define	wrt_dat(b,n,c)	(*(volatile UB *)(FPGA_BASE + (n)) = (c))	///< 画像データ読出し
// 一般
#define	FPGA_VER		0x0000000				///< FPGAバージョンレジスタ
#define	SENS_MON		0x0000002				///< 生態検知センサの状態をモニターする
#define	AUX_OUT			0x0000004				///< AUX OUTの制御
#define	AUX_IN			0x0000006				///< AUX INの状態
#define	TPL_CRL			0x0000008				///< タッチパネルコントロール
#define	TPL_CBYTE		0x000000A				///< タッチパネルコントローラへのCONTROL BYTEレジスタ
#define	TPL_DBYTE		0x000000C				///< タッチパネルコントローラへのDATA BYTEレジスタ

#define	FROM_BANK		0x0000010				///< フラッシュメモリのバンク選択
#define	INT_CRL			0x0000012				///< 割込み制御
#define	LED_CRL			0x0000014				///< LED制御
#define	BUZ_CRL			0x0000016				///< ブザー制御
#define	BUZ1_CYC		0x0000018				///< ブザー周期設定

#define	SEG_CRL			0x0000020				///< 7SEG出力
#define	SEG1_CRL		0x0000020				///< 7SEG1出力(最上位)
#define	SEG2_CRL		0x0000022				///< 7SEG2出力
#define	SEG3_CRL		0x0000024				///< 7SEG3出力
#define	SEG4_CRL		0x0000026				///< 7SEG4出力
#define	SEG5_CRL		0x0000028				///< 7SEG5出力
#define	SEG6_CRL		0x000002A				///< 7SEG6出力
#define	SEG7_CRL		0x000002C				///< 7SEG7出力
#define	SEG8_CRL		0x000002E				///< 7SEG8出力(最下位)

#define	KEY_IN			0x0000030				///< キーボード
#define	IRLED_CRL		0x0000040				///< 赤外線LED
#define	IRLED_DUTY		0x0000042				///< 赤外線LEDのDuty
#define	IRLED1_DUTY		0x0000042				///< 赤外線LED1のDuty
#define	IRLED2_DUTY		0x0000044				///< 赤外線LED2のDuty
#define	IRLED3_DUTY		0x0000046				///< 赤外線LED3のDuty
#define	IRLED4_DUTY		0x0000048				///< 赤外線LED4のDuty
#define	IRLED5_DUTY		0x000004A				///< 赤外線LED5のDuty

#define	CNF_ROM			0x0000050				///< FPGAコンフィグレーションメモリ
#define FPGA_RECONF		0x000005E				///< FPGAリコンフィグレーションメモリ

// GCT社・カメラ関連 
#define	CMR_CAP			0x0000100				///< カメラキャプチャー制御
#define	CMR_PRM_AES		0x0000102				///< カメラのAES制御の値
#define	CMR_PRM_SHT		0x0000104				///< カメラのFix Shutter Controlの値
#define	CMR_MOD_WAIT	0x0000106				///< カメラの画像取り込み時Mode4のWait時間
#define	CMR_WAKEUP_WAIT	0x0000108				///< カメラのWakeUp後のWait時間	.... 追加　永井　2013.4.12
#define	CMR_CRL			0x0000200				///< カメラのSerial I/F制御
#define	CMR_BAU			0x0000202				///< カメラ通信のボーレート
#define	CMR_CSZ			0x0000204				///< カメラのコマンドパケットサイズ
#define	CMR_CMON		0x0000206				///< カメラのコマンド通信状態監視
#define	CMR_RSZ			0x0000208				///< カメラの応答パケットサイズ
#define	CMR_RMON		0x000020A				///< カメラの応答通信状態監視

#define	CMR_CMD			0x0000400				///< カメラのコマンドデータ
#define	CMR_RES			0x0000600				///< カメラの応答データ
// GCT社・カメラ関連 end.

#define	TRIM_CRL		0x0000210				///< 画像トリミング制御
#define	TRIM_BNK		0x0000212				///< 画像トリミングの画像選択、格納場所指定
#define	TRIM_ST_X		0x0000214				///< 画像トリミング開始X座標
#define	TRIM_ST_Y		0x0000216				///< 画像トリミング開始Y座標
#define	TRIM_SZ_X		0x0000218				///< 画像トリミングXサイズ
#define	TRIM_SZ_Y		0x000021A				///< 画像トリミングYサイズ

#define	RSZ_CRL			0x0000220				///< 画像圧縮制御
#define	RSZ_BNK			0x0000222				///< 画像圧縮の選択及び格納場所指定
#define	RSZ_SZ_X		0x0000224				///< 画像圧縮のXサイズ
#define	RSZ_SZ_Y		0x0000226				///< 画像圧縮のYサイズ

// NC社・NCM03-Vカメラ関連	/* Added T.N 2016.3.8 */
#define NCMR_SERAL_CTRL	0x0000300				///< NC社カメラ I2Cコントロール	用fpgaレジスタ・アドレス
#define NCMR_SERAL_ADDR	0x0000302				///< NC社カメラ I2Cアドレス用fpgaレジスタ・アドレス
#define NCMR_SERAL_WDATA	0x0000304			///< NC社カメラ I2Cライト・データ用fpgaレジスタ・アドレス
#define NCMR_SERAL_RDATA	0x0000306			///< NC社カメラ I2Cリード・データ用fpgaレジスタ・アドレス

#define NCMR_CRL_SND 	0x0001					///< カメラI2C送信開始要求
#define NCMR_CRL_RCV 	0x0002					///< カメラI2C受信開始要求

#define NCMR_CAP_CTR	0x0000100				///< カメラ・キャプチャ・コントロール用fpgaレジスタ・アドレス
#define NCMR_CAP_STA	0x0001					///< キャプチャ開始要求　=0:終了、=1:開始
#define NCMR_CAP_SEL	0x0002					///< 画像格納エリア種別　=0:トリミングエリア、=1:キャプチャエリア(全画面640x480)
#define NCMR_CAP_BNK	0x0300					///< 画像格納バンク選択  =(00):CapturBank0 又は TrimArea0
												///< 					 =(01):CapturBank1 又は TrimArea1
												///< 					 =(10):CapturBank2 又は TrimArea2
												///< 					 =(11):CapturBank3 又は TrimArea3
#define NCMR_CAP_WAIT	0x7000					///< 画像取込み時の先頭廃棄フレーム数:0～7
// NC社・NCM03-Vカメラ関連 end.	/* Added END */


#define	CMR_HGM			0x0001000				///< カメラ画像のヒストグラム
//@}

/// @name レジスタビット定義
//@{
// ビット定義
#define	SENS_MON_ON		0x0001					///< 指の挿入状態

#define	AUX_OUT_1		0x0001					///< AUX OUT1
#define	AUX_OUT_2		0x0002					///< AUX OUT2

#define	AUX_IN_A0		0x0001					///< AUX IN A0
#define	AUX_IN_A1		0x0002					///< AUX IN A1
#define	AUX_IN_B0		0x0004					///< AUX IN B0
#define	AUX_IN_B1		0x0008					///< AUX IN B1

#define	TPL_CRL_WRC		0x0001					///< タッチパネルにCONTROLL BYTE設定
#define	TPL_CRL_WRD		0x0002					///< タッチパネルにDATA BYTE設定
#define	TPL_CRL_RD		0x0004					///< タッチパネルに読出し設定

#define	INT_CRL_LAN		0x0001					///< LANの割込み許可
#define	INT_CRL_SENS	0x0002					///< タッチセンサの割込み許可
#define	INT_CRL_TPL		0x0004					///< タッチパネルの割込み許可
#define	INT_CRL_LCD		0x0008					///< LCDの割込み許可
#define	INT_CRL_CAP		0x0010					///< カメラのキャプチャ割込み許可
#define	INT_CRL_KEY		0x0020					///< 10キーの割込み許可
#define	INT_CRL_AUXIN	0x0040					///< AUXINの割込み許可
#define	INT_CRL_TPL_CMD	0x0080					///< タッチパネルのコマンド割込み許可
#define	INT_CRL_CMR_CMD	0x0100					///< カメラのコマンド割込み許可
#define	INT_CRL_CMR_RES	0x0200					///< カメラの受信割込み許可

#define	LED_CRL_PWR		0x0001					///< 緑色LED
#define	LED_CRL_OK		0x0002					///< 橙色LED
#define	LED_CRL_NG		0x0004					///< 赤色LED

#define	BUZ_CRL_BZ1		0x0001					///< ブザー1制御
#define	BUZ_CRL_BZ2		0x0002					///< ブザー2制御

#define	KEY_IN_0		0x0001					///< '0'のキー
#define	KEY_IN_1		0x0002					///< '1'のキー
#define	KEY_IN_2		0x0004					///< '2'のキー
#define	KEY_IN_3		0x0008					///< '3'のキー
#define	KEY_IN_4		0x0010					///< '4'のキー
#define	KEY_IN_5		0x0020					///< '5'のキー
#define	KEY_IN_6		0x0040					///< '6'のキー
#define	KEY_IN_7		0x0080					///< '7'のキー
#define	KEY_IN_8		0x0100					///< '8'のキー
#define	KEY_IN_9		0x0200					///< '9'のキー
#define	KEY_IN_ENTER	0x0400					///< 呼出キー
#define	KEY_IN_ASTERISK	0x0800					///< '*'キー
#define	KEY_IN_CLR		0x1000					///< 取消キー
#define	KEY_IN_NONE		0x0000					///< 何も押されていない

#define	IRLED_CRL_LED1	0x0001					///< IR LED1
#define	IRLED_CRL_LED2	0x0002					///< IR LED2
#define	IRLED_CRL_LED3	0x0004					///< IR LED3
#define	IRLED_CRL_LED4	0x0008					///< IR LED4
#define	IRLED_CRL_LED5	0x0010					///< IR LED5

#define	CMR_CAP_CRL		0x0001					///< カメラキャプチャ制御
#define	CMR_CAP_MOD		0x0010					///< カメラキャプチャモード
#define	CMR_CAP_BANK	0x0100					///< カメラキャプチャバンク
#define	CMR_CAP_ERR		0x8000					///< カメラエラーフラグ

#define	CMR_MOD_0		0x0000					///< カメラモード0
#define	CMR_MOD_1		0x0001					///< カメラモード1
#define	CMR_MOD_2		0x0002					///< カメラモード2
#define	CMR_MOD_3		0x0003					///< カメラモード3
#define	CMR_MOD_4		0x0004					///< カメラモード4

#define	CMR_BANK_0		0x0000					///< カメラバンク0
#define	CMR_BANK_1		0x0001					///< カメラバンク1
#define	CMR_BANK_2		0x0002					///< カメラバンク2
#define	CMR_BANK_3		0x0003					///< カメラバンク3

#define	TRIM_CRL_ST		0x0001					///< 画像トリミング開始
#define	TRIM_BNK_ORG	0x0001					///< トリミングの画像選択
#define	TRIM_BNK_DST	0x0004					///< トリミングデータの格納場所

#define	RSZ_CRL_ST		0x0001					///< リサイズ開始
#define	RSZ_CRL_MOD		0x0002					///< リサイズのモード
#define	RSZ_BNK_ORG		0x0001					///< リサイズの画像選択
#define	RSZ_BNK_DST		0x0004					///< リサイズデータの格納場所

#define	CMR_CRL_ACT		0x0001					///< カメラ通信制御

//@}

/// その他定義

// EXTERNの定義
#if defined(EXTERN)
#undef EXTERN
#endif

#if defined(_DRV_FPGA_C_)
#define	EXTERN
#else
#define	EXTERN	extern
#endif
/// @name プロトタイプ宣言
//@{
EXTERN ER FpgaInit(void);								///< FPGA初期化
EXTERN ER FpgaSetWord(int iReg, UH uhVal, UH uhMask);	///< FPGAに値設定
EXTERN void FpgaCapLineGet(int iBank, int iX, int iY, int iSize, UB *p);	///< 画像データの1ライン取得
EXTERN void FpgaIrLedSet(UB ubSel, UB ubVal);			///< 赤外線LEDのDuty比設定
EXTERN int FpgaRegDisplayLine(int iLine, char* p);		///< FPGAレジスタ情報
EXTERN void FpgaWrtDat(int iBank, int iX, int iY, int iSize, UB *p);	//Miya
//@}
#endif									/* end of _DRV_FPGA_H_				*/
