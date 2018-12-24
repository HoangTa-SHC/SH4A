/**
*	VA-300�v���O����
*
*	@file err_ctrl.h
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/08/01
*	@brief  �G���[�X�e�[�^�X��`���(���Č����痬�p)
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#ifndef	_ERR_CTRL_H_
#define	_ERR_CTRL_H_

// ��`
typedef struct {
	FP	task;							///< �^�X�N�A�h���X
	B	*byTaskName;					///< �^�X�N
	UB	ubErrType;						///< �G���[�^�C�v
	int	ercd;							///< �G���[�R�[�h
}ST_ERR;

enum MAIN_ERR_CODE {					///< ���C���G���[�R�[�h��`
	MAIN_ERR_NONE,						///< �G���[�R�[�h��
	
	MAIN_ERR_LAN = 7,					///< �ʐM�ُ�
	MAIN_ERR_PROGRAM,					///< �v���O�����G���[
	MAIN_ERR_RTOS,						///< ���A���^�C��OS�G���[
	MAIN_ERR_FPGA,						///< FPGA�G���[
	MAIN_ERR_PARM,						///< �p�����[�^�G���[
};

enum SUB_ERR_PRGERR {					///< �v���O�����G���[�̃T�u�R�[�h
	SUB_ERR_PRG_CODE,					///< �v���O�����R�[�h�G���[
	
};

enum SUB_ERR_PARMERR {
	SUB_ERR_PARM_CHG = 1,				///< ����v���p�e�B�G���[
};

// FPGA�G���[��`
#define	FPGA_ERR_EEP			0x04	///< EEPROM�G���[
#define	FPGA_ERR_DMA			0x02	///< DMA�]���G���[

#define	ErrCodeSet(ercd)	ErrStatusSet(MAIN_ERR_RTOS, (int)ercd, __FILE__, __LINE__)
#define	PrgErrSet()			ErrStatusSet(MAIN_ERR_PROGRAM, SUB_ERR_PRG_CODE, __FILE__, __LINE__)

// �v���g�^�C�v�錾
#if defined(_ERR_CTRL_C_)
void ErrStatusInit(void);				// �G���[������
ER ErrStatusSet(UB, int, char*, int);	// �G���[�X�e�[�^�X�ݒ�
void ErrStatusGet(UB*, UB*);			// �ŐV�̃G���[�R�[�h�Ǐo��
void ErrLog(char *p);					// �G���[���O������
#else
extern void ErrStatusInit(void);		// �G���[������
extern ER ErrStatusSet(UB, int, char*, int);	// �G���[�X�e�[�^�X�ݒ�
extern void ErrStatusGet(UB*, UB*);		// �ŐV�̃G���[�R�[�h�Ǐo��
extern void ErrLog(char *p);			// �G���[���O������
#endif
#endif									/* end of _ERR_CTRL_H_				*/
