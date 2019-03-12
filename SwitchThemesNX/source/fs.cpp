#include "fs.hpp"
#include <unistd.h>
#include <filesystem>
#include "ViewFunctions.hpp"
#include <sys/stat.h>

using namespace std;

string CfwFolder = "";

vector<string> SearchCfwFolders()
{
	vector<string> res;
	DIR * dir = nullptr;
	#define CHECKFOLDER(f) dir = opendir(f); \
	if (dir) { res.push_back(f); closedir(dir); dir = nullptr;}
	CHECKFOLDER("/atmosphere")
	CHECKFOLDER("/reinx")
	CHECKFOLDER("/sxos")
	#undef CHECKFOLDER
	if (res.size() == 1)
		CfwFolder = res[0];
	return res;
}

bool StrEndsWith(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

vector<string> GetThemeFilesInDirRecursive(const string &path, int level)
{
	vector<string> res;
	if (level > 7) return res;
	for (auto p : filesystem::directory_iterator(path))
	{
		if (p.is_directory() && p.path().filename() != "systemData" && p.path().filename() != "shuffle")
		{
			res.push_back(p.path());
			auto v = GetThemeFilesInDirRecursive(p.path(), level + 1);
			res.insert(res.end(), v.begin(), v.end());
		}
		else if (p.is_regular_file())
		{
			if (StrEndsWith(p.path(), ".szs") || StrEndsWith(p.path(), ".nxtheme")  || StrEndsWith(p.path(), ".ttf"))
				res.push_back(p.path());
		}
	}
	return res;
}

vector<string> GetThemeFiles()
{
	vector<string> res;
	
	{	
		DIR *dir = nullptr;
		if (dir = opendir("/themes"))
			closedir(dir);
		else
			return res;
	}
	
	res = GetThemeFilesInDirRecursive("/themes",0);
	
	return res;	
}

vector<u8> OpenFile(const string &name)
{
	FILE* f = fopen(name.c_str(),"rb");
	if (!f){
		ErrorFatal("File open fail");
	}
	fseek(f,0,SEEK_END);
	auto len = ftell(f);
	rewind(f);

	vector<u8> coll(len);
	fread(coll.data(), 1, len, f);
	fclose(f);
	return coll;
}

void WriteFile(const string &name,const vector<u8> &data)
{
	if (filesystem::exists(name))
		remove(name.c_str());
	
	FILE* f = fopen(name.c_str(),"wb");
	if (!f)
	{
		ErrorFatal("Fail save");
	}
	fwrite(data.data(),1,data.size(),f);
	fflush(f);
	fclose(f);
}

void RecursiveDeleteFolder(const string &path)
{
	if (!filesystem::exists(path)) return;
	vector<string> toDelete;
	for (auto p : filesystem::directory_iterator(path))
	{
		if (p.is_directory())
		{
			toDelete.push_back(p.path());
		}
		else if (p.is_regular_file())
		{
			remove(p.path().c_str());
		}
	}
	for (auto p : toDelete)
	{
		RecursiveDeleteFolder(p);
		rmdir(p.c_str());
	}
}

void UninstallTheme(bool full = false)
{
	#define DelDirFromCfw(x) if (filesystem::exists(CfwFolder + x)) \
		RecursiveDeleteFolder(CfwFolder + x);
	
	if (full)
	{
		DelDirFromCfw("/titles/0100000000001000")
		DelDirFromCfw("/titles/0100000000001013")
	}
	else 
	{
		DelDirFromCfw("/titles/0100000000001000/romfs/lyt")
		DelDirFromCfw("/titles/0100000000001013/romfs/lyt")
	}
	DelDirFromCfw("/titles/0100000000001007") //Player select
	DelDirFromCfw("/titles/0100000000000811") //Custom font
	DelDirFromCfw("/titles/0100000000000039") //needed to enable custom font
	
	#undef DelDirFromCfw
}

void CreateFsMitmStructure(const string &tid)
{
	string path = CfwFolder + "/titles";
	mkdir(path.c_str(), ACCESSPERMS);
	path += "/" + tid;
	mkdir(path.c_str(), ACCESSPERMS);
	if (!filesystem::exists(path + "/fsmitm.flag"))
	{
		vector<u8> t; 
		WriteFile(path + "/fsmitm.flag", t);
	}		
}

void CreateRomfsDir(const std::string &tid)
{
	string path = CfwFolder + "/titles/" + tid;
	mkdir((path + "/romfs").c_str(), ACCESSPERMS);
}

void CreateThemeStructure(const string &tid)
{	
	CreateFsMitmStructure(tid);
	CreateRomfsDir(tid);
	mkdir((CfwFolder + "/titles/" + tid + "/romfs/lyt").c_str(), ACCESSPERMS);
}

bool CheckThemesFolder()
{
	if (!filesystem::exists("/themes"))
		mkdir("/themes", ACCESSPERMS);
	bool Result = filesystem::exists("/themes/systemData");
	if (!Result)
		mkdir("/themes/systemData", ACCESSPERMS);
	return Result;
}

string GetFileName(const string &path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}

string GetPath(const string &path)
{
	return path.substr(0, path.find_last_of("/\\") + 1);
}

string GetParentDir(const string &path)
{
	string _path = path;
	if (StrEndsWith(_path,"/"))
		_path = _path.substr(0,_path.length() - 1);
	
	return _path.substr(0, _path.find_last_of("/\\") + 1);
}

std::string GetNcaPath(u64 tid)
{
	char path[FS_MAX_PATH] = {0};
	auto rc = lrInitialize();		
	if (R_FAILED(rc))
		ErrorFatal((string)"lrInitialize : " + to_string(rc));
	
	LrLocationResolver res;
	rc = lrOpenLocationResolver(FsStorageId_NandSystem ,&res);
	if (R_FAILED(rc))
		ErrorFatal((string)"lrOpenLocationResolver :" + to_string(rc));
	
	rc = lrLrResolveProgramPath(&res, tid, path);
	if (R_FAILED(rc))
		ErrorFatal((string)"lrLrResolveDataPath : "+ to_string(rc));
	
	std::string result(path);
	result.erase(0, ((std::string)"@SystemContent://").length());
	return (std::string)"System:/Contents/" + result;
}

bool DumpHomeMenuNca()
{	
	FsFileSystem sys;
    fsOpenBisFileSystem(&sys, 31, "");
	fsdevMountDevice("System", sys);
	try {		
		auto targetNca = GetNcaPath(0x0100000000001000);
		WriteFile("sdmc:/themes/systemData/home.nca",OpenFile(targetNca));
		targetNca = GetNcaPath(0x0100000000001013);
		WriteFile("sdmc:/themes/systemData/user.nca",OpenFile(targetNca));
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
}

void RemoveSystemDataDir()
{
	RecursiveDeleteFolder("sdmc:/themes/systemData/");
	mkdir("sdmc:/themes/systemData/",ACCESSPERMS);
}

int GetShuffleCount()
{
	if (!filesystem::exists("sdmc:/themes/shuffle"))
		return 0;
	int dirCount = 0;
	for (auto p : filesystem::directory_iterator("sdmc:/themes/shuffle"))
	{
		if (p.is_directory())
			dirCount++;
	}
	return dirCount;
}

string MakeThemeShuffleDir()
{
	if (!filesystem::exists("sdmc:/themes/shuffle"))
		mkdir("sdmc:/themes/shuffle", ACCESSPERMS);
	int dirCount = GetShuffleCount();
	string dirName = "sdmc:/themes/shuffle/set" + to_string(dirCount) + "/";
	while (filesystem::exists(dirName))
	{
		dirName = "sdmc:/themes/shuffle/set" + to_string(++dirCount) + "/";
	}
	mkdir(dirName.c_str(), ACCESSPERMS);
	return dirName;
}

void ClearThemeShuffle()
{
	if (!filesystem::exists("sdmc:/themes/shuffle"))
		return;
	RecursiveDeleteFolder("sdmc:/themes/shuffle/");
	rmdir("sdmc:/themes/shuffle/");	
}

bool WriteHomeDumpVer()
{
	FILE *ver = fopen("/themes/systemData/ver.cfg", "w");
	if (!ver)
		return false;
	fprintf(ver,"%s",SystemVer.c_str());
	fclose(ver);
	return true;
}

