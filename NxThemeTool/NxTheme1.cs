using SARCExt;
using SwitchThemes.Common;
using SwitchThemes.Common.Images;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace NxThemeTool
{
    public class NxTheme1
    {
        public static bool IsNxTheme1(ReadOnlySpan<byte> fileHeader) => ManagedYaz0.IsYaz0(fileHeader.ToArray());
        public static bool IsNxTheme1(byte[] fileHeader) => ManagedYaz0.IsYaz0(fileHeader);

        // This will rewind the stream
        public static bool IsNxTheme1(Stream stream) 
        {
            var bytes = new byte[10];
            stream.Read(bytes, 0, bytes.Length);
            stream.Seek(0, SeekOrigin.Begin);
            return IsNxTheme1(bytes);
        }

        public readonly ThemeFileManifest Manifest;

        public readonly byte[]? MainBgFile;
        public readonly LayoutPatch? MainLayoutFile;
        public readonly LayoutPatch? CommonLayoutFile;

        // Indexed by NxThemeName without file extension
        public readonly Dictionary<string, byte[]> Icons = new();

        public NxTheme1(byte[] data) : this(MakeContentProvider(data)) { }

        // This does not take ownership of the content provider, the caller is responsible for disposing it
        // It's fine if we don't dispose MakeContentProvider since that is a sarc
        public NxTheme1(IContentProvider sarc)
        {
            if (!sarc.HasFile("info.json"))
                throw new ArgumentException("Content provider does not contain a manifest.json file.");

            Manifest = ThemeFileManifest.Deserialize(sarc.GetString("info.json"));
            
            if (sarc.HasFile("image.dds"))
                MainBgFile = sarc.GetFile("image.dds");
            else if (sarc.HasFile("image.jpg"))
                MainBgFile = sarc.GetFile("image.jpg");

            if (sarc.HasFile("layout.json"))
                MainLayoutFile = LayoutPatch.Load(sarc.GetString("layout.json"));

            if (sarc.HasFile("common.json"))
                CommonLayoutFile = LayoutPatch.Load(sarc.GetString("common.json"));

            if (TextureReplacement.NxNameToList.TryGetValue(Manifest.Target, out var replacement))
            {
                foreach (var repl in replacement)
                {
                    if (sarc.HasFile(repl.NxThemeName + ".dds"))
                        Icons.Add(repl.NxThemeName, sarc.GetFile(repl + ".dds"));
                    else if (sarc.HasFile(repl.NxThemeName + ".png"))
                        Icons.Add(repl.NxThemeName, sarc.GetFile(repl + ".png"));
                }
            }
        }

        static SarcContentProvider MakeContentProvider(byte[] data)
        {
            if (!IsNxTheme1(data))
                throw new ArgumentException("Data is not a valid NxTheme1 file.");

            var decompressed = ManagedYaz0.Decompress(data);

            return new SarcContentProvider(SARC.Unpack(decompressed));
        }

        string HashResources() 
        {
            using var sha = SHA1.Create();

            void AddId(string id)
            {
                var idBytes = Encoding.UTF8.GetBytes(id);
                sha.TransformBlock(idBytes, 0, idBytes.Length, null, 0);
            }

            if (MainBgFile != null)
            {
                AddId("<<<<MainBgFile>>>>");
                sha.TransformBlock(MainBgFile, 0, MainBgFile.Length, null, 0);
            }

            if (MainLayoutFile != null)
            {
                AddId("<<<<MainLayoutFile>>>>");
                var layoutBytes = MainLayoutFile.AsByteArray();
                sha.TransformBlock(layoutBytes, 0, layoutBytes.Length, null, 0);
            }

            if (CommonLayoutFile != null)
            {
                AddId("<<<<CommonLayoutFile>>>>");
                var layoutBytes = CommonLayoutFile.AsByteArray();
                sha.TransformBlock(layoutBytes, 0, layoutBytes.Length, null, 0);
            }

            foreach (var icon in Icons)
            {
                AddId($"<<<<icon:{icon.Key}>>>>");
                sha.TransformBlock(icon.Value, 0, icon.Value.Length, null, 0);
            }

            sha.TransformFinalBlock(Array.Empty<byte>(), 0, 0);
            return Convert.ToHexString(sha.Hash!).ToLower();
        }

        public void UnpackToDirectory(string directory)
        {
            using var writer = new DirectoryContentWriter(directory);

            writer.WriteFile("info.json", Encoding.UTF8.GetBytes(Manifest.Serialize(true)));

            if (MainBgFile != null)
            {
                var extension = ImageUtil.DetectImageExtension(MainBgFile);
                writer.WriteFile("image." + extension, MainBgFile);
            }

            if (MainLayoutFile != null)
                writer.WriteFile("layout.json", MainLayoutFile.AsByteArray());

            if (CommonLayoutFile != null)
                writer.WriteFile("common.json", CommonLayoutFile.AsByteArray());

            foreach (var icon in Icons)
            {
                var extension = ImageUtil.DetectImageExtension(icon.Value);
                writer.WriteFile(icon.Key + "." + extension, icon.Value);
            }
        }

        public void ConvertToNxtheme2(IContentWriter writer, ProcessResult validation)
        {
            var theme = new NxTheme2("converted:" + HashResources());

            theme.Manifest.ThemeName = Manifest.ThemeName;
            theme.Manifest.Author = Manifest.Author;

            var part = new NxTheme2.Part(Manifest.Target, new())
            {
                MainImage = MainBgFile,
                LayoutJson = MainLayoutFile?.AsJson()
            };

            foreach (var icon in Icons)
            {
                var extension = ImageUtil.DetectImageExtension(icon.Value);
                part.ExtraImages.Add(icon.Key + "." + extension, icon.Value);
            }

            theme.Parts.Add(part);

            if (CommonLayoutFile is not null && Manifest.Target == CommonInfo.PartHome)
            {
                var commonPart = new NxTheme2.Part(CommonInfo.PartQlaunchCommon, new())
                {
                    LayoutJson = CommonLayoutFile.AsJson()
                };

                theme.Parts.Add(commonPart);
            }

            theme.Validate(validation);
            theme.Pack(writer);
        }
    }
}
