/**
 * @file  Search.c
 * @brief の動作を司るクラス
 */

/*----------------------------------------------------------------------
	Includes
 ----------------------------------------------------------------------*/
#include "Search.h"
#include "../iodefine.h"
#include "../Global.h"
#include "MouseController.h"
#include "../Devices/LightSensor.h"


//----現在地格納共用・構造体----
volatile union map_coor{
	_UBYTE PLANE;		//YX座標
	struct coor_axis{
		_UBYTE Y:4;		//Y座標
		_UBYTE X:4;		//X座標
	}AXIS;
}PRELOC;

_UBYTE map[16][16];		// マップ格納配列
_UBYTE smap[16][16];		// 歩数マップ格納配列
_UBYTE wallInfo;			// 壁情報格納変数
_UBYTE mDir;				// マウスの方向
_UBYTE mStep;			// 歩数格納
_UBYTE route[256];		// 最短経路格納
_UBYTE routeCnt;			// 経路カウンタ

_UBYTE stopFlag;			// 走行中断用フラグ
int count;				// 何回曲がったかをカウント

/*----------------------------------------------------------------------
	Private Method Declarations
 ----------------------------------------------------------------------*/


/*----------------------------------------------------------------------
	Public Method Definitions
 ----------------------------------------------------------------------*/
/*===========================================================
		探索系関数
===========================================================*/
/*-----------------------------------------------------------
		初期化
-----------------------------------------------------------*/
void Search_Init()
{
	// ---- 探索系 ----
	Search_MapInit();				//マップの初期化
	PRELOC.PLANE = 0x00;	//現在地の初期化
	Search_SetDir(DIR_TURN_0);		//マウス方向の初期化
	stopFlag = 0;			//走行中断用フラグの初期化
}

/*-----------------------------------------------------------
	足立法探索走行
-----------------------------------------------------------*/
void Search_Adachi()
{
	// ==== 歩数等初期化 ====
	count = 0;				// 曲がりカウンタの初期化
	mStep = routeCnt = 0;	// 歩数の初期化
	Search_GetWallInfo();			// 壁情報の初期化
	Search_WriteMap();				// 地図の初期化
	Search_MakeStepMap();			// 歩数図の初期化
	Search_MakeRoute();			// 最短経路探索

	// ==== 探索走行 ====
	do{
		// ボタンで途中停止
		if( GetSwitchState() ){
			stopFlag = 1;
			HalfSectionD();
			break;
		}

		// ---- 進行 ----
		switch( route[routeCnt++] ){
			// ---- 前進 ----
			case 0x88:
				HalfSectionA();		//半区画加速前進
				break;

			// ---- 右折 ----
			case 0x44:
				TurnR90AD();
				// 規定数回以上曲がっていて かつ 曲がった後に後ろ壁がある時
				if((count >= ADJUST_NUM) && (wallInfo & 0x11)){
					if( wallInfo & 0x11 ){
						SetPosition();			// 曲がった後に縦位置補正
						count = 0;					// 曲がりカウンタのリセット
					}
				}
				// 位置補正しない時
				else count++;						// 曲がりカウンタのインクリメント
				HalfSectionA();					// 半区画加速前進

				Search_TurnDir(DIR_TURN_R90);
				break;

			// ---- 180回転 ----
			case 0x22:
          		TurnR180AD();
        		if(wallInfo & 0x88){			// 180回転後に後ろ壁があるとき位置補正を行う
					SetPosition();
					count = 0;					// 曲がりカウンタのリセット
				}
				HalfSectionA();
				Search_TurnDir(DIR_TURN_180);
				break;

			// ---- 左折 ----
			case 0x11:
				TurnL90AD();
				// 規定数回以上曲がっていて かつ 曲がった後に後ろ壁がある時
				if( (count >= ADJUST_NUM) && (wallInfo & 0x44)){
					if( wallInfo & 0x44 ){
						SetPosition();					// 曲がった後に縦位置補正
						count = 0;							// 曲がりカウンタのリセット
					}
				}
				// 位置補正しない時
				else count++;							// 曲がりカウンタのインクリメント

				HalfSectionA();					// 半区画前進

				Search_TurnDir(DIR_TURN_L90);
				break;

			// 進行方向がないとき（ゴールに至るルートがないとき）
			default:
				PlaySound(800);
				PlaySound(600);
				PlaySound(400);
				PlaySound(200);
				PlaySound(400);
				WaitMS(1500);
				return;			// 関数から抜ける
				break;
		}
		Search_GetWallInfo();						// 壁情報の取得
		Search_AdvancePosition();					// 位置情報前進
		Search_ConfirmRoute();						// 進路判定

		// ---- 連続走行を続行するか判定 ----
		// 進行ルートが前なら加速し走行続行
		if( route[routeCnt] & 0x88 ){
			HalfSectionA();
		}
		// 進行ルートが右or左のとき
		else if( (route[routeCnt] & 0x44) || (route[routeCnt] & 0x11) ){
			HalfSectionD();			//減速し停止
		}
    	// 進行ルートが後ろなら減速し停止
		else{
			HalfSectionD();
		}

	}while( (PRELOC.AXIS.X != GOAL_X) || (PRELOC.AXIS.Y != GOAL_Y) );

	// ==== ゴール後の処理 ====
  	if(stopFlag != 1) {
//		MF.STATE.BIT.GOAL_ = 1;
		WaitMS(1500);
	}
	stopFlag = 0;
}

/*-----------------------------------------------------------
		位置情報前進
-----------------------------------------------------------*/
void Search_AdvancePosition()
{
	switch(mDir){
		case 0x00:
			(PRELOC.AXIS.Y)++;
			break;
		case 0x01:
			(PRELOC.AXIS.X)++;
			break;
		case 0x02:
			(PRELOC.AXIS.Y)--;
			break;
		case 0x03:
			(PRELOC.AXIS.X)--;
			break;
	}
}

/*------------------------------------------------------------
		壁情報取得
------------------------------------------------------------*/
void Search_GetWallInfo()
{
	_UBYTE ledPattern;

	// ---- 壁情報の初期化 ----
	wallInfo = 0x00;
	ledPattern = 0x00;

	LSVal *lsv = LightSensor_GetValue();


	// ---- 前壁を見る ----
	if( lsv->Now.FwdL > WALL_BASE_FWD_L ){
		wallInfo |= 0x88;
		ledPattern |= 0x06;
	}

	// ---- 右壁を見る ----
	if( lsv->Now.Right > WALL_BASE_RIGHT ){
		wallInfo |= 0x44;
		ledPattern |= 0x01;
	}

	// ---- 左壁を見る ----
	if( lsv->Now.Left > WALL_BASE_LEFT ){
		wallInfo |= 0x11;
		ledPattern |= 0x08;
	}
	DispLED(ledPattern);
}

/*-----------------------------------------------------------
		進路判定
-----------------------------------------------------------*/
void Search_ConfirmRoute()
{
	// ---- 壁情報書き込み ----
	Search_WriteMap();

	// ---- 最短経路上に壁があれば進路変更 ----
	if(wallInfo & route[routeCnt]){
		Search_MakeStepMap();			// 歩数マップ作成
		Search_MakeRoute();			// 最短経路作成
		routeCnt = 0;
	}
}
/*-----------------------------------------------------------
		マップデータ初期化
-----------------------------------------------------------*/
void Search_MapInit()
{
	// ==== 変数宣言 ====
	_UBYTE x, y;

	// ==== 初期化開始 ====
	for( y = 0; y <= 0x0f; y++ ){
		for(x = 0; x <= 0x0f; x++){
			map[y][x] = 0xf0;		// 上位を壁、下位を壁なしとする。
		}
	}
	for( y = 0; y <= 0x0f; y++ ){
		map[y][0] |= 0xf1;
		map[y][15] |= 0xf4;
	}
	for( x = 0; x <= 0x0f; x++ ){
		map[0][x] |= 0xf2;
		map[15][x] |= 0xf8;
	}
}
/*-----------------------------------------------------------
		マップデータ書き込み
-----------------------------------------------------------*/
void Search_WriteMap()
{
	// ==== 変数宣言 ====
	_UBYTE mTemp;

	// ==== 壁情報の補正格納 ====
	mTemp = (wallInfo >> mDir) & 0x0f;
	mTemp |= (mTemp << 4);					// この作業でmTempにNESW順で壁が格納

	// ==== データの書き込み ====
	map[PRELOC.AXIS.Y][PRELOC.AXIS.X] = mTemp; // 現在地に書き込み
	// ---- 周辺に書き込む ----
	if(PRELOC.AXIS.Y != 15){				// 北側について(現在最北端でないとき)
		if(mTemp & 0x88){		//北壁がある場合
			//北側の区画から見て南壁書き込み
			map[PRELOC.AXIS.Y + 1][PRELOC.AXIS.X] |= 0x22;
		}else{					//ない場合
			//北側の区画から見て南壁なし
			map[PRELOC.AXIS.Y + 1][PRELOC.AXIS.X] &= 0xDD;
		}
	}
	if(PRELOC.AXIS.X != 15){	//東側について(現在最東端でないとき)
		if(mTemp & 0x44){		//東壁があるとき
			//東側の区画から見て西壁存在を書き込む
			map[PRELOC.AXIS.Y][PRELOC.AXIS.X + 1] |= 0x11;
		}else{					//壁が存在しないとき
			//東側の区画から見て西壁なしを書き込む
			map[PRELOC.AXIS.Y][PRELOC.AXIS.X + 1] &= 0xEE;
		}
	}
	if(PRELOC.AXIS.Y != 0){		//南壁について(現在最南端でないとき)
		if(mTemp & 0x22){		//南壁があるとき
			//南側から見て北壁を書き込む
			map[PRELOC.AXIS.Y - 1][PRELOC.AXIS.X] |= 0x88;
		}else{					//南壁がないとき
			//南側から見て北壁なしを書き込む
			map[PRELOC.AXIS.Y - 1][PRELOC.AXIS.X] &= 0x77;
		}
	}
	if(PRELOC.AXIS.X != 0){		//西側について(現在最西端でないとき)
		if(mTemp & 0x11){		//西壁があるとき
			//西側から見て東壁の存在を書き込む
			map[PRELOC.AXIS.Y][PRELOC.AXIS.X - 1] |= 0x44;
		}else{
			//西側から見て東側に壁をなしとする
			map[PRELOC.AXIS.Y][PRELOC.AXIS.X - 1] &= 0xBB;
		}
	}
}

/*-----------------------------------------------------------
		マウスの方向を変更(定数で直接やった方がいいか？)
-----------------------------------------------------------*/
void Search_TurnDir(_UBYTE t_pat)
{
	//====方向を変更====
	mDir = (mDir + t_pat) & 0x03;
}

/*-----------------------------------------------------------
		マウスの方向を直接設定
-----------------------------------------------------------*/
void Search_SetDir(_UBYTE t_pat)
{
	//====方向を変更====
	mDir = t_pat & 0x03;
}

/*-----------------------------------------------------------
		歩数マップ作成
-----------------------------------------------------------*/
void Search_MakeStepMap()
{
	//====変数宣言====
	_UBYTE x, y;		//マップ用カウンタ
	_UBYTE mTemp;	//マップデータ一時保持

	//====歩数マップのクリア====
	for(y = 0; y <= 0x0f; y++){
		for( x = 0; x <= 0x0f; x++){
			smap[y][x] = 0xff;		//歩数最大とする
		}
	}

	//====ゴール座標を0にする====
	smap[GOAL_Y][GOAL_X] = 0;

	//====歩数カウンタを0にする====
	mStep = 0;

	//====自分の座標にたどり着くまでループ====
	do{
		//----マップ全域を捜索----
		for( y = 0; y <= 0x0f; y++){
			for( x = 0; x <= 0x0f; x++){
				//----現在最大の歩数を発見したとき----
				if( smap[y][x] == mStep){
					mTemp = map[y][x];
//					if( _MF.STATE.BIT.SCND ){	//二次走行用マップを作るときは1にする
//						mTemp >>= 4;			//4bitシフトさせる
//					}
					//----北壁がなく現在最北端でないとき----
					if(!(mTemp & 0x08) && y != 0x0f){
						if(smap[y+1][x] == 0xff){		//北側がクリア状態なら
							smap[y+1][x] = mStep + 1;	//次の歩数を書き込む
						}
					}
					//----東壁についての処理----
					if(!(mTemp & 0x04) && x != 0x0f){
						if(smap[y][x+1] == 0xff){
							smap[y][x+1] = mStep + 1;
						}
					}
					//----南壁についての処理----
					if(!(mTemp & 0x02) && y != 0){
						if(smap[y-1][x] == 0xff){
							smap[y-1][x] = mStep + 1;
						}
					}
					//----西壁についての処理----
					if(!(mTemp & 0x01) && x != 0){
						if(smap[y][x-1] == 0xff){
							smap[y][x-1] = mStep + 1;
						}
					}
				}
			}
		}
		//====歩数カウンタのインクリメント====
		mStep++;
	}while(smap[PRELOC.AXIS.Y][PRELOC.AXIS.X] == 0xff);
}

/*-----------------------------------------------------------
		最短経路導出
-----------------------------------------------------------*/
void Search_MakeRoute()
{
	// ==== 変数宣言 ====
	_UBYTE i = 0;				// カウンタ
	_UBYTE x, y;
	_UBYTE dirTemp =  mDir;		// 方向変数
	_UBYTE mTemp;

	// ==== 最短経路を初期化 ====
	do{
		route[i++] = 0xff;
	}while(i != 0);

	// ==== 歩数カウンタをセット ====
	mStep = smap[PRELOC.AXIS.Y][PRELOC.AXIS.X];

	// ==== x, yに現在座標を書き込み ====
	x = (_UBYTE)PRELOC.AXIS.X;
	y = (_UBYTE)PRELOC.AXIS.Y;

	// ==== 最短経路を導出 ====
	do{
		mTemp = map[y][x];			//比較用マップ情報の格納
//		if( MF.STATE.BIT.SCND ){	//二次走行時はここをTrueにする
//			mTemp >>= 4;
//		}
		// ---- 北を見る ----
		if(!(mTemp & 0x08) && (smap[y+1][x] < mStep)){
			route[i] = (0x00 - mDir) & 0x03;
			mStep = smap[y+1][x];
			y++;
		}
		// ---- 東を見る ----
		else if(!(mTemp & 0x04) && (smap[y][x+1] < mStep)){
			route[i] = (0x01 - mDir) & 0x03;
			mStep = smap[y][x+1];
			x++;
		}
		// ---- 南を見る ----
		else if(!(mTemp & 0x02) && (smap[y-1][x] < mStep)){
			route[i] = (0x02 - mDir) & 0x03;
			mStep = smap[y-1][x];
			y--;
		}
		// ---- 西を見る ----
		else if(!(mTemp & 0x01) && (smap[y][x-1] < mStep)){
			route[i] = (0x03 - mDir) & 0x03;
			mStep = smap[y][x-1];
			x--;
		}

		// ---- 格納データ形式変更 ----
		switch(route[i]){
			case 0x00:
				route[i] = 0x88;
				break;
			case 0x01:
				Search_TurnDir(DIR_TURN_R90);
				route[i] = 0x44;
				break;
			case 0x02:
				Search_TurnDir(DIR_TURN_180);
				route[i] = 0x22;
				break;
			case 0x03:
				Search_TurnDir(DIR_TURN_L90);
				route[i] = 0x11;
				break;
			default:
				route[i] = 0x00;
				break;
		}
		i++;
	}while( smap[y][x] != 0);

	mDir = dirTemp;
}

/*----------------------------------------------------------------------
	Private Method Definitions
 ----------------------------------------------------------------------*/

