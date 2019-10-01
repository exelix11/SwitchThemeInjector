#include "ThemeEntry.hpp"
#include "../../fs.cpp"

using namespace std;

class FontEntry : public ThemeEntry
{
public:
	FontEntry(const string& fileName, vector<u8>&& RawData)
	{
		FileName = fileName;
		file = move(RawData);
		ParseFont();
	}

	bool IsFolder() override { return false; }
	bool CanInstall() override { return _CanInstall; }
	bool HasPreview() override { return false; }
protected:
	bool DoInstall(bool ShowDialogs = true, const string& homeDirOverride = "") override
	{
		if (homeDirOverride != "")
		{
			DialogBlocking("Can't install a font to theme shuffle.");
			return false;
		}

		if (ShowDialogs)
			DisplayLoading("Installing font...");

		fs::CreateFsMitmStructure("0100000000000811");
		fs::CreateRomfsDir("0100000000000811");
		fs::WriteFile(CfwFolder + "/titles/0100000000000811/romfs/nintendo_udsg-r_std_003.bfttf", SwitchThemesCommon::TTF::ConvertToBFTTF(file));
		fs::CreateFsMitmStructure("0100000000000039");
		fs::CreateRomfsDir("0100000000000039");
		fs::WriteFile(CfwFolder + "/titles/0100000000000039/romfs/dummy.bin", { 0x70,0x61,0x70,0x65,0x20,0x53,0x61,0x74,0x61,0x6E,0x20,0x41,0x6C,0x65,0x70,0x70,0x65,0x21 });
		return true;
	}
private:
	bool _CanInstall = true;

	LoadedImage GetPreview() override
	{
		throw runtime_error("Preview is not implemented for fonts");
	}

	void ParseFont()
	{
		lblLine2 = ("Custom font");
		auto fontName = SwitchThemesCommon::TTF::GetFontName(file);
		_CanInstall = fontName != "";
		lblFname = (CanInstall ? fontName : "Invalid font :(");
		lblLine1 = (fs::GetFileName(FileName));
	}
};