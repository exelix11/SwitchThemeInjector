#include "Bflyt.hpp"
#include <stdexcept>
#include <algorithm>

using namespace std;
using namespace Panes;

string BasePane::ToString() { return "Pane " + name + " " + to_string(length); }

BasePane::BasePane(const string &_name, u32 len)  : name(_name), length(len), data(len - 8) {}

BasePane::BasePane(const BasePane &ref) : name(ref.name), length(ref.length), data(ref.data) {}

BasePane::BasePane(const string &_name, Buffer &reader) : name(_name)
{
	length = reader.readInt32();
	data = reader.readBytes(length - 8);
}

void BasePane::WritePane(Buffer &writer)
{
	writer.Write(name);
	length = data.size() + 8;
	writer.Write(length);
	writer.Write(data);
}

Grp1Pane::Grp1Pane(u32 version) : Version{version}, BasePane("grp1", 8) {}
Grp1Pane::Grp1Pane(Buffer& buf, u32 version) : Version { version }, BasePane("grp1", buf)
{
	LoadProperties();
}

void Grp1Pane::WritePane(Buffer& writer)
{
	ApplyChanges();
	BasePane::WritePane(writer);
}

void Grp1Pane::LoadProperties() 
{
	Buffer bin{ data };
	bin.ByteOrder = Endianness::LittleEndian;
	if (Version > 0x05020000)
		GroupName = bin.readStr_Fixed(34);
	else
		GroupName = bin.readStr_Fixed(24);
	auto NodeCount = bin.readUInt16();
	if (Version <= 0x05020000)
		bin.readUInt16();
	auto pos = bin.Position;
	for (u64 i = 0; i < NodeCount; i++)
	{
		bin.Position = pos + i * 24;
		Panes.push_back(bin.readStr_Fixed(24));
	}
}

void Grp1Pane::ApplyChanges() 
{
	Buffer bin;
	if (Version > 0x05020000)
		bin.WriteFixedLengthString(GroupName, 34);
	else
		bin.WriteFixedLengthString(GroupName, 24);
	bin.Write((u16)Panes.size());
	if (Version <= 0x05020000)
		bin.Write((u16)0);
	for(const auto &s : Panes)
		bin.WriteFixedLengthString(s, 24);
	data = bin.getBuffer();
}

string PropertyEditablePane::ToString() { return "Pane " + name + " " + PaneName; }
bool PropertyEditablePane::GetVisible() {return (_flag1 & 0x1) == 0x1;}

void PropertyEditablePane::SetVisible(bool value)
{
	if (value)
		_flag1 |= 0x1;
	else
		_flag1 &= 0xFE;
}

Vector3 ReadVec3(Buffer &buf)
{
	Vector3 res;
	res.X = buf.readFloat();
	res.Y = buf.readFloat();
	res.Z = buf.readFloat();
	return res;
}

Vector2 ReadVec2(Buffer &buf)
{
	Vector2 res;
	res.X = buf.readFloat();
	res.Y = buf.readFloat();
	return res;
}

PropertyEditablePane::PropertyEditablePane(const BasePane &p) : BasePane(p)
{
	Buffer buf(p.data);
	buf.ByteOrder = Endianness::LittleEndian;
	_flag1 = buf.readUInt8();
	buf.Position += 3;
	PaneName = buf.readStr_NullTerm(0x18);
	buf.Position = 0x2c - 8;
	Position = ReadVec3(buf);
	Rotation = ReadVec3(buf);
	Scale = ReadVec2(buf);
	Size = ReadVec2(buf);
	if (name == "pic1")
	{
		buf.Position = 0x54 - 8;
		ColorData = buf.ReadU32Array(4);
	}
}

#define WriteVec3(_x) bin.Write(_x.X);bin.Write(_x.Y);bin.Write(_x.Z);
#define WriteVec2(_x) bin.Write(_x.X);bin.Write(_x.Y);

void PropertyEditablePane::ApplyChanges() 
{
	Buffer bin(data);
	bin.Position = 0;
	bin.Write(_flag1);
	bin.Position = 0x2C - 8;
	WriteVec3(Position)
	WriteVec3(Rotation)
	WriteVec2(Scale)
	WriteVec2(Size)
	if (name == "pic1")
	{
		bin.Position = 0x54 - 8;
		bin.WriteU32Array(ColorData);
	}
	data = bin.getBuffer();
}

void PropertyEditablePane::WritePane(Buffer &writer) 
{
	ApplyChanges();
	BasePane::WritePane(writer);
}

TextureSection::TextureSection(Buffer &buf) : BasePane("txl1",buf)
{
	Buffer rd(data);
	rd.ByteOrder = Endianness::LittleEndian;
	int texCount = rd.readInt32();
	u32 BaseOff = rd.Position;
	auto Offsets = rd.ReadS32Array(texCount);
	for (auto off : Offsets)
	{
		rd.Position = BaseOff + off;
		Textures.push_back(rd.readStr_NullTerm());
	}
}

TextureSection::TextureSection() : BasePane("txl1", 8) {}

void TextureSection::WritePane(Buffer &writer)
{
	Buffer dataWriter;
	dataWriter.ByteOrder = writer.ByteOrder;
	dataWriter.Write((s32)Textures.size());
	for (int i = 0; i < Textures.size(); i++)
		dataWriter.Write((s32)0);
	for (int i = 0; i < Textures.size(); i++)
	{
		u32 off = dataWriter.Position;
		dataWriter.Write(Textures[i], Buffer::BinaryString::NullTerminated);
		dataWriter.WriteAlign(4);
		u32 endPos = dataWriter.Position;
		dataWriter.Position = 4 + i * 4;
		dataWriter.Write(off - 4);
		dataWriter.Position = endPos;
	}
	data = dataWriter.getBuffer();
	BasePane::WritePane(writer);
}

MaterialsSection::MaterialsSection(u32 version) : BasePane("mat1", 8), Version(version) {}
MaterialsSection::MaterialsSection(Buffer &reader, u32 version) : BasePane("mat1", reader) , Version(version)
{
	Buffer dataReader(data);
	dataReader.ByteOrder = reader.ByteOrder;
	int matCount = dataReader.readInt32();
	auto Offsets = dataReader.ReadS32Array(matCount);
	for (int i = 0; i < matCount; i++)
	{
		int matLen = (i == matCount - 1 ? (int)dataReader.Length() : Offsets[i + 1] - 8) - (int)dataReader.Position;
		BflytMaterial mat (dataReader.readBytes(matLen), Version, dataReader.ByteOrder);
		Materials.push_back(move(mat));
	}
}

void MaterialsSection::WritePane(Buffer &writer)
{
	Buffer dataWriter;
	dataWriter.ByteOrder = writer.ByteOrder;
	dataWriter.Write((s32)Materials.size());
	for (int i = 0; i < Materials.size(); i++)
		dataWriter.Write((s32)0);
	for (int i = 0; i < Materials.size(); i++)
	{
		u32 off = dataWriter.Position;
		dataWriter.Write(Materials[i].Write(Version, dataWriter.ByteOrder));
		u32 endPos = dataWriter.Position;
		dataWriter.Position = 4 + i * 4;
		dataWriter.Write(off + 8);
		dataWriter.Position = endPos;
	}
	data = dataWriter.getBuffer();
	BasePane::WritePane(writer);
}

PicturePane::PicturePane(Buffer &buf) : BasePane("pic1",buf)
{
	_PaneName = BflytFile::TryGetPanelName(this);
}

string PicturePane::PaneName() { return _PaneName; }

string PicturePane::ToString() { return "Picture " + PaneName(); }

string BflytFile::TryGetPanelName(const BasePane *ptr)
{
	if (ptr == nullptr || ptr->data.size() < 0x18 + 4) return "";
	Buffer dataReader(ptr->data);
	dataReader.ByteOrder = Endianness::LittleEndian;
	dataReader.readInt32(); //Unknown
	return dataReader.readStr_NullTerm(0x18);
}

BasePane*& BflytFile::operator[] (int index)
{
	return Panes[index];
}

BflytFile::BflytFile(const vector<u8>& file) 
{
	Buffer bin(file);
	bin.ByteOrder = Endianness::LittleEndian;
	if (bin.readStr(4) != "FLYT") throw "Wrong signature";
	bin.readUInt16(); //BOM
	bin.readUInt16(); //HeaderSize
	Version = bin.readUInt32();
	bin.readUInt32(); //File size
	u16 sectionCount = bin.readUInt16();
	bin.readUInt16(); //padding
	for (int i = 0; i < sectionCount; i++)
	{
		string name = bin.readStr(4);
		if (name == "txl1")
			Panes.push_back((BasePane*) new TextureSection(bin));
		else if (name == "mat1")
			Panes.push_back((BasePane*) new MaterialsSection(bin,Version));
		else if (name == "pic1")
			Panes.push_back((BasePane*) new PicturePane(bin));
		else if (name == "usd1")
			Panes.push_back((BasePane*) new Usd1Pane(bin));
		else if (name == "grp1")
			Panes.push_back((BasePane*) new Grp1Pane(bin, Version));
		else 
			Panes.push_back(new BasePane(name,bin));

		if (i == sectionCount - 1 && bin.Position != bin.Length()) //load sections missing in the section count (my old bflyt patch)
		{
			u8 c = 0;
			while (bin.Position < bin.Length() && (c = bin.readUInt8() == 0)) {}
			if (c != 0)
				bin.Position--;
			if (bin.Length() - bin.Position >= 8) //min section size
			{
				sectionCount++;
			}
		}
	}
}

BflytFile::~BflytFile() 
{
	for (auto ptr : Panes)
		delete ptr;
	Panes.clear();
}

TextureSection* BflytFile::GetTexSection()
{
	for (auto ptr : Panes)
	{
		if (ptr->name == "txl1")
			return (TextureSection*)ptr;
	}
	TextureSection *ptr = new TextureSection();
	Panes.insert(Panes.begin() + 2, (Panes::BasePane*)ptr);
	return ptr;
}

MaterialsSection* BflytFile::GetMatSection()
{
	for (auto ptr : Panes)
	{
		if (ptr->name == "mat1")
			return (MaterialsSection*)ptr;
	}
	MaterialsSection *ptr = new MaterialsSection(Version);
	Panes.insert(Panes.begin() + 3, (Panes::BasePane*)ptr);
	return ptr;
}

vector<u8> BflytFile::SaveFile() 
{
	Buffer bin;
	bin.ByteOrder = Endianness::LittleEndian;
	bin.Write("FLYT");
	bin.Write((u8)0xFF);
	bin.Write((u8)0xFE); //Little endian
	bin.Write((u16)0x14); //Header size
	bin.Write((u32)Version);
	bin.Write((s32)0);
	bin.Write((u16)Panes.size());
	bin.Write((u16)0); //padding
	for (auto p : Panes)
		p->WritePane(bin);
	bin.WriteAlign(4);
	bin.Position = 0xC;
	bin.Write((u32)bin.Length());
	bin.Position = bin.Length();
	return bin.getBuffer();
}

bool BflytFile::PatchTextureName(const string &original, const string &_new) 
{
	bool patchedSomething = false;
	auto texSection = GetTexSection();
	if (texSection == nullptr)
		throw "this layout doesn't have any texture section (?)";
	for (int i = 0; i < texSection->Textures.size(); i++)
	{
		if (texSection->Textures[i] == original)
		{
			patchedSomething = true;
			texSection->Textures[i] = _new;
		}
	}
	return patchedSomething;
}

inline int indexOf(const vector<string>& v, const string& s)
{
	for (int i = 0; i < v.size(); i++)
		if (v[i] == s) return i;
	return -1;
}

int BflytFile::AddBgMat(const std::string &texName) 
{
	auto MatSect = GetMatSection();
	//Add texture
	auto texSection = GetTexSection();
	int texIndex = indexOf(texSection->Textures, texName);
	if (texIndex == -1)
	{
		texIndex = texSection->Textures.size();
		texSection->Textures.push_back(texName);
	}
	//Add material
	{
		Buffer bin;
		bin.ByteOrder = Endianness::LittleEndian;
		bin.Write("P_Custm", Buffer::BinaryString::NullTerminated);
		for (int i = 0; i < 0x14; i++)
			bin.Write((u8)0);
		bin.Write((s32)0x15);
		bin.Write((s32)0x8040200);
		bin.Write((s32)0);
		bin.Write((u32)0xFFFFFFFF);
		bin.Write((u16)texIndex);
		bin.Write((u16)0x0);
		for (int i = 0; i < 0xC; i++)
			bin.Write((u8)0);
		bin.Write((float)1);
		bin.Write((float)1);
		for (int i = 0; i < 0x10; i++)
			bin.Write((u8)0);
		BflytMaterial mat{ bin.getBuffer() , Version, bin.ByteOrder };
		MatSect->Materials.push_back(move(mat));
	}
	return MatSect->Materials.size() - 1;
}

vector<string> BflytFile::GetPaneNames()
{
	vector<string> names;
	for (int i = 0; i < Panes.size(); i++)
		names.push_back(TryGetPanelName(Panes[i]));
	return names;
}

vector<string> BflytFile::GetGroupNames()
{
	vector<string> names;
	for (int i = 0; i < Panes.size(); i++)
		if (Panes[i]->name == "grp1")
			names.push_back(((Grp1Pane*)Panes[i])->GroupName);
	return names;
}

BflytFile::PatchResult BflytFile::AddGroupNames(const std::vector<ExtraGroup>& Groups)
{
	if (Groups.size() == 0) return PatchResult::OK;
	auto PaneNames = GetPaneNames();
	auto GroupNames = GetGroupNames();

	auto rootGroupIndex = find_if(Panes.rbegin(), Panes.rend(),	[](BasePane* i) { return i->name == "gre1"; });
	if (rootGroupIndex == Panes.rend())
	{
		rootGroupIndex = find_if(Panes.rbegin(), Panes.rend(), [](BasePane * i) { return i->name == "grp1"; });
		if (rootGroupIndex == Panes.rend())
			return PatchResult::CorruptedFile;
		Panes.insert(rootGroupIndex.base(), new BasePane("gre1", 8));
		rootGroupIndex = find_if(Panes.rbegin(), Panes.rend(), [](BasePane * i) { return i->name == "grp1"; });
		Panes.insert(rootGroupIndex.base(), new BasePane("grs1", 8));
	}

	for (const auto &g : Groups)
	{
		rootGroupIndex = find_if(Panes.rbegin(), Panes.rend(), [](BasePane * i) { return i->name == "gre1"; }); //Not sure if after inserting the iterator is still valid

		if (find(GroupNames.begin(), GroupNames.end(), g.GroupName) != GroupNames.end()) continue;
		for (const auto& s : g.Panes)
		{
			if (find(PaneNames.begin(), PaneNames.end(), s) == PaneNames.end()) 
				return PatchResult::Fail;
		}

		auto grp = new Grp1Pane(Version);
		grp->GroupName = g.GroupName;
		grp->Panes = g.Panes;
		Panes.insert(rootGroupIndex.base() - 1, (BasePane*)grp);
	}

	return PatchResult::OK;
}

BflytFile::PatchResult BflytFile::ApplyLayoutPatch(const vector<PanePatch>& Patches) 
{
	auto names = GetPaneNames();
	for (int i = 0; i < Patches.size(); i++)
	{
		int index = indexOf(names, Patches[i].PaneName);
		if (index == -1)
			//return PatchResult::CorruptedFile;
			continue;
			//The layout patching has been made less strict to allow some 8.x layouts to work on lower firmwares
			
		auto p = Patches[i];
		auto e = new PropertyEditablePane(*Panes[index]);
		delete Panes[index];
		Panes[index] = (BasePane*)e;
		if (p.ApplyFlags & (u32)PanePatch::Flags::Visible)
			e->SetVisible(p.Visible);

		if (p.ApplyFlags & (u32)PanePatch::Flags::Position)
		{
			e->Position.X = p.Position.X;
			e->Position.Y = p.Position.Y;
			e->Position.Z = p.Position.Z;
		}			
		if (p.ApplyFlags & (u32)PanePatch::Flags::Rotation)
		{
			e->Rotation.X = p.Rotation.X;
			e->Rotation.Y = p.Rotation.Y;
			e->Rotation.Z = p.Rotation.Z;
		}
		if (p.ApplyFlags & (u32)PanePatch::Flags::Scale)
		{
			e->Scale.X = p.Scale.X;
			e->Scale.Y = p.Scale.Y;
		}
		if (p.ApplyFlags & (u32)PanePatch::Flags::Size)
		{
			e->Size.X = p.Size.X;
			e->Size.Y = p.Size.Y;
		}

		if (e->name == "pic1")
		{
			if (p.ApplyFlags & (u32)PanePatch::Flags::ColorTL)
				e->ColorData[0] = (u32)std::stoul(p.ColorTL, 0, 16);
			if (p.ApplyFlags & (u32)PanePatch::Flags::ColorTR)
				e->ColorData[1] = (u32)std::stoul(p.ColorTR, 0, 16);
			if (p.ApplyFlags & (u32)PanePatch::Flags::ColorBL)
				e->ColorData[2] = (u32)std::stoul(p.ColorBL, 0, 16);
			if (p.ApplyFlags & (u32)PanePatch::Flags::ColorBR)
				e->ColorData[3] = (u32)std::stoul(p.ColorBR, 0, 16);
		}

		if ((p.ApplyFlags & (u32)PanePatch::Flags::Usd1) && 
			(Panes.size() > index + 1) && Panes[index + 1]->name == "usd1")
		{
			Usd1Pane* usd = (Usd1Pane*)Panes[index + 1];
			for (const auto &patch : p.UsdPatches)
			{
				auto v = usd->FindName(patch.PropName);
				if (v == nullptr)
					usd->AddProperty(patch.PropName, patch.PropValues, (Panes::Usd1Pane::ValueType)patch.type);
				else if (v && v->ValueCount == patch.PropValues.size() && (int)v->type && patch.type)
				{
					v->value = patch.PropValues;
				}
			}
		}
	}
	return PatchResult::OK;
}

BflytFile::PatchResult BflytFile::ApplyMaterialsPatch(const vector<MaterialPatch>& Patches) 
{
	auto mats = GetMatSection();
	if (!mats) return PatchResult::Fail;
	for (const auto& p : Patches)
	{
		auto target = find_if(mats->Materials.begin(), mats->Materials.end(), [&p](BflytMaterial m) {return m.Name == p.MaterialName; });
		if (target == mats->Materials.end()) continue; //Less strict patching
		if (p.ForegroundColor != "")
			(*target).ForegroundColor = (u32)std::stoul(p.ForegroundColor, 0, 16);
		if (p.BackgroundColor!= "")
			(*target).BackgroundColor = (u32)std::stoul(p.BackgroundColor, 0, 16);
	}
	return PatchResult::OK;
}

BflytFile::PatchResult BflytFile::PatchBgLayout(const PatchTemplate& patch) 
{
	//Detect patch
	for (int i = 0; i < Panes.size(); i++)
	{
		if (Panes[i]->name != "pic1") continue;
		auto p = (PicturePane*)Panes[i];
		if (p->PaneName() == patch.PatchIdentifier) return PatchResult::AlreadyPatched;
		if (p->PaneName() == "3x3lxBG") //Fix old layout
		{
			delete Panes[i];
			Panes.erase(Panes.begin() + i);
			GetTexSection()->Textures[0] = "White1x1^r";
			GetMatSection()->Materials.erase(GetMatSection()->Materials.begin() + 1);
		}
	}
	//Find and remove target panes
	s32 target = INT32_MAX;
	for (int i = 0; i < Panes.size() - 1; i++)
	{
		string name = TryGetPanelName(Panes[i]);
		if (name != "" && indexOf(patch.targetPanels, name) != -1)
		{
			if (i < target) target = i;
			if (patch.DirectPatchPane)
			{
				int m = AddBgMat(patch.MaintextureName);
				Buffer bin(Panes[i]->data);
				bin.ByteOrder = Endianness::LittleEndian;
				bin.Position = 0x64 - 8;
				bin.Write((u16)m);
				Panes[i]->data = move(bin.getBuffer());
			}
			else if (!patch.NoRemovePanel)
			{
				Buffer bin(Panes[i]->data);
				bin.ByteOrder = Endianness::LittleEndian;
				bin.Position = 0x24;
				bin.Write((float)5000.0);
				bin.Write((float)60000.0);
				Panes[i]->data = bin.getBuffer();
			}
		}
	}
	if (target == INT32_MAX)
		return PatchResult::Fail;

	if (!patch.DirectPatchPane)
		return AddBgPanel(target, patch.MaintextureName, patch.PatchIdentifier);
	else return PatchResult::OK;
}

BflytFile::PatchResult BflytFile::AddBgPanel(int index, const string &TexName, const string &Pic1Name)
{
	//Add pitcture
	if (Pic1Name.length() > 0x18)
		throw "Pic1Name should not be longer than 24 chars";
	auto BgPanel = new BasePane("pic1", 0x8);
	Panes.insert(Panes.begin() + index, BgPanel);
	int TexIndex = AddBgMat(TexName);
	{
		Buffer bin;
		bin.ByteOrder = Endianness::LittleEndian;
		bin.Write((u8)0x01);
		bin.Write((u8)0x00);
		bin.Write((u8)0xFF);
		bin.Write((u8)0x04);
		bin.Write(Pic1Name);
		int zerCount = Pic1Name.length();
		while (zerCount++ < 0x38)
			bin.Write((u8)0x00);
		bin.Write((float)1);
		bin.Write((float)1);
		bin.Write((float)1280);
		bin.Write((float)720);
		bin.Write((u32)0xFFFFFFFF);
		bin.Write((u32)0xFFFFFFFF);
		bin.Write((u32)0xFFFFFFFF);
		bin.Write((u32)0xFFFFFFFF);
		bin.Write((u16)TexIndex);
		bin.Write((u16)1);
		bin.Write((u32)0);
		bin.Write((u32)0);
		bin.Write((float)1);
		bin.Write((u32)0);
		bin.Write((u32)0);
		bin.Write((float)1);
		bin.Write((float)1);
		bin.Write((float)1);
		BgPanel->data = bin.getBuffer();
	}
	return PatchResult::OK;
}

//Discard the pointer if the Properties vector is changed
Usd1Pane::EditableProperty* Usd1Pane::FindName(const string &name) 
{
	for (auto &p : Properties)
		if (p.Name == name)
			return &p;
	return nullptr;
}

Usd1Pane::Usd1Pane(Buffer &reader) : BasePane("usd1", reader)
{
	LoadProperties();
}

void Usd1Pane::LoadProperties() 
{
	Buffer dataReader(data);
	dataReader.ByteOrder = Endianness::LittleEndian;
	dataReader.Position = 0;
	u16 Count = dataReader.readUInt16();
	u16 Unk1 = dataReader.readUInt16();
	for (int i = 0; i < Count; i++)
	{
		auto EntryOffset = dataReader.Position;
		u32 NameOffset = dataReader.readUInt32();
		u32 DataOffset = dataReader.readUInt32();
		u16 ValueLen = dataReader.readUInt16();
		u8 dataType = dataReader.readUInt8();
		dataReader.readUInt8(); //padding ?

		if (!(dataType == 1 || dataType == 2))
			continue;

		auto pos = dataReader.Position;
		dataReader.Position = EntryOffset + NameOffset;
		string propName = dataReader.readStr_NullTerm();
		auto type = (ValueType)dataType;

		dataReader.Position = EntryOffset + DataOffset;
		vector<string> values;

		for (int j = 0; j < ValueLen; j++)
		{
			if (type == ValueType::int32)
				values.push_back(to_string(dataReader.readInt32()));
			else 
				values.push_back(to_string(dataReader.readFloat()));
		}

		Properties.push_back(EditableProperty
			{
				propName,
				EntryOffset + DataOffset,
				ValueLen,
				type,
				std::move(values)
			});

		dataReader.Position = pos;
	}
}

void Usd1Pane::ApplyChanges()
{
	Buffer bin;
	bin.Write((u16)(Properties.size() + AddedProperties.size()));
	bin.Write((u16)0);
	for (int i = 0; i < 3 * AddedProperties.size(); i++) bin.Write((u32)0);
	bin.Write(data, 4, data.size() - 4); //write rest of entries, adding new elements first doesn't break relative offets in the struct
	for(const auto &m : Properties)
	{
		if (m.type != ValueType::int32 && m.type != ValueType::single) continue;
		bin.Position = m.ValueOffset + 0xC * AddedProperties.size();
		for (int i = 0; i < m.ValueCount; i++)
		{
			if (m.type == ValueType::int32)
				bin.Write(stoi(m.value[i]));
			else
				bin.Write(stof(m.value[i]));
		}
	}
	for (int i = 0; i < AddedProperties.size(); i++)
	{
		bin.Position = bin.Length();
		u32 DataOffset = (u32)bin.Position;
		for (int j = 0; j < AddedProperties[i].ValueCount; j++)
			if (AddedProperties[i].type == ValueType::int32)
				bin.Write(stoi(AddedProperties[i].value[j]));
			else
				bin.Write(stof(AddedProperties[i].value[j]));
		u32 NameOffest = (u32)bin.Position;
		bin.Write(AddedProperties[i].Name, Buffer::BinaryString::NullTerminated);
		while (bin.Position % 4) bin.Write((u8)0);
		u32 entryStart = (u32)(4 + i * 0xC);
		bin.Position = entryStart;
		bin.Write(NameOffest - entryStart);
		bin.Write(DataOffset - entryStart);
		bin.Write(AddedProperties[i].ValueCount);
		bin.Write((u8)AddedProperties[i].type);
		bin.Write((u8)0);
	}
	data = std::move(bin.getBuffer());
}

void Usd1Pane::WritePane(Buffer &writer)
{
	ApplyChanges();
	BasePane::WritePane(writer);
}

void Usd1Pane::AddProperty(const std::string &name, const std::vector<std::string> &values, ValueType type)
{
	AddedProperties.push_back({ name, 0, (u16)values.size(), type, values });
}