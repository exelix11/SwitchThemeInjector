#include "Worker.hpp"
#include "../../ViewFunctions.hpp"

RemoteInstall::Worker::BaseWorker::BaseWorker(const std::vector<std::string>& urls_ist)
    : urls(urls_ist), Results(urls_ist.size())
{
    cm = curl_multi_init();

    if (!cm)
        throw std::runtime_error("curl_multi_init failed");

    curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, MaxSessions * 4);
    curl_multi_setopt(cm, CURLMOPT_MAX_TOTAL_CONNECTIONS, MaxSessions);
    curl_multi_setopt(cm, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);

    for (size_t i = 0; i < urls.size(); i++)
    {
        CURL* transfer = RemoteInstall::API::Util::EasyGET(urls[i], Results[i], i);
        curl_multi_add_handle(cm, transfer);
    }

    UpdateStatusMessage();
}

void RemoteInstall::Worker::BaseWorker::Update()
{
    if (Done) 
        return;

    int running_handles = 1;
    curl_multi_perform(cm, &running_handles);
    
    int msgs_left = -1;
    while (CURLMsg* msg = curl_multi_info_read(cm, &msgs_left)) {
        if (msg->msg == CURLMSG_DONE) {
            CURL* e = msg->easy_handle;
            uintptr_t index = 0;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &index);
            
            if (msg->data.result != CURLE_OK || !OnFinished(index))
            {
                if (index < urls.size())
                {
                    Errors << urls[index];
                    Results[index] = {};
                }
                else
                    Errors << "Unknown id " << index; //Can this ever happen ?

                if (msg->data.result != CURLE_OK)
                    Errors << " failed: " << curl_easy_strerror(msg->data.result) << "(" << msg->data.result << ")" << std::endl;
                else
                    Errors << " failed due to handler error" << std::endl;

                OnError(index);
            }

            Completed++;
            UpdateStatusMessage();
            curl_multi_remove_handle(cm, e);
            curl_easy_cleanup(e);
        }
        else {
            Errors << "Error: CURLMsg " << msg->msg << std::endl;
        }
    }

    if (!running_handles)
    {
        Done = true;
        OnComplete();
        PopPage(this);
    }
}

void RemoteInstall::Worker::BaseWorker::Render(int X, int Y)
{
	ImGui::PushFont(font30);

	Utils::ImGuiNextFullScreen();
	ImGui::Begin("RemoteInstallWorker", nullptr, DefaultWinFlags);

    const float BaseY = SCR_H / 2 - ImGui::GetTextLineHeightWithSpacing() / 2;
    ImGui::SetCursorPosY(BaseY);

    Utils::ImGuiCenterString(LoadingLine.c_str());

	ImGui::End();
	ImGui::PopFont();
}

RemoteInstall::Worker::BaseWorker::~BaseWorker()
{
    if (cm)
        curl_multi_cleanup(cm);
}

void RemoteInstall::Worker::BaseWorker::Dialog(const std::string& msg)
{
    DialogBlocking(msg);
}

void RemoteInstall::Worker::BaseWorker::UpdateStatusMessage()
{
    std::stringstream msg;
    msg << "Downloading data (" << Completed << " of " << urls.size() << ") ...";
    LoadingLine = msg.str();
}

void RemoteInstall::Worker::ImageFetch::OnComplete()
{
    const auto& str = Errors.str();
    if (str.length())
        DialogBlocking(str);

    for (const auto& res : Results)
    {
        if (res.size())
            OutResult.List.push_back(Image::Load(res));
        else
            OutResult.List.push_back(0);
    }
}

void RemoteInstall::Worker::DownloadSingle::OnComplete()
{
    const auto& str = Errors.str();
    if (str.length())
    {
        OutBuffer = {};
        DialogBlocking(str);
    }
    else OutBuffer = std::move(Results[0]);
}