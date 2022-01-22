#include "Worker.hpp"
#include "../../ViewFunctions.hpp"

RemoteInstall::Worker::BaseWorker::BaseWorker(const std::vector<std::string>& urls, bool cancellable)
    : Cancellable(cancellable), urls(urls), Results(urls.size())
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
        handles.push_back(transfer);
        curl_multi_add_handle(cm, transfer);
    }

    UpdateStatusMessage();
}

void RemoteInstall::Worker::BaseWorker::Update()
{
    if (Done) 
        return;

    if (CancellationRequested)
    {
        Errors << "Operation cancelled by the user";
        for (auto h : handles)
        {
            curl_multi_remove_handle(cm, h);
            curl_easy_cleanup(h);
        }
    }

    int running_handles = 1;
    curl_multi_perform(cm, &running_handles);
    
    int msgs_left = -1;
    while (CURLMsg* msg = curl_multi_info_read(cm, &msgs_left)) {
        if (msg->msg == CURLMSG_DONE) {
            CURL* e = msg->easy_handle;
            
            uintptr_t index = 0;
            long httpCode = 0;

            curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &index);
            curl_easy_getinfo(e, CURLINFO_RESPONSE_CODE, &httpCode);

            if (msg->data.result != CURLE_OK || !OnFinished(index, httpCode))
            {
                if (appendUrlToError) {
                    if (index < urls.size())
                    {
                        Errors << urls[index];
                        Results[index] = {};
                    }
                    else
                        Errors << "Unknown id " << index; //Can this ever happen ?
                }

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
        handles.clear();
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

    if (Cancellable)
    {
        if (CancellationRequested)
            Utils::ImGuiCenterString("Cancelling operation...");
        else {
            if (Utils::ImGuiCenterButton("Cancel operation")) {
                CancellationRequested = true;
            }
        }
    }
            
	ImGui::End();
	ImGui::PopFont();
}

RemoteInstall::Worker::BaseWorker::~BaseWorker()
{
    if (cm)
        curl_multi_cleanup(cm);
}

void RemoteInstall::Worker::BaseWorker::SetLoadingLine(std::string_view s)
{
    LoadingLine = s;
    CustomLine = true;
}

void RemoteInstall::Worker::BaseWorker::Dialog(const std::string& msg)
{
    DialogBlocking(msg);
}

void RemoteInstall::Worker::BaseWorker::UpdateStatusMessage()
{
    if (CustomLine) 
           return;

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