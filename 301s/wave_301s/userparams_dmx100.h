/*
 * userparams.h
 *
 *  Created on: 2020/05/14
 *      Author: miya
 */

#ifndef INCLUDE_USERPARAMS_H_
#define INCLUDE_USERPARAMS_H_

//#include "thermistor.h"		// tki_1

/* 書き込みは 128Byte単位でしかできない為パラメータ追加の際は構造体サイズを調整すること。 */
typedef struct userparams {
	// 0x0000	様々なパラメーター
	unsigned int param0;					//0x00	// ヘッダ @onomc0 FPGA バージョンが反映される。
	unsigned int param1;							// テーブルリビジョン
	unsigned int machine_id;						// 機種識別子
		#define MACHID_BACKDOOR_LIMIT		0x00000001		//@onome2 mmp5500util のバックドアアクセスを制限する
		#define MACHID_RLE_PASS				0x00000002		//@onomf2 パス単位RLEを使用する。 通常をライン単位RLEとする。通常機体と共用できない。
		#define MACHID_CLFLPOS				0x00000004		//@onomf7 混色防止フラッシング時位置変更。
		 													// フラッシング位置がホームポジション以外では使用できない。
	unsigned int board_serial;						// シリアル番号
	unsigned int ipaddr;					//0x10	// IPアドレス
	unsigned int netmask;							// ネットマスク
	unsigned int gwaddr;							// ゲートウェイアドレス
	unsigned int flashing_count_cleaning;			// クリーニング時フラッシングカウント
	unsigned int flashing_count_printing;	//0x20	// 印刷時フラッシングカウント
	unsigned int flashing_interval;					// 印刷時フラッシングインターバル（秒）
	unsigned int cap_ink_purge_count;				// キャップ内インクパージカウント
	unsigned int capping_timer;						// ヘッド乾燥防止タイマー
	unsigned int loglevel;					//0x30	// ログレベル
	char logfilename[128];					//0x34	// ログメッセージを出力するデバイスまたはファイル名
	unsigned int flashing_sps;				//0xb4	//onom8 フラッシング吐出周波数
	unsigned int cl_timer;					//0xb8	// ワイプからフラッシングまでの遅延時間（秒）
	unsigned int t1_threshold;						// T1強制クリーニング閾値（秒）
	unsigned int t4_threshold;				//0xc0	// T4強制クリーニング閾値（秒）
	unsigned int rocking_count;						// 印字外微振動カウント
	unsigned int cleaning_vacum;					//onomb4 クリーニング吸引量(パルス数)
	unsigned int rocking_prepare;					//onomb4 印刷前印字外微振動数
	unsigned int flashing_count_prepare;	//0xd0	//onomb4 印刷前フラッシングカウント
	unsigned int count_rocking_outofarea;			//onomd1 印字外微振動発数
	unsigned int flashing_count_suspension;			//onomd4d 休止前まとめ打ちフラッシング発数（上限）
	unsigned int rocking_count_suspension;			//onomd4d 休止前まとめ打ち印字外微振動発数
	unsigned int flashing_offset_suspension;//0xe0	//onomf6 20241217
	unsigned char reserved1[0x400-sizeof (int)*57];	//onomf6 20241217 onomb4 onomd1 onomd4d

	// 0x0400	統計情報パラメーター
	unsigned long long cr_active_time;		//0x400	// CRモーター駆動時間
	unsigned long long pf_active_time;				// PFモーター駆動時間
	unsigned long long pump_active_time;			// ポンプモーター駆動時間
//	unsigned int print_count;						// 累積印刷枚数
	unsigned long long energizing_time;		//0x410	// 累積通電時間
	unsigned int params_update_period;				// ユーザーパラメーター書き戻し間隔時間(秒)

	unsigned char reserved2[0x400-sizeof (int)*10];

	// 0x0800	CR軸パラメーター
	int cr_limit_value;						// CR可動範囲の最大値
	int cr_print_region_left;				// CR印刷領域の左端座標
	int cr_print_region_width;				// CR印刷領域幅
	int cr_pre_run;							// CR助走距離
	int cr_post_run;						// CR惰走距離
	int cr_flashing_pos;					// フラッシング位置
	int cr_max_dot_frequency;				// 最大吐出周波数
	int cr_trigger_adjust_reverse;			// 双方向印刷戻りパストリガー位置調整値
	int cr_nozzle_offset0;					// カラー#0吐出遅延
	int cr_nozzle_offset1;					// カラー#1吐出遅延
	int cr_nozzle_offset2;					// カラー#2吐出遅延
	int cr_nozzle_offset3;					// カラー#3吐出遅延
	int cr_nozzle_offset4;					// カラー#4吐出遅延
	int cr_nozzle_offset5;					// カラー#5吐出遅延
	int cr_nozzle_offset6;					// カラー#6吐出遅延
	int cr_nozzle_offset7;					// カラー#7吐出遅延
	unsigned char reserved3[0x400-sizeof (int)*16];

	// 0x0c00	PF軸パラメーター
	int pf_top_margin;						// PF軸先頭マージン(1/720in)
	int pf_max_print_length;				// 最大印刷長(1/720in)
	unsigned char reserved4[0x400-sizeof (int)*2];

	// 0x1000	ポンプキャップユニットパラーメーター
	unsigned char reserved5[0x400-sizeof (int)*0];

	// 0x1400	未使用
	unsigned char reserved6[0xc00-sizeof (int)*0];

	// 0x2000	波形データ
	int	wave_number;						// 使用する波形番号
	unsigned char head_id_data[100];		// Head ID データ(ASCII で記録。最大100文字)	// 100 byte

	//ヘッド温度の上限、下限
	int	head_temp_max;					// (1/13)
	int head_temp_min;					// (2/13)

	//ヒートシンクの上限、下限、上側警告
	int heat_sink_max;					// (3/13)
	int heat_sink_min;					// (4/13)

	//ヘッド温度上限値ディレイ（休止時間 ms）
	int head_temp_delay;				// (5/13)

	//ヒートシンク上側警告ディレイ (休止時間 ms)
	int heat_sink_delay;				// (6/13)

	//サーミスタ関連
	int temp_thermistor_divider;		// (7/13)
	int temp_def_head_tilt;				// (8/13)
	int temp_def_heatsink_tilt;			// (9/13)
	int temp_def_head_offset;			// (10/13)
	int temp_def_heatsink_offset;		// (11/13)
	int temp_def_head_timeconst;		// (12/13)
	int temp_def_heatsink_timeconst;	// (13/13)

	// タイマー関連
	unsigned int t1_timer;				// 通電中タイマー（秒）
	unsigned int t4_timer;				// 休止タイマー（秒）
	unsigned int t2_timer;				//onomd4e
	unsigned char reserved7[0x1ff8 - sizeof(int) - 100 - sizeof (int)*13 - sizeof (int) * 3];	//onod4e

	// 0x3ff8	チェックサム
	unsigned int checksum;

	// 0x3ffc	制御フラグ
	union {								// +00
		unsigned long WORD;
		struct {
			unsigned long WR_APPLI : 1;		// 0:アプリケーション書き換え中フラグ
			unsigned long WR_PARAM : 1;		// 0:パラメータ書き換え中フラグ
			unsigned long WR_bmrx : 1;		// 0:boot monitor書き換え中フラグ
			unsigned long : 29;				//
		} BIT;
	} FLAG_PROGRAM_BUSY;
} userparams_t;

#define	SIZE_OF_USERPARAMS		16384


unsigned int userparams_checksum_calc (userparams_t* params);
int userparams_reset_to_default ();
int userparams_update ();
int userparams_init ();
void userparams_set_dirty ();
int userparams_is_dirty ();

extern userparams_t* userparams;

#define	LOGFILENAME				(userparams->logfilename)

#endif /* INCLUDE_USERPARAMS_H_ */
