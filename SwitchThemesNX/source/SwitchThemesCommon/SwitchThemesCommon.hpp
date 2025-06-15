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

	enum class LayoutCompatibilityOption : int
	{
		// Layout fixes will be applied automatically using our heuristics and version detection
		Default,
		// Disable all layout fixes
		DisableFixes,
		// Forces pre-11.0 layout by removing the new applet icons
		RemoveHomeAppletIcons
	};
	
	class SzsPatcher 
	{
	public:
		SzsPatcher(SARC::SarcData&& s);
		SzsPatcher(SARC::SarcData& s);
		~SzsPatcher();

		LayoutCompatibilityOption CompatFixes = LayoutCompatibilityOption::Default;
		
		bool PatchLayouts(const LayoutPatch& patch);
		bool PatchLayouts(const LayoutPatch& patch, const std::string& PartName);
		bool PatchMainBG(const std::vector<u8>& DDS);
		bool PatchAppletIcon(const std::vector<u8>& DDS, const std::string& texName);
		bool PatchBntxTexture(const std::vector<u8>& DDS, const std::vector<std::string>& texNames, u32 ChannelData = 0xFFFFFFFF);
		bool PatchBntxTextureAttribs(const std::vector<BntxTexAttribPatch>& patches);
		static std::optional<PatchTemplate> DetectSarc(const SARC::SarcData&);

		const std::optional<PatchTemplate>& DetectedSarc();

		const SARC::SarcData& GetSarc();
		SARC::SarcData& GetFinalSarc();

		int TotalNonCompatibleFixes = 0;
	private:		
		SARC::SarcData sarc;
		ConsoleFirmware currentFirmware;
		std::optional<PatchTemplate> currentTemplate;
		std::string nxthemePartName;

		void Initialize();

		QuickBntx* bntx = nullptr;
		QuickBntx& OpenBntx();
		void SaveBntx();
		
		bool EnableAnimations = true;

		void ApplyRawPatch(const std::optional<LayoutPatch>& p);
		void ApplyRawPatch(const LayoutPatch* p);

		std::optional<uint32_t> FirmwareTargetBflanVersion = std::nullopt;
		bool ApplyAnimPatch(const AnimFilePatch& p);
		bool ApplyLayoutPatch(const LayoutFilePatch& p);

		int FilterIncompatibleAnimations(LayoutPatch& p);
	};
	
	std::string GeneratePatchListString(const std::vector<PatchTemplate>& templates);
}