#include "CppUnitTest.h"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/Layouts/Bflan.hpp"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/Layouts/Bflyt/Bflyt.hpp"
#include "picosha2.h"
#include "Util.hpp"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/Layouts/Bflyt/BflytPatcher.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SwitchThemesNXTests
{
	TEST_CLASS(SyntheticTests)
	{
	public:
		TEST_METHOD(BflanDeserialize)
		{
			auto bflan = BflanDeserializer::FromJson(Util::ReadString("Synthetic/bflan.json"));
			auto hash = Util::StringHash(bflan->WriteFile());
			delete bflan;

			if (hash != "43CE2CDE8B2638E36CA1723328CD571DB350D3BC011B6389944FAD69260BC748")
				throw std::runtime_error("");
		}

		TEST_METHOD(BgPaneInjection)
		{
			auto bflyt = BflytFile(Util::ReadData("Synthetic/bginjection.bflyt"));

			BflytPatcher p(bflyt);
			
			auto t = *std::find_if(Patches::DefaultTemplates.begin(), Patches::DefaultTemplates.end(),
				[](const PatchTemplate& t) {
					return t.szsName == "ResidentMenu.szs" && t.targetPanels[0] == "P_Bg_00";
				});

			if (!p.PatchBgLayout(t))
				throw std::runtime_error("");

			auto d = bflyt.SaveFile();

			auto hash = Util::StringHash(d);

			if (hash != "C4F98DF5F9227E122076DA31BEA351523E2780C2287EC7F604FBC86D59703C21")
				throw std::runtime_error("");
		}
	};
}
