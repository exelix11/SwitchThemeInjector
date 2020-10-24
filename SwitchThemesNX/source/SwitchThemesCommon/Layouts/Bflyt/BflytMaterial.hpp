#pragma once
#include "../../MyTypes.h"
#include "../../BinaryReadWrite/Buffer.hpp"

namespace Panes {
	class BflytMaterial
	{
	public:
		struct TextureTransform
		{
			float X;
			float Y;
			float Rotation;
			float ScaleX;
			float ScaleY;
		};

		struct TextureReference
		{
			u16 TextureId;
			u8 WrapS;
			u8 WrapT;
		};

		std::string Name;
		u32 ForegroundColor;
		u32 BackgroundColor;

		std::vector<TextureReference> Textures;
		std::vector<TextureTransform> TextureTransformations;

		BflytMaterial(const std::vector<u8>& data, u32 Version, Endianness bo) 
		{
			Data = data;
			Buffer buf{ Data };
			buf.ByteOrder = bo;
			Name = buf.readStr_Fixed(28);
			if (Version >= 0x08000000)
			{
				flags = buf.readUInt32();
				buf.readUInt32();
				ForegroundColor = buf.readUInt32_LE();
				BackgroundColor = buf.readUInt32_LE();
			}
			else
			{
				ForegroundColor = buf.readUInt32_LE();
				BackgroundColor = buf.readUInt32_LE();
				flags = buf.readUInt32();
			}

			Textures.resize(flags & 3);
			for (auto& t : Textures)
				t = {buf.readUInt16(), buf.readUInt8(), buf.readUInt8() };

			TextureTransformations.resize((flags & 0xC) >> 2);
			for (auto& t : TextureTransformations)
				t = { buf.readFloat(), buf.readFloat(), buf.readFloat(), buf.readFloat(), buf.readFloat()};
		}

		std::vector<u8> Write(u32 version, Endianness bo)
		{
			if (Textures.size() > 3) throw std::runtime_error("[" + Name + "] A material can have no more than 3 texture references.");
			if (TextureTransformations.size() > 3) throw std::runtime_error("[" + Name + "] A material can have no more than 3 texture transformations.");

			flags &= ~3;
			flags |= Textures.size();

			flags &= ~0xC;
			flags |= TextureTransformations.size() << 2;

			Buffer buf{ Data };
			buf.ByteOrder = bo;
			buf.Position = 0;

			buf.WriteFixedLengthString(Name, 28);
			if (version >= 0x08000000)
			{
				buf.writeUInt32_LE(flags);
				buf.Position += 4;
				buf.writeUInt32_LE(ForegroundColor);
				buf.writeUInt32_LE(BackgroundColor);
			}
			else
			{
				buf.writeUInt32_LE(ForegroundColor);
				buf.writeUInt32_LE(BackgroundColor);
				buf.writeUInt32_LE(flags);
			}

			for (const auto& t : Textures)
			{
				buf.writeUInt16_LE(t.TextureId);
				buf.Write(t.WrapS);
				buf.Write(t.WrapT);
			}

			for (const auto& t : TextureTransformations)
			{
				buf.Write(t.X);
				buf.Write(t.Y);
				buf.Write(t.Rotation);
				buf.Write(t.ScaleX);
				buf.Write(t.ScaleY);
			}

			return buf.getBuffer();
		}

	private:
		std::vector<u8> Data;
		u32 flags;
	};
}