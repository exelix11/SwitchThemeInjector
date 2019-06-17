#pragma once
#include <string>
#include <vector>
#include "MyTypes.h"
#include "SarcLib/Sarc.hpp"
#include "SarcLib/Yaz0.hpp"
#include "Layouts/Bflyt.hpp"
#include "Layouts/Patches.hpp"
#include "Fonts/TTF.hpp"

namespace SwitchThemesCommon {
	
	struct BntxTexAttribPatch
	{
		std::string TargetTexutre;
		u32 ChannelData;
	};
	
	extern const std::string CoreVer;
	
	std::string GeneratePatchListString(const std::vector < PatchTemplate >& templates);

	BflytFile::PatchResult PatchAnimations(SARC::SarcData& sarc, const std::vector<AnimFilePatch> &files);

	BflytFile::PatchResult PatchLayouts(SARC::SarcData &sarc, const LayoutPatch &patch, const std::string &PartName, bool Fix8x, bool AddAnimations);

	BflytFile::PatchResult PatchBgLayouts(SARC::SarcData &sarc, const PatchTemplate &layout);

	BflytFile::PatchResult PatchBntx(SARC::SarcData &sarc, const std::vector<u8> &DDS, const PatchTemplate &targetPatch);

	BflytFile::PatchResult PatchBntxTexture(SARC::SarcData &sarc, const std::vector<u8> &DDS, const std::string &texName, u32 ChannelData = 0xFFFFFFFF);
	
	BflytFile::PatchResult PatchBntxTextureAttribs(SARC::SarcData &sarc, const std::vector<BntxTexAttribPatch> &patches);

	PatchTemplate DetectSarc(const SARC::SarcData &sarc);
}