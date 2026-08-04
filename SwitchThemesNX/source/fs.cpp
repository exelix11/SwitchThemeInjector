#include <cstring>
#include <sstream>
#include <filesystem>

#include "fs.hpp"
#include "SwitchThemesCommon/Common.hpp"

#define FS_TROUBLESHOOT_MSG \
	"Make sure the file exists, this can also be caused by sd corruption with exfat or the archive bit, especially if you used this sd card with a mac.\n" \
	"Try removing the archive bit from the themes folder on a windows pc or with hekate, alternatively delete themes folder and copy the files via FTP"

using namespace std;
using namespace fs;

bool StrEndsWith(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() &&
		str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool StrStartsWith(const std::string& str, const std::string& prefix)
{
	return str.size() >= prefix.size() &&
		str.compare(0, prefix.size(), prefix) == 0;
}

static string CfwFolder = "";
static string TitlesFolder = "";
static bool ThemeListDirty = true;

const std::string& fs::path::CfwFolder() { return ::CfwFolder; }

std::string fs::path::FsMitmFolder() { return ::CfwFolder + TitlesFolder; }

std::string fs::path::RomfsFolder(const std::string& contentID)
{
	return path::FsMitmFolder() + contentID + "/romfs/";
}

std::string fs::path::GetFreeDownloadFolder()
{
	std::stringstream ss;

	for (size_t i = 1; i < std::numeric_limits<size_t>::max(); i++) {
		ss.str("");
		ss.clear();
		ss << DownloadsFolder << "Group " << i;
		if (!fs::Exists(ss.str()))
			return ss.str();
	}

	return "";
}

string& fs::path::ToUnixSeparators(string& str)
{
	char* c = str.data();
	while (*c)
	{
		if (*c == '\\')
			*c = '/';
		c++;
	}

	return str;
}

static vector<string> GetThemeFilesInDirRecursive(const string& path, int level)
{
	vector<string> res;
	if (level > 7) return res;
	for (auto p : filesystem::directory_iterator(path))
	{
		if (p.is_directory())
		{
			// Folders excluded from search
			if (p.path().filename() == "systemData" ||
				p.path().filename() == "systemPatches")
				continue;

			auto path = p.path().string();
			res.push_back(fs::path::ToUnixSeparators(path));

			auto v = GetThemeFilesInDirRecursive(p.path().string(), level + 1);
			res.insert(res.end(), v.begin(), v.end());
		}
		else if (p.is_regular_file())
		{
			if (StrEndsWith(p.path().string(), ".szs") ||
				StrEndsWith(p.path().string(), ".jpg") ||
				StrEndsWith(p.path().string(), ".jpeg") ||
				StrEndsWith(p.path().string(), ".png") ||
				StrEndsWith(p.path().string(), ".nxtheme") ||
				StrEndsWith(p.path().string(), ".zip") ||
				StrEndsWith(p.path().string(), ".ttf")) {
				auto str = p.path().string();
				res.push_back(fs::path::ToUnixSeparators(str));
			}
		}
	}
	return res;
}

vector<u8> fs::OpenFile(const string& name)
{
	FILE* f = fopen(name.c_str(), "rb");
	if (!f)
		throw std::runtime_error("Opening file " + name + " failed !\n" FS_TROUBLESHOOT_MSG);

	fseek(f, 0, SEEK_END);
	size_t len = 0;
	{
		auto fsz = ftell(f);
		if (fsz < 0)
			throw std::runtime_error("Reading file size for " + name + " failed !\n" FS_TROUBLESHOOT_MSG);
		len = fsz;
	}
	rewind(f);

	vector<u8> coll(len);
	if (fread(coll.data(), 1, len, f) != len)
		throw std::runtime_error("Reading from file " + name + " failed !\n" FS_TROUBLESHOOT_MSG);

	fclose(f);
	return coll;
}

void fs::WriteFile(const string& name, std::span<const u8> data)
{
	if (filesystem::exists(name))
		remove(name.c_str());

	FILE* f = fopen(name.c_str(), "wb");
	if (!f)
		throw std::runtime_error("Saving file " + name + "failed !");

	fwrite(data.data(), 1, data.size(), f);
	fflush(f);
	fclose(f);
}

bool fs::Exists(const std::string& name) { return std::filesystem::exists(name); }

bool fs::DirectoryExists(const std::string& name) { return std::filesystem::is_directory(name); }

void fs::Delete(const std::string& path) { std::filesystem::remove(path); }

void fs::CreateDirectory(const std::string& path) { std::filesystem::create_directories(path); }

bool fs::CheckFlagFile(const std::string& name)
{
	return Exists(fs::path::SystemDataFolder + ".flag_" + name);
}

void fs::SetFlagFile(const std::string& name, bool value)
{
	auto target = fs::path::SystemDataFolder + ".flag_" + name;
	try {
		auto exists = fs::Exists(target);
		if (value && !exists)
			fs::WriteFile(target, {});
		else if (!value && exists)
			fs::Delete(target);
	}
	catch (...)
	{
		// This is not critical, ignore errors
	}
}

void fs::DeleteDirectory(const std::string& path) {
	// remove_all fails for some reason so we must iterate manually.
	if (!std::filesystem::is_directory(path))
	{
		if (fs::Exists(path))
			fs::Delete(path);

		return;
	}

	for (auto& p : std::filesystem::directory_iterator(path))
	{
		if (p.is_directory())
			DeleteDirectory(p.path().string());
		else
			std::filesystem::remove(p.path());
	}

	std::filesystem::remove(path);
}

std::string fs::SanitizeName(const std::string& name)
{
	const char* forbiddenChars = "/?<>\\:*|\".";

	std::string res = name.length() > 30 ? name.substr(0, 30) : name;
	char* c = res.data();
	while (*c)
	{
		if (std::strchr(forbiddenChars, *c))
			*c = '_';
		c++;
	}

	return res;
}

bool fs::EnsureThemesFolderExists()
{
	if (!filesystem::exists(path::ThemesFolder))
		CreateDirectory(path::ThemesFolder);

	bool Result = filesystem::exists(path::SystemDataFolder);
	if (!Result)
		CreateDirectory(path::SystemDataFolder);

	patches::CreateFolder();

	return Result;
}

void fs::EnsureDownloadsFolderExists()
{
	if (!filesystem::exists(path::DownloadsFolder))
		CreateDirectory(path::DownloadsFolder);
}

string fs::GetPath(const string& path)
{
	return path.substr(0, path.find_last_of("/\\") + 1);
}

string fs::GetParentDir(const string& path)
{
	string _path = path;
	if (StrEndsWith(_path, "/"))
		_path = _path.substr(0, _path.length() - 1);

	return _path.substr(0, _path.find_last_of("/\\") + 1);
}

string fs::JoinPath(const string& first, const string& second)
{
	string path = first;
	if (!StrEndsWith(path, "/"))
		path.append("/");

	path.append(second);
	return path;
}

void fs::RemoveSystemDataDir()
{
	DeleteDirectory(path::SystemDataFolder);
	CreateDirectory(path::SystemDataFolder);
}

vector<string> fs::theme::ScanThemeFiles()
{
	vector<string> res;

	{
		DIR* dir = opendir(path::ThemesFolder.c_str());
		if (dir)
			closedir(dir);
		else
			return res;
	}

	res = GetThemeFilesInDirRecursive(path::ThemesFolder, 0);

	ThemeListDirty = false;
	return res;
}

void fs::theme::RequestThemeListRefresh()
{
	ThemeListDirty = true;
}

bool fs::theme::ShouldRescanThemeList()
{
	return ThemeListDirty;
}

void fs::theme::UninstallTheme(bool full)
{
	if (full)
	{
		DeleteDirectory(path::FsMitmFolder() + "0100000000001000");
		DeleteDirectory(path::FsMitmFolder() + "0100000000001013");
	}
	else
	{
		DeleteDirectory(path::FsMitmFolder() + "0100000000001000/romfs");
		DeleteDirectory(path::FsMitmFolder() + "0100000000001013/romfs");

		// Additionally, delete any backup folders that have been created by the update check sysmodule
		if (std::filesystem::is_directory(path::FsMitmFolder() + "0100000000001000"))
		{
			for (auto& p : std::filesystem::directory_iterator(path::FsMitmFolder() + "0100000000001000"))
			{
				if (!p.is_directory())
					continue;
					
				if (!fs::GetFileName(p.path().string()).starts_with("bak_"))
					continue;

				if (Exists((p.path() / "old_version_hash.bin").string()))
				{
					DeleteDirectory(p.path().string());
				}
			}
		}
	}

	DeleteDirectory(path::FsMitmFolder() + "0100000000001007"); //Player select
	DeleteDirectory(path::FsMitmFolder() + "0100000000000811"); //Custom font
	DeleteDirectory(path::FsMitmFolder() + "0100000000000039"); //needed to enable custom font
}

void fs::theme::CreateMitmStructure(const string& id)
{
	string path = path::FsMitmFolder();
	CreateDirectory(path);
	path += id + "/";
	CreateDirectory(path);
	if (!filesystem::exists(path + "fsmitm.flag"))
	{
		vector<u8> t = {};
		WriteFile(path + "fsmitm.flag", t);
	}
}

void fs::theme::CreateRomfsDir(const std::string& id)
{
	CreateDirectory(path::RomfsFolder(id));
}

void fs::theme::CreateStructure(const string& id)
{
	CreateMitmStructure(id);
	CreateDirectory(path::RomfsFolder(id) + "lyt");
}

void fs::theme::WriteSystemVersionFile()
{
	static bool VersionAlreadyWritten = false;

	// The system version can't change during runtime so we can only write it once
	if (VersionAlreadyWritten)
		return;

	try
	{
		WriteFile(
			fs::path::FsMitmFolder() + ThemeTargetInfo::TitleIdToString(ThemeTargetInfo::QlaunchID) + "/version_hash.bin",
			hos::VersionHash);

		VersionAlreadyWritten = true;
	}
	catch (const std::exception&)
	{
		// Ignore
	}
}

bool fs::cfw::IsAms()
{
	return CfwFolder == path::Atmosphere;
}

bool fs::cfw::IsSX()
{
	return CfwFolder == path::SX;
}

bool fs::cfw::IsRnx()
{
	return CfwFolder == path::Reinx;
}

std::vector<std::string> fs::cfw::SearchFolders()
{
	vector<string> res;
#define CHECKFOLDER(f) \
	if (fs::Exists(f) && std::filesystem::is_directory(f)) res.push_back(f)
	CHECKFOLDER(path::Atmosphere);
	CHECKFOLDER(path::Reinx);
	CHECKFOLDER(path::SX);
#undef CHECKFOLDER
	return res;
}

void fs::cfw::SetFolder(const std::string& s)
{
	// Should probably normalize all path code to use unix style path separators
	CfwFolder = std::strchr("/\\", s[s.size() - 1]) ? s : s + '/';

	bool useContents = false;

	if (cfw::IsAms())
		// Since 0.19.0 ams doesn't come with a contents folder anymore, to simplify the logic support for the titles folder has been dropped.
		useContents = true;
	else if (cfw::IsRnx())
		// Use contents if titles doesn't exist
		useContents = !filesystem::exists(CfwFolder + "titles/");
	else if (cfw::IsSX())
		// Sx still uses titles
		useContents = false;

	TitlesFolder = useContents ? "contents/" : "titles/";
}

std::vector<std::string> fs::patches::GetSdPatches()
{
	std::vector<std::string> result;

	if (!fs::Exists(fs::path::PatchesDir))
		return result;

	for (auto p : filesystem::directory_iterator(fs::path::PatchesDir))
	{
		if (!p.is_regular_file() || p.path().extension() != ".ips")
			continue;
		auto s = p.path().stem();
		if (!s.empty())
			result.push_back(s.string());
	}

	return result;
}

void fs::patches::CreateFolder()
{
	if (Exists(fs::path::PatchesDir) && std::filesystem::is_regular_file(fs::path::PatchesDir))
		Delete(fs::path::PatchesDir);
	CreateDirectory(fs::path::PatchesDir);
}

void fs::patches::WritePatchForBuild(const std::string& buildId, const std::vector<u8>& data)
{
	return WriteFile(fs::path::PatchesDir + buildId + ".ips", data);
}

std::vector<u8> fs::patches::OpenPatchForBuild(const std::string& buildId)
{
	return OpenFile(fs::path::PatchesDir + buildId + ".ips");
}

bool fs::patches::hasPatchForBuild(const std::string& buildId)
{
	return fs::Exists(fs::path::PatchesDir + buildId + ".ips");
}