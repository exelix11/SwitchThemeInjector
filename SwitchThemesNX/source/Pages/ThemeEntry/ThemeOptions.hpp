#pragma once
#include <string_view>
#include <string>
#include <vector>

#include "BaseImageOptionsDialog.hpp"
#include "../../SwitchThemesCommon/NXTheme.hpp"
#include "../../SwitchThemesCommon/MyTypes.h"

class ThemeOptionsDialog : public BaseImageOptionsDialog
{
public:
	ThemeOptionsDialog(std::string_view name, NxTheme& theme) :
		name(name), theme(theme) {
		
		Initialize();
	}

	void Update() override {};

protected:
	void RenderTop() override;
	void RenderLeftPanel(float allowedWidth) override;
	void RenderRightPanel(float x, float allowedWidth, float endY) override;
	void RenderBottom() override;

private:
	std::string name;
	NxTheme& theme;
	std::string previewError;
	std::vector<u8> rawImage;
	ImageRef previewImage;

	void Initialize();
	void Preview();
	void InstallOnlyImage();
	void InstallOnlyLayout();
	void ExtractToSD();
};