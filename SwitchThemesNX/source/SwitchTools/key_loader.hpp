#pragma once

#include <hactool.h>
#include <string>
#include <memory>
#include <cstdint>

#if __SWITCH__
#include <switch.h>
#endif

namespace hactool 
{
	// Holds service handles for the extraction process
	class ExtractionContext
	{
	public:
		ExtractionContext& operator=(const ExtractionContext&) = delete;
		ExtractionContext& operator=(ExtractionContext&&) = delete;

#if __SWITCH__
		FsFileSystem sys;
		void ExitServices();
#endif
		std::string getNcaPath(uint64_t id);

		ExtractionContext();
		~ExtractionContext() noexcept(false);
	};

	std::unique_ptr<ExtractionContext> Initialize();
	void LoadKeys(hactool_settings_t* settings);
}