﻿using System;
using System.Collections.Generic;
using System.Text;
using Newtonsoft.Json;

namespace SwitchThemes.Common
{
	public class LayoutPatch
	{
		public string PatchName;
		public string AuthorName;
		public string TargetName;
		public string ID;
		/*	11.0 added a new home menu button for online services, to show it it uses an animation that moves around all the applet buttons
			if this flag is set to true or null the installer will automatically remove that animation emulating the layout of pre 11.0 qlaunch */
		public bool? HideOnlineBtn;

        // This is a value from the ConsoleFirmware enum
        // This is auto-generated based on the detected szs firmware when producing a layout patch and used by the installer to detect if the layout needs some compatibility patches for newer firmwares
        // When this is not specified we assume Fw11_0 which is the common firmware compatibility level for all layouts at the time this feature was introduced
        // When this is specified with a value not known by the injector/installer the value is treated as-is so if it's a value lower than (int)ConsoleFirmware.Fw11_0 it will receive compatibility fixes that apply to <= ConsoleFirmware.Fw11_0. If it's a value higher than the highest known firmware in ConsoleFirmware it will not receive fixes in this version of the installer/injector.
		// This behavior is intended to avoid newer themes getting wrong fixes on older installer versions.
        public int TargetFirmware = (int)ConsoleFirmware.Fw11_0;

		[JsonIgnore]
		public ConsoleFirmware TargetFirmwareValue
		{
			get => (ConsoleFirmware)TargetFirmware;
			set => TargetFirmware = (int)value;
		}

        [JsonIgnore]
		public bool UsesOldFixes => ID == null && !Ready8X;

		[Obsolete("This was used to detect whether a layout would need patches to support 8.0+ qlaunch, since version 4.5 prefer the ID value to detect layouts")]
		public bool Ready8X;
		
		[Obsolete("Editing C_W in the usd panes is a better way of achieving this")]
		public bool PatchAppletColorAttrib = false;
		
		public LayoutFilePatch[] Files;
		public AnimFilePatch[] Anims;

		public override string ToString() => PatchName + " by " + AuthorName;

		// Note: this is just used as a quick filter and does not guarantee compatibility
		public bool IsCompatible(SARCExt.SarcData szs)
		{
			for (int i = 0; i < Files.Length; i++)
			{
				if (!szs.Files.ContainsKey(Files[i].FileName)) 
					return false;
			}
			return true;
		}

		public byte[] AsByteArray()
		{
			return Encoding.UTF8.GetBytes(AsJson(Formatting.None));
		}

		public string AsJson(Formatting format = Formatting.Indented)
		{
			if (ID == null && Ready8X) //Upgrade old layouts using Ready8X to a random ID
				ID = $"updated_{Guid.NewGuid()}";
			Ready8X = false; //Don't include a Ready8x Property
			JsonSerializerSettings settings = new JsonSerializerSettings()
			{
				DefaultValueHandling = DefaultValueHandling.Ignore,
				NullValueHandling = NullValueHandling.Ignore,
				Formatting = format,
			};
			return JsonConvert.SerializeObject(this, settings);
		}

#if DEBUG
		public static string CreateTestPatches()
		{
			var p = new LayoutPatch()
			{
				PatchName = "test patch",
				AuthorName = "exelix",
				Files = new LayoutFilePatch[]
				{
					new LayoutFilePatch()
					{
						FileName = "blyt/RdtBase.bflyt",
						Patches = new PanePatch[]
						{
							new PanePatch()
							{
								PaneName = "L_BtnNoti",
								Position = new Vector3{ Y= -500 },
								Scale = new Vector2{ X = 10}
							}
						}
					}
				}
			};
			return p.AsJson();
		}
#endif
		public static LayoutPatch Load(string json) =>
			JsonConvert.DeserializeObject<LayoutPatch>(json);
	}

	public class AnimFilePatch
	{
		public string FileName;
		public string AnimJson;
	}

	public class MaterialPatch
	{
		// Texture related fields are identified by texture name rather than index
		public struct TexReference
		{
			public string Name;
			public byte? WrapS, WrapT;
		}

		// I assume transforms are per-texture in the same order as references in the material, see the CheckMaterialTexturesAssumption test
		// Not sure about this as the material has two separate counts for references and transforms
		public struct TexTransform
		{
			public string Name; 
			public float? X;
			public float? Y;
			public float? Rotation;
			public float? ScaleX;
			public float? ScaleY;
		}

		public string MaterialName;
		public string ForegroundColor = null;
		public string BackgroundColor = null;
		
		public TexReference[] Refs = null;
		public TexTransform[] Transforms = null;

        public override string ToString() => 
			$"Material patch for {MaterialName}";

        public bool IsEmpty() 
		{
			if (ForegroundColor != null || BackgroundColor != null)
				return false;

			if (Refs != null && Refs.Length > 0)
				return false;

			if (Transforms != null && Transforms.Length > 0)
				return false;

			return true;
		}
	}

	public class LayoutFilePatch
	{
		public string FileName;
		public PanePatch[] Patches;
		public ExtraGroup[] AddGroups = null;
		public MaterialPatch[] Materials = null;
		//Note: these properties change the panes order and are not used by the differ, they must be manually set in a json
		public string[] PushBackPanes = null;
		public string[] PullFrontPanes = null;
	}

	public class ExtraGroup
	{
		public string GroupName;
		public string[] Panes;

        public override string ToString() =>
            $"Extra group patch for {GroupName}";
    }

	public class PanePatch
	{
		public string PaneName;
		public Vector3? Position = null;
		public Vector3? Rotation = null;
		public Vector2? Scale = null;
		public Vector2? Size = null;
		public bool? Visible = null;

		public List<UsdPatch> UsdPatches = null;

		public byte? OriginX;
		public byte? OriginY;
		public byte? ParentOriginX;
		public byte? ParentOriginY;

        //These fields are used to store extra data according to the pane type.
        [JsonIgnore]
		private string[] PaneSpecific = new string[4];

		//For compatibility reasons these fields have the old pic1-specific name in layouts 

		/// <summary>
		/// PIC1 : Top left color <br/>
		/// TXT1 : Top font color
		/// </summary>
		[JsonProperty("ColorTL")]
		public string PaneSpecific0 { get => PaneSpecific[0]; set => PaneSpecific[0] = value; }

		/// <summary>
		/// PIC1 : Top right color <br/>
		/// TXT1 : Top shadow color
		/// </summary>
		[JsonProperty("ColorTR")]
		public string PaneSpecific1 { get => PaneSpecific[1]; set => PaneSpecific[1] = value; }

		/// <summary>
		/// PIC1 : Bottom left color <br/>
		/// TXT1 : Bottom font color
		/// </summary>
		[JsonProperty("ColorBL")]
		public string PaneSpecific2 { get => PaneSpecific[2]; set => PaneSpecific[2] = value; }

		/// <summary>
		/// PIC1 : Bottom right color <br/>
		/// TXT1 : Bottom shadow color
		/// </summary>
		[JsonProperty("ColorBR")]
		public string PaneSpecific3 { get => PaneSpecific[3]; set => PaneSpecific[3] = value; }

        public override string ToString() => $"Pane patch for {PaneName}";
    }

	public class UsdPatch
	{
		public string PropName;
		public string[] PropValues;
		public int type;
	}
	
	[JsonObject]
	public struct Vector3 : IEquatable<Vector3>
	{
		public float X, Y, Z;
		#region AutoGenerated
		public Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }

		public void Deconstruct(out float x, out float y, out float z) =>
			(x, y, z) = (X, Y, Z);

		public override bool Equals(object obj)
		{
			return obj is Vector3 vector && Equals(vector);
		}

		public bool Equals(Vector3 other)
		{
			return X == other.X &&
				   Y == other.Y &&
				   Z == other.Z;
		}

		public override int GetHashCode()
		{
			int hashCode = -307843816;
			hashCode = hashCode * -1521134295 + X.GetHashCode();
			hashCode = hashCode * -1521134295 + Y.GetHashCode();
			hashCode = hashCode * -1521134295 + Z.GetHashCode();
			return hashCode;
		}

		public static bool operator ==(Vector3 left, Vector3 right)
		{
			return left.Equals(right);
		}

		public static bool operator !=(Vector3 left, Vector3 right)
		{
			return !(left == right);
		}

		public static implicit operator Vector3((float, float, float) v)
		{
			var (x, y, z) = v;
			return new Vector3(x, y, z);
		}
		#endregion
	}

	[JsonObject]
	public struct Vector2 : IEquatable<Vector2>
	{
		public float X, Y;
		#region AutoGenerated
		public Vector2(float x, float y) { X = x; Y = y; }

		public void Deconstruct(out float x, out float y) =>
			(x, y) = (X, Y);

		public override bool Equals(object obj)
		{
			return obj is Vector2 vector && Equals(vector);
		}

		public bool Equals(Vector2 other)
		{
			return X == other.X &&
				   Y == other.Y;
		}

		public override int GetHashCode()
		{
			int hashCode = 1861411795;
			hashCode = hashCode * -1521134295 + X.GetHashCode();
			hashCode = hashCode * -1521134295 + Y.GetHashCode();
			return hashCode;
		}

		public static bool operator ==(Vector2 left, Vector2 right)
		{
			return left.Equals(right);
		}

		public static bool operator !=(Vector2 left, Vector2 right)
		{
			return !(left == right);
		}

		public static implicit operator Vector2((float, float) v)
		{
			var (x, y) = v;
			return new Vector2(x, y);
		}
		#endregion
	}
}
