#pragma once
#include <string>
#include <vector>
#include "MyTypes.h"
#include "SarcLib/Sarc.hpp"
#include "SarcLib/Yaz0.hpp"
#include "Layouts/Bflyt.hpp"
#include "Layouts/Patches.hpp"
#include "Fonts/TTF.hpp"
#include "Bntx/QuickBntx.hpp"

namespace SwitchThemesCommon {
	
	extern const std::string CoreVer;

	struct BntxTexAttribPatch
	{
		std::string TargetTexutre;
		u32 ChannelData;
	};
	
	class SzsPatcher 
	{
	public:
		SzsPatcher(SARC::SarcData&& s);
		SzsPatcher(SARC::SarcData& s);
		~SzsPatcher();
		
		BflytFile::PatchResult PatchAnimations(const std::vector<AnimFilePatch>& files);
		BflytFile::PatchResult PatchLayouts(const LayoutPatch& patch, const std::string& PartName, bool Fix8x);
		BflytFile::PatchResult PatchMainBG(const std::vector<u8>& DDS);
		BflytFile::PatchResult PatchAppletIcon(const std::vector<u8>& DDS, const std::string& texName);
		BflytFile::PatchResult PatchBntxTexture(const std::vector<u8>& DDS, const std::string& texName, u32 ChannelData = 0xFFFFFFFF);
		BflytFile::PatchResult PatchBntxTextureAttribs(const std::vector<BntxTexAttribPatch>& patches);
		PatchTemplate DetectSarc();
		static PatchTemplate DetectSarc(const SARC::SarcData&);

		void SetPatchAnimations(bool);

		const SARC::SarcData& GetSarc();
		SARC::SarcData& GetFinalSarc();

	private:
		BflytFile::PatchResult PatchSingleLayout(const LayoutFilePatch& p);

		SARC::SarcData sarc;
		QuickBntx* bntx = nullptr;

		QuickBntx* OpenBntx();
		void SaveBntx();

		bool EnableAnimations = true;
	};
	
	std::string GeneratePatchListString(const std::vector < PatchTemplate >& templates);
}