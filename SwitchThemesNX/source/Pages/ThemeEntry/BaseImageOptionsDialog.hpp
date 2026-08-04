#pragma once
#include "../../UI/UI.hpp"

// A base class for dialogs that show an image on the left side and some options on the right side
class BaseImageOptionsDialog : public IUIControlObj
{
	void Render(int X, int Y) override;
private:
	float MaxItemWidth = 0;
	bool FirstInteractionFocus = false;
protected:
	bool LockExit = false;
	bool SkipCancelButton = false;
	const char* PageName = "BaseImageOptions";
	float PaddingSizeX = 0;
	float PaddingSizeY = 0;

	u32 FirstItemId;
	u32 LastItemId;

	void PaddingLine();
	
	void FirstItemHere();
	void LastItemHere();

	bool Selectable(const char* label);

	virtual void RenderTop() = 0;
	virtual void RenderLeftPanel(float allowedWidth) = 0;
	virtual void RenderRightPanel(float x, float allowedWidth, float endY) = 0;
	virtual void RenderBottom() = 0;
};