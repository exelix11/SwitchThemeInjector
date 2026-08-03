#include "BaseImageOptionsDialog.hpp"
#include "../../ViewFunctions.hpp"

void BaseImageOptionsDialog::PaddingLine()
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + PaddingSizeY);
}

bool BaseImageOptionsDialog::Selectable(const char* label)
{
	return ImGui::Selectable(label, false, ImGuiSelectableFlags_DontClosePopups, { MaxItemWidth, 0 });
}

void BaseImageOptionsDialog::Render(int X, int Y) 
{
	Utils::ImGuiNextFullScreen();
	ImGui::Begin(PageName, nullptr, DefaultWinFlags);

	if (PaddingSizeX == 0)
	{
		PaddingSizeY = ImGui::GetStyle().ItemSpacing.x;
		PaddingSizeX = PaddingSizeY * 3.5f;
	}

	PaddingLine();
	RenderTop();

	auto startY = ImGui::GetCursorPosY();

	// Three paddings: left, image to list, list to right
	auto previewWidth = 2 * (SCR_W - PaddingSizeX * 3) / 3.0f;

	ImGui::SetCursorPosX(PaddingSizeX);
	RenderLeftPanel(previewWidth);

	auto endY = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(startY);

	auto itemStart = previewWidth + PaddingSizeX * 2;
	auto itemSize = SCR_W - itemStart - PaddingSizeX;

	MaxItemWidth = itemSize;
	RenderRightPanel(itemStart, itemSize, endY);

	if (!SkipCancelButton)
	{
		static float lineSize = 0;

		if (lineSize == 0)
			lineSize = ImGui::CalcTextSize("Cancel").y;

		ImGui::SetCursorPos({ itemStart, endY - lineSize - PaddingSizeY });

		if (Selectable("Cancel"))
			PopPage(this);
	}

	ImGui::SetCursorPos({ PaddingSizeX, endY });
	RenderBottom();

	if (!LockExit && Utils::PageLeaveFocusInput(false))
		PopPage(this);

	ImGui::End();
}