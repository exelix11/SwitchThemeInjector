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
#include "KeyLocation.hpp"

#include <switch/types.h>

class KeyCollection {
public:
    KeyCollection();

    // get KeyLocations and find keys in them
    void get_keys();

    Key // from FS
        header_kek_source,
        header_key_source,
        key_area_key_application_source; 

    std::vector<Key *>  fs_rodata_keys;

private:
   
    // hash of empty string used to verify titlekeys for personalized tickets
    static const u8 null_hash[0x20];
};