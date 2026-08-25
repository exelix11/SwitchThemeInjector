#pragma once
#include "API.hpp"
#include <vector>
#include "Worker.hpp"

namespace RemoteInstall
{
	class ListPage : public IUIControlObj
	{
	public:
		ListPage(API::APIResponse&& response, Worker::ImageFetch::Result&& img);

		void Update() override;
		void Render(int X, int Y) override;
	private:
		enum class Result
		{
			None,
			Clicked,
			Preview
		};

		Result RenderWidget(size_t index);

		API::APIResponse response;
		Worker::ImageFetch::Result images;

		std::vector<bool> Selection;
		size_t SelectedCount() const;
		void ApplySelection(bool all);
		void ToggleSelected(size_t i);
		bool IsSelected(size_t i);

		std::string DownloadBtnText = "Download";
		void SelectionChanged();

		void DownloadClicked();

		void PopulateScrollIDs();
		std::vector<std::string> ScrollIDs;
	};
}
