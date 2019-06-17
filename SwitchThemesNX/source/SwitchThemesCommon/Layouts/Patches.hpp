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

	std::string ColorTL;
	std::string ColorTR;
	std::string ColorBL;
	std::string ColorBR;

	u32 ApplyFlags; //to disable the properties set to null in the json

	enum class Flags : u32 
	{
		Visible = 1,
		Position = 1 << 1,
		Rotation = 1 << 2,
		Scale = 1 << 3,
		Size = 1 << 4,
		ColorTL = 1 << 5,
		ColorTR = 1 << 6,
		ColorBL = 1 << 7,
		ColorBR = 1 << 8,
		Usd1 = 1 << 9
	};

	std::vector<UsdPatch> UsdPatches;
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
	bool Ready8X = false;

	bool IsCompatible(const SARC::SarcData &sarc);
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

namespace Patches {
	extern std::vector<PatchTemplate> DefaultTemplates;
	LayoutPatch LoadLayout(const std::string &json);
}

namespace NewFirmFixes 
{
	extern std::vector<LayoutFilePatch> GetFix(const std::string& LayoutName);
}