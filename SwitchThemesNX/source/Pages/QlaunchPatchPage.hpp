#pragma once
#include "../UI/UI.hpp"
#include <map>
#include "../SwitchTools/PatchMng.hpp"

class QlaunchPatchPage : public IPage
{
public:
	QlaunchPatchPage();

	void Render(int X, int Y) override;
	void Update() override;

	bool ShouldShow();
private:
	void CheckForUpdates();

	PatchMng::InstallResult patchStatus = PatchMng::InstallResult::Ok;
	std::string updateMessageString = "";
	bool updateMessageIsError = false;
};