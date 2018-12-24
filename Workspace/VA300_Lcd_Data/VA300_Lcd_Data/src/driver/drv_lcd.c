/**
*	LCDドライバ
*
*	@file drv_lcd.c
*	@version 1.00
*	@version 1.01 LCDのちらつきに対応
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2013/03/09
*	@brief  LCDドライバ
*
*	Copyright (C) 2007, OYO Electric Corporation
*/
#define	_DRV_LCD_C_
#include <kernel.h>
#include "drv_fpga.h"
#include "drv_lcd.h"

#define	LCD_BASE		0xA8000000	///LCDコントローラレジスタのベースアドレス
#define LCD_REG_ADD     0x00000000  ///LCDレジスタのアドレス
#define LCD_REG_DATA    0x00000002  ///LCDレジスタのデータ

#define	lcdPreBufSet()	(*(volatile UH *)(LCD_BASE+LCD_REG_ADD) = 0x66)	///< アドレスレジスタにバッファを設定

// プロトタイプ宣言
static void lcdcRegSet(int iAddr, UH uhData);	///< LCDコントローラレジスタ書込み
static UH lcdcRegGet(int iAddr);				///< LCDコントローラレジスタ読み出し
static void lcdcBufSet(UH uhData);				///< LCDコントローラデータレジスタ書込み
static ER lcdcInit(void);						///< LCD初期化(本体)

// 変数宣言
const T_CSEM csem_lcd   = { TA_TFIFO, 1, 1, (B *)"sem_lcd" };
static ID s_idSem;								///< セマフォID
extern const unsigned short imgWord[];

/*==========================================================================*/
/*
 * LCDコントローラレジスタ書き込み
 * @param  iAddr  レジスタアドレス
 * @param  uhData レジスタデータ
*/
/*==========================================================================*/
static void lcdcRegSet(int iAddr, UH uhData)
{
	volatile UH *puhReg;

	puhReg  = (volatile UH*)(LCD_BASE+LCD_REG_ADD);
	*puhReg = (UH)iAddr;
	puhReg++;
	
	*puhReg = uhData;
}
/*==========================================================================*/
/*
 * LCDコントローラレジスタ読み込み
 * @param  iAddr レジスタアドレス
 * @return レジスタデータ
*/
/*==========================================================================*/
static UH lcdcRegGet(int iAddr)
{
	volatile UH *puhReg;
	volatile UH uhRet;

	puhReg  = (volatile UH*)(LCD_BASE+LCD_REG_ADD);
	*puhReg = (UH)iAddr;
	puhReg++;
	
	uhRet = *puhReg;
	
	return uhRet; 
}
/*==========================================================================*/
/*
 * LCDコントローラレジスタマスク付き書き込み
 * @param  add レジスタアドレス
 * @param  mask フラグパラメータ(1の部分が書き込み対象)
 * @param  data レジスタデータ
*/
/*==========================================================================*/
static void lcdcRegSetMask(int add,UH mask,UH data)
{
	UH dat;
			
	dat  = lcdcRegGet(add);
	dat &= (~mask);
	dat|=data;
	lcdcRegSet(add,dat);	
	
}

/*==========================================================================*/
/*
 * LCDコントローラバッファへ書き込み
 * @param  uhData 画像データ
*/
/*==========================================================================*/
static void lcdcBufSet(UH uhData)
{
	volatile UH *puhData;

	puhData = (volatile UH *)(LCD_BASE+LCD_REG_DATA);
	*puhData = uhData;
}

/*==========================================================================*/
/*
 * 　　LCDテスト（動作チェック用プログラム）
 *
*/
/*==========================================================================*/
void LcdTest(void)
{
	int lop;	
	volatile UH *p;
	
	lcdcRegSet(0x04,0x1a);
	lcdcRegSet(0x06,0x01);	
	lcdcRegSet(0x08,0x01);
	lcdcRegSet(0x0c,0x23);		
	lcdcRegSet(0x0e,0x3f);
	lcdcRegSet(0x12,0x00);	
	lcdcRegSet(0x04,0x9a);	
	
	//50ms 待ち
	dly_tsk(50/MSEC);

	lcdcRegSet(0x12,0x00);	

	// 入力データタイプを24bppmode2に設定
	lcdcRegSet(0x14, 0x02);

	//LCDタイミング設定 480 * 272
	lcdcRegSet(0x16, 59);
	lcdcRegSet(0x18, 109);
	lcdcRegSet(0x1A, 0x0E);
	lcdcRegSet(0x1C, 0x01);
	lcdcRegSet(0x1E, 6);
	lcdcRegSet(0x20, 40);
	lcdcRegSet(0x22, 177);
	lcdcRegSet(0x24, 9);
	lcdcRegSet(0x26, 2);
	lcdcRegSet(0x28, 0x00);
	
	//  使用するSDRAM の種類を設定する。128mbit
	lcdcRegSet(0x82, 0x03);
	
	// SDRAM リフレッシュカウンタ
	lcdcRegSet(0x8C, 0x00);
//	lcdcRegSet(0x8C, 0xFF);
	lcdcRegSet(0x8E, 0x01);
//	lcdcRegSet(0x8E, 0x03);
	
	// SDRAM ライトバッファメモリサイズ 384KBに設定  SDRAM_BUF_SIZE=0x60000
	// 480*272*3(byte)を16KB単位に切り上げ(0x60000)
	lcdcRegSet(0x90, SDRAM_BUF_SIZE >> 14);	
			
	
	// ソフトウェアリセット
	lcdcRegSet(0x68, 0xE8);
	
	// リセット解除
	lcdcRegSet(0x68, 0x00);
	
	// SDCLKをイネーブル
	lcdcRegSet(0x68, 0x01);

	
	// SDRAMのイニシャライズ
	lcdcRegSet(0x84, 0x82);
	
	for(;;){//0x86アドレスのbit1=1待ち
		if( (lcdcRegGet(0x86)&0x02)!=0)break;
	}

	// PWMの比率
	lcdcRegSet(0x72, 0xFF);	
	lcdcRegSet(0x7A, 0xFF);	

	// PWMのイニシャライズ
//	lcdcRegSet(0x70, 0x05);	
	lcdcRegSet(0x70, 0x86);	

//-----------------------------
	
	//表示オフ シングルバッファ選択
	lcdcRegSet(0x2a, 0x00);

	//アップデート　不要？？
	lcdcRegSet(0x50, 0x80);
	lcdcRegSet(0x50, 0x00);	
	
	//書き込みバッファ１選択
	lcdcRegSet(0x52, 0x00);

	//透過色Rの値
	lcdcRegSet(0x54, 0x00);
	
	//透過色Rの値
	lcdcRegSet(0x56, 0x00);	
	
	//透過色Bの値
	lcdcRegSet(0x58, 0x00);		
		
	//X開始位置//1
	lcdcRegSet(0x5a, 0x00);
	
	//Y開始位置//1
	lcdcRegSet(0x5c, 0x00);			
	lcdcRegSet(0x5e, 0x00);

	//X終了位置//480
	lcdcRegSet(0x60, 118);

	//Y終了位置//272
//	lcdcRegSet(0x62, 0x43);			
//	lcdcRegSet(0x64, 0x03);   	
	lcdcRegSet(0x62, 0x0F);			
	lcdcRegSet(0x64, 0x01);   	
	
//-----------------------------	
	p = imgWord;
	lcdcRegSet(0x66, *p);		// Debug
	p++;
	for(lop=1;lop<=480*272*2;lop++, p++){//黒データ書き込み(16bit bus)
		lcdcBufSet( *p );		// Debug
//		lcdcBufSet( 0x00FF );	// Debug
	}
//-----------------------------	

	//バッファ１表示ON	
	lcdcRegSet(0x2a, 0x01);
	
	//アップデート　不要？？		
	lcdcRegSetMask(0x50,0x80,0x80);	
	lcdcRegSetMask(0x50,0x80,0x00);	
	
	//IO制御（表示）
	lcdcRegSet(0x6e, 0x08);		
		
}
/*==========================================================================*/
/*
 *	LCDを初期化
 *	@param idSem LCD用セマフォID
 *	@return E_OK 成功
*/
/*==========================================================================*/
ER LcdcInit(ID idSem)
{
	ER ercd;
	
	s_idSem = 0;
	ercd = cre_sem(idSem, &csem_lcd);	// Create semaphore
	if (ercd == E_OK) {
		s_idSem = idSem;
		ercd = lcdcInit();
	}
	return ercd;
}

/*==========================================================================*/
/*
 *	LCDを初期化(本体)
 *	@param idSem LCD用セマフォID
 *	@return E_OK 成功
*/
/*==========================================================================*/
static ER lcdcInit(void)
{
	volatile UH dat;
	int	iTmout;
	ER	ercd;

	lcdcRegSet(0x04,0x1a);
	lcdcRegSet(0x06,0x01);	
	lcdcRegSet(0x08,0x01);
	lcdcRegSet(0x0c,0x23);		
	lcdcRegSet(0x0e,0x3f);
	lcdcRegSet(0x12,0x00);	
	lcdcRegSet(0x04,0x9a);	
	
	//20ms Wait
	dly_tsk(20/MSEC);
	
	lcdcRegSet(0x12,0x00);	
	
	//  使用するLCD パネルのタイミング、極性を設定する。
	// (HDISP+HNDP)(dot)*(VDISP+VNDP)(dot)*60(frame/sec)=11992800≒12MHz=SYSCLK(PCLK)

	// 入力データタイプを24bppmode2に設定
	lcdcRegSet(0x14, 0x02);

	// HDISPを480に設定
	lcdcRegSet(0x16, (SCREEN_WIDTH / 8)- 1);

	// HNDPを220に設定
	lcdcRegSet(0x18, 109);

	// VDISPを272に設定
	lcdcRegSet(0x1A, (SCREEN_HEIGHT & 0xFF)- 1);	// 15(+1)
	lcdcRegSet(0x1C, SCREEN_HEIGHT / 256);			// +256

	// VNDPを14に設定
	lcdcRegSet(0x1E, 6);

	// HSWを41に設定
	lcdcRegSet(0x20, 40);

	// HPSを177に設定
	lcdcRegSet(0x22, 177);


	// VSWを10に設定
	lcdcRegSet(0x24, 9);

	// VPSを2に設定
	lcdcRegSet(0x26, 2);


	// PCLK極性を立ち下がりに設定
	lcdcRegSet(0x28, 0x00);
	
	
	//  使用するSDRAM の種類を設定する。
	// SDRAM 128Mbitに設定
	lcdcRegSet(0x82, 0x03);
	
	// SDRAM リフレッシュカウンタを14.2us(=(1/36MHz) x 512)に設定
	lcdcRegSet(0x8C, 0x00);
	lcdcRegSet(0x8E, 0x02);
	
	// SDRAM リフレッシュカウンタを1 . us(=(1/27MHz) x 420)に設定
	lcdcRegSet(0x8C, 0xA0);
	lcdcRegSet(0x8E, 0x01);
	
	// SDRAM ライトバッファメモリサイズ 384KBに設定
	// 480*272*3(byte)を16KB単位に切り上げ(0x60000)
	lcdcRegSet(0x90, SDRAM_BUF_SIZE >> 14);	
			
	
	// ソフトウェアリセット
	lcdcRegSet(0x68, 0xE8);
	
	// リセット解除
	lcdcRegSet(0x68, 0x00);
	
	// SDCLKをイネーブル
	lcdcRegSet(0x68, 0x01);
	
	// SDRAMのイニシャライズ
	lcdcRegSet(0x84, 0x82);
	
	
	// 初期化完了待ち
	ercd = E_TMOUT;
	for(iTmout = 0;iTmout < 50;iTmout++){
		if ((lcdcRegGet(0x86) & 0x02) == 0) {
			dly_tsk((10/MSEC));
		} else {
			ercd = E_OK;
			break;
		}
	}

	// PWMのイニシャライズ
	/// MODIFY_BEGIN LCD点灯 '13/5/1
	lcdcRegSet(0x72, 0x6B);
	lcdcRegSet(0x74, 0x00);
	lcdcRegSet(0x76, 0x00);
	lcdcRegSet(0x78, 0x00);

	lcdcRegSet(0x7A, 0x0B);
	lcdcRegSet(0x7C, 0x00);
	lcdcRegSet(0x7E, 0x00);
	lcdcRegSet(0x80, 0x00);
	
//	lcdcRegSet(0x70, 0x05);	
	lcdcRegSet(0x70, 0x86);	
	/// MODIFY_END LCD点灯
	
	/// INSERT_BEGIN LCD表示しない '13/5/8
	lcdcRegSetMask(0x2a, 0x0f, 0x0B);
	lcdcRegSetMask(0x50, 0x80, 0x80);
	lcdcRegSetMask(0x50, 0x80, 0x00);
	lcdcRegSet(0x6e, 0x08);	
	/// INSERT_END LCD表示しない '13/5/8

	return ercd;
}
/*========================================================================*/
/*
 * ディスプレイウィンドウの入力画像条件の設定
 *
 * @param colorEnb 透過色イネーブル(0:ディセーブル、1:イネーブル)
 * @param keyR     透過色R(0～255)
 * @param keyG     透過色G(0～255)
 * @param keyB     透過色B(0～255)
 * @param Xstart   X開始位置(0,8,16,･･･472)
 * @param Ystart   Y開始位置(0～271)
 * @param Xend     X終了位置(7,15,23･･･479)
 * @param Yend     Y終了位置(0～271)
 * @retval E_OK 成功
*/
/*========================================================================*/
ER LcdcDisplayWindowSet( UB bufNum,		// 画面バッファ(0～15)
					 UB colorEnb,	// 透過色イネーブル(0:ディセーブル、1:イネーブル)
					 UB keyR,		// 透過色R(0～255)
					 UB keyG,		// 透過色G(0～255)
					 UB keyB, 		// 透過色B(0～255)
					 UH Xstart,		// X開始位置(0,8,16,･･･472)
					 UH Ystart,		// Y開始位置(0～271)
					 UH Xend,		// X終了位置(7,15,23･･･479)
					 UH Yend)		// Y終了位置(0～271)
{

	ER ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// セマフォの取得	
	if (ercd == E_OK) {
	 	/// DELETE_BEGIN LCDちらつく '13/5/7
//		lcdcRegSet(0x2a, 0x0f);
//		lcdcRegSetMask(0x50, 0x80, 0x80);	
	 	/// DELETE_BEGIN LCDちらつく '13/5/7
		lcdcRegSetMask(0x50, 0x80, 0x00);			
		
		lcdcRegSetMask(0x52, 0xf0, bufNum << 4);	// Input Mode Registerアドレス

		// 透過色設定
		if (colorEnb == 0) {
			lcdcRegSetMask(0x52, 0x08, 0x00);	// Input Mode Registerアドレス
		} else {
			lcdcRegSetMask(0x52, 0x08, 0x08);	// Input Mode Registerアドレス
		}
		//透過色Rの値
		lcdcRegSet(0x54, keyR);
	
		//透過色Rの値
		lcdcRegSet(0x56, keyG);	
	
		//透過色Bの値
		lcdcRegSet(0x58, keyB);		
		
		//X開始位置
		lcdcRegSet(0x5a, (Xstart >> 2) & 0xfe);
	
		//Y開始位置
		lcdcRegSet(0x5c, (Ystart >> 2) & 0xff);			
		lcdcRegSet(0x5e, Ystart        & 3   );

		//X終了位置
		lcdcRegSet(0x60, ((Xend +1 - 8) >> 2) & 0xfe);

		//Y終了位置
		lcdcRegSet(0x62, ((Xend +1 - 8) >> 2) & 0xfe);
		lcdcRegSet(0x64, (Yend) & 3               );
		
		// アドレスレジスタにバッファを設定
//		lcdPreBufSet();
	}

	return ercd;
 	
}
/*========================================================================*/
/*	ディスプレイウィンドウに16ビット画像データを書き込み
 *	※データの連続書込みなのでセマフォはかけない
 * @param  val 画像データ
*/ 
/*========================================================================*/
ER LcdImageWrite(UH val)
{
	lcdcBufSet( val );

	return E_OK;
}
/*========================================================================*/
/*   ディスプレイウィンドウのための設定を完了
 *   @retval E_OK 成功
*/ 
/*========================================================================*/
ER LcdcDisplayWindowFinish(void)
{
	ER ercd;	
	
	ercd = pol_sem( s_idSem );				// セマフォ取得済みチェック	
	if (ercd == E_TMOUT) {
		/// DELETE_BEGIN LCDちらつく '13/5/7
//		lcdcRegSet(0x2a, 0x00);		
//		lcdcRegSet(0x2a, 0x01);	
//		lcdcRegSetMask(0x50, 0x80, 0x80);	
//		lcdcRegSetMask(0x50, 0x80, 0x00);	
	
//		lcdcRegSet(0x6e, 0x08);		
		/// DELETE_END LCDちらつく '13/5/7

		ercd = sig_sem(s_idSem);			// セマフォ返却
	} else if (ercd == E_OK) {
		ercd = E_OBJ;
	}

	return ercd;
}
/*==========================================================================*/
/*
 * アルファブレンディングのための表示条件を設定
 * @param mode 入力画像の選択(0:入力画像のみ、1:入力画像1と入力画像2)
 * @param alpha アルファ値(0～32)
 * @param width 水平画像サイズ(8,16,24...480)
 * @param height 垂直画像サイズ(1～280)
 * @param src1bufnum 入力画像1の画面バッファNo(0～15)
 * @param src1Xstart 入力画像1のX開始位置(0,8,16,24...472)
 * @param src1Ystart 入力画像1のY開始位置(0～271)
 * @param src2bufnum 入力画像2の画面バッファNo(0～15)
 * @param src2Xstart 入力画像2のX開始位置(0,8,16,24...472)
 * @param src2Ystart 入力画像2のY開始位置(0～271)							
 * @param dstbufnum 出力画像の画面バッファNo(0～15)
 * @param dstXstart 出力画像のX開始位置(0,8,16,24...472)
 * @param dstYstart 出力画像のY開始位置(0～271)
 * @return  E_OK 成功
 */
/*==========================================================================*/
ER LcdcAlphablendSet(UB mode,       //入力画像の選択(0:入力画像のみ、1:入力画像1と入力画像2)
						UB alpha,      //アルファ値(0～32)
						UH width,      //水平画像サイズ(8,16,24...480)
						UH height,     //垂直画像サイズ(1～280)
						UB src1bufnum, //入力画像1の画面バッファNo(0～15)
						UH src1Xstart, //入力画像1のX開始位置(0,8,16,24...472)
						UH src1Ystart, //入力画像1のY開始位置(0～271)
						UB src2bufnum, //入力画像2の画面バッファNo(0～15)
						UH src2Xstart, //入力画像2のX開始位置(0,8,16,24...472)
						UH src2Ystart, //入力画像2のY開始位置(0～271)							
						UB dstbufnum,  //出力画像の画面バッファNo(0～15)
						UH dstXstart,  //出力画像のX開始位置(0,8,16,24...472)
						UH dstYstart) //出力画像のY開始位置(0～271)
{

	UB	setting;

	UW	src1adr, src2adr, dstadr;
	ER ercd;
	
	ercd = twai_sem(s_idSem, (100/MSEC));	// セマフォの取得
	
	if (ercd == E_OK) {
		//アルファブレンディング 水平画像サイズ
		lcdcRegSet(0x98, (width >> 3) - 1);	// 値を設定

		//アルファブレンディング 垂直画像サイズ
		lcdcRegSet(0x9a, (height - 1) & 0xff);	// 値を設定			
		lcdcRegSet(0x9c, (height - 1) >> 8);	// 値を設定	


		//アルファブレンディング設定値
		if (mode == 0) {
			setting = 0xc0;	// 入力画像1のみ
		} else {
			setting = 0x80;	// 入力画像1+入力画像2
		}
		setting |= (alpha & 0x3f);

		//アルファブレンディング設定レジスタ
		lcdcRegSet(0x9e, setting);	// 値を設定			


		// src1,src2,dstのスタートアドレスを計算	
		// SDRAMアドレス = バッファnのアドレス + (YS * Xフルサイズ) * 3バイト + XS * 3バイト
		src1adr = src1bufnum * SDRAM_BUF_SIZE + (src1Ystart * SCREEN_WIDTH + src1Xstart)* 3;
		src2adr = src2bufnum * SDRAM_BUF_SIZE + (src2Ystart * SCREEN_WIDTH + src2Xstart)* 3;
		dstadr  = dstbufnum  * SDRAM_BUF_SIZE + (dstYstart  * SCREEN_WIDTH + dstXstart )* 3;
			
		//アルファブレンディング 入力画像１ メモリ開始アドレス
		lcdcRegSet(0xa0, src1adr & 0xf8);	// 値を設定			
		src1adr >>= 8;
		lcdcRegSet(0xa2, src1adr & 0xff);	// 値を設定			
		src1adr >>= 8;
		lcdcRegSet(0xa4, src1adr & 0xff);	// 値を設定			

		//アルファブレンディング 入力画像２ メモリ開始アドレス
		lcdcRegSet(0xa6, src2adr & 0xf8);	// 値を設定	
		src2adr >>= 8;
		lcdcRegSet(0xa8, src2adr & 0xff);	// 値を設定		
		src2adr >>= 8;
		lcdcRegSet(0xaa, src2adr & 0xff);	// 値を設定			

		//アルファブレンディング 出力画像 メモリ開始アドレス
		lcdcRegSet(0xac, dstadr & 0xf8);	// 値を設定			
		dstadr >>= 8;
		lcdcRegSet(0xae, dstadr & 0xff);	// 値を設定			
		dstadr >>= 8;
		lcdcRegSet(0xb0, dstadr & 0xff);	// 値を設定		

		//Interrupt Control Registerの割込みをイネーブルにする
		lcdcRegSet(0xB2, 0x01);

		// Interrupt Status Registerのフラグをクリアする
		lcdcRegSet(0xB6, 0x01);
		lcdcRegSet(0xB6, 0x0);
	
		// アルファブレンディングスタート
		lcdcRegSet(0x94, 1);
		lcdcRegSet(0x94, 0);

		sig_sem(s_idSem);						// セマフォ返却
	}
	return ercd;

}
/*========================================================================*/
/* アルファブレンディングコピー完了判定を行なう
 * @param  *result   0:コピー完了   1:コピー中
 * @return  E_OK 成功
*/
/*========================================================================*/
// アルファブレンディング機能によるデータコピーが完了したら、フラグを戻す
//	Interrupt Status Registerのフラグをチェック
ER LcdcAlphablendEnd( UB *result )
{
	ER ercd;
	UH uhVal;
	
	ercd = E_OK;
	
	*result = 1;
	
	//	Interrupt Status Registerのフラグをチェック
	uhVal = lcdcRegGet(0xB4);
	if (uhVal & 0x01) {
		*result = 0;
		ercd = twai_sem(s_idSem, (100/MSEC));	// セマフォの取得
		if (ercd == E_OK) {
			// アルファブレンディング機能によるデータコピーが完了
			// Interrupt Status Registerのフラグをクリアする
			lcdcRegSet(0xB6, 0x01);
			lcdcRegSet(0xB6, 0x0);
			
			sig_sem(s_idSem);		// セマフォ返却
		}
	}

	return ercd;
}
/*========================================================================*/
/*  Picture In Picture表示のための条件を設定
 *	@return  E_OK 成功
*/
/*========================================================================*/
ER LcdcPictureInPictureSet(UB pipnum,				// 0：PIP1、1：PIP2
						UH width_pip, 		        //水平画像サイズ(8,16,24...480)
						UH height_pip,		        //垂直画像サイズ(1～272)
						UB srcbufnum_pip,		    // 入力画像の画面バッファNo（0～15)
						UH srcXstart_pip,		    //入力画像のX開始位置(0,8,16,24...472)
						UH srcYstart_pip,		    //入力画像のY開始位置(0～271)
						UH dstXstart_pip,		    //メイン画面 X開始位置(0,8,16,24...472)
						UH dstYstart_pip)		    //メイン画面 Y開始位置(0～271)
{
	UW	srcadr;
	UB	regadr;
	ER	ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// セマフォの取得
	
	if (ercd == E_OK) {
		if (pipnum == 0) {
			/* REG[2Ch] PIP1 Display Memory Start Address Register 0のアドレスを設定 */
			regadr = 0x2C;	//
		} else if (pipnum == 1) {
			/* REG[3Eh] PIP2 Display Memory Start Address Register 0のアドレスを設定 */
			regadr = 0x3E;	//
		} else {
			return ercd;
		}

		//PIP1またはPIP2 メモリ開始アドレスレジスタのアドレス
		// srcのスタートアドレスを計算	
		// SDRAMアドレス = バッファnのアドレス + (YS * Xフルサイズ) * 3バイト + XS * 3バイト
		srcadr = srcbufnum_pip * SDRAM_BUF_SIZE + (srcYstart_pip * SCREEN_WIDTH + srcXstart_pip)* 3;
			
		//メモリ開始アドレス
		lcdcRegSet(regadr, srcadr & 0xf8);	// 値を設定
		regadr += 2;
		srcadr >>= 8;
		lcdcRegSet(regadr, srcadr & 0xff);	// 値を設定
		regadr += 2;
		srcadr >>= 8;
		lcdcRegSet(regadr, srcadr & 0xff);	// 値を設定
		regadr += 2;

		// X開始位置
		lcdcRegSet(regadr, (dstXstart_pip >> 2) & 0xfe);	// 値を設定
		regadr += 2;
		

		// Y開始位置
		lcdcRegSet(regadr, (dstYstart_pip >> 2) & 0xff);	// 値を設定
		regadr += 2;
		lcdcRegSet(regadr,  (dstYstart_pip) & 3);			// 値を設定
		regadr += 2;
			
		// X終了位置
		lcdcRegSet(regadr,  ((dstXstart_pip + width_pip - 1 + 1 - 8) >> 2) & 0xfe);	// 値を設定
		regadr += 2;

		// Y終了位置
		lcdcRegSet(regadr,  ((dstYstart_pip + height_pip - 1) >> 2) & 0xff);	// 値を設定
		regadr += 2;
		lcdcRegSet(regadr,  ((dstYstart_pip + height_pip - 1)) & 3);			// 値を設定
		regadr += 2;
		
		sig_sem(s_idSem);						// セマフォ返却
	}
	return ercd;
}
/*========================================================================*/
/*  メイン画面表示バッファを選択
 *  @param  bufNum メイン画面表示バッファ(0～15)
 *  @param  pipSel PIP画面表示選択(0:PIP表示なし　1:PIP1   2:PIP2   3:PIP1&PIP2  )
 *	@return  E_OK 成功
*/
/*========================================================================*/
ER LcdcDisplayModeSet( UB bufNum, UB pipSel)
{
	ER ercd;

	// レジスタにあわせた処置
	if (pipSel) {
		pipSel &= 0x03;
		pipSel++;
	}

	ercd = twai_sem(s_idSem, (100/MSEC));	// セマフォの取得
	
	if (ercd == E_OK) {
		lcdcRegSet(0x2A, ((bufNum & 0xf) << 4) | (pipSel << 1) | 1);	// 次の画面・次のPIP出力選択を設定する
		lcdcRegSet(0x50, 0x80);										// 表示設定レジスタ(0x2A～0x4E)更新
	
		sig_sem(s_idSem);					// セマフォ返却
	}
	
	return ercd;
}
/*==========================================================================*/
/*
 * LCDパネルのバックライトを点灯
 * @return E_OK 成功
 */
/*==========================================================================*/
ER LcdcBackLightOn(void)
{
	ER ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// セマフォの取得	
	if (ercd == E_OK) {	
		/// MODIFY_BEGIN LCD点灯 '13/5/1
//		lcdcRegSetMask(0x70, 0x07, 0x05);
		lcdcRegSetMask(0x70, 0x07, 0x06);
		/// MODIFY_END LCD点灯
		sig_sem(s_idSem);					// セマフォ返却
	}
	
	return ercd;
	
}
/*==========================================================================*/
/*
 * LCDパネルのバックライトを消灯
 * @return E_OK 成功
 */
/*==========================================================================*/
ER LcdcBackLightOff(void)
{
	ER ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// セマフォの取得	
	if (ercd == E_OK) {	
		/// MODIFY_BEGIN バックライトOFFできないに対応 '13/5/7
//		lcdcRegSetMask(0x70, 0x07, 0x00);
		lcdcRegSetMask(0x70, 0x07, 0x04);
		lcdcRegSetMask(0x70, 0x04, 0x00);
		/// MODIFY_END バックライトOFFできないに対応 '13/5/7
		sig_sem(s_idSem);					// セマフォ返却
	}		

	return ercd;
}
/*==========================================================================*/
/*
 * LCDパネルにカラーバーを表示
 * @return E_OK 成功
 */
/*==========================================================================*/
ER LcdcTestColorBarSet(void)
{
	ER ercd;

	ercd = twai_sem(s_idSem, (100/MSEC));	// セマフォの取得	
	if (ercd == E_OK) {	
		lcdcRegSetMask(0x2a, 0x0f, 0x0f);
		lcdcRegSetMask(0x50, 0x80, 0x80);
		lcdcRegSetMask(0x50, 0x80, 0x00);
	
		lcdcRegSet(0x6e, 0x08);		
	
		sig_sem(s_idSem);					// セマフォ返却
	}
	
	return ercd;
}


