#include "hactool.hpp"
#include "../Dialogs.hpp"
#include "../Pages/NcaDumpPage.hpp"
#include "key_loader.hpp"

#include <sstream>
#include <cstring>
#include <hactool.h>
#include <vector>
#include <map>

#if __SWITCH__
#include <unistd.h>
#include <sys/stat.h>
#endif

enum class ExtractionTarget
{
	RomFS,
	ExeFS,
};

class HactoolHelper
{
	std::unique_ptr<hactool::ExtractionContext> context;
	std::string fiter = "";
	std::string ncaFile;
	ExtractionTarget target;

public:
	std::map<std::string, std::vector<u8>> ExtractedFiles;

	HactoolHelper(ExtractionTarget t) : 
		context(hactool::Initialize()),
		target(t) 
	{
		UseFilter(nullptr);
	}

	HactoolHelper& ContentID(u64 id)
	{
		return NcaPath(context->getNcaPath(id));
	}

	HactoolHelper& UseFilter(const char* filter)
	{
		if (filter)
			fiter = filter;
		else
			fiter = "";
		return *this;
	}
	
	HactoolHelper& NcaPath(const std::string& str)
	{
		ncaFile = str;
		return *this;
	}

	void Process()
	{
		// Move these to the heap because msvc complains about the stack size of this function
		std::unique_ptr<hactool_ctx_t> tool_ctx = std::make_unique<hactool_ctx_t>();
		std::unique_ptr<hactool_ctx_t> base_ctx = std::make_unique<hactool_ctx_t>();
		std::unique_ptr<nca_ctx_t> nca_ctx = std::make_unique<nca_ctx_t>();

		nca_init(nca_ctx.get());
		memset(tool_ctx.get(), 0, sizeof(tool_ctx));
		memset(base_ctx.get(), 0, sizeof(base_ctx));

		nca_ctx->tool_ctx = tool_ctx.get();
		nca_ctx->is_cli_target = false;

		nca_ctx->tool_ctx->file_type = FILETYPE_NCA;
		base_ctx->file_type = FILETYPE_NCA;

		nca_ctx->tool_ctx->action = ACTION_INFO | ACTION_EXTRACT;
		pki_initialize_keyset(&tool_ctx->settings.keyset, KEYSET_RETAIL);

		nca_ctx->tool_ctx->settings.extraction_file_stream_cb = OnFileDumped;
		nca_ctx->tool_ctx->settings.extra_context = this;

		if (target == ExtractionTarget::ExeFS)
		{
			nca_ctx->tool_ctx->settings.extraction_exefs = true;
		}
		else if (target == ExtractionTarget::RomFS)
		{
			nca_ctx->tool_ctx->settings.extraction_romfs = true;
			nca_ctx->tool_ctx->settings.romfs_filter = FileFilterFunction;
		}
		else
		{
			throw std::runtime_error("Invalid extraction target");
		}

		if ((tool_ctx->file = fopen(ncaFile.c_str(), "rb")) == NULL && tool_ctx->file_type != FILETYPE_BOOT0)
			throw std::runtime_error("Couldn't open " + ncaFile);

		hactool::LoadKeys(&tool_ctx->settings);

		if (nca_ctx->tool_ctx->base_nca_ctx != NULL) {
			memcpy(&base_ctx->settings.keyset, &tool_ctx->settings.keyset, sizeof(nca_keyset_t));
			base_ctx->settings.known_titlekeys = tool_ctx->settings.known_titlekeys;
			nca_ctx->tool_ctx->base_nca_ctx->tool_ctx = base_ctx.get();
			nca_process(nca_ctx->tool_ctx->base_nca_ctx);
			int found_romfs = 0;
			for (unsigned int i = 0; i < 4; i++) {
				if (nca_ctx->tool_ctx->base_nca_ctx->section_contexts[i].is_present && 
					nca_ctx->tool_ctx->base_nca_ctx->section_contexts[i].type == ROMFS) {
					found_romfs = 1;
					break;
				}
			}
			if (found_romfs == 0) {
				throw std::runtime_error("Unable to locate RomFS in base NCA!\n");
			}
		}

		nca_ctx->file = tool_ctx->file;
		nca_process(nca_ctx.get());
		nca_free_section_contexts(nca_ctx.get());

		if (nca_ctx->tool_ctx->base_file_type == BASEFILE_FAKE) {
			nca_ctx->tool_ctx->base_file = NULL;
		}

		if (nca_ctx->tool_ctx->base_file != NULL) {
			fclose(nca_ctx->tool_ctx->base_file);
			if (nca_ctx->tool_ctx->base_file_type == BASEFILE_NCA) {
				nca_free_section_contexts(nca_ctx->tool_ctx->base_nca_ctx);
				free(nca_ctx->tool_ctx->base_nca_ctx);
			}
		}

		if (tool_ctx->settings.known_titlekeys.titlekeys != NULL) {
			free(tool_ctx->settings.known_titlekeys.titlekeys);
		}

		if (tool_ctx->file != NULL) {
			fclose(tool_ctx->file);
		}

		printf("hactool done\n");
	}

private:
	static bool FileFilterFunction(void* context, const char* str)
	{
		HactoolHelper* helper = (HactoolHelper*)context;

		if (helper->fiter.empty())
			return true;

#if !__SWITCH__
		std::string s(str);
		fs::path::ToUnixSeparators(s);
		return s.find(helper->fiter) != std::string::npos;
#endif

		std::string_view strView(str);
		return strView.find(helper->fiter) != std::string_view::npos;
	}

	static void OnFileDumped(void* context, const char* file_name, unsigned char* file_data, size_t length)
	{
		printf("Dumped %s, size: %zu, ptr: %p\n", file_name, length, file_data);

		if (!length || !file_data)
			return;

		HactoolHelper* helper = (HactoolHelper*)context;

		std::string name(file_name);
		fs::path::ToUnixSeparators(name);

		helper->ExtractedFiles[name] = std::vector<u8>(file_data, file_data + length);

		free(file_data);
	}
};

//Don't use this multiple times from the same archive
static void NcaExtractSingleFile(const std::string &file, u64 id, const std::string &targetName)
{	
	auto extractor = HactoolHelper{ ExtractionTarget::RomFS };
	extractor.ContentID(id)
		.UseFilter(file.c_str())
		.Process();

	if (extractor.ExtractedFiles.size() == 0)
		throw std::runtime_error(file + " not found in the target nca !");

	auto& data = extractor.ExtractedFiles.begin()->second;
	fs::WriteFile(fs::path::SystemDataFolder + targetName, data);
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
	
	auto ext = HactoolHelper{ ExtractionTarget::RomFS };
	ext.ContentID(QlaunchID)
		.UseFilter("lyt/")
		.Process();

	if (!ext.ExtractedFiles.count("/lyt/ResidentMenu.szs"))
		throw std::runtime_error("ResidentMenu not found in the extracted NCA content");	

	for (auto& file : ext.ExtractedFiles)
	{
		fs::WriteFile(fs::path::SystemDataFolder + fs::GetFileName(file.first), file.second);
	}
	
	NcaDumpPage::WriteHomeNcaVersion();
}

std::array<u8, 32> hactool::GetTitleBuildID(u64 contentID)
{
	auto extractor = HactoolHelper{ ExtractionTarget::ExeFS };
	extractor.ContentID(QlaunchID)
		.Process();

	if (extractor.ExtractedFiles.size() != 1)
		throw std::runtime_error("Home menu main extraction error");

	auto& data = extractor.ExtractedFiles.begin()->second;

	if (data.size() < 0x40 + 32)
		throw std::runtime_error("File buffer is too small");

	std::array<u8, 32> res;

	// https://switchbrew.org/wiki/NSO
	std::memcpy(res.data(), &data[0x40], 32);

	return res;
}

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