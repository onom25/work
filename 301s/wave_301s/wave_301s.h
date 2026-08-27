//
//
#ifndef __wave_301s__
#define	__wave_301s__


//mmp5500\src\include\fpga.h 内と共通宣言
typedef struct struct_wave {
	union {
		unsigned short WORD;
		struct {
			// TOCONVアクセス許可
			unsigned short DAC	: 10;	// DAC出力値
			unsigned short LAT	: 1;
			unsigned short CH	: 1;
			unsigned short NCHG	: 1;
			unsigned short DTS	: 1;
			unsigned short		: 1;	// 未使用
			unsigned short ENB	: 1;	// 1:有効 0:無効
		} BIT;
	} WAVE_MEM;
} struct_wave_t;



typedef struct st_wave301S {
	
	struct struct_wave wave[8192];	// FPGA波形メモリ (DMX100 0x0500_8000番地)

	unsigned long WAVE001;			// 波形0先頭 (DMX100 0x0500_0044番地)
	unsigned long WAVE010;			// 波形1先頭 (DMX100 0x0500_0048番地)
	unsigned long WAVE011;			// 波形2先頭 (DMX100 0x0500_004c番地)
	unsigned long WAVE100;			// 波形3先頭 (DMX100 0x0500_0050番地)
	unsigned long WAVE101;			// 波形4先頭 (DMX100 0x0500_0054番地)
	unsigned long WAVE110;			// 波形5先頭 (DMX100 0x0500_0058番地)
	unsigned long WAVE111;			// 波形6先頭 (DMX100 0x0500_005c番地)
	// ポインタだけれど マイコンが32bitなので変数とする
	unsigned short temprature_head;			// ヘッド温度
	unsigned short temprature_heatsink;		// 放熱版温度
	unsigned long wave_num;			// 波形番号 (DMX100 パラメータ0x2000番地)
	unsigned char rank[60];			// ランク   (DMX100 パラメータ0x2004番地)
} st_wave301S_t;


#endif	//__wave_301s__
