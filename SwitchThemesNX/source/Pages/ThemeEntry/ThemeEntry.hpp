#pragma once
#include <optional>
#include <memory>
#include <vector>
#include "../../UI/UI.hpp"
#include "../../SwitchThemesCommon/MyTypes.h"
#include "../../SwitchThemesCommon/NXTheme.hpp"
#include "../../SwitchThemesCommon/Common.hpp"
#include "../../SwitchThemesCommon/Patcher.hpp"
#include "../../SwitchThemesCommon/SarcLib/Sarc.hpp"

class ThemeEntry 
{
	public:
		static void DisplayInstallDialog(const std::string& path);

		enum class UserAction 
		{
			None,
			Enter,
			Options
		};

		static std::unique_ptr<ThemeEntry> FromFile(const std::string& fileName);
		static std::unique_ptr<ThemeEntry> FromMemory(const std::vector<u8>& RawData);

		virtual ~ThemeEntry();
		
		static constexpr int EntryW = 860;

		virtual bool IsFolder() = 0;
		virtual bool CanInstall() = 0;		
		virtual bool HasOptions() { return false; }
		virtual void OpenOptions() { }
		
		bool Install(bool ShowDialogs = true);

		bool IsHighlighted();
		std::string GetPath() {return FileName;}
		
		virtual UserAction Render(bool OverrideColor = false);

		std::string InstallLog;
		std::string CannotInstallReason;
	protected:
		virtual bool DoInstall(bool ShowDialogs = true) = 0;

		void AppendInstallMessage(const std::string& msg)
		{
			if (InstallLog.empty())
				InstallLog = msg;
			else
				InstallLog += "\n" + msg;
		}

		std::vector<u8> file;		
		
		std::string FileName;
		std::string lblFname;
		std::string lblLine1;
		std::string lblLine2;

		//Used to return by reference for the background image
		const static std::vector<u8> _emtptyVec;
};

class NxEntry : public ThemeEntry
{
public:
	NxEntry(const std::string& fileName, std::vector<u8>&& RawData);
	NxEntry(const std::string& fileName, FileContainer&& container);

	bool IsFolder() override { return false; }
	bool CanInstall() override { return _CanInstall; }
	bool HasOptions() override { return true; }

protected:
	bool DoInstall(bool ShowDialogs = true) override;
	void OpenOptions() override;

private:
	bool _CanInstall = true;
	bool _HasPreview = false;
	NxTheme theme;
	const ThemeTargetInfo* TargetInfo = nullptr;

	void Initialize();
	std::optional<FileData> GetBackgroundImage();
	bool PatchLayout(SwitchThemesCommon::SzsPatcher& patcher, std::string_view JSON, const std::string& PartName);
};

class LegacyEntry : public ThemeEntry
{
public:
	LegacyEntry(const std::string& fileName, std::vector<u8>&& RawData);
	LegacyEntry(const std::string& fileName, SARC::SarcData&& _SData);

	bool IsFolder() override { return false; }
	bool CanInstall() override { return _CanInstall; }
protected:
	bool DoInstall(bool ShowDialogs = true) override;

private:
	bool _CanInstall = true;
	SARC::SarcData SData;

	void ParseLegacyTheme(SARC::SarcData&& _Sdata);
};

class FontEntry : public ThemeEntry
{
public:
	FontEntry(const std::string& fileName, std::vector<u8>&& RawData);

	bool IsFolder() override { return false; }
	bool CanInstall() override { return _CanInstall; }
protected:
	bool DoInstall(bool ShowDialogs = true) override;

private:
	bool _CanInstall = true;
	void ParseFont();
};

class ImageEntry : public ThemeEntry
{
public:
	ImageEntry(const std::string& fileName, std::vector<u8>&& RawData);

	bool IsFolder() override { return false; }
	bool CanInstall() override { return CannotInstallReason.empty(); }
protected:
	bool DoInstall(bool ShowDialogs = true) override;

private:
	ImageRef previewImage = nullptr;
	std::vector<u8> imageData {};
	bool resizeWarning = false;
	bool conversionDone = false;

	// Lazy conversion, only when needed for preview or installation, and the result is cached
	void PerformConversion();
	ImageRef GetConvertedImage();
};