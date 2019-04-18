#pragma once
#include <string>
#include <vector>
#include "../MyTypes.h"
//FROM
//https://gist.github.com/williamdes/308b95ac9ef1ee89ae0143529c361d37

namespace Base64 {
	static const std::string b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//=
	inline std::string Encode(const std::vector<u8>& in) {
		std::string out;

		int val = 0, valb = -6;
		for (u8 c : in) {
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) {
				out.push_back(b[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6) out.push_back(b[((val << 8) >> (valb + 8)) & 0x3F]);
		while (out.size() % 4) out.push_back('=');
		return out;
	}

	inline std::vector<u8> Decode(const std::string & in) {

		std::vector<u8> out;

		std::vector<int> T(256, -1);
		for (int i = 0; i < 64; i++) T[b[i]] = i;

		int val = 0, valb = -8;
		for (u8 c : in) {
			if (T[c] == -1) break;
			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0) {
				out.push_back(u8((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}
}