#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "../../BinaryReadWrite/Buffer.hpp"
#include "../../MyTypes.h"
#include "BasePane.hpp"

namespace Panes 
{
	class Usd1Pane : public BasePane
	{
	public:
		enum class ValueType : u8
		{
			data = 0,
			int32 = 1,
			single = 2,
			other = 3
		};

		struct EditableProperty
		{
			std::string Name;
			size_t ValueOffset;
			u16 ValueCount;
			ValueType type;

			std::vector<std::string> value;
		};

		std::vector<EditableProperty> Properties;
		//Discard the pointer if the Properties vector is changed
		EditableProperty* FindName(const std::string& name)
		{
			for (auto& p : Properties)
				if (p.Name == name)
					return &p;
			return nullptr;
		}

		void AddProperty(const std::string& name, const std::vector<std::string>& values, ValueType type)
		{
			AddedProperties.push_back({ name, 0, (u16)values.size(), type, values });
		}

		Usd1Pane(Buffer& reader) : BasePane("usd1", reader)
		{
			LoadProperties();
		}

		void ApplyChanges(Buffer& bin) override
		{
			bin.Write((u16)(Properties.size() + AddedProperties.size() + UnknownPropertiesCount));
			bin.Write((u16)0);
			for (size_t i = 0; i < 3 * AddedProperties.size(); i++) bin.Write((u32)0);
			bin.Write(data, 4, data.size() - 4); //write rest of entries, adding new elements first doesn't break relative offets in the struct
			for (const auto& m : Properties)
			{
				if (m.type != ValueType::int32 && m.type != ValueType::single) continue;
				bin.Position = m.ValueOffset + 0xC * AddedProperties.size();
				for (size_t i = 0; i < m.ValueCount; i++)
				{
					if (m.type == ValueType::int32)
						bin.Write(stoi(m.value[i]));
					else
						bin.Write(stof(m.value[i]));
				}
			}
			for (size_t i = 0; i < AddedProperties.size(); i++)
			{
				bin.Position = bin.Length();
				u32 DataOffset = bin.Position;
				for (int j = 0; j < AddedProperties[i].ValueCount; j++)
					if (AddedProperties[i].type == ValueType::int32)
						bin.Write(stoi(AddedProperties[i].value[j]));
					else
						bin.Write(stof(AddedProperties[i].value[j]));
				u32 NameOffest = (u32)bin.Position;
				bin.Write(AddedProperties[i].Name, Buffer::BinaryString::NullTerminated);
				while (bin.Position % 4) bin.Write((u8)0);
				u32 entryStart = (u32)(4 + i * 0xC);
				bin.Position = entryStart;
				bin.Write(NameOffest - entryStart);
				bin.Write(DataOffset - entryStart);
				bin.Write(AddedProperties[i].ValueCount);
				bin.Write((u8)AddedProperties[i].type);
				bin.Write((u8)0);
			}
			data = std::move(bin.getBuffer());
		}
	private:
		std::vector<EditableProperty> AddedProperties;
		size_t UnknownPropertiesCount = 0;

		void LoadProperties()
		{
			Buffer dataReader(data);
			dataReader.ByteOrder = Endianness::LittleEndian;
			dataReader.Position = 0;
			u16 Count = dataReader.readUInt16();
			/*u16 Unk1 =*/ dataReader.readUInt16();
			for (int i = 0; i < Count; i++)
			{
				auto EntryOffset = dataReader.Position;
				u32 NameOffset = dataReader.readUInt32();
				u32 DataOffset = dataReader.readUInt32();
				u16 ValueLen = dataReader.readUInt16();
				u8 dataType = dataReader.readUInt8();
				dataReader.readUInt8(); //padding ?

				if (!(dataType == 1 || dataType == 2))
				{
					UnknownPropertiesCount++;
					continue;
				}

				auto pos = dataReader.Position;
				dataReader.Position = EntryOffset + NameOffset;
				std::string propName = dataReader.readStr_NullTerm();
				auto type = (ValueType)dataType;

				dataReader.Position = EntryOffset + DataOffset;
				std::vector<std::string> values;

				for (int j = 0; j < ValueLen; j++)
				{
					if (type == ValueType::int32)
						values.push_back(std::to_string(dataReader.readInt32()));
					else
						values.push_back(std::to_string(dataReader.readFloat()));
				}

				Properties.push_back(EditableProperty
					{
						propName,
						EntryOffset + DataOffset,
						ValueLen,
						type,
						std::move(values)
					});

				dataReader.Position = pos;
			}
		}
	};
}