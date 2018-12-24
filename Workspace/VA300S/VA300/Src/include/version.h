/**
*	VA-300プログラム
*
*	@file version.h
*	@version 0.01
*
*	@date   2012/08/01
*	@brief  ドライバ共通定義情報(他案件から流用)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/

#ifndef _VERSION_
#define _VERSION_

 #if defined(_MAIN_)
//  static UB Senser_soft_VER[ 4 ] = { '1','.','2','0' };		// センサー端末SH4　ソフトウェア・バージョン番号
//  static UB Senser_FPGA_VER[ 5 ] = { '1','.','0','0','1' };	// センサー端末FPGA　ソフトウェア・バージョン番号
//  static UB Ninshou_soft_VER[ 4 ] = { '1','.','3','0' };		// 制御BOX　Linux認証ソフトウェア・バージョン番号
//  static UB KeyIO_board_soft_VER[ 4 ] = { '1','.','0','0' };	// 制御BOX　錠制御SH2　ソフトウェア・バージョン番号

// VA300Sからのバージョン
//20140423 GA西神中央先行設置改造
//20140522 GA西神中央先行設置改造 認証NG時｢ピッピッ｣音が鳴り続ける不具合対策
//         電気錠開錠施錠の制御時間変更 MAX1sec→MAX3sec
//  const UB Senser_soft_VER[ 4 ] = { '2','.','0','2' };			// センサー端末SH4　ソフトウェア・バージョン番号
// const UB Senser_FPGA_VER[ 5 ] = { '1','.','0','0','1' };		// センサー端末FPGA　ソフトウェア・バージョン番号(20130214 VA300と同じ)
//  const UB Ninshou_soft_VER[ 4 ] = { '2','.','0','1' };			// 制御BOX　Linux認証ソフトウェア/認証アルゴ・バージョン番号
// const UB KeyIO_board_soft_VER[ 4 ] = { '2','.','0','2' };		// 制御BOX　錠制御SH2　ソフトウェア・バージョン番号

//20140905 認証アルゴ(LBP)、FPGA撮影シーケンス変更

// センサー端末SH4　ソフトウェア・バージョン番号
  //const UB Senser_soft_VER[ 4 ] = { '2','.','0','3' };			// センサー端末SH4　ソフトウェア・バージョン番号
  //const UB Senser_soft_VER[ 5 ] = { '2','.','0','3','5' };
  //const UB Senser_soft_VER[ 5 ] = { '2','.','0','3','6' };		//20150531Miya 位置減点緩和(NEWALGO=1)
  //const UB Senser_soft_VER[ 5 ] = { '2','.','0','3','7' };			// 認証失敗時の待ち画面30秒TimeUpへ変更。
  //const UB Senser_soft_VER[ 5 ] = { '2','.','0','3','8' };			//20151118Miya 極小認証見直し
  //const UB Senser_soft_VER[ 5 ] = { '2','.','1','0','0' };			//20160108Miya FinKeyS
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','0' };			//20160515 T.Nagai New Camera(日本ケミコン製カメラ対応)
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','1' };			//20160810Miya(日本ケミコン製カメラ対応アルゴ1)
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','3' };			//20160810Miya(日本ケミコン製カメラ対応アルゴ1)
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','4' };			//20160930Miya PCからVA300Sを制御する
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','6' };				//20170711Miya
  const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','7' };				//20171206Miya 画面フリーズ対策

// センサー端末FPGA　ソフトウェア・バージョン番号(20130214 VA300と同じ)		
  const UB Senser_FPGA_VER[ 5 ] = { '1','.','0','0','2' };		

// 制御BOX　Linux認証ソフトウェア/認証アルゴ・バージョン番号
  //const UB Ninshou_soft_VER[ 4 ] = { '2','.','0','2' };		
  const UB Ninshou_soft_VER[ 4 ] = { '2','.','0','3' };				//20150531Miya 位置減点緩和(NEWALGO=1)		

 
 
//  const UB KeyIO_board_soft_VER[ 4 ] = { '2','.','0','2' };		// 制御BOX　錠制御SH2　ソフトウェア・バージョン番号

// static UB KeyIO_board_soft_VER[ 4 ];		// 制御BOX　錠制御SH2　ソフトウェア・バージョン番号
											// VA-300, VA-300s兼用。
	//　↑　定義はva300.h、デホルト設定は、関数"power_on_process"　へ移動。2014.9.29　T.Nagai

  const char VER_MAJOR    = '0';
  const char VER_MINOR    = '0';
  const char VER_REVISION = '1';
 
 #else
//  extern unsigned char Senser_soft_VER[ 4 ];
//  extern unsigned char Senser_FPGA_VER[ 5 ];
//  extern unsigned char Ninshou_soft_VER[ 4 ];
//  extern unsigned char KeyIO_board_soft_VER[ 4 ];
  extern const unsigned char Senser_soft_VER[ 4 ];
  extern const unsigned char Senser_FPGA_VER[ 5 ];
  extern const unsigned char Ninshou_soft_VER[ 4 ];
//  extern const unsigned char KeyIO_board_soft_VER[ 4 ];
 
  extern const char VER_MAJOR;
  extern const char VER_MINOR;
  extern const char VER_REVISION;

 #endif
#endif

