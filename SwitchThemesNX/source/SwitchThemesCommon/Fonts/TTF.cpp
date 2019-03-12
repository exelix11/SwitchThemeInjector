#include "TTF.hpp"
#include <cstring>

using namespace std;

string SwitchThemesCommon::TTF::GetFontName(const vector<u8> &Data) 
{
	Buffer buf(Data);
	buf.ByteOrder = Endianness::BigEndian;
	if (buf.readUInt16() != 1) //major version 
		return "";
	if (buf.readUInt16() != 0) //minor version 
		return "";
	u16 tableCount = buf.readUInt16();
	buf.Position += 6; //Go to start of the first table
	u32 NameTableOff = 0, NameTableLen = 0;
	for (int i = 0; i < tableCount; i++)
	{
		auto tableName = buf.readBytes(4);
		if (memcmp(tableName.data(), "name", 4) != 0)
		{
			buf.Position += 0xC;
			continue;
		}
		buf.Position += 4;
		NameTableOff = buf.readUInt32();
		NameTableLen = buf.readUInt32();
		break;
	}
	if (NameTableOff == 0 || NameTableLen == 0) return "";

	buf.Position = NameTableOff;
	if (buf.readUInt16() != 0) return ""; //The font name table header starts with 0
	u16 NamesCount = buf.readUInt16();
	u32 StorageOffset = NameTableOff + buf.readUInt16();
	for (int i = 0; i < NamesCount; i++)
	{
		buf.Position += 2 * 3; //skip 3 fields
		if (buf.readUInt16() == 1) //NameId is the font name
		{
			u16 StringLength = buf.readUInt16();
			u16 StringOff = buf.readUInt16();
			buf.Position = StorageOffset + StringOff;
			return buf.readStr(StringLength);
		}
		else buf.Position += 2 * 2; //skip two fields and go to the start of the next entry
	}
}

vector<u8> SwitchThemesCommon::TTF::ConvertToBFTTF(const vector<u8> &Data) 
{
	const u32 Magic = 0x06186249;
	Buffer Out, In(Data);
	Out.Write(0x18029a7f ^ Magic);
	
	u32 DSize = Data.size();
	u32 BeSize = ((DSize & 0x000000ff) << 24) + ((DSize & 0x0000ff00) << 8) +
				 ((DSize & 0x00ff0000) >> 8) + ((DSize & 0xff000000) >> 24);

	Out.Write(BeSize ^ Magic);

	for (int i = 0; i < Data.size() / 4; i++)
		Out.Write(In.readUInt32() ^ Magic);

	return Out.getBuffer();
}