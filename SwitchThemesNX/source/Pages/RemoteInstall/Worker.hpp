#pragma once
#include <vector>
#include <string>
#include <sstream>

#include "ApiUtil.hpp"
#include "../../UI/UI.hpp"

namespace RemoteInstall::Worker
{
	class BaseWorker : public IUIControlObj
	{
	public:
		static constexpr long MaxSessions = 6;

		BaseWorker(const std::vector<std::string>& urls);

		void Update() override;
		void Render(int X, int Y) override;
		virtual ~BaseWorker();
	protected:
		virtual void OnError(uintptr_t index) {}
		virtual bool OnFinished(uintptr_t index) { return true; }

		virtual void OnComplete() = 0;

		void Dialog(const std::string& msg);

		const std::vector<std::string> urls;
		std::vector<std::vector<u8>> Results;

		std::string LoadingLine = "Downloading data...";
		
		std::stringstream Errors;
	private:
		void UpdateStatusMessage();

		int Completed = 0;
		bool Done = false;
		CURLM* cm;
	};

	class ImageFetch : public BaseWorker
	{
	public:
		struct Result {
			size_t Total, Failed;
			std::vector<LoadedImage> List;
		};

		ImageFetch(const std::vector<std::string>& urls, Result& result) : BaseWorker(urls), OutResult(result)
		{
			OutResult = {};
			OutResult.Total = urls.size();
		}
	protected:
		void OnComplete() override;
		void OnError(uintptr_t index) override { OutResult.Failed++; }

		Result& OutResult;
	};

	class DownloadSingle : public BaseWorker
	{
	public:
		DownloadSingle(const std::string& url, std::vector<u8>& out) : BaseWorker({ url }), OutBuffer(out) {}
	protected:
		void OnComplete() override;
		std::vector<u8>& OutBuffer;
	};

	template<typename T>
	class ActionOnItemFinish : public BaseWorker 
	{
	public:
		ActionOnItemFinish(const std::vector<std::string>& urls, size_t& failed, const T& action) : BaseWorker(urls), Failed(failed), Action(action)
		{
			Failed = 0;
		}
	protected:
		void OnError(uintptr_t index) override { Failed++; }

		void OnComplete() override 
		{
			const auto& str = Errors.str();
			if (str.length())
				Dialog(str);
		}
		
		bool OnFinished(uintptr_t index) override
		{
			return Action(std::move(Results[index]), index);
		}

		size_t& Failed;
		const T& Action;
	};
}