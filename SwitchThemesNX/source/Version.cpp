#include "Version.hpp"

#define VER_NAME "Ver. 2.6.3"

#ifndef GITVER
	#define GITVER "Unknown version"
#endif

#ifdef DEVBUILD
	const std::string Version::Name = VER_NAME " - DEV BUILD";
#else
	const std::string Version::Name = VER_NAME;
#endif

const std::string Version::Commit = "Commit: " GITVER;