/*
 * Copyright (c) 2018 shchmue
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Key.hpp"

#include <vector>

#include <switch/types.h>

#define FS_TID      0x0100000000000000
#define SSL_TID     0x0100000000000024
#define SPL_TID     0x0100000000000028
#define ES_TID      0x0100000000000033

#define FIRMA_TID   0x0100000000000819

#define ES_COMMON_SAVE_ID       0x80000000000000E1
#define ES_PERSONALIZED_SAVE_ID 0x80000000000000E2

#define SEG_TEXT    BIT(0)
#define SEG_RODATA  BIT(1)
#define SEG_DATA    BIT(2)

#define KNOWN_KEYBLOBS 6
#define KNOWN_MASTER_KEYS 7

#define KEYBLOB_OFFSET 0x180000

typedef std::vector<u8> byte_vector;

class KeyLocation {
public:
    // get memory in requested segments from running title
    void get_from_memory(u64 tid, u8 seg_mask);
    // get keyblobs from BOOT0
    void get_keyblobs();
    // locate keys in data
    void find_keys(std::vector<Key *> &keys);

    // data found by get functions
    byte_vector data;
};