#pragma once
#include "../../SwitchThemesCommon/MyTypes.h"
#include "../../UI/UI.hpp"
#include "BaseImageOptionsDialog.hpp"

#include <string>
#include <span>

class InstallImageDialog : public BaseImageOptionsDialog
{
public:
	InstallImageDialog(ImageRef preview,
		const std::vector<u8>& imageBytes,
		bool resizeWarning,
		bool showInstallDialogs,
		bool* outSuccess,
		std::span<const u8> bootImageBytes = {},
		bool showBootloaderSuccessDialog = true,
		std::string* outInstallWarning = nullptr);

	void Update() override {};
protected:
	void RenderTop() override;
	void RenderLeftPanel(float allowedWidth) override;
	void RenderRightPanel(float x, float allowedWidth, float endY) override;
	void RenderBottom() override;

private:
	ImageRef previewImage;
	ImageRef previewOverlay;
	std::string currentPreviewOverlay;

	std::span<const u8> imageBytes;
	std::span<const u8> bootImageBytes;
	bool resizeWarning;
	bool showInstallDialogs;
	bool showBootloaderSuccessDialog;
	bool* outSuccess;
	std::string* outInstallWarning;

	bool previewLoadFailure = false;
	std::string previewError = "";

	void ApplyToPart(const std::string& part);
	void ApplyToBootloader();
	ImageRef LoadOverlayPart(const std::string& part);
};
