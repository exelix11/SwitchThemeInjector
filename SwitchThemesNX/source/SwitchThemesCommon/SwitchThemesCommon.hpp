#pragma once
#include <string>
#include <vector>
#include "MyTypes.h"
#include "SarcLib/Sarc.hpp"
#include "SarcLib/Yaz0.hpp"
#include "Layouts/Patches.hpp"
#include "Fonts/TTF.hpp"
#include "Bntx/QuickBntx.hpp"

namespace SwitchThemesCommon {
	
	extern const std::string CoreVer;
	extern const int NXThemeVer;

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
		
		bool PatchAnimations(const std::vector<AnimFilePatch>& files);
		bool PatchLayouts(const LayoutPatch& patch, const std::string& PartName);
		bool PatchMainBG(const std::vector<u8>& DDS);
		bool PatchAppletIcon(const std::vector<u8>& DDS, const std::string& texName);
		bool PatchBntxTexture(const std::vector<u8>& DDS, const std::string& texName, u32 ChannelData = 0xFFFFFFFF);
		bool PatchBntxTextureAttribs(const std::vector<BntxTexAttribPatch>& patches);
		PatchTemplate DetectSarc();
		static PatchTemplate DetectSarc(const SARC::SarcData&);

		void SetPatchAnimations(bool);

		const SARC::SarcData& GetSarc();
		SARC::SarcData& GetFinalSarc();

	private:
		bool PatchSingleLayout(const LayoutFilePatch& p);

		SARC::SarcData sarc;
		QuickBntx* bntx = nullptr;

		QuickBntx* OpenBntx();
		void SaveBntx();

		bool EnableAnimations = true;
	};
	
	std::string GeneratePatchListString(const std::vector < PatchTemplate >& templates);
}