/**
*	VA-300プログラム
*
*	@file drv_PC28F00B.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/13
*	@brief  Micron製PC28F00BM29EWHA用ドライバ
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#define	_DRV_PC28F00B_C_
#include <kernel.h>
#include <string.h>
#include <machine.h>
#include "drv_PC28F00B.h"
#include "drv_tim.h"

#pragma section FL

// メモリ接続方法で異なる
#define	FL_ADDR1	(FL_ADDR + 0xAAAA)
#define	FL_ADDR2	(FL_ADDR + 0x5554)

// フラッシュブート領域定義
enum FLASH_BOOT {						///< ブート領域定義
	FLASH_BOOT_BOTTOM = 0,				///< BOTTOMブート領域
	FLASH_BOOT_MIDDLE,					///< 一般領域
	FLASH_BOOT_TOP,						///< TOPブート領域
	
	FLASH_BOOT_MAX
};

// フラッシュメモリブロック定義
typedef struct {						///< ブロック情報構造体
	UW Size;							///< 1ブロックのサイズ
	UW Count;							///< ブロック数
}ST_BLK;

// フラッシュメモリマップ定義
#define	FLASH_BOTTOM_SIZE 	0x20000		///< ボトム領域の1ブロックのサイズ
#define	FLASH_BOTTOM_CNT	2048		///< ボトム領域のブロック数
#define	FLASH_MIDDLE_SIZE 	0			///< 通常領域の1ブロックのサイズ
#define	FLASH_MIDDLE_CNT	0			///< 通常領域のブロック数
#define	FLASH_TOP_SIZE 		0			///< トップ領域の1ブロックのサイズ
#define	FLASH_TOP_CNT		0			///< トップ領域のブロック数

static const struct stFlashMap{
	ST_BLK Boot[ FLASH_BOOT_MAX ];		///< ブート領域定義
} s_stFlashMap = {
	{ FLASH_BOTTOM_SIZE, FLASH_BOTTOM_CNT,
	  FLASH_MIDDLE_SIZE, FLASH_MIDDLE_CNT,
	  FLASH_TOP_SIZE,    FLASH_TOP_CNT}
};

static const int BlkSize = FLASH_BOTTOM_CNT + FLASH_MIDDLE_CNT + FLASH_TOP_CNT;
static const UW s_uwFlashSize = FLASH_BOTTOM_SIZE * FLASH_BOTTOM_CNT
								+ FLASH_MIDDLE_SIZE * FLASH_MIDDLE_CNT
								+ FLASH_TOP_SIZE * FLASH_TOP_CNT;

#define	USER_AREA_END		(0x400000 / 0x20000 - 1)	// 実際にはUSER_AREA_END-1ブロックまで消去・書込みされる

// プロトタイプ宣言
static ER fdErase(UW uwAddr);				///< 1ブロック消去
static ER fdWriteWord(volatile UH*, UH);	///< 1ワード書込み

static void SecErase(volatile UH *);		///< セクタ消去
static void WordPrg(volatile UH *, UH);		///< PROGRAMコマンド
static void ReadReset(volatile UH *);		///< リードリセット
static void IdRead(void);					///< ID読出し
static UW fdGetErsAddr(UW);					///< 消去ブロックのアドレス(相対)を求める
static UW Offset(UH);						///< オフセット算出
static UW fdBlkSize(UH);					///< ブロックサイズ算出
static int fdVerify(UH volatile *, UH);		///< ベリファイ

//=============================================================================
/**
 * フラッシュメモリのブロック消去
 * @param uwAddr 消去アドレス(ブロック先頭)
 * @return エラーコード
 */
//=============================================================================
ER FdErase(UW uwAddr)
{
	ER ercd;
	UINT psw;
	
	psw = vdis_psw();				// 割込み禁止
	ercd = fdErase(uwAddr);			// 消去
	vset_psw(psw);					// 割込み禁止解除

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
UW FdWrite(UW uwFp, UH *puhBp, UW n)
{
	UH	*puhAddr, *puhStart;
	UH	uhData;
	ER	ercd;
	UINT imask;
	int i, j;
	UB ledon;
	
	i = 0;
	j = 0;
	ledon = 0;
	
	if( !(uwFp & 0x01)) {						// アドレスチェック
		puhAddr = (UH*)(FL_ADDR + (uwFp & (FL_AREA_SZ - 1)));
		puhStart = puhAddr;

		while( n ) {
			n--;
			if (puhAddr < (UH*)(FL_ADDR + FL_AREA_SZ)) {
				uhData = *puhBp++;
				if (uhData != 0xFFFF) {
					imask = get_imask();			// 割込み禁止
					set_imask(15);
					ercd = fdWriteWord(puhAddr, uhData);
					set_imask(imask);				// 割込み禁止解除
					if (ercd == E_OK) {
						i += 2;
						j += 1;
						if( j >= 6400 ){	//12800 == 画像1個分
							j = 0;
							if(ledon == 1){
								LedOut(0x04, 0x00);
								ledon = 0;
							}else{
								LedOut(0x04, 0x01);
								ledon = 1;
							}
						}
					} else {
						break;
					}
				}
				puhAddr++;
				if ((UW)puhAddr & 0xFFF) {
					continue;
				} else {
					ReadReset(puhStart);
					dly_tsk( 10/MSEC);				// 書込み完了待ち
				}
			} else {								// 実装エラー
				
			}
		}
		ReadReset(puhStart);
	} else {										// 実装エラー
		
	}
	return i;
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
ER FdRead(UW uwFp, UH *puhBp, UW n)
{
	UH	*puhAddr;
	
	if( !(uwFp & 0x01)) {						// アドレスチェック
		puhAddr = (UH*)(FL_ADDR + (uwFp & (FL_AREA_SZ - 1)));
		ReadReset(puhAddr);						// リードリセット

		while( n ) {
			n--;
			if (puhAddr < (UH*)(FL_ADDR + FL_AREA_SZ)) {
				*puhBp = *puhAddr;
				puhBp++;
				puhAddr++;
				if ((UW)puhAddr & 0xFFF) {
					continue;
				} else {
					dly_tsk( 10/MSEC);			// 書込み完了待ち
				}
			} else {							// 実装エラー
				return E_PAR;
			}
		}
	} else {									// 実装エラー
		return E_PAR;
	}
	return E_OK;
}

//=============================================================================
/**
 * フラッシュメモリのブロック消去
 * @param uwAddr 消去アドレス(ブロック先頭)
 * @return エラーコード
 */
//=============================================================================
static ER fdErase(UW uwAddr)
{
	UW	uwErsAddr;
	int	iRetry, iStat;
	UW	uwTmout;
	
	iRetry = 0;									// リトライ回数初期化
	
	// フラッシュメモリの消去コマンド
	if (uwAddr < s_uwFlashSize) {
		uwErsAddr = FL_ADDR + fdGetErsAddr( uwAddr );
		dly_tsk( 10/MSEC);
		ReadReset((UH*)uwErsAddr );				// リードリセット
		dly_tsk( 10/MSEC);
		SecErase((UH*)uwErsAddr );				// 消去コマンド実行
	} else {									// 指定アドレスエラー
		return E_PAR;
	}
	
	// タイマ設定
	uwTmout = ERS_TMOUT;
	
	// 消去完了待ち
	while( uwTmout ) {
		// ブロックの消去状態確認
		iStat = fdVerify((UH*)uwErsAddr, 0xFFFF);
		if (iStat == 1) {
			ReadReset((UH*)uwErsAddr );
			return E_OK;
		} else if (iStat) {
			if (iRetry >= 2) {
				break;
			} else {
				iRetry++;						// リトライ回数インクリメント
				SecErase((UH*)uwErsAddr);		// 消去コマンド実行
				uwTmout = ERS_TMOUT;			// タイマ設定
			}
		}
		uwTmout--;
	}
	ReadReset((UH*)uwErsAddr );					// リードリセット
	
	return E_TMOUT;
}

//-----------------------------------------------------------------------------
// 1ワード書込み
//-----------------------------------------------------------------------------
static ER fdWriteWord(volatile UH* puhAddr, UH uhData)
{
	int iStat;
	UW	uwTmout;

	WordPrg( puhAddr, uhData);		// 書込みコマンド実行
	
	uwTmout = PRG_TMOUT;			// タイマ設定
	
	while( (iStat = fdVerify(puhAddr, uhData)) == 0) {
		if( uwTmout) {
//			dly_tsk( 1/MSEC);
			uwTmout--;
			continue;
		}
		// 書込みタイムアウト
		ReadReset( puhAddr );
		return E_TMOUT;
	}
	
	if (iStat == 1) {				// 書込み成功
		return E_OK;
	}
	// 書込み失敗
	return E_OBJ;
}

//=============================================================================
// [機能] Ｔコマンドの実行
// [引数] なし
// [戻値] なし
// [補足] 特になし
//=============================================================================
void CmdTest(void)
{
	*((UH*)(FL_ADDR + 0x00AAAA)) = 0xAA;		// データ書き込み
	*((UH*)(FL_ADDR + 0x005554)) = 0x55;
	*((UH*)(FL_ADDR + 0x00AAAA)) = 0xA0;
	*((UH*)(FL_ADDR + 0x000000)) = 0x1234;
}

//-----------------------------------------------------------------------------
// セクタ消去開始
//-----------------------------------------------------------------------------
static void SecErase(volatile UH *SecAddr)
{
	UINT psw;
	int i;
	
//	psw = vdis_psw();							// 割込み禁止

//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR1) = 0xAA;			// 書き込みサイクル UNLOCK
//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR2) = 0x55;
//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR1) = 0x80;			// セットアップコマンド
//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR1) = 0xAA;			// 書き込みサイクル UNLOCK
//	for(i=0;i<100;i++)	{ nop(); }
	*((volatile UH*)FL_ADDR2) = 0x55;
//	for(i=0;i<100;i++)	{ nop(); }
	*SecAddr = 0x30;							// セクタ消去命令
//	for(i=0;i<100;i++)	{ nop(); }
	
//	vset_psw( psw );							// 割込み禁止を戻す
}

//-----------------------------------------------------------------------------
// PROGRAMコマンド
//-----------------------------------------------------------------------------
static void WordPrg(volatile UH *pPrgAddr, UH uhData)
{
	*((volatile UH*)FL_ADDR1) = 0xAA;			// データ書き込み
	*((volatile UH*)FL_ADDR2) = 0x55;
	*((volatile UH*)FL_ADDR1) = 0xA0;
	*pPrgAddr = uhData;
}

//-----------------------------------------------------------------------------
// リードリセット
//-----------------------------------------------------------------------------
static void ReadReset(volatile UH *pSecAddr)
{
	*pSecAddr = 0xFF;							// リードリセット命令
}

//-----------------------------------------------------------------------------
// ID READコマンド
//-----------------------------------------------------------------------------
static void IdRead(void)
{
	*((volatile UH*)FL_ADDR1) = 0xAA;				// ID Readコマンド
	*((volatile UH*)FL_ADDR2) = 0x55;
	*((volatile UH*)FL_ADDR1) = 0x90;
}

//-----------------------------------------------------------------------------
// 消去ブロックのアドレス(相対)を求める
//-----------------------------------------------------------------------------
static UW fdGetErsAddr(UW uwAddr)
{
	UW ulBlk;
	
	for(ulBlk = 0;ulBlk < (BlkSize - 1);ulBlk++) {
		if (uwAddr < Offset( ulBlk + 1 )) {
			return Offset( ulBlk );
		}
	}
	
	return 0;
}

//-----------------------------------------------------------------------------
// オフセットを求める
//-----------------------------------------------------------------------------
static UW Offset(UH blkno)
{
	UW	ofs;
	
	ofs = 0;									// オフセット初期化
	if( blkno-- < 1) return 0;					// オフセット0の検査
	do { ofs += fdBlkSize( blkno ); } while( blkno--);
	return ofs;
}

//-----------------------------------------------------------------------------
// ブロックサイズを求める
//-----------------------------------------------------------------------------
static UW fdBlkSize(UH blkno)
{
	int iBoot;
	
	for (iBoot = FLASH_BOOT_BOTTOM;iBoot < FLASH_BOOT_MAX;iBoot++) {
		if (blkno < s_stFlashMap.Boot[ iBoot ].Count) {
			return s_stFlashMap.Boot[ iBoot ].Size;
		}
	}
	
	return 0;
}

//-----------------------------------------------------------------------------
// フラッシュＲＯＭ消去/書込み終了検査
//-----------------------------------------------------------------------------
static int fdVerify(UH volatile *pAddr, UH data)
{
	UB	flg;

	flg = *pAddr;									// シーケンスフラグ参照
	if (!((flg ^ data) & 0x80)) return 1;			// 消去/書込 成功
	if (! (flg & 0x20))			return 0;			// 消去/書込 未完
	flg = *pAddr;									// シーケンスフラグ再検査
	if (!((flg ^ data) & 0x80)) return 1;			// 消去/書込 成功
	return 999;										// 消去/書込 失敗
}

//-----------------------------------------------------------------------------
// フラッシュＲＯＭサイズ
//-----------------------------------------------------------------------------
UW FdFlAllSize(void)
{
	return s_uwFlashSize;
}

#pragma section
