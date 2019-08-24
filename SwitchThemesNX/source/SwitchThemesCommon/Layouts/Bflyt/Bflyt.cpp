#include "Bflyt.hpp"
#include "Usd1Pane.hpp"
#include "Grp1Pane.hpp"
#include "Pic1Pane.hpp"

#include <stdexcept>
#include <algorithm>

using namespace std;
using namespace Panes;

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

void TextureSection::ApplyChanges(Buffer &dataWriter)
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

void MaterialsSection::ApplyChanges(Buffer & dataWriter)
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

PanePtr& BflytFile::operator[] (int index)
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
	for (size_t i = 0; i < sectionCount; i++)
	{
		string name = bin.readStr(4);
		if (name == "txl1")
			Panes.emplace_back(new TextureSection(bin));
		else if (name == "mat1")
			Panes.emplace_back(new MaterialsSection(bin, Version));
		else if (name == "pic1")
			Panes.emplace_back(new Pic1Pane(bin, bin.ByteOrder));
		else if (name == "usd1")
			Panes[Panes.size() - 1]->UserData = make_unique<Usd1Pane>(bin);
		else if (name == "grp1")
			Panes.emplace_back(new Grp1Pane(bin, Version));
		else if (name == "pan1" || name == "prt1" || name == "wnd1" || name == "bnd1")
			Panes.emplace_back(new Pan1Pane(bin, bin.ByteOrder, name));
		else 
			Panes.emplace_back(new BasePane(name,bin));

		//if (i == sectionCount - 1 && bin.Position != bin.Length()) //load sections missing in the section count (my old bflyt patch)
		//{
		//	u8 c = 0;
		//	while (bin.Position < bin.Length() && (c = bin.readUInt8() == 0)) {}
		//	if (c != 0)
		//		bin.Position--;
		//	if (bin.Length() - bin.Position >= 8) //min section size
		//	{
		//		sectionCount++;
		//	}
		//}
	}
}

BflytFile::~BflytFile() 
{
	Panes.clear();
}

//TODO make this return nullptr
shared_ptr<TextureSection> BflytFile::GetTexSection()
{
	for (auto ptr : Panes)
	{
		if (ptr->name == "txl1")
			return dynamic_pointer_cast<TextureSection>(ptr);
	}
	Panes.emplace(Panes.begin() + 2, new TextureSection());
	return dynamic_pointer_cast<TextureSection>(Panes[2]);
}

shared_ptr<MaterialsSection> BflytFile::GetMatSection()
{
	for (auto ptr : Panes)
	{
		if (ptr->name == "mat1")
			return dynamic_pointer_cast<MaterialsSection>(ptr);
	}
	Panes.emplace(Panes.begin() + 3, new MaterialsSection(Version));
	return dynamic_pointer_cast<MaterialsSection>(Panes[3]);
}

vector<u8> BflytFile::SaveFile() 
{
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

vector<string> BflytFile::GetPaneNames()
{
	vector<string> names;
	for (size_t i = 0; i < Panes.size(); i++)
		names.push_back(Panes[i]->PaneName);
	return names;
}

vector<string> BflytFile::GetGroupNames()
{
	vector<string> names;
	for (size_t i = 0; i < Panes.size(); i++)
		if (Panes[i]->name == "grp1")
			names.push_back(dynamic_pointer_cast<Grp1Pane>(Panes[i])->GroupName);
	return names;
}

int BflytFile::FindPaneEnd(int paneIndex)
{
	auto pane = Panes[paneIndex];

	string childStarter = pane->name == "grp1" ? "grs1" : "pas1";
	string childCloser = pane->name == "grp1" ? "gre1" : "pae1";

	if (Panes[paneIndex + 1]->name == childStarter)
	{
		int childLevel = 0;
		size_t i;
		for (i = paneIndex + 2; i < Panes.size(); i++)
		{
			if (Panes[i]->name == childCloser)
			{
				if (childLevel == 0)
					break;
				childLevel--;
			}
			if (Panes[i]->name == childStarter)
				childLevel++;
		}
		return (int)i;
	}
	return paneIndex;
}

void BflytFile::RebuildParentingData()
{
	RebuildGroupingData();
	PanePtr CurrentRoot = nullptr;
	int RootIndex = -1;
	for (size_t i = 0; i < Panes.size(); i++)
		if (Panes[i]->name == "RootPane")
		{
			CurrentRoot = Panes[i];
			RootIndex = i;
			break;
		}
	if (!CurrentRoot) throw("Couldn't find the root pane");
	RootPane = CurrentRoot;
	RootPane->Children.clear();
	RootPane->Parent = nullptr;
	for (size_t i = RootIndex + 1; i < Panes.size(); i++)
	{
		if (Panes[i]->name == "pas1")
		{
			CurrentRoot = Panes[i - 1];
			CurrentRoot->Children.clear();
			continue;
		}
		if (Panes[i]->name == "pae1")
		{
			CurrentRoot = CurrentRoot->Parent;
			if (CurrentRoot == nullptr) return;
			continue;
		}
		Panes[i]->Parent = CurrentRoot;
		CurrentRoot->Children.push_back(Panes[i]);
	}
	if (!CurrentRoot)
		throw("Unexpected pane data ending: one or more children sections are not closed by the end of the file");
}

void BflytFile::RebuildGroupingData() 
{
	RootGroup = nullptr;
	int rootGroupIndex = -1;
	for (size_t i = 0; i < Panes.size(); i++)
		if (Panes[i]->name == "grp1")
		{
			rootGroupIndex = i;
			break;
		}
	if (rootGroupIndex == -1) return;
	auto curRoot = dynamic_pointer_cast<Grp1Pane>(Panes[rootGroupIndex]);
	RootGroup = curRoot;
	curRoot->Parent = nullptr;
	curRoot->Children.clear();
	for (size_t i = rootGroupIndex + 1; i < Panes.size(); i++)
	{
		if (Panes[i]->name == "grs1")
		{
			curRoot = dynamic_pointer_cast<Grp1Pane>(Panes[i - 1]);
			curRoot->Children.clear();
			continue;
		}
		else if (Panes[i]->name == "gre1")
		{
			curRoot = dynamic_pointer_cast<Grp1Pane>(curRoot->Parent);
			if (curRoot == nullptr) return;
			continue;
		}
		if (Panes[i]->name != "grp1") break;
		Panes[i]->Parent = curRoot;
		curRoot->Children.push_back(Panes[i]);
	}
	if (curRoot != RootGroup)
		throw("Unexpected pane data ending: one or more group sections are not closed by the end of the file");
}

void BflytFile::RemovePane(PanePtr& pane)
{
	size_t PaneIndex = Utils::IndexOf(Panes, pane);
	size_t end = FindPaneEnd(PaneIndex);

	Panes.erase(Panes.begin() + PaneIndex, Panes.begin() + end);

	if (pane->Parent)
	{
		auto &PChildList = pane->Parent->Children;
		PChildList.erase(remove(PChildList.begin(), PChildList.end(), pane), PChildList.end());
	}
	RebuildParentingData();
}

void BflytFile::AddPane(size_t offsetInChildren, PanePtr& Parent, vector<PanePtr>& pane)
{
	string childStarter = pane[0]->name == "grp1" ? "grs1" : "pas1";
	string childCloser = pane[0]->name == "grp1" ? "gre1" : "pae1";

	if (pane.size() > 1 && (pane[1]->name != childStarter || pane[0]->Parent == pane[2]->Parent))
		throw("The BasePane array must be a single pane, optionally with children already in the proper structure");

	if (!Parent) Parent = RootPane;
	size_t parentIndex = Utils::IndexOf(Panes, Parent);
	if (parentIndex == SIZE_MAX) throw("Parent not found");
	if (Panes.size() <= parentIndex + 1 || Panes[parentIndex + 1]->name != childStarter)
	{
		if (Parent->Children.size() != 0) throw("Inconsistend data !");
		Panes.emplace(Panes.begin() + parentIndex + 1, new BasePane(childStarter, 8));
		Panes.emplace(Panes.begin() + parentIndex + 2, new BasePane(childCloser, 8));
	}

	pane[0]->Parent = Parent;
	if (offsetInChildren <= 0 || offsetInChildren >= Parent->Children.size())
	{
		Parent->Children.insert(Parent->Children.end(), pane.begin(), pane.end());
		Panes.insert(Panes.begin() + parentIndex + 2, pane.begin(), pane.end());
	}
	else 
	{
		int actualInserOffset = 0;
		int childCount = 0;
		for (int i = parentIndex + 2; ; i++)
		{
			i = FindPaneEnd(i) + 1;
			childCount++;
			if (childCount == offsetInChildren) 
			{
				actualInserOffset = i;
				break;
			}
		}

		Parent->Children.insert(Parent->Children.end(), pane.begin(), pane.end());
		Panes.insert(Panes.begin() + actualInserOffset, pane.begin(), pane.end());
	}
	RebuildParentingData();
}

void BflytFile::MovePane(PanePtr& pane, PanePtr& NewParent, size_t childOffset)
{
	if (childOffset > NewParent->Children.size())
		childOffset = NewParent->Children.size();

	int parentIndex = Utils::IndexOf(Panes, NewParent);
	if (parentIndex == -1) throw("No parent !");

	int paneIndex = Utils::IndexOf(Panes, pane);

	int paneCount = FindPaneEnd(paneIndex) - paneIndex + 1;

	vector<PanePtr> tmpForCopy;
	for (int i = paneIndex; i < paneIndex + paneCount; i++)
		tmpForCopy.push_back(Panes[i]);

	Panes.erase(Panes.begin() + paneIndex, Panes.begin() + paneCount);

	AddPane(childOffset, NewParent, tmpForCopy);
}