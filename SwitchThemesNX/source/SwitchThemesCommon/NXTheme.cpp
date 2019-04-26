#include <map>
#include <unordered_map>
#include "NXTheme.hpp"
#include "BinaryReadWrite/Buffer.hpp"
#include "Layouts/json.hpp"
#include "SwitchThemesCommon.hpp"

#ifdef  __SWITCH__
int NXTheme_FirmMajor = -1;
#else
int NXTheme_FirmMajor = 8; //For code running on pc emulate 8.x firm to match the injector
#endif //  __SWITCH__

std::unordered_map<std::string,std::string> ThemeTargetToName {};
std::unordered_map<std::string,std::string> ThemeTargetToFileName {};

using namespace std;
using json = nlohmann::json;

ThemeFileManifest ParseNXThemeFile(SARC::SarcData &Archive)
{
	if (!Archive.files.count("info.json"))
	{
		return {-1,"","",""};
	}
	string jsn(reinterpret_cast<char*>((Archive.files["info.json"]).data()),(Archive.files["info.json"]).size());
	auto j = json::parse(jsn);
	
	ThemeFileManifest res = {0};
	if (j.count("Version") && j.count("Target"))
	{
		res.Version = j["Version"].get<int>();
		res.Target = j["Target"].get<string>();
	}
	else 
	{
		res.Version = -1;
		return res;
	}
	if (j.count("Author"))
		res.Author = j["Author"].get<string>();
	if (j.count("ThemeName"))
		res.ThemeName = j["ThemeName"].get<string>();
	if (j.count("LayoutInfo"))
		res.LayoutInfo = j["LayoutInfo"].get<string>();	
	
	return res;
}