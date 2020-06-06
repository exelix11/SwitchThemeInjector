#include "hactool.hpp"
#include "../ViewFunctions.hpp"
#ifdef __SWITCH__
#include <hactool.h>
#include <unistd.h>
#include <sys/stat.h>
#include "lockpick/KeyCollection.hpp"

using namespace std;
using namespace fs;

static inline void CopyFile(const std::string &Src, const std::string &Dst)
{
	WriteFile(Dst,OpenFile(Src));
}

static void CopyLytDir()
{
	for (auto p : filesystem::directory_iterator("sdmc:/themes/systemData/tmp/lyt/"))
	{		
		if (p.is_regular_file() && StrEndsWith(p.path(), ".szs"))
		{
			string Target = "sdmc:/themes/systemData/" + GetFileName(p.path());
			CopyFile(p.path(), Target);
		}
	}
}

static struct NcaDecryptionkeys
{
	bool Initialized;
	Key header_key, key_area_key_application_source;
} g_Keys = {false};

enum class ExtractionTarget
{
	RomFS,
	ExeFS
};

const char* g_filter = nullptr;
static bool LytFileFilter(const char* str)
{
	if (!g_filter) return true;

	return strstr(str, g_filter);
}

static bool HactoolExtractNCA(const std::string &NcaFile, const std::string &OutDir, ExtractionTarget target)
{    
	if (!g_Keys.Initialized) 
	{
		DialogBlocking("Keys have not been initialized");
		return false;
	}	

    hactool_ctx_t tool_ctx;
    hactool_ctx_t base_ctx; /* Context for base NCA, if used. */
    nca_ctx_t nca_ctx;
    filepath_t keypath;

    nca_init(&nca_ctx);
    memset(&tool_ctx, 0, sizeof(tool_ctx));
    memset(&base_ctx, 0, sizeof(base_ctx));
    filepath_init(&keypath);
    nca_ctx.tool_ctx = &tool_ctx;
    nca_ctx.is_cli_target = false;

    nca_ctx.tool_ctx->file_type = FILETYPE_NCA;
    base_ctx.file_type = FILETYPE_NCA;

    nca_ctx.tool_ctx->action = ACTION_INFO | ACTION_EXTRACT;
    pki_initialize_keyset(&tool_ctx.settings.keyset, KEYSET_RETAIL);
    
	if (target == ExtractionTarget::ExeFS)
	{
		nca_ctx.tool_ctx->settings.exefs_dir_path.enabled = 1;
		filepath_set(&nca_ctx.tool_ctx->settings.exefs_dir_path.path, OutDir.c_str());
	}
	else {
		nca_ctx.tool_ctx->settings.romfs_filter = LytFileFilter;
		nca_ctx.tool_ctx->settings.romfs_dir_path.enabled = 1;
		filepath_set(&nca_ctx.tool_ctx->settings.romfs_dir_path.path, OutDir.c_str());
	}

    if ((tool_ctx.file = fopen(NcaFile.c_str(), "rb")) == NULL && tool_ctx.file_type != FILETYPE_BOOT0) {
        Dialog("Couldn't open " + NcaFile);
        return false;
    }
	
	// Copy keys
	memcpy(tool_ctx.settings.keyset.header_key, g_Keys.header_key.key.data(), 0x20);
	memcpy(tool_ctx.settings.keyset.key_area_key_application_source, g_Keys.key_area_key_application_source.key.data(), 0x10);
	
	if (nca_ctx.tool_ctx->base_nca_ctx != NULL) {
		memcpy(&base_ctx.settings.keyset, &tool_ctx.settings.keyset, sizeof(nca_keyset_t));
		base_ctx.settings.known_titlekeys = tool_ctx.settings.known_titlekeys;
		nca_ctx.tool_ctx->base_nca_ctx->tool_ctx = &base_ctx;
		nca_process(nca_ctx.tool_ctx->base_nca_ctx);
		int found_romfs = 0;
		for (unsigned int i = 0; i < 4; i++) {
			if (nca_ctx.tool_ctx->base_nca_ctx->section_contexts[i].is_present && nca_ctx.tool_ctx->base_nca_ctx->section_contexts[i].type == ROMFS) {
				found_romfs = 1;
				break;
			}
		}
		if (found_romfs == 0) {
			Dialog("Unable to locate RomFS in base NCA!\n");
			return false;
		}
	}

	nca_ctx.file = tool_ctx.file;
	nca_process(&nca_ctx);
	nca_free_section_contexts(&nca_ctx);

	if (nca_ctx.tool_ctx->base_file_type == BASEFILE_FAKE) {
		nca_ctx.tool_ctx->base_file = NULL;
	}

	if (nca_ctx.tool_ctx->base_file != NULL) {
		fclose(nca_ctx.tool_ctx->base_file);
		if (nca_ctx.tool_ctx->base_file_type == BASEFILE_NCA) {
			nca_free_section_contexts(nca_ctx.tool_ctx->base_nca_ctx);
			free(nca_ctx.tool_ctx->base_nca_ctx);
		}
	}
	
    if (tool_ctx.settings.known_titlekeys.titlekeys != NULL) {
        free(tool_ctx.settings.known_titlekeys.titlekeys);
    }

    if (tool_ctx.file != NULL) {
        fclose(tool_ctx.file);
    }

	printf("hactool done\n");

    return true;
}

static bool GetKeys() 
{
	if (g_Keys.Initialized) 
		return true;
	
	if (!(envIsSyscallHinted(0x60) &&     // svcDebugActiveProcess
          envIsSyscallHinted(0x63) &&     // svcGetDebugEvent
          envIsSyscallHinted(0x65) &&     // svcGetProcessList
          envIsSyscallHinted(0x69) &&     // svcQueryDebugProcessMemory
          envIsSyscallHinted(0x6a))) {    // svcReadDebugProcessMemory
            DialogBlocking("Lockpick Error: Please run with debug svc permissions.\nMake sure you're using latest version of your cfw and try launching this app from the album applet");
			return false;
        }
		
	KeyCollection Keys;
    Keys.get_keys();
	
	Key header_key = {"header_key", 0x20, {}};
    if (Keys.header_kek_source.found() && Keys.header_key_source.found()) {
        u8 tempheaderkek[0x10], tempheaderkey[0x20];
        splCryptoGenerateAesKek(Keys.header_kek_source.key.data(), 0, 0, tempheaderkek);
        splCryptoGenerateAesKey(tempheaderkek, Keys.header_key_source.key.data(), tempheaderkey);
        splCryptoGenerateAesKey(tempheaderkek, Keys.header_key_source.key.data() + 0x10, tempheaderkey + 0x10);
		header_key = {"header_key", 0x20, byte_vector(tempheaderkey, tempheaderkey + 0x20)};
    }
	else 
	{
		DialogBlocking("Key extraction from FS failed !");
		return false;
	}
	
	g_Keys = {true, header_key, Keys.key_area_key_application_source};
	
	return true;
}

//Don't use this multiple times from the same archive
static bool NcaExtractSingleFile(const string &file, u64 titleid, const string &targetName)
{
	g_filter = file.c_str();

	RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");
	if (HactoolExtractNCA(GetNcaPath(titleid),"sdmc:/themes/systemData/tmp", ExtractionTarget::RomFS))
	{
		if (!filesystem::exists("sdmc:/themes/systemData/tmp/" + file))
		{			
			Dialog(file + " not found in the target nca !");
			return false;
		}
		CopyFile("sdmc:/themes/systemData/tmp/" + file,"sdmc:/themes/systemData/" + targetName);
		RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");
		return true;
	}
	
	return false;
}

class FsExtractionCtx
{
public:
	FsFileSystem sys;

#define R_THROW(x) if (R_FAILED(x)) { ExitServices(); throw std::runtime_error(#x); }	
	FsExtractionCtx() {
		R_THROW(pmdmntInitialize())
		R_THROW(splInitialize())
		R_THROW(splCryptoInitialize())
		R_THROW(fsOpenBisFileSystem(&sys, FsBisPartitionId_System, ""))
		
		if (fsdevMountDevice("System", sys) == -1)
			throw std::runtime_error("fsdevMountDevice");
	}
#undef R_THROW

	~FsExtractionCtx() noexcept(false) {
		ExitServices();
		fsFsClose(&sys);		
		
		if (fsdevUnmountDevice("System") == -1)
			throw std::runtime_error("fsdevUnmountDevice");
	}
	
	FsExtractionCtx& operator=(const FsExtractionCtx&) = delete;
	FsExtractionCtx& operator=(FsExtractionCtx&&) = delete;
private:
	void ExitServices() 
	{
		pmdmntExit();
		splCryptoExit();
		splExit();
	}
};

bool hactool::ExtractPlayerSelectMenu()
{
	FsExtractionCtx ctx;
	
	if (!GetKeys()) return false;
	
	DisplayLoading("Extracting player select menu...");
	if (!NcaExtractSingleFile("lyt/Psl.szs", 0x0100000000001007, "Psl.szs"))
	{
		DialogBlocking("Couldn't extract player select menu");
		return false;
	}
	return true;
}

bool hactool::ExtractUserPage()
{
	FsExtractionCtx ctx;
	
	if (!GetKeys()) return false;
	
	DisplayLoading("Extracting user page...");
	if (!NcaExtractSingleFile("lyt/MyPage.szs", 0x0100000000001013, "MyPage.szs"))
	{
		DialogBlocking("Couldn't extract user.nca");
		return false;
	}
	return true;
}

bool hactool::ExtractHomeMenu()
{
	FsExtractionCtx ctx;
	
	if (!GetKeys()) return false;
	
	DisplayLoading("Extracting home menu...");
	RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");
	g_filter = "lyt/";
	if (HactoolExtractNCA(GetNcaPath(0x0100000000001000), "sdmc:/themes/systemData/tmp", ExtractionTarget::RomFS))
	{
		if (!filesystem::exists("sdmc:/themes/systemData/tmp/lyt/ResidentMenu.szs"))
		{			
			DialogBlocking("ResidentMenu not found in lyt dir !");
			return false;
		}
		CopyLytDir();
		RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");
	}
	else
	{
		DialogBlocking("Couldn't extract home.nca");
		return false;
	}	
	
	if (!WriteHomeDumpVer())
		DialogBlocking("The home menu was succesfully extracted but version information couldn't be saved, you can ignore this warning.");
	return true;
}

bool hactool::ExtractTitle(u64 titleID, const string& Path) {
	FsExtractionCtx ctx;
	
	if (!GetKeys()) return false;

	DisplayLoading("Extracting ...");
	RecursiveDeleteFolder(Path);
	g_filter = nullptr;
	if (HactoolExtractNCA(GetNcaPath(titleID), Path, ExtractionTarget::RomFS))
		DialogBlocking("Done");
	else
	{
		DialogBlocking("Couldn't extract");
		return false;
	}
	
	return true;
}

bool hactool::ExtractHomeExefs()
{
	FsExtractionCtx ctx;
	
	if (!GetKeys()) return false;
	
	g_filter = nullptr;
	return HactoolExtractNCA(GetNcaPath(0x0100000000001000), "sdmc:/themes/systemData/", ExtractionTarget::ExeFS);
}

#else

bool hactool::ExtractPlayerSelectMenu() { return true; }
bool hactool::ExtractUserPage() { return true; }
bool hactool::ExtractHomeMenu() { return true; }
bool hactool::ExtractTitle(u64 titleID, const std::string& Path) { return true; }

#endif
