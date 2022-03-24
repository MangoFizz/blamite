/*

GS SDK challenge-response algorithm 0.1
by Luigi Auriemma
e-mail: aluigi@autistici.org
web:    aluigi.org


INTRODUCTION
============
This algorithm is referred to the challenge-response method used by some
of the games that use the Gamespy SDK like Halo and Soldier of Anarchy.
This handshake is used to let valid client to join the game servers, in
fact if clients answer with a wrong response they are immediately kicked.
If we get Halo as practical example we can see that we have the
following 3 packets easily visible using a packet analyzer:
- client -> server: client challenge (a text string of 32 chars)
- server -> client: response to the client's challenge plus the
                    server's challenge
- client -> server: response calculated on the server's challenge


HOW TO USE
==========
The function gssdkcr() requires the following parameters:
- the destination buffer that will contain the resulted string.
  It must be 33 bytes long (32 plus the final NULL byte).
- the "challenge" string (sent by the server or by the client).
- the buffer containing the game's text string used for the calculation
  of the response.
  By default the Gamespy SDK uses 3b8dd8995f7c40a9a5c5b7dd5b481341 but
  some games might use different values like Soldier of Anarchy that
  uses 0AB3F935936211D19A2B080000300512 (the CLSID of the game).
  However if the value is NULL, will be used the default value.

The return value is a pointer to the destination string.


EXAMPLE
=======
In Halo for example we must use:
  char  resp[33],
        chall[] = ")nTu4y&t,Cr{P5j{6k<]^E@-ToF#Kg>m";
  gssdkcr(resp, chall, 0);

while in Soldier of Anarchy we must change this one:
  gssdkcr(resp, chall, "0AB3F935936211D19A2B080000300512");


LICENSE
=======
    Copyright 2004,2005,2006 Luigi Auriemma

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

#ifndef ALUIGI_GSSDKCR
#define ALUIGI_GSSDKCR

#include <string.h>
#include <time.h>


unsigned char *gssdkcr(
  unsigned char *dst,
  unsigned char *src,
  unsigned char *key) {

    unsigned int    oz,
                    i,
                    keysz,
                    count,
                    old,
                    tmp,
                    randnum;
    unsigned char   *ptr;
    const static char
                    key_default[] =
                    "3b8dd8995f7c40a9a5c5b7dd5b481341";

    randnum = time(NULL);   // something random
    if(!key) key = (unsigned char *)key_default;
    keysz = strlen((char *)key);

    ptr = src;
    old = *ptr;
    tmp = old < 0x4f;
    count = 0;
    for(oz = i = 1; i < 32; i++) {
        count ^= ((((*ptr < old) ^ ((old ^ i) & 1)) ^ (*ptr & 1)) ^ tmp);
        ptr++;
        if(count) {
            if(!(*ptr & 1)) { oz = 0; break; }
        } else {
            if(*ptr & 1) { oz = 0; break; }
        }
    }

    ptr = dst;
    for(i = 0; i < 32; i++, ptr++) {
        if(!oz || !i || (i == 13)) {
            randnum = (randnum * 0x343FD) + 0x269EC3;
            *ptr = (((randnum >> 16) & 0x7fff) % 93) + 33;
            continue;
        } else if((i == 1) || (i == 14)) {
            old = src[i];
        } else {
            old = src[i - 1];
        }
        tmp = (old * i) * 17991;
        old = src[(key[(src[i] + i) % keysz] + (src[i] * i)) & 31];
        *ptr = ((old ^ key[tmp % keysz]) % 93) + 33;
    }
    *ptr = 0;

    return(dst);
}

#endif
