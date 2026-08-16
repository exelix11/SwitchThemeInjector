#include "ThemeEntry.hpp"
#include "../../fs.hpp"
#include "../../SwitchTools/PatchMng.hpp"
#include "../../SwitchThemesCommon/SarcLib/Yaz0.hpp"
#include "../../SwitchThemesCommon/Patcher.hpp"

LegacyEntry::LegacyEntry(const std::string& fileName, std::vector<u8>&& RawData)
{
	FileName = fileName;
	file = RawData;
	Icon = Icons::Type::File;
	auto DecompressedFile = Yaz0::Decompress(file);
	ParseLegacyTheme(SARC::Unpack(DecompressedFile));
}

LegacyEntry::LegacyEntry(const std::string& fileName, SARC::SarcData&& _SData)
{
	FileName = fileName;

	auto packed = SARC::Pack(_SData);
	file = move(Yaz0::Compress(packed.data));

	ParseLegacyTheme(std::move(_SData));
}

bool LegacyEntry::DoInstall(bool ShowDialogs)
{
	if (ShowDialogs)
		ThemeEntry::DisplayInstallDialog(FileName);

	auto patch = SwitchThemesCommon::SzsPatcher::DetectSarc(SData);

	if (!patch)
		throw std::runtime_error("Couldn't find a compatible patch template");

	if (!PatchMng::ExefsCompatAsk(patch->SzsName))
		return false;

	fs::theme::CreateStructure(patch->TitleId);
	fs::WriteFile(fs::path::RomfsFolder(patch->TitleId) + "lyt/" + patch->SzsName, file);

	return true;
}

void LegacyEntry::ParseLegacyTheme(SARC::SarcData&& _Sdata)
{
	SData = _Sdata;
	if (FileName == "")
	{
		lblFname = "Unknown.szs";
		lblLine1 = "Remote install";
	}
	else
	{
		lblFname = fs::GetFileName(FileName);
		lblLine1 = FileName;
	}

	auto patch = SwitchThemesCommon::SzsPatcher::DetectSarc(SData);
	if (!patch)
		MakeError("Couldn't find a compatible patch template");
	else 
		lblRightSide = (patch->TemplateName + " for " + patch->FirmName);

	canInstallInternal = true;
}