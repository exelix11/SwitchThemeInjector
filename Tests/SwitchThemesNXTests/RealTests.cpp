#include "CppUnitTest.h"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/Layouts/Bflan.hpp"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/Layouts/Bflyt/Bflyt.hpp"
#include "picosha2.h"
#include "Util.hpp"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/SarcLib/Sarc.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SwitchThemesNXTests
{
	TEST_CLASS(RealTests)
	{
	public:
		#define MAKE_TEST_SZS(x) TEST_METHOD(x) { ProcessSzs(#x); }
		
		MAKE_TEST_SZS(ResidentMenu)
		
		MAKE_TEST_SZS(Entrance)
		
		MAKE_TEST_SZS(Notification)
		
		MAKE_TEST_SZS(Flaunch)
		
		MAKE_TEST_SZS(Set)

	private:
		void CompareSarc(const SARC::SarcData& a, const SARC::SarcData& b)
		{
			if (a.files.size() != b.files.size())
				throw std::runtime_error("");

			for (const auto &[key, value] : a.files)
			{
				if (!b.files.count(key))
					throw std::runtime_error("file missing: " + key);

				if (b.files.at(key) != value)
					throw std::runtime_error("file different: " + key);
			}
		}
		
		void ProcessSzs(const std::string& name)
		{
			auto src = SARC::Unpack(Yaz0::Decompress(Util::ReadTestData("Source/" + name + ".szs")));
			auto exp = SARC::Unpack(Yaz0::Decompress(Util::ReadTestData("Expected/" + name + ".szs")));
		
			std::string lyt = Util::ExistsTest("Source/" + name + ".json") ? 
				Util::ReadTestString("Source/" + name + ".json") : "";

			SwitchThemesCommon::SzsPatcher p(std::move(src));
			if (!p.PatchMainBG(DDS))
				throw std::runtime_error("");

			if (lyt != "")
			{
				auto l = Patches::LoadLayout(lyt);
				if (!p.PatchLayouts(l))
					throw std::runtime_error("");
			}
			
			auto fin = p.GetFinalSarc();
			CompareSarc(exp, fin);
		}

		std::vector<u8> DDS = Util::ReadTestData("bg.dds");
	};
}
