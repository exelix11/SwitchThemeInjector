#include "ThemeEntry.hpp"
#include "../../fs.hpp"
#include "../../SwitchThemesCommon/Fonts/TTF.hpp"
#include "../../ViewFunctions.hpp"
#include "../ThemePage.hpp"

class FontEntry : public ThemeEntry
{
public:
	FontEntry(const std::string& fileName, std::vector<u8>&& RawData)
	{
		FileName = fileName;
		file = RawData;
		ParseFont();
	}

	bool IsFolder() override { return false; }
	bool CanInstall() override { return _CanInstall; }
	bool HasPreview() override { return false; }
protected:
	bool DoInstall(bool ShowDialogs = true) override
	{
		if (ShowDialogs)
			ThemeEntry::DisplayInstallDialog(FileName);

		fs::theme::CreateMitmStructure("0100000000000811");
		fs::theme::CreateRomfsDir("0100000000000811");
		fs::WriteFile(fs::path::RomfsFolder("0100000000000811") + "nintendo_udsg-r_std_003.bfttf", SwitchThemesCommon::TTF::ConvertToBFTTF(file));
		fs::theme::CreateMitmStructure("0100000000000039");
		fs::theme::CreateRomfsDir("0100000000000039");
		fs::WriteFile(fs::path::RomfsFolder("0100000000000039") + "dummy.bin", { 0x70,0x61,0x70,0x65,0x20,0x53,0x61,0x74,0x61,0x6E,0x20,0x41,0x6C,0x65,0x70,0x70,0x65,0x21 });
		return true;
	}
private:
	bool _CanInstall = true;

	LoadedImage GetPreview() override
	{
		throw std::runtime_error("Preview is not implemented for fonts");
	}

	void ParseFont()
	{
		lblLine2 = ("Custom font");
		auto fontName = SwitchThemesCommon::TTF::GetFontName(file);
		_CanInstall = fontName != "";
		CannotInstallReason = "Invalid font file";
		lblFname = (CanInstall() ? fontName : "Invalid font :(");
		lblLine1 = (fs::GetFileName(FileName));
	}
};