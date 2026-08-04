#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <span>
#include <utility>
#include <string_view>
#include <format>

#include "MyTypes.h"
#include "NXTheme.hpp"
#include "json.hpp"
#include "SarcLib/Sarc.hpp"
#include "SarcLib/Yaz0.hpp"
#include "Bntx/ImageConversion.hpp"
#include "../../Libs/zip/zip.h"
#include <exception>
#include "Common.hpp"

namespace
{
	std::string_view GetStringView(const std::vector<u8>& vec)
	{
		return std::string_view(reinterpret_cast<const char*>(vec.data()), vec.size());
	}
}

bool zip::IsZip(std::span<const u8> data)
{
	return data.size() >= 4 && data[0] == 'P' && data[1] == 'K';
}

ContainerResult zip::Extract(std::span<const u8> data)
{
	zip_t* zip = zip_stream_open((const char*)data.data(), data.size(), 0, 'r');
	if (!zip)
		return "Failed to open zip archive";

	FileContainer res = {};

	int entries = zip_entries_total(zip);
	for (int i = 0; i < entries; ++i) {
		zip_entry_openbyindex(zip, i);
		if (!zip_entry_isdir(zip))
		{
			const char* name = zip_entry_name(zip);
			unsigned long long size = zip_entry_size(zip);
			
			auto vec = std::vector<u8>(size);
			if (zip_entry_noallocread(zip, vec.data(), vec.size()) < 0)
			{
				std::string error = "Failed to extract archive item: " + std::string(name);
				zip_entry_close(zip);
				zip_stream_close(zip);
				
				return error;
			}

			res[name] = std::move(vec);
		}
		zip_entry_close(zip);
	}

	zip_stream_close(zip);
	return res;
}

ContainerResult szs::Extract(const std::vector<u8>& data)
{
	if (data.size() < 4) return "The provided file is too short";
	std::vector<u8> decompressed;

	// Is this Yaz0 compressed?
	if (Yaz0::IsYaz0(data))
	{
		try
		{
			decompressed = Yaz0::Decompress(std::vector<u8>(data.begin(), data.end()));
			return szs::Extract(decompressed);
		}
		catch (const std::exception& e)
		{
			return std::string("Failed to decompress Yaz0 data: ") + e.what();
		}
	}

	try 
	{
		auto unpacked = SARC::Unpack(data);
		return unpacked.files;
	}
	catch (const std::exception& e)
	{
		return std::string("Failed to unpack SARC data: ") + e.what();
	}
}

NxTheme NxTheme::FromError(std::string message)
{
	auto res = NxTheme({});
	res.error = std::move(message);
	return res;
}

NxTheme NxTheme::TryLoad(const std::vector<u8>& data)
{
	if (data.size() < 4)
		return NxTheme::FromError("The provided file is too small");

	// New nxtheme format: zip archive	
	auto extracted = zip::IsZip(data) ?
		zip::Extract(data) : 
		szs::Extract(data); // Legacy nxtheme format: szs archive
	
	if (std::holds_alternative<std::string>(extracted)) 
		return NxTheme::FromError(std::get<std::string>(extracted));

	return NxTheme(std::move(std::get<FileContainer>(extracted)));
}

void NxTheme::initialize() 
{
	try 
	{
		if (!files.count("info.json"))
		{
			error = "This theme does not contain the info.json manifest file.";
			return;
		}

		const auto& manifestData = files["info.json"];
		
		// ASCII is the only relevant encoding anyway...
		std::string manifestStr(
			reinterpret_cast<const char*>(manifestData.data()),
			manifestData.size());

		manifest = ThemeFileManifest::FromJson(manifestStr);

		if (manifest->Version <= 0)
		{
			error = "Parsing the metadata of this theme failed.";
			return;
		}
	}
	catch (const std::exception& e)
	{
		error = std::string("Error while loading theme data: ") + e.what();
		return;
	}
}

bool NxTheme::HasImagePart(std::string_view partName) const 
{
	if (files.count(std::string(partName) + ".dds")) return true;
	if (files.count(std::string(partName) + ".png")) return true;
	return false;
}

FileResult NxTheme::ConvertToDDS(const FileData& image, bool transparent, int width, int height)
{
	try
	{
#ifndef SWITCHTHEMESCOMMON_TESTS
		auto dds = ImageConversion::ToDDS(image, transparent, width, height);
		if (dds.IsSuccess())
			return dds.Data;

		return dds.ErrorMessage;
#else
		return "Not implemented";
#endif
	}
	catch (const std::exception& e)
	{
		return std::string("Error while converting image: ") + e.what();
	}
}

FileData NxTheme::GetRawManifest() const
{
	return files.at("info.json");
}

FileResult NxTheme::GetConvertedMainImage() const
{
	if (files.count("image.dds")) return files.at("image.dds");
	if (!files.count("image.jpg")) return "This theme does not have an image";		
	return ConvertToDDS(files.at("image.jpg"), false, 1280, 720);
}

FileResult NxTheme::GetRawMainImage() const
{
	if (files.count("image.dds")) return files.at("image.dds");
	if (!files.count("image.jpg")) return "This theme does not have an image";		
	return files.at("image.jpg");
}

FileResult NxTheme::GetImagePart(std::string_view partName, int width, int height) const
{
	auto name = std::string(partName) + ".dds";
	if (files.count(name)) return files.at(name);
	
	name = std::string(partName) + ".png";
	if (!files.count(name)) return "Part name not found";

	const auto& data = files.at(name);

	return ConvertToDDS(data, true, width, height);
}

FileData NxTheme::GetRawMainLayout() const
{
	return files.at("layout.json");
}

std::string_view NxTheme::GetMainLayout() const
{
	if (!HasMainLayout()) return "";
	return GetStringView(files.at("layout.json"));
}

FileData NxTheme::GetRawCommonLayout() const
{
	return files.at("common.json");
}

std::string_view NxTheme::GetCommonLayout() const
{
	if (!HasCommonLayout()) return "";
	return GetStringView(files.at("common.json"));
}

FileData ThemeFileManifest::ForInternalUse(const std::string& target)
{
	std::string msg = std::format(
		R"({{
				"Target": "{}",
				"ThemeName": "New Theme",
				"Version": {}
			}})",
		target, SwitchThemesCommon::NXThemeVer);

	return FileData(msg.begin(), msg.end());
}

ThemeFileManifest ThemeFileManifest::FromJson(std::string_view json)
{
	auto j = nlohmann::json::parse(json);
	
	ThemeFileManifest res = {0};
	if (j.count("Version") && j.count("Target"))
	{
		res.Version = j["Version"].get<int>();
		res.Target = j["Target"].get<std::string>();
	}
	else 
	{
		res.Version = -1;
		return res;
	}

	if (j.count("Author"))
		res.Author = j["Author"].get<std::string>();

	if (j.count("ThemeName"))
		res.ThemeName = j["ThemeName"].get<std::string>();

	if (j.count("LayoutInfo"))
		res.LayoutInfo = j["LayoutInfo"].get<std::string>();
	
	return res;
}