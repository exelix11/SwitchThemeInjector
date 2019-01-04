#include "hactool.hpp"
#include "../ViewFunctions.hpp"
extern "C"
{
#include "../../Libs/include/hactool/types.h"
#include "../../Libs/include/hactool/utils.h"
#include "../../Libs/include/hactool/settings.h"
#include "../../Libs/include/hactool/pki.h"
#include "../../Libs/include/hactool/nca.h"
#include "../../Libs/include/hactool/xci.h"
#include "../../Libs/include/hactool/nax0.h"
#include "../../Libs/include/hactool/extkeys.h"
#include "../../Libs/include/hactool/packages.h"
#include "../../Libs/include/hactool/nso.h"
}
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

inline void CopyFile(const std::string &Src, const std::string &Dst)
{
	WriteFile(Dst,OpenFile(Src));
}

void CopyLytDir()
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

bool ExtractNca(const std::string &NcaFile, const std::string &OutDir, const std::string &KeyFile)
{    
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
    nca_ctx.tool_ctx->action = ACTION_EXTRACT;
    pki_initialize_keyset(&tool_ctx.settings.keyset, KEYSET_RETAIL);
    
    nca_ctx.tool_ctx->settings.romfs_dir_path.enabled = 1;
    filepath_set(&nca_ctx.tool_ctx->settings.romfs_dir_path.path, OutDir.c_str());		
 
    filepath_set(&keypath, KeyFile.c_str());
    FILE *keyfile = NULL;
    if(keypath.valid == VALIDITY_VALID) keyfile = os_fopen(keypath.os_path, OS_MODE_READ);

    if(keyfile != NULL)
    {
        extkeys_initialize_keyset(&tool_ctx.settings.keyset, keyfile);
        if (tool_ctx.settings.has_sdseed) {
            for (unsigned int key = 0; key < 2; key++) {
                for (unsigned int i = 0; i < 0x20; i++) {
                    tool_ctx.settings.keyset.sd_card_key_sources[key][i] ^= tool_ctx.settings.sdseed[i & 0xF];
                }
            }
        }
        pki_derive_keys(&tool_ctx.settings.keyset);
        fclose(keyfile);
    }

    if ((tool_ctx.file = fopen(NcaFile.c_str(), "rb")) == NULL && tool_ctx.file_type != FILETYPE_BOOT0) {
        Dialog("Couldn't open " + NcaFile);
        return false;
    }
    
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

    return true;
}

std::string GetNcaPath(u64 tid);


bool ExtractHomeMenu()
{
	#define UnmountSys {fsdevUnmountDevice("System"); fsFsClose(&sys);}
	DisplayLoading("Extracting home...");
	FsFileSystem sys;
    fsOpenBisFileSystem(&sys, 31, "");
	fsdevMountDevice("System", sys);
	RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");
	if (ExtractNca(GetNcaPath(0x0100000000001000),"sdmc:/themes/systemData/tmp",FindKeyFile()))
	{
		if (!filesystem::exists("sdmc:/themes/systemData/tmp/lyt/ResidentMenu.szs"))
		{
			UnmountSys
			Dialog("ResidentMenu not found in lyt dir !");
			return false;
		}
		CopyLytDir();
		RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");
	}
	else
	{
		UnmountSys
		Dialog("Couldn't extract home.nca");
		return false;
	}
	DisplayLoading("Extracting User...");
	if (ExtractNca(GetNcaPath(0x0100000000001013),"sdmc:/themes/systemData/tmp",FindKeyFile()))
	{
		if (!filesystem::exists("sdmc:/themes/systemData/tmp/lyt/MyPage.szs"))
		{
			UnmountSys
			Dialog("MyPage not found in lyt dir !");
			return false;
		}
		CopyFile("sdmc:/themes/systemData/tmp/lyt/MyPage.szs","sdmc:/themes/systemData/MyPage.szs");
		RecursiveDeleteFolder("sdmc:/themes/systemData/tmp");
		rmdir("sdmc:/themes/systemData/tmp");
	}
	else
	{
		UnmountSys
		Dialog("Couldn't extract user.nca");
		return false;
	}
	UnmountSys
	return true;
	#undef UnmountSys
}
