#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "../../BinaryReadWrite/Buffer.hpp"
#include "../../MyTypes.h"
#include "../Patches.hpp"
#include "Bflyt.hpp"

class BflytPatcher 
{
public:
	BflytPatcher(BflytFile& layout) : lyt(layout), Panes(layout.Panes) {}

	bool ClearUVData(const std::string& name);
	
	bool ApplyLayoutPatch(const std::vector<PanePatch>& Patches);
	
	bool ApplyMaterialsPatch(const std::vector<MaterialPatch>& Patches);

	bool AddGroupNames(const std::vector<ExtraGroup>& Groups);

	bool PatchTextureName(const std::string& original, const std::string& _new);

	u16 AddBgMat(const std::string& texName);

	bool AddBgPanel(int index, const std::string &TexName, const std::string &Pic1Name);
	
	bool PatchBgLayout(const PatchTemplate& patch);	
private:
	BflytFile& lyt;
	std::vector<Panes::PanePtr>& Panes;

	auto GetPaneNames() { return lyt.GetPaneNames(); }
	auto GetGroupNames() { return lyt.GetGroupNames(); }
};
