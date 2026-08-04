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

void BaseImageOptionsDialog::FirstItemHere() { FirstItemId = ImGui::GetItemID();} 
void BaseImageOptionsDialog::LastItemHere() { LastItemId = ImGui::GetItemID(); }

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

		LastItemHere();
	}

	ImGui::SetCursorPos({ PaddingSizeX, endY });
	RenderBottom();

	if (!LockExit && Utils::PageLeaveFocusInput(false))
		PopPage(this);

	// Handle cursor interactions
	if (FirstItemId && !FirstInteractionFocus)
	{
		if (ImGui::GetFocusID() == 0)
		{
			ImGui::SetFocusID(FirstItemId, ImGui::GetCurrentWindow());
			FirstInteractionFocus = true;
		}
	}

	if (LastItemId && FirstItemId && ImGui::GetFocusID())
	{
		if (ImGui::GetFocusID() == FirstItemId && NAV_UP)
			ImGui::SetFocusID(LastItemId, ImGui::GetCurrentWindow());
		if (ImGui::GetFocusID() == LastItemId && NAV_DOWN)
			ImGui::SetFocusID(FirstItemId, ImGui::GetCurrentWindow());
	}

	ImGui::End();
}