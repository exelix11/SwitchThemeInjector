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

class Key;

typedef std::vector<u8> byte_vector;

namespace Common {
    void sha256(const u8 *data, u8 *hash, size_t length);
    // reads "<keyname> = <hexkey>" and returns byte vector
    byte_vector key_string_to_byte_vector(std::string key_string);
}