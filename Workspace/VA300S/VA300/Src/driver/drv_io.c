//=============================================================================
/**
 *
 * VA-300プログラム
 * <<I/O制御関連モジュール>>
 *
 *	@brief I/O入力制御機能。
 *	
 *	@file drv_io.c
 *	
 *	@date	2013/03/08
 *	@version 1.00 新規作成
 *	@author	F.Saeki
 *
 *	Copyright (C) 2012, OYO Electric Corporation
 */
//=============================================================================
#define	_DRV_IO_C_
#include "kernel.h"
#include "sh7750.h"
#include "drv_fpga.h"

// マクロ定義
#define	IO_IP			8				///< I/O入力割込みレベル(8)
#define	IO_INT			INT_IRL8		///< I/O入力割込み番号
#define	enable_io_int()	fpga_setw(INT_CRL, (INT_CRL_AUXIN));	///< I/O入力割込み許可
#define	clear_io_int()	fpga_clrw(INT_CRL, (INT_CRL_AUXIN));	///< I/O入力割込みクリア

// プロトタイプ宣言
static INTHDR ioInt(void);				///< 生態検知センサ割込み
static void io_int(void);				///< 生態検知センサ割込み(本体)


// 変数定義
static ID s_idTsk;						///< 待ちタスク
const T_DINH dinh_io = { TA_HLNG, ioInt, IO_IP};		// 割込み定義

//=============================================================================
/**
 * I/O入力割込み初期化
 *
 * @param idTsk 割込み時に起床するタスク
 * @retval E_OK 成功
 */
//=============================================================================
ER IoInit(ID idTsk)
{
	ER ercd;
	UW psw;
	
	// タスクIDを設定
	if (idTsk) {
		s_idTsk = idTsk;
	} else {
		return E_PAR;
	}

	//
	// ポートの設定(FPGAなので不要と思われる)
	//
	
	
	//
	// 割込み設定
	//
	psw = vdis_psw();
	
	ercd = def_inh(IO_INT, &dinh_io);				// I/O入力割込み設定

	// 割込み許可
	if (ercd == E_OK) {
		enable_io_int();
	}
	vset_psw(psw);

	return ercd;
}

#pragma interrupt(ioInt)
//=============================================================================
/**
 * I/O入力割込み
 */
//=============================================================================
static INTHDR ioInt(void)
{
	ent_int();
	io_int();						// I/O入植割込み処理(本体)
	ret_int();
}

//=============================================================================
/**
 * I/O入力割込み(本体)
 */
//=============================================================================
static void io_int(void)
{
	clear_io_int();					// I/O入力割込みクリア&不許可
	if (s_idTsk) {
		iwup_tsk( s_idTsk );
	}
	enable_io_int();				// I/O入力割込み許可
}

