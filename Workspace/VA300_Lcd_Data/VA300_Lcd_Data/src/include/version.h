/**
*	VA-300�v���O����
*
*	@file version.h
*	@version 0.01
*
*	@date   2012/08/01
*	@brief  �h���C�o���ʒ�`���(���Č����痬�p)
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

