#pragma once
#include "BasePane.hpp"

namespace Panes 
{
	class Grp1Pane : public BasePane
	{
	public:
		u32 Version;
		std::string GroupName;
		std::vector<std::string> Panes;

		Grp1Pane(Buffer& buf, u32 version) : BasePane("grp1", buf), Version{ version }
		{
			LoadProperties();
		}

		Grp1Pane(u32 version) : BasePane("grp1", 8), Version{ version } {};
	private:
		void LoadProperties() 
		{
			Buffer bin{ data };
			bin.ByteOrder = Endianness::LittleEndian;
			if (Version > 0x05020000)
				GroupName = bin.readStr_Fixed(34);
			else
				GroupName = bin.readStr_Fixed(24);
			auto NodeCount = bin.readUInt16();
			if (Version <= 0x05020000)
				bin.readUInt16();
			auto pos = bin.Position;
			for (size_t i = 0; i < NodeCount; i++)
			{
				bin.Position = pos + i * 24;
				Panes.push_back(bin.readStr_Fixed(24));
			}
		}

		void ApplyChanges(Buffer &bin) override
		{
			if (Version > 0x05020000)
				bin.WriteFixedLengthString(GroupName, 34);
			else
				bin.WriteFixedLengthString(GroupName, 24);
			bin.Write((u16)Panes.size());
			if (Version <= 0x05020000)
				bin.Write((u16)0);
			for (const auto& s : Panes)
				bin.WriteFixedLengthString(s, 24);
			data = bin.getBuffer();
		}
	};
}