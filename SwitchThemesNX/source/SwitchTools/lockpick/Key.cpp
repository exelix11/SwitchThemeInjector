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

#include "Key.hpp"

#include <algorithm>
#include <vector>

#include <mbedtls/aes.h>
#include <mbedtls/cmac.h>

#include "Common.hpp"
#include "xxhash64.h"

size_t Key::saved_key_count = 0;

Key::Key(std::string name, u64 xx_hash, byte_vector hash, u8 length, byte_vector key) :
    key(key),
    name(name),
    xx_hash(xx_hash),
    hash(hash),
    length(length)
{
}

// init with hash only
Key::Key(std::string name, u64 xx_hash, byte_vector hash, u8 length) :
    Key(name, xx_hash, hash, length, {})
{
}

// init with key only
Key::Key(std::string name, u8 length, byte_vector key) :
    Key(name, {}, {}, length, key)
{
    is_found = true;
}

// nameless key
Key::Key(byte_vector key, u8 length) :
    Key({}, {}, {}, length, key)
{
    is_found = true;
}

// key to be assigned later
Key::Key(std::string name, u8 length) :
    Key(name, {}, {}, length, {})
{
}

// declare only
Key::Key() :
    Key({}, {}, {}, {}, {})
{
}

void Key::PrintKey() {
    printf("%s = ", name.c_str());
	if (!found())
	{
		printf("NOT FOUND !");
        return;
	}
	
    for (auto n : key)
        printf("%02x", n);
    printf("\n");

    saved_key_count++;
}

byte_vector Key::aes_decrypt_ctr(const byte_vector &data, byte_vector iv) {
    byte_vector dest(data.size());
    if (!found())
        return dest;

    // used internally
    size_t nc_off = 0;
    u8 stream_block[0x10];
    
    mbedtls_aes_context dec;
    mbedtls_aes_init(&dec);
    mbedtls_aes_setkey_enc(&dec, key.data(), length * 8);
    mbedtls_aes_crypt_ctr(&dec, data.size(), &nc_off, iv.data(), stream_block, data.data(), dest.data());
    mbedtls_aes_free(&dec);

    return dest;
}

byte_vector Key::aes_decrypt_ecb(const byte_vector &data) {
    byte_vector dest(data.size());
    if (!found())
        return dest;

    mbedtls_aes_context dec;
    mbedtls_aes_init(&dec);
    mbedtls_aes_setkey_dec(&dec, key.data(), length * 8);
    for (size_t offset = 0; offset < data.size(); offset += 0x10)
        mbedtls_aes_crypt_ecb(&dec, MBEDTLS_AES_DECRYPT, data.data() + offset, dest.data() + offset);
    mbedtls_aes_free(&dec);

    return dest;
}

byte_vector Key::cmac(byte_vector data) {
    byte_vector dest(data.size());
    if (!found())
        return dest;

    mbedtls_cipher_cmac(mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB), key.data(), length * 8, data.data(), data.size(), dest.data());

    return dest;
}

void Key::find_key(const byte_vector &buffer, size_t start) {
    if ((buffer.size() == 0) || (found()))
        return;

    u8 temp_hash[0x20];

    if (buffer.size() == length) {
        Common::sha256(buffer.data(), temp_hash, length);
        if (!std::equal(hash.begin(), hash.end(), temp_hash))
            return;
        std::copy(buffer.begin(), buffer.begin() + length, std::back_inserter(key));
        is_found = true;
        return;
    }

    // hash every length-sized byte chunk in buffer until it matches member hash
    for (size_t i = start; i < buffer.size() - length; i++) {
        if (xx_hash == XXHash64::hash(buffer.data() + i, length, 0)) {
            // double-check sha256 since xxhash64 isn't as collision-safe
            Common::sha256(buffer.data() + i, temp_hash, length);
            if (!std::equal(hash.begin(), hash.end(), temp_hash))
                continue;
            std::copy(buffer.begin() + i, buffer.begin() + i + length, std::back_inserter(key));
            is_found = true;
            break;
        }
    }
}

 byte_vector Key::generate_kek(Key &master_key, const Key &kek_seed, const Key &key_seed) {
    Key kek(master_key.aes_decrypt_ecb(kek_seed.key), 0x10);
    Key srcKek(kek.aes_decrypt_ecb(key), 0x10);
    if (key_seed.found())
        return srcKek.aes_decrypt_ecb(key_seed.key);
    else
        return srcKek.key;
}