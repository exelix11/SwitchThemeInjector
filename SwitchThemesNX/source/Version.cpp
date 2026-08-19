#include "Version.hpp"

#ifdef _MSC_VER
	#define VER_NUM "Development"
#endif

#ifndef GITVER
	#define GITVER "Unknown version"
#endif

// VER_NUM and GITVER come from the makefile
#define VER_NAME "Ver. " VER_NUM

#ifdef DEVBUILD
	const std::string Version::Name = VER_NAME " - DEV BUILD";
#else
	const std::string Version::Name = VER_NAME;
#endif

const std::string Version::Commit = "Commit: " GITVER;
const std::string Version::UserAgent = "NXThemes/" VER_NUM;