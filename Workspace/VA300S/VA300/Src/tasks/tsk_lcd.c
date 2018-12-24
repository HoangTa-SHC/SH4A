/**
*	VA-300テストプログラム
*
*	@file tsk_lcd.c
*	@version 1.00
*
*	@author OYO Electric Co.Ltd F.Saeki
*	@date   2012/09/28
*	@brief  LCD表示タスク
*
*	Copyright (C) 2012, OYO Electric Corporation
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#include "kernel.h"
#include "sh7750.h"
#include "id.h"
#include "drv_fpga.h"
#include "err_ctrl.h"

#include "va300.h"
#include "drv_buz.h"
#include "drv_tpl.h"


#include "version.h"

//extern unsigned char Senser_soft_VER[ 4 ];
//extern static UB Senser_soft_VER[ 4 ];




#define LCDX	480
#define LCDY	272

#define LCDBACKLIGHTON	1
#define LCDBACKLIGHTOFF	0

#define INITMODE10KEYGMNBUF	1
#define GMN10KEYCOADE 10
#define GMNMKEYCOADE 11
#define GMN10KEYKIGOUCOADE	12	//20140925Miya password open
#define GMN10KEYHIDECOADE	13	//20140925Miya password open
#define GMNERRMESCODE 20
#define GMNKEYHEADCODE 30
#define GMNNOBTNCODE 40

#define MOJICODEAGYOU	1
#define MOJICODEKAGYOU	2
#define MOJICODESAGYOU	3
#define MOJICODETAGYOU	4
#define MOJICODENAGYOU	5
#define MOJICODEHAGYOU	6
#define MOJICODEMAGYOU	7
#define MOJICODEYAGYOU	8
#define MOJICODERAGYOU	9
#define MOJICODEWAGYOU	10
#define MOJICODEGAGYOU	11
#define MOJICODEZAGYOU	12
#define MOJICODEDAGYOU	13
#define MOJICODEBAGYOU	14
#define MOJICODEPAGYOU	15
#define MOJICODECAN		0xff
#define MOJICODEDEL		0xfe
#define MOJICODERTN		0xfd
#define MOJICODECSR		0xfc
#define MOJIKODEDAKUTN	0xfb


#define KEY1	0
#define KEY2	1
#define KEY3	2
#define KEYCAN	3
#define KEY4	4
#define KEY5	5
#define KEY6	6
#define KEYDEL	7
#define KEY7	8
#define KEY8	9
#define KEY9	10
#define KEYRTN	11
#define KEY0	12
#define KEYSP1	13
#define KEYKIGOU	13
#define KEYSP2	14
#define KEYCSR	14
#define KEYSP3	15

#define SELON	1
#define SELOFF	0

#define SEL_0	0
#define SEL_1	1
#define SEL_2	2


enum sousa{
	GMN_NOSOUSA,	//0:何もしない
	GMN_MENUSOUSA,	//1:メニュー選択
	GMN_YNSOUSA,	//2:Yes/No
	GMN_SELSOUSA,	//3:要素選択
	GMN_KEYSOUSA,	//4:キー操作
	GMN_WAITSOUSA,	//5:タイマー操作
	GMN_DEFSOUSA,	//6:その他
	GMN_OKSOUSA		//7:確認操作(OK操作)
};




// 変数定義
static ID s_idTsk;

// プロトタイプ宣言
static TASK LcdTask( void );		///< LCD表示タスク


// タスクの定義
const T_CTSK ctsk_lcd = { TA_HLNG, NULL, LcdTask, 7, 2048, NULL, (B *)"lcd task" };//

static void SetGmnToLcdBuff(int buf_num, int gmn_num, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
static void SetPartGmnToLcdBuff(int buf_num, int gmn_num, int times, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);//20161031Miya Ver2204 LCDADJ
static UH GetPixDataFromGamen01(int buf, unsigned long cnt);
static UH GetPixDataFromGamen100(int buf, unsigned long cnt);
static UH GetPixDataFromGamen600(int buf, unsigned long cnt);
static UH GetPixDataFromGamenAdd(int gnum, unsigned long cnt);
static UH GetPixDataFromGamen200(int buf, unsigned long cnt);
UB LcdProcMain(int buf_num, int gmn_num, UINT *size, UB *msg);
UB TouchKeyProc(int buf_num, int num, UINT *msize, UB *msg);
UB KeyInputNumSousa( int gmn, int buf, int *keta, int btn );
UB KeyInputNumSousaPassKaijyou( int gmn, int buf, int *keta, int btn );
UB KeyInputMojiSousa( int buf, int *hit, int *keta, int btn );
UB KeyPageSousa( int *page, int btn );
void SetVerNum(int buf_num);
void SetMainteSelBtn(int buf_num, int line, int sel, int sw, int mode);
void SetMenuNoBtn(int buf_num);
void SetErrMes(int buf_num, int mes_num);
void SetKeyHeadMes(int buf_num, int mes_num);
void SetKinkyuNum1(int buf_num, int keta, int key);
void SetKinkyuNum2(int buf_num, int keta, int key);
void SetKeyNum(int page, int keta, int key);
void SetKeyNumPara(int buf_num, int type);
void SetKeyNumPassWord(int gmn, int buf_num, int keta, int key);
void SetKeyMoji(int buf_num, int moji, int hit, int keta, int key);
void SetInfo(int page);
UH GetPixDataFromInputGmn(int buf, int sub_num1, int sub_num2, unsigned long cnt);

void SetPassKeyBtn(int buf_num, int ket_type, int randam);
void SetGmnToLcdBuffPassBtn(int buf_num, int key_num, int key_type, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGmnToLcdBuffKeyHeadMesBanKi(int buf_num, int key_num, int key_type, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
UH GetPixDataFromPassBtn(int type, int num, unsigned long cnt);


UB WaitKeyProc(int sousa, int tflg, UB *msg);
//void SetGamen01(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
//void SetGamen02(int buf, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
//UH GetPixDataFromGamen02(int buf, unsigned long cnt);
void SetGmnToLcdBuff(int buf_num, int gmn_num, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGmnToLcdBuffInImage(int buf_num, int type);
void SetGmnToLcdBuff02(int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGmnToLcdBuff03(int offset, int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGmnToLcdBuff04(int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
void SetGmnToLcdBuff05(int buf_num, int para, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy);
//UINT DbgSendMessageToMain(int num, int sousa, UB *msg);
UH GetPixDataFromGamen400(int buf, unsigned long cnt);
UH GetPixDataFromGamen500(int buf, unsigned long cnt);
void shuffle10( int *key);

/*
extern const unsigned short BtnPosType1[4][2];
extern const unsigned short BtnPosType2[10][2];
extern const unsigned short BtnPosType3[14][2];
extern const unsigned short BtnPosType4[14][2];
extern const unsigned short BtnPosType5[4][2];
extern const unsigned short BtnPosType6[32][2];
extern const unsigned short BtnPosType7[4][2];
extern const unsigned short BtnPosType8[2][2];
extern const unsigned short InpPosKey[16][2];
extern const unsigned short InpPos1[8][2];
extern const unsigned short InpPos2[8][2];
extern const unsigned short InpPos3[4][2];
*/

extern const unsigned short LcdGmn000[];
extern const unsigned short LcdGmn001[];
extern const unsigned short LcdGmn002[];
extern const unsigned short LcdGmn005[];
extern const unsigned short LcdGmn006[];
extern const unsigned short LcdGmn007[];
extern const unsigned short LcdGmn008[];
extern const unsigned short LcdGmn009[];
extern const unsigned short LcdGmn101[];
extern const unsigned short LcdGmn107[];
extern const unsigned short LcdGmn113[];
extern const unsigned short LcdGmn114[];
extern const unsigned short LcdGmn115[];
extern const unsigned short LcdGmn116[];
extern const unsigned short LcdGmn117[];
extern const unsigned short LcdGmn118[];
extern const unsigned short LcdGmn201[];
extern const unsigned short LcdGmn202[];
extern const unsigned short LcdGmn203[];
extern const unsigned short LcdGmn204[];
extern const unsigned short LcdGmn205[];
extern const unsigned short LcdGmn206[];
extern const unsigned short LcdGmn207[];
extern const unsigned short LcdGmn301[];
extern const unsigned short LcdGmn302[];
extern const unsigned short LcdGmn303[];
extern const unsigned short LcdGmn307[];
extern const unsigned short LcdGmn308[];
//extern const unsigned short LcdGmn309[];
extern const unsigned short LcdGmn401[];
//extern const unsigned short LcdGmn402[];
extern const unsigned short LcdGmn403[];
extern const unsigned short LcdGmn404[];
//extern const unsigned short LcdGmn405[];
extern const unsigned short LcdGmn407[];
extern const unsigned short LcdGmn408[];
extern const unsigned short LcdGmnDataNum[10][2560];
extern const unsigned short LcdGmnDataKigou[10][2560];
extern const unsigned short LcdGmnDataX[2][2560];
extern const unsigned short LcdGmnDataAgyou[10][2560];
extern const unsigned short LcdGmnDataKAgyou[5][2560];
extern const unsigned short LcdGmnDataSAgyou[5][2560];
extern const unsigned short LcdGmnDataTAgyou[6][2560];
extern const unsigned short LcdGmnDataNAgyou[5][2560];
extern const unsigned short LcdGmnDataHAgyou[5][2560];
extern const unsigned short LcdGmnDataMAgyou[5][2560];
extern const unsigned short LcdGmnDataYAgyou[6][2560];
extern const unsigned short LcdGmnDataRAgyou[5][2560];
extern const unsigned short LcdGmnDataWAgyou[6][2560];
extern const unsigned short LcdGmnDataGAgyou[5][2560];
extern const unsigned short LcdGmnDataZAgyou[5][2560];
extern const unsigned short LcdGmnDataDAgyou[5][2560];
extern const unsigned short LcdGmnDataBAgyou[5][2560];
extern const unsigned short LcdGmnDataPAgyou[5][2560];
extern const unsigned short LcdBoardMes01[];
extern const unsigned short LcdBoardMes02[];
extern const unsigned short LcdBoardMes03[];
extern const unsigned short LcdBoardMes04[];
extern const unsigned short LcdBoardMes05[];
extern const unsigned short LcdBoardMes06[];
extern const unsigned short LcdBoardMes07[];
extern const unsigned short LcdBoardMes08[];
extern const unsigned short LcdBoardMes09[];
extern const unsigned short LcdBoardMes10[];
extern const unsigned short LcdBoardMes11[];

/**/
extern const unsigned short MesPosition[2][2];
extern const unsigned short KeyHeadMesPosition[4][2];
extern const unsigned short kinkyuNumPos1[16][2];
extern const unsigned short kinkyuNumPos2[16][2];
/**/

extern const unsigned short LcdMes001[];
extern const unsigned short InpGmnMigi[];
extern const unsigned short InpGmnHidari[];
extern const unsigned short InpGmnYubi01[];
extern const unsigned short InpGmnYubi02[];
extern const unsigned short InpGmnYubi03[];
extern const unsigned short InpGmnYubi04[];
extern const unsigned short InpGmnYubi05[];
extern const unsigned short DispFontNum[13][576];
extern const unsigned short SelBtnOn[8][2016];
extern const unsigned short SelBtnOff[8][2016];
extern const unsigned short LcdKeyTopNum[10][8192];
extern const unsigned short LcdKeyTopKigou[10][8192];
extern const unsigned short LcdKeyTopMesBanKi[2][6272];

//専用部
extern const unsigned short LcdGmn102[];
extern const unsigned short LcdGmn103[];
extern const unsigned short LcdGmn104[];
extern const unsigned short LcdGmn105[];
extern const unsigned short LcdGmn106[];
extern const unsigned short LcdGmn108[];
//extern const unsigned short LcdGmn109[];
extern const unsigned short LcdGmn208[];
extern const unsigned short LcdGmn209[];

//FibKeyS
extern const unsigned short LcdGmn109[];
extern const unsigned short LcdGmn211[];
extern const unsigned short Title_rus[];			//お留守番設定
extern const unsigned short MenuBtnData[4][39168];	//[0]空ぼたん　[1]メンテナンス　[2]※　[3]戻る
extern const unsigned short OsiraseMes[7][34816];	//0:スペース 1:登録を続けますか 2:中止しますか 3:削除しますか
													//4:終了しますか 5:初期化しますか 6:工事用ボタン 

//法人(1対1)
extern const unsigned short LcdGmn111[];
extern const unsigned short LcdGmn112[];
extern const unsigned short LcdGmn304[];
extern const unsigned short LcdGmn305[];
extern const unsigned short LcdGmn306[];
extern const unsigned short LcdMenuNoBtn01[];
extern const unsigned short LcdMenuNoBtn02[];
extern const unsigned short LcdMenuNoBtn03[];

//extern UB Senser_soft_VER[ 4 ];

const unsigned short BtnPosType1[4][2] =
{
	{140,  52}, {212, 220},		//登録
	{268,  52}, {340, 220}		//メンテナンス
};

const unsigned short BtnPosType2[10][2] =
//const unsigned short BtnPosType2[6][2] =
{
	{ 58,  52}, {102, 220},		//登録
	{138,  52}, {182, 220},		//削除
	{218,  52}, {262, 220},		//緊急番号設定
	{298,  52}, {342, 220},		//緊急解錠
	{378,  52}, {422, 220}		//メンテナンス
};

const unsigned short BtnPosType3[14][2] =
{
	{ 19,  16}, { 61,  58},		//中止
	{ 90,  20}, {134, 252},		//責任者1
	{172,  20}, {214, 252},		//責任者2
	{250,  20}, {294, 252},		//責任者3
	{330,  20}, {374, 252},		//責任者4
	{410,  20}, {454,  61},		//→ボタン
	{410, 210}, {454, 252}		//←ボタン
};


const unsigned short BtnPosType4[14][2] =
{
	{ 19,  16}, { 61,  58},		//中止
	{170, 183}, {214, 255},		//左人差指
	{274, 183}, {318, 255},		//左中指
	{378, 183}, {422, 255},		//左薬指
	{170,  17}, {214,  89},		//右人差指
	{274,  17}, {318,  89},		//右中指
	{378,  17}, {422,  89},		//右薬指
};

const unsigned short BtnPosType5[6][2] =
{
	{282,  164}, {326, 236},	//はい
	{282,   36}, {326, 108},	//いいえ
	{382,  164}, {422, 236}		//工事	//20160112Miya FinKeyS
};

const unsigned short BtnPosType6[32][2] =
{
	{202,  213}, {245, 256},	//[1]
	{202,  147}, {245, 190},	//[2]
	{202,   81}, {245, 124},	//[3]
	{202,   15}, {245,  58},	//[中止]
	{274,  213}, {317, 256},	//[4]
	{274,  147}, {317, 190},	//[5]
	{274,   81}, {317, 124},	//[6]
	{274,   15}, {317,  58},	//[修正]
	{346,  213}, {389, 256},	//[7]
	{346,  147}, {389, 190},	//[8]
	{346,   81}, {389, 124},	//[9]
	{346,   15}, {389,  58},	//[決定]
	{418,  213}, {461, 256},	//[0]
	{418,  147}, {461, 190},	//[ ]
	{418,   81}, {461, 124},	//[ ]
//	{418,   15}, {461,  58},	//[ ]
	{418,   15}, {461, 124},	//[ ]
};

const unsigned short BtnPosType6kana[32][2] =
{
	{202,  213}, {245, 256},	//[1]
	{202,  147}, {245, 190},	//[2]
	{202,   81}, {245, 124},	//[3]
	{202,   15}, {245,  58},	//[中止]
	{274,  213}, {317, 256},	//[4]
	{274,  147}, {317, 190},	//[5]
	{274,   81}, {317, 124},	//[6]
	{274,   15}, {317,  58},	//[修正]
	{346,  213}, {389, 256},	//[7]
	{346,  147}, {389, 190},	//[8]
	{346,   81}, {389, 124},	//[9]
	{346,   15}, {389,  58},	//[決定]
	{418,  213}, {461, 256},	//[0]
	{418,  147}, {461, 190},	//[ ]
	{418,   81}, {461, 124},	//[ ]
	{418,   15}, {461,  58},	//[ ]
//	{418,   15}, {461, 124},	//[ ]
};


const unsigned short BtnPosType6Pass2[34][2] =
{
	{202,  213}, {245, 256},	//[1]
	{202,  147}, {245, 190},	//[2]
	{202,   81}, {245, 124},	//[3]
	{202,   15}, {245,  58},	//[中止]
	{274,  213}, {317, 256},	//[4]
	{274,  147}, {317, 190},	//[5]
	{274,   81}, {317, 124},	//[6]
	{274,   15}, {317,  58},	//[修正]
	{346,  213}, {389, 256},	//[7]
	{346,  147}, {389, 190},	//[8]
	{346,   81}, {389, 124},	//[9]
	{346,   15}, {389,  58},	//[決定]
	{418,  213}, {461, 256},	//[0]
	{  8,  16}, { 60,  56},		//中止
	//{418,  147}, {461, 190},	//[ ]
	{418,   81}, {461, 124},	//[ ]
//	{418,   15}, {461,  58},	//[ ]
	{418,   15}, {461, 124},	//[ ]
};

const unsigned short BtnPosType7[4][2] =
{
	{ 19,  16}, { 61,  58},		//中止
	{410,  17}, {454,  61},		//→ボタン
};

const unsigned short BtnPosType8[2][2] =
{
	{378,  100}, {422, 172}		//確認, 戻る
};

/*
const unsigned short BtnPosType9[14][2] =
{
	{380,  183}, {420, 254},		//戻る
	{380,   17}, {420,  88},		//確定
	{ 58,  112}, { 77, 149},		//選択1-0
	{ 58,  112}, { 77, 149},		//選択1-1
//	{ 58,   61}, { 77,  98},		//選択1-1
	{ 58,   10}, { 77,  47},		//選択1-2
	{ 98,  112}, {117, 149},		//選択2-0
	{ 98,   61}, {117,  98},		//選択2-1
};
*/
const unsigned short BtnPosType9[34][2] =
{
	{378,  183}, {422, 255},		//戻る
	{378,   17}, {422,  89},		//確定
	{ 56,  110}, { 80, 152},		//占有
	{ 56,   59}, { 80, 101},		//共用
	{ 56,    8}, { 80,  50},		//SMT
	{ 96,   59}, {120, 101},		//ON
	{ 96,    8}, {120,  50},		//OFF
	{ 136,  59}, {160, 101},		//ON
	{ 136,   8}, {160,  50},		//OFF
	{ 176,  59}, {200, 101},		//ON
	{ 176,   8}, {200,  50},		//OFF
	{ 216, 110}, {240, 152},		//0
	{ 216,  59}, {240, 101},		//1
	{ 216,   8}, {240,  50},		//2
	{ 256, 110}, {280, 152},		//0
	{ 256,  59}, {280, 101},		//1
	{ 256,   8}, {280,  50},		//2
};
/*
const unsigned short BtnPosType10[20][2] =
{
	{380,  183}, {420, 254},		//戻る
	{380,   17}, {420,  88},		//確定
	{ 62,   61}, { 81,  98},		//ON
	{ 62,   10}, { 81,  47},		//OFF
	{ 102,  61}, {121,  98},		//ON
	{ 102,  10}, {121,  47},		//OFF
	{ 142,  61}, {161,  98},		//ON
	{ 142,  10}, {161,  47},		//OFF
	{ 182,  61}, {201,  98},		//ON
	{ 182,  10}, {201,  47},		//OFF
};
*/
const unsigned short BtnPosType10[24][2] =
{
	{378,  183}, {422, 255},		//戻る
	{378,   17}, {422,  89},		//確定
	{ 56,   59}, { 80, 101},		//ON
	{ 56,    8}, { 80,  50},		//OFF
	{ 96,   59}, {120, 101},		//ON
	{ 96,    8}, {120,  50},		//OFF
	{136,   59}, {160, 101},		//ON
	{136,    8}, {160,  50},		//OFF
	{176,   59}, {200, 101},		//ON
	{176,    8}, {200,  50},		//OFF
	{216,   59}, {240, 101},		//ON
	{216,    8}, {240,  50},		//OFF
};

const unsigned short BtnPosType11[4][2] =
{
	{220,  99}, {260, 171},		//真ん中
	{378, 100}, {422, 172}		//確認, 戻る
};

//20160108Miya FinKeyS
const unsigned short BtnPosType12[4][2] =
{
	{  8,  16}, { 60,  56},		//中止
	{380,  52}, {420, 220}		//※ボタン
};

const unsigned short InpPosKey[16][2] =
{
	{136, 231}, {175, 262},		//1ケタ
	{136, 199}, {175, 230},		//2ケタ
	{136, 167}, {175, 198},		//3ケタ
	{136, 135}, {175, 166},		//4ケタ
	{136, 103}, {175, 134},		//5ケタ
	{136,  71}, {175, 102},		//6ケタ
	{136,  39}, {175,  70},		//7ケタ
	{136,   7}, {175,  38},		//8ケタ
};

const unsigned short InpPos1[8][2] =
{
	{ 80,  76}, {111, 107},		//責任者1
	{160,  76}, {191, 107},		//責任者2
	{240,  76}, {271, 107},		//責任者3
	{320,  76}, {351, 107},		//責任者4
};

const unsigned short InpPos2[8][2] =
{
	{ 80,  12}, {111, 75},		//責任者1
	{160,  12}, {191, 75},		//責任者2
	{240,  12}, {271, 75},		//責任者3
	{320,  12}, {351, 75},		//責任者4
};

const unsigned short InpPos3[4][2] =
{
	{112, 231},		//責任者1
	{192, 231},		//責任者2
	{272, 231},		//責任者3
	{352, 231},		//責任者4
};

const unsigned short kinkyuNumPos1[16][2] =
{
	{304, 231}, {343, 262},		//1ケタ
	{304, 199}, {343, 230},		//2ケタ
	{304, 167}, {343, 198},		//3ケタ
	{304, 135}, {343, 166},		//4ケタ
	{304, 103}, {343, 134},		//5ケタ
	{304,  71}, {343, 102},		//6ケタ
	{304,  39}, {343,  70},		//7ケタ
	{304,   7}, {343,  38},		//8ケタ
};

const unsigned short kinkyuNumPos2[16][2] =
{
	{344, 231}, {383, 262},		//1ケタ
	{344, 199}, {383, 230},		//2ケタ
	{344, 167}, {383, 198},		//3ケタ
	{344, 135}, {383, 166},		//4ケタ
	{344, 103}, {383, 134},		//5ケタ
	{344,  71}, {383, 102},		//6ケタ
	{344,  39}, {383,  70},		//7ケタ
	{344,   7}, {383,  38},		//8ケタ
};

const unsigned short MesPosition[2][2] =
{
	{352, 0}, {431, 271}		//エラーメッセージ
};

const unsigned short KeyHeadMesPosition[4][2] =
{
	{  0,   0}, {119, 271},		//エラーメッセージ
	{ 48, 174}, { 79, 271}		//番号or記号 //20140925Miya password open
};

const unsigned short MenuBtn[6][2] =
{
	{ 48,  0}, {111, 271},		//監督者
	{128,  0}, {191, 271},		//管理者
	{208,  0}, {271, 271},		//一般者
};


const unsigned short PassBtnPos[22][2] =
{
	{408,  203}, {471, 266},	//[0]
	{192,  203}, {255, 266},	//[1]
	{192,  137}, {255, 200},	//[2]
	{192,   71}, {255, 134},	//[3]
	{264,  203}, {327, 266},	//[4]
	{264,  137}, {327, 200},	//[5]
	{264,   71}, {327, 134},	//[6]
	{336,  203}, {399, 266},	//[7]
	{336,  137}, {399, 200},	//[8]
	{336,   71}, {399, 134},	//[9]
	{  8,    5}, { 71,  68},		//番号or記号
};


extern UB s_ID_Authority_Level; // ID権限問合せコマンドで問合せたユーザーIDの権限レベル 。  ASCIIコード。 
extern UB s_Kantoku_num[ 2 ];	// ID権限問合せ応答コマンドで得た監督者の総数。 ASCIIコード。
extern UB s_Kanri_num[ 2 ];		// ID権限問合せ応答コマンドで得た管理者の総数。 ASCIIコード。
extern UB s_Ippan_num[ 6 ];		// ID権限問合せ応答コマンドで得た一般者の総数。 ASCIIコード。



extern struct{
	unsigned int LcdMsgSize;
	UB LcdMsgBuff[1024];
}g_LcdmsgData;

//extern static UB	s_CapResult;		// 指認証の結果



int sv_reg_num;
UB	sv_keyindat[8];
int sv_mkey;
UB	init_flg;	//初期駆動フラグ
UB sv_yubi_seq_no[4];
UB sv_yubi_no[3];
int sel_lvl;
int ini_reg_cnt;
UB	sv_keyinid[3][4];
int reg_cnt_lvl0;
int reg_cnt_lvl1;
int sv_reg_cnt_lvl;

UB	pass_key_hyouji_flg;	//0:番号 1:記号 //20140925Miya password open
unsigned short g_mem_errnum;	//20141006Miya エラー表示番号記憶

int dbg_Auth_hcnt;

int g_lcdpos[3][2];	//20161031Miya Ver2204 LCDADJ
int g_lcd_adj_f, g_lcd_adj_cnt; //20161031Miya Ver2204 LCDADJ

//extern T_YBDATA yb_touroku_data;	// 指登録情報（１指分）


/*==========================================================================*/
/**
 * LCD表示タスク初期化
 *
 * @param idTsk タスクID
 * @retval E_OK 正常起動
 */
/*==========================================================================*/
ER LcdTaskInit(ID idTsk)
{
	ER ercd;
	
	// タスクの生成
	if (idTsk > 0) {
		ercd = cre_tsk(idTsk, &ctsk_lcd);
		if (ercd == E_OK) {
			s_idTsk = idTsk;
		}
	} else {
		ercd = acre_tsk(&ctsk_lcd);
		if (ercd > 0) {
			s_idTsk = ercd;
		}
	}
	if (ercd < 0) {
		return ercd;
	}
	
	// タスクの起動
	ercd = sta_tsk(s_idTsk, 0);
	
	return ercd;
}

/*==========================================================================*/
/**
 * LCD表示タスク
 * 
 */
/*==========================================================================*/
static TASK LcdTask( void )
{
	FLGPTN	flgptn;
	ER		ercd;
	UB	sts_key;
	UB	perm_enb, perm_r, perm_g, perm_b;
	UH	st_x, st_y, ed_x, ed_y;
	int buf_num, gmn_num;
	
	int i, cnt, num;
	int posx, posy;
	UB	gmn_snd_flg;	//画面転送確認フラグ
	
	/** メッセージ・バッファ用の宣言  **/
	static UB *msg; 		// <-- サイズは120くらいまでの範囲で適宜変えて下さい。
	static UINT msg_size;
	
	float AdjXx, AdjXy, AdjXofst, AdjYx, AdjYy, AdjYofst;	//20161031Miya Ver2204 LCDADJ

#if(FPGA_HI)
	double s1, s2;
#endif

	// LCDの機器Initialize は、ここに記入。
	perm_enb = 0;	//透過色設定 0:未使用 1:使用
	perm_r = 0;		//R透過色
	perm_g = 0;		//G透過色
	perm_b = 0;		//B透過色
	st_x = 0;
	st_y = 0;
	ed_x = 479;
	ed_y = 271;
	gmn_snd_flg = 0;

	//20140925 Miya password open
	for(i = 0 ; i < 10 ; i++ ){
		g_key_arry[i] = i;	//パスワード開錠用キー配列初期化
	}

	msg = &g_LcdmsgData.LcdMsgBuff[0];
	init_flg = 0;
	
	LcdcBackLightOff();
	
	// 通常処理開始
	for(;;) {
		// メインタスクからのの受信待ち
//		ercd = wai_flg( ID_FLG_LCD, ( FPTN_LCD_INIT				// LCD初期画面表示要求(メイン→LCD、ノーマルモード移行の時)
		ercd = twai_flg( ID_FLG_LCD, ( FPTN_LCD_INIT				// LCD初期画面表示要求(メイン→LCD、ノーマルモード移行の時) Modify T.N 2015.3.10
									| FPTN_LCD_SCREEN1			// 画面１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN2			// 画面２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN3			// 画面３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN4			// 画面４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN5			// 画面５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN6			// 画面６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN7			// 画面７表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN8			// 画面８表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN9			// 画面９表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN10			// 画面１０表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN11			// 画面１１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN12			// 画面１２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN100 		// 画面１００表示要求(メイン→LCD)　通常モード
									| FPTN_LCD_SCREEN101 		// 画面１０１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN102 		// 画面１０２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN103 		// 画面１０３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN104 		// 画面１０４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN105 		// 画面１０５表示要求(メイン→LCD)	//20140423Miya 認証リトライ
									| FPTN_LCD_SCREEN106 		// 画面１０５表示要求(メイン→LCD)	//20140423Miya 認証リトライ
									| FPTN_LCD_SCREEN108 		// 画面１０５表示要求(メイン→LCD)	//20140925Miya password_open
									| FPTN_LCD_SCREEN109 		// 画面１０５表示要求(メイン→LCD)	//20140925Miya password_open
									| FPTN_LCD_SCREEN120		// 画面１２０表示要求(メイン→LCD)　通常モード（登録）
									| FPTN_LCD_SCREEN121		// 画面１２１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN122		// 画面１２２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN123		// 画面１２３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN124		// 画面１２４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN125		// 画面１２５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN126		// 画面１２６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN127		// 画面１２７表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN128		// 画面１２８表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN129		// 画面１２９表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN130		// 画面１３０表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN131		// 画面１３１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN132		// 画面１３２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN140		// 画面１４０表示要求(メイン→LCD)　通常モード（削除）
									| FPTN_LCD_SCREEN141		// 画面１４１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN142		// 画面１４２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN143		// 画面１４３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN144		// 画面１４４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN145		// 画面１４５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN146		// 画面１４６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN160		// 画面１４０表示要求(メイン→LCD)　通常モード（削除）
									| FPTN_LCD_SCREEN161		// 画面１４１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN162		// 画面１４２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN163		// 画面１４３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN164		// 画面１４４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN165		// 画面１４５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN170		// 画面１２０表示要求(メイン→LCD)　通常モード（登録）
									| FPTN_LCD_SCREEN171		// 画面１２１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN172		// 画面１２２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN173		// 画面１２３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN174		// 画面１２４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN175		// 画面１２５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN176		// 画面１２６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN177		// 画面１２７表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN178		// 画面１２８表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN180		//20140925Miya passwoed open
									| FPTN_LCD_SCREEN181		//20140925Miya passwoed open
									| FPTN_LCD_SCREEN182		//20140925Miya passwoed open
									| FPTN_LCD_SCREEN183		//20140925Miya passwoed open
									| FPTN_LCD_SCREEN184		//20140925Miya passwoed open
									| FPTN_LCD_SCREEN185		//20140925Miya passwoed open
									| FPTN_LCD_SCREEN186		//20140925Miya passwoed open
									| FPTN_LCD_SCREEN187		//20140925Miya passwoed open
									| FPTN_LCD_SCREEN200		// 画面２００表示要求(メイン→LCD)　メンテナンス・モード
									| FPTN_LCD_SCREEN201		// 画面２０１表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN202		// 画面２０２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN203		// 画面２０３表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN204		// 画面２０４表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN205		// 画面２０５表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN206		// 画面２０６表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN207		// 画面２０７表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN208		// 画面２０８表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN220		//20140925Miya add mainte
									| FPTN_LCD_SCREEN221		//20140925Miya add mainte
									| FPTN_LCD_SCREEN222		//20140925Miya add mainte
									| FPTN_LCD_SCREEN223		//20140925Miya add mainte
									| FPTN_LCD_SCREEN224		//20140925Miya add mainte
									| FPTN_LCD_SCREEN225		//20140925Miya add mainte
									| FPTN_LCD_SCREEN240		//20140925Miya add mainte
									| FPTN_LCD_SCREEN241		//20140925Miya add mainte
									| FPTN_LCD_SCREEN242		//20140925Miya add mainte
									| FPTN_LCD_SCREEN243		//20140925Miya add mainte
									| FPTN_LCD_SCREEN244		//20140925Miya add mainte
									| FPTN_LCD_SCREEN245		//20140925Miya add mainte
									| FPTN_LCD_SCREEN246		//20140925Miya add mainte
									| FPTN_LCD_SCREEN247		//20140925Miya add mainte
									| FPTN_LCD_SCREEN248		//20140925Miya add mainte
									| FPTN_LCD_SCREEN249		//20140925Miya add mainte
									| FPTN_LCD_SCREEN260		//20140925Miya add mainte
									| FPTN_LCD_SCREEN261		//20140925Miya add mainte
									| FPTN_LCD_SCREEN262		//20140925Miya add mainte
									| FPTN_LCD_SCREEN263		//20140925Miya add mainte
									| FPTN_LCD_SCREEN264		//20140925Miya add mainte
									| FPTN_LCD_SCREEN401		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN402		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN403		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN404		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN405		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN406		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN407		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN408		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN409		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN410		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN411		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN500		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN501		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN502		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN503		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN504		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN505		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN506		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN520		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN521		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN522		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN523		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN524		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN525		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN526		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN527		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN528		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN520		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN530		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN531		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN532		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN533		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN534		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN535		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN536		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN537		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN538		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN542		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN543		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN544		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN545		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN546		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN547		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN548		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN549		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN550		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN551		// 画面表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN600		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN601		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN602		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN603		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN604		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN605		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN606		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN607		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN608		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN609		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN610		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN611		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN612		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN613		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN620		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN621		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN622		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN623		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN624		// 2016108Miya FinKeyS
									| FPTN_LCD_SCREEN625		// 2016108Miya FinKeyS
//									| FPTN_LCD_SCREEN999 ), TWF_ORW, &flgptn );	// 画面５２２表示要求(メイン→LCD)
									| FPTN_LCD_SCREEN999 ), TWF_ORW, &flgptn, 50/MSEC );	// 画面５２２表示要求(メイン→LCD)

		lcd_TSK_wdt = FLG_ON;		// LCDタスク　WDTカウンタリセット・リクエスト・フラグ Added T.N 2015.3.10

		if ( ercd == E_TMOUT ){
			continue;
		}

		if ( ercd != E_OK ){
			break;
		}
														
		// 以下に、Switch文などで受信内容に沿った処理を記述
		// この時、処理内容の最初に、
		//		ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_INIT );		// フラグのクリア
		// 		if ( ercd != E_OK ){
		//			break;
		//		}
		//  などで、受信したフラグ毎に、クリア処理を行う。
		//  wai_flg() で使用するフラグパターンは、id.h　を参照のこと。
		// 


		switch(flgptn)
		{
// イニシャライズ ------------------------------------------------------------------------------------//
			case FPTN_LCD_INIT:
				gmn_num = 0;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_INIT );
				if ( ercd != E_OK ){
					break;
				}

				LcdcBackLightOff();
				SetGmnToLcdBuff(0, 0, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcBackLightOn();
				LcdcDisplayModeSet(0, 0);
				init_flg = 1;

				//20160120Miya 初期化後ランダムキーの初期化がなかった
				for(i = 0 ; i < 10 ; i++ ){
					g_key_arry[i] = i;	//パスワード開錠用キー配列初期化
				}

				break;
// 初期モード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN1:
				gmn_num = 1;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN1 );
				if ( ercd != E_OK ){
					break;
				}

				if( init_flg == 1 ){//初期駆動の場合、画面バッファを設定する
					//chg_pri(TSK_SELF, TMIN_TPRI);
					for(cnt = 1 ; cnt < 13 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, gmn_num + cnt -1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
						lcd_TSK_wdt = FLG_ON;	//20160118Miya 画面転送に約21秒かかるのでリセット回避のためON
					}
					//chg_pri(TSK_SELF, TPRI_INI);
					init_flg = 0;
				}else{
					cnt = 1;	//画面1のみ再設定(キ－入力印字のため)
					SetGmnToLcdBuff(cnt, gmn_num + cnt -1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}					

				break;
			case FPTN_LCD_SCREEN2:
				gmn_num = 2;
				buf_num = 2;
				IrLedOnOffSet(1, irDuty2, irDuty3, irDuty4, irDuty5);	//20160312Miya 極小精度UP
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN2 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN3:
				gmn_num = 3;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN3 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN4:
				gmn_num = 4;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN4 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN5:
				gmn_num = 5;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN5 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN6:
				gmn_num = 6;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN6 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN7:
				gmn_num = 7;
				buf_num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN7 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN8:
				gmn_num = 8;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN8 );
				if ( ercd != E_OK ){
					break;
				}
				SetPassKeyBtn(buf_num, 2, 0);	//20160112Miya FinKeyS [中止]ボタン消す
				break;
			case FPTN_LCD_SCREEN9:
				gmn_num = 9;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN9 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN10:
				gmn_num = 10;
				buf_num = 10;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN10 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN11:
				gmn_num = 11;
				buf_num = 11;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN11 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN12:
				gmn_num = 12;
				buf_num = 12;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN12 );
				if ( ercd != E_OK ){
					break;
				}
				break;
// 通常モード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN100:
				gmn_num = 100;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN100 );
				if ( ercd != E_OK ){
					break;
				}

#if ( VA300S == 1 || VA300S == 2 )	
				ercd = set_reg_param_for_Lcd();
#endif
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(0, 0);
				//chg_pri(TSK_SELF, TMIN_TPRI);
				if(g_TechMenuData.SysSpec == 0){	//20160108Miya FinKeyS
					for(cnt = 1 ; cnt <= 10 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, gmn_num + cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
				}else{
					for(cnt = 1 ; cnt <= 13 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, 600 + cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
				}
				//chg_pri(TSK_SELF, TPRI_INI);

				break;
			case FPTN_LCD_SCREEN101:
				gmn_num = 101;
				buf_num = 1;
				IrLedOnOffSet(0, 0, 0, 0, 0);	//20160312Miya 極小精度UP
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN101 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN102:
				gmn_num = 102;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN102 );
				if ( ercd != E_OK ){
					break;
				}
				LcdcBackLightOff();
				break;
			case FPTN_LCD_SCREEN103:
				gmn_num = 103;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN103 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN104:
				gmn_num = 104;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN104 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN105:	//20140423Miya 認証リトライ
				gmn_num = 105;
				buf_num = 5;
				
				Ninshou_wait_timer = NINSHOU_WAIT_TIME;	//20150928Miya ｢もう1度、・・・｣で30秒反応ない場合
				
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN105 );
				if ( ercd != E_OK ){
					break;
				}
				
				SetPassKeyBtn(buf_num, 2, 0);	//20160112Miya FinKeyS [中止]ボタン消す

				break;
			case FPTN_LCD_SCREEN106:	//20140423Miya 認証リトライ
				gmn_num = 106;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN106 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN108:	//20140925Miya password_open
				gmn_num = 108;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN108 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN109:	//20140925Miya password_open
				gmn_num = 109;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN109 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN110:	//20140925Miya password_open
				gmn_num = 110;
				buf_num = 10;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN110 );
				if ( ercd != E_OK ){
					break;
				}
				break;
//20160108Miya FinKeyS ->
// 通常モード2 FinKeyS ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN601:
				gmn_num = 601;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN601 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN602:
				gmn_num = 602;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN602 );
				if ( ercd != E_OK ){
					break;
				}
				LcdcBackLightOff();
				break;
			case FPTN_LCD_SCREEN603:
				gmn_num = 603;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN603 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN604:
				gmn_num = 604;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN604 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN605:	//20140423Miya 認証リトライ
				gmn_num = 605;
				buf_num = 5;
				
				Ninshou_wait_timer = NINSHOU_WAIT_TIME;	//20150928Miya ｢もう1度、・・・｣で30秒反応ない場合
				
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN605 );
				if ( ercd != E_OK ){
					break;
				}

				SetPassKeyBtn(buf_num, 2, 0);	//20160112Miya FinKeyS [中止]ボタン消す
				break;
			case FPTN_LCD_SCREEN606:	//20140423Miya 認証リトライ
				gmn_num = 606;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN606 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN608:	//20140925Miya password_open
				gmn_num = 608;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN608 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN609:	//エラー表示
				gmn_num = 609;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN609 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN610:	//おでかけ設定
				gmn_num = 610;
				buf_num = 10;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN610 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN611:	//お留守番設定
				gmn_num = 611;
				buf_num = 11;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN611 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, 1611, perm_enb, perm_r, perm_g, perm_b, 0, 80, 71, 271);	//お留守番設定
				break;
			case FPTN_LCD_SCREEN612:	//設定メニュー
				gmn_num = 612;
				buf_num = 12;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN612 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, 1612, perm_enb, perm_r, perm_g, perm_b, 288, 0, 359, 271);	//空ボタン
				SetGmnToLcdBuff(buf_num, 1613, perm_enb, perm_r, perm_g, perm_b, 368, 0, 439, 271);	//戻るボタン
				break;
			case FPTN_LCD_SCREEN613:	//しばらくお待ち下さい
				gmn_num = 613;
				buf_num = 13;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN613 );
				if ( ercd != E_OK ){
					break;
				}
				break;
// 工事モード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN620:
				gmn_num = 620;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN620 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 0 ; cnt < 6 ; cnt++){	//画面を画面バッファに転送する 620～625
					SetGmnToLcdBuff(cnt, gmn_num + cnt, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
				gmn_snd_flg = 0;

				break;
			case FPTN_LCD_SCREEN621:
				gmn_num = 621;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN621 );
				if ( ercd != E_OK ){
					break;
				}
				//SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN622:
				gmn_num = 622;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN622 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN623:
				gmn_num = 623;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN623 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN624:
				gmn_num = 624;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN624 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN625:
				gmn_num = 625;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN625 );
				if ( ercd != E_OK ){
					break;
				}
				break;
//20160108Miya FinKeyS <-
// 通常モード(登録) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN120:
				gmn_num = 120;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN120 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 2 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//SetGmnToLcdBuff(cnt, (gmn_num + cnt + 1), 0, 0, 0, 0, 0, 0, 479, 271);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
				gmn_snd_flg = 0;

				break;
			case FPTN_LCD_SCREEN121:
				gmn_num = 121;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN121 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN122:
				gmn_num = 122;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN122 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN123:
				gmn_num = 123;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN123 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN124:
				gmn_num = 124;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN124 );
				if ( ercd != E_OK ){
					break;
				}
			
				if( gmn_snd_flg == 0 ){
					//chg_pri(TSK_SELF, TMIN_TPRI);
					for(cnt = 3 ; cnt <= 9 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, 120 + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					SetGmnToLcdBuff(15, 120 + 15 + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//chg_pri(TSK_SELF, TPRI_INI);
					//gmn_snd_flg = 1;
				}

				break;
			case FPTN_LCD_SCREEN125:
				gmn_num = 129;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN125 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN126:
				gmn_num = 130;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN126 );
				if ( ercd != E_OK ){
					break;
				}
#if(PCCTRL == 1)	//20160930Miya PCからVA300Sを制御する				
				if( gmn_snd_flg == 0 ){
					for(cnt = 3 ; cnt <= 9 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, 120 + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					SetGmnToLcdBuff(15, 120 + 15 + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
#endif				
				break;
			case FPTN_LCD_SCREEN127:
				gmn_num = 131;
				buf_num = 10;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN127 );
				if ( ercd != E_OK ){
					break;
				}

				if( gmn_snd_flg == 0 ){
					//chg_pri(TSK_SELF, TMIN_TPRI);
					for(cnt = 10 ; cnt <= 14 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, 120 + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					//chg_pri(TSK_SELF, TPRI_INI);
					gmn_snd_flg = 1;
				}

				//SetPassKeyBtn(buf_num, 2, 0);	//20160112Miya FinKeyS [中止]ボタン消す

				break;
			case FPTN_LCD_SCREEN128:
				gmn_num = 132;
				buf_num = 11;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN128 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN129:
				gmn_num = 133;
				buf_num = 12;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN129 );
				if ( ercd != E_OK ){
					break;
				}
				
				SetPassKeyBtn(buf_num, 2, 0);	//20160112Miya FinKeyS [中止]ボタン消す
				
				break;
			case FPTN_LCD_SCREEN130:
				gmn_num = 134;
				buf_num = 13;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN130 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN131:
				gmn_num = 135;
				buf_num = 14;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN131 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN132:
				gmn_num = 136;
				buf_num = 15;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN132 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, 1132, perm_enb, perm_r, perm_g, perm_b, 144, 0, 207, 271);	//20160112Miya FinKeyS //初期化しますか？
				break;
			case FPTN_LCD_SCREEN133:
				gmn_num = 136;
				buf_num = 15;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN133 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, 1133, perm_enb, perm_r, perm_g, perm_b, 144, 0, 207, 271);//終了しますか	//20160112Miya FinKeyS //初期化しますか？
				break;
// 通常モード(削除) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN140:
				gmn_num = 140;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN140 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);

				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 2 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
				gmn_snd_flg = 0;

				break;
			case FPTN_LCD_SCREEN141:
				gmn_num = 141;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN141 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN142:
				gmn_num = 142;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN142 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN143:
				gmn_num = 143;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN143 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN144:
				gmn_num = 144;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN144 );
				if ( ercd != E_OK ){
					break;
				}

				if( gmn_snd_flg == 0 ){
					//chg_pri(TSK_SELF, TMIN_TPRI);
					for(cnt = 3 ; cnt <= 9 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, 140 + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					SetGmnToLcdBuff(15, 120 + 15 + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//chg_pri(TSK_SELF, TPRI_INI);
					gmn_snd_flg = 1;
				}
				break;
			case FPTN_LCD_SCREEN145:
				gmn_num = 149;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN145 );
				if ( ercd != E_OK ){
					break;
				}
				//SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//デモのみ
				break;
			case FPTN_LCD_SCREEN146:
				gmn_num = 150;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN146 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, 1133, perm_enb, perm_r, perm_g, perm_b, 144, 0, 207, 271);//終了しますか	//20160112Miya FinKeyS //初期化しますか？
				break;
// 通常モード(緊急解錠番号設定) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN160:
				gmn_num = 160;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN160 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
/*
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 4 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
*/
				gmn_snd_flg = 0;

				break;
			case FPTN_LCD_SCREEN161:
				gmn_num = 161;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN161 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN162:
				gmn_num = 162;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN162 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN163:
				gmn_num = 163;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN163 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN164:
				gmn_num = 164;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN164 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 1);
				break;
			case FPTN_LCD_SCREEN165:
				gmn_num = 165;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN165 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;

// 通常モード(緊急解錠) ------------------------------------------------------------------------------------//

			case FPTN_LCD_SCREEN170:
				gmn_num = 170;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN170 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
/*
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 7 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
*/
				gmn_snd_flg = 0;

				break;
			case FPTN_LCD_SCREEN171:
				gmn_num = 171;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN171 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				for(cnt = 0 ; cnt < 8 ; cnt++){
					if(kinkyuu_tel_no[cnt] == 0x20){
						num = 10;
					}else{
						num = kinkyuu_tel_no[cnt] - 0x30;
						if(num < 0 || num >9 ){
							num = 10;
						}
					}
					
					SetKinkyuNum1(buf_num, cnt+1, num);
				}
				for(cnt = 0 ; cnt < 8 ; cnt++){
					if(kinkyuu_tel_no[cnt+8] == 0x20){
						num = 10;
					}else{
						num = kinkyuu_tel_no[cnt+8] - 0x30;
						if(num < 0 || num >9 ){
							num = 10;
						}
					}
					
					SetKinkyuNum2(buf_num, cnt+1, num);
				}
				
				break;
			case FPTN_LCD_SCREEN172:
				gmn_num = 172;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN172 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN173:
				gmn_num = 173;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN173 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);

				for(cnt = 0 ; cnt < 8 ; cnt++){
					if(kinkyuu_hyouji_no[cnt] == 0x20){
						num = 10;
					}else{
						num = kinkyuu_hyouji_no[cnt] - 0x30;
						if(num < 0 || num >9 ){
							num = 10;
						}
					}
					
					SetKinkyuNum1(buf_num, cnt+1, num);
				}

				break;
			case FPTN_LCD_SCREEN174:
				gmn_num = 174;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN174 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 2);
				break;
			case FPTN_LCD_SCREEN175:
				gmn_num = 175;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN175 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 1);
				break;
			case FPTN_LCD_SCREEN176:
				gmn_num = 176;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN176 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN177:
				gmn_num = 177;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN177 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN178:
				gmn_num = 178;
				buf_num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN178 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
// 通常モード(パスワード開錠設定) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN180:
				gmn_num = 180;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN180 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
				gmn_snd_flg = 0;

				if( gmn_snd_flg == 0 ){
					//chg_pri(TSK_SELF, TMIN_TPRI);
					for(cnt = 3 ; cnt <= 7 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, 188 + cnt - 3, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					//chg_pri(TSK_SELF, TPRI_INI);
					gmn_snd_flg = 1;
				}

				break;
			case FPTN_LCD_SCREEN181:
				gmn_num = 181;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN181 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN182:
				gmn_num = 182;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN182 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN183:
				gmn_num = 183;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN183 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN184:
				gmn_num = 184;
				//buf_num = 3;
				buf_num = 8;	//20160108Miya FinKeyS
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN184 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN185:
				gmn_num = 185;
				//buf_num = 4;
				buf_num = 9;	//20160108Miya FinKeyS
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN185 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN186:
				gmn_num = 186;
				//buf_num = 5;
				buf_num = 10;	//20160108Miya FinKeyS
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN186 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);

				if( g_PasswordOpen.hide_num == FLG_ON ){
					SetMainteSelBtn(buf_num, 1, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 1, 2, SELON, 1);
				}

				if( g_PasswordOpen.kigou_inp == FLG_ON ){
					SetMainteSelBtn(buf_num, 2, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 2, 2, SELON, 1);
				}

				if( g_PasswordOpen.random_key == FLG_ON ){
					SetMainteSelBtn(buf_num, 3, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 3, 2, SELON, 1);
				}
				
				if( g_PasswordOpen2.family_sw == FLG_ON ){	//20160112Miya FinKeyS
					SetMainteSelBtn(buf_num, 4, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 4, 2, SELON, 1);
				}
			
				break;
			case FPTN_LCD_SCREEN187:
				gmn_num = 187;
				//buf_num = 6;
				buf_num = 11;	//20160108Miya FinKeyS
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN187 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN188:
				gmn_num = 188;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN188 );
				if ( ercd != E_OK ){
					break;
				}

				//if( gmn_snd_flg == 0 ){
				//	//chg_pri(TSK_SELF, TMIN_TPRI);
				//	for(cnt = 3 ; cnt <= 9 ; cnt++){	//画面を画面バッファに転送する
				//		SetGmnToLcdBuff(cnt, gmn_num + cnt - 3, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				//	}
				//	//chg_pri(TSK_SELF, TPRI_INI);
				//	gmn_snd_flg = 1;
				//}
				//LcdcDisplayModeSet(buf_num, 0);
				break;

// メンテナンスモード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN200:
				gmn_num = 200;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN200 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
/*
				//chg_pri(TSK_SELF, TMIN_TPRI);
				for(cnt = 1 ; cnt <= 2 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//SetGmnToLcdBuff(cnt, (gmn_num + cnt + 1), 0, 0, 0, 0, 0, 0, 479, 271);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
				gmn_snd_flg = 0;
*/

				break;
			case FPTN_LCD_SCREEN201:
				gmn_num = 201;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN201 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 3);
				//SetVerNum(buf_num);
				break;
			case FPTN_LCD_SCREEN202:
				gmn_num = 202;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN202 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(0, 200, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN203:
				gmn_num = 203;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN203 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN204:
				gmn_num = 204;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN204 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN205:
				gmn_num = 205;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN205 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN206:
				gmn_num = 206;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN206 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN207:
				gmn_num = 207;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN207 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetVerNum(6);
				break;
			case FPTN_LCD_SCREEN208:
				gmn_num = 208;
				buf_num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN208 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				if( GetSysSpec() == SYS_SPEC_MANTION ){
					SetMainteSelBtn(buf_num, 1, 0, SELON, 0);
				}else if( GetSysSpec() == SYS_SPEC_OFFICE ){
					SetMainteSelBtn(buf_num, 1, 2, SELON, 0);
				}else{
					SetMainteSelBtn(buf_num, 1, 1, SELON, 0);
				}
				if( sys_demo_flg == SYS_SPEC_NOMAL ){
					SetMainteSelBtn(buf_num, 2, 1, SELON, 0);
				}else{
					SetMainteSelBtn(buf_num, 2, 0, SELON, 0);
				}
				break;
// メンテナンスモード(情報) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN220:
				gmn_num = 220;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN220 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);

				break;
			case FPTN_LCD_SCREEN221:
				gmn_num = 221;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN221 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN222:
				gmn_num = 222;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN222 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetVerNum(buf_num);
				break;
			case FPTN_LCD_SCREEN223:
				gmn_num = 223;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN223 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN224:
				gmn_num = 224;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN224 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN225:
				gmn_num = 225;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN225 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
// メンテナンスモード(設定変更) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN240:
				gmn_num = 240;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN240 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);

				break;
			case FPTN_LCD_SCREEN241:
				gmn_num = 241;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN241 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN242:
				gmn_num = 242;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN242 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);

				if( g_PasswordOpen.hide_num == FLG_ON ){
					SetMainteSelBtn(buf_num, 1, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 1, 2, SELON, 1);
				}

				if( g_PasswordOpen.kigou_inp == FLG_ON ){
					SetMainteSelBtn(buf_num, 2, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 2, 2, SELON, 1);
				}

				if( g_PasswordOpen.random_key == FLG_ON ){
					SetMainteSelBtn(buf_num, 3, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 3, 2, SELON, 1);
				}

				if( g_PasswordOpen2.family_sw == FLG_ON ){
					SetMainteSelBtn(buf_num, 4, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 4, 2, SELON, 1);
				}

				if( g_PasswordOpen.sw == FLG_ON ){
					SetMainteSelBtn(buf_num, 5, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 5, 2, SELON, 1);
				}

				break;
			case FPTN_LCD_SCREEN243:
				gmn_num = 243;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN243 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 7);
				break;
			case FPTN_LCD_SCREEN244:
				gmn_num = 244;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN244 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 8);
				break;
			case FPTN_LCD_SCREEN245:
				gmn_num = 245;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN245 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 9);
				break;
			case FPTN_LCD_SCREEN246:
				gmn_num = 246;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN246 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				//SetKeyHeadMes(buf_num, 10);	//20161031Miya Ver2204 LCDADJ
				
				//20161031Miya Ver2204 LCDADJ
				SetPartGmnToLcdBuff(buf_num, gmn_num, 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				if(g_TechMenuData.DebugHyouji == FLG_OFF){
					TplRevInit();
				}					

				//TplRevGet(&AdjXx, &AdjXy, &AdjXofst, &AdjYx, &AdjYy, &AdjYofst);
				//TplRevInit();
				//TplRevGet(&AdjXx, &AdjXy, &AdjXofst, &AdjYx, &AdjYy, &AdjYofst);
				nop();
				
				break;
			case FPTN_LCD_SCREEN247:
				gmn_num = 247;
				buf_num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN247 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetKeyHeadMes(buf_num, 11);
				break;
			case FPTN_LCD_SCREEN248:
				gmn_num = 248;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN248 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN249:
				gmn_num = 249;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN249 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
// メンテナンスモード(技術) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN260:
				gmn_num = 260;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN260 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);

				break;
			case FPTN_LCD_SCREEN261:
				gmn_num = 261;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN261 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);

				break;
			case FPTN_LCD_SCREEN262:
				gmn_num = 262;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN262 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetGmnToLcdBuff(buf_num, 1262, perm_enb, perm_r, perm_g, perm_b, 144, 0, 207, 271);	//20160112Miya FinKeyS //初期化しますか？
				SetGmnToLcdBuff(buf_num, 1263, perm_enb, perm_r, perm_g, perm_b, 368, 0, 431, 271);	//20160112Miya FinKeyS //初期化しますか？
				break;
			case FPTN_LCD_SCREEN263:
				gmn_num = 263;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN263 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
/*
				if( g_TechMenuData.SysSpec == 0 ){
					SetMainteSelBtn(buf_num, 1, 1, SELON, 1);
				}else{
					SetMainteSelBtn(buf_num, 1, 2, SELON, 2);
				}
*/
				//20160112Miya FunKeyS
				if( g_TechMenuData.SysSpec == 1 ){
					SetMainteSelBtn(buf_num, 1, 1, SELON, 2);
				}else if( g_TechMenuData.SysSpec == 2 ){
					SetMainteSelBtn(buf_num, 1, 2, SELON, 2);
				}else{
					SetMainteSelBtn(buf_num, 1, 0, SELON, 2);
				}

				if( g_TechMenuData.DemoSw == FLG_ON ){
					SetMainteSelBtn(buf_num, 2, 1, SELON, 2);
				}else{
					SetMainteSelBtn(buf_num, 2, 2, SELON, 2);
				}
				if( g_TechMenuData.HijyouRemocon == FLG_ON ){
					SetMainteSelBtn(buf_num, 3, 1, SELON, 2);
				}else{
					SetMainteSelBtn(buf_num, 3, 2, SELON, 2);
				}
				if( g_TechMenuData.DebugHyouji == FLG_ON ){
					SetMainteSelBtn(buf_num, 4, 1, SELON, 2);
				}else{
					SetMainteSelBtn(buf_num, 4, 2, SELON, 2);
				}
				//SetMainteSelBtn(buf_num, 5, 0, SELON, 2);
				if(g_BkDataNoClear.LedPosi == 0){
					SetMainteSelBtn(buf_num, 6, 0, SELON, 2);
				}else if(g_BkDataNoClear.LedPosi == 1){
					SetMainteSelBtn(buf_num, 6, 1, SELON, 2);
				}else{
					SetMainteSelBtn(buf_num, 6, 2, SELON, 2);
				}

				break;
			case FPTN_LCD_SCREEN264:
				gmn_num = 264;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN264 );
				if ( ercd != E_OK ){
					break;
				}
				
#if(FPGA_HI == 1){	//20160902Miya FPGA高速化
				MakeTestImg();	//20160902Miya FPGA高速化 forDebug
				AutoMachingFPGA(0, 0, &s1, &s2);
#endif
				
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN265:
				gmn_num = 265;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN265 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN266:
				gmn_num = 266;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN266 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN267:
				gmn_num = 267;
				buf_num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN267 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
// 法人仕様初期モード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN401:
				gmn_num = 401;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN401 );
				if ( ercd != E_OK ){
					break;
				}

				if( init_flg == 1 ){//初期駆動の場合、画面バッファを設定する
					for(cnt = 4 ; cnt <= 11 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, gmn_num + cnt - 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					}
					init_flg = 0;
					sv_reg_num = 0;
					sel_lvl = 0;
					ini_reg_cnt = 0;
					memset(&sv_keyinid[0][0], 0, sizeof(UB)*12 );
				}				

				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				break;
			case FPTN_LCD_SCREEN402:
				gmn_num = 402;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN402 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//番号表示消す
				SetKeyHeadMes(buf_num, 5);
				break;
			case FPTN_LCD_SCREEN403:
				gmn_num = 403;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN403 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//番号表示消す
				break;
			case FPTN_LCD_SCREEN404:
				gmn_num = 404;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN404 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN405:
				gmn_num = 405;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN405 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN406:
				gmn_num = 406;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN406 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN407:
				gmn_num = 407;
				buf_num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN407 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN408:
				gmn_num = 408;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN408 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN409:
				gmn_num = 409;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN409 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN410:
				gmn_num = 410;
				buf_num = 10;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN410 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN411:
				gmn_num = 411;
				buf_num = 11;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN411 );
				if ( ercd != E_OK ){
					break;
				}
				break;
// 法人仕様通常モード ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN500:
				gmn_num = 500;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN500 );
				if ( ercd != E_OK ){
					break;
				}

				chg_pri(TSK_SELF, TMIN_TPRI);
				SetGmnToLcdBuff(0, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(0, 0);
				for(cnt = 4 ; cnt <= 6 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt - 1, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				cnt = 14;
				SetGmnToLcdBuff(13, 536, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetGmnToLcdBuff(14, 537, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				SetGmnToLcdBuff(15, 538, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				chg_pri(TSK_SELF, TPRI_INI);

				break;
			case FPTN_LCD_SCREEN501:
				gmn_num = 501;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN501 );
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//番号表示消す
				break;
			case FPTN_LCD_SCREEN502:
				gmn_num = 502;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN502 );
				if ( ercd != E_OK ){
					break;
				}
				LcdcBackLightOff();
				break;
			case FPTN_LCD_SCREEN503:
				gmn_num = 503;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN503 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN504:
				gmn_num = 504;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN504 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN505:
				gmn_num = 505;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN505 );
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN506:
				gmn_num = 506;
				buf_num = 13;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN506 );
				if ( ercd != E_OK ){
					break;
				}
				break;
// 法人仕様通常モード(登録) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN520:
				gmn_num = 520;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN520 );
				if ( ercd != E_OK ){
					break;
				}

				LcdcDisplayModeSet(0, 0);
				//chg_pri(TSK_SELF, TMIN_TPRI);
				SetGmnToLcdBuff(2, 526, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				for(cnt = 7 ; cnt <= 11 ; cnt++){	//画面を画面バッファに転送する
					SetGmnToLcdBuff(cnt, gmn_num + cnt + 2, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				}
				//chg_pri(TSK_SELF, TPRI_INI);
				SetGmnToLcdBuff(12, 535, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);

				break;
			case FPTN_LCD_SCREEN521:
				gmn_num = 521;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN521);
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//番号表示消す
				break;
			case FPTN_LCD_SCREEN522:
				gmn_num = 522;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN522);
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//番号表示消す
				SetKeyHeadMes(buf_num, 4);
				//gmn_snd_flg = 0;
				break;
			case FPTN_LCD_SCREEN523:
				gmn_num = 523;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN523);
				if ( ercd != E_OK ){
					break;
				}
				reg_cnt_lvl0 = (int)s_Kantoku_num[1] - 0x30;
				reg_cnt_lvl1 = 10 * ( (int)s_Kanri_num[0] - 0x30 );
				reg_cnt_lvl1 = reg_cnt_lvl1 + ( (int)s_Kanri_num[1] - 0x30 );
				sv_reg_cnt_lvl = 0;

				break;
			case FPTN_LCD_SCREEN524:
				gmn_num = 524;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN524);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN525:
				gmn_num = 525;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN525);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN526:
				gmn_num = 526;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN526);
				if ( ercd != E_OK ){
					break;
				}
				SetMenuNoBtn(buf_num);
				break;
			case FPTN_LCD_SCREEN527:
				gmn_num = 527;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN527);
				if ( ercd != E_OK ){
					break;
				}
/*
				if(gmn_snd_flg == 0){
					//chg_pri(TSK_SELF, TMIN_TPRI);
					for(cnt = 7 ; cnt <= 11 ; cnt++){	//画面を画面バッファに転送する
						SetGmnToLcdBuff(cnt, gmn_num + cnt - 5, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
						//SetGmnToLcdBuff(cnt, (gmn_num + cnt + 1), 0, 0, 0, 0, 0, 0, 479, 271);
					}
					SetGmnToLcdBuff(12, 535, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//chg_pri(TSK_SELF, TPRI_INI);
					gmn_snd_flg = 1;
				}
*/
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//番号表示消す
				SetKeyHeadMes(buf_num, 5);
				break;
			case FPTN_LCD_SCREEN528:
				gmn_num = 528;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN528);
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//番号表示消す
				break;
			case FPTN_LCD_SCREEN529:
				gmn_num = 529;
				buf_num = 7;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN529);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN530:
				gmn_num = 530;
				buf_num = 8;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN530);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN531:
				gmn_num = 531;
				buf_num = 9;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN531);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN532:
				gmn_num = 532;
				buf_num = 10;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN532);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN533:
				gmn_num = 533;
				buf_num = 11;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN533);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN534:
				gmn_num = 534;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN534);
				if ( ercd != E_OK ){
					break;
				}
				if( sv_reg_cnt_lvl == 0 && reg_cnt_lvl0 > 0){
					--reg_cnt_lvl0;
				}
				if( sv_reg_cnt_lvl == 1 && reg_cnt_lvl1 > 0){
					--reg_cnt_lvl1;
				}
				
				break;
			case FPTN_LCD_SCREEN535:
				gmn_num = 535;
				buf_num = 12;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN535);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN536:
				gmn_num = 536;
				buf_num = 13;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN536);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN537:
				gmn_num = 537;
				buf_num = 14;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN537);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN538:
				gmn_num = 538;
				buf_num = 15;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN538);
				if ( ercd != E_OK ){
					break;
				}
				break;
// 法人仕様通常モード(削除) ------------------------------------------------------------------------------------//
			case FPTN_LCD_SCREEN542:
				gmn_num = 542;
				buf_num = 1;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN542);
				if ( ercd != E_OK ){
					break;
				}
				gmn_snd_flg = 0;
				break;
			case FPTN_LCD_SCREEN543:
				gmn_num = 543;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN543);
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//番号表示消す
				SetKeyHeadMes(buf_num, 4);
				break;
			case FPTN_LCD_SCREEN544:
				gmn_num = 544;
				buf_num = 4;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN544);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN545:
				gmn_num = 545;
				buf_num = 5;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN545);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN546:
				gmn_num = 546;
				buf_num = 6;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN546);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN547:
				gmn_num = 547;
				buf_num = 2;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN547);
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);//番号表示消す
				SetKeyHeadMes(buf_num, 6);
				break;
			case FPTN_LCD_SCREEN548:
				gmn_num = 548;
				buf_num = 3;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN548);
				if ( ercd != E_OK ){
					break;
				}
				//if(gmn_snd_flg == 0){
					//chg_pri(TSK_SELF, TMIN_TPRI);
					SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
					//chg_pri(TSK_SELF, TPRI_INI);
					//gmn_snd_flg = 1;
				//}
				break;
			case FPTN_LCD_SCREEN549:
				gmn_num = 549;
				buf_num = 13;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN549);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN550:
				gmn_num = 550;
				buf_num = 14;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN550);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN551:
				gmn_num = 551;
				buf_num = 15;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN551);
				if ( ercd != E_OK ){
					break;
				}
				break;
			case FPTN_LCD_SCREEN999:
				gmn_num = 999;
				buf_num = 0;
				//////// フラグのクリア ////////
				ercd = clr_flg( ID_FLG_LCD, ~FPTN_LCD_SCREEN551);
				if ( ercd != E_OK ){
					break;
				}
				SetGmnToLcdBuff(buf_num, gmn_num, perm_enb, perm_r, perm_g, perm_b, st_x, st_y, ed_x, ed_y);
				LcdcDisplayModeSet(0, 0);
				break;
			default:
				break;
		}

		if( gmn_num != 0 ){
			memset( msg, 0x20, sizeof(msg) );
			sts_key = LcdProcMain(buf_num, gmn_num, &msg_size, msg);
			g_LcdmsgData.LcdMsgSize = msg_size;
			
			if(sts_key == 1){	//キー押下あり
				///////// フラグセット ////////
				ercd = set_flg(ID_FLG_MAIN, FPTN_LCD_CHG_REQ);
				if(ercd != E_OK){
					//エラ－処理要
				}
			}
			
			if(dbg_cap_flg >= 1){
				dbg_cap_flg = 0;
			}
				
		}
			
	}

	PrgErrSet();
	slp_tsk();					//ここに来るのは、システム・エラー
}


static void SetGmnToLcdBuff(int buf_num, int gmn_num, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;
	int sub_num;

	num = buf_num;
	LcdcDisplayWindowSet(num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	num = gmn_num;
	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;

	if( num == 999 ){
		for(cnt = 0 ; cnt < size ; cnt++ ){
			if(( cnt / 2) == 0 ){
				val1 = 0;
			}else{
				val1 = 0x007F;
			}
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
		return;
	}


	if(num < 100){
		val1 = GetPixDataFromGamen01(num, size);
	}else if(num < 200){
		val1 = GetPixDataFromGamen100(num, size);
	}else if(num < 400){
		val1 = GetPixDataFromGamen200(num, size);
	}else if(num < 500){
		for(cnt = 0 ; cnt < size ; cnt++ ){
			val1 = GetPixDataFromGamen400(num, cnt);
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else if(num < 600){
		for(cnt = 0 ; cnt < size ; cnt++ ){
			val1 = GetPixDataFromGamen500(num, cnt);
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else if(num < 700){
		val1 = GetPixDataFromGamen600(num, size);
	}else{
		if(num == 1611 || num == 1612 || num == 1613 || num == 1262 || num == 1263 || num == 1132 || num == 1133){
			val1 = GetPixDataFromGamenAdd(num, size);
		}
				
	}

	LcdcDisplayWindowFinish();

	//20161031Miya Ver2204
	if(num == 241){
		sub_num = 0; //0:* 1:E
		x1 = 224;
		y1 = 152;
		x2 = x1 + 40 - 1;
		y2 = y1 + 32 - 1;
		SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, 0, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
			
		x1 = 224;
		y1 = 184;
		x2 = x1 + 40 - 1;
		y2 = y1 + 32 - 1;
		SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, 0, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
			
	}

	if(num == 266){
		sub_num = 0; //0:* 1:E
		x1 = 224;
		y1 = 102;
		x2 = x1 + 40 - 1;
		y2 = y1 + 32 - 1;
		SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, 0, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
			
		x1 = 224;
		y1 = 134;
		x2 = x1 + 40 - 1;
		y2 = y1 + 32 - 1;
		SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, 0, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
			
	}
	
	//20161031Miya Ver2204 LCDADJ
/*
	if(num == 246){
		stx = 32;
		edx = 47;
		sty = 32;
		edy = 47;
		LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
		x1 = stx;
		x2 = edx;
		y1 = sty;
		y2 = edy;
		size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
		for(cnt = 0 ; cnt < size ; cnt++ ){
			val1 = 0xFFFF;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
		LcdcDisplayWindowFinish();

		stx = 424;
		edx = 439;
		sty = 224;
		edy = 239;
		LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
		x1 = stx;
		x2 = edx;
		y1 = sty;
		y2 = edy;
		size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
		for(cnt = 0 ; cnt < size ; cnt++ ){
			val1 = 0xFFFF;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
		LcdcDisplayWindowFinish();

		stx = 424;
		edx = 439;
		sty = 32;
		edy = 47;
		LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
		x1 = stx;
		x2 = edx;
		y1 = sty;
		y2 = edy;
		size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
		for(cnt = 0 ; cnt < size ; cnt++ ){
			val1 = 0xFFFF;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
		LcdcDisplayWindowFinish();
	}
*/	
	
}


//20161031Miya Ver2204 LCDADJ
static void SetPartGmnToLcdBuff(int buf_num, int gmn_num, int times, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, val2, val3, num, lst;
	//UH val2;
	unsigned long cnt;

	num = gmn_num;

	if(times == 1){
		val1 = 0xFFFF;
		val2 = 0x0000;
		val3 = 0x0000;
		lst = 0;
	}else if(times == 2){
		val1 = 0x0000;
		val2 = 0xFFFF;
		val3 = 0x0000;
		lst = 0;
	}else if(times == 3){
		val1 = 0x0000;
		val2 = 0x0000;
		val3 = 0xFFFF;
		lst = 0;
	}else{
		val1 = 0xFFFF;
		val2 = 0xFFFF;
		val3 = 0xFFFF;
		lst = 1;
	}

	if(num == 246){
		stx = 32;
		edx = 47;
		sty = 32;
		edy = 47;
		LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
		x1 = stx;
		x2 = edx;
		y1 = sty;
		y2 = edy;
		size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
		for(cnt = 0 ; cnt < size ; cnt++ ){
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
		LcdcDisplayWindowFinish();

		stx = 424;
		edx = 439;
		sty = 224;
		edy = 239;
		LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
		x1 = stx;
		x2 = edx;
		y1 = sty;
		y2 = edy;
		size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
		for(cnt = 0 ; cnt < size ; cnt++ ){
			rtn = LcdImageWrite(val2);	
			if(rtn != E_OK ){
				break;
			}
		}
		LcdcDisplayWindowFinish();

		stx = 424;
		edx = 439;
		sty = 32;
		edy = 47;
		LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
		x1 = stx;
		x2 = edx;
		y1 = sty;
		y2 = edy;
		size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
		for(cnt = 0 ; cnt < size ; cnt++ ){
			rtn = LcdImageWrite(val3);	
			if(rtn != E_OK ){
				break;
			}
		}
		LcdcDisplayWindowFinish();
		
		if(lst == 1){
			stx = 232;
			edx = 247;
			sty = 128;
			edy = 143;
			LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
			x1 = stx;
			x2 = edx;
			y1 = sty;
			y2 = edy;
			size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
			for(cnt = 0 ; cnt < size ; cnt++ ){
				rtn = LcdImageWrite(val3);	
				if(rtn != E_OK ){
					break;
				}
			}
			LcdcDisplayWindowFinish();
		}
		
	}
}

static UH GetPixDataFromGamen01(int buf, unsigned long cnt)
{
	UH val;
	ER rtn;
	unsigned long xx;
	UH val1;
	unsigned short *gmn;
	char zerof;

	rtn = 0;
	zerof = 0;
	
	if(buf == 0){
			gmn = &LcdGmn000[0];
	}else if(buf == 1){
			gmn = &LcdGmn001[0];
	}else if(buf == 2){
			gmn = &LcdGmn101[0];
	}else if(buf == 3){
			gmn = &LcdGmn102[0];
	}else if(buf == 4){
			gmn = &LcdGmn002[0];
	}else if(buf == 5){
			gmn = &LcdGmn107[0];
	}else if(buf == 6){
			gmn = &LcdGmn201[0];
	}else if(buf == 7){
			gmn = &LcdGmn202[0];
	}else if(buf == 8){
			gmn = &LcdGmn203[0];
	}else if(buf == 9){
			gmn = &LcdGmn204[0];
	}else if(buf == 10){
			gmn = &LcdGmn206[0];
	}else if(buf == 11){
			gmn = &LcdGmn301[0];
	}else if(buf == 12){
			gmn = &LcdGmn302[0];
	}else{
			zerof = 1;
	}	
	
	if(zerof == 1){
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = 0x00;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else{
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = *(gmn + xx);
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}

	return((UH)rtn);
}


static UH GetPixDataFromGamen100(int gnum, unsigned long cnt)
{
	UH val;
	ER rtn;
	unsigned long xx;
	UH val1;
	unsigned short *gmnbuf;
	char zerof;

	rtn = 0;
	zerof = 0;

	if( gnum < 120 ){
		if(gnum == 100){
			zerof = 1;
		}else if(gnum == 101){
#if(PCCTRL == 1)	//20160930Miya PCからVA300Sを制御する
			gmnbuf = &LcdGmn201[0];
#else
			//20140925Miya password_open
			if( g_PasswordOpen.sw == FLG_OFF )
				gmnbuf = &LcdGmn108[0];
			else
				gmnbuf = &LcdGmn113[0];
#endif
		}else if(gnum == 102){
			zerof = 1;
		}else if(gnum == 103){
			gmnbuf = &LcdGmn205[0];
		}else if(gnum == 104){
			gmnbuf = &LcdGmn206[0];
		}else if(gnum == 105){		//20140423Miya 認証リトライ
			gmnbuf = &LcdGmn203[0];
		}else if(gnum == 106){		//20140423Miya 認証リトライ
			//gmnbuf = 0x6767;
			zerof = 2;
		}else if(gnum == 107){		//20140925Miya password_open
			zerof = 1;
		}else if(gnum == 108){		//20140925Miya password_open
			if( g_PasswordOpen.kigou_inp == FLG_OFF )
				gmnbuf = &LcdGmn006[0];
			else
				gmnbuf = &LcdGmn008[0];
		}else if(gnum == 109){		//20140925Miya password_open
			gmnbuf = &LcdGmn307[0];
		}else if(gnum == 110){		//20140925Miya password_open
			gmnbuf = &LcdGmn308[0];
		}else{
			zerof = 1;
		}
	}else if( gnum < 140 ){
		if(gnum == 120){
			zerof = 1;
		}else if(gnum == 121){
			gmnbuf = &LcdGmn207[0];
		}else if(gnum == 122){
			gmnbuf = &LcdGmn205[0];
		}else if(gnum == 123){
			gmnbuf = &LcdGmn206[0];
		}else if(gnum == 124){
			gmnbuf = &LcdGmn102[0];
		}else if(gnum == 125){
			gmnbuf = &LcdGmn103[0];
		}else if(gnum == 126){
			gmnbuf = &LcdGmn104[0];
		}else if(gnum == 127){
			gmnbuf = &LcdGmn105[0];
		}else if(gnum == 128){
			gmnbuf = &LcdGmn106[0];
		}else if(gnum == 129){
			gmnbuf = &LcdGmn002[0];
		}else if(gnum == 130){
			gmnbuf = &LcdGmn107[0];
		}else if(gnum == 131){
			gmnbuf = &LcdGmn201[0];
		}else if(gnum == 132){
			gmnbuf = &LcdGmn202[0];
		}else if(gnum == 133){
			gmnbuf = &LcdGmn203[0];
		}else if(gnum == 134){
			gmnbuf = &LcdGmn204[0];
		}else if(gnum == 135){
			gmnbuf = &LcdGmn206[0];
		}else if(gnum == 136){
			gmnbuf = &LcdGmn302[0];
		}else{
			zerof = 1;
		}
	}else if( gnum < 160 ){
		if(gnum == 140){
			zerof = 1;
		}else if(gnum == 141){
			gmnbuf = &LcdGmn207[0];
		}else if(gnum == 142){
			gmnbuf = &LcdGmn205[0];
		}else if(gnum == 143){
			gmnbuf = &LcdGmn206[0];
		}else if(gnum == 144){
			gmnbuf = &LcdGmn102[0];
		}else if(gnum == 145){
			gmnbuf = &LcdGmn103[0];
		}else if(gnum == 146){
			gmnbuf = &LcdGmn104[0];
		}else if(gnum == 147){
			gmnbuf = &LcdGmn105[0];
		}else if(gnum == 148){
			gmnbuf = &LcdGmn106[0];
		}else if(gnum == 149){
			gmnbuf = &LcdGmn303[0];
		}else if(gnum == 150){
			gmnbuf = &LcdGmn302[0];
		}else{
			zerof = 1;
		}
	}else if( gnum < 170 ){
		if(gnum == 160){
			zerof = 1;
		}else if(gnum == 161){
			gmnbuf = &LcdGmn207[0];
		}else if(gnum == 162){
			gmnbuf = &LcdGmn205[0];
		}else if(gnum == 163){
			gmnbuf = &LcdGmn206[0];
		}else if(gnum == 164){
			gmnbuf = &LcdGmn001[0];
		}else if(gnum == 165){
			gmnbuf = &LcdGmn302[0];
		}else{
			zerof = 1;
		}
	}else if( gnum < 180 ){
		if(gnum == 170){
			zerof = 1;
		}else if(gnum == 171){
			gmnbuf = &LcdGmn208[0];
		}else if(gnum == 172){
			gmnbuf = &LcdGmn001[0];
		}else if(gnum == 173){
			gmnbuf = &LcdGmn209[0];
		}else if(gnum == 174){
			gmnbuf = &LcdGmn001[0];
		}else if(gnum == 175){
			gmnbuf = &LcdGmn001[0];
		}else if(gnum == 176){
			gmnbuf = &LcdGmn205[0];
		}else if(gnum == 177){
			gmnbuf = &LcdGmn206[0];
		}else if(gnum == 178){
			gmnbuf = &LcdGmn302[0];
		}else{
			zerof = 1;
		}
	}else{
		if(gnum == 180){
			zerof = 1;
		}else if(gnum == 181){
			gmnbuf = &LcdGmn207[0];
		}else if(gnum == 182){
			gmnbuf = &LcdGmn205[0];
		}else if(gnum == 183){
			gmnbuf = &LcdGmn206[0];
		}else if(gnum == 184){
			gmnbuf = &LcdGmn114[0];
		}else if(gnum == 185){
			if( g_PasswordOpen.kigou_inp == FLG_OFF )
				gmnbuf = &LcdGmn006[0];
			else
				gmnbuf = &LcdGmn008[0];
		}else if(gnum == 186){
			gmnbuf = &LcdGmn403[0];
		}else if(gnum == 187){
			gmnbuf = &LcdGmn302[0];
		}else if(gnum == 188){
			gmnbuf = &LcdGmn102[0];
		}else if(gnum == 189){
			gmnbuf = &LcdGmn103[0];
		}else if(gnum == 190){
			gmnbuf = &LcdGmn104[0];
		}else if(gnum == 191){
			gmnbuf = &LcdGmn105[0];
		}else if(gnum == 192){
			gmnbuf = &LcdGmn106[0];
		}
	}

	if(zerof == 1){
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = 0x00;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else if(zerof == 2){
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = 0x6767;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else{
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = *(gmnbuf + xx);
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}

	return((UH)rtn);
}

//20160108Miya FinKeyS
static UH GetPixDataFromGamen600(int gnum, unsigned long cnt)
{
	UH val;
	ER rtn;
	unsigned long xx;
	UH val1;
	unsigned short *gmnbuf;
	char zerof;

	rtn = 0;
	zerof = 0;

	if( gnum < 620 ){
		if(gnum == 600){
			zerof = 1;
		}else if(gnum == 601){
			gmnbuf = &LcdGmn109[0];
		}else if(gnum == 602){
			zerof = 1;
		}else if(gnum == 603){
			gmnbuf = &LcdGmn205[0];
		}else if(gnum == 604){
			gmnbuf = &LcdGmn206[0];
		}else if(gnum == 605){
			gmnbuf = &LcdGmn203[0];
		}else if(gnum == 606){
			zerof = 2;
		}else if(gnum == 607){
			zerof = 1;
		}else if(gnum == 608){
			if( g_PasswordOpen.kigou_inp == FLG_OFF )
				gmnbuf = &LcdGmn006[0];
			else
				gmnbuf = &LcdGmn008[0];
		}else if(gnum == 609){
			gmnbuf = &LcdGmn307[0];
		}else if(gnum == 610){
			gmnbuf = &LcdGmn211[0];
		}else if(gnum == 611){
			gmnbuf = &LcdGmn211[0];
		}else if(gnum == 612){
			gmnbuf = &LcdGmn113[0];
		}else if(gnum == 613){
			gmnbuf = &LcdGmn308[0];
		}else{
			zerof = 1;
		}
	}else if( gnum < 640 ){
		if(gnum == 620){
			zerof = 1;
		}else if(gnum == 621){
			if( g_PasswordOpen.kigou_inp == FLG_OFF )
				gmnbuf = &LcdGmn006[0];
			else
				gmnbuf = &LcdGmn008[0];
		}else if(gnum == 622){
			zerof = 1;
		}else if(gnum == 623){
			gmnbuf = &LcdGmn205[0];
		}else if(gnum == 624){
			gmnbuf = &LcdGmn206[0];
		}else if(gnum == 625){
			gmnbuf = &LcdGmn307[0];
		}else{
			zerof = 1;
		}
	}

	if(zerof == 1){
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = 0x00;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else if(zerof == 2){
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = 0x6767;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else{
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = *(gmnbuf + xx);
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}

	return((UH)rtn);
}

static UH GetPixDataFromGamenAdd(int gnum, unsigned long cnt)
{
	UH val;
	ER rtn;
	unsigned long xx;
	UH val1;
	unsigned short *gmnbuf;

	rtn = 0;

	if(gnum == 1611){
		gmnbuf = &Title_rus[0];			//お留守番
	}else if(gnum == 1612){
		gmnbuf = &MenuBtnData[0][0];	//空ボタン
	}else if(gnum == 1613){
		gmnbuf = &MenuBtnData[3][0];	//戻る
	}else if(gnum == 1262){
		gmnbuf = &OsiraseMes[5][0];	//初期化しますか
	}else if(gnum == 1263){
		gmnbuf = &OsiraseMes[6][0];	//工事用
	}else if(gnum == 1132){
		gmnbuf = &OsiraseMes[2][0];	//中止しますか
	}else if(gnum == 1133){
		gmnbuf = &OsiraseMes[4][0];	//終了しますか
	}else{
		return((UH)rtn);
	}

	for(xx = 0 ; xx < cnt ; xx++ ){
		val1 = *(gmnbuf + xx);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}

	return((UH)rtn);
}


static UH GetPixDataFromGamen200(int gnum, unsigned long cnt)
{
	UH val;
	ER rtn;
	unsigned long xx;
	UH val1;
	unsigned short *gmnbuf;
	char zerof;

	rtn = 0;
	zerof = 0;

	if( gnum < 220 ){	//画面No200～219
		if(gnum == 200){
				zerof = 1;
		}else if(gnum == 201){
				gmnbuf = &LcdGmn001[0];
		}else if(gnum == 202){
				//gmnbuf = &LcdGmn109[0];
				gmnbuf = &LcdGmn115[0];		//20140925Miya password_open
		}else if(gnum == 203){
				//gmnbuf = &LcdGmn201[0];
				gmnbuf = &LcdGmn308[0];
		}else if(gnum == 204){
				//gmnbuf = &LcdGmn205[0];
				gmnbuf = &LcdGmn404[0];
		}else if(gnum == 205){
				gmnbuf = &LcdGmn206[0];
		}else if(gnum == 206){
				gmnbuf = &LcdGmn303[0];
		}else if(gnum == 207){
				gmnbuf = &LcdGmn401[0];
		}else if(gnum == 208){
//				gmnbuf = &LcdGmn402[0];
		}else{
				zerof = 1;
		}
	}else if( gnum < 240 ){	//画面No220～239
		if(gnum == 220){
				zerof = 1;
		}else if(gnum == 221){
				gmnbuf = &LcdGmn116[0];
		}else if(gnum == 222){
				gmnbuf = &LcdGmn401[0];
		}else if(gnum == 223){
				gmnbuf = &LcdGmn404[0];
		}else if(gnum == 224){
				gmnbuf = &LcdGmn404[0];
		}else if(gnum == 225){
				gmnbuf = &LcdGmn404[0];
		}else{
				zerof = 1;
		}
	}else if( gnum < 260 ){	//画面No240～259
		if(gnum == 240){
				zerof = 1;
		}else if(gnum == 241){
				gmnbuf = &LcdGmn117[0];
		}else if(gnum == 242){
				gmnbuf = &LcdGmn407[0];
		}else if(gnum == 243){
				gmnbuf = &LcdGmn001[0];
		}else if(gnum == 244){
				gmnbuf = &LcdGmn001[0];
		}else if(gnum == 245){
				gmnbuf = &LcdGmn001[0];
		}else if(gnum == 246){
				//gmnbuf = &LcdGmn001[0];
				//zerof = 2;	//20161031Miya Ver2204 LCDADJ
				gmnbuf = &LcdGmn404[0];	//20161031Miya Ver2204 LCDADJ
		}else if(gnum == 247){
				gmnbuf = &LcdGmn001[0];
		}else if(gnum == 248){
				//gmnbuf = &LcdGmn404[0];
				gmnbuf = &LcdGmn307[0];	//20141120Miya
		}else if(gnum == 249){
				gmnbuf = &LcdGmn308[0];
		}else{
				zerof = 1;
		}
	}else if( gnum < 280 ){	//画面No260～279
		if(gnum == 260){
				zerof = 1;
		}else if(gnum == 261){
				gmnbuf = &LcdGmn118[0];
		}else if(gnum == 262){
				gmnbuf = &LcdGmn303[0];
		}else if(gnum == 263){
				gmnbuf = &LcdGmn408[0];
		}else if(gnum == 264){
				gmnbuf = &LcdGmn404[0];
		}else if(gnum == 265){
				gmnbuf = &LcdGmn308[0];
		}else if(gnum == 266){
				gmnbuf = &LcdGmn404[0];
		}else if(gnum == 267){
				gmnbuf = &LcdGmn404[0];
		}else{
				zerof = 1;
		}
	}
	
	if(zerof == 1){
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = 0x00;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else if(zerof == 2){
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = 0x6767;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else{
		for(xx = 0 ; xx < cnt ; xx++ ){
			val1 = *(gmnbuf + xx);
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}

	return((UH)rtn);
}

UB LcdProcMain(int buf_num, int gmn_num, UINT *size, UB *msg)
{
	UB	sts_key;
	int i, cnt;
	int posx, posy;
	UINT msg_size;
	UB *buf;
	char sv_dat, pass_num;
	int d100, d10, d1, dint;

	buf = msg;
	LcdcBackLightOn();	//20160711Miya LCDバックライトON(予備)　画面真っ暗対応(仮)

	//20160120Miya
/*
	if(gmn_num == 1 && g_TechMenuData.SysSpec == 2){
			*(buf + 0) = ( UB )LCD_USER_ID;
			*(buf + 1) = '9';
			*(buf + 2) = '9';
			*(buf + 3) = '9';
			*(buf + 4) = '9';
			*(buf + 5) = 0;
			*size = 6;
			return(1);
	}
*/

	//////// 画面表示 ////////
	//if(gmn_num == 1 || gmn_num == 100 || gmn_num == 120 || gmn_num == 140 || gmn_num == 200 ){
	if( gmn_num == 100 || gmn_num == 120 || gmn_num == 140 || gmn_num == 620 || gmn_num == 200 || gmn_num == 500 || gmn_num == 520 || gmn_num == 999){	//黒画像
		nop();
	}else{
		//if(gmn_num == 3 || gmn_num == 124 || gmn_num == 144 ){	//責任者、一般者選択
		if(gmn_num == 3 || gmn_num == 124 || gmn_num == 144 ||gmn_num == 188 ){	//20160108Miya FinKeyS //責任者、一般者選択
			SetInfo(buf_num);
		}
		//if(gmn_num == 10 || gmn_num == 104 || gmn_num == 123 || gmn_num == 135 || gmn_num == 143 || gmn_num == 409 || gmn_num == 505 || gmn_num == 525 || gmn_num == 534 || gmn_num == 546){	//認証NG
		//20160108Miya FinKeyS
		if(gmn_num == 10 || gmn_num == 104 || gmn_num == 123 || gmn_num == 135 || gmn_num == 143 || gmn_num == 409 || gmn_num == 604 || gmn_num == 624 || gmn_num == 505 || gmn_num == 525 || gmn_num == 534 || gmn_num == 546){	//認証NG
			if(s_CapResult == CAP_JUDGE_E1){	//エラーメッセージ表示ありの場合
				SetErrMes(buf_num, 1);
			}else{
				SetErrMes(buf_num, 0);
			}
		}
		//if(gmn_num == 4 || gmn_num == 129 || gmn_num == 403){	//ひらがなキ－
		//if(gmn_num == 4 || gmn_num == 129 || gmn_num == 403 || gmn_num == 108 || gmn_num == 195 ){	//20140925Miya password_open //ひらがなキ－
		//20160108Miya FinKeyS
		if(gmn_num == 4 || gmn_num == 129 || gmn_num == 403 || gmn_num == 108 || gmn_num == 185 || gmn_num == 608 || gmn_num == 621 ){	//20140925Miya password_open //ひらがなキ－
			for(i=0;i< 8;i++){
				SetKeyMoji(buf_num, MOJICODEDEL, 0, i+1, 0);
			}
		}
		
		//if(gmn_num == 105){	//20140423Miya 認証リトライ回数
		if(gmn_num == 105 || gmn_num == 605){	//20160108Miya FinKeyS
			SetKeyNum(buf_num, 10, g_AuthCnt);
		}

//for debug 登録No表示(FARチェック)
//#if ( NEWALGO > 0 )
#if ( 1 )
		//if((gmn_num == 103 || gmn_num == 104 || gmn_num == 105) && g_TechMenuData.DebugHyouji == FLG_ON){	//20140423Miya 認証リトライ回数
		if((gmn_num == 103 || gmn_num == 104 || gmn_num == 105 || gmn_num == 603 || gmn_num == 604 || gmn_num == 605) && g_TechMenuData.DebugHyouji == FLG_ON){	//20160108Miya FinKeyS
			if(gmn_num == 603 && g_AuthType == 1){	//20160120Miya
				if( g_PasswordOpen2.num+1 >= 10 ){
					if(g_PasswordOpen2.num+1 >= 20){
						SetKeyNum(buf_num, 11, g_PasswordOpen2.num+1-20);
						SetKeyNum(buf_num, 10, 2);	//10位
					}else{
						SetKeyNum(buf_num, 11, g_PasswordOpen2.num+1-10);
						SetKeyNum(buf_num, 10, 1); //10位
					}
				}else{
					SetKeyNum(buf_num, 10, 0);
					SetKeyNum(buf_num, 11, g_PasswordOpen2.num+1);
				}
			}else{
				if( g_RegUserInfoData.RegNum+1 >= 10 ){
					if(g_RegUserInfoData.RegNum+1 >= 20){
						SetKeyNum(buf_num, 11, g_RegUserInfoData.RegNum+1-20);
						SetKeyNum(buf_num, 10, 2);	//10位
					}else{
						SetKeyNum(buf_num, 11, g_RegUserInfoData.RegNum+1-10);
						SetKeyNum(buf_num, 10, 1); //10位
					}
				}else{
					SetKeyNum(buf_num, 10, 0);
					SetKeyNum(buf_num, 11, g_RegUserInfoData.RegNum+1);
				}
			}

			d100 = g_PosGenten[0].scr1 / 100;
			d10  = (g_PosGenten[0].scr1 - 100 * d100) / 10;
			d1   = g_PosGenten[0].scr1 - 100 * d100 - 10 * d10;
			SetKeyNum(buf_num, 12, d100);	//100位
			SetKeyNum(buf_num, 13, d10);	//10位
			SetKeyNum(buf_num, 14, d1);		//1位
			d100 = g_PosGenten[0].scr2 / 100;
			d10  = (g_PosGenten[0].scr2 - 100 * d100) / 10;
			d1   = g_PosGenten[0].scr2 - 100 * d100 - 10 * d10;
			SetKeyNum(buf_num, 15, d100);	//100位
			SetKeyNum(buf_num, 16, d10);	//10位
			SetKeyNum(buf_num, 17, d1);		//1位

			SetKeyNum(buf_num, 20, 0);
			SetKeyNum(buf_num, 21, g_PosGenten[0].low_f);
			dint = (int)(100.0 * g_PosGenten[0].gen1 + 0.5);
			d100 = dint / 100;
			d10  = (dint - 100 * d100) / 10;
			d1   = dint - 100 * d100 - 10 * d10;
			SetKeyNum(buf_num, 22, d100);	//100位
			SetKeyNum(buf_num, 23, d10);	//10位
			SetKeyNum(buf_num, 24, d1);		//1位
			dint = (int)(100.0 * g_PosGenten[0].gen2 + 0.5);
			d100 = dint / 100;
			d10  = (dint - 100 * d100) / 10;
			d1   = dint - 100 * d100 - 10 * d10;
			SetKeyNum(buf_num, 25, d100);	//100位
			SetKeyNum(buf_num, 26, d10);	//10位
			SetKeyNum(buf_num, 27, d1);		//1位
		}

#else
		if(gmn_num == 103 && g_TechMenuData.DebugHyouji == FLG_ON){	//20140423Miya 認証リトライ回数
			if( g_RegUserInfoData.RegNum+1 >= 10 ){
				if(g_RegUserInfoData.RegNum+1 >= 20){
					SetKeyNum(buf_num, 11, g_RegUserInfoData.RegNum+1-20);
					SetKeyNum(buf_num, 10, 2);	//10位
				}else{
					SetKeyNum(buf_num, 11, g_RegUserInfoData.RegNum+1-10);
					SetKeyNum(buf_num, 10, 1); //10位
				}
			}else{
				SetKeyNum(buf_num, 10, 0);
				SetKeyNum(buf_num, 11, g_RegUserInfoData.RegNum+1);
			}
			if( g_RegUserInfoData.lbp_lvl >= 10 ){
				SetKeyNum(buf_num, 13, 1); //10位
				SetKeyNum(buf_num, 14, g_RegUserInfoData.lbp_lvl-10);
			}else{
				SetKeyNum(buf_num, 13, 0);
				SetKeyNum(buf_num, 14, g_RegUserInfoData.lbp_lvl);
			}
			if(g_RegUserInfoData.lbp_pls == 1){
				SetKeyNum(buf_num, 16, 1);
			}else{
				SetKeyNum(buf_num, 16, 0);
			}
		}
#endif

		//if( gmn_num == 108 ){
		if( gmn_num == 108 || gmn_num == 608 || gmn_num == 621 ){	//20160108Miya FinKeyS
			pass_key_hyouji_flg = 0;
			if( g_PasswordOpen.random_key == FLG_ON )
				SetPassKeyBtn(buf_num, 0, 1);
			else
				SetPassKeyBtn(buf_num, 0, 0);
		}

		//if(gmn_num == 109){	//20140925Miya add mainte
		if(gmn_num == 109 || gmn_num == 609 || gmn_num == 625){	//20160108Miya FinKeyS
			SetKeyNumPara(buf_num, 0);	// エラー表示
			++g_MainteLog.err_rcnt;
			g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;
		}

		if( gmn_num == 185 ){	//20140925Miya password_open 
			//if(g_TechMenuData.SysSpec == 2){	//20160108Miya FinKeyS
			if(g_PasswordOpen2.family_sw == 1){	//20160120Miya FinKeyS
				//g_PasswordOpen2.family_sw = 1;
				g_PasswordOpen.keta = g_PasswordOpen2.keta[g_PasswordOpen2.num];
				for(i = 0 ; i < 10 ; i++){
					g_PasswordOpen.password[i] = g_PasswordOpen2.password[g_PasswordOpen2.num][i];
				}
			}
		
			if(g_PasswordOpen.keta >= 4 && g_PasswordOpen.keta <= 8){
				for(i=0 ; i< g_PasswordOpen.keta ; i++){
					if( g_PasswordOpen.password[i] >= 10){	//記号
						pass_num = g_PasswordOpen.password[i] - 10;
						pass_key_hyouji_flg = 1;
					}else{
						pass_num = g_PasswordOpen.password[i];
						pass_key_hyouji_flg = 0;
					}
					SetKeyNumPassWord(gmn_num, buf_num, i+1, pass_num);
				}
				//g_PasswordOpen.hide_num = sv_dat;
				pass_key_hyouji_flg = 0;
				SetPassKeyBtn(buf_num, 0, 0);
			}else{	//パスワード未入力
				pass_key_hyouji_flg = 0;
				SetPassKeyBtn(buf_num, 0, 0);
			}
		}			

		if(gmn_num == 204){	//20140925Miya add mainte
			SetKeyNumPara(buf_num, 2);	// 診断結果表示
		}

		if(gmn_num == 223){	//20140925Miya add mainte
			SetKeyNumPara(buf_num, 1);	// エラー履歴表示
		}

		if(gmn_num == 224){	//20140925Miya add mainte
			SetKeyNumPara(buf_num, 3);	// 認証パラメータ表示
		}

		if(gmn_num == 225){	//20140925Miya add mainte
			SetKeyNumPara(buf_num, 4);	// 時刻表示
		}

		if(gmn_num == 264){	//20140925Miya add mainte
#if (NEWCMR == 0)	//20160601Miya
			if( g_TechMenuData.DebugHyouji == FLG_ON ){
				//SetGmnToLcdBuffInImage(buf_num, 0);
				if(dbg_Auth_hcnt < g_RegUserInfoData.TotalNum ){
					SetGmnToLcdBuffInImage(buf_num, 11);
					SetGmnToLcdBuffInImage(buf_num, 12);
					//SetGmnToLcdBuffInImage(buf_num, 13);
					//SetGmnToLcdBuffInImage(buf_num, 14);
					//SetGmnToLcdBuffInImage(buf_num, 15);
					//SetGmnToLcdBuffInImage(buf_num, 16);
					SetKeyNumPara(buf_num, 6);
				}
			}else{

#if(AUTHTEST >= 1)	//20160613Miya
				SetGmnToLcdBuffInImage(buf_num, 4);
#else
				SetGmnToLcdBuffInImage(buf_num, 1);
				SetGmnToLcdBuffInImage(buf_num, 2);
				SetGmnToLcdBuffInImage(buf_num, 3);
				SetKeyNumPara(buf_num, 5);
#endif
			}

			if( g_RegUserInfoData.trim_sty >= 100 ){
				SetKeyNum(buf_num, 10, 0);
				SetKeyNum(buf_num, 11, 0);
			}else{
				cnt = g_RegUserInfoData.trim_sty / 10;
				SetKeyNum(buf_num, 10, cnt);
				cnt = g_RegUserInfoData.trim_sty % 10;
				SetKeyNum(buf_num, 11, cnt);
			}
#else
			//CmrResize(0, 0, 0, 640, 480, 1000 );//20131210Miya cng	// １枚目画像の縮小・圧縮
			//dly_tsk( 1000/MSEC );
			//CmrResizeGet(0, 0, ( 320 * 240 ), &g_ubCapBuf[ 0 ]);//20131210Miya cng		// １枚目縮小画像の取得
			//dly_tsk( 1000/MSEC );
			//SetGmnToLcdBuffInImage(buf_num, 2);
			if( g_TechMenuData.DebugHyouji == FLG_ON ){
				//SetGmnToLcdBuffInImage(buf_num, 0);
				if(dbg_Auth_hcnt < g_RegUserInfoData.TotalNum ){
					SetGmnToLcdBuffInImage(buf_num, 11);
					SetGmnToLcdBuffInImage(buf_num, 12);
					SetKeyNumPara(buf_num, 6);
				}
			}else{
				//SetGmnToLcdBuffInImage(buf_num, 21);
				//SetGmnToLcdBuffInImage(buf_num, 22);
				//SetGmnToLcdBuffInImage(buf_num, 23);
				//SetGmnToLcdBuffInImage(buf_num, 24);
				SetGmnToLcdBuffInImage(buf_num, 2);
			}
#endif			
		}

		if(gmn_num == 267){	//20140925Miya add mainte
#if (NEWCMR == 0)	//20160601Miya
			SetGmnToLcdBuffInImage(buf_num, 1);
			SetGmnToLcdBuffInImage(buf_num, 2);
			SetGmnToLcdBuffInImage(buf_num, 3);

			if( g_RegUserInfoData.trim_sty >= 100 ){
				SetKeyNum(buf_num, 10, 0);
				SetKeyNum(buf_num, 11, 0);
			}else{
				cnt = g_RegUserInfoData.trim_sty / 10;
				SetKeyNum(buf_num, 10, cnt);
				cnt = g_RegUserInfoData.trim_sty % 10;
				SetKeyNum(buf_num, 11, cnt);
			}
#else
			//CmrResize(0, 0, 0, 400, 160, 1000 );//20131210Miya cng	// １枚目画像の縮小・圧縮
			//dly_tsk( 1000/MSEC );
			//CmrResizeGet(0, 0, ( 200 * 80 ), &g_ubCapBuf[ 0 ]);//20131210Miya cng		// １枚目縮小画像の取得
			//dly_tsk( 1000/MSEC );
			SetGmnToLcdBuffInImage(buf_num, 1);
#endif			
		}

		if(gmn_num == 248){//20141120Miya
			SetErrMes(buf_num, 2);
		}

		LcdcDisplayModeSet(buf_num, 0);
	}

	///////// キー入力待ち ////////
	sts_key = TouchKeyProc(buf_num, gmn_num, &msg_size, buf);
	*size = msg_size;
	return(sts_key);
}


UB TouchKeyProc(int buf_num, int num, UINT *msize, UB *msg)
{
	UB	rtn=0;
	UB	rtnsts=0;
	UB	lcd_sw=LCDBACKLIGHTON;
	UB	proc;	//0:なし 1:複数ボタン選択(中止あり) 2:YES/NOボタン 3:複数ボタン選択(中止あり) 4:キーボード入力 5:タイマー待ち 6:強制キーあり
	ER	ercd;
	unsigned short pos[40][2];
	int x1[20], x2[20], y1[20], y2[20];
	int posx, posy;
	int i, j, btn_num, btn_press, dat, unused_btn, tm_proc;
	int cnt;
	int tcnt=0, tcntTH;
	int page, keta, hit;
	int sel1, sel2, sel3, sel4, sel5, sel6;
	int mainte_touch_cnt=0;
	UB	errsts=0;
	UB *buf;
	double tm;
	int key_read;

	buf = msg;
	memset( x1, 0, sizeof(int) * 16 );
	memset( x2, 0, sizeof(int) * 16 );
	memset( y1, 0, sizeof(int) * 16 );
	memset( y2, 0, sizeof(int) * 16 );
	btn_num = 0;
	btn_press = 0;
	unused_btn = 0;
	page = 0;
	tm_proc = 0;
	sel1 = sel2 = 0;
	LcdcBackLightOn();	//20160711Miya LCDバックライトON(予備)　画面真っ暗対応(仮)

	switch(num)
	{
		case 1:
		case 164:
		case 172:
		case 174:
		case 175:
		case 201:
		case 243:
		case 244:
		case 245:
		//case 246:	//20161031Miya Ver2204 LCDADJ
		case 247:
		case 402:
		case 522:
		case 527:
		case 543:
		case 547:
			proc = GMN_KEYSOUSA;
			btn_num = 16;
			keta = 1;
			memcpy( &pos[0][0], &BtnPosType6[0][0], sizeof(short) * btn_num * 4 );
			memset( &sv_keyindat[0], 0x20, 8);
			break;
		case 246:	//20161031Miya Ver2204 LCDADJ
			proc = GMN_KEYSOUSA;
			btn_num = 1;
			keta = 1;
			pos[0][0] = 0;
			pos[0][1] = 0;
			pos[1][0] = 480;
			pos[1][1] = 272;
			//memcpy( &pos[0][0], &BtnPosType6[0][0], sizeof(short) * btn_num * 4 );
			memset( &sv_keyindat[0], 0x20, 8);
			g_lcd_adj_f = 1;
			g_lcd_adj_cnt = 0;
			break;
		case 4:
		case 129:
		case 403:
		case 528:
			proc = GMN_KEYSOUSA;
			btn_num = 16;
			keta = 1;
			hit = 0;
			sv_mkey = 0xff;
			//memcpy( &pos[0][0], &BtnPosType6[0][0], sizeof(short) * btn_num * 4 );
			memcpy( &pos[0][0], &BtnPosType6kana[0][0], sizeof(short) * btn_num * 4 );
			memset( &sv_keyindat[0], 0, 8);
			break;
//		case 129:
//			proc = GMN_DEFSOUSA;
//			break;
		case 2:
		case 401:
			proc = GMN_MENUSOUSA;
			//btn_num = 2;
			btn_num = 1;	//メンテボタン無効
			memcpy( &pos[0][0], &BtnPosType1[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 3:
//20130610_Miya 画像データ採取実施フラグ
//#ifdef GETIMG20130611
//			proc = GMN_DEFSOUSA;
//			btn_press = 1;
//#else
			proc = GMN_SELSOUSA;
			//btn_num = 7;
			btn_num = 5;	//←→ボタン無効
			memcpy( &pos[0][0], &BtnPosType3[0][0], sizeof(short) * btn_num * 4 );
//#endif
			break;
		case 5:
		case 130:
		case 404:
		case 529:
			proc = GMN_SELSOUSA;
			btn_num = 7;
			memcpy( &pos[0][0], &BtnPosType4[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 9:
		case 134:
		case 408:
		case 533:
			BuzTplSet(0x87, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();
			proc = GMN_WAITSOUSA;
			break;
		case 10:
		case 135:
		case 409:
		case 534:
			BuzTplSet(0xB0, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();
			if(s_CapResult == CAP_JUDGE_E1){	//エラーメッセージ表示ありの場合
				dly_tsk((2500/MSEC));	//タイマー2.5sec(時間追加)
				//エラーメッセージ表示クリアー
			}else{
				//20140211Miya カメラ再設定用にインターバル
				dly_tsk((1500/MSEC));	//タイマー1.5sec(時間追加)
			}				
			proc = GMN_WAITSOUSA;
			break;
		case 103:
		case 122:
		case 142:
		case 162:
		case 182:	//20140925Miya password_open
		case 176:
		case 603:	//20160108Miya FinKeyS
		case 623:	//20160108Miya FinKeyS
		//case 204:
		case 504:
		case 524:
		case 545:
			BuzTplSet(0x87, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();

			//20140211Miya カメラ再設定用にインターバル
			dly_tsk((1500/MSEC));	//タイマー1.5sec(時間追加)

			proc = GMN_WAITSOUSA;
			break;
		case 104:
		case 123:
		case 143:
		case 163:
		case 183:	//20140925Miya password_open
		case 177:
		case 205:
		case 604:	//20160108Miya FinKeyS
		case 624:	//20160108Miya FinKeyS
		case 505:
		case 525:
		case 546:
			BuzTplSet(0xB0, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();
			if(s_CapResult == CAP_JUDGE_E1){	//エラーメッセージ表示ありの場合
				dly_tsk((2500/MSEC));	//タイマー2.5sec(時間追加)
				//エラーメッセージ表示クリアー
			}else{
				//20140211Miya カメラ再設定用にインターバル
				dly_tsk((1500/MSEC));	//タイマー1.5sec(時間追加)
			}				
			proc = GMN_WAITSOUSA;
			break;
		case 11:
		case 12:
		case 136:
		case 149:
		case 150:
		case 165:
		case 178:
		case 187:	//20140925Miya password open
		case 206:
		case 410:
		case 411:
		case 538:
		case 548:
		case 551:
			proc = GMN_YNSOUSA;
			btn_num = 2;
			memcpy( &pos[0][0], &BtnPosType5[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 262:	//20160112Miya FinKeyS
			proc = GMN_YNSOUSA;
			btn_num = 3;
			memcpy( &pos[0][0], &BtnPosType5[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 100:
		case 110:	//20141005Miya jikosindan
		case 120:
		case 140:
		case 160:
		case 170:
		case 180:	//20140925Miya password open
		case 620:	//20160112Miya FinKeyS
		case 200:
		case 220:	//20140925Miya add mainte
		case 240:	//20140925Miya add mainte
		case 249:	//20140925Miya add mainte
		case 260:	//20140925Miya add mainte
		case 265:	//20140925Miya add mainte
		case 613:	//20160108Miya FinKeyS
		case 500:
		case 520:
			proc = GMN_DEFSOUSA;
			break;
		case 101:
		case 601:	//20160108Miya FinKeyS
			proc = GMN_MENUSOUSA;
			lcd_sw = LCDBACKLIGHTON;
			btn_num = 5;
			//btn_num = 2;	//緊急番号設定、緊急解錠ボタン無効
			memcpy( &pos[0][0], &BtnPosType2[0][0], sizeof(short) * btn_num * 4 );
			tm_proc = 1;
			dbg_cap_flg = 1;
			mainte_touch_cnt = 0;
			break;
		case 102:
		case 602:	//20160108Miya FinKeyS
			proc = GMN_MENUSOUSA;
			lcd_sw = LCDBACKLIGHTOFF;
			btn_num = 5;
			//btn_num = 3;	//緊急番号設定、緊急解錠ボタン無効
			memcpy( &pos[0][0], &BtnPosType2[0][0], sizeof(short) * btn_num * 4 );
			tm_proc = 1;
			dbg_cap_flg = 1;
			break;
		case 124:
		case 144:
		case 188:	//20160108Miya FinKeyS
			proc = GMN_SELSOUSA;
			page = 3;
			btn_num = 7;
			//btn_num = 6;	//←ボタン無効
			memcpy( &pos[0][0], &BtnPosType3[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 125:
		case 126:
		case 127:
		case 128:
		case 145:
		case 146:
		case 147:
		case 148:
		case 189:	//20160108Miya FinKeyS
		case 190:	//20160108Miya FinKeyS
		case 191:	//20160108Miya FinKeyS
		case 192:	//20160108Miya FinKeyS
			proc = GMN_SELSOUSA;
			btn_num = 7;
			memcpy( &pos[0][0], &BtnPosType3[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 171:
		case 173:
			proc = GMN_SELSOUSA;
			btn_num = 2;
			memcpy( &pos[0][0], &BtnPosType7[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 610:	//20160108Miya FinKeyS
		case 611:	//20160108Miya FinKeyS
			proc = GMN_SELSOUSA;
			btn_num = 2;
			memcpy( &pos[0][0], &BtnPosType12[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 202:
		case 612:	//20160108Miya FinKeyS
			proc = GMN_MENUSOUSA;
			btn_num = 5;
			unused_btn = 0;
			memcpy( &pos[0][0], &BtnPosType2[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 208:
//			proc = GMN_SELSOUSA;
//			btn_num = 7;
//			memcpy( &pos[0][0], &BtnPosType9[0][0], sizeof(short) * btn_num * 4 );

//			if( GetSysSpec() == SYS_SPEC_MANTION ){
//				sel1 = SYS_SPEC_MANTION;
//			}else if( GetSysSpec() == SYS_SPEC_OFFICE ){
//				sel1 = SYS_SPEC_OFFICE;
//			}else{
//				sel1 = SYS_SPEC_ENTRANCE;
//			}
//			if( sys_demo_flg == SYS_SPEC_NOMAL ){
//				sel2 = SYS_SPEC_NOMAL;
//			}else{
//				sel2 = SYS_SPEC_DEMO;
//			}

			break;
		case 501:
			proc = GMN_KEYSOUSA;
			btn_num = 16;
			keta = 1;
			lcd_sw = LCDBACKLIGHTON;
			memcpy( &pos[0][0], &BtnPosType6[0][0], sizeof(short) * btn_num * 4 );
			memset( &sv_keyindat[0], 0x20, 8);
			break;
		case 108:	//20140925Miya password_open
		case 608:	//20160108Miya FinKeyS
		case 621:	//20160108Miya FinKeyS
			proc = GMN_KEYSOUSA;
			btn_num = 16;
			keta = 1;
			tm_proc = 1;
			dbg_cap_flg = 1;
			lcd_sw = LCDBACKLIGHTON;
			if( g_PasswordOpen.kigou_inp == FLG_OFF ){
				memcpy( &pos[0][0], &BtnPosType6[0][0], sizeof(short) * btn_num * 4 );
			}else{
				memcpy( &pos[0][0], &BtnPosType6Pass2[0][0], sizeof(short) * btn_num * 4 );
			}
			memset( &sv_keyindat[0], 0x20, 8);
			break;
		case 185:	//20140925Miya password_open
			proc = GMN_KEYSOUSA;
			btn_num = 16;
			memset( &sv_keyindat[0], 0x20, 8);
			if( g_PasswordOpen.keta >= 4 && g_PasswordOpen.keta <= 8 ){
				keta = g_PasswordOpen.keta;
				for( i = 0 ; i < keta ; i++ ){
					sv_keyindat[i] = g_PasswordOpen.password[i] + 0x30;
				}
				keta += 1;
			}else{
				keta = 1;
			}
			lcd_sw = LCDBACKLIGHTON;
			if( g_PasswordOpen.kigou_inp == FLG_OFF ){
				memcpy( &pos[0][0], &BtnPosType6[0][0], sizeof(short) * btn_num * 4 );
			}else{
				memcpy( &pos[0][0], &BtnPosType6Pass2[0][0], sizeof(short) * btn_num * 4 );
			}
			break;
		case 502:
			proc = GMN_KEYSOUSA;
			btn_num = 16;
			keta = 1;
			lcd_sw = LCDBACKLIGHTOFF;
			memcpy( &pos[0][0], &BtnPosType6[0][0], sizeof(short) * btn_num * 4 );
			memset( &sv_keyindat[0], 0x20, 8);
			break;
		case 109:	//エラー表示 //20140925Miya add err
		case 204:	//診断結果表示画面	//20140925Miya add mainte
		case 207:
		case 222:	//バージョン表示画面	//20140925Miya add mainte
		case 223:	//エラー履歴表示画面	//20140925Miya add mainte
		case 224:	//認証状況表示画面		//20140925Miya add mainte
		case 225:	//時刻状況表示画面		//20140925Miya add mainte
		case 248:	//LCD位置調整画面		//20140925Miya add mainte
		case 264:	//画像確認画面		//20140925Miya add mainte
		//case 266:	//テスト撮影画面		//20140930Miya add mainte
		case 267:	//テスト撮影画面		//20140930Miya add mainte
		case 609:	//20160108Miya FinKeyS
		case 625:	//20160108Miya FinKeyS
		case 506:
		case 535:
		case 536:
		case 537:
		case 549:
		case 550:
			proc = GMN_OKSOUSA;
			btn_num = 1;
			memcpy( &pos[0][0], &BtnPosType8[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 266:	//テスト撮影画面		//20140930Miya add mainte
			proc = GMN_OKSOUSA;
			btn_num = 2;
			memcpy( &pos[0][0], &BtnPosType11[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 521:
		case 526:
		//case 542:
			proc = GMN_MENUSOUSA;
			btn_num = 5;
			unused_btn = 1;
			memcpy( &pos[0][0], &BtnPosType2[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 7:
		case 132:
		case 406:
		case 531:
			BuzTplSet(0x87, 1);
			dly_tsk((50/MSEC));	//50msecのウエイト
			BuzTplOff();
			proc = GMN_NOSOUSA;
			rtn = 0;
			return(rtn);
			//break;	
		case 105:
		case 605:	//20160108Miya FinKeyS
			BuzTplSet(0x87, 1);
			dly_tsk((30/MSEC));	//30msecのウエイト
			BuzTplOff();
			dly_tsk((50/MSEC));	//30msecのウエイト
			BuzTplSet(0x87, 1);
			dly_tsk((30/MSEC));	//30msecのウエイト
			BuzTplOff();
			proc = GMN_NOSOUSA;
			rtn = 0;
			return(rtn);
		case 184:	//パスワード開錠設定モード画面 //20140925Miya password_open
		case 221:	//メンテナンス情報モード画面 //20140925Miya add mainte
		case 241:	//メンテナンス情報モード画面 //20140925Miya add mainte
		case 261:	//メンテナンス情報モード画面 //20140925Miya add mainte
			proc = GMN_MENUSOUSA;
			btn_num = 5;
			unused_btn = 0;
			memcpy( &pos[0][0], &BtnPosType2[0][0], sizeof(short) * btn_num * 4 );
			break;
		case 186:
			proc = GMN_SELSOUSA;
			if(g_TechMenuData.SysSpec == 2 || GetSysSpec() == SYS_SPEC_KOUJIM || GetSysSpec() == SYS_SPEC_KOUJIO || GetSysSpec() == SYS_SPEC_KOUJIS)	//20160112Miya FinKeyS
				btn_num = 8;
			else
				btn_num = 10;
			memcpy( &pos[0][0], &BtnPosType10[0][0], sizeof(short) * btn_num * 4 );

			if( g_PasswordOpen.hide_num == FLG_ON )		sel1 = SELON;
			else										sel1 = SELOFF;

			if( g_PasswordOpen.kigou_inp == FLG_ON )	sel2 = SELON;
			else										sel2 = SELOFF;

			if( g_PasswordOpen.random_key == FLG_ON )	sel3 = SELON;
			else										sel3 = SELOFF;

			if( g_PasswordOpen2.family_sw == FLG_ON )	sel4 = SELON;		//20160112Miya FinKeyS
			else										sel4 = SELOFF;

			break;
		case 242:
			proc = GMN_SELSOUSA;
			btn_num = 12;
			memcpy( &pos[0][0], &BtnPosType10[0][0], sizeof(short) * btn_num * 4 );

			if( g_PasswordOpen.hide_num == FLG_ON )		sel1 = SELON;
			else										sel1 = SELOFF;

			if( g_PasswordOpen.kigou_inp == FLG_ON )	sel2 = SELON;
			else										sel2 = SELOFF;

			if( g_PasswordOpen.random_key == FLG_ON )	sel3 = SELON;
			else										sel3 = SELOFF;

			if( g_PasswordOpen2.family_sw == FLG_ON )	sel4 = SELON;
			else										sel4 = SELOFF;

			if( g_PasswordOpen.sw == FLG_ON )			sel5 = SELON;
			else										sel5 = SELOFF;

			break;
		case 263:
			proc = GMN_SELSOUSA;
			//btn_num = 11;
			btn_num = 16;
			memcpy( &pos[0][0], &BtnPosType9[0][0], sizeof(short) * btn_num * 4 );

			//if( g_TechMenuData.SysSpec == FLG_ON )			sel1 = SELON;
			//else											sel1 = SELOFF;

			if( g_TechMenuData.SysSpec == 2 ) 		sel1 = SEL_2;	//スマート制御盤
			else if( g_TechMenuData.SysSpec == 1 )	sel1 = SEL_1;	//オフィス・共用部
			else												sel1 = SEL_0;	//占有部

			if( g_TechMenuData.DemoSw == FLG_ON )			sel2 = SELON;
			else											sel2 = SELOFF;

			if( g_TechMenuData.HijyouRemocon == FLG_ON )	sel3 = SELON;
			else											sel3 = SELOFF;

			if( g_TechMenuData.DebugHyouji == FLG_ON )		sel4 = SELON;
			else											sel4 = SELOFF;

			//20161031Miya Ver2204 ->
			sel5 = SEL_0;
			
			if(g_BkDataNoClear.LedPosi == 0)		sel6 = SEL_0;
			else if(g_BkDataNoClear.LedPosi == 1)	sel6 = SEL_1;
			else									sel6 = SEL_2;
			//20161031Miya Ver2204 <-

			break;
		case 203:
			cnt = 0;
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			rtn = 1;
			return(rtn);
			//break;
		default:
			proc = GMN_NOSOUSA;
			rtn = 0;
			return(rtn);
			//break;	
	}

	if( proc == GMN_WAITSOUSA || proc == GMN_DEFSOUSA ){
		if( proc == GMN_WAITSOUSA ){
			rtn = 1;
			if( num == 122 || num == 142 ){//登録時の画面バッファ転送時間対策(画面122の場合ウエイトしない)
				nop();
			}else{
				dly_tsk((2500/MSEC));	//タイマー2.5sec
			}
		}else {	//GMN_DEFSOUSA
			rtn = 1;
		}
	}else{
		key_read = 0;
		while(1){
			if(tm_proc == 1){
				//ercd = TplPosGet(&posx, &posy, (500/MSEC));	//キー待ち(500msec)
				ercd = TplPosGet(&posx, &posy, (200/MSEC));	//20160711Miya撮影高速化に伴い　//キー待ち(200msec)
				ercd = TplPosGet(&posx, &posy, (100/MSEC));	//20160711Miya撮影高速化に伴い　//キー待ち(200msec)
				
#if(PCCTRL == 1)
				if(g_capallow == 0){
					tcntTH = 10;
				}else{
					tm = 1000.0 * (double)g_pc_authtime / 200.0;
					tcntTH = (int)tm;
				}
#else
				tcntTH = 200;
#endif
				if(tcnt < tcntTH ){	//60sec判定(60sec以内の場合)
					++tcnt;
				}else{
					if(lcd_sw == LCDBACKLIGHTON){
						LcdcBackLightOff();
							LcdcDisplayModeSet(2, 0);
						lcd_sw = LCDBACKLIGHTOFF;
						ercd = E_TMOUT;		//キー操作に反応しないようにするため
						mainte_touch_cnt = 0;
#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
						if(g_capallow == 1){
							g_capallow = 0;
							send_sio_AUTHPROC(1);
						}
#endif
					}
					if(	ercd == E_OK ){
						BuzTplSet(0x87, 1);
						dly_tsk((50/MSEC));	//50msecのウエイト(タッチキーの反応解除用)
						BuzTplOff();
						dly_tsk((250/MSEC));	//250msecのウエイト(タッチキーの反応解除用)
						//if( num == 108 )
						if( num == 108 || num == 608 )	//20160108Miya FinKeyS
							LcdcDisplayModeSet(8, 0);
						else
							LcdcDisplayModeSet(1, 0);
						LcdcBackLightOn();
						lcd_sw = LCDBACKLIGHTON;
						tcnt = 0;
						ercd = E_TMOUT;		//キー操作に反応しないようにするため
					}
				}
			}else{
				//if(num == 109){
				if(num == 109 || num == 609 || num == 625){	//2160108Miya FinKeyS
					ercd = TplPosGet(&posx, &posy, (500/MSEC));	//キー待ち(500msec)
				}else{
//					ercd = TplPosGet(&posx, &posy, TMO_FEVR);	//キー待ち(押されるまで)	Modify T.N 2015.3.10
					ercd = TplPosGet( &posx, &posy, (200/MSEC) );	//キー待ち(押されるまで)
					ercd = TplPosGet( &posx, &posy, (100/MSEC) );	//キー待ち(押されるまで)
				}
/*				
				if(ercd == E_TMOUT){
					key_read = 0;
				}
				if(ercd == E_OK){
					key_read++;
					if(key_read >= 2){
						key_read = 0;
					}else{
						ercd = E_TMOUT;
					}
				}
*/
			}
			
			lcd_TSK_wdt = FLG_ON;			// LCDタスク　WDTカウンタリセット・リクエスト・フラグON。 Added T.N 2015.3.10

			if(	ercd == E_OK ){
				//dly_tsk((500/MSEC));	//500msecのウエイト(タッチキーの反応解除用)
				//20161031Miya Ver2204 LCDADJ
				if(num == 246){
					if(posx >= 380 && posx <= 420 && posy >= 99 && posy <= 171){	//確認, 戻る
						btn_press = KEYCAN;
						rtn = 1;
						break;
					}
					g_lcdpos[g_lcd_adj_cnt][0] = posx;
					g_lcdpos[g_lcd_adj_cnt][1] = posy;
					SetKeyNumPara(buf_num, 7);
					if(g_lcd_adj_cnt == 0){
						if(posx >= 40 - 30 && posx <= 40 + 30 && posy >= 40 - 30 && posy <= 40 + 30){
							++g_lcd_adj_cnt;
							SetPartGmnToLcdBuff(buf_num, num, g_lcd_adj_cnt+1, 0, 0, 0, 0, 0, 0, 479, 271);
						}
					}else if(g_lcd_adj_cnt == 1){
						if(posx >= 432 - 30 && posx <= 432 + 30 && posy >= 232 - 30 && posy <= 232 + 30){
							++g_lcd_adj_cnt;
							SetPartGmnToLcdBuff(buf_num, num, g_lcd_adj_cnt+1, 0, 0, 0, 0, 0, 0, 479, 271);
						}
					}else if(g_lcd_adj_cnt == 2){
						if(posx >= 432 - 30 && posx <= 432 + 30 && posy >= 40 - 30 && posy <= 40 + 30){
							++g_lcd_adj_cnt;
							SetPartGmnToLcdBuff(buf_num, num, g_lcd_adj_cnt+1, 0, 0, 0, 0, 0, 0, 479, 271);
						}
					}else{
						if(g_TechMenuData.DebugHyouji == FLG_OFF){
							rtn = 1;
							break;
						}
					}
				}
				for(i = 0, j = 0 ; i < btn_num ; i++, j+=2 ){
					x1[i] = pos[j][0];
					y1[i] = pos[j][1];
					x2[i] = pos[j+1][0];
					y2[i] = pos[j+1][1];
					if( (posx > x1[i] && posx < x2[i]) && (posy > y1[i] && posy < y2[i]) ){
						btn_press = i;
					
						BuzTplSet(0x87, 1);
						dly_tsk((50/MSEC));	//50msecのウエイト(タッチキーの反応解除用)
						BuzTplOff();
						dly_tsk((200/MSEC));	//200msecのウエイト(タッチキーの反応解除用)
						//if( (num >= 124 && num <= 128) || (num >= 144 && num <= 148)){
						if(page > 0){
							rtn = KeyPageSousa( &page, btn_press );
							if( rtn == 1 ){
								break;	//CHG_REQ要求
							}
						}else{
							if( proc == GMN_KEYSOUSA ){
								//if( num == 1 || num == 201 || num == 164 || num == 182 || num == 184 || num == 185
								// || num == 402 || num == 501 || num == 522 || num == 527 || num == 543 || num == 547){	//テンキ－操作の場合
								//20140925Miya password_open
								if( num == 1 || num == 201 || num == 164 || num == 172 || num == 174 || num == 175 
								 || num == 243 || num == 244 || num == 245 || /*num == 246 ||*//*20161031Miya Ver2204 LCDADJ*/ num == 247
								 || num == 402 || num == 501 || num == 522 || num == 527 || num == 543 || num == 547){	//テンキ－操作の場合
									rtn = KeyInputNumSousa( num, buf_num, &keta, btn_press );
									if( btn_press == KEYCAN && num != 501 ){
										rtn = 1;
									}
								}else if( num == 4 || num == 129 || num == 403 || num == 528){	//ひらがなキ－
									rtn = KeyInputMojiSousa( buf_num, &hit, &keta, btn_press );
								//}else if( num == 108 || num == 195 ){	//パスワード開錠キ－ //20140925Miya password open
								//20160108Miya FinKeyS
								}else if( num == 108 || num == 185 || num == 608 || num == 621 ){	//パスワード開錠キ－ //20140925Miya password open
									rtn = KeyInputNumSousaPassKaijyou( num, buf_num, &keta, btn_press );
									if( btn_press == KEYCAN ){
										rtn = 1;
									}
								}		

								if( rtn == 1 ){
									break;	//CHG_REQ要求
								}
							}else if( proc == GMN_MENUSOUSA ){
								if( unused_btn > 0 ){
									if( num == 521 && btn_press != 3 ){//法人メインメニュー
										rtn = 1;	//CHG_REQ要求
									}
									if( num == 526 ){//法人レベル選択メニュー
										if( s_ID_Authority_Level == '0' ){	//監督者
											if( reg_cnt_lvl0 >= 2 || reg_cnt_lvl1 >= 10 ){
												if( reg_cnt_lvl0 >= 2 && reg_cnt_lvl1 >= 10 ){
													if( btn_press == 2 || btn_press == 4 ){
														rtn = 1;
													}
												}else{
													if( reg_cnt_lvl0 >= 2 ){
														if( btn_press == 1 || btn_press == 2 || btn_press == 4 ){
															rtn = 1;
														}
													}else{
														if( btn_press == 0 || btn_press == 2 || btn_press == 4 ){
															rtn = 1;
														}
													}
												}
											}else{
												if( btn_press != 3 ){
													rtn = 1;
												}
											}
										}else if( s_ID_Authority_Level == '1' ){	//管理者
											if( btn_press == 2 || btn_press == 4 ){
												rtn = 1;
											}
										}else{	//一般者
											if( btn_press == 4 ){
												rtn = 1;
											}
										}
									}	
								}else{
#if ( VA300S == 1 || VA300S == 2 )	
									//メンテナンスボタン押下時、回数をかぞえる。
									//if( btn_press == 4 && (num == 101 || num == 102) ){	//メンテ
									if( btn_press == 4 && (num == 101 || num == 102 || num == 601 || num == 602) ){	//20160108Miya FinKeyS
										if( g_PasswordOpen.sw == FLG_OFF ){	//20140925Miya password_open
											++mainte_touch_cnt;
											if(mainte_touch_cnt >= 5){
												rtn = 1;
											}
										}else{
											rtn = 1;
										}
									}else{
										rtn = 1;
									}
									
#else								
									rtn = 1;	//CHG_REQ要求
#endif
								}
								if( rtn == 1 ){
									break;	//CHG_REQ要求
								}
							}else if(proc == GMN_SELSOUSA){
								if(num == 208){
									if(btn_press == 0 || btn_press == 1){
										rtn = 1;
									}else{
										if( btn_press < 5 ){
											if( sel1 == SYS_SPEC_MANTION ){
												SetMainteSelBtn(buf_num, 1, 0, SELOFF, 0);
											}
											if( sel1 == SYS_SPEC_ENTRANCE ){
												SetMainteSelBtn(buf_num, 1, 1, SELOFF, 0);
											}
											if( sel1 == SYS_SPEC_OFFICE ){
												SetMainteSelBtn(buf_num, 1, 2, SELOFF, 0);
											}
											if(btn_press == 2){	sel1 = SYS_SPEC_MANTION; }
											if(btn_press == 3){	sel1 = SYS_SPEC_ENTRANCE; }
											if(btn_press == 4){	sel1 = SYS_SPEC_OFFICE; }
											if( sel1 == SYS_SPEC_MANTION ){
												SetMainteSelBtn(buf_num, 1, 0, SELON, 0);
											}
											if( sel1 == SYS_SPEC_ENTRANCE ){
												SetMainteSelBtn(buf_num, 1, 1, SELON, 0);
											}
											if( sel1 == SYS_SPEC_OFFICE ){
												SetMainteSelBtn(buf_num, 1, 2, SELON, 0);
											}
										}else{
											if( sel2 == SYS_SPEC_DEMO ){
												SetMainteSelBtn(buf_num, 2, 0, SELOFF, 0);
											}
											if( sel2 == SYS_SPEC_NOMAL ){
												SetMainteSelBtn(buf_num, 2, 1, SELOFF, 0);
											}
											if(btn_press == 5){	sel2 = SYS_SPEC_DEMO; }
											if(btn_press == 6){	sel2 = SYS_SPEC_NOMAL; }
											if( sel2 == SYS_SPEC_DEMO ){
												SetMainteSelBtn(buf_num, 2, 0, SELON, 0);
											}
											if( sel2 == SYS_SPEC_NOMAL ){
												SetMainteSelBtn(buf_num, 2, 1, SELON, 0);
											}
										}
										break;
									}
								}else if(num == 242 || num == 186){
									if(btn_press == 0 || btn_press == 1){
										rtn = 1;
									}else{
										if( btn_press == 2 ){
											sel1 = SELON;
											SetMainteSelBtn(buf_num, 1, 1, SELON, 1);
											SetMainteSelBtn(buf_num, 1, 2, SELOFF, 1);
										}	
										if( btn_press == 3 ){
											sel1 = SELOFF;
											SetMainteSelBtn(buf_num, 1, 1, SELOFF, 1);
											SetMainteSelBtn(buf_num, 1, 2, SELON, 1);
										}	
										if( btn_press == 4 ){
											sel2 = SELON;
											SetMainteSelBtn(buf_num, 2, 1, SELON, 1);
											SetMainteSelBtn(buf_num, 2, 2, SELOFF, 1);
										}	
										if( btn_press == 5 ){
											sel2 = SELOFF;
											SetMainteSelBtn(buf_num, 2, 1, SELOFF, 1);
											SetMainteSelBtn(buf_num, 2, 2, SELON, 1);
										}	
										if( btn_press == 6 ){
											sel3 = SELON;
											SetMainteSelBtn(buf_num, 3, 1, SELON, 1);
											SetMainteSelBtn(buf_num, 3, 2, SELOFF, 1);
										}	
										if( btn_press == 7 ){
											sel3 = SELOFF;
											SetMainteSelBtn(buf_num, 3, 1, SELOFF, 1);
											SetMainteSelBtn(buf_num, 3, 2, SELON, 1);
										}	
										if( btn_press == 8 ){
											sel4 = SELON;
											SetMainteSelBtn(buf_num, 4, 1, SELON, 1);
											SetMainteSelBtn(buf_num, 4, 2, SELOFF, 1);
										}	
										if( btn_press == 9 ){
											sel4 = SELOFF;
											SetMainteSelBtn(buf_num, 4, 1, SELOFF, 1);
											SetMainteSelBtn(buf_num, 4, 2, SELON, 1);
										}	
										if( btn_press == 10 ){	//20160112Miya FinKeyS
											sel5 = SELON;
											SetMainteSelBtn(buf_num, 5, 1, SELON, 1);
											SetMainteSelBtn(buf_num, 5, 2, SELOFF, 1);
										}	
										if( btn_press == 11 ){	//20160112Miya FinKeyS
											sel5 = SELOFF;
											SetMainteSelBtn(buf_num, 5, 1, SELOFF, 1);
											SetMainteSelBtn(buf_num, 5, 2, SELON, 1);
										}	
									}									
								}else if(num == 263){
									if(btn_press == 0 || btn_press == 1){
										rtn = 1;
									}else{
										if( btn_press == 2 ){
											sel1 = SEL_0;
											SetMainteSelBtn(buf_num, 1, 0, SELON, 2);
											SetMainteSelBtn(buf_num, 1, 1, SELOFF, 2);
											SetMainteSelBtn(buf_num, 1, 2, SELOFF, 2);
										}	
										if( btn_press == 3 ){
											sel1 = SEL_1;
											SetMainteSelBtn(buf_num, 1, 0, SELOFF, 2);
											SetMainteSelBtn(buf_num, 1, 1, SELON, 2);
											SetMainteSelBtn(buf_num, 1, 2, SELOFF, 2);
										}	
										if( btn_press == 4 ){
											sel1 = SEL_2;
											SetMainteSelBtn(buf_num, 1, 0, SELOFF, 2);
											SetMainteSelBtn(buf_num, 1, 1, SELOFF, 2);
											SetMainteSelBtn(buf_num, 1, 2, SELON, 2);
										}	
										if( btn_press == 5 ){
											sel2 = SELON;
											SetMainteSelBtn(buf_num, 2, 1, SELON, 2);
											SetMainteSelBtn(buf_num, 2, 2, SELOFF, 2);
										}	
										if( btn_press == 6 ){
											sel2 = SELOFF;
											SetMainteSelBtn(buf_num, 2, 1, SELOFF, 2);
											SetMainteSelBtn(buf_num, 2, 2, SELON, 2);
										}	
										if( btn_press == 7 ){
											sel3 = SELON;
											SetMainteSelBtn(buf_num, 3, 1, SELON, 2);
											SetMainteSelBtn(buf_num, 3, 2, SELOFF, 2);
										}	
										if( btn_press == 8 ){
											sel3 = SELOFF;
											SetMainteSelBtn(buf_num, 3, 1, SELOFF, 2);
											SetMainteSelBtn(buf_num, 3, 2, SELON, 2);
										}	
										if( btn_press == 9 ){
											sel4 = SELON;
											SetMainteSelBtn(buf_num, 4, 1, SELON, 2);
											SetMainteSelBtn(buf_num, 4, 2, SELOFF, 2);
										}	
										if( btn_press == 10 ){
											sel4 = SELOFF;
											SetMainteSelBtn(buf_num, 4, 1, SELOFF, 2);
											SetMainteSelBtn(buf_num, 4, 2, SELON, 2);
										}
										//20161031Miya Ver2204 ->
										if( btn_press == 11 ){
											sel5 = SEL_0;
											SetMainteSelBtn(buf_num, 5, 0, SELON, 2);
											SetMainteSelBtn(buf_num, 5, 1, SELOFF, 2);
											SetMainteSelBtn(buf_num, 5, 2, SELOFF, 2);
										}	
										if( btn_press == 12 ){
											sel5 = SEL_1;
											SetMainteSelBtn(buf_num, 5, 0, SELOFF, 2);
											SetMainteSelBtn(buf_num, 5, 1, SELON, 2);
											SetMainteSelBtn(buf_num, 5, 2, SELOFF, 2);
										}	
										if( btn_press == 13 ){
											sel5 = SEL_2;
											SetMainteSelBtn(buf_num, 5, 0, SELOFF, 2);
											SetMainteSelBtn(buf_num, 5, 1, SELOFF, 2);
											SetMainteSelBtn(buf_num, 5, 2, SELON, 2);
										}	
										if( btn_press == 14 ){
											sel6 = SEL_0;
											SetMainteSelBtn(buf_num, 6, 0, SELON, 2);
											SetMainteSelBtn(buf_num, 6, 1, SELOFF, 2);
											SetMainteSelBtn(buf_num, 6, 2, SELOFF, 2);
										}	
										if( btn_press == 15 ){
											sel6 = SEL_1;
											SetMainteSelBtn(buf_num, 6, 0, SELOFF, 2);
											SetMainteSelBtn(buf_num, 6, 1, SELON, 2);
											SetMainteSelBtn(buf_num, 6, 2, SELOFF, 2);
										}	
										if( btn_press == 16 ){
											sel6 = SEL_2;
											SetMainteSelBtn(buf_num, 6, 0, SELOFF, 2);
											SetMainteSelBtn(buf_num, 6, 1, SELOFF, 2);
											SetMainteSelBtn(buf_num, 6, 2, SELON, 2);
										}	
										//20161031Miya Ver2204 <-
									}									
								}else{
									rtn = 1;	//CHG_REQ要求
								}
								if( rtn == 1 ){
									break;	//CHG_REQ要求
								}
							}else{
								rtn = 1;	//CHG_REQ要求
								break;
							}
						}
					}
				}

				if(rtn == 1){
					break;
				}else{
					dly_tsk((100/MSEC));	//100msecのウエイト(タッチキーの反応解除用)
				}
			}

			//if( num == 101 || num == 102 || num == 108){
			if( num == 101 || num == 102 || num == 108 || num == 601 || num == 602 || num == 608 || num == 610 || num == 611 ){	//20160108Miya FinKeyS
				rtnsts = MdGetMode();		//指挿入チェック
				if( rtnsts == MD_CAP ){		//指挿入ありの場合
					rtn = 2;	//CHG_REQなし
					LcdcBackLightOn();
					return(rtn);
					//break;
				}
			}

#if(PCCTRL == 1)	//20160930Miya PCからVA300Sを制御する
			if( num == 101 && g_pcproc_f == 1){
				rtn = 1;
				btn_press = 5;
				g_pcproc_f = 0;
				g_capallow = 1;
				break;
			}
			if( num == 101 && g_pcproc_f == 2){
				rtn = 1;
				btn_press = 0;
				g_pcproc_f = 0;
				g_capallow = 1;
				break;
			}
#endif
			
			if( dbg_cap_flg == 2 ){	//20160711Miya NewCmr
					rtn = 2;	//CHG_REQなし
					LcdcBackLightOn();
					return(rtn);
			}

			//if( num == 101 || num == 102 ){
			if( num == 101 || num == 102 || num == 601 || num == 602 || num == 621 || num == 622 ){	//20160108Miya FinKeyS
				if(g_MainteLog.err_wcnt != g_MainteLog.err_rcnt){	//エラー検出	//20140925Miya add err
					if( g_MainteLog.err_buff[g_MainteLog.err_rcnt][0] > 0 && g_MainteLog.err_buff[g_MainteLog.err_rcnt][0] < 5 ){//20150326Miya 未定義エラー対策
						if(g_MainteLog.err_buff[g_MainteLog.err_rcnt][4] == 0){
							g_MainteLog.err_rcnt++;
							g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;
						}else{
							errsts = 1;
							rtn = 1;
							break;
						}
					}else{
						g_MainteLog.err_rcnt++;
						g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;
					}						
				}
			}

			//エラー表示中にエラー解除された場合
			//if( num == 109 ){
			if( num == 109 || num == 609 || num == 625 ){	//20160108Miya FinKeyS
				if(g_MainteLog.err_wcnt != g_MainteLog.err_rcnt){	//エラー検出	//20140925Miya add err
					if( g_MainteLog.err_buff[g_MainteLog.err_rcnt][0] > 0 && g_MainteLog.err_buff[g_MainteLog.err_rcnt][0] < 5 ){//20150326Miya 未定義エラー対策
						if(g_mem_errnum == g_MainteLog.err_buff[g_MainteLog.err_rcnt][0] 
						   && g_MainteLog.err_buff[g_MainteLog.err_rcnt][4] == 0 ){
							g_MainteLog.err_rcnt++;
							g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;
							rtn = 1;
							break;
						}
					}else{
						g_MainteLog.err_rcnt++;
						g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;
					}						
				}				
			}

		}
	}

	LcdcBackLightOn();	//20160711Miya LCDバックライトON(予備)　画面真っ暗対応(仮)
	cnt = 0;
	switch(num)
	{
		case 1:
			*(buf + cnt++) = ( UB )LCD_USER_ID;
			*(buf + cnt++) = sv_keyindat[0];
			*(buf + cnt++) = sv_keyindat[1];
			*(buf + cnt++) = sv_keyindat[2];
			*(buf + cnt++) = sv_keyindat[3];
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 2:
			if(btn_press == 0){	//初期登録
				*(buf + cnt++) = ( UB )LCD_INIT_INPUT;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{	//メンテナンス
				*(buf + cnt++) = ( UB )LCD_MAINTE;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 3:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else{
				dat = btn_press;
				sv_reg_num = dat;
				dat += 0x30;
				*(buf + cnt++) = ( UB )LCD_YUBI_ID;
				*(buf + cnt++) = '0';
				*(buf + cnt++) = '0';
				*(buf + cnt++) = (UB)dat;
				*(buf + cnt++) = 0;
				
				sv_yubi_seq_no[0] = '0';
				sv_yubi_seq_no[1] = '0';
				sv_yubi_seq_no[2] = (UB)dat;
				//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
				//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '0';
				//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
			}
			*msize = cnt;
			break;
		case 4:

			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				for(i = 0 ; i < 24 ; i++ ){
					*(buf + i) = 0x20;
				}
				*(buf + 24) = 0;
				
				*(buf + cnt++) = ( UB )LCD_NAME;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];
				*(buf + cnt++) = sv_keyindat[4];
				*(buf + cnt++) = sv_keyindat[5];
				*(buf + cnt++) = sv_keyindat[6];
				*(buf + cnt++) = sv_keyindat[7];
				*msize = 26;
			
				//memcpy(&yb_touroku_data20[sv_reg_num].name[0], &sv_keyindat[0], 8);
			}
			
			break;
		case 5:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
				if(btn_press <= 3){
					*(buf + cnt++) = '1';	//左手
					dat = btn_press + 1 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '1';
					sv_yubi_no[1] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = '1';
					//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = (UB)dat;
				}else{
					*(buf + cnt++) = '0';	//右手
					dat = btn_press - 2 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '0';
					sv_yubi_no[1] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = '0';
					//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = (UB)dat;
				}
				
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 9:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = sv_yubi_seq_no[0];
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = sv_yubi_seq_no[1];
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = sv_yubi_seq_no[2];
			yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = sv_yubi_no[0];
			yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = sv_yubi_no[1];
			memcpy(&yb_touroku_data20[sv_reg_num].name[0], &sv_keyindat[0], 8);
			break;
		case 10:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = 0x30;
			//memset(&yb_touroku_data20[sv_reg_num].name[0], 0, 8);
			break;
		case 11:
			if(btn_press == 0){		//YES
				//2013.08.07 Miya del
				//*(buf + cnt++) = ( UB )LCD_YES;
				//*(buf + cnt++) = 0;

				//2013.08.07 Miya add
				j = 0;
				for(i = 1 ; i <= 4 ; i++){
					if(yb_touroku_data20[i].yubi_seq_no[2] == 0 || yb_touroku_data20[i].yubi_seq_no[2] == 0x30){
						++j;
					}
				}
				if( j == 0 ){	
					*(buf + cnt++) = ( UB )LCD_NO;
					*(buf + cnt++) = 0;
				}else{
					*(buf + cnt++) = ( UB )LCD_YES;
					*(buf + cnt++) = 0;
				}


			}else{					//NO
//20130610_Miya 画像データ採取実施フラグ
#ifdef GETIMG20130611
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
#else
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
#endif
			}
			*msize = cnt;
			break;
		case 12:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
//20130610_Miya 画像データ採取実施フラグ
#ifdef GETIMG20130611
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
#else
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
#endif
			}
			*msize = cnt;
			break;
		case 100:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 101:
		case 102:
			if( errsts == 1 ){
				*(buf + cnt++) = ( UB )LCD_ERR_REQ;
				*(buf + cnt++) = 0;
			}else{
				if(btn_press == 0){			//登録
					*(buf + cnt++) = ( UB )LCD_TOUROKU;
					*(buf + cnt++) = 0;
				}else if(btn_press == 1){	//削除
					*(buf + cnt++) = ( UB )LCD_SAKUJYO;
					*(buf + cnt++) = 0;
				}else if(btn_press == 2){	//緊急解錠番号
					*(buf + cnt++) = ( UB )LCD_KINKYUU_SETTEI;
					*(buf + cnt++) = 0;
				}else if(btn_press == 3){	//緊急解錠
					*(buf + cnt++) = ( UB )LCD_KINKYUU_KAIJYOU;
					*(buf + cnt++) = 0;
				}else if(btn_press == 4){	//メンテ
					if( g_PasswordOpen.sw == FLG_OFF ){	//20140925Miya password_open
						*(buf + cnt++) = ( UB )LCD_MAINTE;
						*(buf + cnt++) = 0;
					}else{
						*(buf + cnt++) = ( UB )LCD_PASSWORD_OPEN;
						*(buf + cnt++) = 0;
					}
				}
#if(PCCTRL)	//20160930Miya PCからVA300Sを制御する
				if(btn_press == 5){		//認証開始
					*(buf + cnt++) = ( UB )LCD_BACK;
					*(buf + cnt++) = 0;
				}
#endif
			}
			*msize = cnt;
			break;
		case 103:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 104:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 108:	//20140925Miya password_open
			if( errsts == 1 ){
				*(buf + cnt++) = ( UB )LCD_ERR_REQ;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				if(btn_press == KEYRTN){
					if( keta == 5 && sv_keyindat[0] == '3' && sv_keyindat[1] == '2' && sv_keyindat[2] == '2' && sv_keyindat[3] == '7'){
						*(buf + cnt++) = ( UB )LCD_MAINTE;
						*(buf + cnt++) = 0;
						*msize = cnt;
					}else{
						//20160711Miya デモ機
						if( g_TechMenuData.DemoSw == FLG_ON && keta == 5 && sv_keyindat[0] == '9' && sv_keyindat[1] == '9' && sv_keyindat[2] == '9' && sv_keyindat[3] == '9'){
							*(buf + cnt++) = ( UB )LCD_MAINTE;
							*(buf + cnt++) = 0;
							*msize = cnt;
							g_MainteLvl = 1;
						}else{
							*(buf + cnt++) = ( UB )LCD_MENU;
							//*(buf + cnt++) = ( UB )LCD_CANCEL;
							*(buf + cnt++) = 0;
							*msize = cnt;
						}
					}
				}else if(btn_press == KEYCAN){
					*(buf + cnt++) = ( UB )LCD_CANCEL;
					*(buf + cnt++) = 0;
					*msize = cnt;
				}else{
					*(buf + cnt++) = ( UB )LCD_KEYIN_PASSWORD;
					*(buf + cnt++) = sv_keyindat[0] - 0x30;
					*(buf + cnt++) = sv_keyindat[1] - 0x30;
					*(buf + cnt++) = sv_keyindat[2] - 0x30;
					*(buf + cnt++) = sv_keyindat[3] - 0x30;
					*(buf + cnt++) = sv_keyindat[4] - 0x30;
					*(buf + cnt++) = sv_keyindat[5] - 0x30;
					*(buf + cnt++) = sv_keyindat[6] - 0x30;
					*(buf + cnt++) = sv_keyindat[7] - 0x30;

					*(buf + cnt++) = 0;
					*msize = cnt;
					g_InPasswordOpen.keta = (short)keta - 1;
				}
			}
			break;
		case 109:	//20140925Miya add err
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 110:	//20141005Miya jikosindan
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 120:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 122:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 123:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 124:
		case 125:
		case 126:
		case 127:
		case 128:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else if(btn_press == 5){
				nop();
			}else if(btn_press == 6){
				nop();
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_ID;
				*(buf + cnt++) = '0';
				dat = btn_press + 4 * (page - 3);
				sv_reg_num = dat;
				if(dat >= 10){
					if(dat >= 20){
						*(buf + cnt++) = '2';
						dat = dat - 20 + 0x30;
						*(buf + cnt++) = (UB)dat;
						sv_yubi_seq_no[0] = '0';
						sv_yubi_seq_no[1] = '2';
						sv_yubi_seq_no[2] = (UB)dat;
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '2';
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}else{
						*(buf + cnt++) = '1';
						dat = dat - 10 + 0x30;
						*(buf + cnt++) = (UB)dat;
						sv_yubi_seq_no[0] = '0';
						sv_yubi_seq_no[1] = '1';
						sv_yubi_seq_no[2] = (UB)dat;
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '1';
						//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}
				}else{
					*(buf + cnt++) = '0';
					dat = dat + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_seq_no[0] = '0';
					sv_yubi_seq_no[1] = '0';
					sv_yubi_seq_no[2] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
					//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '0';
					//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
				}
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 129:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				for(i = 0 ; i < 24 ; i++ ){
					*(buf + i) = 0x00;
				}
				*(buf + 24) = 0;
				
				*(buf + cnt++) = ( UB )LCD_NAME;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];
				*(buf + cnt++) = sv_keyindat[4];
				*(buf + cnt++) = sv_keyindat[5];
				*(buf + cnt++) = sv_keyindat[6];
				*(buf + cnt++) = sv_keyindat[7];
				*msize = 26;
				//memcpy(&yb_touroku_data20[sv_reg_num].name[0], &sv_keyindat[0], 8);
			}
			break;
		case 130:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
				if(btn_press <= 3){
					*(buf + cnt++) = '1';	//左手
					dat = btn_press + 1 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '1';
					sv_yubi_no[1] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = '1';
					//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = (UB)dat;
				}else{
					*(buf + cnt++) = '0';	//右手
					dat = btn_press - 2 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '0';
					sv_yubi_no[1] = (UB)dat;
					//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = '0';
					//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = (UB)dat;
				}
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 134:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = sv_yubi_seq_no[0];
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = sv_yubi_seq_no[1];
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = sv_yubi_seq_no[2];
			yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = sv_yubi_no[0];
			yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = sv_yubi_no[1];
			memcpy(&yb_touroku_data20[sv_reg_num].name[0], &sv_keyindat[0], 8);
			break;
		case 135:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = 0x30;
			//memset(&yb_touroku_data20[sv_reg_num].name[0], 0, 8);
			break;
		case 136:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = 0x30;
			//yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = 0x30;
			//memset(&yb_touroku_data20[sv_reg_num].name[0], 0, 8);
			break;
		case 140:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 142:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 143:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 144:
		case 145:
		case 146:
		case 147:
		case 148:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else if(btn_press == 5){
				nop();
			}else if(btn_press == 6){
				nop();
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_ID;
				*(buf + cnt++) = '0';
				dat = btn_press + 4 * (page - 3);
				sv_reg_num = dat;
				if(dat >= 10){
					if(dat >= 20){
						*(buf + cnt++) = '2';
						dat = dat - 20 + 0x30;
						*(buf + cnt++) = (UB)dat;
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '2';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}else{
						*(buf + cnt++) = '1';
						dat = dat - 10 + 0x30;
						*(buf + cnt++) = (UB)dat;
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '1';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}
				}else{
					*(buf + cnt++) = '0';
					dat = dat + 0x30;
					*(buf + cnt++) = (UB)dat;
					yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
					yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '0';
					yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
				}
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
/*
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else if(btn_press == 5){
				nop();
			}else if(btn_press == 6){
				nop();
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_ID;
				*(buf + cnt++) = '0';
				dat = btn_press + 4 * (page - 3);
				if(dat >= 10){
					if(dat >= 20){
						*(buf + cnt++) = '2';
						dat = dat - 20 + 0x30;
						*(buf + cnt++) = (UB)dat;
					}else{
						*(buf + cnt++) = '1';
						dat = dat - 10 + 0x30;
						*(buf + cnt++) = (UB)dat;
					}
				}else{
					*(buf + cnt++) = '0';
					*(buf + cnt++) = (UB)dat;
				}
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
*/
		case 149:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
				yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = 0x30;
				yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = 0x30;
				yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = 0x30;
				yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = 0x30;
				yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = 0x30;
				memset(&yb_touroku_data20[sv_reg_num].name[0], 0, 8);
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 150:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 160:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 162:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 163:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 164:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_KINKYUU_BANGOU;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 165:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 170:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 171:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
			}else{	//→
				*(buf + cnt++) = ( UB )LCD_NEXT;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 172:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_USER_ID;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 173:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
			}else{	//→
				*(buf + cnt++) = ( UB )LCD_NEXT;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 174:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_KINKYUU_KAIJYOU_BANGOU;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];
				*(buf + cnt++) = sv_keyindat[4];
				*(buf + cnt++) = sv_keyindat[5];
				*(buf + cnt++) = sv_keyindat[6];
				*(buf + cnt++) = sv_keyindat[7];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 175:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_KINKYUU_BANGOU;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 176:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 177:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 178:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 180:	//20140925Miya password open
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 182:	//20140925Miya password open
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 183:	//20140925Miya password open
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 184:	//20140925Miya password open
			if(btn_press == 0){			//パスワード変更
				*(buf + cnt++) = ( UB )LCD_PASS_HENKOU_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 1){	//パスワード開錠設定変更
				*(buf + cnt++) = ( UB )LCD_PASS_SETTEI_HENKOU_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 2){
				*(buf + cnt++) = ( UB )LCD_NOUSE;
				*(buf + cnt++) = 0;
			}else if(btn_press == 3){
				*(buf + cnt++) = ( UB )LCD_NOUSE;
				*(buf + cnt++) = 0;
			}else if(btn_press == 4){	//終了
				*(buf + cnt++) = ( UB )LCD_MAINTE_END;
				*(buf + cnt++) = 0;
			}else{
				
			}

			*msize = cnt;
			break;
		case 185:	//20140925Miya password_open
			if(btn_press == KEYRTN){
				//*(buf + cnt++) = ( UB )LCD_MENU;
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_KEYIN_PASSWORD;
				*(buf + cnt++) = sv_keyindat[0] - 0x30;
				*(buf + cnt++) = sv_keyindat[1] - 0x30;
				*(buf + cnt++) = sv_keyindat[2] - 0x30;
				*(buf + cnt++) = sv_keyindat[3] - 0x30;
				*(buf + cnt++) = sv_keyindat[4] - 0x30;
				*(buf + cnt++) = sv_keyindat[5] - 0x30;
				*(buf + cnt++) = sv_keyindat[6] - 0x30;
				*(buf + cnt++) = sv_keyindat[7] - 0x30;

				*(buf + cnt++) = 0;
				*msize = cnt;
				g_PasswordOpen.keta = (short)keta - 1;
			}
			break;
		case 186:	//20140925Miya cng mainte
			if(btn_press == 0){		//戻る
				*(buf + cnt++) = ( UB )LCD_BACK;
				*(buf + cnt++) = 0;
			}else{	//確定
				*(buf + cnt++) = ( UB )LCD_ENTER;
				*(buf + cnt++) = ( UB )sel1;
				*(buf + cnt++) = ( UB )sel2;
				*(buf + cnt++) = ( UB )sel3;
				*(buf + cnt++) = ( UB )sel4;	//20160112Miya FinKeyS
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 187:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 188:	//20160108Miya FinKeyS
		case 189:	//20160108Miya FinKeyS
		case 190:	//20160108Miya FinKeyS
		case 191:	//20160108Miya FinKeyS
		case 192:	//20160108Miya FinKeyS
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				//*(buf + cnt++) = '0';
				*(buf + cnt++) = 0;
			}else if(btn_press == 5){
				nop();
			}else if(btn_press == 6){
				nop();
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_ID;
				*(buf + cnt++) = '0';
				dat = btn_press + 4 * (page - 3);
				sv_reg_num = dat;
				if(dat >= 10){
					if(dat >= 20){
						*(buf + cnt++) = '2';
						dat = dat - 20 + 0x30;
						*(buf + cnt++) = (UB)dat;
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '2';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}else{
						*(buf + cnt++) = '1';
						dat = dat - 10 + 0x30;
						*(buf + cnt++) = (UB)dat;
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '1';
						yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
					}
				}else{
					*(buf + cnt++) = '0';
					dat = dat + 0x30;
					*(buf + cnt++) = (UB)dat;
					yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = '0';
					yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = '0';
					yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)dat;
				}
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 200:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 201:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_USER_ID;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 202:
		//20140925Miya cng mainte
			if(btn_press == 0){			//情報
				*(buf + cnt++) = ( UB )LCD_JYOUHOU_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 1){	//設定変更
				*(buf + cnt++) = ( UB )LCD_SETTEI_HENKOU_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 2){	//診断
				*(buf + cnt++) = ( UB )LCD_SINDAN_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 3){	//初期設定
				*(buf + cnt++) = ( UB )LCD_SYOKI_SETTEI_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 4){	//終了
				*(buf + cnt++) = ( UB )LCD_MAINTE_END;
				*(buf + cnt++) = 0;
			}else{
				
			}

/*
			if(btn_press == 0){			//初期化
				*(buf + cnt++) = ( UB )LCD_MAINTE_SHOKIKA_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 1){	//情報
				*(buf + cnt++) = ( UB )LCD_JYOUHOU_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 2){	//仕様切替
				*(buf + cnt++) = ( UB )LCD_SPEC_CHG_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 3){	//フル画像
				*(buf + cnt++) = ( UB )LCD_FULL_PIC_SEND_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 4){	//終了
				*(buf + cnt++) = ( UB )LCD_MAINTE_END;
				*(buf + cnt++) = 0;
			}else{
				
			}
*/
			*msize = cnt;
			break;
		case 204:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 205:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 206:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 207:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 208:
			if(btn_press == 0){		//戻る
				*(buf + cnt++) = ( UB )LCD_BACK;
				*(buf + cnt++) = 0;
			}else{	//確定
				*(buf + cnt++) = ( UB )LCD_ENTER;
				*(buf + cnt++) = ( UB )sel1;
				*(buf + cnt++) = ( UB )sel2;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 220:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 221:	//20140925Miya cng mainte
			if(btn_press == 0){			//バージョン
				*(buf + cnt++) = ( UB )LCD_VERSION_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 1){	//エラー履歴
				*(buf + cnt++) = ( UB )LCD_ERROR_REREKI_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 2){	//認証状況
				*(buf + cnt++) = ( UB )LCD_NINSYOU_JYOUKYOU_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 3){	//
				*(buf + cnt++) = ( UB )LCD_JIKOKU_HYOUJI_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 4){	//終了
				*(buf + cnt++) = ( UB )LCD_MAINTE_END;
				*(buf + cnt++) = 0;
			}else{
				
			}

			*msize = cnt;
			break;
		case 222:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 223:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 224:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 225:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 240:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 241:	//20140925Miya cng mainte
			if(btn_press == 0){			//パスワード開錠
				*(buf + cnt++) = ( UB )LCD_PASS_KAIJYOU_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 1){	//コールセンターTEL
				*(buf + cnt++) = ( UB )LCD_CALLCEN_TEL_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 2){	//時刻合わせ
				*(buf + cnt++) = ( UB )LCD_JIKOKU_AWASE_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 3){	//LCD位置調整
				*(buf + cnt++) = ( UB )LCD_ITI_TYOUSEI_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 4){	//終了
				*(buf + cnt++) = ( UB )LCD_MAINTE_END;
				*(buf + cnt++) = 0;
			}else{
				
			}

			*msize = cnt;
			break;
		case 242:	//20140925Miya cng mainte
			if(btn_press == 0){		//戻る
				*(buf + cnt++) = ( UB )LCD_BACK;
				*(buf + cnt++) = 0;
			}else{	//確定
				*(buf + cnt++) = ( UB )LCD_ENTER;
				*(buf + cnt++) = ( UB )sel1;
				*(buf + cnt++) = ( UB )sel2;
				*(buf + cnt++) = ( UB )sel3;
				*(buf + cnt++) = ( UB )sel4;
				*(buf + cnt++) = ( UB )sel5;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 243: //20140925Miya cng mainte
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_CALLCEN_TEL_REQ;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 244: //20140925Miya cng mainte
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_CALLCEN_TEL_REQ;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 245: //20140925Miya cng mainte
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_CALLCEN_TEL_REQ;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 246: //20140925Miya cng mainte
			//20161031Miya Ver2204 LCDADJ
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_JIKOKU_AWASE_REQ;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}
/*
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_JIKOKU_AWASE_REQ;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
*/
			break;
		case 247: //20140925Miya cng mainte
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_JIKOKU_AWASE_REQ;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 248:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 249:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 260:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 261:	//20140925Miya cng mainte
			if(btn_press == 0){			//初期化
				*(buf + cnt++) = ( UB )LCD_MAINTE_SHOKIKA_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 1){	//仕様変更
				*(buf + cnt++) = ( UB )LCD_SPEC_CHG_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 2){	//画像確認
				*(buf + cnt++) = ( UB )LCD_IMAGE_KAKUNIN_REQ;
				*(buf + cnt++) = 0;
			}else if(btn_press == 3){	//
				*(buf + cnt++) = ( UB )LCD_NOUSE;
				*(buf + cnt++) = 0;
			}else if(btn_press == 4){	//終了
				*(buf + cnt++) = ( UB )LCD_MAINTE_END;
				*(buf + cnt++) = 0;
			}else{
				
			}

			*msize = cnt;
			break;
		case 262:
			if(btn_press == 0){			//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else if(btn_press == 1){	//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}else{
				*(buf + cnt++) = ( UB )LCD_NOUSE;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 263:	//20140925Miya cng mainte
			if(btn_press == 0){		//戻る
				*(buf + cnt++) = ( UB )LCD_BACK;
				*(buf + cnt++) = 0;
			}else{	//確定
				*(buf + cnt++) = ( UB )LCD_ENTER;
				*(buf + cnt++) = ( UB )sel1;
				*(buf + cnt++) = ( UB )sel2;
				*(buf + cnt++) = ( UB )sel3;
				*(buf + cnt++) = ( UB )sel4;
				*(buf + cnt++) = ( UB )sel5;
				*(buf + cnt++) = ( UB )sel6;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 264:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 265:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 266:	//20140925Miya cng mainte
			if(btn_press == 0){			//真ん中
				g_LedCheck = 1;	//20161031Miya Ver2204
				*(buf + cnt++) = ( UB )LCD_NEXT;
				*(buf + cnt++) = 1;
				*(buf + cnt++) = 0;
			}else{
				g_LedCheck = 0;	//20161031Miya Ver2204
				*(buf + cnt++) = ( UB )LCD_NEXT;
				*(buf + cnt++) = 0;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 267:	//20140925Miya cng mainte
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 401:
			if(btn_press == 0){	//初期登録
				*(buf + cnt++) = ( UB )LCD_INIT_INPUT;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{	//メンテナンス
				*(buf + cnt++) = ( UB )LCD_INIT_INPUT;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 402:
			*(buf + cnt++) = ( UB )LCD_USER_ID;
			*(buf + cnt++) = sv_keyindat[0];
			*(buf + cnt++) = sv_keyindat[1];
			*(buf + cnt++) = sv_keyindat[2];
			*(buf + cnt++) = sv_keyindat[3];
			*(buf + cnt++) = 0;
			*msize = cnt;

			sv_keyinid[0][0] = sv_keyindat[0];
			sv_keyinid[0][1] = sv_keyindat[1];
			sv_keyinid[0][2] = sv_keyindat[2];
			sv_keyinid[0][3] = sv_keyindat[3];
			sv_reg_num = 1;
			sel_lvl = 0; //監督者
			break;
		case 403:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				for(i = 0 ; i < 24 ; i++ ){
					*(buf + i) = 0x20;
				}
				*(buf + 24) = 0;
				
				*(buf + cnt++) = ( UB )LCD_NAME;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];
				*(buf + cnt++) = sv_keyindat[4];
				*(buf + cnt++) = sv_keyindat[5];
				*(buf + cnt++) = sv_keyindat[6];
				*(buf + cnt++) = sv_keyindat[7];
				*msize = 26;
			}
			
			break;
		case 404:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
				if(btn_press <= 3){
					*(buf + cnt++) = '1';	//左手
					dat = btn_press + 1 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '1';
					sv_yubi_no[1] = (UB)dat;
				}else{
					*(buf + cnt++) = '0';	//右手
					dat = btn_press - 2 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '0';
					sv_yubi_no[1] = (UB)dat;
				}
				
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 408:
			if(ini_reg_cnt > 0){
				if(	sv_keyinid[0][0] == sv_keyinid[ini_reg_cnt][0] && sv_keyinid[0][1] == sv_keyinid[ini_reg_cnt][1] && sv_keyinid[0][2] == sv_keyinid[ini_reg_cnt][2] && sv_keyinid[0][3] == sv_keyinid[ini_reg_cnt][3]){
					--ini_reg_cnt;
				}
			}

			++ini_reg_cnt;
			sv_keyinid[ini_reg_cnt][0] = sv_keyinid[0][0];
			sv_keyinid[ini_reg_cnt][1] = sv_keyinid[0][1];
			sv_keyinid[ini_reg_cnt][2] = sv_keyinid[0][2];
			sv_keyinid[ini_reg_cnt][3] = sv_keyinid[0][3];

			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
/*
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 0 ] = 0x30;
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 1 ] = 0x30;
			yb_touroku_data20[sv_reg_num].yubi_seq_no[ 2 ] = (UB)sv_reg_num + 0x30;
			yb_touroku_data20[sv_reg_num].yubi_no[ 0 ] = sv_yubi_no[0];
			yb_touroku_data20[sv_reg_num].yubi_no[ 1 ] = sv_yubi_no[1];
			memcpy(&yb_touroku_data20[sv_reg_num].name[0], &sv_keyindat[0], 8);
*/
			break;
		case 409:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 410:
			if(btn_press == 0 && ini_reg_cnt < 2){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 411:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 500:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 501:
			if(btn_press == KEYRTN){
				*(buf + cnt++) = ( UB )LCD_MENU;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_USER_ID;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];
				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 504:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 505:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 506:
			*(buf + cnt++) = ( UB )LCD_OK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 520:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 521:
			if(btn_press == 0){			//登録
				*(buf + cnt++) = ( UB )LCD_TOUROKU;
				*(buf + cnt++) = 0;
			}else if(btn_press == 1){	//削除
				*(buf + cnt++) = ( UB )LCD_SAKUJYO;
				*(buf + cnt++) = 0;
			}else if(btn_press == 2){	//終了
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
			}else if(btn_press == 4){	//メンテナンス
				*(buf + cnt++) = ( UB )LCD_MAINTE;
				*(buf + cnt++) = 0;
			}else{
				
			}
			*msize = cnt;
			break;
		case 526:
			if(btn_press == 0){			//監督者
				*(buf + cnt++) = ( UB )LCD_KANTOKU;
				*(buf + cnt++) = 0;
				++reg_cnt_lvl0;
				sv_reg_cnt_lvl = 0;
			}else if(btn_press == 1){	//管理者
				*(buf + cnt++) = ( UB )LCD_KANRI;
				*(buf + cnt++) = 0;
				++reg_cnt_lvl1;
				sv_reg_cnt_lvl = 1;
			}else if(btn_press == 2){	//一般者
				*(buf + cnt++) = ( UB )LCD_IPPAN;
				*(buf + cnt++) = 0;
				sv_reg_cnt_lvl = 2;
			}else if(btn_press == 4){	//終了
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
			}else{
				
			}
			*msize = cnt;
			break;
		case 522:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_USER_ID;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 524:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 525:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 527:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_USER_ID;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 528:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				for(i = 0 ; i < 24 ; i++ ){
					*(buf + i) = 0x20;
				}
				*(buf + 24) = 0;
				
				*(buf + cnt++) = ( UB )LCD_NAME;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];
				*(buf + cnt++) = sv_keyindat[4];
				*(buf + cnt++) = sv_keyindat[5];
				*(buf + cnt++) = sv_keyindat[6];
				*(buf + cnt++) = sv_keyindat[7];
				*msize = 26;
			}
			
			break;
		case 529:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
			}else{
				*(buf + cnt++) = ( UB )LCD_YUBI_SHUBETU;
				if(btn_press <= 3){
					*(buf + cnt++) = '1';	//左手
					dat = btn_press + 1 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '1';
					sv_yubi_no[1] = (UB)dat;
				}else{
					*(buf + cnt++) = '0';	//右手
					dat = btn_press - 2 + 0x30;
					*(buf + cnt++) = (UB)dat;
					sv_yubi_no[0] = '0';
					sv_yubi_no[1] = (UB)dat;
				}
				
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 533:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 534:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 535:
		case 536:
		case 537:
			*(buf + cnt++) = ( UB )LCD_OK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 538:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
//		case 542:
//			if(btn_press == 0){			//監督者
//				*(buf + cnt++) = ( UB )LCD_KANTOKU;
//				*(buf + cnt++) = 0;
//			}else if(btn_press == 1){	//管理者
//				*(buf + cnt++) = ( UB )LCD_KANRI;
//				*(buf + cnt++) = 0;
//			}else if(btn_press == 2){	//一般者
//				*(buf + cnt++) = ( UB )LCD_IPPAN;
//				*(buf + cnt++) = 0;
//			}else if(btn_press == 4){	//終了
//				*(buf + cnt++) = ( UB )LCD_CANCEL;
//				*(buf + cnt++) = 0;
//			}else{
//				
//			}
//			*msize = cnt;
//			break;
		case 543:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_USER_ID;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 545:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 546:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 547:
			if(btn_press == KEYCAN){
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				*(buf + cnt++) = ( UB )LCD_USER_ID;
				*(buf + cnt++) = sv_keyindat[0];
				*(buf + cnt++) = sv_keyindat[1];
				*(buf + cnt++) = sv_keyindat[2];
				*(buf + cnt++) = sv_keyindat[3];

				*(buf + cnt++) = 0;
				*msize = cnt;
			}
			break;
		case 548:
		case 551:
			if(btn_press == 0){		//YES
				*(buf + cnt++) = ( UB )LCD_YES;
				*(buf + cnt++) = 0;
			}else{					//NO
				*(buf + cnt++) = ( UB )LCD_NO;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 549:
		case 550:
			*(buf + cnt++) = ( UB )LCD_OK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 601:
		case 602:
			if( errsts == 1 ){
				*(buf + cnt++) = ( UB )LCD_ERR_REQ;
				*(buf + cnt++) = 0;
			}else{
				if(btn_press == 0){			//おでかけ
					*(buf + cnt++) = ( UB )LCD_ODEKAKE;
					*(buf + cnt++) = 0;
				}else if(btn_press == 1){	//お留守番
					*(buf + cnt++) = ( UB )LCD_ORUSUBAN;
					*(buf + cnt++) = 0;
				}else if(btn_press == 2){	//設定
					*(buf + cnt++) = ( UB )LCD_SETTEI;
					*(buf + cnt++) = 0;
				}else if(btn_press == 3){	//緊急解錠
					*(buf + cnt++) = ( UB )LCD_KINKYUU_KAIJYOU;
					*(buf + cnt++) = 0;
				}else if(btn_press == 4){	//メンテ
					if( g_PasswordOpen.sw == FLG_OFF ){	//20140925Miya password_open
						*(buf + cnt++) = ( UB )LCD_MAINTE;
						*(buf + cnt++) = 0;
					}else{
						*(buf + cnt++) = ( UB )LCD_PASSWORD_OPEN;
						*(buf + cnt++) = 0;
					}
				}
			}
			*msize = cnt;
			break;
		case 603:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 604:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 608:	//20140925Miya password_open
			if( errsts == 1 ){
				*(buf + cnt++) = ( UB )LCD_ERR_REQ;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				if(btn_press == KEYRTN){
					if( keta == 5 && sv_keyindat[0] == '3' && sv_keyindat[1] == '2' && sv_keyindat[2] == '2' && sv_keyindat[3] == '7'){
						*(buf + cnt++) = ( UB )LCD_MAINTE;
						*(buf + cnt++) = 0;
						*msize = cnt;
					}else{
						*(buf + cnt++) = ( UB )LCD_MENU;
						*(buf + cnt++) = 0;
						*msize = cnt;
					}
				}else if(btn_press == KEYCAN){
					*(buf + cnt++) = ( UB )LCD_CANCEL;
					*(buf + cnt++) = 0;
					*msize = cnt;
				}else{
					*(buf + cnt++) = ( UB )LCD_KEYIN_PASSWORD;
					*(buf + cnt++) = sv_keyindat[0] - 0x30;
					*(buf + cnt++) = sv_keyindat[1] - 0x30;
					*(buf + cnt++) = sv_keyindat[2] - 0x30;
					*(buf + cnt++) = sv_keyindat[3] - 0x30;
					*(buf + cnt++) = sv_keyindat[4] - 0x30;
					*(buf + cnt++) = sv_keyindat[5] - 0x30;
					*(buf + cnt++) = sv_keyindat[6] - 0x30;
					*(buf + cnt++) = sv_keyindat[7] - 0x30;

					*(buf + cnt++) = 0;
					*msize = cnt;
					g_InPasswordOpen.keta = (short)keta - 1;
				}
			}
			break;
		case 609:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 610:
		case 611:
			if(btn_press == 0){	//中止
				*(buf + cnt++) = ( UB )LCD_CANCEL;
				*(buf + cnt++) = 0;
			}else{	//→
				*(buf + cnt++) = ( UB )LCD_PASSWORD_OPEN;
				*(buf + cnt++) = 0;
			}
			*msize = cnt;
			break;
		case 612:
			if( errsts == 1 ){
				*(buf + cnt++) = ( UB )LCD_ERR_REQ;
				*(buf + cnt++) = 0;
			}else{
				if(btn_press == 0){			//登録
					*(buf + cnt++) = ( UB )LCD_TOUROKU;
					*(buf + cnt++) = 0;
				}else if(btn_press == 1){	//削除
					*(buf + cnt++) = ( UB )LCD_SAKUJYO;
					*(buf + cnt++) = 0;
				}else if(btn_press == 2){	//緊急解錠番号
					*(buf + cnt++) = ( UB )LCD_KINKYUU_SETTEI;
					*(buf + cnt++) = 0;
				}else if(btn_press == 3){	//未使用
					*(buf + cnt++) = ( UB )LCD_NOUSE;
					*(buf + cnt++) = 0;
				}else if(btn_press == 4){	//戻る
					*(buf + cnt++) = ( UB )LCD_BACK;
					*(buf + cnt++) = 0;
				}
			}
			*msize = cnt;
			break;
		case 613:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 620:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 621:
			if( errsts == 1 ){
				*(buf + cnt++) = ( UB )LCD_ERR_REQ;
				*(buf + cnt++) = 0;
				*msize = cnt;
			}else{
				if(btn_press == KEYRTN){
					if( keta == 5 && sv_keyindat[0] == 0x3A && sv_keyindat[1] == 0x3A && sv_keyindat[2] == 0x3A && sv_keyindat[3] == 0x3A){	//★4つ
						*(buf + cnt++) = ( UB )LCD_MAINTE;
						*(buf + cnt++) = 0;
						*msize = cnt;
					}else if( keta == 5 && sv_keyindat[0] == '3' && sv_keyindat[1] == '2' && sv_keyindat[2] == '2' && sv_keyindat[3] == '7'){
						*(buf + cnt++) = ( UB )LCD_MAINTE;
						*(buf + cnt++) = 0;
						*msize = cnt;
					}else{
#if(KOUJYOUCHK)
						if( keta == 5 && sv_keyindat[0] == '9' && sv_keyindat[1] == '9' && sv_keyindat[2] == '9' && sv_keyindat[3] == '9'){
							*(buf + cnt++) = ( UB )LCD_MAINTE;
							*(buf + cnt++) = 0;
							*msize = cnt;
						}else{
							*(buf + cnt++) = ( UB )LCD_MENU;
							if(keta > 1){
								*(buf + cnt++) = sv_keyindat[0] - 0x30;
								*(buf + cnt++) = sv_keyindat[1] - 0x30;
								*(buf + cnt++) = sv_keyindat[2] - 0x30;
								*(buf + cnt++) = sv_keyindat[3] - 0x30;
								*(buf + cnt++) = sv_keyindat[4] - 0x30;
								*(buf + cnt++) = sv_keyindat[5] - 0x30;
								*(buf + cnt++) = sv_keyindat[6] - 0x30;
								*(buf + cnt++) = sv_keyindat[7] - 0x30;
							}
							*(buf + cnt++) = 0;
							*msize = cnt;
							g_InPasswordOpen.keta = (short)keta - 1;		
						}	
#else
						*(buf + cnt++) = ( UB )LCD_MENU;
						if(keta > 1){
							*(buf + cnt++) = sv_keyindat[0] - 0x30;
							*(buf + cnt++) = sv_keyindat[1] - 0x30;
							*(buf + cnt++) = sv_keyindat[2] - 0x30;
							*(buf + cnt++) = sv_keyindat[3] - 0x30;
							*(buf + cnt++) = sv_keyindat[4] - 0x30;
							*(buf + cnt++) = sv_keyindat[5] - 0x30;
							*(buf + cnt++) = sv_keyindat[6] - 0x30;
							*(buf + cnt++) = sv_keyindat[7] - 0x30;
						}
						*(buf + cnt++) = 0;
						*msize = cnt;
						g_InPasswordOpen.keta = (short)keta - 1;
#endif
					}
				}else if(btn_press == KEYCAN){
					*(buf + cnt++) = ( UB )LCD_CANCEL;
					*(buf + cnt++) = 0;
					*msize = cnt;
				}else{
					*(buf + cnt++) = ( UB )LCD_KEYIN_PASSWORD;
					*(buf + cnt++) = sv_keyindat[0] - 0x30;
					*(buf + cnt++) = sv_keyindat[1] - 0x30;
					*(buf + cnt++) = sv_keyindat[2] - 0x30;
					*(buf + cnt++) = sv_keyindat[3] - 0x30;
					*(buf + cnt++) = sv_keyindat[4] - 0x30;
					*(buf + cnt++) = sv_keyindat[5] - 0x30;
					*(buf + cnt++) = sv_keyindat[6] - 0x30;
					*(buf + cnt++) = sv_keyindat[7] - 0x30;

					*(buf + cnt++) = 0;
					*msize = cnt;
					g_InPasswordOpen.keta = (short)keta - 1;
				}
			}
			break;
		case 623:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 624:
			*(buf + cnt++) = ( UB )LCD_NEXT;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		case 625:
			*(buf + cnt++) = ( UB )LCD_BACK;
			*(buf + cnt++) = 0;
			*msize = cnt;
			break;
		default:
			proc = 0;
			break;	
	}

	return(rtn);
}


UB KeyInputNumSousa( int gmn, int buf, int *keta, int btn )
{
	UB rtn=0;
	int num, key;
	int lmt, key_rtn_menu;
	
	num = *keta;
	
	if(gmn == 174 ){
		lmt = 8;
	}else{
		lmt = 4;
	}

	if(sys_kindof_spec == 1 ){//法人仕様の場合は、[menu]ボタン
		key_rtn_menu = 1;
	}else{
		key_rtn_menu = 1;
	}

	switch( btn )
	{
		case KEY1:
			key = 1;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY2:
			key = 2;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY3:
			key = 3;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY4:
			key = 4;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY5:
			key = 5;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY6:
			key = 6;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY7:
			key = 7;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY8:
			key = 8;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY9:
			key = 9;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEY0:
			key = 0;
			if( num <= lmt ){
				sv_keyindat[num-1] = 0x30 + (UB)key;
				SetKeyNum(buf, num, key);
				++num;
			}
			break;
		case KEYCAN:
			break;
		case KEYDEL:
			key = 10;
			if( num > 1 ){
				sv_keyindat[num-1] = 0x20;
				--num;
				SetKeyNum(buf, num, key);
			}
			break;
		case KEYRTN:
			//if(sys_kindof_spec == 1 ){//法人仕様の場合は、[menu]ボタン
			if(key_rtn_menu == 1 ){ //2014925Miya password_open [menu]ボタン
				rtn = 1;
			}else{
				if( gmn == 243 || gmn == 244 || gmn == 245 ){
					rtn = 1;
				}else{
					if( num > lmt ){
						rtn = 1;
					}
				}
			}
			break;
		case KEYSP2:
		case KEYSP3:
			if(gmn == 501){//法人仕様
				if( num > lmt ){
					rtn = 1;
				}
			}
			break;
		default:
			break;
	}
	
	*keta = num;
	return(rtn);	
}


UB KeyInputNumSousaPassKaijyou( int gmn, int buf, int *keta, int btn )
{
	UB rtn=0;
	int num, key;
	int lmt, key_rtn_menu;
	
	num = *keta;
	
	lmt = 8;
	key_rtn_menu = 1;

	switch( btn )
	{
		case KEY1:
			key = 1;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEY2:
			key = 2;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEY3:
			key = 3;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEY4:
			key = 4;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEY5:
			key = 5;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEY6:
			key = 6;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEY7:
			key = 7;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEY8:
			key = 8;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEY9:
			key = 9;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEY0:
			key = 0;
			if( g_PasswordOpen.random_key == FLG_ON ){	//20140925Miya password open
				key = g_key_arry[key];
			}
			if( num <= lmt ){
				if( pass_key_hyouji_flg == 0 ){
					sv_keyindat[num-1] = 0x30 + (UB)key;
				}else{
					sv_keyindat[num-1] = 0x30 + (UB)key + 10;
				}
				SetKeyNumPassWord(gmn, buf, num, key);
				++num;
			}
			break;
		case KEYCAN:
			break;
		case KEYDEL:
			key = 10;
			if( num > 1 ){
				sv_keyindat[num-1] = 0x20;
				--num;
				SetKeyNumPassWord(gmn, buf, num, key);
			}
			break;
		case KEYRTN:
			//if(sys_kindof_spec == 1 ){//法人仕様の場合は、[menu]ボタン
			if(key_rtn_menu == 1 ){ //2014925Miya password_open [menu]ボタン
				rtn = 1;
			}else{
				if( num > lmt ){
					rtn = 1;
				}
			}
			break;
		case KEYSP2:
		case KEYSP3:
			if( num > 4 ){
				rtn = 1;
			}
			if(g_TechMenuData.SysSpec == 2 && num == 1){	//20160120Miya
				rtn = 1;
			}
			break;
		case KEYSP1:	
			if( g_PasswordOpen.kigou_inp == FLG_ON ){
				if(pass_key_hyouji_flg == 0){
					SetPassKeyBtn(buf, 1, 0);
					pass_key_hyouji_flg = 1;
				}else{
					SetPassKeyBtn(buf, 0, 0);
					pass_key_hyouji_flg = 0;
				}
			}
			break;
		default:
			break;
	}
	
	*keta = num;
	return(rtn);	
}




//ひらがなキーボード操作
UB KeyInputMojiSousa( int buf, int *hit, int *keta, int btn )
{
	UB rtn=0, code;
	int num, key, cnt;
	
	num = *keta;
	cnt = *hit;

	switch( btn )
	{
		case KEY1:
			key = MOJICODEAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 10){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY2:
			key = MOJICODEKAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEKAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY3:
			key = MOJICODESAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODESAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY4:
			key = MOJICODETAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 6){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODETAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY5:
			key = MOJICODENAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODENAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY6:
			key = MOJICODEHAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEHAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY7:
			key = MOJICODEMAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEMAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY8:
			key = MOJICODEYAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 6){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEYAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY9:
			key = MOJICODERAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 5){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODERAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEY0:
			key = 0;
			if(sv_mkey == MOJICODEKAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEGAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEGAGYOU;
				}
			}else if(sv_mkey == MOJICODESAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEZAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEZAGYOU;
				}
			}else if(sv_mkey == MOJICODETAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEDAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEDAGYOU;
				}
			}else if(sv_mkey == MOJICODEHAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEBAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEBAGYOU;
				}
			}else if(sv_mkey == MOJICODEBAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEPAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEPAGYOU;
				}
			}else if(sv_mkey == MOJICODEGAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEKAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEKAGYOU;
				}
			}else if(sv_mkey == MOJICODEZAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODESAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODESAGYOU;
				}
			}else if(sv_mkey == MOJICODEDAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODETAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODETAGYOU;
				}
			}else if(sv_mkey == MOJICODEPAGYOU){
				--num;
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODEHAGYOU, cnt, num, key);
					++num;
					sv_mkey = MOJICODEHAGYOU;
				}
			}			
			
			break;
		case KEYKIGOU:
			key = MOJICODEWAGYOU;
			if(sv_mkey == key){
				--num;
				if( cnt < 6){
					++cnt;
				}else{
					cnt = 1;
				}
			}else{
				cnt = 1;
			}
			sv_mkey = key;
		
			if( num <= 8 ){
				SetKeyMoji(buf, MOJICODEWAGYOU, cnt, num, key);
				++num;
			}
			break;
		case KEYCAN:
			key = 0xff;
			rtn = 1;
			break;
		case KEYDEL:
			key = 0xfe;
			cnt = 0;
			if( num > 1 ){
				//sv_keyindat[num-1] = 0x20;
				--num;
				sv_keyindat[num-1] = 0;
				SetKeyMoji(buf, MOJICODEDEL, cnt, num, key);
			}
			sv_mkey = key;
			break;
		case KEYRTN:
			key = 0xfd;
			if( num > 1 ){
				rtn = 1;
			}
			break;
		case KEYCSR:
			key = 0xfc;
			if( cnt >0 ){
				if( num <= 8 ){
					SetKeyMoji(buf, MOJICODECSR, cnt, num, key);
					cnt = 0;
					sv_mkey = 0xff;
				}
			}
			break;
		default:
			sv_mkey = 0xff;
			break;
	}
	
	*keta = num;
	*hit = cnt;
	return(rtn);	
}




UB KeyPageSousa( int *page, int btn )
{
	UB rtn=0;
	int num;
	
	num = *page;

	if( btn == 5 ){			//→ボタン
		num += 1;
		if( num > 7 ){
			num = 3;
		}
		SetInfo(num);
		LcdcDisplayModeSet(num, 0);
	}else if( btn == 6 ){		//←ボタン
		if(num > 3){
			num -= 1;
			SetInfo(num);
			LcdcDisplayModeSet(num, 0);
		}
	}else{
		rtn = 1;	//CHG_REQ要求
	}
	
	*page = num;
	
	return(rtn);
}


void SetVerNum(int buf_num)
{
	int x1, x2, y1, y2;
	int num, dat;
	int i, j;
	unsigned short VerPos1[4][2] ={ {80, 136}, {120, 136}, {160, 136}, {200, 136} };

	x2 = VerPos1[0][0];
	y2 = VerPos1[0][1];
	x1 = x2 - 23;
	y1 = y2 - 11;
	for(i = 0; i < 4; i++){
		if( i == 1 ){
			dat = 10;		
		}else{
			dat = (int)Senser_soft_VER[i] - 0x30;
		}

		SetGmnToLcdBuff04(buf_num, dat, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
		y1 -= 12;
		y2 -= 12;
	}

	//20141014Miya
	y1 -= 12;
	y2 -= 12;
	dat = (int)Senser_soft_VER[4] - 0x30;
	SetGmnToLcdBuff04(buf_num, dat, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);

	if(AUTHTEST >= 1){	//20160711Miya デモ機
		y1 -= 12;
		y2 -= 12;
		dat = 12;	//11:「※」
		SetGmnToLcdBuff04(buf_num, dat, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
	}else if(FORDEMO == 1){	//20160711Miya デモ機
		y1 -= 12;
		y2 -= 12;
		dat = 11;	//11:「-」
		SetGmnToLcdBuff04(buf_num, dat, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
	}


	x2 = VerPos1[1][0];
	y2 = VerPos1[1][1];
	x1 = x2 - 23;
	y1 = y2 - 11;
	for(i = 0; i < 5; i++){
		//20140905Miya lbp追加 FPGAバージョンアップ
		if(i == 0){
			dat = FpgaVerNum & 0xF000;
			dat = dat >> 12;
		}
		if( i == 1 ){
			dat = 10;		//','
		}		
		if(i == 2){
			dat = FpgaVerNum & 0x0F00;
			dat = dat >> 8;
		}
		if(i == 3){
			dat = FpgaVerNum & 0x00F0;
			dat = dat >> 4;
		}
		if(i == 4){
			dat = FpgaVerNum & 0x000F;
		}

//		if( i == 1 ){
//			dat = 10;		
//		}else{
//			dat = (int)Senser_FPGA_VER[i] - 0x30;
//		}

		SetGmnToLcdBuff04(buf_num, dat, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
		y1 -= 12;
		y2 -= 12;
	}

	x2 = VerPos1[2][0];
	y2 = VerPos1[2][1];
	x1 = x2 - 23;
	y1 = y2 - 11;
	for(i = 0; i < 4; i++){
		if( i == 1 ){
			dat = 10;		
		}else{
			dat = (int)Ninshou_soft_VER[i] - 0x30;
		}

		SetGmnToLcdBuff04(buf_num, dat, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
		y1 -= 12;
		y2 -= 12;
	}

	x2 = VerPos1[3][0];
	y2 = VerPos1[3][1];
	x1 = x2 - 23;
	y1 = y2 - 11;
	for(i = 0; i < 4; i++){
		if( i == 1 ){
			dat = 10;		
		}else{
			dat = (int)KeyIO_board_soft_VER[i] - 0x30;
		}

		SetGmnToLcdBuff04(buf_num, dat, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
		y1 -= 12;
		y2 -= 12;
	}
}

void SetMainteSelBtn(int buf_num, int line, int sel, int sw, int mode)
{
	int x1, x2, y1, y2;
	int num, dat, set_sel;
	int i, j;
	unsigned short Pos1[3][2] ={ { 80, 151}, { 80,  100}, { 80,  49} };
	unsigned short Pos2[3][2] ={ {120, 151}, {120,  100}, {120,  49} };
	unsigned short Pos3[3][2] ={ {160, 151}, {160,  100}, {160,  49} };
	unsigned short Pos4[3][2] ={ {200, 151}, {200,  100}, {200,  49} };
	unsigned short Pos5[3][2] ={ {240, 151}, {240,  100}, {240,  49} };	//20160112Miya FinKeyS
	unsigned short Pos6[3][2] ={ {280, 151}, {280,  100}, {280,  49} };	//20161031Miya Ver2204

	if( mode == 0 ){	//mode 0:旧仕様切替 1:ﾊﾟｽﾜｰﾄﾞ開錠 2:新仕様切替
		if( line == 1 ){
			set_sel = sel;
		}else{
			set_sel = sel + 3;
		}
	}else if( mode == 1 ){
		set_sel = sel + 2;	//3:ON 4:OFF
	}else{
		if( line == 1 ){
			set_sel = sel;
		}else if( line >= 5 ){
			set_sel = sel + 5;
		}else{
			set_sel = sel + 2;
		}
	}

	switch(line)
	{
		case 1:
			x2 = Pos1[sel][0];
			y2 = Pos1[sel][1];
			x1 = x2 - 23;
			y1 = y2 - 41;
			break;
		case 2:
			x2 = Pos2[sel][0];
			y2 = Pos2[sel][1];
			x1 = x2 - 23;
			y1 = y2 - 41;
			break;
		case 3:
			x2 = Pos3[sel][0];
			y2 = Pos3[sel][1];
			x1 = x2 - 23;
			y1 = y2 - 41;
			break;
		case 4:
			x2 = Pos4[sel][0];
			y2 = Pos4[sel][1];
			x1 = x2 - 23;
			y1 = y2 - 41;
			break;
		case 5:	//20160112Miya FinKeyS
			x2 = Pos5[sel][0];
			y2 = Pos5[sel][1];
			x1 = x2 - 23;
			y1 = y2 - 41;
			break;
		case 6:	//20161031Miya Ver2204
			x2 = Pos6[sel][0];
			y2 = Pos6[sel][1];
			x1 = x2 - 23;
			y1 = y2 - 41;
			break;
		default:
			x2 = Pos1[sel][0];
			y2 = Pos1[sel][1];
			x1 = x2 - 23;
			y1 = y2 - 41;
			break;
	}

	SetGmnToLcdBuff05(buf_num, set_sel, sw, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);

}



void SetMenuNoBtn(int buf_num)
{
	int x1, x2, y1, y2;
	int no_btn;

	no_btn = 0;
	if( s_ID_Authority_Level == '0' && reg_cnt_lvl0 >= 2){	//監督者
		no_btn = 1;
	}
	if( s_ID_Authority_Level == '1' ){
		no_btn = 1;
	}
	if(no_btn == 1){
		x1 = MenuBtn[0][0];
		y1 = MenuBtn[0][1];
		x2 = MenuBtn[1][0];
		y2 = MenuBtn[1][1];
		SetGmnToLcdBuff02(buf_num, GMNNOBTNCODE, 1, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
	}		
		
	no_btn = 0;
	if( s_ID_Authority_Level == '0' && reg_cnt_lvl1 >= 10){	//監督者
		no_btn = 1;
	}
	if( s_ID_Authority_Level == '1' ){
		no_btn = 1;
	}
	if(no_btn == 1){
		x1 = MenuBtn[2][0];
		y1 = MenuBtn[2][1];
		x2 = MenuBtn[3][0];
		y2 = MenuBtn[3][1];
		SetGmnToLcdBuff02(buf_num, GMNNOBTNCODE, 2, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);
	}		
}


void SetErrMes(int buf_num, int mes_num)
{
	int x1, x2, y1, y2;
	int num;
	
	x1 = MesPosition[0][0];
	y1 = MesPosition[0][1];
	x2 = MesPosition[1][0];
	y2 = MesPosition[1][1];

	if(mes_num == 2){
		x1 = 160;
		x2 = x1 + 40 - 1;
		mes_num = 0;
	}


	SetGmnToLcdBuff02(buf_num, GMNERRMESCODE, mes_num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}

void SetKeyHeadMes(int buf_num, int mes_num)
{
	int x1, x2, y1, y2;
	int num;
	
	x1 = KeyHeadMesPosition[0][0];
	y1 = KeyHeadMesPosition[0][1];
	x2 = KeyHeadMesPosition[1][0];
	y2 = KeyHeadMesPosition[1][1];

	SetGmnToLcdBuff02(buf_num, GMNKEYHEADCODE, mes_num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}

void SetKinkyuNum1(int buf_num, int keta, int key)
{
	int x1, x2, y1, y2;
	int num;
	
	num = 2 * (keta - 1);
	
	x1 = kinkyuNumPos1[num][0];
	y1 = kinkyuNumPos1[num][1];
	x2 = kinkyuNumPos1[num+1][0];
	y2 = kinkyuNumPos1[num+1][1];
	SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, key, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}

void SetKinkyuNum2(int buf_num, int keta, int key)
{
	int x1, x2, y1, y2;
	int num;
	
	num = 2 * (keta - 1);
	
	x1 = kinkyuNumPos2[num][0];
	y1 = kinkyuNumPos2[num][1];
	x2 = kinkyuNumPos2[num+1][0];
	y2 = kinkyuNumPos2[num+1][1];
	SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, key, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}


void SetKeyNum(int buf_num, int keta, int key)
{
	int x1, x2, y1, y2;
	int num;

	//20140423Miya 認証リトライ回数
	if( keta >= 20 ){
		if( keta == 20 ){
			x1 = 400;
			y1 = 231;	//121
			x2 = 439;
			y2 = 262;	//152
		}else if(keta == 21){
			x1 = 400;
			y1 = 199;
			x2 = 439;
			y2 = 230;
		}else if(keta == 22){
			x1 = 400;
			y1 = 167;
			x2 = 439;
			y2 = 198;
		}else if(keta == 23){
			x1 = 400;
			y1 = 135;
			x2 = 439;
			y2 = 166;
		}else if(keta == 24){
			x1 = 400;
			y1 = 103;
			x2 = 439;
			y2 = 134;
		}else if(keta == 25){
			x1 = 400;
			y1 = 71;
			x2 = 439;
			y2 = 102;
		}else if(keta == 26){
			x1 = 400;
			y1 = 39;
			x2 = 439;
			y2 = 70;
		}else{
			x1 = 400;
			y1 = 7;
			x2 = 439;
			y2 = 38;
		}
	}else if( keta >= 10 ){
		if( keta == 10 ){
			x1 = 440;
			y1 = 231;	//121
			x2 = 479;
			y2 = 262;	//152
		}else if(keta == 11){
			x1 = 440;
			y1 = 199;
			x2 = 479;
			y2 = 230;
		}else if(keta == 12){
			x1 = 440;
			y1 = 167;
			x2 = 479;
			y2 = 198;
		}else if(keta == 13){
			x1 = 440;
			y1 = 135;
			x2 = 479;
			y2 = 166;
		}else if(keta == 14){
			x1 = 440;
			y1 = 103;
			x2 = 479;
			y2 = 134;
		}else if(keta == 15){
			x1 = 440;
			y1 = 71;
			x2 = 479;
			y2 = 102;
		}else if(keta == 16){
			x1 = 440;
			y1 = 39;
			x2 = 479;
			y2 = 70;
		}else{
			x1 = 440;
			y1 = 7;
			x2 = 479;
			y2 = 38;
		}
	}else{
		num = 2 * (keta - 1);
	
		x1 = InpPosKey[num][0];
		y1 = InpPosKey[num][1];
		x2 = InpPosKey[num+1][0];
		y2 = InpPosKey[num+1][1];
	}
		
	SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, key, 0, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}


//20140925Miya add mainte
void SetKeyNumPara(int buf_num, int type)
{
	int i, line, x1, x2, y1, y2, cnt, keta;
	int num, in_num[6], sub_num;
	int num1, num2, num3, num4, num5, num6;
	unsigned long dat, dat1;
	UINT now_t, now_m, now_s;

	if(type == 0){			// エラー表示
		x1 = 160;	y1 = 200;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
		cnt = g_MainteLog.err_rcnt;
		if( g_MainteLog.err_buff[cnt][0] > 0 && g_MainteLog.err_buff[cnt][0] < 100 ){
//			if( g_MainteLog.err_buff[cnt][0] == 40 ){	//カメラ撮影異常時エラー表示しない
//				++g_MainteLog.err_rcnt;
//				g_MainteLog.err_rcnt = g_MainteLog.err_rcnt & 0x7F;
//			}else{
				sub_num = 1; //0:* 1:E
				SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, num, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			
				dat = g_MainteLog.err_buff[cnt][0] / 10;
				in_num[0] = (int)dat;
				dat = g_MainteLog.err_buff[cnt][0] % 10;
				in_num[1] = (int)dat;

				g_mem_errnum = g_MainteLog.err_buff[cnt][0];

				y1 = y1 - 32;
				y2 = y2 - 32;
				num = in_num[0];
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);

				y1 = y1 - 32;
				y2 = y2 - 32;
				num = in_num[1];
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
//			}
		}
	}else if(type == 1){	// エラー履歴表示
		keta = 5;
		for( line = 0 ; line < 12 ; line++ ){
			if( line == 0 ){		//1行目
				x1 = 0;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}else if( line == 1 ){	//2行目
				x1 = 40;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				cnt = g_MainteLog.err_wcnt - 1;
 				if( cnt < 0 )	cnt = 0x7F;
				if( g_MainteLog.err_buff[cnt][0] == 0 || g_MainteLog.err_buff[cnt][0] >= 100 ){
					continue;
				}
				num = 1;					
				sub_num = 1; //0:* 1:E
				SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, num, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}else if( line == 2 ){	//3行目
				x1 = 80;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				cnt = g_MainteLog.err_wcnt - 2;
 				if( cnt < 0 )	cnt = 0x7F;
				if( g_MainteLog.err_buff[cnt][0] == 0 || g_MainteLog.err_buff[cnt][0] >= 100 ){
					continue;
				}					
				num = 1;					
				sub_num = 1; //0:* 1:E
				SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, num, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}else if( line == 3 ){	//4行目
				x1 = 120;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				cnt = g_MainteLog.err_wcnt - 3;
 				if( cnt < 0 )	cnt = 0x7F;
				if( g_MainteLog.err_buff[cnt][0] == 0 || g_MainteLog.err_buff[cnt][0] >= 100 ){
					continue;
				}					
				num = 1;					
				sub_num = 1; //0:* 1:E
				SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, num, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}else if( line == 4 ){	//5行目
				x1 = 160;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				cnt = g_MainteLog.err_wcnt - 4;
 				if( cnt < 0 )	cnt = 0x7F;
				if( g_MainteLog.err_buff[cnt][0] == 0 || g_MainteLog.err_buff[cnt][0] >= 100 ){
					continue;
				}					
				num = 1;					
				sub_num = 1; //0:* 1:E
				SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, num, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}else if( line == 5 ){	//6行目
				x1 = 200;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				cnt = g_MainteLog.err_wcnt - 5;
 				if( cnt < 0 )	cnt = 0x7F;
				if( g_MainteLog.err_buff[cnt][0] == 0 || g_MainteLog.err_buff[cnt][0] >= 100 ){
					continue;
				}					
				num = 1;					
				sub_num = 1; //0:* 1:E
				SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, num, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}else if( line == 6 ){	//7行目
				x1 = 240;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				cnt = g_MainteLog.err_wcnt - 6;
 				if( cnt < 0 )	cnt = 0x7F;
				if( g_MainteLog.err_buff[cnt][0] == 0 || g_MainteLog.err_buff[cnt][0] >= 100 ){
					continue;
				}					
				num = 1;					
				sub_num = 1; //0:* 1:E
				SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, num, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}else if( line == 7 ){	//8行目
				x1 = 280;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				cnt = g_MainteLog.err_wcnt - 7;
 				if( cnt < 0 )	cnt = 0x7F;
				if( g_MainteLog.err_buff[cnt][0] == 0 || g_MainteLog.err_buff[cnt][0] >= 100 ){
					continue;
				}					
				num = 1;					
				sub_num = 1; //0:* 1:E
				SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, num, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}else{	//9行目
				continue;
			}

			if(g_MainteLog.err_buff[cnt][0] >= 100){
				g_MainteLog.err_buff[cnt][0] = 0;
			}

			dat = g_MainteLog.err_buff[cnt][0] / 10;
			in_num[0] = (int)dat;
			dat = g_MainteLog.err_buff[cnt][0] % 10;
			in_num[1] = (int)dat;

			if( g_MainteLog.err_buff[cnt][4] == 1 )	in_num[2] = 11;	//*表示
			else									in_num[2] = 10;	//スペース

			in_num[3] = 10;	//スペース

			if(line == 1 ){
				if( g_MainteLog.cmr_seq_err_f & 0x01 )	in_num[4] = 11;	//*表示
				else									in_num[4] = 10;	//スペース
			}else if(line == 2 ){
				if( g_MainteLog.cmr_seq_err_f & 0x02 )	in_num[4] = 11;	//*表示
				else									in_num[4] = 10;	//スペース
			}else{
				in_num[4] = 10;	//スペース
			}

/*
			if( g_MainteLog.err_buff[cnt][1] < 100 ){
				dat = g_MainteLog.err_buff[cnt][1] / 10;
				in_num[3] = (int)dat;
				dat = g_MainteLog.err_buff[cnt][1] % 10;
				in_num[4] = (int)dat;
			}else{
				in_num[3] = 11;
				in_num[4] = 11;
			}

			if( g_MainteLog.err_buff[cnt][2] < 100 ){
				dat = g_MainteLog.err_buff[cnt][2] / 10;
				in_num[5] = (int)dat;
				dat = g_MainteLog.err_buff[cnt][2] % 10;
				in_num[6] = (int)dat;
			}else{
				in_num[5] = 11;
				in_num[6] = 11;
			}
*/
			for( i = 0 ; i < keta ; i++ ){	//6桁表示
				y1 = y1 - 32;
				y2 = y2 - 32;
				num = in_num[i];
				if(num < 10){
					SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
					LcdcDisplayModeSet(buf_num, 0);
				}else{
					if(num == 11){
						sub_num = 0; //0:* 1:E
						SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, 0, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
						LcdcDisplayModeSet(buf_num, 0);
					}
				}
			}
		}
	}else if(type == 2){	// 診断結果表示
		keta = 3;
		cnt = g_MainteLog.diag_cnt1 - 1;
 		if( cnt < 0 )	cnt = 0x07;
		for( line = 0 ; line < 12 ; line++ ){
			if( line == 0 ){		//1行目
				x1 = 0;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}else if( line == 1 ){	//2行目
				x1 = 40;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 1;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_MainteLog.diag_buff[cnt][0];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 2 ){	//3行目
				x1 = 80;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 2;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_MainteLog.diag_buff[cnt][1];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 3 ){	//4行目
				x1 = 120;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 3;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_MainteLog.diag_buff[cnt][2];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 4 ){	//5行目
				x1 = 160;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 4;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_MainteLog.diag_buff[cnt][3];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 5 ){	//6行目
				x1 = 200;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 5;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_MainteLog.diag_buff[cnt][4];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else{
				continue;
			}

			for( i = 0 ; i < keta ; i++ ){	//1桁表示
				y1 = y1 - 32;
				y2 = y2 - 32;
				if( dat > 0 ){
					if( i == 2 )	sub_num = 1; //0:* 1:E
					else			sub_num = 0; //0:* 1:E
					SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, num, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
				}else{
					num = 0;
					SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				}
				LcdcDisplayModeSet(buf_num, 0);
			}
		}
	}else if(type == 3){	// 認証パラメータ表示
		keta = 6;
		for( line = 0 ; line < 12 ; line++ ){
			if( line == 0 ){		//1行目
				x1 = 0;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}else if( line == 1 ){	//2行目
				x1 = 40;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 1;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_AuthLog.ok_cnt;
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 2 ){	//3行目
				x1 = 80;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 2;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_AuthLog.ng_cnt;
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 3 ){	//4行目
				x1 = 120;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 3;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_AuthLog.ok_cnt1st;
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 4 ){	//5行目
				x1 = 160;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 4;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_AuthLog.ok_cnt2nd;
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 5 ){	//6行目
				x1 = 200;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 5;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_AuthLog.ok_cnt3rd;
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 6 ){	//7行目
				x1 = 240;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 1;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				if( g_AuthLog.wcnt == 0 ){ cnt = 7; }
				else                     { cnt = g_AuthLog.wcnt - 1; }
				//20161031Miya Ver2204 ->
				dat = g_BkDataNoClear.LiveCnt;
				dat = 1000 * dat;
				dat = dat + g_AuthLog.now_result[cnt][0];
				//dat = g_AuthLog.now_result[cnt][0];
				//20161031Miya Ver2204 <-
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 7 ){	//8行目
				x1 = 280;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 2;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				if( g_AuthLog.wcnt == 0 ){ cnt = 7; }
				else                     { cnt = g_AuthLog.wcnt - 1; }
				dat = g_AuthLog.now_result[cnt][1];
				dat = 1000 * dat;
				dat = dat + g_AuthLog.now_result[cnt][2];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 8 ){	//9行目
				x1 = 320;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 3;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				if( g_AuthLog.wcnt == 0 ){ cnt = 7; }
				else                     { cnt = g_AuthLog.wcnt - 1; }
				dat = g_AuthLog.now_seq_num;			//シーケンスNo
				dat = 1000 * dat;
				dat = dat + g_AuthLog.now_result[cnt][3];		//LBP
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 9 ){	//10行目
				x1 = 360;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}else if( line == 10 ){	//11行目
				x1 = 400;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}else if( line == 11 ){	//12行目
				x1 = 420;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}
	
			in_num[0] = (int)(dat / 100000);
			dat = dat - (unsigned long)in_num[0] * 100000;
			in_num[1] = (int)(dat / 10000);
			dat = dat - (unsigned long)in_num[1] * 10000;
			in_num[2] = (int)(dat / 1000);
			dat = dat - (unsigned long)in_num[2] * 1000;
			in_num[3] = (int)(dat / 100);
			dat = dat - (unsigned long)in_num[3] * 100;
			in_num[4] = (int)(dat / 10);
			dat = dat - (unsigned long)in_num[4] * 10;
			in_num[5] = (int)dat;

			for( i = 0 ; i < keta ; i++ ){	//6桁表示
				y1 = y1 - 32;
				y2 = y2 - 32;
				num = in_num[i];
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}
		}
	}else if(type == 4){
		keta = 6;
		for( line = 0 ; line < 12 ; line++ ){
			if( line == 0 ){		//1行目
				x1 = 0;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}else if( line == 1 ){	//2行目
				x1 = 40;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 1;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);

				//現在時刻取得
				now_t = count_1hour;
				now_m = count_1min;
				now_s = count_1sec;

				dat = now_t / 10;
				in_num[0] = (int)dat;
				dat = now_t % 10;
				in_num[1] = (int)dat;

				dat = now_m / 10;
				in_num[2] = (int)dat;
				dat = now_m % 10;
				in_num[3] = (int)dat;

				dat = now_s / 10;
				in_num[4] = (int)dat;
				dat = now_s % 10;
				in_num[5] = (int)dat;
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 2 ){	//2行目
				x1 = 80;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 1;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);

				dat = g_MainteLog.st_hour / 10;
				in_num[0] = (int)dat;
				dat = g_MainteLog.st_hour % 10;
				in_num[1] = (int)dat;

				dat = g_MainteLog.st_min / 10;
				in_num[2] = (int)dat;
				dat = g_MainteLog.st_min % 10;
				in_num[3] = (int)dat;

				dat = g_MainteLog.st_sec / 10;
				in_num[4] = (int)dat;
				dat = g_MainteLog.st_sec % 10;
				in_num[5] = (int)dat;
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else{
				continue;
			}
			
			for( i = 0 ; i < keta ; i++ ){	//6桁表示
				y1 = y1 - 32;
				y2 = y2 - 32;
				num = in_num[i];
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}
		}			
	}else if(type == 5){	//5
		keta = 6;
		for( line = 0 ; line < 12 ; line++ ){
			if( line == 0 ){		//1行目
				x1 = 0;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}else if( line == 6 ){	//7行目
				x1 = 240;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 1;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_RegUserInfoData.RegDensityAve[0];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 7 ){	//8行目
				x1 = 280;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 2;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_RegUserInfoData.RegDensityAve[1];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 8 ){	//9行目
				x1 = 320;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 3;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_RegUserInfoData.RegDensityAve[2];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else{
				continue;
			}
			
			in_num[0] = (int)(dat / 100000);
			dat = dat - (unsigned long)in_num[0] * 100000;
			in_num[1] = (int)(dat / 10000);
			dat = dat - (unsigned long)in_num[1] * 10000;
			in_num[2] = (int)(dat / 1000);
			dat = dat - (unsigned long)in_num[2] * 1000;
			in_num[3] = (int)(dat / 100);
			dat = dat - (unsigned long)in_num[3] * 100;
			in_num[4] = (int)(dat / 10);
			dat = dat - (unsigned long)in_num[4] * 10;
			in_num[5] = (int)dat;

			for( i = 0 ; i < keta ; i++ ){	//6桁表示
				y1 = y1 - 32;
				y2 = y2 - 32;
				num = in_num[i];
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}
		}			
	}else if(type == 6){	//6	
		keta = 6;
		for( line = 0 ; line < 12 ; line++ ){
			if( line == 0 ){		//1行目
				x1 = 0;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}else if( line == 5 ){	//6行目
				x1 = 200;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = g_RegBloodVesselTagData[dbg_Auth_hcnt].RegImageSens;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_RegBloodVesselTagData[dbg_Auth_hcnt].RegFingerSize;
				y1 = y1 - 32;
				y2 = y2 - 32;

			}else if( line == 6 ){	//7行目
				x1 = 240;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 1;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_RegBloodVesselTagData[dbg_Auth_hcnt].RegDensityAve[0];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 7 ){	//8行目
				x1 = 280;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 2;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_RegBloodVesselTagData[dbg_Auth_hcnt].RegDensityAve[1];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 8 ){	//9行目
				x1 = 320;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 3;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_RegBloodVesselTagData[dbg_Auth_hcnt].RegDensityAve[2];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else{
				continue;
			}
			
			in_num[0] = (int)(dat / 100000);
			dat = dat - (unsigned long)in_num[0] * 100000;
			in_num[1] = (int)(dat / 10000);
			dat = dat - (unsigned long)in_num[1] * 10000;
			in_num[2] = (int)(dat / 1000);
			dat = dat - (unsigned long)in_num[2] * 1000;
			in_num[3] = (int)(dat / 100);
			dat = dat - (unsigned long)in_num[3] * 100;
			in_num[4] = (int)(dat / 10);
			dat = dat - (unsigned long)in_num[4] * 10;
			in_num[5] = (int)dat;

			for( i = 0 ; i < keta ; i++ ){	//6桁表示
				y1 = y1 - 32;
				y2 = y2 - 32;
				num = in_num[i];
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}
		}			
	}else if(type == 7){	//20161031Miya Ver2204 LCDADJ //7
		keta = 3;
		for( line = 0 ; line < 3 ; line++ ){
			if( line == 0 ){		//1行目
				x1 = 0;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				continue;
			}else if( line == 1 ){	//7行目
				x1 = 240;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 1;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_lcdpos[g_lcd_adj_cnt][0];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else if( line == 2 ){	//8行目
				x1 = 280;	y1 = 231;	x2 = x1 + 40 - 1;	y2 = y1 + 32 - 1;
				num = 2;
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
				dat = g_lcdpos[g_lcd_adj_cnt][1];
				y1 = y1 - 32;
				y2 = y2 - 32;
			}else{
				continue;
			}
			
			in_num[0] = (int)(dat / 100);
			dat = dat - (unsigned long)in_num[0] * 100;
			in_num[1] = (int)(dat / 10);
			dat = dat - (unsigned long)in_num[1] * 10;
			in_num[2] = (int)dat;

			for( i = 0 ; i < keta ; i++ ){	//6桁表示
				y1 = y1 - 32;
				y2 = y2 - 32;
				num = in_num[i];
				SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, num, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				LcdcDisplayModeSet(buf_num, 0);
			}
		}			
	}
}



//20140925Miya password open
void SetKeyNumPassWord(int gmn, int buf_num, int keta, int key)
{
	int x1, x2, y1, y2;
	int num, sub_num;

	num = 2 * (keta - 1);
	
	x1 = InpPosKey[num][0];
	y1 = InpPosKey[num][1];
	x2 = InpPosKey[num+1][0];
	y2 = InpPosKey[num+1][1];

	//if(g_PasswordOpen.hide_num == FLG_ON && gmn == 108){
	if(g_PasswordOpen.hide_num == FLG_ON && (gmn == 108 || gmn == 608 || gmn == 621) ){	//20160108Miya FinKeyS
		sub_num = 0; //0:* 1:E
		SetGmnToLcdBuff02(buf_num, GMN10KEYHIDECOADE, key, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
	}else{
		sub_num = 0; //未使用
		if( pass_key_hyouji_flg == 0 ){
			SetGmnToLcdBuff02(buf_num, GMN10KEYCOADE, key, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
		}else{
			SetGmnToLcdBuff02(buf_num, GMN10KEYKIGOUCOADE, key, sub_num, 0, 0, 0, 0, x1, y1, x2, y2);
		}
	}
	LcdcDisplayModeSet(buf_num, 0);
}



void SetKeyMoji(int buf_num, int moji, int hit, int keta, int key)
{
	UB code;
	int x1, x2, y1, y2;
	int num;


	//if(moji >= 0 && moji <= 0x0E ){
	if(moji >= 0 && moji <= 0x0F ){	//20140204Miya 「ぱ」が記録されないバグ修正
		code = (UB)moji << 4;
		code = code | ((UB)hit - 1);
		sv_keyindat[keta-1] = code;
	}

	num = 2 * (keta - 1);
	
	x1 = InpPosKey[num][0];
	y1 = InpPosKey[num][1];
	x2 = InpPosKey[num+1][0];
	y2 = InpPosKey[num+1][1];

	SetGmnToLcdBuff02(buf_num, GMNMKEYCOADE, moji, hit-1, 0, 0, 0, 0, x1, y1, x2, y2);
	LcdcDisplayModeSet(buf_num, 0);
}

void SetInfo(int page)
{
	int num, inp_num;
	int i, j;
	int x1, x2, y1, y2;
	int moji, hit;
	
	
	num = 4 * (page - 3) + 1;

	for(i = 0, j = 0 ; i < 4 ; i++, j+=2 ){
		if( yb_touroku_data20[num].yubi_seq_no[0] == 0x30 && yb_touroku_data20[num].yubi_seq_no[1] == 0x30 && yb_touroku_data20[num].yubi_seq_no[2] == 0x30){
			x1 = InpPos1[j][0];
			y1 = InpPos1[j][1];
			x2 = InpPos1[j+1][0];
			y2 = InpPos1[j+1][1];		
			inp_num = 7;	//消去
			SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);

			x1 = InpPos2[j][0];
			y1 = InpPos2[j][1];
			x2 = InpPos2[j+1][0];
			y2 = InpPos2[j+1][1];
			inp_num = 7;	//消去
			SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);

			//20160120Miya
			x1 = InpPos1[j][0] + 8;
			y1 = InpPos1[j+1][1] + 16;
			x2 = x1 + 7;
			y2 = y1 + 8;
			inp_num = 7;
			SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
		}else{
			x1 = InpPos1[j][0];
			y1 = InpPos1[j][1];
			x2 = InpPos1[j+1][0];
			y2 = InpPos1[j+1][1];		
			inp_num = yb_touroku_data20[num].yubi_no[ 0 ] - 0x30;
			if( inp_num == 0 || inp_num == 1 ){
				SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
			}

			x1 = InpPos2[j][0];
			y1 = InpPos2[j][1];
			x2 = InpPos2[j+1][0];
			y2 = InpPos2[j+1][1];
			inp_num = yb_touroku_data20[num].yubi_no[ 1 ] - 0x30;
			if( inp_num >= 1 && inp_num <= 5 ){
				inp_num += 1;
				SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
			}

			//20160120Miya
			if(GetScreenNo() == LCD_SCREEN188){
				x1 = InpPos1[j][0] + 8;
				y1 = InpPos1[j+1][1] + 16;
				x2 = x1 + 7;
				y2 = y1 + 8;
				if(g_PasswordOpen2.keta[num-1] >= 4 && g_PasswordOpen2.keta[num-1] <= 8){
					inp_num = 6;
					SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				}else{
					inp_num = 7;
					SetGmnToLcdBuff02(page, inp_num, 0, 0, 0, 0, 0, 0, x1, y1, x2, y2);
				}
			}
		}
		num++;
	}
	
	num = 4 * (page - 3) + 1;
	for(i = 0 ; i < 4 ; i++){
		if( yb_touroku_data20[num].yubi_seq_no[0] == 0x30 && yb_touroku_data20[num].yubi_seq_no[1] == 0x30 && yb_touroku_data20[num].yubi_seq_no[2] == 0x30){
			for( j = 0 ; j < 8 ; j++ ){
				x1 = InpPos3[i][0];
				y1 = InpPos3[i][1] - 32 * j;
				x2 = x1 + 31;
				y2 = y1 + 31;
				
				hit = 0;
				SetGmnToLcdBuff03(4, page, GMNMKEYCOADE, MOJICODEDEL, hit, 0, 0, 0, 0, x1, y1, x2, y2);	//消去
			}	
		}else{
			for( j = 0 ; j < 8 ; j++ ){
				x1 = InpPos3[i][0];
				y1 = InpPos3[i][1] - 32 * j;
				x2 = x1 + 31;
				y2 = y1 + 31;
				
				moji = yb_touroku_data20[num].name[j];
				if(moji != 0){
					hit = moji & 0x0F;
					moji = (moji & 0xF0) >> 4;
					//SetGmnToLcdBuff02(page, GMNMKEYCOADE, moji, hit, 0, 0, 0, 0, x1, y1, x2, y2);
					SetGmnToLcdBuff03(4, page, GMNMKEYCOADE, moji, hit, 0, 0, 0, 0, x1, y1, x2, y2);
				}else{
					hit = 0;
					SetGmnToLcdBuff03(4, page, GMNMKEYCOADE, MOJICODEDEL, hit, 0, 0, 0, 0, x1, y1, x2, y2);	//消去
				}					
			}	
		}
		num++;
	}
}

//20140925Miya add mainte
void SetGmnToLcdBuffInImage(int buf_num, int type)
{
	ER rtn;
	volatile unsigned long size, cnt, x, y, offset, cnt2;
	volatile UH val1;
	UH x1, x2, y1, y2;
	UB perm, perm_r, perm_g, perm_b;
	volatile UB pix;

	perm = perm_r = perm_g = perm_b = 0;
	if(type >= 10){
		if(type == 11){
			size = 80 * 40; 
			x1 = 48;
			y1 = 191;
			x2 = 127;
			y2 = 230;
			memcpy(&g_ubResizeSvBuf[0], &RegImgBuf[dbg_Auth_hcnt][0][0][0], size);
		}else if(type == 12){
			size = 80 * 40; 
			x1 = 48;
			y1 = 96;
			x2 = 127;
			y2 = 135;
			memcpy(&g_ubResizeSvBuf[0], &RegImgBuf[dbg_Auth_hcnt][1][0][0], size);
		}else if(type == 13){
			size = 20 * 10; 
			x1 = 152;
			y1 = 191;
			x2 = 175;
			y2 = 200;
			cnt = 0;
			cnt2 = 0;
			for(y = 0 ; y < 10 ; y++){
				for(x = 0 ; x < 24 ; x++){
					if(x < 20)
						g_ubResizeSvBuf[cnt] = g_RegBloodVesselTagData[dbg_Auth_hcnt].MinAuthImg[0][cnt2++];
					else					
						g_ubResizeSvBuf[cnt] =  0x67;
					++cnt;
				}
			}
		}else if(type == 14){
			size = 20 * 10; 
			x1 = 152;
			y1 = 96;
			x2 = 175;
			y2 = 105;
			cnt = 0;
			cnt2 = 0;
			for(y = 0 ; y < 10 ; y++){
				for(x = 0 ; x < 24 ; x++){
					if(x < 20)
						g_ubResizeSvBuf[cnt] = g_RegBloodVesselTagData[dbg_Auth_hcnt].MinAuthImg[1][cnt2++];
					else					
						g_ubResizeSvBuf[cnt] =  0x67;
					++cnt;
				}
			}
		}else if(type == 15){
			size = 20 * 10; 
			x1 = 152;
			y1 = 144;
			x2 = 175;
			y2 = 153;
			memcpy(&g_ubSobelR2Buf[0], &RegImgBuf[dbg_Auth_hcnt][0][1][0], 80*40);
			ImgResize4(2);	//20140423miya R3リサイズ位置変更
			cnt = 0;
			cnt2 = 0;
			for(y = 0 ; y < 10 ; y++){
				for(x = 0 ; x < 24 ; x++){
					if(x < 20)
						g_ubResizeSvBuf[cnt] = g_ubSobelR3Buf[cnt2++];
					else					
						g_ubResizeSvBuf[cnt] =  0x6767;
					++cnt;
				}
			}
		}else if(type == 16){
			size = 20 * 10; 
			x1 = 152;
			y1 = 49;
			x2 = 175;
			y2 = 58;
			memcpy(&g_ubSobelR2Buf[0], &RegImgBuf[dbg_Auth_hcnt][1][1][0], 80*40);
			ImgResize4(2);	//20140423miya R3リサイズ位置変更
			cnt = 0;
			cnt2 = 0;
			for(y = 0 ; y < 10 ; y++){
				for(x = 0 ; x < 24 ; x++){
					if(x < 20)
						g_ubResizeSvBuf[cnt] = g_ubSobelR3Buf[cnt2++];
					else					
						g_ubResizeSvBuf[cnt] =  0x6767;
					++cnt;
				}
			}
		}else if(type == 21){
			size = 80 * 40; 
			x1 = 48;
			y1 = 191;
			x2 = 127;
			y2 = 230;
			//memcpy(&g_ubResizeSvBuf[0], &g_ubResizeBuf[0], size);
			memcpy(&g_ubResizeSvBuf[0], &CapImgBuf[0][0][0], size);
		}else if(type == 22){
			size = 80 * 40; 
			x1 = 48;
			y1 = 96;
			x2 = 127;
			y2 = 135;
			//memcpy(&g_ubResizeSvBuf[0], &g_ubSobelR1Buf[0], size);
			memcpy(&g_ubResizeSvBuf[0], &CapImgBuf[0][1][0], size);
		}else if(type == 23){
			//size = 80 * 40; 
			//x1 = 48;
			//y1 = 2;
			//x2 = 127;
			//y2 = 41;
			//memcpy(&g_ubResizeSvBuf[0], &g_ubSobelR2Buf[0], size);
			size = 80 * 40; 
			x1 = 144;
			y1 = 191;
			x2 = 223;
			y2 = 230;
			memcpy(&g_ubResizeSvBuf[0], &CapImgBuf[1][0][0], size);
		}else if(type == 24){
			size = 80 * 40; 
			x1 = 144;
			y1 = 96;
			x2 = 223;
			y2 = 135;
			memcpy(&g_ubResizeSvBuf[0], &CapImgBuf[1][1][0], size);
		}

		LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, x1, y1, x2, y2);

		size = (x2 - x1 + 1) * (y2 - y1 + 1);
		for(cnt = 0 ; cnt < size ; cnt++ ){
			pix = g_ubResizeSvBuf[cnt];

			val1 = (UH)pix;
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}

			val1 = val1 << 8;
			val1 = val1 | (UH)pix;		
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else{
#if (NEWCMR == 0)	//20160601Miya
		if(type == 1){
			x1 = 48;
			y1 = 191;
			x2 = 207;
			y2 = 270;
			offset = 0;
		}else if(type == 2){
			x1 = 48;
			y1 = 96;
			x2 = 207;
			y2 = 175;
			offset = iReSizeX * iReSizeY;
		}else if(type == 3){
			x1 = 48;
			y1 = 2;
			x2 = 207;
			y2 = 81;
			offset = 2 * iReSizeX * iReSizeY;
		}else{
			x1 = 48;
			y1 = 116;
			x2 = 151;		//8の倍数にするため、ｘサイズ100->104にする
			y2 = 155;
			offset = 0;

#if(AUTHTEST >= 1)	//20160613Miya
			memcpy(&g_ubCapBuf[0], &TstAuthImgBuf[dbg_Auth_hcnt][0], 100*40);
#endif
		}

		LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, x1, y1, x2, y2);

		if(type < 4){
			size = (x2 - x1 + 1) * (y2 - y1 + 1);
			for(cnt = 0 ; cnt < size ; cnt++ ){
				pix = g_ubCapBuf[cnt + offset];

				val1 = (UH)pix;
				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}

				val1 = val1 << 8;
				val1 = val1 | (UH)pix;		
				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}
			}
		}else{
			cnt = 0;
			for(y = 0 ; y < 40 ; y++){
				for(x = 0 ; x < 104 ; x++){
					if(x < 100){
						pix = g_ubCapBuf[cnt + offset];
						++cnt;
					}else{
						pix = 0x67;
					}
					
					val1 = (UH)pix;
					rtn = LcdImageWrite(val1);	
					if(rtn != E_OK ){
						break;
					}

					val1 = val1 << 8;
					val1 = val1 | (UH)pix;		
					rtn = LcdImageWrite(val1);	
					if(rtn != E_OK ){
						break;
					}
				}
			}
		}
#else		
		if(type == 1){
			if( g_TechMenuData.DebugHyouji == FLG_ON ){
				x1 = 48;
				y1 = 96;
				x2 = 247;
				y2 = 175;
				offset = 0;
			}else{
				x1 = 0;
				y1 = 16;
				x2 = 319;
				y2 = 255;
				offset = 0;
			}

			LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, x1, y1, x2, y2);
		
			size = (x2 - x1 + 1) * (y2 - y1 + 1);
			for(cnt = 0 ; cnt < size ; cnt++ ){
				pix = g_ubCapBuf[cnt + offset];

				val1 = (UH)pix;
				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}

				val1 = val1 << 8;
				val1 = val1 | (UH)pix;		
				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}
			}
		}else{
			x1 = 48;
			y1 = 116;
			x2 = 151;		//8の倍数にするため、ｘサイズ100->104にする
			y2 = 155;
			offset = 0;

#if(AUTHTEST >= 1)	//20160613Miya
			memcpy(&g_ubCapBuf[0], &TstAuthImgBuf[dbg_Auth_hcnt][0], 100*40);
#endif

			LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, x1, y1, x2, y2);

			cnt = 0;
			for(y = 0 ; y < 40 ; y++){
				for(x = 0 ; x < 104 ; x++){
					if(x < 100){
						pix = g_ubCapBuf[cnt + offset];
						++cnt;
					}else{
						pix = 0x67;
					}
					
					val1 = (UH)pix;
					rtn = LcdImageWrite(val1);	
					if(rtn != E_OK ){
						break;
					}

					val1 = val1 << 8;
					val1 = val1 | (UH)pix;		
					rtn = LcdImageWrite(val1);	
					if(rtn != E_OK ){
						break;
					}
				}
			}
		}

		LcdcDisplayWindowFinish();
		
		if( g_TechMenuData.DebugHyouji == FLG_OFF && type == 1 ){
		//枠線を引く 20160315
			x1 = 0;
			//y1 = 96 + (g_TechMenuData.CmrCenter / 2);
			y1 = 96 + g_TechMenuData.CmrCenter;
			x2 = x1 + 320 - 1;
			y2 = y1;
			//y2 = y1 + 80 - 1;

			LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, x1, y1, x2, y2);
		
			size = (x2 - x1 + 1) * (y2 - y1 + 1);
			for(cnt = 0 ; cnt < size ; cnt++ ){

				val1 = 0xFFFF;
				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}

				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}
			}
		
			LcdcDisplayWindowFinish();

			x1 = x1;
			y1 = y1 + 80 - 1;
			x2 = x1 + 320 - 1;
			y2 = y1;

			LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, x1, y1, x2, y2);
		
			size = (x2 - x1 + 1) * (y2 - y1 + 1);
			for(cnt = 0 ; cnt < size ; cnt++ ){

				val1 = 0xFFFF;
				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}

				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}
			}
		
			LcdcDisplayWindowFinish();

			x1 = x1;
			y1 = y1 - 40;
			x2 = x1 + 40 - 1;
			y2 = y1;

			LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, x1, y1, x2, y2);
		
			size = (x2 - x1 + 1) * (y2 - y1 + 1);
			for(cnt = 0 ; cnt < size ; cnt++ ){

				if(g_TechMenuData.CmrCenter == 0)	val1 = 0x0000;
				else								val1 = 0xFFFF;
				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}

				if(g_TechMenuData.CmrCenter == 0)	val1 = 0xFF00;	//白色(入力なし)
				else								val1 = 0x0000;	//赤色(入力あり)
				rtn = LcdImageWrite(val1);	
				if(rtn != E_OK ){
					break;
				}
			}
		
			LcdcDisplayWindowFinish();
		}

		return;
#endif	
	}


	LcdcDisplayWindowFinish();
}


void SetGmnToLcdBuff02(int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	//num = buf;
	LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		val1 = GetPixDataFromInputGmn(gmn_num, sub_num1, sub_num2, cnt);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();
}

void SetGmnToLcdBuff03(int offset, int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	int st, imgx, imgy, ln_size, ln_num;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	//num = buf;
	LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	imgx = 40;
	imgy = 32;
	ln_size = 2 * (x2 - x1 + 1);
	ln_num = y2 - y1 + 1;

	for( y = 0 ; y < ln_num ; y++ ){
		st = y * imgx * 2 + 2 * offset;
		for(x = 0 ; x < ln_size ; x++ ){
			val1 = GetPixDataFromInputGmn(gmn_num, sub_num1, sub_num2, st);
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
			++st;
		}
	}
	
	LcdcDisplayWindowFinish();
}


void SetGmnToLcdBuff04(int buf_num, int gmn_num, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	//num = buf;
	LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		//val1 = GetPixDataFromInputGmn(gmn_num, sub_num1, sub_num2, cnt);
		val1 = DispFontNum[gmn_num][cnt];
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();
}


void SetGmnToLcdBuff05(int buf_num, int para, int sub_num1, int sub_num2, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	//num = buf;
	LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	
	if( sub_num1 == SELON ){
		for(cnt = 0 ; cnt < size ; cnt++ ){
			val1 = SelBtnOn[para][cnt];
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else{
		for(cnt = 0 ; cnt < size ; cnt++ ){
			val1 = SelBtnOff[para][cnt];
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}
	
	LcdcDisplayWindowFinish();
}



UH GetPixDataFromInputGmn(int buf, int sub_num1, int sub_num2, unsigned long cnt)
{
	UH val;
	
	switch(buf){
		case 0:
			val = InpGmnMigi[cnt];
			break;
		case 1:
			val = InpGmnHidari[cnt];
			break;
		case 2:
			val = InpGmnYubi01[cnt];
			break;
		case 3:
			val = InpGmnYubi02[cnt];
			break;
		case 4:
			val = InpGmnYubi03[cnt];
			break;
		case 5:
			val = InpGmnYubi04[cnt];
			break;
		case 6:
			//val = InpGmnYubi05[cnt];
			if( (cnt % 2) == 0 ){
				val = 0x00ff;
			}else{
				val = 0xff7f;
			}
			break;
		case 7:
			if( (cnt % 2) == 0 ){
				val = 0x0034;
			}else{
				val = 0x9ae8;
			}
			break;
		case GMN10KEYCOADE:
			if(sub_num1 == 10){
				if( (cnt % 2) == 0 ){
					val = 0x0034;
				}else{
					val = 0x9ae8;
				}
			}else{
				val = LcdGmnDataNum[sub_num1][cnt];
			}
			break;
		case GMN10KEYKIGOUCOADE:	//20140925Miya password open
			if(sub_num1 == 10){
				if( (cnt % 2) == 0 ){
					val = 0x0034;
				}else{
					val = 0x9ae8;
				}
			}else{
				val = LcdGmnDataKigou[sub_num1][cnt];
			}
			break;
		case GMN10KEYHIDECOADE:	//20140925Miya password open
			if(sub_num1 == 10){
				if( (cnt % 2) == 0 ){
					val = 0x0034;
				}else{
					val = 0x9ae8;
				}
			}else{
				val = LcdGmnDataX[sub_num2][cnt];
			}
			break;
		case GMNMKEYCOADE:	
			if(sub_num1 == MOJICODEDEL){
				if( (cnt % 2) == 0 ){
					val = 0x0034;
				}else{
					val = 0x9ae8;
				}
				break;
			}
			if(sub_num1 == MOJICODECSR){
				if( (cnt % 2) == 0 ){
					val = 0x00ff;
				}else{
					val = 0xffff;
				}
				break;
			}
		
			if(sub_num1 == MOJICODEAGYOU){
				val = LcdGmnDataAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEKAGYOU ){
				val = LcdGmnDataKAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODESAGYOU ){
				val = LcdGmnDataSAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODETAGYOU ){
				val = LcdGmnDataTAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODENAGYOU ){
				val = LcdGmnDataNAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEHAGYOU ){
				val = LcdGmnDataHAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEMAGYOU ){
				val = LcdGmnDataMAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEYAGYOU ){
				val = LcdGmnDataYAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODERAGYOU ){
				val = LcdGmnDataRAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEWAGYOU ){
				val = LcdGmnDataWAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEGAGYOU ){
				val = LcdGmnDataGAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEZAGYOU ){
				val = LcdGmnDataZAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEDAGYOU ){
				val = LcdGmnDataDAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEBAGYOU ){
				val = LcdGmnDataBAgyou[sub_num2][cnt];
			}else if(sub_num1 == MOJICODEPAGYOU ){
				val = LcdGmnDataPAgyou[sub_num2][cnt];
			}

			break;
		case GMNERRMESCODE:
			if(sub_num1 == 0){
				if( (cnt % 2) == 0 ){
					val = 0x0067;
				}else{
					val = 0x6767;
				}
			}else{
				val = LcdMes001[cnt];
			}
			break;
		case GMNKEYHEADCODE:
			if(sub_num1 == 1){
				val = LcdBoardMes01[cnt];
			}else if(sub_num1 == 2){
				val = LcdBoardMes02[cnt];
			}else if(sub_num1 == 3){
				val = LcdBoardMes03[cnt];
			}else if(sub_num1 == 4){
				val = LcdBoardMes04[cnt];
			}else if(sub_num1 == 5){
				val = LcdBoardMes05[cnt];
			}else if(sub_num1 == 6){
				val = LcdBoardMes06[cnt];
			}else if(sub_num1 == 7){
				val = LcdBoardMes07[cnt];
			}else if(sub_num1 == 8){
				val = LcdBoardMes08[cnt];
			}else if(sub_num1 == 9){
				val = LcdBoardMes09[cnt];
			}else if(sub_num1 == 10){
				val = LcdBoardMes10[cnt];
			}else if(sub_num1 == 11){
				val = LcdBoardMes11[cnt];
			}		
		
			break;
		case GMNNOBTNCODE:
			if(sub_num1 == 1){
				val = LcdMenuNoBtn01[cnt];
			}else if(sub_num1 == 2){
				val = LcdMenuNoBtn02[cnt];
			}else if(sub_num1 == 3){
				val = LcdMenuNoBtn03[cnt];
			}	
		
			break;
		default:
			val = 0x00;
			break;
	}
	
	return(val);
}



//20140925 Miya password open
void SetPassKeyBtn(int buf_num, int key_type, int randam)
{
	int i, x1, x2, y1, y2;
	int num;

	if(key_type == 2){	//20160112Miya FinKeyS 中止を消す
		x1 = PassBtnPos[20][0];
		y1 = PassBtnPos[20][1];
		x2 = PassBtnPos[21][0];
		y2 = PassBtnPos[21][1];

		num = 10;
		SetGmnToLcdBuffPassBtn(buf_num, num, key_type, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);

		return;
	}


	if(g_PasswordOpen.kigou_inp == FLG_ON){
		x1 = PassBtnPos[20][0];
		y1 = PassBtnPos[20][1];
		x2 = PassBtnPos[21][0];
		y2 = PassBtnPos[21][1];

		num = 10;
		SetGmnToLcdBuffPassBtn(buf_num, num, key_type, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);

		x1 = KeyHeadMesPosition[2][0];
		y1 = KeyHeadMesPosition[2][1];
		x2 = KeyHeadMesPosition[3][0];
		y2 = KeyHeadMesPosition[3][1];

		SetGmnToLcdBuffKeyHeadMesBanKi(buf_num, num, key_type, 0, 0, 0, 0, x1, y1, x2, y2);
		LcdcDisplayModeSet(buf_num, 0);

	}


	if(randam == 1){
		shuffle10( &g_key_arry[0] );
	}
	
	for(i = 0 ; i < 10 ; i++ ){
		x1 = PassBtnPos[2*i][0];
		y1 = PassBtnPos[2*i][1];
		x2 = PassBtnPos[2*i+1][0];
		y2 = PassBtnPos[2*i+1][1];

		num = g_key_arry[i];
		SetGmnToLcdBuffPassBtn(buf_num, num, key_type, 0, 0, 0, 0, x1, y1, x2, y2);

		LcdcDisplayModeSet(buf_num, 0);
	}
}

//20140925 Miya password open
void SetGmnToLcdBuffPassBtn(int buf_num, int key_num, int key_type, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	//num = buf;
	LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	for(cnt = 0 ; cnt < size ; cnt++ ){
		val1 = GetPixDataFromPassBtn(key_type, key_num, cnt);
		rtn = LcdImageWrite(val1);	
		if(rtn != E_OK ){
			break;
		}
	}
	
	LcdcDisplayWindowFinish();
}

//20140925 Miya password open
UH GetPixDataFromPassBtn(int type, int num, unsigned long cnt)
{
	UH val;

	if( type == 0 ){
		val = LcdKeyTopNum[num][cnt];
	}else if( type == 1 ){
		val = LcdKeyTopKigou[num][cnt];
	}else{
		val = 0x6767;
	}

	return(val);
}


//20140925 Miya password open
void SetGmnToLcdBuffKeyHeadMesBanKi(int buf_num, int key_num, int key_type, UB perm, UB perm_r, UB perm_g, UB perm_b, UH stx, UH sty, UH edx, UH edy)
{
	ER rtn;
	int x, y, x1, x2, y1, y2, size;
	UH val1, num;
	//UH val2;
	unsigned long cnt;

	//num = buf;
	LcdcDisplayWindowSet(buf_num, perm, perm_r, perm_g, perm_b, stx, sty, edx, edy);
	
	x1 = stx;
	x2 = edx;
	y1 = sty;
	y2 = edy;

	size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
	if( key_type == 0 ){
		for(cnt = 0 ; cnt < size ; cnt++ ){
			val1 = LcdKeyTopMesBanKi[0][cnt];
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}else{
		for(cnt = 0 ; cnt < size ; cnt++ ){
			val1 = LcdKeyTopMesBanKi[1][cnt];
			rtn = LcdImageWrite(val1);	
			if(rtn != E_OK ){
				break;
			}
		}
	}
	
	LcdcDisplayWindowFinish();
}

UB WaitKeyProc(int sousa, int tflg, UB *msg)
{
	UB	rtn=0;
	UB	rtnsts;
	ER	ercd;
	int posx, posy;
	int x1, x2, y1, y2;
	int tcnt;
	int dbg_cnt;
	UB *buf;

	buf = msg;
	tcnt = 0;
	dbg_cnt = 0;
	while(1){
		ercd = TplPosGet(&posx, &posy, (500/MSEC));	//キー待ち500msec
		
		lcd_TSK_wdt = FLG_ON;			// LCDタスク　WDTリセット・リクエスト・フラグON。 Added T.N 2015.3.10
		
		if(	ercd == E_OK ){
			dly_tsk((500/MSEC));	//500msecのウエイト(タッチキーの反応解除用)
			rtn = 1;	//CHG_REQ要求

			if(tflg == 2){	//画面Sleep状態の場合(LCDバックライトをOFFにしている)
				LcdcBackLightOn();
				tflg = 1;
				tcnt = 0;
			}else{
				if( sousa == 99 ){
					x1 = 208;
					x2 = 272;
					y1 = 104;
					y2 = 167;
					if( (posx > x1 && posx < x2) && (posy > y1 && posy < y2) ){
						break;
					}
				}else{
					break;
				}
			}
			
		}

		if(tflg == 1){
			++tcnt;
			if(tcnt >= 120){	//画面Sleep状態にする
				LcdcBackLightOff();
				tflg = 2;
				tcnt = 0;
			}
		}

		rtnsts = MdGetMode();
		if( rtnsts == MD_CAP ){
			rtn = 2;	//CHG_REQなし
			LcdcBackLightOn();
			break;
		}
	}

	return(rtn);
}


UH GetPixDataFromGamen400(int buf, unsigned long cnt)
{
	UH val;

	switch(buf){
		case 400:
			val = LcdGmn000[cnt];
			break;
		case 401:
			val = LcdGmn101[cnt];
			break;
		case 402:
			val = LcdGmn001[cnt];
			break;
		case 403:
			val = LcdGmn002[cnt];
			break;
		case 404:
			val = LcdGmn107[cnt];
			break;
		case 405:
			val = LcdGmn201[cnt];
			break;
		case 406:
			val = LcdGmn202[cnt];
			break;
		case 407:
			val = LcdGmn203[cnt];
			break;
		case 408:
			val = LcdGmn204[cnt];
			break;
		case 409:
			val = LcdGmn206[cnt];
			break;
		case 410:
			val = LcdGmn301[cnt];
			break;
		case 411:
			val = LcdGmn302[cnt];
			break;
		default:
			val = 0x00;
			break;
	}
	return(val);
}

UH GetPixDataFromGamen500(int buf, unsigned long cnt)
{
	UH val;

	switch(buf){
		case 500:
			val = 0x00;
			break;
		case 501:
			val = LcdGmn005[cnt];
			break;
		case 502:
			val = 0x00;
			break;
		case 503:
			val = LcdGmn201[cnt];
			break;
		case 504:
			val = LcdGmn205[cnt];
			break;
		case 505:
			val = LcdGmn206[cnt];
			break;
		case 506:
			val = LcdGmn304[cnt];
			break;
		case 520:
			val = 0x00;
			break;
		case 521:
			val = LcdGmn111[cnt];
			break;
		case 522:
			val = LcdGmn001[cnt];
			break;
		case 523:
			val = LcdGmn201[cnt];
			break;
		case 524:
			val = LcdGmn205[cnt];
			break;
		case 525:
			val = LcdGmn206[cnt];
			break;
		case 526:
			val = LcdGmn112[cnt];
			break;
		case 527:
			val = LcdGmn001[cnt];
			break;
		case 528:
			val = LcdGmn002[cnt];
			break;
		case 529:
			val = LcdGmn107[cnt];
			break;
		case 530:
			val = LcdGmn201[cnt];
			break;
		case 531:
			val = LcdGmn202[cnt];
			break;
		case 532:
			val = LcdGmn203[cnt];
			break;
		case 533:
			val = LcdGmn204[cnt];
			break;
		case 534:
			val = LcdGmn206[cnt];
			break;
		case 535:
			val = LcdGmn306[cnt];
			break;
		case 536:
			val = LcdGmn304[cnt];
			break;
		case 537:
			val = LcdGmn305[cnt];
			break;
		case 538:
			val = LcdGmn302[cnt];
			break;
//		case 542:
//			val = LcdGmn112[cnt];
//			break;
		case 543:
			val = LcdGmn001[cnt];
			break;
		case 544:
			val = LcdGmn201[cnt];
			break;
		case 545:
			val = LcdGmn205[cnt];
			break;
		case 546:
			val = LcdGmn206[cnt];
			break;
		case 547:
			val = LcdGmn001[cnt];
			break;
		case 548:
			val = LcdGmn303[cnt];
			break;
		case 549:
			val = LcdGmn304[cnt];
			break;
		case 550:
			val = LcdGmn305[cnt];
			break;
		case 551:
			val = LcdGmn302[cnt];
			break;
		default:
			val = 0x00;
			break;
	}
	return(val);
}


void shuffle10( int *key)
{
	int i, x, y;
	int *buf;

	buf = key;
	
	for( i = 0 ; i < 10 ; i++ ){
		x = rand() % 10;
		y = *(buf + i);
		*(buf + i) = *(buf + x);
		*(buf + x) = y;
	}
}



