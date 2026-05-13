/*
2026/04/14	FT232H 高速転送プロトタイプ
written by omon.mastermind@gmail.com
*/

`define	_TIMECONST	16'd18		// 時間生成定数


// デバック用信号の生成
module detect_uisw (
	CLK, nRES,
	UISW1, UISW2,
	TRG1, TRG2, TRG1ON, TRG2ON
);
	input				CLK;
	input				nRES;
	input				UISW1;
	input				UISW2;
	output	reg			TRG1;
	output	reg			TRG2;
	output				TRG1ON;
	output				TRG2ON;
			reg	[15:0]	cnt;
			reg	[1:0]	SYSW1;
			reg	[1:0]	SYSW2;
			reg	[1:0]	TRG1SFR;
			reg [1:0]	TRG2SFR;
			reg			SW1FLG;
			reg			SW2FLG;	// 2スイッチ共OFF検知済

	// スイッチ信号同期とTRG1の微分
	always @(posedge CLK or negedge nRES) begin
		if (!nRES) begin
			SYSW1	<= 2'b11;
			SYSW2	<= 2'b11;
			TRG1SFR	<= 2'b00;
			TRG2SFR	<= 2'b00;
			end
		else begin
			SYSW1[1]	<= SYSW1[0];
			SYSW1[0]	<= UISW1;
			SYSW2[1]	<= SYSW2[0];
			SYSW2[0]	<= UISW2;
			TRG1SFR[1]	<= TRG1SFR[0];
			TRG1SFR[0]	<= TRG1;
			TRG2SFR[1]	<= TRG2SFR[0];
			TRG2SFR[0]	<= TRG2;
			end
	end

	// 時間生成ダウンカウント
	always @(posedge CLK or negedge nRES) begin
		if (!nRES)	cnt		<= 16'h0;
		else if ((TRG1SFR == 2'b01) & (cnt == 16'h0))
					cnt		<= `_TIMECONST;
		else if (cnt != 16'h0)
					cnt		<= cnt - 16'h1;
		else		cnt		<= cnt;
	end

	// TRG1信号の生成
	// チャタリング除去用 SW1FLG 検出
	always @(posedge CLK or negedge nRES) begin
		if (!nRES)		SW1FLG	<= 1'b1;
		else if ((SYSW1 == 2'b10) & (SYSW2 == 2'b00))	//TRG1条件
						SW1FLG	<= 1'b0;
		else if ((SW1FLG == 1'b0) & (SYSW2 == 2'b11) & (SYSW1 == 2'b11))
						SW1FLG	<= 1'b1;
		else			SW1FLG	<= SW1FLG;
	end
	// トリガ
	always @(posedge CLK or negedge nRES) begin
		if (!nRES)	TRG1	<= 1'b0;
		else if ((SW1FLG==1'b1) & (SYSW1==2'b10) & (SYSW2==2'b00))		// SW2 ON状態のSW1押下でトリガ
					TRG1	<= 1'b1;
		else if ((TRG1 == 1'b1) & (cnt == 16'h0001))
					TRG1	<= 1'b0;
		else		TRG1	<= TRG1;
	end
	// TRG1ON
	assign TRG1ON = (TRG1SFR==2'b01) ? 1'b1 : 1'b0;

	// TRG2信号生成
	// チャタリング除去用 SW2FLG 検出
	always @(posedge CLK or negedge nRES) begin
		if (!nRES)		SW2FLG	<= 1'b1;
		else if ((SYSW2 == 2'b10) & (SYSW1 == 2'b00))	//TRG2条件
						SW2FLG	<= 1'b0;
		else if ((SW2FLG == 1'b0) & (SYSW2 == 2'b11) & (SYSW1 == 2'b11))
						SW2FLG	<= 1'b1;
		else			SW2FLG	<= SW2FLG;
	end
	// トリガ
	always @(posedge CLK or negedge nRES) begin
		if (!nRES)	TRG2	<= 1'b0;
		else if ((SW2FLG==1'b1) & (SYSW2==2'b10) & (SYSW1==2'b00))		// SW1 ON状態のSW1押下でトリガトグル
					TRG2	<=~TRG2;
		else		TRG2	<= TRG2;
	end
	assign TRG2ON = (TRG2SFR==2'b01) ? 1'b1 : 1'b0;
endmodule



module mode_SyncFIFO(
	CLK, nRES,
	nRXF, nTXE, Din, UISW1, UISW2,
	Dout, nRD, nWR, nOE, wrOE, nPOWSAV, nSIWU,
	TP1, TP2
);

	input				CLK;		// CLK
	input				nRES;		// RESET
	input				nRXF;		// data available in the FIFO
	input				nTXE;		// data can be written into the FIFO
	input		[7:0]	Din;		// Read data bus
	input				UISW1;
	input				UISW2;

	output	reg	[7:0]	Dout;		// Write data bus
	output	reg			nRD;		// Enables the current FIFO
	output	reg			nWR;		// to be written into the transmit FIFO
	output	reg			nOE;
	output	reg			wrOE;
	output				nPOWSAV;
	output				nSIWU;
	output				TP1;
	output				TP2;
			reg	[4:0]	state;
			wire		TRG1, TRG2, TRG1ON, TRG2ON;
			reg [15:0]	letancycount;	// 転送中のレイテンシー変化観測用

	detect_uisw i_detect_uisw (
		.CLK(CLK), .nRES(nRES),
		.UISW1(UISW1), .UISW2(UISW2), .TRG1(TRG1), .TRG2(TRG2), .TRG1ON(TRG1ON), .TRG2ON(TRG2ON)
	);

	// ダミー
	always @(posedge CLK or negedge nRES) begin
		if (!nRES)
			Dout	<= 8'h20;
		else if ((state == 5'b10011) | (state == 5'b01011)) begin
			if (Dout==8'h6f)	Dout <= 8'h20;
			else				Dout <= Dout + 8'h1;
			end
		else
			Dout	<= Dout;
	end

	// ステートマシン
	// 000:idle
	always @(posedge CLK or negedge nRES) begin
		if (!nRES) begin
			state	<= 5'b00000;
			nOE		<= 1'b1;
			nRD		<= 1'b1;
			nWR		<= 1'b1;
			wrOE	<= 1'b0;
			end
		else if ((state == 5'b00000) & (TRG1==1)) begin
			state	<= 5'b01000;				// IDLE -> 書込1
			nOE		<= nOE;
			nRD		<= nRD;
			nWR		<= nWR;
			wrOE	<= wrOE;
			end
		else if ((state == 5'b00000) & (nRXF == 0)) begin
			state	<= 5'b00001;				// IDLE -> 読込1
			nOE		<= 1'b0;
			nRD		<= 1'b1;
			nWR		<= 1'b1;
			wrOE	<= 1'b0;
			end
		else if (state == 5'b00001) begin		// 読込1
			if (nRXF == 1'b1) begin
				state	<= 5'b00000;			// 読込1 -> IDLE
				nOE		<= 1'b1;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			else begin
				state	<= 5'b00010;			// 読込1 -> 読込2
//				state	<= 5'b00100;			// 読込1 -> 保持 FIFOをもつことにした場合の実力試験
				nOE		<= 1'b0;
				nRD		<= 1'b1;				// nRDは受信ACKなのでここではアサートしない。
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			end
		else if (state == 5'b00010) begin		// 読込2
			if (nRXF == 1'b1) begin
				state	<= 5'b00000;			// 読込2 → IDLE
				nOE		<= 1'b1;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			else begin
				state	<= 5'b00011;			// 読込2 -> 保持
				nOE		<= 1'b0;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			end
		else if (state == 5'b00011) begin		// 保持
			state	<= 5'b00100;				// 保持 -> 保持2
			nOE		<= 1'b0;
			nRD		<= 1'b1;
			nWR		<= 1'b1;
			wrOE	<= 1'b0;
			end
		else if (state == 5'b00100) begin		// 保持2
			state	<= 5'b00101;				// 保持2 -> 保持3
			nOE		<= 1'b0;
			nRD		<= 1'b0;					//　nRDは受信ACKなのでここでアサートする
			nWR		<= 1'b1;
			wrOE	<= 1'b0;
			end
		else if (state == 5'b00101) begin		// 保持3
			if (TRG1 == 1'b1) begin
				state	<= 5'b10000;				// 保持3 -> 書込1
				nOE		<= 1'b1;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			else if (TRG2 == 1'b1) begin
				state	<= state;				// 保持3滞留
				nOE		<= 1'b0;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			else if (nRXF == 1'b1) begin
				state	<= 5'b00000;			// 保持3 -> IDLE
				nOE		<= 1'b1;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			else begin
				state	<= 5'b00000;			// 保持3 -> IDLE
//				state	<= 5'b0001;				// 保持3 -> 読込1
				nOE		<= 1'b0;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			end
		else if ((state == 5'b10000) | (state == 5'b01000)) begin
			if ((nTXE == 1) & (state == 5'b10000)) begin
				state	<= 5'b00101;			// 書込1 -> 保持3
				nOE		<= 1'b0;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			else if ((nTXE == 1) & (state == 5'b01000)) begin
				state	<= 5'b00000;			// 書込1 -> IDLE
				nOE		<= 1'b1;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			else begin
				state	<= state + 5'b1;		// 書込1 -> 値セット
				nOE		<= 1'b1;
				nRD		<= 1'b1;
				nWR		<= 1'b1;
				wrOE	<= 1'b0;
				end
			end
		else if ((state == 5'b10001) | (state == 5'b01001)) begin
			state	<= state + 5'b1;			// 値セット -> 値セット2
			nOE		<= 1'b1;
			nRD		<= 1'b1;
			nWR		<= 1'b1;
			wrOE	<= 1'b1;					// バス方向切替
			end
		else if ((state == 5'b10010) | (state == 5'b01010)) begin
			state	<= state + 5'b1;			// 値セット2 -> 解放
			nOE		<= 1'b1;
			nRD		<= 1'b1;
			nWR		<= 1'b0;					// WR
			wrOE	<= 1'b1;
			end
		else if ((state == 5'b10011) | (state == 5'b01011)) begin
			state	<= state + 5'b1;			// 解放 -> 書込終了
			nOE		<= 1'b1;
			nRD		<= 1'b1;
			nWR		<= 1'b1;
			wrOE	<= 1'b0;
			end
		else if (state == 5'b10100) begin
			state	<= 5'b00101;				// 解放 -> 保持3
			nOE		<= 1'b0;
			nRD		<= 1'b1;
			nWR		<= 1'b1;
			wrOE	<= 1'b0;
			end
		else if (state == 5'b01100) begin
			state	<= 5'b00000;				// 解放 -> IDLE
			nOE		<= 1'b1;
			nRD		<= 1'b1;
			nWR		<= 1'b1;
			wrOE	<= 1'b0;
			end
		else begin								// 未定義状態
			state	<= state;
			nOE		<= 1'b1;
			nRD		<= 1'b1;
			nWR		<= 1'b1;
			wrOE	<= 1'b0;
			end
		end

	assign nPOWSAV = 1;			// 1:Normal operation 0:Suspend
	assign nSIWU = 1;			// 1:

//-- 以下デバック用
//-- UISWの動作検証
//	assign TP1 = UISW1;	//スイッチ入力応答OK
//	assign TP2 = UISW2;	//スイッチ入力応答OK
	assign TP1 = TRG1;
	assign TP2 = TRG2;

//-- 大容量転送中のレイテンシ観測機能
	always @(posedge CLK) begin
		if (!nRXF)
			letancycount	<= 0;
		else if (letancycount != 16'hffff)
			letancycount	<= letancycount + 1'b1;
		else
			letancycount	<= letancycount;
	end
//	assign TP1 = (letancycount == 16'd12000);	// 200uSレイテンシ
//	assign TP1 = (letancycount == 16'd3000);	// 50uSレイテンシ
//	assign TP2 = (letancycount == 16'd6000);	// 100uS測済レイテンシ
//	assign TP1 = (letancycount == 16'b1111111111);	// 17.05uSレイテンシ
//	assign TP2 = (letancycount == 16'd600);	// 10uS測済レイテンシ
//	assign TP1 = (letancycount == 16'd60);	// 1uSレイテンシ
//	assign TP2 = (letancycount == 16'd7);	// 観測済レイテンシ

//	assign TP1 = (state == 5'b00000);
//	assign TP2 = (state == 5'b00100);		// nRDタイミング
endmodule
