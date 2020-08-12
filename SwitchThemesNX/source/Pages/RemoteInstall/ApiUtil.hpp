#pragma once
#include <vector>
#include <string>
#include <curl/curl.h>
#include "../../SwitchThemesCommon/MyTypes.h"

namespace RemoteInstall::API::Util
{
    CURL* EasyGET(const std::string& url, std::vector<u8>& out, intptr_t priv = 0);
}