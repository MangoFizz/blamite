/*

Halo packets decryption/encryption algorithm and keys builder 0.1.3
by Luigi Auriemma
e-mail: aluigi@autistici.org
web:    aluigi.org


INTRODUCTION
============
The famous game called Halo (I talk about the PC version) uses
encrypted packets and the set of functions available here is all you
need to decrypt and encrypt the packets of this game.
It's a bit complex to explain the details of the algorithm moreover for
me since I have no knowledge of cryptography, however it uses the TEA
algorithm to encrypt and decrypt the packets and exist 2 keys exchanged
between the 2 hosts plus a private key (a random hash) for each one.
It's not possible for a third person to decrypt the data between them
due to the usage of this nice method to handle keys, so capturing the
exchanged keys will not let you to decrypt the data.
FYI, the data in the packets is stored in bitstream format and the
latest 4 bytes are a classical 32 bits checksum of the packet, so keep
that in mind when you want to analyze the data.


HOW TO USE
==========
First, you need to specify the following buffers in your program:

  u_char    enckey[16],  // used to encrypt
            deckey[16],  // used to decrypt
            hash[17];    // the private key, it's 17 bytes long (NULL)

You need only 3 functions to do everything but there are many others
available in this file so you have the maximum freedom of using your
preferred way to handle the keys and the data:

- halo_generate_keys()
  needs 3 arguments: the random hash, the source key and the
  destination key
  All these fields are automatically zeroed when needed so you must do
  nothing.
  This function must be called the first time to send the key to the
  other host and other 2 consecutive times to calculate the decryption
  and encryption key.
  The hash field is just your private key which is random.

  To create your key use NULL as source key, a buffer of 17 bytes for
  the private key and a destination buffer of 16 bytes that will contain
  the generated key.
    Example:    halo_generate_keys(hash, NULL, enckey);
                // you can use enckey or a temporary buffer too

  To create the decryption and encryption keys use the key received from
  the other host as source and a buffer of 16 bytes as destination.
  The hash is ever the same, you must not touch it.
    Example:    halo_generate_keys(hash, packet_buffer + 7, deckey);
                halo_generate_keys(hash, packet_buffer + 7, enckey);
                // "packet_buffer + 7" is where is located the received
                // key

- void halo_tea_decrypt()
  needs 3 arguments, the buffer to decrypt, its size and the decryption
  key previously generated with the halo_generate_keys() function:
    halo_tea_decrypt(buffer, len, deckey);

- void halo_tea_encrypt()
  needs 3 arguments, the buffer to encrypt, its size and the encryption
  key previously generated:
    halo_tea_encrypt(buffer, len, enckey);

Useful is also the halo_crc32() function that calculates the CRC number
that must be placed at the end of each packet. The data that must be
passed to the function usually starts at offset 7 of each packet, the
same resulted from the decryption/encryption. Remember that the size
must not contain the last 4 bytes occupied by the checksum.
  Example:      halo_crc32(packet_buffer + 7, packet_len - 7 - 4);


REAL EXAMPLES
=============
Check the stuff I have written for Halo on my website and my
proof-of-concept for a vulnerability I found:

  http://aluigi.org/papers.htm#halo
  http://aluigi.org/poc/haloloop.zip


LICENSE
=======
    Copyright 2005,2006 Luigi Auriemma

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

    http://www.gnu.org/licenses/gpl.txt
*/

#ifndef ALUIGI_PCK_ALGO_H
#define ALUIGI_PCK_ALGO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#ifdef WIN32            // something better than a simple time(NULL)
    #include <windows.h>
    #define HALO_RAND   (uint32_t)GetTickCount()    // 1000/s resolution
#else
    #include <sys/times.h>
    #define HALO_RAND   (uint32_t)times(0)          // 100/s resolution
#endif



void halo_create_randhash(uint8_t *out) {
    uint32_t            randnum;
    int                 i;
    const static char   hex[17] = "0123456789ABCDEF";

    randnum = HALO_RAND;
    for(i = 0; i < 16; i++) {
        randnum = (randnum * 0x343FD) + 0x269EC3;
        *out++ = hex[(randnum >> 16) & 15];
    }
    *out = 0;
}



void halo_byte2hex(uint8_t *in, uint8_t *out) {
    int                 i;
    const static char   hex[17] = "0123456789ABCDEF";

    for(i = 16; i; i--) {
        if(*in) break;
        in++;
    }
    while(i--) {
        *out++ = hex[*in >> 4];
        *out++ = hex[*in & 15];
        in++;
    }
    *out = 0;
}



void halo_hex2byte(uint8_t *in, uint8_t *out) {
    int     i,
            j,
            t;

    memset(out, 0, 16);
    while(*in) {
        for(j = 0; j < 4; j++) {
            t = 0;
            for(i = 15; i >= 0; i--) {
                t += (out[i] << 1);
                out[i] = t;
                t >>= 8;
            }
        }
        t = *in |= 0x20;
        out[15] |= ((t - (0x27 * (t > 0x60))) - 0x30);
        in++;
    }
}



void halo_fix_check(uint8_t *key1, uint8_t *key2) {
    int     i,
            j;

    for(i = 0; i < 16; i++) {
        if(key1[i] != key2[i]) break;
    }
    if((i < 16) && (key1[i] > key2[i])) {
        for(j = 0, i = 16; i--; j >>= 8) {
            j += (key1[i] - key2[i]);
            key1[i] = j;
        }
    }
}



void halo_key_scramble(uint8_t *key1, uint8_t *key2, uint8_t *fixnumb) {
    int     i,
            j,
            cnt;
    uint8_t tk1[16],
            tk2[16];

    memcpy(tk1,  key1, 16);
    memcpy(tk2,  key2, 16);
    memset(key1, 0,    16);

    cnt = 16 << 3;
    while(cnt--) {
        if(tk1[15] & 1) {
            for(j = 0, i = 16; i--; j >>= 8) {
                j += key1[i] + tk2[i];
                key1[i] = j;
            }
            halo_fix_check(key1, fixnumb);
        }

        for(j = i = 0; i < 16; i++, j <<= 8) {
            j |= tk1[i];
            tk1[i] = j >> 1;
            j &= 1;
        }

        for(j = 0, i = 16; i--; j >>= 8) {
            j += (tk2[i] << 1);
            tk2[i] = j;
        }
        halo_fix_check(tk2, fixnumb);
    }
}



void halo_create_key(uint8_t *keystr, uint8_t *randhash, uint8_t *fixnum, uint8_t *dest) {
    int     i,
            j,
            cnt;
    uint8_t keystrb[16],
            randhashb[16],
            fixnumb[16];

    halo_hex2byte(keystr,   keystrb);
    halo_hex2byte(randhash, randhashb);
    halo_hex2byte(fixnum,   fixnumb);

    memset(dest, 0, 16);
    dest[15] = 0x01;

    cnt = 16 << 3;
    while(cnt--) {
        if(randhashb[15] & 1) {
            halo_key_scramble(dest, keystrb, fixnumb);
        }
        halo_key_scramble(keystrb, keystrb, fixnumb);

        for(j = i = 0; i < 16; i++, j <<= 8) {
            j |= randhashb[i];
            randhashb[i] = j >> 1;
            j &= 1;
        }
    }
}



void tea_decrypt(uint32_t *p, uint32_t *keyl) {
    uint32_t    y,
                z,
                sum,
                a = keyl[0],
                b = keyl[1],
                c = keyl[2],
                d = keyl[3];
    int         i;

    y = p[0];
    z = p[1];
    sum = 0xc6ef3720;
    for(i = 0; i < 32; i++) {
        z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
        y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
        sum -= 0x9e3779b9;
    }
    p[0] = y;
    p[1] = z;
}



void halo_tea_decrypt(uint8_t *data, int size, uint8_t *key) {
    uint32_t    *p    = (uint32_t *)data,
                *keyl = (uint32_t *)key;

    if(size & 7) {
        tea_decrypt((uint32_t *)(data + size - 8), keyl);
    }

    size >>= 3;
    while(size--) {
        tea_decrypt(p, keyl);
        p += 2;
    }
}



void tea_encrypt(uint32_t *p, uint32_t *keyl) {
    uint32_t    y,
                z,
                sum,
                a = keyl[0],
                b = keyl[1],
                c = keyl[2],
                d = keyl[3];
    int         i;

    y = p[0];
    z = p[1];
    sum = 0;
    for(i = 0; i < 32; i++) {
        sum += 0x9e3779b9;
        y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
        z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
    }
    p[0] = y;
    p[1] = z;
}



void halo_tea_encrypt(uint8_t *data, int size, uint8_t *key) {
    uint32_t    *p    = (uint32_t *)data,
                *keyl = (uint32_t *)key;
    int         rest  = size & 7;

    size >>= 3;
    while(size--) {
        tea_encrypt(p, keyl);
        p += 2;
    }

    if(rest) {
        tea_encrypt((uint32_t *)((uint8_t *)p - (8 - rest)), keyl);
    }
}



void halo_generate_keys(uint8_t *hash, uint8_t *source_key, uint8_t *dest_key) {
    uint8_t tmp_key[33],
            fixed_key[33];

    strcpy((char *)fixed_key, "10001"); // key 1

    if(!source_key) {           // encryption
        strcpy((char *)tmp_key, "3");   // key 2
        halo_create_randhash(hash);
    } else {
        halo_byte2hex(source_key, tmp_key);
    }

    source_key = tmp_key;
    halo_create_key(source_key, hash, fixed_key, dest_key);
}



uint32_t halo_crc32(uint8_t *data, int size) {
    const static uint32_t   crctable[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
        0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
        0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
        0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
        0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
        0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
        0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
        0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
        0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
        0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
        0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
        0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
        0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
        0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
        0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
        0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
        0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
        0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
        0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
        0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
        0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
        0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
        0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
        0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
        0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
        0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
        0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
        0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
        0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
        0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
        0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
        0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
        0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
        0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
        0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
        0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
        0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
        0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
        0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
    };
    uint32_t    crc = 0xffffffff;

    while(size--) {
        crc = crctable[(*data ^ crc) & 0xff] ^ (crc >> 8);
        data++;
    }
    return(crc);
}



#undef HALO_RAND

#endif
