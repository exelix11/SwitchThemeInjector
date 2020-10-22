#include "CppUnitTest.h"
#include "../../SwitchThemesNX/source/SwitchThemesCommon/SwitchThemesCommon.hpp"
#include "Util.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SwitchThemesNXTests
{
	TEST_CLASS(Compression)
	{
	public:
		TEST_METHOD(ConsistentCompression)
		{
			auto c = Yaz0::Compress(MakeData(), 9);
			auto h = Util::StringHash(c);

			if (h != "7865BE4B54FBFE3ED21DEA9CB1E184F0F305404251203AD9EFDFA264280CD0FD")
				throw std::runtime_error("");
		}

		TEST_METHOD(CompressionDecompression)
		{
			auto d = MakeData();
			auto c = Yaz0::Compress(d, 9);
			auto u = Yaz0::Decompress(c);

			if (u != d)
				throw std::runtime_error("");
		}

	private:
		std::vector<u8> MakeData()
		{
			Buffer b;
			b.Write("Hello word, here's some data", Buffer::BinaryString::NoPrefixOrTermination);
			for (int i = 0; i < 100; i++)
				b.Write(i);
			return b.getBuffer();
		}
	};
}
