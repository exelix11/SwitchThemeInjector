#pragma once
#include "BasePane.hpp"
#include "Pan1Pane.hpp"
#include "RGBAColor.hpp"

namespace Panes
{
	// TODO: Implement all properties
	class Txt1Pane : public Pan1Pane
	{
	public:
		RGBAColor FontTopColor;
		RGBAColor ShadowTopColor;
		RGBAColor FontBottomColor;
		RGBAColor ShadowBottomColor;

		Txt1Pane(Buffer& reader, Endianness e) : Pan1Pane(reader, e, "txt1")
		{
			LoadProperties(e);
		}

		void ApplyChanges(Buffer& bin) override 
		{
			Pan1Pane::ApplyChanges(bin);
			bin.Position = 0x54 - 8 + 20; 
			FontTopColor.Write(bin);
			FontBottomColor.Write(bin);
			bin.Position = 0x54 - 8 + 64;
			ShadowTopColor.Write(bin);
			ShadowBottomColor.Write(bin);
		}

	private:
		void LoadProperties(Endianness e) 
		{
			Buffer buf(data);
			buf.ByteOrder = e;
			buf.Position = 0x54 - 8 + 20;
			FontTopColor = RGBAColor::Read(buf);
			FontBottomColor = RGBAColor::Read(buf);
			buf.Position = 0x54 - 8 + 64;
			ShadowTopColor = RGBAColor::Read(buf);
			ShadowBottomColor = RGBAColor::Read(buf);
		}
	};
};