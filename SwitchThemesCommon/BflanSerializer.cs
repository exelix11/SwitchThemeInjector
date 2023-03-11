using Newtonsoft.Json;
using SwitchThemes.Common.Bflan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using static SwitchThemes.Common.Bflan.Pai1Section;

namespace SwitchThemes.Common.Serializers
{
	public class BflanSerializer
	{
		public bool LittleEndian;
		public uint Version;

		//Doesn't seem to contain other sections
		public Pat1Serializer pat1;
		public Pai1Serializer pai1;

		public static string ToJson(BflanFile file)
		{
			JsonSerializerSettings settings = new JsonSerializerSettings()
			{
#if WIN
				Formatting = Formatting.None,
				ReferenceLoopHandling = ReferenceLoopHandling.Ignore
#endif
			};
			return JsonConvert.SerializeObject(BflanSerializer.Serialize(file), settings);
		}

		public static BflanFile FromJson(string json) =>
			JsonConvert.DeserializeObject<BflanSerializer>(json).Deserialize();

		public static BflanSerializer Serialize(BflanFile file)
		{
			BflanSerializer res = new BflanSerializer()
			{
				LittleEndian = file.byteOrder == Syroot.BinaryData.ByteOrder.LittleEndian,
				Version = file.Version
			};

			Pat1Section pat1 = file.patData;
			res.pat1 = new Pat1Serializer()
			{
				AnimationOrder = pat1.AnimationOrder,
				ChildBinding = pat1.ChildBinding,
				Groups = pat1.Groups,
				Name = pat1.Name,
				Unk_EndOfFile = pat1.Unk_EndOfFile,
				Unk_StartOfFile = pat1.Unk_StartOfFile,
				Unk_EndOfHeader = pat1.Unk_EndOfHeader
			};

			res.pai1 = Pai1Serializer.Serialize(file.paiData);

			return res;
		}

		public BflanFile Deserialize()
		{
			BflanFile res = new BflanFile();
			res.byteOrder = LittleEndian ? Syroot.BinaryData.ByteOrder.LittleEndian : Syroot.BinaryData.ByteOrder.BigEndian;
			res.Version = Version;

			Pat1Section _pat1 = new Pat1Section()
			{
				AnimationOrder = pat1.AnimationOrder,
				ChildBinding = pat1.ChildBinding,
				Groups = pat1.Groups,
				Name = pat1.Name,
				Unk_EndOfFile = pat1.Unk_EndOfFile,
				Unk_StartOfFile = pat1.Unk_StartOfFile,
				Unk_EndOfHeader = pat1.Unk_EndOfHeader
			};
			res.Sections.Add(_pat1);
			res.Sections.Add(pai1.Deserialize());
			
			return res;
		}
	}

	public class Pat1Serializer
	{
		public ushort AnimationOrder;
		public string Name;
		public byte ChildBinding;
		public string[] Groups;

		public UInt16 Unk_StartOfFile;
		public UInt16 Unk_EndOfFile;
		public byte[] Unk_EndOfHeader;
	}

	public class Pai1Serializer
	{
		public UInt16 FrameSize;
		public byte Flags;
		public string[] Textures;
		public List<PaiEntrySerializer> Entries;

		public static Pai1Serializer Serialize(Pai1Section p)
		{
			var res = new Pai1Serializer()
			{
				Textures = p.Textures,
				Flags = p.Flags,
				FrameSize = p.FrameSize
			};
			res.Entries = new List<PaiEntrySerializer>();
			foreach (var e in p.Entries)
				res.Entries.Add(PaiEntrySerializer.Serialize(e));
			return res;
		}

		public Pai1Section Deserialize()
		{
			var res = new Pai1Section()
			{
				Textures = Textures,
				Flags = Flags,
				FrameSize = FrameSize
			};
			foreach (var e in Entries)
				res.Entries.Add(e.Deserialize());
			return res;
		}
	}

	public class PaiEntrySerializer
	{
		public string Name; 
		public byte Target;
		public List<PaiTagSerializer> Tags;
		public byte[] UnkwnownData;

		public PaiEntry Deserialize()
		{
			PaiEntry res = new PaiEntry()
			{
				Name = Name,
				Target = (PaiEntry.AnimationTarget)Target,
				UnkwnownData = UnkwnownData
			};
			foreach (var t in Tags)
				res.Tags.Add(t.Deserialize());
			return res;
		}

		public static PaiEntrySerializer Serialize(PaiEntry e)
		{
			var res = new PaiEntrySerializer()
			{
				Name = e.Name,
				Target = (byte)e.Target,
				UnkwnownData = e.UnkwnownData
			};
			res.Tags = new List<PaiTagSerializer>();
			foreach (var t in e.Tags)
				res.Tags.Add(PaiTagSerializer.Serialize(t));
			return res;
		}
	}

	public class PaiTagSerializer
	{
		public uint Unknown;
		public string TagType;
		public List<PaiTagEntrySerializer> Entries;

		public PaiTag Deserialize()
		{
			var res = new PaiTag()
			{
				Unknown = Unknown,
				TagType = TagType
			};
			foreach (var e in Entries)
				res.Entries.Add(e.Deserialize());
			return res;
		}

		public static PaiTagSerializer Serialize(PaiTag tag)
		{
			var res = new PaiTagSerializer()
			{
				Unknown = tag.Unknown,
				TagType = tag.TagType
			};

			res.Entries = new List<PaiTagEntrySerializer>();
			foreach (var e in tag.Entries)
				res.Entries.Add(PaiTagEntrySerializer.Serialize(e));
			return res;
		}
	}

	public class PaiTagEntrySerializer
	{
		public byte Index;
		public byte AnimationTarget;
		public UInt16 DataType;
		public List<KeyFrameSerializer> KeyFrames;

		public uint FLEUUnknownInt;
		public string FLEUEntryName;

		public PaiTagEntry Deserialize()
		{
			var res = new PaiTagEntry()
			{
				Index = Index,
				AnimationTarget = AnimationTarget,
				DataType = DataType,
				FLEUEntryName = FLEUEntryName,
				FLEUUnknownInt = FLEUUnknownInt
			};

            res.KeyFrames.AddRange(KeyFrames.Select(x => x.Deserialize()));

            return res;
		}

		public static PaiTagEntrySerializer Serialize(PaiTagEntry entry)
		{
			var res = new PaiTagEntrySerializer()
			{
				Index = entry.Index,
				AnimationTarget = entry.AnimationTarget,
				DataType = entry.DataType,
				FLEUEntryName = entry.FLEUEntryName,
				FLEUUnknownInt = entry.FLEUUnknownInt
			};

			res.KeyFrames = new List<KeyFrameSerializer>();
			foreach (var k in entry.KeyFrames)
				res.KeyFrames.Add(new KeyFrameSerializer() { Blend = k.Blend, Frame = k.Frame, Value = k.Value });

			return res;
		}
	}

	public class KeyFrameSerializer
	{
		public float Frame;
		public float Value;
		public float Blend;

		public KeyFrame Deserialize()
		{
            return new KeyFrame() { Frame = Frame, Value = Value, Blend = Blend };
        }
	}
}
