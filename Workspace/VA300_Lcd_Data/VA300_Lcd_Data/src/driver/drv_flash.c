//=============================================================================
/**
*	VA-300プログラム
*
*	@file drv_flash.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  フラッシュメモリ書込み
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
//=============================================================================
#include <kernel.h>
#include <string.h>
#include "drv_flash.h"

#pragma section FL

// フラッシュメモリマップ定義
#define	USER_AREA_END		(0x400000 / 0x20000 - 1)	///< 実際にはUSER_AREA_END-1ブロックまで消去・書込みされる

// 変数定義
static ID s_IdSem;							///< フラッシュメモリのセマフォ

// プロトタイプ宣言
static ER flErase(UW uwAddr);				///< フラッシュメモリの消去
static UW flWrite(UW uwFp, UH *puhBp, UW n);///< フラッシュメモリの書込み
static ER flRead(UW uwFp, UH *puhBp, UW n);	///< フラッシュメモリの読込み

#pragma section
// セマフォ定義(セクションを通常にしておく)
const T_CSEM csem_fl = { TA_TFIFO, 1, 1, (B *)"sem_fl" };

/*==========================================================================*/
/**
 * フラッシュメモリドライバ初期化
 * @param id フラッシュメモリ用セマフォID
 */
/*==========================================================================*/
void FlInit(ID id)
{
	ER ercd; 

	s_IdSem = 0;
	
	// セマフォの生成
	if (id > 0) {
		ercd = cre_sem(id, &csem_fl);	
		if (ercd == E_OK) {
			s_IdSem = id;
		}
	} else {
		ercd = acre_sem(&csem_fl);
		if (ercd > 0) {
			s_IdSem = ercd;
		}
	}
}

#pragma section FL
//=============================================================================
/**
 * フラッシュメモリのブロック消去
 * @param uwAddr 消去アドレス(ブロック先頭)
 * @return エラーコード
 */
//=============================================================================
ER FlErase(UW uwAddr)
{
	ER ercd;
	
	ercd = twai_sem(s_IdSem, (100/MSEC));	// セマフォの取得
	
	if (ercd == E_OK) {
		ercd = flErase(uwAddr);				// 消去
		
		sig_sem(s_IdSem);					// セマフォ返却
	}

	return ercd;
}

//=============================================================================
/**
 * フラッシュメモリのブロック消去(本体)
 * @param uwAddr 消去アドレス(ブロック先頭)
 * @return エラーコード
 */
//=============================================================================
static ER flErase(UW uwAddr)
{
	ER ercd;
	UH uhBank;
	
	uhBank = FlBankCalc(uwAddr);
	// アクセスウィンドウ切替
	FlBankSet( uhBank );
	
	ercd = FdErase((uwAddr & (FL_AREA_SZ - 1)));
	
	
	return ercd;
}


//=============================================================================
/**
 * フラッシュメモリの書込み
 * @param uwFp フラッシュメモリのポインタ
 * @param puhBp データ格納バッファのポインタ
 * @param n 書込みたいデータ数
 * @return 書込み成功データ数
 */
//=============================================================================
UW FlWrite(UW uwFp, UH *puhBp, UW n)
{
	ER	ercd;
	UW uwSize;
	
	uwSize = 0;
	
	ercd = twai_sem(s_IdSem, (100/MSEC));	// セマフォの取得
	
	if (ercd == E_OK) {
		uwSize = flWrite(uwFp, puhBp, n);
		sig_sem(s_IdSem);					// セマフォ返却
	}
	
	return uwSize;
}

//=============================================================================
/**
 * フラッシュメモリの書込み(本体)
 * @param uwFp フラッシュメモリのポインタ
 * @param puhBp データ格納バッファのポインタ
 * @param n 書込みたいデータ数
 * @return 書込み成功データ数
 */
//=============================================================================
static UW flWrite(UW uwFp, UH *puhBp, UW n)
{
	UW uwSize;
	UW uwCnt;
	UH uhBank;
	UH uwEndSize;
	
	uwEndSize = 0;						// 書込み完了個数をクリア
	
	while(uwFp < FdFlAllSize()) {
		uhBank = FlBankCalc(uwFp);
		uwCnt = n;						// まずは指定回数に設定
		// 同じアクセスウィンドウか
		if (uhBank != FlBankCalc((uwFp + 2 * n - 1))) {
			// 同一でないときはアクセスウィンドウ最終までの回数に設定
			uwCnt = (FL_AREA_SZ - (uwFp & (FL_AREA_SZ - 1))) / 2;
		}
		uwSize = uwCnt << 1;
		
		// アクセスウィンドウ切替
		FlBankSet( uhBank );
		
		// 書込み
		if (FdWrite((uwFp & (FL_AREA_SZ - 1)), puhBp, uwCnt) == uwSize) {
			uwFp      += uwSize;
			puhBp     += uwCnt;
			uwEndSize += uwSize;
			n -= uwCnt;
			if (n > 0) {
				continue;
			}
		}
		break;
	}
	
	return uwEndSize;
}

//=============================================================================
/**
 * フラッシュメモリの読込み
 * @param uwFp フラッシュメモリのポインタ
 * @param puhBp データ格納バッファのポインタ
 * @param n 読込みたいデータ数
 * @return エラーコード
 */
//=============================================================================
ER FlRead(UW uwFp, UH *puhBp, UW n)
{
	ER ercd;
	
	ercd = twai_sem(s_IdSem, (100/MSEC));	// セマフォの取得
	
	if (ercd == E_OK) {
		ercd = flRead(uwFp, puhBp, n);
		sig_sem(s_IdSem);					// セマフォ返却
	}

	return ercd;
}

//=============================================================================
/**
 * フラッシュメモリの読込み(本体)
 * @param uwFp フラッシュメモリのポインタ
 * @param puhBp データ格納バッファのポインタ
 * @param n 読込みたいデータ数
 * @return エラーコード
 */
//=============================================================================
static ER flRead(UW uwFp, UH *puhBp, UW n)
{
	UW uwCnt;
	UH uhBank;
	ER ercd;
	
	ercd = E_OK;
	
	while(uwFp < FdFlAllSize()) {
		uhBank = FlBankCalc(uwFp);
		uwCnt = n;				// まずは指定回数に設定
		// 同じアクセスウィンドウか
		if (uhBank != FlBankCalc((uwFp + 2 * n - 1))) {
			// 同一でないときはアクセスウィンドウ最終までの回数に設定
			uwCnt = (FL_AREA_SZ - (uwFp & (FL_AREA_SZ - 1))) / 2;
		}
		// アクセスウィンドウ切替
		FlBankSet( uhBank );
		
		// 書込み
		ercd = FdRead((uwFp & (FL_AREA_SZ - 1)), puhBp, uwCnt);
		if (ercd == E_OK) {
			uwFp  += (uwCnt << 1);	// 読込みアドレスインクリメント
			puhBp += uwCnt;			// 書込みアドレスインクリメント
			n -= uwCnt;				// 残りデータ数減算
			if (n > 0) {			// 残りデータ数あるなら処理を続ける
				continue;
			}
		}
		break;
	}
	return ercd;
}
