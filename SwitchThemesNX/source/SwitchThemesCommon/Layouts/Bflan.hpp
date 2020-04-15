#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "../BinaryReadWrite/Buffer.hpp"
#include "../MyTypes.h"
#include "Patches.hpp"

class BflanSection
{
public:
	std::string TypeName;
	std::vector<u8> Data;

	BflanSection(const std::string& _name);
	BflanSection(const std::string& _name, const std::vector<u8> &data);
	
	virtual void BuildData(Endianness byteOrder);
	virtual void Write(Buffer& buf);

	virtual ~BflanSection() {};
};

class Pat1Section : public BflanSection 
{
private:
	const int groupNameLen = 0x24;
public:
	u16 AnimationOrder;
	std::string Name;
	u8 ChildBinding;
	std::vector<std::string> Groups;

	u16 Unk_StartOfFile;
	u16 Unk_EndOfFile;
	std::vector<u8> Unk_EndOfHeader;

	Pat1Section();
	Pat1Section(const std::vector<u8> &data, Endianness bo);

	~Pat1Section() {}

	void ParseData(Endianness bo);
	void BuildData(Endianness byteOrder) override;
};

class KeyFrame 
{
public:
	float Frame, Value, Blend;

	KeyFrame(Buffer& buf, u16 DataType);
	KeyFrame();
};

class PaiTagEntry 
{
public:
	u8 Index;
	u8 AnimationTarget;
	u16 DataType;
	std::vector<KeyFrame> KeyFrames;

	u32 FLEUUnknownInt;
	std::string FLEUEntryName;

	PaiTagEntry();
	PaiTagEntry(Buffer& buf, std::string TagName);

	void Write(Buffer& buf, std::string TagName);
};

class PaiTag 
{
public:
	u32 Unknown;
	std::string TagType;
	std::vector<PaiTagEntry> Entries;

	PaiTag(Buffer& buf, u8 TargetType);
	PaiTag();

	void Write(Buffer& buf, u8 TargetType);
};

class PaiEntry 
{
public:
	enum class AnimationTarget : u8 
	{
		Pane = 0,
		Material = 1,
		UserData = 2,
		MAX = 3
	};

	std::string Name;
	AnimationTarget Target;
	std::vector<PaiTag> Tags;
	std::vector<u8> UnkwnownData;

	PaiEntry();
	PaiEntry(Buffer& buf);

	void Write(Buffer& buf);
};

class Pai1Section : public BflanSection 
{
public:
	u16 FrameSize;
	u8 Flags;
	std::vector<std::string> Textures;
	std::vector<PaiEntry> Entries;

	Pai1Section();
	Pai1Section(const std::vector<u8>& data, Endianness bo);

	~Pai1Section() {}

	void ParseData(Endianness bo);

	void BuildData(Endianness bo) override;
};

class Bflan 
{
public:
	Endianness byteOrder;
	u32 Version;

	std::vector<BflanSection*> Sections;
	~Bflan();
	   
	Bflan();
	Bflan(const std::vector<u8>& data);
	std::vector<u8> WriteFile();
private:
	void ParseFile(Buffer& buf);
};

class BflanDeserializer 
{
public:
	static Bflan *FromJson(std::string json);
};