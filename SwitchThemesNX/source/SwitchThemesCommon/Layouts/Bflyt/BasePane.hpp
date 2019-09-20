
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "../../BinaryReadWrite/Buffer.hpp"
#include "../../MyTypes.h"
#include <memory>

namespace Panes
{
	static inline Vector3 ReadVec3(Buffer& buf)
	{
		Vector3 res;
		res.X = buf.readFloat();
		res.Y = buf.readFloat();
		res.Z = buf.readFloat();
		return res;
	}

	static inline Vector2 ReadVec2(Buffer& buf)
	{
		Vector2 res;
		res.X = buf.readFloat();
		res.Y = buf.readFloat();
		return res;
	}

	static inline void WriteVec3(Buffer& bin, Vector3 _x)
	{
		bin.Write(_x.X); bin.Write(_x.Y); bin.Write(_x.Z);
	}

	static inline void WriteVec2(Buffer& bin, Vector2 _x)
	{
		bin.Write(_x.X); bin.Write(_x.Y);
	}

	class BasePane
	{
	public:
		std::unique_ptr<BasePane> UserData;
		std::weak_ptr<BasePane> Parent;
		std::vector<std::shared_ptr<BasePane>> Children;

		const std::string name;
		std::vector<u8> data;

		std::string PaneName = ""; //This is optional

		BasePane(const std::string& _name, u32 len) : name(_name), data(len - 8)
		{
		}

		//BasePane(const BasePane& ref);
		BasePane(const std::string& _name, Buffer& reader) : name(_name)
		{
			auto length = reader.readUInt32();
			data = reader.readBytes(length - 8);
		}

		virtual ~BasePane() {}
		virtual void ApplyChanges(Buffer& writer) {}
		void WritePane(Buffer& writer)
		{
			Buffer bin;
			bin.ByteOrder = writer.ByteOrder;
			ApplyChanges(bin);
			if (bin.Length() != 0)
			{
				data = bin.getBuffer();
			}

			writer.WriteFixedLengthString(name, 4);
			writer.Write(u32(data.size() + 8));
			writer.Write(data);
			if (UserData)
				UserData->WritePane(writer);
		}
	};
}