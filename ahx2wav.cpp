/*
	ahx2wav v0.11 - mod

	Credits to gomaigasane (original code) and LemonHaze (latest additions)

	Changes:
		* Improved portability
		* Added verbose and batch switches

 * Original Copyright (C) 2006
	http://pie.bbspink.com/test/read.cgi/leaf/1141063964/191
	http://pie.bbspink.com/test/read.cgi/leaf/1141063964/258

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


	mpeg2 audio layer-2????????mpg123????????
		http://www.mpg123.de/
*/
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#pragma warning(disable:4244)

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <cmath>
#endif

#define IS_ARG_PRESENT(x) strstr(argv[i], x)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>

#ifndef _MSC_VER
#include <experimental/filesystem>
#else
#include <filesystem>
#endif

#include <chrono>

namespace fs = std::experimental::filesystem;

char fn_ahx[4096], fn_wav[4096]; 

bool verbose = false;

unsigned long getword(unsigned char *p)
{
	return p[0] + p[1] * 256;
} //modify for bcc32

unsigned long getlong(unsigned char *p)
{
	return p[0] + p[1] * 256 + (p[2] + p[3] * 256) * 65536;
} //modify for bcc32

unsigned long getlongb(unsigned char *p)
{
	return p[3] + p[2] * 256 + (p[1] + p[0] * 256) * 65536;
} //modify for bcc32

void putword(unsigned char *p, int a)
{
	p[0] = a;
	p[1] = a >> 8;
} //modify for bcc32

void putlong(unsigned char *p, int a)
{
	p[0] = a;
	p[1] = a >> 8;
	p[2] = a >> 16;
	p[3] = a >> 24;
} //modify for bcc32

int bit_alloc_table[32] = { 4,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 };
int offset_table[5][16] = {
	{ 0 },
{ 0 },
{ 0, 1,  3, 4, },
{ 0, 1,  3, 4, 5, 6,  7, 8, },
{ 0, 1,  2, 3, 4, 5,  6, 7,  8,  9, 10, 11, 12, 13, 14 }
};

struct quantclass {
	int nlevels;
	int bits;
} const qc_table[17] = {
	{ 3,  -5 },
{ 5,  -7 },
{ 7,   3 },
{ 9, -10 },
{ 15,   4 },
{ 31,   5 },
{ 63,   6 },
{ 127,   7 },
{ 255,   8 },
{ 511,   9 },
{ 1023,  10 },
{ 2047,  11 },
{ 4095,  12 },
{ 8191,  13 },
{ 16383,  14 },
{ 32767,  15 },
{ 65535,  16 }
};
int intwinbase[257] = {
	0,    -1,    -1,    -1,    -1,    -1,    -1,    -2,    -2,    -2,    -2,    -3,    -3,    -4,    -4,    -5,
	-5,    -6,    -7,    -7,    -8,    -9,   -10,   -11,   -13,   -14,   -16,   -17,   -19,   -21,   -24,   -26,
	-29,   -31,   -35,   -38,   -41,   -45,   -49,   -53,   -58,   -63,   -68,   -73,   -79,   -85,   -91,   -97,
	-104,  -111,  -117,  -125,  -132,  -139,  -147,  -154,  -161,  -169,  -176,  -183,  -190,  -196,  -202,  -208,
	-213,  -218,  -222,  -225,  -227,  -228,  -228,  -227,  -224,  -221,  -215,  -208,  -200,  -189,  -177,  -163,
	-146,  -127,  -106,   -83,   -57,   -29,     2,    36,    72,   111,   153,   197,   244,   294,   347,   401,
	459,   519,   581,   645,   711,   779,   848,   919,   991,  1064,  1137,  1210,  1283,  1356,  1428,  1498,
	1567,  1634,  1698,  1759,  1817,  1870,  1919,  1962,  2001,  2032,  2057,  2075,  2085,  2087,  2080,  2063,
	2037,  2000,  1952,  1893,  1822,  1739,  1644,  1535,  1414,  1280,  1131,   970,   794,   605,   402,   185,
	-45,  -288,  -545,  -814, -1095, -1388, -1692, -2006, -2330, -2663, -3004, -3351, -3705, -4063, -4425, -4788,
	-5153, -5517, -5879, -6237, -6589, -6935, -7271, -7597, -7910, -8209, -8491, -8755, -8998, -9219, -9416, -9585,
	-9727, -9838, -9916, -9959, -9966, -9935, -9863, -9750, -9592, -9389, -9139, -8840, -8492, -8092, -7640, -7134,
	-6574, -5959, -5288, -4561, -3776, -2935, -2037, -1082,   -70,   998,  2122,  3300,  4533,  5818,  7154,  8540,
	9975, 11455, 12980, 14548, 16155, 17799, 19478, 21189, 22929, 24694, 26482, 28289, 30112, 31947, 33791, 35640,
	37489, 39336, 41176, 43006, 44821, 46617, 48390, 50137, 51853, 53534, 55178, 56778, 58333, 59838, 61289, 62684,
	64019, 65290, 66494, 67629, 68692, 69679, 70590, 71420, 72169, 72835, 73415, 73908, 74313, 74630, 74856, 74992,
	75038 };
double costable[5][16], decwin[512 + 32];

int getbits(unsigned char **src, int *bit_data, int *bit_rest, int bits)
{
	int __ret;

	while (*bit_rest<24) {
		*bit_data <<= 8;
		*bit_data |= **src;
		*src = *src + 1;
		*bit_rest += 8;
	}

	__ret = (*bit_data >> (*bit_rest - bits)) & ((1 << bits) - 1);
	*bit_rest -= bits;

	//printf("[verbose] getbits : %d %x\n", bits, __ret);

	return(__ret);
} //modify for bcc32 : from ExtractData

void dct(double *src, double *dst0, double *dst1)
{
	double tmp[2][32];
	int i;

	for (i = 0; i<32; i++) {
		if (i & 16) tmp[0][i] = (-src[i] + src[31 ^ i]) * costable[0][~i & 15];
		else     tmp[0][i] = (+src[i] + src[31 ^ i]);
	}
	for (i = 0; i<32; i++) {
		if (i & 8) tmp[1][i] = (-tmp[0][i] + tmp[0][15 ^ i]) * costable[1][~i & 7] * ((i & 16) ? -1.0 : 1.0);
		else    tmp[1][i] = (+tmp[0][i] + tmp[0][15 ^ i]);
	}
	for (i = 0; i<32; i++) {
		if (i & 4) tmp[0][i] = (-tmp[1][i] + tmp[1][7 ^ i]) * costable[2][~i & 3] * ((i & 8) ? -1.0 : 1.0);
		else    tmp[0][i] = (+tmp[1][i] + tmp[1][7 ^ i]);
	}
	for (i = 0; i<32; i++) {
		if (i & 2) tmp[1][i] = (-tmp[0][i] + tmp[0][3 ^ i]) * costable[3][~i & 1] * ((i & 4) ? -1.0 : 1.0);
		else    tmp[1][i] = (+tmp[0][i] + tmp[0][3 ^ i]);
	}
	for (i = 0; i<32; i++) {
		if (i & 1) tmp[0][i] = (-tmp[1][i] + tmp[1][1 ^ i]) * costable[4][0] * ((i & 2) ? -1.0 : 1.0);
		else    tmp[0][i] = (+tmp[1][i] + tmp[1][1 ^ i]);
	}

	//for (int i = 0; i < 8; i++)
	//tmp[0][i * 4 + 2] += tmp[0][i * 4 + 3];
	for (i = 0; i < 32; i += 4)
		tmp[0][i + 2] += tmp[0][i + 3]; //modify : from ExtractData

										//for (int i = 0; i < 4; i++) {
										//	tmp[0][i * 8 + 4] += tmp[0][i * 8 + 6];
										//	tmp[0][i * 8 + 6] += tmp[0][i * 8 + 5];
										//	tmp[0][i * 8 + 5] += tmp[0][i * 8 + 7];
										//}
	for (i = 0; i < 32; i += 8) {
		tmp[0][i + 4] += tmp[0][i + 6];
		tmp[0][i + 6] += tmp[0][i + 5];
		tmp[0][i + 5] += tmp[0][i + 7];
	} //modify : from ExtractData

	  //for (int i = 0; i < 2; i++) {
	  //	tmp[0][i * 16 +  8] += tmp[0][i * 16 + 12];
	  //	tmp[0][i * 16 + 12] += tmp[0][i * 16 + 10];
	  //	tmp[0][i * 16 + 10] += tmp[0][i * 16 + 14];
	  //	tmp[0][i * 16 + 14] += tmp[0][i * 16 +  9];
	  //	tmp[0][i * 16 +  9] += tmp[0][i * 16 + 13];
	  //	tmp[0][i * 16 + 13] += tmp[0][i * 16 + 11];
	  //	tmp[0][i * 16 + 11] += tmp[0][i * 16 + 15];
	  //}
	for (i = 0; i < 32; i += 16) {
		tmp[0][i + 8] += tmp[0][i + 12];
		tmp[0][i + 12] += tmp[0][i + 10];
		tmp[0][i + 10] += tmp[0][i + 14];
		tmp[0][i + 14] += tmp[0][i + 9];
		tmp[0][i + 9] += tmp[0][i + 13];
		tmp[0][i + 13] += tmp[0][i + 11];
		tmp[0][i + 11] += tmp[0][i + 15];
	} //modify : from ExtractData

	dst0[16] = tmp[0][0];
	dst0[15] = tmp[0][16 + 0] + tmp[0][16 + 8];
	dst0[14] = tmp[0][8];
	dst0[13] = tmp[0][16 + 8] + tmp[0][16 + 4];
	dst0[12] = tmp[0][4];
	dst0[11] = tmp[0][16 + 4] + tmp[0][16 + 12];
	dst0[10] = tmp[0][12];
	dst0[9] = tmp[0][16 + 12] + tmp[0][16 + 2];
	dst0[8] = tmp[0][2];
	dst0[7] = tmp[0][16 + 2] + tmp[0][16 + 10];
	dst0[6] = tmp[0][10];
	dst0[5] = tmp[0][16 + 10] + tmp[0][16 + 6];
	dst0[4] = tmp[0][6];
	dst0[3] = tmp[0][16 + 6] + tmp[0][16 + 14];
	dst0[2] = tmp[0][14];
	dst0[1] = tmp[0][16 + 14] + tmp[0][16 + 1];
	dst0[0] = tmp[0][1];

	dst1[0] = tmp[0][1];
	dst1[1] = tmp[0][16 + 1] + tmp[0][16 + 9];
	dst1[2] = tmp[0][9];
	dst1[3] = tmp[0][16 + 9] + tmp[0][16 + 5];
	dst1[4] = tmp[0][5];
	dst1[5] = tmp[0][16 + 5] + tmp[0][16 + 13];
	dst1[6] = tmp[0][13];
	dst1[7] = tmp[0][16 + 13] + tmp[0][16 + 3];
	dst1[8] = tmp[0][3];
	dst1[9] = tmp[0][16 + 3] + tmp[0][16 + 11];
	dst1[10] = tmp[0][11];
	dst1[11] = tmp[0][16 + 11] + tmp[0][16 + 7];
	dst1[12] = tmp[0][7];
	dst1[13] = tmp[0][16 + 7] + tmp[0][16 + 15];
	dst1[14] = tmp[0][15];
	dst1[15] = tmp[0][16 + 15];
}

int decode_ahx(unsigned char *src, unsigned char *dst, int srclen)
{
	int bit_rest = 0, bit_data;
	unsigned char *src_start = src;
	int i, j, sb, gr, phase = 0;
	int bit_alloc[32], scfsi[32], scalefactor[32][3] = { 0 }; // modify for bcc32
	short *dst_p = (short*)dst; // from ExtractData
	double sbsamples[36][32], powtable[64];
	double dctbuf[2][16][17];
	double *win;
	int frame = 0;

	for (i = 0; i<64; i++) powtable[i] = pow(2.0, (3 - i) / 3.0);
	for (i = 0; i<36; i++) for (sb = 0; sb<32; sb++) sbsamples[i][sb] = 0.0;

	//for (int i = 0; i < 16; i++)
	//	costable[0][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) / 64.0);

	//for (int i = 0; i < 8; i++)
	//	costable[1][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) / 32.0);

	//for (int i = 0; i < 4; i++)
	//	costable[2][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) / 16.0);

	//for (int i = 0; i < 2; i++)
	//	costable[3][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) /  8.0);

	//for (int i = 0; i < 1; i++)
	//	costable[4][i] = 0.5 / cos(M_PI * ((double) i * 2.0 + 1.0) /  4.0);

	for (i = 0; i < 16; i++)
		costable[0][i] = 0.5 / cos(M_PI * ((i << 1) + 1) / 64.0); //modify : from ExtractData

	for (i = 0; i < 8; i++)
		costable[1][i] = 0.5 / cos(M_PI * ((i << 1) + 1) / 32.0); //modify : from ExtractData

	for (i = 0; i < 4; i++)
		costable[2][i] = 0.5 / cos(M_PI * ((i << 1) + 1) / 16.0); //modify : from ExtractData

	for (i = 0; i < 2; i++)
		costable[3][i] = 0.5 / cos(M_PI * ((i << 1) + 1) / 8.0); //modify : from ExtractData

	for (i = 0; i < 1; i++)
		costable[4][i] = 0.5 / cos(M_PI * ((i << 1) + 1) / 4.0); //modify : from ExtractData

	for (i = 0, j = 0; i<256; i++, j += 32) {
		if (j<512 + 16) decwin[j] = decwin[j + 16] = intwinbase[i] / 65536.0 * 32768.0 * ((i & 64) ? +1.0 : -1.0);
		if ((i & 31) == 31) j -= 1023;
	}
	for (i = 0, j = 8; i<256; i++, j += 32) {
		if (j<512 + 16) decwin[j] = decwin[j + 16] = intwinbase[256 - i] / 65536.0 * 32768.0 * ((i & 64) ? +1.0 : -1.0);
		if ((i & 31) == 31) j -= 1023;
	}

	src += src[2] * 256 + src[3] + 4;

	while (src - src_start < srclen && getbits(&src, &bit_data, &bit_rest, 12) == 0xfff) {
		frame++;
		getbits(&src, &bit_data, &bit_rest, 1);	// LSF
		getbits(&src, &bit_data, &bit_rest, 2);	// layer
		getbits(&src, &bit_data, &bit_rest, 1);	// CRC
		getbits(&src, &bit_data, &bit_rest, 4);	// bitrate
		getbits(&src, &bit_data, &bit_rest, 2);	// freq
		getbits(&src, &bit_data, &bit_rest, 1);	// padding
		getbits(&src, &bit_data, &bit_rest, 1);	// gap
		getbits(&src, &bit_data, &bit_rest, 2);	// mode
		getbits(&src, &bit_data, &bit_rest, 2);	// mode_ext
		getbits(&src, &bit_data, &bit_rest, 1);	// protect
		getbits(&src, &bit_data, &bit_rest, 1);	// copy
		getbits(&src, &bit_data, &bit_rest, 2);	// emphasis

		for (sb = 0; sb<30; sb++) {
			bit_alloc[sb] = getbits(&src, &bit_data, &bit_rest, bit_alloc_table[sb]);
		}
		for (sb = 0; sb<30; sb++) {
			if (bit_alloc[sb]) {
				scfsi[sb] = getbits(&src, &bit_data, &bit_rest, 2);
			}
		}
		for (sb = 0; sb<30; sb++) {
			if (bit_alloc[sb]) {
				scalefactor[sb][0] = getbits(&src, &bit_data, &bit_rest, 6);
				switch (scfsi[sb]) {
				case 0:
					scalefactor[sb][1] = getbits(&src, &bit_data, &bit_rest, 6);
					scalefactor[sb][2] = getbits(&src, &bit_data, &bit_rest, 6);
					break;
				case 1:
					scalefactor[sb][1] = scalefactor[sb][0];
					scalefactor[sb][2] = getbits(&src, &bit_data, &bit_rest, 6);
					break;
				case 2:
					scalefactor[sb][1] = scalefactor[sb][0];
					scalefactor[sb][2] = scalefactor[sb][0];
					break;
				case 3:
					scalefactor[sb][1] =
						scalefactor[sb][2] = getbits(&src, &bit_data, &bit_rest, 6);
					break;
				}
			}
		}
		for (gr = 0; gr<12; gr++) {
			for (sb = 0; sb<30; sb++) {
				if (bit_alloc[sb]) {
					int index = offset_table[bit_alloc_table[sb]][bit_alloc[sb] - 1], q;
					if (qc_table[index].bits<0) {
						int t = getbits(&src, &bit_data, &bit_rest, -qc_table[index].bits);
						q = (t % qc_table[index].nlevels) * 2 - qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 0][sb] = (double)q / (double)qc_table[index].nlevels;
						t /= qc_table[index].nlevels;
						q = (t % qc_table[index].nlevels) * 2 - qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 1][sb] = (double)q / (double)qc_table[index].nlevels;
						t /= qc_table[index].nlevels;
						q = t * 2 - qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 2][sb] = (double)q / (double)qc_table[index].nlevels;
					}
					else {
						q = getbits(&src, &bit_data, &bit_rest, qc_table[index].bits) * 2 - qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 0][sb] = (double)q / (double)qc_table[index].nlevels;
						q = getbits(&src, &bit_data, &bit_rest, qc_table[index].bits) * 2 - qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 1][sb] = (double)q / (double)qc_table[index].nlevels;
						q = getbits(&src, &bit_data, &bit_rest, qc_table[index].bits) * 2 - qc_table[index].nlevels + 1;
						sbsamples[gr * 3 + 2][sb] = (double)q / (double)qc_table[index].nlevels;
					}
				}
				else {
					sbsamples[gr * 3 + 0][sb] = 0;
					sbsamples[gr * 3 + 1][sb] = 0;
					sbsamples[gr * 3 + 2][sb] = 0;
				}
				//sbsamples[gr * 3 + 0][sb] *= powtable[scalefactor[sb][gr / 4]];
				//sbsamples[gr * 3 + 1][sb] *= powtable[scalefactor[sb][gr / 4]];
				//sbsamples[gr * 3 + 2][sb] *= powtable[scalefactor[sb][gr / 4]];
				sbsamples[gr * 3 + 0][sb] *= powtable[scalefactor[sb][gr >> 2]]; //modify : from ExtractData
				sbsamples[gr * 3 + 1][sb] *= powtable[scalefactor[sb][gr >> 2]]; //modify : from ExtractData
				sbsamples[gr * 3 + 2][sb] *= powtable[scalefactor[sb][gr >> 2]]; //modify : from ExtractData
			}
		}
		// synth
		for (gr = 0; gr<36; gr++) {
			double sum = 0;
			if (phase & 1)
				dct(sbsamples[gr], dctbuf[0][phase + 1 & 15], dctbuf[1][phase]);
			else
				dct(sbsamples[gr], dctbuf[1][phase], dctbuf[0][phase + 1]);
			win = decwin + 16 - (phase | 1);
			for (i = 0; i<16; i++, win += 16) {
				sum = *win++ * dctbuf[phase & 1][0][i];
				sum -= *win++ * dctbuf[phase & 1][1][i];
				sum += *win++ * dctbuf[phase & 1][2][i];
				sum -= *win++ * dctbuf[phase & 1][3][i];
				sum += *win++ * dctbuf[phase & 1][4][i];
				sum -= *win++ * dctbuf[phase & 1][5][i];
				sum += *win++ * dctbuf[phase & 1][6][i];
				sum -= *win++ * dctbuf[phase & 1][7][i];
				sum += *win++ * dctbuf[phase & 1][8][i];
				sum -= *win++ * dctbuf[phase & 1][9][i];
				sum += *win++ * dctbuf[phase & 1][10][i];
				sum -= *win++ * dctbuf[phase & 1][11][i];
				sum += *win++ * dctbuf[phase & 1][12][i];
				sum -= *win++ * dctbuf[phase & 1][13][i];
				sum += *win++ * dctbuf[phase & 1][14][i];
				sum -= *win++ * dctbuf[phase & 1][15][i];
				if (sum >= 32767) *dst_p++ = 32767;
				else if (sum <= -32767) *dst_p++ = -32767;
				else *dst_p++ = sum;
			}
			sum = win[0] * dctbuf[phase & 1][0][16];
			sum += win[2] * dctbuf[phase & 1][2][16];
			sum += win[4] * dctbuf[phase & 1][4][16];
			sum += win[6] * dctbuf[phase & 1][6][16];
			sum += win[8] * dctbuf[phase & 1][8][16];
			sum += win[10] * dctbuf[phase & 1][10][16];
			sum += win[12] * dctbuf[phase & 1][12][16];
			sum += win[14] * dctbuf[phase & 1][14][16];
			if (sum >= 32767) *dst_p++ = 32767;
			else if (sum <= -32767) *dst_p++ = -32767;
			else *dst_p++ = sum;
			win += -16 + (phase | 1) * 2;
			for (i = 15; i >= 1; i--, win -= 16) {
				sum = -*--win * dctbuf[phase & 1][0][i];
				sum -= *--win * dctbuf[phase & 1][1][i];
				sum -= *--win * dctbuf[phase & 1][2][i];
				sum -= *--win * dctbuf[phase & 1][3][i];
				sum -= *--win * dctbuf[phase & 1][4][i];
				sum -= *--win * dctbuf[phase & 1][5][i];
				sum -= *--win * dctbuf[phase & 1][6][i];
				sum -= *--win * dctbuf[phase & 1][7][i];
				sum -= *--win * dctbuf[phase & 1][8][i];
				sum -= *--win * dctbuf[phase & 1][9][i];
				sum -= *--win * dctbuf[phase & 1][10][i];
				sum -= *--win * dctbuf[phase & 1][11][i];
				sum -= *--win * dctbuf[phase & 1][12][i];
				sum -= *--win * dctbuf[phase & 1][13][i];
				sum -= *--win * dctbuf[phase & 1][14][i];
				sum -= *--win * dctbuf[phase & 1][15][i];
				if (sum >= 32767) *dst_p++ = 32767;
				else if (sum <= -32767) *dst_p++ = -32767;
				else *dst_p++ = sum;
			}
			phase = phase - 1 & 15;
		}
		// skip padding bits
		if (bit_rest & 7) getbits(&src, &bit_data, &bit_rest, bit_rest & 7);
	}
	if(verbose)
		printf("[verbose] [%s] %d frames\n", fn_ahx, frame);
	return ((char*)dst_p - (char*)dst); //modify : from ExtractData
}

void fwrite_wav_hed(int len, int freq, FILE *fp)
{
	static unsigned char hed[0x2c] = {
		'R','I','F','F',0,0,0,0,
		'W','A','V','E','f','m','t',' ',0x10,0,0,0,
		0x01,0x00,      // Linear PCM
		0x02,0x00,      // ch
		0x44,0xac,0x00,0x00,    // 44.1kHz
		0x10,0xb1,0x02,0x00,    // 44.1*4B
		0x04,0x00,      // block size
		0x10,0x00,      // bit/sample
		'd','a','t','a',0,0,0,0
	};
	putword(hed + 0x16, 1);	// ch
	putlong(hed + 0x18, freq);	// freq
	putlong(hed + 0x1c, freq * 2);	// byte/s
	putword(hed + 0x20, 0x02);	// block size
	putword(hed + 0x22, 0x10);	// bit/sample

	putlong(hed + 4, len + 0x2c - 4);
	putlong(hed + 0x28, len);

	fwrite(hed, 1, 0x2c, fp);
}
bool convertFileToWAV(char* ahxPath)
{
	auto start = std::chrono::high_resolution_clock::now();

	FILE *fp;
	unsigned char *ahx_buf, *wav_buf, *hed;
	int wav_buf_len, ahx_buf_len; //modify for ahx2wav
	
	strcpy(fn_ahx, ahxPath); //modify for ahx2wav
	fp = fopen(fn_ahx, "rb");
	if (fp == NULL)
		return false;
	fseek(fp, 0, 2);
	hed = (unsigned char*)malloc(ftell(fp));
	if (hed == NULL)
		return false;

	fseek(fp, 0, SEEK_END);
	ahx_buf_len = ftell(fp);
	ahx_buf = (unsigned char*)malloc(ahx_buf_len);
	fseek(fp, 0, SEEK_SET);
	fread(ahx_buf, 1, ahx_buf_len, fp);
	fclose(fp);

	wav_buf_len = getlongb(ahx_buf + 12) * 2;
	wav_buf = (unsigned char*)malloc(wav_buf_len + 1152 * 16); //modify : from ExtractData; //+1152*2); margin = layer-2 frame size
	wav_buf_len = decode_ahx(ahx_buf, wav_buf, ahx_buf_len); //modify : from ExtractData

	strcpy(fn_wav, fn_ahx);
	strcat(fn_wav, ".wav");

	fp = fopen(fn_wav, "wb");
	fwrite_wav_hed(wav_buf_len, getlongb(ahx_buf + 8), fp);
	fwrite(wav_buf, 1, wav_buf_len, fp);
	fclose(fp);

	if(verbose)
		printf("[verbose] Written %s\n[verbose] Data Length: %d\n", fn_wav, wav_buf_len);

	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	printf("[log] Elapsed time: %fs\n", elapsed.count());

	if (wav_buf_len > 1)
		return true;
	else return false;
}

int batchConvertToWAV(char* ahxDir)
{
	auto start = std::chrono::high_resolution_clock::now();
	int conversions = 0;

	for (auto& p : fs::recursive_directory_iterator(ahxDir))
		if (p.path().extension() == std::string(".ahx"))
			if (convertFileToWAV((char*)p.path().string().c_str()))
				conversions++;

	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	printf("[log] Batch conversion completed in %fs (%d files)\n", elapsed.count(), conversions);
	
	return conversions;
}

int main(int argc, char **argv)
{
	printf("ahx2wav - v0.11 mod\nCredits: gomaigasane & LemonHaze\n================================\n\n");
	if (argc<2) {
		printf("Switches: ahx2wav [--verbose|-v] [--batch|-b] <input file/dir>\nExample Usage: \n\tahx2wav hoge.ahx\n\tahx2wav --verbose -b ./some/directory/\n");
		return -1;
	}
	
	bool batchConversion = false;
	for (int i = 1; i < argc; i++)
	{
		if (IS_ARG_PRESENT("-v") || IS_ARG_PRESENT("--verbose"))
			verbose = true;
		if (IS_ARG_PRESENT("-b") || IS_ARG_PRESENT("--batch"))
			batchConversion = true;
	}
	
	if (batchConversion)
	{
		batchConvertToWAV(argv[argc - 1]);
		return 0;
	}
	else
	{
		if (convertFileToWAV(argv[argc - 1])) {
			printf("[log] Conversion of %s completed successfully.\n", argv[argc - 1]);
			return 0;
		}
		else {
			printf("[log] Conversion of %s was unsuccessful.\n", argv[argc - 1]);
			return -1;
		}
	}

	return 0;
}
