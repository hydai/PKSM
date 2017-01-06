/* This file is part of PKSM

Copyright (C) 2016 Bernardo Giordano

>    This program is free software: you can redistribute it and/or modify
>    it under the terms of the GNU General Public License as published by
>    the Free Software Foundation, either version 3 of the License, or
>    (at your option) any later version.
>
>    This program is distributed in the hope that it will be useful,
>    but WITHOUT ANY WARRANTY; without even the implied warranty of
>    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
>    GNU General Public License for more details.
>
>    You should have received a copy of the GNU General Public License
>    along with this program.  If not, see <http://www.gnu.org/licenses/>.
>    See LICENSE for information.
*/

#include <3ds.h>
#include <string.h>
#include "graphic.h"
#include "editor.h"
#include "util.h"
#include "memecrypto.h"
#include "save.h"

u16 crc16[] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

int getActiveGBO(u8* mainbuf, int game) {
	int ofs = 0;
	int generalBlock = -1;
	
	int k = 0;
	u8 temp[10];
	
	memcpy(&temp, mainbuf, 10);
	for (int t = 0; t < 10; t++)
		if (temp[t] == 0xFF)
			k++;
	if (k == 10)
		return 1;
	
	k = 0;
	memcpy(&temp, &mainbuf[0x40000], 10);
	for (int t = 0; t < 10; t++)
		if (temp[t] == 0xFF)
			k++;
	if (k == 10)
		return 1;
	
	if (game == GAME_HG || game == GAME_SS)
		ofs = 0xF618;
	else if (game == GAME_PLATINUM) 
		ofs = 0xCF1C;
	else if (game == GAME_DIAMOND || game == GAME_PEARL)
		ofs = 0xC0F0;
	
	u16 c1;
	u16 c2;
	memcpy(&c1, &mainbuf[ofs], 2);
	memcpy(&c2, &mainbuf[ofs + 0x40000], 2);
	
	generalBlock = (c1 >= c2) ? 0 : 1;
	
	return generalBlock;
}

int getActiveSBO(u8* mainbuf, int game) {
	int ofs = 0;
	int storageBlock = -1;
	
	if (game == GAME_HG || game == GAME_SS)
		ofs = 0x21A00;
	else if (game == GAME_PLATINUM) 
		ofs = 0x1F100;
	else if (game == GAME_DIAMOND || game == GAME_PEARL)
		ofs = 0x1E2D0;
	
	int k = 0;
	u8 temp[10];
	
	memcpy(&temp, &mainbuf[ofs], 10);
	for (int t = 0; t < 10; t++)
		if (temp[t] == 0xFF)
			k++;
	if (k == 10)
		return 1;
	
	k = 0;
	memcpy(&temp, &mainbuf[ofs + 0x40000], 10);
	for (int t = 0; t < 10; t++)
		if (temp[t] == 0xFF)
			k++;
	if (k == 10)
		return 0;
	
	u16 c1;
	u16 c2;
	memcpy(&c1, &mainbuf[ofs], 2);
	memcpy(&c2, &mainbuf[ofs + 0x40000], 2);
	
	storageBlock = (c1 >= c2) ? 0 : 1;
	
	return storageBlock;
}

u32 CHKOffset(u32 i, int game) {
	if (game == GAME_X || game == GAME_Y) {
		const u32 _xy[] = { 0x05400, 0x05800, 0x06400, 0x06600, 0x06800, 0x06A00, 0x06C00, 0x06E00, 0x07000, 0x07200, 0x07400, 0x09600, 0x09800, 0x09E00, 0x0A400, 0x0F400, 0x14400, 0x19400, 0x19600, 0x19E00, 0x1A400, 0x1AC00, 0x1B400, 0x1B600, 0x1B800, 0x1BE00, 0x1C000, 0x1C400, 0x1CC00, 0x1CE00, 0x1D000, 0x1D200, 0x1D400, 0x1D600, 0x1DE00, 0x1E400, 0x1E800, 0x20400, 0x20600, 0x20800, 0x20C00, 0x21000, 0x22C00, 0x23000, 0x23800, 0x23C00, 0x24600, 0x24A00, 0x25200, 0x26000, 0x26200, 0x26400, 0x27200, 0x27A00, 0x5C600, };
		return _xy[i] - 0x5400;
	}
	else if (game == GAME_OR || game == GAME_AS) {
		const u32 _oras[] = { 0x05400, 0x05800, 0x06400, 0x06600, 0x06800, 0x06A00, 0x06C00, 0x06E00, 0x07000, 0x07200, 0x07400, 0x09600, 0x09800, 0x09E00, 0x0A400, 0x0F400, 0x14400, 0x19400, 0x19600, 0x19E00, 0x1A400, 0x1B600, 0x1BE00, 0x1C000, 0x1C200, 0x1C800, 0x1CA00, 0x1CE00, 0x1D600, 0x1D800, 0x1DA00, 0x1DC00, 0x1DE00, 0x1E000, 0x1E800, 0x1EE00, 0x1F200, 0x20E00, 0x21000, 0x21400, 0x21800, 0x22000, 0x23C00, 0x24000, 0x24800, 0x24C00, 0x25600, 0x25A00, 0x26200, 0x27000, 0x27200, 0x27400, 0x28200, 0x28A00, 0x28E00, 0x30A00, 0x38400, 0x6D000, };
		return _oras[i] - 0x5400;
	}
	else if (game == GAME_SUN || game == GAME_MOON) {
		const u32 _sm[] = { 0x00000, 0x00E00, 0x01000, 0x01200, 0x01400, 0x01C00, 0x02A00, 0x03A00, 0x03E00, 0x04000, 0x04200, 0x04400, 0x04600, 0x04800, 0x04E00, 0x3B400, 0x40C00, 0x40E00, 0x42000, 0x43C00, 0x4A200, 0x50800, 0x54200, 0x54400, 0x54600, 0x64C00, 0x65000, 0x65C00, 0x69C00, 0x6A000, 0x6A800, 0x6AA00, 0x6B200, 0x6B400, 0x6B600, 0x6B800, 0x6BA00 };
		return _sm[i];
	}
	else if (game == GAME_B1 || game == GAME_W1) {
		const u32 _bw[] = { 0x00400, 0x01400, 0x02400, 0x03400, 0x04400, 0x05400, 0x06400, 0x07400, 0x08400, 0x09400, 0x0A400, 0x0B400, 0x0C400, 0x0D400, 0x0E400, 0x0F400, 0x10400, 0x11400, 0x12400, 0x13400, 0x14400, 0x15400, 0x16400, 0x17400, 0x1C800, 0x23F00 };
		return _bw[i];
	}
	else if (game == GAME_B2 || game == GAME_W2) {
		const u32 _b2w2[] = { 0x00400, 0x01400, 0x02400, 0x03400, 0x04400, 0x05400, 0x06400, 0x07400, 0x08400, 0x09400, 0x0A400, 0x0B400, 0x0C400, 0x0D400, 0x0E400, 0x0F400, 0x10400, 0x11400, 0x12400, 0x13400, 0x14400, 0x15400, 0x16400, 0x17400, 0x1C800, 0x25F00 }; 
		return _b2w2[i];
	}
	else return 0;
}

u32 CHKLength(u32 i, int game) {
	if (game == GAME_X || game == GAME_Y) {
		const u32 _xy[] = { 0x002C8, 0x00B88, 0x0002C, 0x00038, 0x00150, 0x00004, 0x00008, 0x001C0, 0x000BE, 0x00024, 0x02100, 0x00140, 0x00440, 0x00574, 0x04E28, 0x04E28, 0x04E28, 0x00170, 0x0061C, 0x00504, 0x006A0, 0x00644, 0x00104, 0x00004, 0x00420, 0x00064, 0x003F0, 0x0070C, 0x00180, 0x00004, 0x0000C, 0x00048, 0x00054, 0x00644, 0x005C8, 0x002F8, 0x01B40, 0x001F4, 0x001F0, 0x00216, 0x00390, 0x01A90, 0x00308, 0x00618, 0x0025C, 0x00834, 0x00318, 0x007D0, 0x00C48, 0x00078, 0x00200, 0x00C84, 0x00628, 0x34AD0, 0x0E058, };
		return _xy[i];
	}
	else if (game == GAME_OR || game == GAME_AS) {
		const u32 _oras[] = { 0x002C8, 0x00B90, 0x0002C, 0x00038, 0x00150, 0x00004, 0x00008, 0x001C0, 0x000BE, 0x00024, 0x02100, 0x00130, 0x00440, 0x00574, 0x04E28, 0x04E28, 0x04E28, 0x00170, 0x0061C, 0x00504, 0x011CC, 0x00644, 0x00104, 0x00004, 0x00420, 0x00064, 0x003F0, 0x0070C, 0x00180, 0x00004, 0x0000C, 0x00048, 0x00054, 0x00644, 0x005C8, 0x002F8, 0x01B40, 0x001F4, 0x003E0, 0x00216, 0x00640, 0x01A90, 0x00400, 0x00618, 0x0025C, 0x00834, 0x00318, 0x007D0, 0x00C48, 0x00078, 0x00200, 0x00C84, 0x00628, 0x00400, 0x07AD0, 0x078B0, 0x34AD0, 0x0E058, };
		return _oras[i];
	}
	else if (game == GAME_SUN || game == GAME_MOON) {
		const u32 _sm[] = { 0xDE0, 0x07C, 0x014, 0x0C0, 0x61C, 0xE00, 0xF78, 0x228, 0x104, 0x200, 0x020, 0x004, 0x058, 0x5E6, 0x36600, 0x572C, 0x008, 0x1080, 0x1A08, 0x6408, 0x6408, 0x3998, 0x100, 0x100, 0x10528, 0x204, 0xB60, 0x3F50, 0x358, 0x728, 0x200, 0x718, 0x1FC, 0x200, 0x120, 0x1C8, 0x200 };
		return _sm[i];
	}
	else if (game == GAME_W1 || game == GAME_B1) {
		const u32 _bw[] = { 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0A94, 0x008C };
		return _bw[i];
	}
	else if (game == GAME_W2 || game == GAME_B2) {
		const u32 _b2w2[] = { 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0A94, 0x0094 };
		return _b2w2[i];
	}
	else return 0;
}

u16 getBlockID(u8* mainbuf, int csoff, u32 i) {
	u16 id;
	memcpy(&id, &mainbuf[csoff + 8 * i - 2], sizeof(u16));
	return id;
}

u32 BWCHKOff(u32 i, int game) {
	if (game == GAME_B1 || game == GAME_W1) {
		const u32 _bw[] = { 0x013F2, 0x023F2, 0x033F2, 0x043F2, 0x053F2, 0x063F2, 0x073F2, 0x083F2, 0x093F2, 0x0A3F2, 0x0B3F2, 0x0C3F2, 0x0D3F2, 0x0E3F2, 0x0F3F2, 0x103F2, 0x113F2, 0x123F2, 0x133F2, 0x143F2, 0x153F2, 0x163F2, 0x173F2, 0x183F2, 0x1D296, 0x23F9A };
		return _bw[i];
	}
	else if (game == GAME_B2 || game == GAME_W2) {
		const u32 _b2w2[]= { 0x013F2, 0x023F2, 0x033F2, 0x043F2, 0x053F2, 0x063F2, 0x073F2, 0x083F2, 0x093F2, 0x0A3F2, 0x0B3F2, 0x0C3F2, 0x0D3F2, 0x0E3F2, 0x0F3F2, 0x103F2, 0x113F2, 0x123F2, 0x133F2, 0x143F2, 0x153F2, 0x163F2, 0x173F2, 0x183F2, 0x1D296, 0x25FA2 };
		return _b2w2[i];
	}
	else return 0;
}

u32 BWCHKMirr(u32 i, int game) {
	if (game == GAME_B1 || game == GAME_W1) {
		const u32 _bw[] = { 0x25F02, 0x25F04, 0x25F06, 0x25F08, 0x25F0A, 0x25F0C, 0x25F0E, 0x25F10, 0x25F12, 0x25F14, 0x25F16, 0x25F18, 0x25F1A, 0x25F1C, 0x25F1E, 0x25F20, 0x25F22, 0x25F24, 0x25F26, 0x25F28, 0x25F2A, 0x25F2C, 0x25F2E, 0x25F30, 0x23F44, 0x23F9A };
		return _bw[i];
	}
	
	else if (game == GAME_W2 || game == GAME_B2) {
		const u32 _b2w2[] = { 0x25F02, 0x25F04, 0x25F06, 0x25F08, 0x25F0A, 0x25F0C, 0x25F0E, 0x25F10, 0x25F12, 0x25F14, 0x25F16, 0x25F18, 0x25F1A, 0x25F1C, 0x25F1E, 0x25F20, 0x25F22, 0x25F24, 0x25F26, 0x25F28, 0x25F2A, 0x25F2C, 0x25F2E, 0x25F30, 0x25F44, 0x25FA2 };
		return _b2w2[i];
	}

	else return 0;	
}

u16 ccitt16(u8* data, u32 len) {
	u16 crc = 0xFFFF;

	for (u32 i = 0; i < len; i++) {
		crc ^= (u16) (data[i] << 8);

		for (u32 j = 0; j < 0x8; j++) {
			if ((crc & 0x8000) > 0)
				crc = (u16) ((crc << 1) ^ 0x1021);
			else
				crc <<= 1;
		}
	}

	return crc;
}

u16 check16(u8 data[], u32 blockID, u32 len) {
	u16 initial = 0;
	
	if (blockID == 36) {
		u8 tmp[0x80];
		memset(tmp, 0, 0x80);
		memcpy(&data[0x100], tmp, 0x80);
	}

	u16 chk = ~initial;
	
	if (len > 1) {
		int ofs = -1;
		if (len % 2 == 0) {
			ofs = 0;
			chk = (crc16[(data[0] ^ chk) & 0xFF] ^ chk >> 8);
		}

		for (int i = (len - 1) / 2; i != 0; i--, ofs += 2) {
			u16 temp = crc16[(data[ofs + 1] ^ chk) & 0xFF];
			chk = (crc16[(data[ofs + 2] ^ temp ^ chk >> 8) & 0xFF] ^ (temp ^ chk >> 8) >> 8);
		}
	}
	if (len > 0)
		chk = (crc16[(data[len - 1] ^ chk) & 0xFF] ^ chk >> 8);

	return ~chk;
}

void rewriteCHK(u8 *mainbuf, int game) {
	u8 blockCount = 0;
	u8* tmp = (u8*)malloc(0x36600 * sizeof(u8));
	u16 cs;
	u32 csoff = 0;
	
	if (game == GAME_X || game == GAME_Y) {
		blockCount = 55;
		csoff = 0x6A81A - 0x5400;
	}

	else if (game == GAME_OR || game == GAME_AS) {
		blockCount = 58;
		csoff = 0x7B21A - 0x5400;
	}
	
	else if (game == GAME_SUN || game == GAME_MOON) {
		blockCount = 37;
		csoff = 0x6BE00 - 0x200 + 0x10 + 0x0A;
	}

	if (game == GAME_X || game == GAME_Y || game == GAME_OR || game == GAME_AS)
		for (u32 i = 0; i < blockCount; i++) {
			memcpy(tmp, mainbuf + CHKOffset(i, game), CHKLength(i, game));
			cs = ccitt16(tmp, CHKLength(i, game));
			memcpy(mainbuf + csoff + i * 8, &cs, 2);
		}
		
	else if (game == GAME_SUN || game == GAME_MOON) {
		for (u32 i = 0; i < blockCount; i++) {
			memcpy(tmp, mainbuf + CHKOffset(i, game), CHKLength(i, game));
			cs = check16(tmp, getBlockID(mainbuf, csoff, i), CHKLength(i, game));
			memcpy(mainbuf + csoff + i * 8, &cs, 2);
		}
		
		resign(mainbuf);
	}

	else if (game == GAME_B1 || game == GAME_W1 || game == GAME_B2 || game == GAME_W2) {
		blockCount = 26;
		for (u32 i = 0; i < blockCount; i++) {
			memcpy(tmp, mainbuf + CHKOffset(i, game), CHKLength(i, game));
			cs = ccitt16(tmp, CHKLength(i, game));
			memcpy(mainbuf + BWCHKOff(i, game), &cs, 2);
			memcpy(mainbuf + BWCHKMirr(i, game), &cs, 2);
		}
	}

	free(tmp);
}

void rewriteCHK4(u8 *mainbuf, int game, int GBO, int SBO) {
	u8* tmp = (u8*)malloc(0x35000 * sizeof(u8));
	u16 cs;

	if (game == GAME_DIAMOND || game == GAME_PEARL) {
		memcpy(tmp, mainbuf + GBO, 0xC0EC);
		cs = ccitt16(tmp, 0xC0EC);
		memcpy(mainbuf + GBO + 0xC0FE, &cs, 2);

		memcpy(tmp, mainbuf + SBO + 0xC100, 0x121CC);
		cs = ccitt16(tmp, 0x121CC);
		memcpy(mainbuf + SBO + 0x1E2DE, &cs, 2);			
	}
	else if (game == GAME_PLATINUM) {
		memcpy(tmp, mainbuf + GBO, 0xCF18);
		cs = ccitt16(tmp, 0xCF18);
		memcpy(mainbuf + GBO + 0xCF2A, &cs, 2);

		memcpy(tmp, mainbuf + SBO + 0xCF2C, 0x121D0);
		cs = ccitt16(tmp, 0x121D0);
		memcpy(mainbuf + SBO + 0x1F10E, &cs, 2);		
	}
	else if (game == GAME_HG || game == GAME_SS) {
		memcpy(tmp, mainbuf + GBO, 0xF618);
		cs = ccitt16(tmp, 0xF618);
		memcpy(mainbuf + GBO + 0xF626, &cs, 2);

		memcpy(tmp, mainbuf + SBO + 0xF700, 0x12300);
		cs = ccitt16(tmp, 0x12300);
		memcpy(mainbuf + SBO + 0x21A0E, &cs, 2);
	}
	free(tmp);
}