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

#include "KeyCollection.hpp"

#include "Common.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <stdio.h>

#include <switch.h>

#define TITLEKEY_BUFFER_SIZE 0x40000

// hash of empty string
const u8 KeyCollection::null_hash[0x20] = {
        0xE3, 0xB0, 0xC4, 0x42, 0x98, 0xFC, 0x1C, 0x14, 0x9A, 0xFB, 0xF4, 0xC8, 0x99, 0x6F, 0xB9, 0x24,
        0x27, 0xAE, 0x41, 0xE4, 0x64, 0x9B, 0x93, 0x4C, 0xA4, 0x95, 0x99, 0x1B, 0x78, 0x52, 0xB8, 0x55};

FsStorage storage;

KeyCollection::KeyCollection() {
    //=====================================Hashes=====================================//
    // from FS
    header_kek_source = {"header_kek_source", 0x9fd1b07be05b8f4d, {
        0x18, 0x88, 0xca, 0xed, 0x55, 0x51, 0xb3, 0xed, 0xe0, 0x14, 0x99, 0xe8, 0x7c, 0xe0, 0xd8, 0x68,
        0x27, 0xf8, 0x08, 0x20, 0xef, 0xb2, 0x75, 0x92, 0x10, 0x55, 0xaa, 0x4e, 0x2a, 0xbd, 0xff, 0xc2}, 0x10};
    header_key_source = {"header_key_source", 0x3e7228ec5873427b, {
        0x8f, 0x78, 0x3e, 0x46, 0x85, 0x2d, 0xf6, 0xbe, 0x0b, 0xa4, 0xe1, 0x92, 0x73, 0xc4, 0xad, 0xba,
        0xee, 0x16, 0x38, 0x00, 0x43, 0xe1, 0xb8, 0xc4, 0x18, 0xc4, 0x08, 0x9a, 0x8b, 0xd6, 0x4a, 0xa6}, 0x20};
    key_area_key_application_source = {"key_area_key_application_source", 0x0b14ccce20dbb59b, {
        0x04, 0xad, 0x66, 0x14, 0x3c, 0x72, 0x6b, 0x2a, 0x13, 0x9f, 0xb6, 0xb2, 0x11, 0x28, 0xb4, 0x6f,
        0x56, 0xc5, 0x53, 0xb2, 0xb3, 0x88, 0x71, 0x10, 0x30, 0x42, 0x98, 0xd8, 0xd0, 0x09, 0x2d, 0x9e}, 0x10};
    
    fs_rodata_keys = {
        &header_kek_source,
        &key_area_key_application_source
    };
};

void KeyCollection::get_keys() {
	KeyLocation FSRodata, FSData;

    FSRodata.get_from_memory(FS_TID, SEG_RODATA);
    FSData.get_from_memory(FS_TID, SEG_DATA);

    FSRodata.find_keys(fs_rodata_keys);

    size_t i = 0;
    /*for ( ; i < FSData.data.size(); i++) {
        // speeds things up but i'm not 100% sure this is always here
        if (*reinterpret_cast<u128 *>(FSData.data.data() + i) == 0x10001)
            break;
    }*/
    header_key_source.find_key(FSData.data, i);
}
