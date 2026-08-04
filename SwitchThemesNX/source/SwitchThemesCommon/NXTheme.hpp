#pragma once
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <variant>
#include <span>
#include <string_view>

#include "MyTypes.h"
#include "Common.hpp"

using FileResult = std::variant<FileData, std::string>;
using ContainerResult = std::variant<FileContainer, std::string>;

namespace zip
{
	bool IsZip(std::span<const u8> data);
	ContainerResult Extract(std::span<const u8> data);
}

namespace szs
{
	ContainerResult Extract(const std::vector<u8>& data);
}

struct ThemeFileManifest
{
	int Version;
	std::string Author;
	std::string ThemeName;
	std::string LayoutInfo;
	std::string Target;

	static ThemeFileManifest FromJson(std::string_view json);
	static FileData ForInternalUse(const std::string& target);
};

class NxTheme 
{
private:
	void initialize();

public:
	FileContainer files;
	std::optional<std::string> error;
	std::optional<ThemeFileManifest> manifest;

	bool IsValid() const { return !error.has_value() && manifest.has_value(); }
	
	NxTheme(FileContainer&& f) : files(std::move(f)) { initialize(); }
	NxTheme(const FileContainer& f) : files(f) { initialize(); }

	// All images are always returned in DDS format
	static FileResult ConvertToDDS(const FileData& image, bool transparent, int width, int height);

	bool HasMainImage() const { return files.count("image.dds") || files.count("image.jpg"); }
	FileResult GetConvertedMainImage() const;
	FileResult GetRawMainImage() const;
	
	FileData GetRawManifest() const;

	bool HasMainLayout() const { return files.count("layout.json"); }
	FileData GetRawMainLayout() const;
	std::string_view GetMainLayout() const;
	
	bool HasCommonLayout() const { return files.count("common.json"); }
	FileData GetRawCommonLayout() const;
	std::string_view GetCommonLayout() const;
		
	bool HasImagePart(std::string_view partName) const;
	FileResult GetImagePart(std::string_view partName, int width, int height) const;

	static NxTheme FromError(std::string message);
	static NxTheme TryLoad(const std::vector<u8>& data);
};