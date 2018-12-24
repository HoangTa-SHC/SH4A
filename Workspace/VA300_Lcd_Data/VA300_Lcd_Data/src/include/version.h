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
  const char VER_MAJOR    = '0';
  const char VER_MINOR    = '0';
  const char VER_REVISION = '1';
 #else
  extern const char VER_MAJOR;
  extern const char VER_MINOR;
  extern const char VER_REVISION;
 #endif
#endif

