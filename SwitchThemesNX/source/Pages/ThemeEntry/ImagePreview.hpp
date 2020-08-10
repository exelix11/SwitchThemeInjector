#pragma once
#include "../../UI/UI.hpp"
#include "../../ViewFunctions.hpp"

class ImagePreview : public IUIControlObj
{
	public:
		ImagePreview(LoadedImage img) : img(img) {}

		void Render(int X, int Y) override
		{
			Utils::ImGuiNextFullScreen();
			ImGui::Begin("ImagePreview", nullptr, DefaultWinFlags);
			
			ImGui::Image((ImTextureID)img, { SCR_W, SCR_H });

			if (ImGui::IsItemClicked() || Utils::PageLeaveFocusInput())
				PopPage(this);

			ImGui::End();
		}
		
		void Update() override {};
	private:
		LoadedImage img;
};
