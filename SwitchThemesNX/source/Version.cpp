#include "Version.hpp"

#ifndef GITVER
#define GITVER "Unknown version"
#endif

const std::string Version::Name = "Ver. 2.5.1";
const std::string Version::Commit = "Commit: " GITVER;