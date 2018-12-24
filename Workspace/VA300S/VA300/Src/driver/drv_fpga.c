//=============================================================================
/**
 *
 * VA-300プログラム
 * <<FPGAドライバモジュール>>
 *
 *	@brief FPGAへのアクセスドライバ。LEDなど固有のものは除外。
 *	
 *	@file drv_fpga.c
 *	
 *	@date	2012/08/31
 *	@version 1.00 新規作成
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#define	_DRV_FPGA_C_
#include "kernel.h"
#include "va300.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"
#include "sh7750.h"

// 定義


// 外部変数


// プロトタイプ宣言


// 変数宣言
static ID s_idTsk;						// 起床タスクID

//=============================================================================
/**
 * FPGA関連の初期化
 *
 * @retval E_OK 初期化成功
 */
//=============================================================================
ER FpgaInit(void)
{
	ER ercd;
	
	// FPGAのレジスタ初期化
	ercd = FpgaRegInit();
	
	// FPGA割込み設定(それぞれのドライバで設定するかも)

	
	return ercd;
}

//=============================================================================
/**
 * FPGAレジスタの初期化
 *
 * @retval E_OK 成功
 */
//=============================================================================
ER FpgaRegInit(void)
{
	ER ercd;

	ercd = E_OK;
	
	return ercd;
}

//=============================================================================
/**
 * FPGAのバージョン取得
 *
 * @param puhMajor メジャーバージョンの格納先
 * @param puhMinor マイナーバージョンの格納先
 * @param puhRelease リリースバージョンの格納先
 */
//=============================================================================
void FpgaGetVer(UH* puhMajor, UH* puhMinor, UH* puhRelease)
{
	ER	ercd;
	
	ercd = twai_sem(SEM_FPGA, 1000/MSEC);
	if (ercd != E_OK) {
		return;
	}
	*puhMajor   = (fpga_inw(FPGA_VER) >> 12) & 0x000F;
	*puhMinor   = (fpga_inw(FPGA_VER) >> 4) & 0x00FF;
	*puhRelease = fpga_inw(FPGA_VER) & 0x000F;

	sig_sem(SEM_FPGA);
}

//=============================================================================
/**
 * FPGAに値を設定する
 *
 * @param iReg レジスタアドレス
 * @param uhVal 設定値
 * @param uhMask マスク(bit onで許可)
 * @retval E_OK 書込みOK
 */
//=============================================================================
ER FpgaSetWord(int iReg, UH uhVal, UH uhMask)
{
	ER	ercd;
	UH	uhRdVal;
	
	// セマフォ獲得
	ercd = twai_sem(SEM_FPGA, 1000/MSEC);
	if (ercd == E_OK) {
		uhRdVal = fpga_inw(iReg);			// 現在の値を読み込む
		fpga_outw(iReg, ((uhRdVal & ~uhMask) | (uhVal & uhMask)));
		sig_sem(SEM_FPGA);					// セマフォ返却
		
		return ercd;
	}

	return ercd;
}

/*==========================================================================*/
/**
 * 画像データの1ライン取得
 *
 *	@param iBank キャプチャー領域番号
 *	@param iX X座標(8で割り切れる値を指定)
 *	@param iY Y座標
 *	@param iSize 指定ビット数(8で割り切れるサイズ)
 *	@param p 格納先アドレス
 */
/*==========================================================================*/
void FpgaCapLineGet(int iBank, int iX, int iY, int iSize, UB *p)
{
	UW uwOfs;
	UB *dp;
	
	uwOfs = iX + iY * CAP_X_SIZE;
	dp = (UB*)&cap_dat(iBank, uwOfs);
	
	while(iSize) {
		*p = *dp;
		iSize--;
		p++;
		dp++;
	}
}

/*==========================================================================*/
/**
 * 赤外線LEDのDuty設定
 *
 *	@param ubSel 赤外線LEDの指定
 *	@param ubVal 設定値(0～255)
 */
/*==========================================================================*/
void FpgaIrLedSet(UB ubSel, UB ubVal)
{
	const UB ubIrLed[] = {
		IRLED_CRL_LED1,					// 赤外線LED1
		IRLED_CRL_LED2,					// 赤外線LED2
		IRLED_CRL_LED3,					// 赤外線LED3
		IRLED_CRL_LED4,					// 赤外線LED4
		IRLED_CRL_LED5,					// 赤外線LED5
	};
	int nIrLed = sizeof ubIrLed / sizeof ubIrLed[ 0 ];
	int i;
	
	for(i = 0;i < nIrLed;i++) {
		if (ubSel & ubIrLed[ i ]) {
			fpga_outw((IRLED_DUTY + (i << 1)), ubVal);
		}
	}
}

//=============================================================================
/**
 * FPGAのレジスタデータを文字列で返す
 *
 * @param iLine 行
 * @param p データ格納先
 * @return 文字数
 */
//=============================================================================
int FpgaRegDisplayLine(int iLine, char* p)
{
	static const struct {				// 表示するFPGAのレジスタ
		char *pcRegName;
		int  iAdr;
		int  iSize;
	} stReg[] = {
		{"FPGA_VER ", FPGA_VER, 2},
		{"SENS_MON ", SENS_MON, 2},
		{"AUX_OUT  ", AUX_OUT,  2},
		{"AUX_IN   ", AUX_IN,   2},
		{"TPL_CRL  ", TPL_CRL,  2},
		{"TPL_CBYTE ", TPL_CBYTE, 2},
		{"TPL_DBYTE ", TPL_DBYTE, 2},
		{"FROM_BANK ", FROM_BANK, 2},
		{"INT_CRL  ", INT_CRL,   2},
		{"LED_CRL  ", LED_CRL,   2},
		{"BUZ_CRL  ", BUZ_CRL,   2},
		{"BUZ1_CYC ", BUZ1_CYC,  2},
		{"SEG1_CRL ", SEG1_CRL,  2},
		{"SEG2_CRL ", SEG2_CRL,  2},
		{"SEG3_CRL ", SEG3_CRL,  2},
		{"SEG4_CRL ", SEG4_CRL,  2},
		{"SEG5_CRL ", SEG5_CRL,  2},
		{"SEG6_CRL ", SEG6_CRL,  2},
		{"SEG7_CRL ", SEG7_CRL,  2},
		{"SEG8_CRL ", SEG8_CRL,  2},
		{"KEY_IN   ", KEY_IN,    2},
		{"IRLED_CRL ", IRLED_CRL, 2},
		{"IRLED1_DUTY ", IRLED1_DUTY, 2},
		{"IRLED2_DUTY ", IRLED2_DUTY, 2},
		{"IRLED3_DUTY ", IRLED3_DUTY, 2},
		{"IRLED4_DUTY ", IRLED4_DUTY, 2},
		{"IRLED5_DUTY ", IRLED5_DUTY, 2},
		{"CNF_ROM  ", CNF_ROM, 2},
		{"CMR_CAP  ", CMR_CAP, 2},
		{"CMR_PRM_AES ", CMR_PRM_AES, 2},
		{"CMR_PRM_SHT ", CMR_PRM_SHT, 2},
		{"CMR_MOD_WAIT ", CMR_MOD_WAIT, 2},
		{"CMR_CRL  ", CMR_CRL, 2},
		{"CMR_BAU  ", CMR_BAU, 2},
		{"CMR_CSZ  ", CMR_CSZ, 2},
		{"CMR_CMON ", CMR_CMON, 2},
		{"CMR_RSZ  ", CMR_RSZ, 2},
		{"CMR_RMON ", CMR_RMON, 2},
		{"CMR_CSZ  ", CMR_CSZ, 2},

		{"CMR_CMD  ", CMR_CMD, 2},
		{"CMR_RES  ", CMR_RES, 2},
		
	};
	static const int nReg = (sizeof stReg) / (sizeof stReg[0]);
	int	 iCnt;
	
	iCnt = 0;
	
	if (iLine < nReg) {
		if (stReg[ iLine ].iSize == 4) {				// LONGサイズのとき
			iCnt += _sprintf(p, "%s(%08X): %08X ", stReg[ iLine ].pcRegName, 
								(FPGA_BASE + stReg[ iLine ].iAdr), fpga_inl(stReg[ iLine ].iAdr));
		} else {
			iCnt += _sprintf(p, "%s(%08X):   %04X ", stReg[ iLine ].pcRegName, 
								(FPGA_BASE + stReg[ iLine ].iAdr), fpga_inw(stReg[ iLine ].iAdr));
		}
	}
	return iCnt;
}


/*==========================================================================*/
/**
 * 画像データの1ライン取得
 *
 *	@param iBank キャプチャー領域番号
 *	@param iX X座標(8で割り切れる値を指定)
 *	@param iY Y座標
 *	@param iSize 指定ビット数(8で割り切れるサイズ)
 *	@param p 格納先アドレス
 */
/*==========================================================================*/
void FpgaWrtDat(int iBank, int iX, int iY, int iSize, UB *p)
{
	int i;
	
	for(i = 0 ; i < iSize ; i++){
		wrt_dat(iBank, 0x04000000 + i , *p++);
	}
	
/*
	UW uwOfs;
	
	uwOfs = iX + iY * CAP_X_SIZE;
	
	while(iSize) {
		wrt_dat(iBank, uwOfs, *p);
		iSize--;
		p++;
	}
*/
}