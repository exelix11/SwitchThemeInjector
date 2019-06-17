#include "Bflyt.hpp"

using namespace std;
using namespace Panes;

BflytMaterial::BflytMaterial(const vector<u8>& data, u32 Version, Endianness bo) 
{
	Data = data;
	Buffer buf{ Data };
	buf.ByteOrder = bo;
	Name = buf.readStr_Fixed(28);
	if (Version >= 0x08000000)
	{
		buf.readUInt64();
		ForegroundColor = buf.readUInt32_LE();
		BackgroundColor = buf.readUInt32_LE();
	}
	else
	{
		ForegroundColor = buf.readUInt32_LE();
		BackgroundColor = buf.readUInt32_LE();
	}
}

vector<u8> BflytMaterial::Write(u32 version, Endianness bo) 
{
	Buffer buf{Data};
	buf.ByteOrder = bo;
	buf.Position = 0;

	buf.WriteFixedLengthString(Name, 28);
	if (version >= 0x08000000)
	{
		buf.Position += 8;
		buf.writeUInt32_LE(ForegroundColor);
		buf.writeUInt32_LE(BackgroundColor);
	}
	else 
	{
		buf.writeUInt32_LE(ForegroundColor);
		buf.writeUInt32_LE(BackgroundColor);
	}

	return buf.getBuffer();
}