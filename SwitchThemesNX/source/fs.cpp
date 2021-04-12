#include "fs.hpp"
#include <sys/stat.h>
#include <cstring>
#include <sstream>

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

string fs::path::Nca(u64 id)
{
#ifdef __SWITCH__
	char path[FS_MAX_PATH] = { 0 };
	auto rc = lrInitialize();
	if (R_FAILED(rc))
		throw std::runtime_error("lrInitialize : "s + std::to_string(rc));

	LrLocationResolver res;
	rc = lrOpenLocationResolver(NcmStorageId_BuiltInSystem, &res);
	if (R_FAILED(rc))
		throw std::runtime_error("lrOpenLocationResolver :"s + std::to_string(rc));

	rc = lrLrResolveProgramPath(&res, id, path);
	if (R_FAILED(rc))
		throw std::runtime_error("lrLrResolveDataPath : "s + std::to_string(rc));

	std::string result(path);
	result.erase(0, ((std::string)"@SystemContent://").length());
	return (std::string)"System:/Contents/" + result;
#else
	return "";
#endif
}

static string &replaceWindowsPathChar(string& str)
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

static vector<string> GetThemeFilesInDirRecursive(const string &path, int level)
{
	vector<string> res;
	if (level > 7) return res;
	for (auto p : filesystem::directory_iterator(path))
	{
		if (p.is_directory() && p.path().filename() != SYSTEMDATA_DIR && p.path().filename() != "shuffle")
		{
			auto path = p.path().string();
			res.push_back(replaceWindowsPathChar(path));
			auto v = GetThemeFilesInDirRecursive(p.path().string(), level + 1);
			res.insert(res.end(), v.begin(), v.end());
		}
		else if (p.is_regular_file())
		{
			if (StrEndsWith(p.path().string(), ".szs") || StrEndsWith(p.path().string(), ".nxtheme") || StrEndsWith(p.path().string(), ".ttf")) {
				auto str = p.path().string();
				res.push_back(replaceWindowsPathChar(str));
			}
		}
	}
	return res;
}

vector<u8> fs::OpenFile(const string &name)
{
	FILE* f = fopen(name.c_str(),"rb");
	if (!f)
		throw std::runtime_error("Opening file " + name + " failed !\n" FS_TROUBLESHOOT_MSG);

	fseek(f,0,SEEK_END);
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

void fs::WriteFile(const string &name,const vector<u8> &data)
{
	if (filesystem::exists(name))
		remove(name.c_str());
	
	FILE* f = fopen(name.c_str(),"wb");
	if (!f)
		throw std::runtime_error("Saving file " + name + "failed !");
	
	fwrite(data.data(),1,data.size(),f);
	fflush(f);
	fclose(f);
}

std::string fs::SanitizeName(const std::string& name)
{
	const char* forbiddenChars = "/?<>\\:*|\".";

	std::string res = name.length() > 30 ? name.substr(0,30) : name;
	char* c = res.data();
	while (*c)
	{
		if (std::strchr(forbiddenChars, *c))
			*c = '_';
		c++;
	}

	return res;
}

void fs::RecursiveDeleteFolder(const string &path)
{
	if (!filesystem::exists(path)) return;
	vector<string> toDelete;
	for (auto p : filesystem::directory_iterator(path))
	{
		if (p.is_directory())
		{
			toDelete.push_back(p.path().string());
		}
		else if (p.is_regular_file())
		{
			remove(p.path().string().c_str());
		}
	}
	for (auto p : toDelete)
	{
		RecursiveDeleteFolder(p);
		DeleteDirectory(p);
	}
	DeleteDirectory(path);
}

bool fs::EnsureThemesFolderExists()
{
	if (!filesystem::exists(path::ThemesFolder))
		CreateDirectory(path::ThemesFolder);
	bool Result = filesystem::exists(path::SystemDataFolder);
	if (!Result)
		CreateDirectory(path::SystemDataFolder);
	return Result;
}

void fs::EnsureDownloadsFolderExists()
{
	if (!filesystem::exists(path::DownloadsFolder))
		CreateDirectory(path::DownloadsFolder);
}

string fs::GetFileName(const string& path)
{
	return path.substr(path.find_last_of("/\\") + 1);
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

void fs::RemoveSystemDataDir()
{
	RecursiveDeleteFolder(path::SystemDataFolder);
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
	#define DelDirFromCfw(x) if (filesystem::exists(path::FsMitmFolder() + x)) \
		RecursiveDeleteFolder(path::FsMitmFolder() + x);
	
	if (full)
	{
		DelDirFromCfw("0100000000001000")
		DelDirFromCfw("0100000000001013")
	}
	else 
	{
		DelDirFromCfw("0100000000001000/romfs/lyt")
		DelDirFromCfw("0100000000001013/romfs/lyt")
	}
	DelDirFromCfw("0100000000001007") //Player select
	DelDirFromCfw("0100000000000811") //Custom font
	DelDirFromCfw("0100000000000039") //needed to enable custom font
	
	#undef DelDirFromCfw
}

void fs::theme::CreateMitmStructure(const string &id)
{
	string path = path::FsMitmFolder();
	CreateDirectory(path);
	path += id + "/";
	CreateDirectory(path);
	if (!filesystem::exists(path + "fsmitm.flag"))
	{
		vector<u8> t; 
		WriteFile(path + "fsmitm.flag", t);
	}		
}

void fs::theme::CreateRomfsDir(const std::string &id)
{
	CreateDirectory(path::RomfsFolder(id));
}

void fs::theme::CreateStructure(const string &id)
{	
	CreateMitmStructure(id);
	CreateRomfsDir(id);
	mkdir((path::RomfsFolder(id) + "lyt").c_str(), ACCESSPERMS);
}

bool fs::theme::DumpHomeMenuNca()
{	
#ifdef __SWITCH__
	FsFileSystem sys;
    fsOpenBisFileSystem(&sys, FsBisPartitionId_System, "");
	fsdevMountDevice("System", sys);
	try {		
		auto targetNca = path::Nca(0x0100000000001000);
		WriteFile(path::SystemDataFolder + "home.nca",OpenFile(targetNca));
		targetNca = path::Nca(0x0100000000001013);
		WriteFile(path::SystemDataFolder + "user.nca",OpenFile(targetNca));
	}
	catch (...)
	{
		fsdevUnmountDevice("System");
		fsFsClose(&sys);
		return false;
	}
	fsdevUnmountDevice("System");
	fsFsClose(&sys);
	return true;
#else
	return false;
#endif
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