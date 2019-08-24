#pragma once
#include "BasePane.hpp"
#include "Pan1Pane.hpp"
#include "RGBAColor.hpp"

namespace Panes
{
	class Pic1Pane : public Pan1Pane
	{
	public:
		RGBAColor ColorTopRight;
		RGBAColor ColorTopLeft;
		RGBAColor ColorBottomRight;
		RGBAColor ColorBottomLeft;

		u16 MaterialIndex;

		struct UVCoord
		{
			Vector2 TopLeft;
			Vector2 TopRight;
			Vector2 BottomLeft;
			Vector2 BottomRight;
		};
		std::vector<UVCoord> UvCoords;

		Pic1Pane(Buffer& reader, Endianness e) : Pan1Pane(reader, e, "pic1")
		{
			LoadProperties(e);
		}

		void ApplyChanges(Buffer& bin) override 
		{
			Pan1Pane::ApplyChanges(bin);
			bin.Position = 0x54 - 8;
			ColorTopLeft.Write(bin);
			ColorTopRight.Write(bin);
			ColorBottomLeft.Write(bin);
			ColorBottomRight.Write(bin);
			bin.Write(MaterialIndex);
			bin.Write((u8)UvCoords.size());
			bin.Write((u8)0);
			for (const auto& uv : UvCoords)
			{
				WriteVec2(bin, uv.TopLeft);
				WriteVec2(bin, uv.TopRight);
				WriteVec2(bin, uv.BottomLeft);
				WriteVec2(bin, uv.BottomRight);
			}
		}

	private:
		void LoadProperties(Endianness e) 
		{
			Buffer buf(data);
			buf.ByteOrder = e;
			buf.Position = 0x54 - 8;
			ColorTopLeft = RGBAColor::Read(buf);
			ColorTopRight = RGBAColor::Read(buf);
			ColorBottomLeft = RGBAColor::Read(buf);
			ColorBottomRight = RGBAColor::Read(buf);
			MaterialIndex = buf.readUInt16(); //material index
			int uvCount = buf.readUInt8();
			buf.readUInt8(); //padding
			for (int i = 0; i < uvCount; i++)
				UvCoords.push_back(
					UVCoord	{
						ReadVec2(buf),
						ReadVec2(buf),
						ReadVec2(buf),
						ReadVec2(buf)
					});
		}
	};
};