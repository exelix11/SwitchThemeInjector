#include "API.hpp"
#include "../../SwitchThemesCommon/Layouts/json.hpp"
#include "../../Platform/Platform.hpp"
#include "ApiUtil.hpp"
#include "Worker.hpp"
#include "../../fs.hpp"
#include "../../Version.hpp"

#include <curl/curl.h>

static bool Initialized = false;
static constexpr std::string_view IdMarker = "%%ID%%";
static std::vector<RemoteInstall::Provider> Providers;

static inline void AssertInitialized() 
{
    if (!Initialized)
        throw std::runtime_error("RemoteInstall::API is not initialized");
}

namespace RemoteInstall::API
{
    void from_json(const nlohmann::json& j, Entry& p) {
        j.at("name").get_to(p.Name);
        j.at("target").get_to(p.Target);
        j.at("url").get_to(p.Url);

        if (j.count("preview"))
            j.at("preview").get_to(p._Preview);

        if (j.count("thumbnail"))
            j.at("thumbnail").get_to(p._Thumbnail);

        // Enforce image links ? Doesn't really matter cause empty strings will just cause a black square
    }

    void from_json(const nlohmann::json& j, APIResponse& p) {
        j.at("themes").get_to(p.Entries);

        if (p.Entries.size() == 0)
            throw std::runtime_error("The server did not return any themes");

        if (j.count("groupname") && j["groupname"].is_string()) {
            j.at("groupname").get_to(p.GroupName);
        }
    }
}

std::string RemoteInstall::API::MakeUrl(const std::string& provider, const std::string& ID)
{
    auto p = provider.find(IdMarker);
    if (p == std::string::npos)
        throw std::runtime_error("Marker not found in provider string");

    std::string res = provider;
    return res.replace(p, IdMarker.length(), ID);
}

template <typename T>
struct ScopeGuard
{
    ScopeGuard(T arg) : lambda(arg) {}
    ~ScopeGuard() { lambda(); }
    T lambda;

    ScopeGuard(ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) = delete;
    ScopeGuard& operator=(ScopeGuard& other) = delete;
};

static nlohmann::json ApiGet(const std::string& url) 
{
    std::vector<u8> result;

    CURL* curl = RemoteInstall::API::Util::EasyGET(url, result);
    ScopeGuard curlguard{ [curl]() {curl_easy_cleanup(curl); } };    

    auto res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(res));

    if (result.size() == 0)
        throw std::runtime_error("Received empty response");

    if (result[0] != '{')
        throw std::runtime_error("Received invalid JSON");

#ifndef __SWITCH__
    std::string StringResponse(result.begin(), result.end());
#endif
    return nlohmann::json::parse(result);
}

RemoteInstall::API::APIResponse RemoteInstall::API::GetManifest(const std::string& url)
{
    AssertInitialized();

    auto response = ApiGet(url);
    return response.get<APIResponse>();
}

void RemoteInstall::API::ReloadProviders()
{
    Providers.clear();
    // Builtin providers
    Providers.push_back({ "Themezer.net", "https://api.themezer.net/switch/nxinstaller/%%ID%%", false });
    // Load extra providers from sd
    try
    {
        auto json = nlohmann::json::parse(fs::OpenFile(fs::path::ProvidersFile));
        
        if (!json.is_array()) 
            throw std::runtime_error("Wrong file format");

        for (const auto& elem : json)
            Providers.push_back({ elem["name"], elem["url"], elem.count("static") ? elem["static"].get<bool>() : false });
    }
    catch (std::exception &ex)
    {
        LOGf("Failed loading SD providers : %s", ex.what());
    }
}

const std::vector<RemoteInstall::Provider>& RemoteInstall::API::GetProviders()
{
    return Providers;
}

const RemoteInstall::Provider& RemoteInstall::API::GetProvider(size_t index)
{
    return Providers.at(index);
}

size_t RemoteInstall::API::ProviderCount()
{
    return Providers.size();
}

bool RemoteInstall::API::IsInitialized()
{
    return Initialized;
}

void RemoteInstall::API::Initialize()
{
    if (Initialized) return;
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res)
    {
        LOGf("Curl init failed : %d", res);
        return;
    }
    Initialized = true;
}

void RemoteInstall::API::Finalize()
{
    if (!Initialized) return;
    Initialized = false;
    curl_global_cleanup();
}

static size_t CurlVectorCallback(void* ptr, size_t size, size_t nmemb, void* stream) {
    auto vec = reinterpret_cast<std::vector<u8>*>(stream);
    auto curSz = vec->size();
    vec->resize(vec->size() + size * nmemb);
    std::memcpy(&(*vec)[curSz], ptr, size * nmemb);
    return size * nmemb;
}

CURL* RemoteInstall::API::Util::EasyGET(const std::string& url, std::vector<u8>& out, intptr_t priv)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl_easy_init failed !");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlVectorCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_PRIVATE, priv);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, Version::UserAgent.c_str()); 
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 

    return curl;
}
