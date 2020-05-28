#pragma once
#include <string>
#include <vector>
#include "../MyTypes.h"
#include "../SarcLib/Sarc.hpp"

struct Vector3 { float X, Y, Z; };
struct Vector2 { float X, Y; };

struct UsdPatch 
{
	std::string PropName;
	std::vector<std::string> PropValues;
	int type;
};

struct PanePatch
{
	std::string PaneName;
	Vector3 Position, Rotation;
	Vector2 Scale, Size;
	bool Visible;

	u8 OriginX;
	u8 OriginY;
	u8 ParentOriginX;
	u8 ParentOriginY;

	u32 ApplyFlags; //to disable the properties set to null in the json

	enum class Flags : u32 
	{
		Visible = 1,
		Position = 1 << 1,
		Rotation = 1 << 2,
		Scale = 1 << 3,
		Size = 1 << 4,
		PaneSpecific0 = 1 << 5,
		PaneSpecific1 = 1 << 6,
		PaneSpecific2 = 1 << 7,
		PaneSpecific3 = 1 << 8,
		Usd1 = 1 << 9,
		OriginX = 1 << 10,
		OriginY = 1 << 11,
		P_OriginX = 1 << 12,
		P_OriginY = 1 << 13,
	};

	std::vector<UsdPatch> UsdPatches;

	// These fields were originally used for color data in PIC1 panes, now they can be used for pane-specific data 
	// For compatibility reasons they keep the original name in the JSON

	// JSON : ColorTL
	// PIC1 : Top left color
	// TXT1 : Top font color
	inline std::string& PaneSpecific0() { return PaneSpecific[0]; }
	
	// JSON : ColorTR
	// PIC1 : Top right color
	// TXT1 : Top shadow color
	inline std::string& PaneSpecific1() { return PaneSpecific[1]; }
	
	// JSON : ColorBL
	// PIC1 : Bottom left color
	// TXT1 : Bottom font color
	inline std::string& PaneSpecific2() { return PaneSpecific[2]; }

	// JSON : ColorBR
	// PIC1 : Bottom right color
	// TXT1 : Bottom shadow color
	inline std::string& PaneSpecific3() { return PaneSpecific[3]; }
private:
	std::string PaneSpecific[4];
};

struct ExtraGroup
{
	std::string GroupName;
	std::vector<std::string> Panes;
};

struct MaterialPatch
{
	std::string MaterialName;
	std::string ForegroundColor;
	std::string BackgroundColor;
};

struct LayoutFilePatch 
{
	std::string FileName;
	std::vector<PanePatch> Patches;
	std::vector<ExtraGroup> AddGroups;
	std::vector<MaterialPatch> Materials;

	std::vector<std::string> PushBackPanes;
	std::vector<std::string> PullFrontPanes;
};

struct AnimFilePatch
{
	std::string FileName;
	std::string AnimJson;
};

struct LayoutPatch 
{
	std::string PatchName;
	std::string AuthorName;
	std::vector<LayoutFilePatch> Files;
	std::vector<AnimFilePatch> Anims;
	bool PatchAppletColorAttrib = false;
	std::string ID;

	bool Obsolete_Ready8X = false;
	bool UsesOldFixes() const { return ID == "" && !Obsolete_Ready8X; }

	bool IsCompatible(const SARC::SarcData &sarc) const;
};

struct PatchTemplate 
{
	std::string TemplateName;
	std::string szsName;
	std::string TitleId;
	std::string FirmName;

	std::vector<std::string> FnameIdentifier;
	std::vector<std::string> FnameNotIdentifier;

	std::string MainLayoutName;
	std::string MaintextureName;
	std::string PatchIdentifier;
	std::vector<std::string> targetPanels;
	std::string SecondaryTexReplace;

	bool DirectPatchPane = false;
	bool NoRemovePanel = false;
};

struct TextureReplacement 
{
	std::string NxThemeName;
	std::string BntxName;
	u32 NewColorFlags;
	std::string FileName;
	std::string PaneName;
	s32 W, H;
	LayoutFilePatch patch;
};

namespace Patches {
	extern std::vector<PatchTemplate> DefaultTemplates;

	namespace textureReplacement {
		//extern std::vector<TextureReplacement> ResidentMenu;
		//extern std::vector<TextureReplacement> Entrance;
		extern std::unordered_map < std::string, std::vector<TextureReplacement>> NxNameToList;
	}

	LayoutPatch LoadLayout(const std::string &json);
}

namespace NewFirmFixes 
{
	std::vector<LayoutFilePatch> GetFixLegacy(const std::string& LayoutName, const std::string& NXThemeName);
	std::vector<LayoutFilePatch> GetFix(const std::string& LayoutID, const std::string& NxPart);
}