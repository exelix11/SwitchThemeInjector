#include "Bflan.hpp"

using namespace std;

KeyFrame::KeyFrame() {}
KeyFrame::KeyFrame(Buffer& bin, u16 DataType)
{
	Frame = bin.readFloat();
	if (DataType == 2)
	{
		Value = bin.readFloat();
		Blend = bin.readFloat();
	}
	else if (DataType == 1)
	{
		Value = (float)bin.readInt16();
		Blend = (float)bin.readInt16();
	}
	else throw "Unexpected data type for keyframe";
}

PaiTagEntry::PaiTagEntry() {}
PaiTagEntry::PaiTagEntry(Buffer& bin, std::string TagName)
{
	u32 tagStart = (u32)bin.Position;
	Index = bin.readUInt8();
	AnimationTarget = bin.readUInt8();
	DataType = bin.readUInt16();
	auto KeyFrameCount = bin.readUInt16();
	bin.readUInt16(); //Padding
	bin.Position = tagStart + bin.readUInt32(); //offset to first keyframe
	for (int i = 0; i < KeyFrameCount; i++)
		KeyFrames.emplace_back(bin, DataType);
	if (TagName == "FLEU")
	{
		FLEUUnknownInt = bin.readUInt32();
		FLEUEntryName = bin.readStr_NullTerm();
	}
}

void PaiTagEntry::Write(Buffer& bin, std::string TagName) 
{
	u32 tagStart = (u32)bin.Position;
	bin.Write(Index);
	bin.Write(AnimationTarget);
	bin.Write(DataType);
	bin.Write((u16)KeyFrames.size());
	bin.Write((u16)0);
	bin.Write((u32)bin.Position - tagStart + 4);
	for (int i = 0; i < KeyFrames.size(); i++)
	{
		bin.Write(KeyFrames[i].Frame);
		if (DataType == 2)
		{
			bin.Write(KeyFrames[i].Value);
			bin.Write(KeyFrames[i].Blend);
		}
		else if (DataType == 1)
		{
			bin.Write((u16)KeyFrames[i].Value);
			bin.Write((u16)KeyFrames[i].Blend);
		}
		else throw "Unexpected data type for KeyFrame";
	}
	if (TagName == "FLEU")
	{
		bin.Write(FLEUUnknownInt);
		bin.Write(FLEUEntryName, Buffer::BinaryString::NullTerminated);
		while (bin.Position % 4 != 0)
			bin.Write((u8)0);
	}
}

PaiTag::PaiTag() {}
PaiTag::PaiTag(Buffer& bin, u8 TargetType) 
{
	if (TargetType == 2)
		Unknown = bin.readUInt32(); //This doesn't seem to be included in the offsets to the entries (?)
	auto sectionStart = (u32)bin.Position;
	TagType = bin.readStr(4);
	auto entryCount = bin.readUInt32();
	vector<u32> EntryOffsets;
	for (int i = 0; i < entryCount; i++)
		EntryOffsets.push_back(bin.readUInt32());
	for (int i = 0; i < entryCount; i++)
	{
		bin.Position = EntryOffsets[i] + sectionStart;
		Entries.emplace_back(bin, TagType);
	}
}

void PaiTag::Write(Buffer& bin, u8 TargetType) 
{
	if (TargetType == 2)
		bin.Write(Unknown);
	auto sectionStart = (u32)bin.Position;
	bin.Write(TagType, Buffer::BinaryString::NoPrefixOrTermination);
	bin.Write((u32)Entries.size());
	auto EntryTable = bin.Position;
	for (int i = 0; i < Entries.size(); i++)
		bin.Write((u32)0);
	for (int i = 0; i < Entries.size(); i++)
	{
		auto oldpos = bin.Position;
		bin.Position = EntryTable + i * 4;
		bin.Write((u32)oldpos - sectionStart);
		bin.Position = oldpos;
		Entries[i].Write(bin, TagType);
	}
}

PaiEntry::PaiEntry() {}
PaiEntry::PaiEntry(Buffer& bin) 
{
	u32 SectionStart = (u32)bin.Position;
	Name = bin.readStr_Fixed(28);
	auto tagCount = bin.readUInt8();
	Target = (AnimationTarget)bin.readUInt8();
	bin.readUInt16(); //padding
	vector<u32> TagOffsets;
	for (int i = 0; i < tagCount; i++)
		TagOffsets.push_back(bin.readUInt32());
	if (tagCount == 0) return;
	UnkwnownData = bin.readBytes((int)(TagOffsets[0] + SectionStart - bin.Position));
	for (int i = 0; i < tagCount; i++)
	{
		bin.Position = TagOffsets[i] + SectionStart;
		Tags.emplace_back(bin, (u8)Target);
	}
}

void PaiEntry::Write(Buffer& bin) 
{
	auto SectionStart = (u32)bin.Position;
	bin.WriteFixedLengthString(Name, 28);
	bin.Write((u8)Tags.size());
	bin.Write((u8)Target);
	bin.Write((u16)0);
	auto tagTable = bin.Position;
	for (int i = 0; i < Tags.size(); i++)
		bin.Write((u32)0);
	bin.Write(UnkwnownData);
	for (int i = 0; i < Tags.size(); i++)
	{
		auto oldPos = (u32)bin.Position;
		bin.Position = tagTable + i * 4;
		bin.Write((u32)(oldPos - SectionStart));
		bin.Position = oldPos;
		Tags[i].Write(bin, (u8)Target);
	}
}

BflanSection::BflanSection(const std::string& _name) : TypeName(_name){}
BflanSection::BflanSection(const std::string& _name, const std::vector<u8>& data) :
TypeName(_name), Data(data)
{}

void BflanSection::BuildData(Endianness byteOrder) {}
void BflanSection::Write(Buffer& bin) 
{
	if (TypeName.size() != 4) throw "unexpected type len";
	BuildData(bin.ByteOrder);
	bin.Write(TypeName, Buffer::BinaryString::NoPrefixOrTermination);
	bin.Write((u32)Data.size() + 8);
	bin.Write(Data);
}

Pat1Section::Pat1Section() : BflanSection("pat1") {}
Pat1Section::Pat1Section(const std::vector<u8>& data, Endianness bo) : BflanSection("pat1", data)
{
	ParseData(bo);
}

void Pat1Section::ParseData(Endianness bo) 
{
	Buffer bin{ Data };
	bin.ByteOrder = bo;
	AnimationOrder = bin.readUInt16();
	auto groupCount = bin.readUInt16();
	if (groupCount != 1) throw "File with unexpected group count";
	auto animName = bin.readUInt32() - 8; //all offsets are shifted by 8 cause this byte block doesn't include the section name and size
	auto groupNames = bin.readUInt32() - 8;
	Unk_StartOfFile = bin.readUInt16();
	Unk_EndOfFile = bin.readUInt16();
	ChildBinding = bin.readUInt8();
	Unk_EndOfHeader = bin.readBytes((int)animName - (int)bin.Position);
	bin.Position = animName;
	Name = bin.readStr_NullTerm();
	for (int i = 0; i < groupCount; i++)
	{
		bin.Position = groupNames + i * groupNameLen;
		Groups.push_back(bin.readStr_Fixed(groupNameLen));
	}
	if (Unk_StartOfFile != 0 || Unk_EndOfFile != 0)
	{
		int a = 0; //breakpoint here
		//this has never been the case till now and i have no clue what those two values mean
	}
}

void Pat1Section::BuildData(Endianness byteOrder) 
{
	Buffer bin;
	bin.ByteOrder = byteOrder;
	bin.Write((u16)AnimationOrder);
	bin.Write((u16)Groups.size());
	auto UpdateOffsetsPos = bin.Position;
	bin.Write((u32)0);
	bin.Write((u32)0);
	bin.Write(Unk_StartOfFile);
	bin.Write(Unk_EndOfFile);
	bin.Write(ChildBinding);
	bin.Write(Unk_EndOfHeader);
	auto oldPos = bin.Position;
	bin.Position = UpdateOffsetsPos;
	bin.Write((u32)oldPos + 8); //name offset
	bin.Position = oldPos;
	bin.Write(Name, Buffer::BinaryString::NullTerminated);
	while (bin.Position % 4 != 0)
		bin.Write((u8)0);
	oldPos = bin.Position;
	bin.Position = UpdateOffsetsPos + 4; //Group name table
	bin.Write((u32)oldPos + 8);
	bin.Position = oldPos;
	for (int i = 0; i < Groups.size(); i++)
		bin.WriteFixedLengthString(Groups[i], groupNameLen);
	Data = bin.getBuffer();
}

Pai1Section::Pai1Section() : BflanSection("pai1") {}
Pai1Section::Pai1Section(const std::vector<u8>& data, Endianness bo) :
BflanSection("pai1", data)
{
	ParseData(bo);
}

void Pai1Section::ParseData(Endianness bo)
{
	Buffer bin{ Data };
	bin.ByteOrder = bo;
	FrameSize = bin.readUInt16();
	Flags = bin.readUInt8();
	bin.readUInt8(); //padding
	auto texCount = bin.readUInt16();
	auto entryCount = bin.readUInt16();
	auto entryTable = bin.readUInt32() - 8;
	if (texCount != 0)
	{
		auto texTableStart = bin.Position;
		vector<u32> offsets;
		for (int i = 0; i < texCount; i++)
			offsets.push_back(bin.readUInt32());
		for (int i = 0; i < texCount; i++)
		{
			bin.Position = texTableStart + offsets[i];
			Textures.push_back(bin.readStr_NullTerm());
		}
	}
	for (int i = 0; i < entryCount; i++)
	{
		bin.Position = entryTable + i * 4;
		bin.Position = bin.readUInt32() - 8;
		Entries.emplace_back(bin);
	}
}

void Pai1Section::BuildData(Endianness bo) 
{
	Buffer bin;
	bin.ByteOrder = bo;
	bin.Write(FrameSize);
	bin.Write(Flags);
	bin.Write((u8)0);
	bin.Write((u16)Textures.size());
	bin.Write((u16)Entries.size());
	auto updateOffsets = bin.Position;
	bin.Write((u32)0);
	if (Textures.size() != 0)
	{
		auto texTableStart = bin.Position;

		for (int i = 0; i < Textures.size(); i++)
			bin.Write((u32)0);

		for (int i = 0; i < Textures.size(); i++)
		{
			auto texPos = bin.Position;
			bin.Write(Textures[i], Buffer::BinaryString::NullTerminated);
			auto endPos = bin.Position;
			bin.Position = texTableStart + i * 4;
			bin.Write((u32)(texPos - texTableStart));
			bin.Position = endPos;
		}

		while (bin.Position % 4 != 0)
			bin.Write((u8)0);
	}
	auto EntryTableStart = bin.Position;
	bin.Position = updateOffsets;
	bin.Write((u32)EntryTableStart + 8);
	bin.Position = EntryTableStart;
	for (int i = 0; i < Entries.size(); i++)
		bin.Write((u32)0);

	for (int i = 0; i < Entries.size(); i++)
	{
		auto oldpos = bin.Position;
		bin.Position = EntryTableStart + 4 * i;
		bin.Write((u32)oldpos + 8);
		bin.Position = oldpos;
		Entries[i].Write(bin);
	}

	Data = bin.getBuffer();
}

Bflan::~Bflan() 
{
	for (BflanSection* ptr : Sections)
		delete ptr;
	Sections.clear();
}

Bflan::Bflan() {}
Bflan::Bflan(const vector<u8>& data) {
	Buffer bin{ data };
	ParseFile(bin);
}

vector<u8> Bflan::WriteFile() 
{
	Buffer bin;
	bin.ByteOrder = byteOrder;
	bin.Write("FLAN", Buffer::BinaryString::NoPrefixOrTermination);
	bin.Write((u16)0xFEFF);
	bin.Write((u16)0x14);
	bin.Write(Version);
	bin.Write((u32)0); //Filesize
	bin.Write((u16)Sections.size());
	bin.Write((u16)0);

	for (int i = 0; i < Sections.size(); i++)
		Sections[i]->Write(bin);

	bin.Position = 0xC;
	bin.Write((u32)bin.Length());
	return bin.getBuffer();
}

void Bflan::ParseFile(Buffer& bin) 
{
	if (bin.readStr(4) != "FLAN")
		throw "Wrong bflan magic";
	u8 BOM = bin.readUInt8();
	if (BOM == 0xFF) byteOrder = Endianness::LittleEndian;
	else if (BOM == 0xFE) byteOrder = Endianness::BigEndian;
	else throw "Unexpected BFLAN BOM";
	bin.ByteOrder = byteOrder;
	bin.readUInt8(); //Second byte of the byte order mask
	if (bin.readUInt16() != 0x14) throw "Unexpected bflan header size";
	Version = bin.readUInt32();
	bin.readUInt32(); //FileSize
	auto sectionCount = bin.readUInt16();
	bin.readUInt16(); //padding ?

	for (int i = 0; i < sectionCount; i++)
	{
		string sectionName = bin.readStr(4);
		s32 sectionSize = bin.readInt32(); //this includes the first 8 bytes we read here
		auto sectionData = bin.readBytes(sectionSize - 8);
		
		if (sectionName == "pat1")
			Sections.push_back((BflanSection*) new Pat1Section(sectionData, bin.ByteOrder));
		else if (sectionName == "pai1")
			Sections.push_back((BflanSection*) new Pai1Section(sectionData, bin.ByteOrder));
		else
			throw "unexpected section";
	}
}

#include "Base64.hpp"
#include "json.hpp"

Bflan *BflanDeserializer::FromJson(std::string jsn) 
{
	Bflan *res = new Bflan;
	auto j = nlohmann::json::parse(jsn);

	res->byteOrder = j["LittleEndian"].get<bool>() ? Endianness::LittleEndian : Endianness::BigEndian;
	res->Version = j["Version"].get<u32>();

	{
		auto pat1 = j["pat1"];
		Pat1Section *p = new Pat1Section();
#define SetVal(x) p->x = pat1[#x]
		//p->AnimationOrder = j["AnimationOrder"].get<u16>();
		SetVal(AnimationOrder).get<u16>();
		SetVal(Name).get<string>();
		SetVal(ChildBinding).get<u8>();
		SetVal(Unk_StartOfFile).get<u16>();
		SetVal(Unk_EndOfFile).get<u16>();
		SetVal(Groups).get<vector<string>>();
		p->Unk_EndOfHeader = Base64::Decode(pat1["Unk_EndOfHeader"]);
#undef SetVal
		res->Sections.push_back((BflanSection*)p);
	}

	{
		auto pai1 = j["pai1"];
		Pai1Section* p = new Pai1Section();
#define SetVal(x) p->x = pai1[#x]
		SetVal(FrameSize).get<u16>();
		SetVal(Flags).get<u8>();
		SetVal(Textures).get<vector<string>>();
#undef SetVal

		for (auto& Entry : pai1["Entries"])
		{
			PaiEntry e;
			e.Name = Entry["Name"];
			e.Target = (PaiEntry::AnimationTarget)Entry["Target"].get<u8>();
			e.UnkwnownData = Base64::Decode(Entry["UnkwnownData"]);
			for (auto& Tag : Entry["Tags"])
			{
				PaiTag t;
				t.Unknown = Tag["Unknown"].get<u32>();
				t.TagType = Tag["TagType"];
				for (auto& Entry : Tag["Entries"])
				{
					PaiTagEntry e;
#define SetVal(x) e.x = Entry[#x]
					SetVal(Index).get<u8>();
					SetVal(AnimationTarget).get<u8>();
					SetVal(DataType).get<u16>();
					SetVal(FLEUUnknownInt).get<u32>();
					SetVal(FLEUEntryName).get<string>();
#undef SetVal
					for (auto& key : Entry["KeyFrames"])
					{
						KeyFrame k;
#define SetVal(x) k.x = key[#x]
						SetVal(Frame).get<float>();
						SetVal(Value).get<float>();
						SetVal(Blend).get<float>();
#undef SetVal
						e.KeyFrames.push_back(k);
					}
					t.Entries.push_back(e);
				}
				e.Tags.push_back(t);
			}
			p->Entries.push_back(e);
		}

		res->Sections.push_back((BflanSection*)p);
	}

	return res;
}