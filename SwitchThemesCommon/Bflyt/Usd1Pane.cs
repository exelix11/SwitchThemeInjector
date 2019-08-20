using ExtensionMethods;
using Syroot.BinaryData;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Dynamic;
using System.IO;
using System.Text;
using System.Linq;
using System.Collections;
using System.Windows.Forms;
using static SwitchThemes.Common.Bflyt.BflytFile;

namespace SwitchThemes.Common.Bflyt
{
	public class Usd1Pane : BasePane
	{
		public ByteOrder order;

		public override string ToString() => "User data pane";

		[TypeConverter(typeof(ExpandableObjectConverter))]
		public class EditableProperty
		{
			public override string ToString() => Name;

			public enum ValueType : byte
			{
				data = 0,
				int32 = 1,
				single = 2,
				other = 3
			}

			public long ValueOffset;
			public ushort ValueCount;

			public ValueType type { get; set; }
			public string Name { get; set; }
			public string[] value { get; set; }
		}

		List<string> OriginalProperties = new List<string>();

		public List<EditableProperty> Properties { get; set; }
		public EditableProperty FindName(string name) => Properties.Where(x => x.Name == name).FirstOrDefault();

		void LoadProperties()
		{
			Properties = new List<EditableProperty>();
			BinaryDataReader dataReader = new BinaryDataReader(new MemoryStream(data));
			dataReader.ByteOrder = order;
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
				OriginalProperties.Add(propName);

				dataReader.Position = pos;
			}
		}

		public Usd1Pane(BinaryDataReader bin) : base("usd1", bin)
		{
			order = bin.ByteOrder;
			LoadProperties();
		}

		public Usd1Pane(Usd1Pane p) : base("usd1", p.data)
		{
			order = p.order;
			LoadProperties();
			Properties = p.Properties;
		}

		List<EditableProperty> AddedProperties = new List<EditableProperty>();
		void AddNewProperties()
		{
			foreach (var p in Properties)
				if (!OriginalProperties.Contains(p.Name))
				{
					if (p.type != EditableProperty.ValueType.int32 && p.type != EditableProperty.ValueType.single)
						throw new Exception("The only types supported for usd properties are single and int32");
					AddedProperties.Add(p);
				}
			Properties.RemoveAll(x => !OriginalProperties.Contains(x.Name));
			foreach (var s in OriginalProperties)
				if (!Properties.Any(x => x.Name == s))
					throw new Exception("You can't remove existing properties");
		}

		public void ApplyChanges()
		{
			AddNewProperties();

			MemoryStream mem = new MemoryStream();
			BinaryDataWriter bin = new BinaryDataWriter(mem);
			bin.ByteOrder = order;
			bin.Write((ushort)(Properties.Count + AddedProperties.Count));
			bin.Write((ushort)0);
			bin.Write(new byte[0xC * AddedProperties.Count]);
			bin.Write(data, 4, data.Length - 4); //write rest of entries, adding new elements first doesn't break relative offets in the struct
			foreach (var m in Properties)
			{
				if ((byte)m.type != 1 && (byte)m.type != 2) continue;
				m.ValueOffset += + 0xC * AddedProperties.Count;
				bin.Position = m.ValueOffset;
				if (m.value.Length != m.ValueCount) throw new Exception("Can't change the number of values of an usd1 property");
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
				AddedProperties[i].ValueOffset = DataOffset;
				AddedProperties[i].ValueCount = (ushort)AddedProperties[i].value.Length;
				for (int j = 0; j < AddedProperties[i].value.Length; j++)
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
				bin.Write((ushort)AddedProperties[i].ValueCount);
				bin.Write((byte)AddedProperties[i].type);
				bin.Write((byte)0);
				OriginalProperties.Add(AddedProperties[i].Name);
			}
			data = mem.ToArray();

			Properties.AddRange(AddedProperties);
			AddedProperties.Clear();
		}

		public override void WritePane(BinaryDataWriter bin)
		{
			ApplyChanges();
			base.WritePane(bin);
		}

		public override BasePane Clone() => new Usd1Pane(this);
	}
}
