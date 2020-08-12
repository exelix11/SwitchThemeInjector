#include "RemoteInstall.hpp"

#include "Worker.hpp"
#include "../../ViewFunctions.hpp"
#include "Detail.hpp"
#include "List.hpp"
#include "API.hpp"

void RemoteInstall::Initialize()
{
    API::Initialize();
    API::ReloadProviders();
}

void RemoteInstall::Finalize()
{
    API::Finalize();
}

bool RemoteInstall::IsInitialized()
{
    return API::IsInitialized();
}

const std::vector<RemoteInstall::Provider>& RemoteInstall::GetProviders()
{
    return API::GetProviders();
}

void RemoteInstall::Begin(const Provider& provider, const std::string& ID)
{
    auto URL = API::MakeUrl(provider.UrlTemplate, ID);

    auto res = API::GetManifest(URL);
    if (res.Entries.size() == 0) return;

    std::vector<std::string> imageUrls;

    for (const auto& res : res.Entries)
        imageUrls.push_back(res.Preview);

    Worker::ImageFetch::Result ImageLoadResult;
    PushPageBlocking(new Worker::ImageFetch(imageUrls, ImageLoadResult));

    if (res.Entries.size() == 1)
    {
        PushPage(new DetailPage(res.Entries[0], ImageLoadResult.List[0]));
    }
    else
    {
        PushPage(new ListPage(std::move(res), std::move(ImageLoadResult)));
    }
}