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
//  static UB Senser_soft_VER[ 4 ] = { '1','.','2','0' };		// �Z���T�[�[��SH4�@�\�t�g�E�F�A�E�o�[�W�����ԍ�
//  static UB Senser_FPGA_VER[ 5 ] = { '1','.','0','0','1' };	// �Z���T�[�[��FPGA�@�\�t�g�E�F�A�E�o�[�W�����ԍ�
//  static UB Ninshou_soft_VER[ 4 ] = { '1','.','3','0' };		// ����BOX�@Linux�F�؃\�t�g�E�F�A�E�o�[�W�����ԍ�
//  static UB KeyIO_board_soft_VER[ 4 ] = { '1','.','0','0' };	// ����BOX�@������SH2�@�\�t�g�E�F�A�E�o�[�W�����ԍ�

// VA300S����̃o�[�W����
//20140423 GA���_������s�ݒu����
//20140522 GA���_������s�ݒu���� �F��NG����s�b�s�b������葱����s��΍�
//         �d�C���J���{���̐��䎞�ԕύX MAX1sec��MAX3sec
//  const UB Senser_soft_VER[ 4 ] = { '2','.','0','2' };			// �Z���T�[�[��SH4�@�\�t�g�E�F�A�E�o�[�W�����ԍ�
// const UB Senser_FPGA_VER[ 5 ] = { '1','.','0','0','1' };		// �Z���T�[�[��FPGA�@�\�t�g�E�F�A�E�o�[�W�����ԍ�(20130214 VA300�Ɠ���)
//  const UB Ninshou_soft_VER[ 4 ] = { '2','.','0','1' };			// ����BOX�@Linux�F�؃\�t�g�E�F�A/�F�؃A���S�E�o�[�W�����ԍ�
// const UB KeyIO_board_soft_VER[ 4 ] = { '2','.','0','2' };		// ����BOX�@������SH2�@�\�t�g�E�F�A�E�o�[�W�����ԍ�

//20140905 �F�؃A���S(LBP)�AFPGA�B�e�V�[�P���X�ύX

// �Z���T�[�[��SH4�@�\�t�g�E�F�A�E�o�[�W�����ԍ�
  //const UB Senser_soft_VER[ 4 ] = { '2','.','0','3' };			// �Z���T�[�[��SH4�@�\�t�g�E�F�A�E�o�[�W�����ԍ�
  //const UB Senser_soft_VER[ 5 ] = { '2','.','0','3','5' };
  //const UB Senser_soft_VER[ 5 ] = { '2','.','0','3','6' };		//20150531Miya �ʒu���_�ɘa(NEWALGO=1)
  //const UB Senser_soft_VER[ 5 ] = { '2','.','0','3','7' };			// �F�؎��s���̑҂����30�bTimeUp�֕ύX�B
  //const UB Senser_soft_VER[ 5 ] = { '2','.','0','3','8' };			//20151118Miya �ɏ��F�،�����
  //const UB Senser_soft_VER[ 5 ] = { '2','.','1','0','0' };			//20160108Miya FinKeyS
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','0' };			//20160515 T.Nagai New Camera(���{�P�~�R�����J�����Ή�)
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','1' };			//20160810Miya(���{�P�~�R�����J�����Ή��A���S1)
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','3' };			//20160810Miya(���{�P�~�R�����J�����Ή��A���S1)
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','4' };			//20160930Miya PC����VA300S�𐧌䂷��
  //const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','6' };				//20170711Miya
  const UB Senser_soft_VER[ 5 ] = { '2','.','2','0','7' };				//20171206Miya ��ʃt���[�Y�΍�

// �Z���T�[�[��FPGA�@�\�t�g�E�F�A�E�o�[�W�����ԍ�(20130214 VA300�Ɠ���)		
  const UB Senser_FPGA_VER[ 5 ] = { '1','.','0','0','2' };		

// ����BOX�@Linux�F�؃\�t�g�E�F�A/�F�؃A���S�E�o�[�W�����ԍ�
  //const UB Ninshou_soft_VER[ 4 ] = { '2','.','0','2' };		
  const UB Ninshou_soft_VER[ 4 ] = { '2','.','0','3' };				//20150531Miya �ʒu���_�ɘa(NEWALGO=1)		

 
 
//  const UB KeyIO_board_soft_VER[ 4 ] = { '2','.','0','2' };		// ����BOX�@������SH2�@�\�t�g�E�F�A�E�o�[�W�����ԍ�

// static UB KeyIO_board_soft_VER[ 4 ];		// ����BOX�@������SH2�@�\�t�g�E�F�A�E�o�[�W�����ԍ�
											// VA-300, VA-300s���p�B
	//�@���@��`��va300.h�A�f�z���g�ݒ�́A�֐�"power_on_process"�@�ֈړ��B2014.9.29�@T.Nagai

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

