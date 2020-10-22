using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace SwitchThemes.Common
{
	public class LayoutPatch
	{
		public string PatchName;
		public string AuthorName;
		public string TargetName;
		public string ID;

		public bool UsesOldFixes => ID == null && !Ready8X;

		[Obsolete("This was used to detect whether a layout would need patches to support 8.0+ qlaunch, since version 4.5 prefer the ID value to detect layouts")]
		public bool Ready8X;
		
		[Obsolete("Editing C_W in the usd panes is a better way of achieving this")]
		public bool PatchAppletColorAttrib = false;
		
		public LayoutFilePatch[] Files;
		public AnimFilePatch[] Anims;

		public override string ToString() => PatchName + " by " + AuthorName;

		public bool IsCompatible(SARCExt.SarcData szs)
		{
			for (int i = 0; i < Files.Length; i++)
			{
				if (!szs.Files.ContainsKey(Files[i].FileName)) return false;
				//string TargetFileAsString = ASCIIEncoding.ASCII.GetString(szs.Files[Files[i].FileName]);
				//for (int j = 0; j < Files[i].Patches.Length; j++)
				//	if (!TargetFileAsString.Contains(Files[i].Patches[j].PaneName))
				//		return false;
			}
			return true;
		}

		public byte[] AsByteArray()
		{
			return Encoding.UTF8.GetBytes(AsJson());
		}

		public string AsJson()
		{
			if (ID == null && Ready8X) //Upgrade old layouts using Ready8X to a random ID
				ID = $"updated_{Guid.NewGuid()}";
			Ready8X = false; //Don't include a Ready8x Property
			JsonSerializerSettings settings = new JsonSerializerSettings()
			{
				DefaultValueHandling = DefaultValueHandling.Ignore,
				NullValueHandling = NullValueHandling.Ignore,
#if WIN
				Formatting = Formatting.Indented,
				ReferenceLoopHandling = ReferenceLoopHandling.Ignore
#endif
			};
			return JsonConvert.SerializeObject(this, settings);
		}

#if DEBUG && WIN
		public static void CreateTestTemplates()
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
							}
						}
					}
				}
			};
			System.IO.File.WriteAllText("ExtraLayouts.json", JsonConvert.SerializeObject(p));
		}
#endif
		public static LayoutPatch LoadTemplate(string json) =>
			JsonConvert.DeserializeObject<LayoutPatch>(json);
	}

	public class AnimFilePatch
	{
		public string FileName;
		public string AnimJson;
	}

	public class MaterialPatch
	{
		public string MaterialName;
		public string ForegroundColor = null;
		public string BackgroundColor = null;
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
	}

	public class UsdPatch
	{
		public string PropName;
		public string[] PropValues;
		public int type;
	}
	
	public struct Vector3 
	{
		public float X, Y, Z;

		public Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }

		public void Deconstruct(out float x, out float y, out float z) =>
			(x, y, z) = (X, Y, Z);

		public static implicit operator Vector3((float, float, float) v)
		{
			var (x, y, z) = v;
			return new Vector3(x, y, z);
		}
	}
	
	public struct Vector2
	{
		public float X, Y;
		
		public Vector2(float x, float y) { X = x; Y = y; }

		public void Deconstruct(out float x, out float y) =>
			(x, y) = (X, Y);

		public static implicit operator Vector2((float, float) v)
		{
			var (x, y) = v;
			return new Vector2(x, y);
		}
	}
}
