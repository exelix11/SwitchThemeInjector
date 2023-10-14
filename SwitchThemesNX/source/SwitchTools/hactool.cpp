#include "hactool.hpp"
#include "../Dialogs.hpp"
#include "../Pages/NcaDumpPage.hpp"

#include <sstream>
#ifdef __SWITCH__
#include <cstring>
#include <hactool.h>
#include <unistd.h>
#include <sys/stat.h>

static void CopyLytDir()
{
	for (auto p : std::filesystem::directory_iterator("sdmc:/themes/systemData/tmp/lyt/"))
	{	
		if (p.is_regular_file() && p.path().extension() == ".szs")
		{
			std::filesystem::rename(p.path(), std::filesystem::path(fs::path::SystemDataFolder) / p.path().filename());
		}
	}
}

static struct NcaDecryptionkeys
{
	bool Initialized;
	// key sources, hardcoded as they are already public.
	u8 header_key_source[0x20];
	u8 header_kek_source[0x10];
	u8 key_area_key_application_source[0x10];
	// acutal keys, derived on hardware using spl services.
	u8 header_key[0x20];
} g_Keys = {
	.Initialized = false,
	// from https://github.com/Atmosphere-NX/Atmosphere/blob/693fb423cbbd5ab28963c5157a6f46d1aae838cf/libraries/libstratosphere/source/fssrv/fssrv_nca_crypto_configuration.cpp#L120
	.header_key_source = {
		0x5A, 0x3E, 0xD8, 0x4F, 0xDE, 0xC0, 0xD8, 0x26, 0x31, 0xF7, 0xE2, 0x5D, 0x19, 0x7B, 0xF5, 0xD0,
		0x1C, 0x9B, 0x7B, 0xFA, 0xF6, 0x28, 0x18, 0x3D, 0x71, 0xF6, 0x4D, 0x73, 0xF1, 0x50, 0xB9, 0xD2
	},
	.header_kek_source = {
		0x1F, 0x12, 0x91, 0x3A, 0x4A, 0xCB, 0xF0, 0x0D, 0x4C, 0xDE, 0x3A, 0xF6, 0xD5, 0x23, 0x88, 0x2A
	},
	.key_area_key_application_source = {
		0x7F, 0x59, 0x97, 0x1E, 0x62, 0x9F, 0x36, 0xA1, 0x30, 0x98, 0x06, 0x6F, 0x21, 0x44, 0xC3, 0x0D
	}
};

enum class ExtractionTarget
{
	RomFS,
	ExeFS,
	MainBuffer
};

class FsExtractionCtx
{
public:
	FsExtractionCtx& operator=(const FsExtractionCtx&) = delete;
	FsExtractionCtx& operator=(FsExtractionCtx&&) = delete;
#ifdef __SWITCH__
	FsFileSystem sys;

#define R_THROW(x) do {				\
		Result rc = x;				\
		if (R_FAILED(rc)) {			\
			ExitServices();			\
			throw std::runtime_error(std::string("Error: ") + #x + ": " + std::to_string(rc));	\
		}	\
	} while (0)

	FsExtractionCtx() {
		R_THROW(pmdmntInitialize());
		R_THROW(splInitialize());
		R_THROW(splCryptoInitialize());
		R_THROW(fsOpenBisFileSystem(&sys, FsBisPartitionId_System, ""));

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
private:
	void ExitServices()
	{
		pmdmntExit();
		splCryptoExit();
		splExit();
	}
#endif
};

class HactoolHelper
{
public:
	FsExtractionCtx fsAccess;

	HactoolHelper(ExtractionTarget t) : target(t) 
	{
		if (!GetKeys())
			throw std::runtime_error("Keys have not been initialized");

		UseFilter(nullptr);
	}

	HactoolHelper& ContentID(u64 id)
	{
		return NcaPath(fs::path::Nca(id));
	}

	HactoolHelper& UseFilter(const char* filter)
	{
		g_filter = filter;
		return *this;
	}
	
	HactoolHelper& NcaPath(const std::string& str)
	{
		_NcaFile = str;
		return *this;
	}

	HactoolHelper& OutDir(const std::string& str)
	{
		if (!UsesPath())
			throw std::runtime_error("Expected a buffer pointer as output");

		_OutDir = str;
		return *this;
	}

	HactoolHelper& OutBuf(file_output_t* ptr)
	{
		if (UsesPath())
			throw std::runtime_error("Expected a path as output");

		_OutBuf = ptr;
		return *this;
	}

	void Process() 
	{
		if (UsesPath())
		{
			if (_OutDir == "")
				throw std::runtime_error("Output path was not set");
		}
		else 
		{
			if (!_OutBuf)
				throw std::runtime_error("Output buffer was not set");
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
			filepath_set(&nca_ctx.tool_ctx->settings.exefs_dir_path.path, _OutDir.c_str());
		}
		else if (target == ExtractionTarget::RomFS)
		{
			if (_OutDir == "")
				throw std::runtime_error("Output path was not set");

			nca_ctx.tool_ctx->settings.romfs_filter = FileFilterFunction;
			nca_ctx.tool_ctx->settings.romfs_dir_path.enabled = 1;
			filepath_set(&nca_ctx.tool_ctx->settings.romfs_dir_path.path, _OutDir.c_str());
		}
		else //MainBuffer
		{
			nca_ctx.tool_ctx->settings.exefs_main_out = _OutBuf;
		}

		if ((tool_ctx.file = fopen(_NcaFile.c_str(), "rb")) == NULL && tool_ctx.file_type != FILETYPE_BOOT0)
			throw std::runtime_error("Couldn't open " + _NcaFile);

		// Copy keys
		memcpy(tool_ctx.settings.keyset.header_key, g_Keys.header_key, sizeof(g_Keys.header_key));
		memcpy(tool_ctx.settings.keyset.key_area_key_application_source, g_Keys.key_area_key_application_source, sizeof(g_Keys.key_area_key_application_source));

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
				throw std::runtime_error("Unable to locate RomFS in base NCA!\n");
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
	}

private:
	std::string _NcaFile;
	std::string _OutDir;
	file_output_t* _OutBuf;
	ExtractionTarget target;

	bool UsesPath() 
	{
		return target != ExtractionTarget::MainBuffer;
	}

	static bool GetKeys()
	{
#ifdef __SWITCH__
		if (g_Keys.Initialized)
			return true;

		u8 tempheaderkek[0x10];
		Result rc = splCryptoGenerateAesKek(g_Keys.header_kek_source, 0, 0, tempheaderkek);
		if (R_FAILED(rc))
		{
			printf("splCryptoGenerateAesKek failed: %x\n", rc);
		}
		else {		
			rc = splCryptoGenerateAesKey(tempheaderkek, g_Keys.header_key_source, g_Keys.header_key);
			if (R_FAILED(rc))		
			{
				printf("splCryptoGenerateAesKey (1) failed: %x\n", rc);
			}
			else 
			{
				rc = splCryptoGenerateAesKey(tempheaderkek, g_Keys.header_key_source + 0x10, g_Keys.header_key + 0x10);
			
				if (R_FAILED(rc))
					printf("splCryptoGenerateAesKey (2) failed: %x\n", rc);
			}
		}

		if (R_FAILED(rc))
		{
			DialogBlocking("Key extraction from FS failed !");
			return false;
		}

		g_Keys.Initialized = true;		
#endif
		return true;
	}

	static const char* g_filter;
	static bool FileFilterFunction(const char* str)
	{
		if (!g_filter) return true;
		return strstr(str, g_filter);
	}
};

const char* HactoolHelper::g_filter = nullptr;

//Don't use this multiple times from the same archive
static void NcaExtractSingleFile(const std::string &file, u64 id, const std::string &targetName)
{	
	fs::RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");

	HactoolHelper{ ExtractionTarget::RomFS }
		.ContentID(id)
		.OutDir("sdmc:/themes/systemData/tmp")
		.UseFilter(file.c_str())
		.Process();

	if (!std::filesystem::exists("sdmc:/themes/systemData/tmp/" + file))
		throw std::runtime_error(file + " not found in the target nca !");

	std::filesystem::rename("sdmc:/themes/systemData/tmp/" + file, std::filesystem::path(fs::path::SystemDataFolder) / targetName);
	fs::RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");	
}

void hactool::ExtractPlayerSelectMenu()
{
	DisplayLoading("Extracting player select menu...");
	NcaExtractSingleFile("lyt/Psl.szs", PslID, "Psl.szs");
}

void hactool::ExtractUserPage()
{
	DisplayLoading("Extracting user page...");
	NcaExtractSingleFile("lyt/MyPage.szs", UserPageID, "MyPage.szs");
}

void hactool::ExtractHomeMenu()
{	
	DisplayLoading("Extracting home menu...");
	fs::RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");
	
	HactoolHelper{ ExtractionTarget::RomFS }
		.ContentID(QlaunchID)
		.OutDir("sdmc:/themes/systemData/tmp")
		.UseFilter("lyt/")
		.Process();

	if (!std::filesystem::exists("sdmc:/themes/systemData/tmp/lyt/ResidentMenu.szs"))
		throw std::runtime_error("ResidentMenu not found in lyt dir !");
	
	CopyLytDir();
	fs::RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");
	
	NcaDumpPage::WriteHomeNcaVersion();
}

void hactool::ExtractTitle(u64 ContentID, const std::string& Path) {
	DisplayLoading("Extracting ...");
	fs::RecursiveDeleteFolder(Path);

	HactoolHelper{ ExtractionTarget::RomFS }
		.ContentID(ContentID)
		.OutDir(Path)
		.Process();

	DialogBlocking("Done");
}

void hactool::ExtractHomeExefs()
{
	HactoolHelper{ ExtractionTarget::ExeFS }
		.ContentID(QlaunchID)
		.OutDir(fs::path::SystemDataFolder)
		.Process();
}

std::array<u8, 32> hactool::GetTitleBuildID(u64 contentID)
{
	file_output_t output = {};

	HactoolHelper{ ExtractionTarget::MainBuffer }
		.ContentID(QlaunchID)
		.OutBuf(&output)
		.Process();

	if (!output.size || !output.data)
		throw std::runtime_error("Home menu main extraction error");

	if (output.size < 0x40 + 32)
	{
		free(output.data);
		throw std::runtime_error("File buffer is too small");
	}

	std::array<u8, 32> res;

	// https://switchbrew.org/wiki/NSO
	std::memcpy(res.data(), output.data + 0x40, 32);
	
	// This was allocated from hactool with malloc
	free(output.data);

	return res;
}
#else
void hactool::ExtractPlayerSelectMenu() {  }
void hactool::ExtractUserPage() {}
void hactool::ExtractHomeMenu() {}
void hactool::ExtractTitle(u64 contentID, const std::string& Path) {}
void hactool::ExtractHomeExefs() {}
std::array<u8, 32> hactool::GetTitleBuildID(u64 contentID) { return std::array<u8, 32>{}; }
#endif

std::string hactool::BuildIDToString(std::array<u8, 32> data)
{
	std::stringstream ss;

	for (u8 b : data)
		ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << (int)b;

	return ss.str();
}

std::string hactool::QlaunchBuildID()
{
	return BuildIDToString(GetTitleBuildID(QlaunchID));
}