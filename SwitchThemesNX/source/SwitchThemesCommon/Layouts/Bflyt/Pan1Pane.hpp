#pragma once
#include "BasePane.hpp"
#include "../Patches.hpp"

namespace Panes 
{
	enum class OriginX : u8
	{
		Center = 0,
		Left = 1,
		Right = 2
	};

	enum class OriginY : u8
	{
		Center = 0,
		Top = 1,
		Bottom = 2
	};

	class Pan1Pane : public BasePane
	{
	public:
		Vector3 Position, Rotation;
		Vector2 Scale, Size;

		bool GetVisible() { return (_flag1 & 0x1) == 0x1; }
		void SetVisible(bool value) {
			if (value)
				_flag1 |= 0x1;
			else
				_flag1 &= 0xFE;
		}

		OriginX GetOriginX() {
			return (OriginX)((_flag2 & 0xC0) >> 6);;
		}

		void SetOriginX(OriginX val) {
			_flag2 &= ((u8)(~0xC0));
			_flag2 |= (u8)((u8)val << 6);
		}

		OriginY GetOriginY() {
			return (OriginY)((_flag2 & 0x30) >> 4);
		}

		void SetOriginY(OriginY val) {
			_flag2 &= ((u8)(~0x30));
			_flag2 |= (u8)((u8)val << 4);
		}

		OriginX GetParentOriginX() {
			return (OriginX)((_flag2 & 0xC) >> 2);
		}

		void SetParentOriginX(OriginX val) {
			_flag2 &= ((u8)(~0xC));
			_flag2 |= (u8)((u8)val << 2);
		}

		OriginY GetParentOriginY() {
			return (OriginY)((_flag2 & 0x3));
		}

		void SetParentOriginY(OriginY val) {
			_flag2 &= ((u8)(~0x3));
			_flag2 |= (u8)val;
		}

		Pan1Pane(Buffer& b, Endianness e, const std::string &name = "pan1") : BasePane(name, b)
		{
			LoadProperties(e);
		}
		
		void ApplyChanges(Buffer &bin) override
		{
			bin.Write(data);
			bin.Position = 0;
			bin.Write(_flag1);
			bin.Write(_flag2);
			bin.Position = 0x2C - 8;
			WriteVec3(bin,Position);
			WriteVec3(bin, Rotation);
			WriteVec2(bin, Scale);
			WriteVec2(bin, Size);
		}

	private:
		void LoadProperties(Endianness e) 
		{
			Buffer buf(data);
			buf.ByteOrder = e;
			_flag1 = buf.readUInt8();
			_flag2 = buf.readUInt8();
			buf.Position += 2;
			PaneName = buf.readStr_NullTerm(0x18);
			buf.Position = 0x2c - 8;
			Position = ReadVec3(buf);
			Rotation = ReadVec3(buf);
			Scale = ReadVec2(buf);
			Size = ReadVec2(buf);
		}

		u8 _flag1;
		u8 _flag2;
	};
};