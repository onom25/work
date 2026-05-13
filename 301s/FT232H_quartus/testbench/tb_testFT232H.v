/*
	prtdtsend testbench
	written by onom
*/
`timescale 1ns/100ps

module tb_testFT232H;
	reg				CLK = 1;
	reg				nRES;
	reg				nRXF;
	reg				nTXE;
	reg		[7:0]	Din;
	reg				UISW1;
	reg				UISW2;
	wire	[7:0]	Dout;
	wire			nRD;
	wire			nWR;
	wire			nOE;
	wire			wrOE;
	wire			nPOWSAV;
	wire			nSIWU;

	mode_SyncFIFO i_mode_SyncFIFO (
		.CLK(CLK), .nRES(nRES),
		.nRXF(nRXF), .nTXE(nTXE), .Din(Din), .UISW1(UISW1), .UISW2(UISW2),
		.Dout(Dout), .nRD(nRD), .nWR(nWR), .nOE(nOE), .wrOE(wrOE), .nPOWSAV(nPOWSAV), .nSIWU(nSIWU)
	);

	always #10 CLK = ~CLK; // 50MHz

	initial begin
				nRES=0; nRXF=1; nTXE=1; Din=8'h55; UISW1=1; UISW2=1;
		#5
		// 読込
		#40		nRES=1;
		#100	nRXF=0;
		#400
		#20		UISW1=0;	#20	UISW2=0;
		#100	UISW2=1;	#20 UISW1=1;
		#400
		#20		UISW1=0;	#20	UISW2=0;
		#400	UISW2=1;	#20 UISW1=1;
		#400	nRXF=1;
		#100
		$stop;
		// 書き込み
				nTXE=0;
		#20		UISW2=0;	#40	UISW1=0;
		#400	UISW1=1;	#40 UISW2=1;
		#400	nTXE=1;
		#200
		$stop;
	end

endmodule
