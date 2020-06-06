
#include "BflytPatcher.hpp"
#include "Pic1Pane.hpp"
#include "Usd1Pane.hpp"
#include "Grp1Pane.hpp"
#include "Txt1Pane.hpp"

#include <stdexcept>
#include <algorithm>

using namespace std;
using namespace Panes;

bool BflytPatcher::ClearUVData(const std::string& name)
{
	auto target = lyt[name];
	if (!target || target->name != "pic1") return false;

	auto e = dynamic_pointer_cast<Pic1Pane>(target);
	for (auto& uv : e->UvCoords)
	{
		uv.TopLeft = { 0,0 };
		uv.TopRight = { 1,0 };
		uv.BottomLeft = { 0,1 };
		uv.BottomRight = { 1,1 };
	}
	return true;
}

bool BflytPatcher::ApplyLayoutPatch(const std::vector<PanePatch>& Patches)
{
	for (size_t i = 0; i < Patches.size(); i++)
	{
		auto target = lyt[Patches[i].PaneName];
		if (!target)
			//return false;
			continue;
		//The layout patching has been made less strict to allow some 8.x layouts to work on lower firmwares

		auto p = Patches[i];
		auto e = dynamic_pointer_cast<Pan1Pane>(target);

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

		if (p.ApplyFlags & (u32)PanePatch::Flags::OriginX)
			e->SetOriginX((OriginX)p.OriginX);
		if (p.ApplyFlags & (u32)PanePatch::Flags::OriginY)
			e->SetOriginY((OriginY)p.OriginY);
		if (p.ApplyFlags & (u32)PanePatch::Flags::P_OriginX)
			e->SetParentOriginX((OriginX)p.ParentOriginX);
		if (p.ApplyFlags & (u32)PanePatch::Flags::P_OriginY)
			e->SetParentOriginY((OriginY)p.ParentOriginY);

		if (e->name == "pic1")
		{
			auto ee = dynamic_pointer_cast<Pic1Pane>(e);

			if (p.ApplyFlags & (u32)PanePatch::Flags::PaneSpecific0)
				ee->ColorTopLeft = RGBAColor(p.PaneSpecific0());
			if (p.ApplyFlags & (u32)PanePatch::Flags::PaneSpecific1)
				ee->ColorTopRight = RGBAColor(p.PaneSpecific1());
			if (p.ApplyFlags & (u32)PanePatch::Flags::PaneSpecific2)
				ee->ColorBottomLeft = RGBAColor(p.PaneSpecific2());
			if (p.ApplyFlags & (u32)PanePatch::Flags::PaneSpecific3)
				ee->ColorBottomRight = RGBAColor(p.PaneSpecific3());
		}

		if (e->name == "txt1")
		{
			auto ee = dynamic_pointer_cast<Txt1Pane>(e);

			if (p.ApplyFlags & (u32)PanePatch::Flags::PaneSpecific0)
				ee->FontTopColor = RGBAColor(p.PaneSpecific0());
			if (p.ApplyFlags & (u32)PanePatch::Flags::PaneSpecific1)
				ee->ShadowTopColor = RGBAColor(p.PaneSpecific1());
			if (p.ApplyFlags & (u32)PanePatch::Flags::PaneSpecific2)
				ee->FontBottomColor = RGBAColor(p.PaneSpecific2());
			if (p.ApplyFlags & (u32)PanePatch::Flags::PaneSpecific3)
				ee->ShadowBottomColor = RGBAColor(p.PaneSpecific3());
		}

		if ((p.ApplyFlags & (u32)PanePatch::Flags::Usd1) && target->UserData)
		{
			auto usd = dynamic_cast<Usd1Pane*>(target->UserData.get());
			for (const auto& patch : p.UsdPatches)
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
	return true;
}

bool BflytPatcher::ApplyMaterialsPatch(const std::vector<MaterialPatch>& Patches)
{
	if (Patches.size() == 0) return true;
	auto mats = lyt.GetMatSection();
	if (!mats) return false;
	for (const auto& p : Patches)
	{
		auto target = find_if(mats->Materials.begin(), mats->Materials.end(), [&p](BflytMaterial m) {return m.Name == p.MaterialName; });
		if (target == mats->Materials.end()) continue; //Less strict patching
		if (p.ForegroundColor != "")
			(*target).ForegroundColor = (u32)std::stoul(p.ForegroundColor, 0, 16);
		if (p.BackgroundColor != "")
			(*target).BackgroundColor = (u32)std::stoul(p.BackgroundColor, 0, 16);
	}
	return true;
}

bool BflytPatcher::AddGroupNames(const std::vector<ExtraGroup>& Groups)
{
	if (Groups.size() == 0) return true;
	if (!lyt.GetRootGroup()) return false;

	vector<string> groupNames;
	vector<string> paneNames;
	//Populate list of group and pane names
	lyt.FindPane([&groupNames, &paneNames](const PanePtr& cur)
		{
			if (cur->PaneName == "") return false;
			if (cur->name == "grp1")
				groupNames.push_back(dynamic_pointer_cast<Grp1Pane>(cur)->GroupName);
			else
				paneNames.push_back(cur->PaneName);
			return false;
		});

	for (const auto& g : Groups)
	{
		if (Utils::IndexOf(groupNames, g.GroupName) != SIZE_MAX) continue;
		for (const auto& s : g.Panes)
			if (Utils::IndexOf(paneNames, s) == SIZE_MAX) return false;
		auto toAdd = new Grp1Pane(lyt.Version);
		toAdd->GroupName = g.GroupName;
		toAdd->Panes = g.Panes;
		lyt.GetRootGroup()->Children.emplace_back(toAdd);
	}
	return true;
}

bool BflytPatcher::PatchTextureName(const std::string& original, const std::string& _new)
{
	bool patchedSomething = false;
	auto texSection = lyt.GetTexSection();
	if (texSection == nullptr)
		throw runtime_error("this layout doesn't have any texture section (?)");
	for (size_t i = 0; i < texSection->Textures.size(); i++)
	{
		if (texSection->Textures[i] == original)
		{
			patchedSomething = true;
			texSection->Textures[i] = _new;
		}
	}
	return patchedSomething;
}

u16 BflytPatcher::AddBgMat(const std::string& texName)
{
	auto MatSect = lyt.GetMatSection();
	//Add texture
	auto texSection = lyt.GetTexSection();
	if (!MatSect || !texSection) return false;
	size_t texIndex = Utils::IndexOf(texSection->Textures, texName);
	if (texIndex == SIZE_MAX)
	{
		texIndex = texSection->Textures.size();
		texSection->Textures.push_back(texName);
	}
	//Add material
	{
		Buffer bin;
		bin.ByteOrder = Endianness::LittleEndian;
		bin.Write("P_Custm", Buffer::BinaryString::NullTerminated);
		for (size_t i = 0; i < 0x14; i++)
			bin.Write((u8)0);
		bin.Write((s32)0x15);
		bin.Write((s32)0x8040200);
		bin.Write((s32)0);
		bin.Write((u32)0xFFFFFFFF);
		bin.Write((u16)texIndex);
		bin.Write((u16)0x0);
		for (size_t i = 0; i < 0xC; i++)
			bin.Write((u8)0);
		bin.Write((float)1);
		bin.Write((float)1);
		for (size_t i = 0; i < 0x10; i++)
			bin.Write((u8)0);
		MatSect->Materials.push_back(BflytMaterial{ bin.getBuffer() , lyt.Version, bin.ByteOrder });
	}
	return u16(MatSect->Materials.size() - 1);
}

bool BflytPatcher::AddBgPanel(PanePtr target, const std::string& TexName, const std::string& Pic1Name)
{
	if (Pic1Name.length() > 0x18)
		throw runtime_error("Pic1Name should not be longer than 24 chars");
	auto BgPane = new BasePane("pic1", 0x8);
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
		BgPane->data = bin.getBuffer();
	}
	auto ptr = target->Parent.lock();
	auto& targetCList = ptr->Children;
	targetCList.emplace(targetCList.begin() + Utils::IndexOf(targetCList, target), BgPane);
	return true;
}

bool BflytPatcher::PatchBgLayout(const PatchTemplate& patch)
{
	//Detect patch
	if (lyt[patch.PatchIdentifier]) return true;
	if (auto p = lyt["3x3lxBG"])
	{
		lyt.RemovePane(p);
		lyt.GetTexSection()->Textures[0] = "White1x1^r";
		lyt.GetMatSection()->Materials.erase(lyt.GetMatSection()->Materials.begin() + 1);
	}
	//Find and remove target panes
	PanePtr target = nullptr;
	for (const auto& tname : patch.targetPanels)
	{
		auto p = lyt[tname];
		if (!p) continue;
		if (!target) target = p;
		if (patch.DirectPatchPane)
		{
			auto m = AddBgMat(patch.MaintextureName);
			if (p->name != "pic1") throw runtime_error("Expected a picture pane !");
			dynamic_pointer_cast<Pic1Pane>(p)->MaterialIndex = m;
		}
		else if (!patch.NoRemovePanel)
		{
			auto t = dynamic_pointer_cast<Pan1Pane>(p);
			t->Position.X = 5000;
			t->Position.Y = 60000;
		}
	}
	if (!target)
		return false;

	if (!patch.DirectPatchPane)
		return AddBgPanel(target, patch.MaintextureName, patch.PatchIdentifier);
	else return true;
}

bool BflytPatcher::PanePullToFront(const std::string& paneName)
{
	auto target = lyt[paneName];
	if (!target) return false;
	auto ptr = target->Parent.lock();
	if (!ptr) return false;
	lyt.MovePane(target, ptr, 0);
	return true;
}

bool BflytPatcher::PanePushBack(const std::string& paneName)
{
	auto target = lyt[paneName];
	if (!target) return false;
	auto ptr = target->Parent.lock();
	if (!ptr) return false;
	lyt.MovePane(target, ptr, ptr->Children.size());
	return true;
}
