#pragma once
#include "API.hpp"
#include "../../UI/UI.hpp"
#include <vector>
#include "../../SwitchThemesCommon/Layouts/Base64.hpp"
#include "Worker.hpp"

namespace RemoteInstall
{
	class ListPage : public IUIControlObj
	{
	public:
		ListPage(API::APIResponse&& response, Worker::ImageFetch::Result&& img);

		void Update() override;
		void Render(int X, int Y) override;
		~ListPage();
	private:
		enum class Result
		{
			None,
			Clicked,
			Preview
		};

		Result RenderWidget(bool selected, const std::string& name, LoadedImage img);

		API::APIResponse response;
		Worker::ImageFetch::Result images;

		std::vector<bool> Selection;
		int SelectedCount() const;
		void ApplySelection(bool all);
		void ToggleSelected(size_t i);
		bool IsSelected(size_t i);
		std::vector<std::string> GetSelectedUrls();

		std::string DownloadBtnText = "Download";
		void SelectionChanged();

		void DownloadClicked();
	};
}