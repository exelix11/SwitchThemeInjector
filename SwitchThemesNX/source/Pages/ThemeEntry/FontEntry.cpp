#include "ThemeEntry.hpp"
#include "../../fs.hpp"
#include "../../SwitchThemesCommon/Fonts/TTF.hpp"
#include "../../ViewFunctions.hpp"
#include "../ThemePage.hpp"

FontEntry::FontEntry(const std::string& fileName, std::vector<u8>&& RawData)
{
	FileName = fileName;
	file = RawData;
	Icon = Icons::Type::Font;
	ParseFont();
}

bool FontEntry::DoInstall(bool ShowDialogs)
{
	if (ShowDialogs)
		ThemeEntry::DisplayInstallDialog(FileName);

	fs::theme::CreateMitmStructure("0100000000000811");
	fs::theme::CreateRomfsDir("0100000000000811");
	fs::WriteFile(fs::path::RomfsFolder("0100000000000811") + "nintendo_udsg-r_std_003.bfttf", SwitchThemesCommon::TTF::ConvertToBFTTF(file));
	fs::theme::CreateMitmStructure("0100000000000039");
	fs::theme::CreateRomfsDir("0100000000000039");
	fs::WriteFile(fs::path::RomfsFolder("0100000000000039") + "dummy.bin", std::vector<u8> { 0x36, 0x36, 0x36 });
	return true;
}


void FontEntry::ParseFont()
{
	auto fontName = SwitchThemesCommon::TTF::GetFontName(file);
	canInstallInternal = true;

	if (fontName == "")
		MakeError("Invalid font file");

	lblFname = (CanInstall() ? fontName : "Invalid font");
	lblLine1 = (fs::GetFileName(FileName));
}