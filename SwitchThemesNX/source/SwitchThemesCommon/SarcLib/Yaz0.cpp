#include "Yaz0.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>

using namespace std;
vector<u8> Yaz0::Decompress(const vector<u8>& Data)
{
	if (Data.size() < 8)
		throw std::runtime_error("File format: invalid length");

	if (std::memcmp(Data.data(), "Yaz0", 4))
		throw std::runtime_error("File format: missing yaz0 magic");

	u32 leng = (u32)(Data[4] << 24 | Data[5] << 16 | Data[6] << 8 | Data[7]);
	vector<u8> Result(leng);
	int Offs = 16;
	u32 dstoffs = 0;
	while (true)
	{
		u8 header = Data[Offs++];
		for (int i = 0; i < 8; i++)
		{
			if ((header & 0x80) != 0) Result.at(dstoffs++) = Data.at(Offs++);
			else
			{
				u8 b = Data.at(Offs++);
				int offs = ((b & 0xF) << 8 | Data.at(Offs++)) + 1;
				int length = (b >> 4) + 2;
				if (length == 2) length = Data.at(Offs++) + 0x12;
				for (int j = 0; j < length; j++)
				{
					Result.at(dstoffs) = Result.at(dstoffs - offs);
					dstoffs++;
				}
			}
			if (dstoffs >= leng) return Result;
			header <<= 1;
		}
	}
}

vector<u8> Yaz0::Compress(const vector<u8> &Data, int level, int reserved1, int reserved2)
{
	level = std::clamp(level, 1, 9);
	const int maxBackLevel = (int)(0x10e0 * (level / 9.0) - 0x0e0);

	vector<u8> result(Data.size() + Data.size() / 8 + 0x10);

#if _MSC_VER
	//Msvc is way too slow using iterators in debug builds
	auto sourceptr = &Data[0];
	auto resultptr = &result[0];
#else
	auto sourceptr = Data.begin();
	auto resultptr = result.begin();
#endif

	*resultptr++ = (u8)'Y';
	*resultptr++ = (u8)'a';
	*resultptr++ = (u8)'z';
	*resultptr++ = (u8)'0';

	*resultptr++ = (u8)((Data.size() >> 24) & 0xFF);
	*resultptr++ = (u8)((Data.size() >> 16) & 0xFF);
	*resultptr++ = (u8)((Data.size() >> 8) & 0xFF);
	*resultptr++ = (u8)((Data.size() >> 0) & 0xFF);
	{
		u8 tmp[4];

		std::memcpy(tmp, &reserved1, sizeof(tmp));
		*resultptr++ = tmp[3];
		*resultptr++ = tmp[2];
		*resultptr++ = tmp[1];
		*resultptr++ = tmp[0];
		
		std::memcpy(tmp, &reserved2, sizeof(tmp));
		*resultptr++ = tmp[3];
		*resultptr++ = tmp[2];
		*resultptr++ = tmp[1];
		*resultptr++ = tmp[0];
	}
	int length = Data.size();
	int dstoffs = 16;
	int Offs = 0;
	while (true)
	{
		int headeroffs = dstoffs++;
		resultptr++;
		u8 header = 0;
		for (int i = 0; i < 8; i++)
		{
			int comp = 0;
			int back = 1;
			int nr = 2;
			if (Offs)
			{
				auto ptr = sourceptr - 1;
				const int maxnum = std::min(length - Offs, 0x111);
				const int maxback = std::min(Offs, maxBackLevel);
				
				auto maxbackptr = sourceptr - maxback;
				int tmpnr;
				while (maxbackptr < ptr)
				{
					if (ptr[0] == sourceptr[0] && ptr[1] == sourceptr[1] && ptr[2] == sourceptr[2])
					{
						tmpnr = 3;
						while (tmpnr < maxnum && ptr[tmpnr] == sourceptr[tmpnr]) tmpnr++;
						if (tmpnr > nr)
						{
							if (Offs + tmpnr > length)
							{
								nr = length - Offs;
								back = (int)(sourceptr - ptr);
								break;
							}
							nr = tmpnr;
							back = (int)(sourceptr - ptr);
							if (nr == maxnum) break;
						}
					}
					--ptr;
				}
			}
			if (nr > 2)
			{
				Offs += nr;
				sourceptr += nr;
				if (nr >= 0x12)
				{
					*resultptr++ = (u8)(((back - 1) >> 8) & 0xF);
					*resultptr++ = (u8)((back - 1) & 0xFF);
					*resultptr++ = (u8)((nr - 0x12) & 0xFF);
					dstoffs += 3;
				}
				else
				{
					*resultptr++ = (u8)((((back - 1) >> 8) & 0xF) | (((nr - 2) & 0xF) << 4));
					*resultptr++ = (u8)((back - 1) & 0xFF);
					dstoffs += 2;
				}
				comp = 1;
			}
			else
			{
				*resultptr++ = *sourceptr++;
				dstoffs++;
				Offs++;
			}
			header = (u8)((header << 1) | ((comp == 1) ? 0 : 1));
			if (Offs >= length)
			{
				header = (u8)(header << (7 - i));
				break;
			}
		}
		result[headeroffs] = header;
		if (Offs >= length) break;
	}
	while ((dstoffs % 4) != 0) dstoffs++;
	
	result.resize(dstoffs);
	return result;
}