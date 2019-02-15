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

#include <string>
#include <vector>

#include <switch/types.h>

#include <stdio.h>

typedef std::vector<u8> byte_vector;

class Key {
public:
    Key(std::string name, u64 xx_hash, byte_vector hash, u8 length, byte_vector key);
    // init with hash only
    Key(std::string name, u64 xx_hash, byte_vector hash, u8 length);
    // init with key only
    Key(std::string name, u8 length, byte_vector key);
    // temp key, no name stored
    Key(byte_vector key, u8 length);
    // key to be assigned later
    Key(std::string name, u8 length);
    // for declaration only
    Key();

    bool found() const { return is_found; }
    void set_found() { is_found = true; }

    void PrintKey();

    static const size_t get_saved_key_count() { return saved_key_count; }

    // return CTR-decrypted data
    byte_vector aes_decrypt_ctr(const byte_vector &data, byte_vector iv);
    // return ECB-decrypted data
    byte_vector aes_decrypt_ecb(const byte_vector &data);
    // return CMAC of data
    byte_vector cmac(byte_vector data);
    // find key in buffer by hash, optionally specify start offset
    void find_key(const byte_vector &buffer, size_t start = 0);
    // get key encryption key
    byte_vector generate_kek(Key &master_key, const Key &kek_seed, const Key &key_seed);

    byte_vector key;
    std::string name;
    u64 xx_hash;
    byte_vector hash;
    u8 length;
    bool is_found = false;

    static size_t saved_key_count;
};