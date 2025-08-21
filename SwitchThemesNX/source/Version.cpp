#include "Version.hpp"

#define VER_NUM "2.8.2"
#define VER_NAME "Ver. " VER_NUM

#ifndef GITVER
	#define GITVER "Unknown version"
#endif

#ifdef DEVBUILD
	const std::string Version::Name = VER_NAME " - DEV BUILD";
#else
	const std::string Version::Name = VER_NAME;
#endif

const std::string Version::Commit = "Commit: " GITVER;
const std::string Version::UserAgent = "NXThemes/" VER_NUM;