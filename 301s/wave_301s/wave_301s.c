/////////////////////////////////////////////////////////////////////////////////////////
//Linux gcc での　TCP/IP サンプルプログラム（ここからはクライアント）
//入力されたデータをクライアントに送り，もらったデータを表示する
//サーバープログラムを実行してからクライアントプログラムを実行して下さい

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include "wave_301s.h"
//#include "userparams_dmx100.h"

#define PORT 56565 //サーバープログラムとポート番号を合わせてください
#define PPORT 9100	// プリンタESCPポート

// IP アドレス，ソケット，sockaddr_in 構造体
char *destination;	//destination[32];
int dstSocket;
struct sockaddr_in dstAddr;
struct hostent *hp;

unsigned char   buf[1024*1024];
int    numrcv;

// 入力ファイル
char *filename = "tmp.dat";		// filename[32]
FILE *fi, *fi2;

struct st_wave301S wave_info;


// 秒指定の待機
void wait_sec (int sec)
{
	int i = 0;
	time_t ct = time(NULL);
	int s;
	struct tm *pt =localtime(&ct);

	s = pt->tm_sec;
	while (1) {
		ct = time(NULL);
		pt =localtime(&ct);
		if (s != pt->tm_sec) {
			i++;
			s = pt->tm_sec;
		}
		if (i > sec) {
			break;
		}
	}
}


// ソケットが接続されるまで待ち続ける。
int connect_port (int port) {
	//ソケット56565が再接続できるまで待機する。
	// ソケットが再接続できるかでコマンドの終了検知を代用します。
//	printf ("wait connect.");
	while (1) {
		dstAddr.sin_port = htons(port);
		dstSocket = socket(AF_INET, SOCK_STREAM, 0);
		//接続
		if (connect(dstSocket, (struct sockaddr *)&dstAddr, sizeof(dstAddr)) < 0){
//			printf("%s can't connect\n",destination);
//			printf (".");
			fflush (stdout);
			wait_sec (2);
		} else {
//			printf("%s(%d) connetcted\n", destination, port);
			return 0;
		}
	}
	return 0;
}


//
int do_toupper (unsigned char *bf) {
	int i=0;
	unsigned char *bfv;
//	printf ("do_toupper[%s]->\n", bf);
	bfv = bf;
	while (*bfv) {
		if (*bfv == '\x0')	break;
		*bfv = toupper(*bfv);
		bfv++;
		i++;
		if (i>=60) break;
	}
//	printf ("[%s]\n", bf);
	return 0;
}


// ファイルに取得情報を更新する
// コマンド単体の実行であるため、実行毎に内部変数を失うため、一時ファイルを用いる
void push_file ()
{
	fi = fopen (filename, "wb");
	if (fi == NULL) {
		printf ("file open error\n");
		return;
	}
	printf ("wave_info save tmp.dat %dbytes\n", sizeof(wave_info));
	fwrite (&wave_info, sizeof(wave_info), 1, fi);
	fclose (fi);
}


// 波形選択 ヘッドランク の取得
int read_headrank (void) {
	unsigned char *wptr;
	int siz, i;

	connect_port (PORT);
	wptr = buf;
	siz = 0;

	write (dstSocket, "\x1b\x80\x00\x0b", 4);
//	printf ("send:backdoor CMD_READ_CRPROFILE\n");
//	wait_sec (1);
	numrcv = read(dstSocket, buf, 4);
	if ((numrcv != 4) || (*(int*)buf != 0x0000801b)) {
		printf ("backdoor open error %d 0x%08x\n", numrcv, *(int*)buf);
		return -1;
	}

	sprintf (wptr, "\x1b\x28\x72\x06\x00", 5);
	*(long*)(wptr+5) = 0x00002000;		// MMP5500波形情報パラメータ
	siz = sizeof(wave_info.wave_num) + sizeof(wave_info.rank);
	*(short*)(wptr+9) = siz;
//	printf ("send:headrank read\n");
	write (dstSocket, wptr, 11);
//	wait_sec (1);
	numrcv = read(dstSocket, buf, siz);
	if (numrcv != siz) {
		printf ("miss recieve count %d\n", numrcv);
		return -1;
	} else {
		wave_info.wave_num = *(long*)buf;
		memcpy (wave_info.rank, buf +  sizeof(wave_info.wave_num), sizeof(wave_info.rank));
		wave_info.rank[sizeof(wave_info.rank)-1] = 0;
		printf ("wave select = [%d]\n", wave_info.wave_num);
		printf ("head rank = [%s<Null>]\n", wave_info.rank);
//		for(i=0; i<sizeof(wave_info.rank); i++)	printf ("%02x ",wave_info.rank[i]);
//		printf ("\n");
	}
	numrcv = read(dstSocket, buf, 4);
	if ((buf[0]!=0x1b) || (buf[1]!=0x80) || (buf[2]!=0x00) || (buf[3]!=0x00)) {
		printf("command RANK error ack %02x,%02x,%02x,%02x\n", *(buf+0), *(buf+1), *(buf+2), *(buf+3));
		close(dstSocket);
		wait_sec (1);
		return -1;
	}
	push_file ();
	printf ("command RRANK complete\n");
}


// FPGAの波形先頭オフセット情報取得
int read_wavepointer (void) {
	unsigned char *wptr;
	int siz, i;

	connect_port (PORT);
	wptr = buf;
	siz = 0;

	write (dstSocket, "\x1b\x80\x00\x0b", 4);
//	printf ("send:backdoor CMD_READ_CRPROFILE\n");
//	wait_sec (1);
	numrcv = read(dstSocket, buf, 4);
	if ((numrcv != 4) || (*(int*)buf != 0x0000801b)) {
		printf ("backdoor open error %d 0x%08x\n", numrcv, *(int*)buf);
		return -1;
	}

	sprintf (wptr, "\x1b\x28\x72\x06\x00", 5);
	*(long*)(wptr+5) = 0x00007fe0;
	*(short*)(wptr+9) = 32;
//	printf ("send:wave pointer read\n");
	write (dstSocket, wptr, 11);
//	wait_sec (1);
	numrcv = read(dstSocket, buf, 32);
	if (numrcv != 32) {
		printf ("miss recieve count %d\n", numrcv);
		return -1;
	} else {
		wave_info.WAVE001 = *(long*)(buf+ 0);
		wave_info.WAVE010 = *(long*)(buf+ 4);
		wave_info.WAVE011 = *(long*)(buf+ 8);
		wave_info.WAVE100 = *(long*)(buf+12);
		wave_info.WAVE101 = *(long*)(buf+16);
		wave_info.WAVE110 = *(long*)(buf+20);
		wave_info.WAVE111 = *(long*)(buf+24);
		wave_info.temprature_head		= *(short*)(buf+28);
		wave_info.temprature_heatsink	= *(short*)(buf+30);
		printf ("wave001(_stop)                : [0x%08x]\n", wave_info.WAVE001);
		printf ("wave010(_start)               : [0x%08x]\n", wave_info.WAVE010);
		printf ("wave011(_terminate)           : [0x%08x]\n", wave_info.WAVE011);
		printf ("wave100(_rocking_outofarea)   : [0x%08x]\n", wave_info.WAVE100);
		printf ("wave101(_rocking_beforeprint) : [0x%08x]\n", wave_info.WAVE101);
		printf ("wave110(_flashing)            : [0x%08x]\n", wave_info.WAVE110);
		printf ("wave111(_print)               : [0x%08x]\n", wave_info.WAVE111);
		printf ("temprature_head               : [%2d]\n", wave_info.temprature_head);
		printf ("temprature_headsink           : [%2d]\n", wave_info.temprature_heatsink);
	}
	numrcv = read(dstSocket, buf, 4);
	if ((buf[0]!=0x1b) || (buf[1]!=0x80) || (buf[2]!=0x00) || (buf[3]!=0x00)) {
		printf("command KIND error ack code %02x,%02x,%02x,%02x\n", *(buf+0), *(buf+1), *(buf+2), *(buf+3));
		close(dstSocket);
		wait_sec (1);
		return -1;
	}
	push_file ();
	printf ("command RKIND complete\n");
}


// FPGAの波形バッファ取得
int read_wavebuffer (int adr, int len) {
	unsigned char *wptr;
	int siz, i;
	unsigned char REP[10];

	adr &= ~0x01;			// 2Byte エイリアス強制
	len &= ~0x01;			// 2Byte エイリアス強制
	if (adr < 0)		adr = 0;
	if (adr > 0x3ffc)	adr = 0x3ffc;
	if (len < 4)		len = 4;
	if ((adr + len) > 0x3fff)
						len = 0x4000 - adr;
	//FPGAの波形メモリは16384Bytes 8192Word

	for (i=0; i<4096; i++) wave_info.wave[i].WAVE_MEM.WORD = i;		// 全初期化

	connect_port (PORT);
	wptr = buf;
	siz = 0;

	printf("<adr>=0x%08x  <len>=0x%x\n", adr, len);

	write (dstSocket, "\x1b\x80\x00\x0b", 4);
//	printf ("send:backdoor CMD_READ_CRPROFILE\n");
//	wait_sec (1);
	numrcv = read(dstSocket, buf, 4);
	if ((numrcv != 4) || (*(int*)buf != 0x0000801b)) {
		printf ("backdoor open error %d 0x%08x\n", numrcv, *(int*)buf);
		return -1;
	}

	sprintf (wptr, "\x1b\x28\x72\x06\x00", 5);
	*(long*)(wptr+5) = adr + 0x8000;
	// \x1b\x28\x72\x06\x00命令の波形領域をオフセット0x8000したコマンド生成
	*(short*)(wptr+9) = (unsigned short)len;
//	printf ("send:wave buffer read\n");
	write (dstSocket, wptr, 11);
//	wait_sec (1);
	siz= 0;
	wptr = (unsigned char *)&wave_info.wave[adr/2];
//	printf ("recv start siz=0x%X wptr=%x wave[0]=%x\n\n", siz, (int)wptr, (int)&wave_info.wave[0]);
	while (1) {
		numrcv = read(dstSocket, wptr, (len-siz));
		siz += numrcv;
		wptr += numrcv;
		if (siz >= len) {
//			printf ("recv %x wave buffer siz=0x%Xbytes wptr=0x%x\n", numrcv, siz, (int)wptr);
			break;
		}
//		printf (" recv %x wave siz=0x%X wptr=%x continue loop\n", numrcv, siz, (int)wptr);
	}
//	printf ("read last reply\n");
	i = read(dstSocket, REP, 4);
	if ((REP[0]!=0x1b) || (REP[1]!=0x80) || (REP[2]!=0x00) || (REP[3]!=0x00)) {
		printf ("command READ error ack code %02x,%02x,%02x,%02x\n", REP[0], REP[1], REP[2], REP[3]);
		close(dstSocket);
		wait_sec (1);
		return -1;
	}
	push_file ();
	printf ("command RWAVE complete\n");
	return 0;
}


//波形情報を表示する       tmp.dat の値を表示する。
int disp_wave_info (unsigned char* prm) {
	unsigned char *wptr;
	int siz, i=0, i2=0;

//	printf ("disp_wave_info name=%s\n", prm);
	do_toupper (prm);
//	printf ("disp_wave_info NAME=%s\n", prm);
	if (strcmp (prm, "RANK") == 0) {
		printf ("[tmp.dat] wave_number = [%d]\n", wave_info.wave_num);
		printf ("[tmp.dat] head rank = [%s<Null>]\n", wave_info.rank);
		return 0;
	} else if (strcmp (prm, "KIND") == 0) {
		printf ("[tmp.dat] wavedata offset address\n");
		printf ("wave001(_stop)                : [0x%08x]\n", wave_info.WAVE001);
		printf ("wave010(_start)               : [0x%08x]\n", wave_info.WAVE010);
		printf ("wave011(_terminate)           : [0x%08x]\n", wave_info.WAVE011);
		printf ("wave100(_rocking_outofarea)   : [0x%08x]\n", wave_info.WAVE100);
		printf ("wave101(_rocking_beforeprint) : [0x%08x]\n", wave_info.WAVE101);
		printf ("wave110(_flashing)            : [0x%08x]\n", wave_info.WAVE110);
		printf ("wave111(_print)               : [0x%08x]\n", wave_info.WAVE111);
		return 0;

	} else if (strcmp (prm, "STOP") == 0) {
//		printf ("offset address 0x%04x\n", wave_info.WAVE001);
		i = wave_info.WAVE001 / 2;
		i2 = i;
	} else if (strcmp (prm, "START") == 0) {
//		printf ("offset address 0x%04x\n", wave_info.WAVE010);
		i = wave_info.WAVE010 / 2;
		i2 = i;
	} else if (strcmp (prm, "TERMINATE") == 0) {
//		printf ("offset address 0x%04x\n", wave_info.WAVE011);
		i = wave_info.WAVE011 / 2;
		i2 = i;
	} else if (strcmp (prm, "ROCK4") == 0) {
//		printf ("offset address 0x%04x\n", wave_info.WAVE100);
		i = wave_info.WAVE100 / 2;
		i2 = i;
	} else if (strcmp (prm, "ROCK5") == 0) {
//		printf ("offset address 0x%04x\n", wave_info.WAVE101);
		i = wave_info.WAVE101 / 2;
		i2 = i;
	} else if (strcmp (prm, "FLASH") == 0) {
//		printf ("offset address 0x%04x\n", wave_info.WAVE110);
		i = wave_info.WAVE110 / 2;
		i2 = i;
	} else if (strcmp (prm, "PRINT") == 0) {
//		printf ("offset address 0x%04x\n", wave_info.WAVE111);
		i = wave_info.WAVE111 / 2;
		i2 = i;
	} else {
		return -1;
	}

	// 波形メモリダンプ
	printf (" no. E N C L  DAC         [%s] wave head point 0x%04x\n", prm, i);
	for ( ; i<0x2000; i++) {
		printf ("%4d %d %d %d %d 0x%04x[%5d]\n", i-i2
				, wave_info.wave[i].WAVE_MEM.BIT.ENB
				, wave_info.wave[i].WAVE_MEM.BIT.NCHG
				, wave_info.wave[i].WAVE_MEM.BIT.CH
				, wave_info.wave[i].WAVE_MEM.BIT.LAT
				, wave_info.wave[i].WAVE_MEM.BIT.DAC, wave_info.wave[i].WAVE_MEM.BIT.DAC
			);
		if (wave_info.wave[i].WAVE_MEM.BIT.ENB == 0) {
			break;
		}
	}
	return 0;
}


// DMX100パラメータの変更を不揮発メモリに反映する。
// 波形番号更新に続いて実施する事としているため ポート接続済で呼び出す必要がある。
int update_volatile () {
	write (dstSocket, "\x1b\x80\x00\x0b", 4);
	numrcv = read(dstSocket, buf, 4);
	if ((numrcv != 4) || (*(int*)buf != 0x0000801b)) {
		printf ("backdoor open error %d 0x%08x\n", numrcv, *(int*)buf);
		return -1;
	}
	sprintf (buf, "\x1b\x28\x77\x06\x00", 5);
	*(long*)(buf+5) = 0x3ff8;
	*(short*)(buf+9) = (unsigned short)4;
	write (dstSocket, buf, 11);
	*(long*)buf = 0;
	write (dstSocket, buf, 4);

	numrcv = read(dstSocket, buf, 4);
	if ((buf[0]!=0x1b) || (buf[1]!=0x80) || (buf[2]!=0x00) || (buf[3]!=0x00)) {
		printf("command KIND error ack code %02x,%02x,%02x,%02x\n", *(buf+0), *(buf+1), *(buf+2), *(buf+3));
		close(dstSocket);
		wait_sec (1);
		return -1;
	}
	return 0;
}


//波形番号を更新する。
//本ツールからプリンター内部のヘッドランクを更新することはしない。
int write_headrank (unsigned char* prm) {
	int i, wave_new;

	i = sscanf (prm, "%d", &wave_new);
	if (i != 1) {
		printf ("wave no. miss! [%s]\n", prm);
		return -1;
	}
	printf ("wave_no change [%d] -> [%d] (wait input  1<enter>:OK   0<enter>:CANCEL)", wave_info.wave_num, wave_new);
	scanf ("%d", &i);
	if (i == 1) {
//		printf ("modify execute\n");

		connect_port (PORT);

		write (dstSocket, "\x1b\x80\x00\x0b", 4);
//		printf ("send:backdoor CMD_READ_CRPROFILE\n");
//		wait_sec (1);
		numrcv = read(dstSocket, buf, 4);
		if ((numrcv != 4) || (*(int*)buf != 0x0000801b)) {
			printf ("backdoor open error %d 0x%08x\n", numrcv, *(int*)buf);
			return -1;
		}

		sprintf (buf, "\x1b\x28\x77\x06\x00", 5);
		*(long*)(buf+5) = 0x2000;
		*(short*)(buf+9) = (unsigned short)4;
//		printf ("tx buf %02x %02x %02x %02x %02x %02x\n", buf[5], buf[6], buf[7], buf[8], buf[9], buf[10]);
//		printf ("tx buf %08x %04x\n", *(long*)(buf+5), *(short*)(buf+9));
		write (dstSocket, buf, 11);
		*(long*)buf = wave_new;
		write (dstSocket, buf, 4);

		numrcv = read(dstSocket, buf, 4);
		if ((buf[0]!=0x1b) || (buf[1]!=0x80) || (buf[2]!=0x00) || (buf[3]!=0x00)) {
			printf("command KIND error ack code %02x,%02x,%02x,%02x\n", *(buf+0), *(buf+1), *(buf+2), *(buf+3));
			close(dstSocket);
			wait_sec (1);
			return -1;
		}
		if (update_volatile () != 0) {		// 不揮発更新
			printf ("error update_volatile ()\n");
			return -1;
		}
		wave_info.wave_num = wave_new;
		push_file ();
	}
	printf ("command WRANK complete\n");
	return 0;
}


//FPGAの波形先頭ポインタレジスタ値を直接更新する。
int write_wavepointer (unsigned char *prm1, unsigned char *prm2) {
	unsigned short ptr;
	int i;

	do_toupper (prm1);
	do_toupper (prm2);
	i = sscanf (prm2, "%x", &ptr);
	if ((i != 1) || (ptr>0x3ffc) || (ptr&0xc000 != 0)) {
		printf ("prm2 pointer value miss [%s]\n", prm2);
		return -1;
	}
	ptr &= ~1;	// 奇数の禁止
	printf ("parameter [%s] [0x%4x]\n", prm1, ptr);

	if (strcmp (prm1, "STOP") == 0) {
		wave_info.WAVE001 = ptr;
	} else if (strcmp (prm1, "START") == 0) {
		wave_info.WAVE010 = ptr;
	} else if (strcmp (prm1, "TERMINATE") == 0) {
		wave_info.WAVE011 = ptr;
	} else if (strcmp (prm1, "ROCK4") == 0) {
		wave_info.WAVE100 = ptr;
	} else if (strcmp (prm1, "ROCK5") == 0) {
		wave_info.WAVE101 = ptr;
	} else if (strcmp (prm1, "FLASH") == 0) {
		wave_info.WAVE110 = ptr;
	} else if (strcmp (prm1, "PRINT") == 0) {
		wave_info.WAVE111 = ptr;
	} else if (strcmp (prm1, "UPDATE") == 0) {
		//
	} else {
		return -1;
	}

	printf ("[tmp.dat] wavedata offset address\n");
	printf ("wave001(_stop)                : [0x%08x]\n", wave_info.WAVE001);
	printf ("wave010(_start)               : [0x%08x]\n", wave_info.WAVE010);
	printf ("wave011(_terminate)           : [0x%08x]\n", wave_info.WAVE011);
	printf ("wave100(_rocking_outofarea)   : [0x%08x]\n", wave_info.WAVE100);
	printf ("wave101(_rocking_beforeprint) : [0x%08x]\n", wave_info.WAVE101);
	printf ("wave110(_flashing)            : [0x%08x]\n", wave_info.WAVE110);
	printf ("wave111(_print)               : [0x%08x]\n", wave_info.WAVE111);

	push_file ();
	if (strcmp (prm1, "UPDATE") == 0) {
		connect_port (PORT);

		write (dstSocket, "\x1b\x80\x00\x0b", 4);
//		printf ("send:backdoor CMD_READ_CRPROFILE\n");
//		wait_sec (1);
		numrcv = read(dstSocket, buf, 4);
		if ((numrcv != 4) || (*(int*)buf != 0x0000801b)) {
			printf ("backdoor open error %d 0x%08x\n", numrcv, *(int*)buf);
			return -1;
		}

		sprintf (buf, "\x1b\x28\x77\x06\x00", 5);
		*(long*)(buf+5) = 0x7fe0;
		*(short*)(buf+9) = (unsigned short)32;
		write (dstSocket, buf, 11);
		write (dstSocket, &wave_info.WAVE001, 32);

		numrcv = read(dstSocket, buf, 4);
		if ((buf[0]!=0x1b) || (buf[1]!=0x80) || (buf[2]!=0x00) || (buf[3]!=0x00)) {
			printf("command KIND error ack code %02x,%02x,%02x,%02x\n", *(buf+0), *(buf+1), *(buf+2), *(buf+3));
			close(dstSocket);
			wait_sec (1);
			return -1;
		}
		printf ("command WKIND complete (do modify printer)\n");
	} else {
		printf ("command WKIND complete (not modify printer)\n");
	}
	return 0;
}


//FPGAの波形メモリを更新する。
// 波形メモリは 8192ワード存在するが、ターゲットの受信バッファが
// 8192バイトなので、全転送はできない。
// 波形先頭レジスタ更新に続いて実施するものとするため ポート接続済みで呼び出す事
int write_wavebuffer (int adr, int len) {
	unsigned short ptr;
	int i;

	if ((adr > 0x3ffc) || ((adr & 0xffffc001) != 0)) {
		// 領域外と奇数指定の除外
		printf ("miss address value [%d]\n", adr);
		return -1;
	}
	if (((adr+len) > 0x4000) || ((len&1) != 0) || (len > 0x2000)) {
		// 領域外と奇数長とターゲット側許容サイズ外の除外
		printf ("miss length value adr=0x%x len=0x%x\n", adr, len);
		return -1;
	}

	printf ("write_wavebuffer (0x%x, 0x%x)\n", adr, len);

//	connect_port (PORT);

	write (dstSocket, "\x1b\x80\x00\x0b", 4);
//	printf ("send:backdoor CMD_READ_CRPROFILE\n");
//	wait_sec (1);
	numrcv = read(dstSocket, buf, 4);
	if ((numrcv != 4) || (*(int*)buf != 0x0000801b)) {
		printf ("backdoor open error %d 0x%08x\n", numrcv, *(int*)buf);
		return -1;
	}

	sprintf (buf, "\x1b\x28\x77\x06\x00", 5);
	*(long*)(buf+5) = adr + 0x8000;
	// \x1b\x28\x72\x06\x00命令の波形領域をオフセット0x8000したコマンド生成
	*(short*)(buf+9) = (unsigned short)len;
	write (dstSocket, buf, 11);
	write (dstSocket, &wave_info.wave[adr/2], len);

	numrcv = read(dstSocket, buf, 4);
	if ((buf[0]!=0x1b) || (buf[1]!=0x80) || (buf[2]!=0x00) || (buf[3]!=0x00)) {
		printf("command WWAVE error ack code %02x,%02x,%02x,%02x\n", *(buf+0), *(buf+1), *(buf+2), *(buf+3));
		close(dstSocket);
		wait_sec (1);
		return -1;
	}
	printf ("command WWAVE complete\n");
	return 0;
}


/*
	引数
*/
void usage () {
		printf ("usage\n");
		printf (" command\n");
		printf (" DISP <prm>         display wave, rank structure info.\n");
		printf (" RRANK              read printer head rank\n");
		printf (" RKIND              read FPGA wave pointer\n");
		printf (" RWAVE <adr> <len>  read FPGA wave buffer\n");
		printf (" WRANK <number>     write wave_number\n");
		printf (" WKIND <prm> <val>  write FPGA wave pointer\n");
		printf (" FSET <filename>    set target FPGA from file\n");
		printf ("\n");
}


int main (int argc, char *argv[]) {
	int adr, dat;
	unsigned char tbf[60];
	int i;

	time_t ct = time(NULL);
	struct tm *pt =localtime(&ct);


//	printf ("sizeof wave_info=%d\n", sizeof(wave_info));

	// 相手先アドレスの入力と送る文字の入力
	destination = argv[1];
	//sockaddr_in 構造体のセット
	bzero((char *)&dstAddr, sizeof(dstAddr));
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_port = htons(PPORT);
	hp = gethostbyname(destination);
	bcopy(hp->h_addr, &dstAddr.sin_addr, hp->h_length);
	//ソケットの生成
	dstSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (argc < 3)	{	usage();	return -1;	}

	fi = fopen (filename, "rb");
	if (fi != NULL) {
		fread (&wave_info, sizeof(wave_info), 1, fi);
		printf ("./temp.dat read\n");
		fclose (fi);
	} else {
		printf ("./tmp.dat not exist un-read\n");
		fclose(fi);
	}

	do_toupper (argv[2]);
	if (strcmp (argv[2], "RRANK") == 0) {
		printf ("RANK: read head rank\n");
		read_headrank();
	} else if (strcmp(argv[2], "RKIND") == 0) {
		printf ("KIND: read wave start pointer\n");
		read_wavepointer();
	} else if (strcmp(argv[2], "RWAVE") == 0) {
		if (argc > 3) {
			i = sscanf (argv[3], "%x", &adr);
			if (i != 1) {
				printf ("arg. error address value\n");
				close (dstSocket);
				usage ();
				return -1;
			}
		} else {
			adr = 0;
		}
		if (argc > 4) {
			i = sscanf (argv[4], "%x", &dat);
			if (i != 1) {
				printf ("arg. error length value\n");
				close (dstSocket);
				usage ();
				return -1;
			}
		} else {
			dat = 0x4000;
		}
//		printf ("Read WAVE buffer adr:0x%x  len:0x%x\n", adr, dat);
		numrcv = read_wavebuffer(adr, dat);
		printf ("read_wavebuffer return %d\n", numrcv);

//		strcpy (tbf, "STOP");
//		i = disp_wave_info (tbf);
//		printf ("STOP return %d\n", i);
//		strcpy (tbf, "START");
//		i = disp_wave_info (tbf);
//		printf ("START return %d\n", i);
//		strcpy (tbf, "TERMINATE");
//		i = disp_wave_info (tbf);
//		printf ("TERMINATE return %d\n", i);
//		strcpy (tbf, "ROCK4");
//		i = disp_wave_info (tbf);
//		printf ("ROCK4 return %d\n", i);
//		strcpy (tbf, "ROCK5");
//		i = disp_wave_info (tbf);
//		printf ("ROCK5 return %d\n", i);
//		strcpy (tbf, "FLASH");
//		i = disp_wave_info (tbf);
//		printf ("FLASH return %d\n", i);
//		strcpy (tbf, "PRINT");
//		i = disp_wave_info (tbf);
//		printf ("PRINT return %d\n", i);

	} else if (strcmp(argv[2], "DISP") == 0) {
		i = -1;
		if (argc > 3) {
//			printf ("disp [%s]\n", argv[3]);
			i = disp_wave_info (argv[3]);
		}
		if (i != 0) {
			printf ("parameter name miss\n");
			printf (" RANK | KIND | STOP | START | TERMINATE | ROCK4 | ROCK5 | FLASH | PRINT");
		}
	} else if (strcmp(argv[2], "WRANK") == 0) {
		i = -1;
		if (argc > 3) {
			i = write_headrank (argv[3]);
		}
	} else if (strcmp(argv[2], "WKIND") == 0) {
		i = -1;
		if (argc > 4) {
//			printf ("WKIND execute\n");
			i = write_wavepointer (argv[3], argv[4]);
		}
		if (1 != 0) {
			printf ("argv[3] STOP | START | TERMINATE | ROCK4 | ROCK5 | FLASH | PRINT | UPDATE\n");
			printf ("argv[4] 0..0x3ffc pointer even value\n");
		}
	} else if (strcmp(argv[2], "FSET") == 0) {
		printf ("file [%s] read & set target\n", argv[3]);
		fi2 = fopen (argv[3], "rb");
		if (fi2 != NULL) {
			fread (&wave_info, sizeof(wave_info), 1, fi2);
//			printf ("get data from [%s]\n", argv[3]);
			fclose (fi2);
		} else {
			printf ("file [%s] not exist un-read\n");
			fclose(fi2);
			close (dstSocket);
			return -1;
		}
		// 波形先頭ポインタのFPGAレジスタ更新
//		printf ("write_wavepointer (UPDATE, 0)\n");
		sprintf (tbf+ 0, "UPDATE\x0");
		sprintf (tbf+10, "0\x0");
		i = write_wavepointer (tbf+0, tbf+10);
		if (i != 0) {
			close (dstSocket);
			return -1;
		}
#if 0
		//-- 転送データの試験用
		for (i=wave_info.WAVE001/2; i<wave_info.WAVE001/2+40; i++) {
			wave_info.wave[i].WAVE_MEM.BIT.DAC = i;
			wave_info.wave[i].WAVE_MEM.BIT.LAT = 0;
			wave_info.wave[i].WAVE_MEM.BIT.CH = 0;
			wave_info.wave[i].WAVE_MEM.BIT.NCHG = 1;
			wave_info.wave[i].WAVE_MEM.BIT.ENB = 1;
		}
		wave_info.wave[i].WAVE_MEM.BIT.DAC = i;
		wave_info.wave[i].WAVE_MEM.BIT.LAT = 0;
		wave_info.wave[i].WAVE_MEM.BIT.CH = 0;
		wave_info.wave[i].WAVE_MEM.BIT.NCHG = 0;
		wave_info.wave[i].WAVE_MEM.BIT.ENB = 0;
		sprintf (tbf+ 0, "STOP\x0");
		disp_wave_info (tbf);		//-- 値の確認
#endif
		// 波形メモリの下位8192byte更新 (受信バッファサイズが小さいため)
		i = write_wavebuffer (     0, 0x2000);
		i = write_wavebuffer (0x2000, 0x2000);
	}

	close(dstSocket);
	return (0);
}
