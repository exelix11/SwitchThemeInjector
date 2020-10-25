#include "CppUnitTest.h"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/Layouts/Patches.hpp"
#include "Util.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SwitchThemesNXTests
{
	TEST_CLASS(Layouts)
	{
	public:
		TEST_METHOD(LayoutLoading)
		{
			// The theme injector will remove useless fields from jsons to save space, this test ensures the jsons keep the same meaning for the C++ parser
			// The ParsedLayouts folder is populated by SwitchThemesCommonTests.Layouts.LoadAndOptimizeAll
			for (const auto& f : std::filesystem::directory_iterator("../SwitchThemes/layouts/"))
			{
				if (f.is_regular_file() && f.path().extension() == ".json")
				{
					auto a = Patches::LoadLayout(Util::ReadAllText(f.path().string()));
					auto b = Patches::LoadLayout(Util::ReadTestString("ParsedLayouts/" + f.path().filename().string()));

					if (a != b)
						throw std::runtime_error("Files don't match: " + f.path().filename().string());
				}				
			}
		}
	};
}
