using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Syroot.BinaryData;
using static SwitchThemes.Common.BflytFile;

namespace SwitchThemes.Common
{
	public class Usd1Pane : BasePanel
	{
		public class EditableProperty //Has to be nullable for the differ
		{
			public override string ToString() => Name;

			public enum ValueType : byte
			{
				data = 0,
				int32 = 1,
				single = 2,
				other = 3
			}

			public string Name;
			public long ValueOffset;
			public ushort ValueCount;
			public ValueType type;

			public string[] value;

			/*
			 public string[] Value
			{
				get
				{
					if (type != ValueType.int32 && type != ValueType.single) return null;
					string[] res = new string[ValueCount];
					BinaryDataReader bin = new BinaryDataReader(new MemoryStream(Data));
					if (type == ValueType.int32)
						for (int i = 0; i < ValueCount; i++) res[i] = bin.ReadInt32().ToString();
					else
						for (int i = 0; i < ValueCount; i++) res[i] = bin.ReadSingle().ToString();
					return res;
				}
				set
				{
					if (type != ValueType.int32 && type != ValueType.single) throw new Exception("Value type not supported");
					MemoryStream mem = new MemoryStream(); BinaryDataWriter bin = new BinaryDataWriter(mem);
					ValueCount = (ushort)value.Length;
					if (type == ValueType.int32)
						for (int i = 0; i < ValueCount; i++) bin.Write(int.Parse(value[i]));
					else
						for (int i = 0; i < ValueCount; i++) bin.Write(float.Parse(value[i]));
					Data = mem.ToArray();
				}
			 */
		}
		
		public List<EditableProperty> Properties = new List<EditableProperty>();

		public EditableProperty FindName(string name) => Properties.Where(x => x.Name == name).FirstOrDefault();

		 List<EditableProperty> AddedProperties = new List<EditableProperty>();
		public void AddNewProperty(string name, string[] value, EditableProperty.ValueType type)
		{
			AddedProperties.Add(new EditableProperty { Name = name, ValueCount = (ushort)value.Length, type = type, value = value });
		}

		void LoadProperties()
		{
			BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
			dataReader.ByteOrder = ByteOrder.LittleEndian;
			dataReader.Position = 0;
			ushort Count = dataReader.ReadUInt16();
			ushort Unk1 = dataReader.ReadUInt16();
			for (int i = 0; i < Count; i++)
			{
				var EntryOffset = dataReader.Position;
				uint NameOffset = dataReader.ReadUInt32();
				uint DataOffset = dataReader.ReadUInt32();
				ushort ValueLen = dataReader.ReadUInt16();
				byte dataType = dataReader.ReadByte();
				dataReader.ReadByte(); //padding ?

				if (!(dataType == 1 || dataType == 2))
					continue;

				var pos = dataReader.Position;
				dataReader.Position = EntryOffset + NameOffset;
				string propName = dataReader.ReadString(BinaryStringFormat.ZeroTerminated);
				var type = (EditableProperty.ValueType)dataType;

				dataReader.Position = EntryOffset + DataOffset;
				string[] values = new string[ValueLen];

				for (int j = 0; j < ValueLen; j++)
					if (type == EditableProperty.ValueType.int32)
						values[j] = dataReader.ReadInt32().ToString();
					else
						values[j] = dataReader.ReadSingle().ToString();

				Properties.Add(new EditableProperty()
				{
					Name = propName,
					type = type,
					ValueOffset = EntryOffset + DataOffset,
					ValueCount = ValueLen,
					value = values
				});

				dataReader.Position = pos;
			}
		}

		public Usd1Pane(BinaryDataReader bin) : base("usd1", bin)
		{
			LoadProperties();
		}

		public Usd1Pane(BasePanel p) : base(p)
		{
			LoadProperties();
		}

		void ApplyChanges()
		{
			MemoryStream mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem);
			bin.Write((ushort)(Properties.Count + AddedProperties.Count));
			bin.Write((ushort)0);
			bin.Write(new byte[0xC * AddedProperties.Count]);
			bin.Write(data,4,data.Length - 4); //write rest of entries, adding new elements first doesn't break relative offets in the struct
			foreach (var m in Properties)
			{
				if ((byte)m.type != 1 && (byte)m.type != 2) continue;
				bin.Position = m.ValueOffset + 0xC * AddedProperties.Count;
				for (int i = 0; i < m.ValueCount; i++)
					if (m.type == EditableProperty.ValueType.int32)
						bin.Write(int.Parse(m.value[i]));
					else
						bin.Write(float.Parse(m.value[i]));
			}
			for (int i = 0; i < AddedProperties.Count; i++)
			{
				bin.Position = bin.BaseStream.Length;
				uint DataOffset = (uint)bin.BaseStream.Position;
				for (int j = 0; j < AddedProperties[i].ValueCount; j++)
					if (AddedProperties[i].type == EditableProperty.ValueType.int32)
						bin.Write(int.Parse(AddedProperties[i].value[j]));
					else
						bin.Write(float.Parse(AddedProperties[i].value[j]));
				uint NameOffest = (uint)bin.BaseStream.Position;
				bin.Write(AddedProperties[i].Name, BinaryStringFormat.ZeroTerminated);
				bin.Align(4);
				uint entryStart = (uint)(4 + i * 0xC);
				bin.BaseStream.Position = entryStart;
				bin.Write(NameOffest - entryStart);
				bin.Write(DataOffset - entryStart);
				bin.Write(AddedProperties[i].ValueCount);
				bin.Write((byte)AddedProperties[i].type);
				bin.Write((byte)0);
			}
			data = mem.ToArray();
		}

		public override void WritePanel(BinaryDataWriter bin)
		{
			ApplyChanges();
			base.WritePanel(bin);
		}
	}
}
