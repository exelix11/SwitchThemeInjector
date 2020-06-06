
#include "Bflyt.hpp"
#include "Usd1Pane.hpp"
#include "Grp1Pane.hpp"
#include "Pic1Pane.hpp"
#include "Txt1Pane.hpp"

#include <stdexcept>
#include <algorithm>
#include <stack>

using namespace std;
using namespace Panes;

TextureSection::TextureSection(Buffer& buf) : BasePane("txl1", buf)
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

void TextureSection::ApplyChanges(Buffer& dataWriter)
{
	dataWriter.Write((s32)Textures.size());
	for (size_t i = 0; i < Textures.size(); i++)
		dataWriter.Write((s32)0);
	for (size_t i = 0; i < Textures.size(); i++)
	{
		u32 off = dataWriter.Position;
		dataWriter.Write(Textures[i], Buffer::BinaryString::NullTerminated);
		u32 endPos = dataWriter.Position;
		dataWriter.Position = 4 + i * 4;
		dataWriter.Write(off - 4);
		dataWriter.Position = endPos;
	}
	dataWriter.WriteAlign(4);
}

MaterialsSection::MaterialsSection(u32 version) : BasePane("mat1", 8), Version(version) {}
MaterialsSection::MaterialsSection(Buffer& reader, u32 version) : BasePane("mat1", reader), Version(version)
{
	Buffer dataReader(data);
	dataReader.ByteOrder = reader.ByteOrder;
	int matCount = dataReader.readInt32();
	auto Offsets = dataReader.ReadS32Array(matCount);
	for (int i = 0; i < matCount; i++)
	{
		int matLen = (i == matCount - 1 ? (int)dataReader.Length() : Offsets[i + 1] - 8) - (int)dataReader.Position;
		BflytMaterial mat(dataReader.readBytes(matLen), Version, dataReader.ByteOrder);
		Materials.push_back(move(mat));
	}
}

void MaterialsSection::ApplyChanges(Buffer& dataWriter)
{
	dataWriter.Write((s32)Materials.size());
	for (size_t i = 0; i < Materials.size(); i++)
		dataWriter.Write((s32)0);
	for (size_t i = 0; i < Materials.size(); i++)
	{
		u32 off = dataWriter.Position;
		dataWriter.Write(Materials[i].Write(Version, dataWriter.ByteOrder));
		u32 endPos = dataWriter.Position;
		dataWriter.Position = 4 + i * 4;
		dataWriter.Write(off + 8);
		dataWriter.Position = endPos;
	}
}

Panes::PanePtr BflytFile::operator[] (const string& name)
{
	return FindPane([name](const Panes::PanePtr& p) {return p->PaneName == name; });
}

Panes::PanePtr BflytFile::FindPane(std::function<bool(const Panes::PanePtr&)> fun)
{
	auto&& b = find_if(PanesBegin(), PanesEnd(), fun);
	if (b == PanesEnd())
		return nullptr;
	else return *b;
}

int BflytFile::FindRootIndex(const string& type)
{
	for (size_t i = 0; i < RootPanes.size(); i++)
		if (RootPanes[i]->name == type)
			return (int)i;
	return -1;
}

Panes::PanePtr BflytFile::FindRoot(const string& type)
{
	int index = FindRootIndex(type);
	return index < 0 ? nullptr : RootPanes[index];
}

BflytFile::BflytFile(const vector<u8>& file)
{
	Buffer bin(file);
	bin.ByteOrder = Endianness::LittleEndian;
	if (bin.readStr(4) != "FLYT") throw runtime_error("Wrong signature");
	bin.readUInt16(); //BOM
	bin.readUInt16(); //HeaderSize
	Version = bin.readUInt32();
	bin.readUInt32(); //File size
	u16 sectionCount = bin.readUInt16();
	bin.readUInt16(); //padding

	Panes::PanePtr lastPane = nullptr;
	stack<Panes::PanePtr> currentRoot;
	auto&& PushPane = [&lastPane, &currentRoot, this](Panes::BasePane* ptr) {
		auto&& x = Panes::PanePtr(ptr);
		if (x->name == "pas1" || x->name == "grs1")
			currentRoot.push(lastPane);
		else if (x->name == "pae1" || x->name == "gre1")
			currentRoot.pop();
		else if (currentRoot.size() == 0)
			RootPanes.push_back(x);
		else
		{
			x->Parent = currentRoot.top();
			currentRoot.top()->Children.push_back(x);
		}
		lastPane = x;
	};

	for (size_t i = 0; i < sectionCount; i++)
	{
		string name = bin.readStr(4);
		if (name == "txl1")
			PushPane(new TextureSection(bin));
		else if (name == "mat1")
			PushPane(new MaterialsSection(bin, Version));
		else if (name == "pic1")
			PushPane(new Pic1Pane(bin, bin.ByteOrder));
		else if (name == "txt1")
			PushPane(new Txt1Pane(bin, bin.ByteOrder));
		else if (name == "usd1") {
			if (!lastPane)
				throw runtime_error("Misplaced user data section");
			lastPane->UserData = make_unique<Usd1Pane>(bin);
		}
		else if (name == "grp1")
			PushPane(new Grp1Pane(bin, Version));
		else if (name == "pan1" || name == "prt1" || name == "wnd1" || name == "bnd1")
			PushPane(new Pan1Pane(bin, bin.ByteOrder, name));
		else
			PushPane(new BasePane(name, bin));
	}
}

BflytFile::~BflytFile()
{
	RootPanes.clear();
}

//TODO make this return nullptr
shared_ptr<TextureSection> BflytFile::GetTexSection()
{
	auto p = FindRoot("txl1");
	if (!p)
	{
		p = PanePtr(new TextureSection());
		int t = FindRootIndex("fnl1");
		if (t >= 0)
			RootPanes.insert(RootPanes.begin() + t + 1, p);
		else
			RootPanes.insert(RootPanes.begin() + 1, p);
	}
	return dynamic_pointer_cast<TextureSection>(p);
}

shared_ptr<MaterialsSection> BflytFile::GetMatSection()
{
	auto p = FindRoot("mat1");
	if (!p)
	{
		p = PanePtr(new MaterialsSection(Version));
		int t = FindRootIndex("txl1");
		if (t >= 0)
			RootPanes.insert(RootPanes.begin() + t + 1, p);
		else
		{
			t = FindRootIndex("fnl1");
			if (t >= 0)
				RootPanes.insert(RootPanes.begin() + t + 1, p);
			else
				RootPanes.insert(RootPanes.begin() + 1, p);
		}
	}
	return dynamic_pointer_cast<MaterialsSection>(p);
}

Panes::PanePtr BflytFile::GetRootElement()
{
	return FindRoot("pan1");
}

shared_ptr<Grp1Pane> BflytFile::GetRootGroup()
{
	auto p = FindRoot("grp1");
	if (!p) return nullptr;
	return dynamic_pointer_cast<Grp1Pane>(p);
}

vector<PanePtr> BflytFile::WritePaneListForBinary()
{
	vector<PanePtr> res;
	stack<PanePtr> ToProc;
	//It's a stack so we reverse the child panes order
	for (auto&& p = RootPanes.rbegin(); p != RootPanes.rend(); p++)
		ToProc.push(*p);

	while (ToProc.size() > 0)
	{
		auto c = ToProc.top();
		ToProc.pop();
		res.push_back(c);
		if (c->Children.size() != 0)
		{
			ToProc.emplace(new BasePane(c->name == "grp1" ? "gre1" : "pae1", 8));
			for (auto&& p = c->Children.rbegin(); p != c->Children.rend(); p++)
				ToProc.push(*p);
			res.emplace_back(new BasePane(c->name == "grp1" ? "grs1" : "pas1", 8));
		}
	}
	return res;
}

vector<u8> BflytFile::SaveFile()
{
	auto&& Panes = WritePaneListForBinary();

	Buffer bin;
	bin.ByteOrder = Endianness::LittleEndian;
	bin.Write("FLYT");
	bin.Write((u16)0xFEFF);
	bin.Write((u16)0x14); //Header size
	bin.Write((u32)Version);
	bin.Write((u32)0);
	u16 PaneCount = 0;
	for (const auto& pane : Panes)
		PaneCount += pane->UserData ? 2 : 1;
	bin.Write(PaneCount);
	bin.Write((u16)0); //padding
	for (auto p : Panes)
		p->WritePane(bin);
	bin.WriteAlign(4);
	bin.Position = 0xC;
	bin.Write((u32)bin.Length());
	bin.Position = bin.Length();
	return bin.getBuffer();
}

void BflytFile::RemovePane(PanePtr& pane)
{
	auto ptr = pane->Parent.lock();
	auto& c = ptr->Children;
	c.erase(std::remove(c.begin(), c.end(), pane), c.end());
}

void BflytFile::AddPane(size_t offset, PanePtr& Parent, PanePtr& pane)
{
	if (offset > Parent->Children.size())
		offset = Parent->Children.size();
	Parent->Children.insert(Parent->Children.begin() + offset, pane);
}

void BflytFile::MovePane(PanePtr& pane, PanePtr& NewParent, size_t offset)
{
	RemovePane(pane);
	AddPane(offset, NewParent, pane);
}