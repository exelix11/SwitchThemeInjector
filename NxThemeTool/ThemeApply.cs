using SARCExt;
using SwitchThemes.Common;
using SwitchThemes.Common.Images;
using SwitchThemes.Common.Patching;

namespace NxThemeTool
{
    public class ThemeApply(NxTheme2 Theme, IContentProvider Szs) : IDisposable
    {
        readonly NxTheme2 Theme = Theme;
        readonly IContentProvider Szs = Szs;

        public LayoutCompatibilityOption Compatibility = LayoutCompatibilityOption.Default;

        public static ThemeApply FromFiles(string nxtheme, string szs, ProcessResult result)
        {
            // Nxtheme v1 compatibility check
            if (File.Exists(nxtheme))
            {
                using var stream = File.OpenRead(nxtheme);
                if (NxTheme1.IsNxTheme1(stream)) 
                {
                    stream.Dispose();

                    result.Warn(nxtheme, "This file uses the previous nxtheme format version, it has been implicitly converted");

                    var theme = new NxTheme1(File.ReadAllBytes(nxtheme));
                    using var theme2 = new InMemoryFileProvider();

                    theme.ConvertToNxtheme2(theme2, result);

                    return new ThemeApply(new NxTheme2(theme2, result), ProviderHelper.OpenFor(szs));
                }
            }

            using var source = ProviderHelper.OpenFor(nxtheme);
            return new ThemeApply(new NxTheme2(source, result), ProviderHelper.OpenFor(szs));
        }

        public void Apply(IContentWriter writer, ProcessResult result) 
        {
            foreach (var part in Theme.Parts)
            {
                var info = CommonInfo.GetPart(part.PartName);
                if (info == null)
                {
                    result.Err("ApplyPart", $"Part {part.PartName} is not recognized and will be skipped.");
                    return;
                }

                var path = $"{info.TitleId}/{info.SzsName}";
                try 
                {
                    var szs = SARC.Unpack(ManagedYaz0.Decompress(Szs.GetFile(path)));
                    ApplyPart(info, part, szs, result);
                    writer.WriteFile(path, ManagedYaz0.Compress(SARC.Pack(szs).Item2));
                }
                catch (Exception ex)
                {
                    result.Err(part.PartName, $"An error occurred while applying part: {ex.Message}");
                    continue;
                }
            }
        }

        void ApplyPart(PatchPartInfo info, NxTheme2.Part part, SarcData szs, ProcessResult result)
        {
            var patcher = new SzsPatcher(szs)
            {
                CompatFixes = Compatibility
            };

            if (part.HasMainImage && info.AllowImages)
            {
                var imageFormat = ImageUtil.DetectFormat(part.MainImage!);

                if (imageFormat != ImageFormat.Dds)
                {
                    result.Err(part.PartName, "This tool can only apply DDS images");
                }
                else
                {
                    if (!patcher.PatchMainBG(part.MainImage!))
                        result.Err(part.PartName, "Failed to patch main background image.");
                }
            }

            if (part.HasExtraImages && info.AllowImages)
            {
                if (TextureReplacement.NxNameToList.TryGetValue(part.PartName, out var textures))
                {
                    foreach (var texture in textures)
                    {
                        var texName = texture.NxThemeName;

                        if (part.ExtraImages.ContainsKey(texName + ".dds"))
                            patcher.PatchAppletIcon2(part.ExtraImages[texName + ".dds"], texture);
                        else if (part.ExtraImages.ContainsKey(texName + ".png"))
                        {
                            result.Err(part.PartName + "/" + texName, "This tool can apply only DDS images");
                        }
                    }                    
                }
            }

            if (part.HasLayout && info.AllowLayout)
            {
                patcher.PatchLayouts(LayoutPatch.Load(part.LayoutJson!));
            }
        }

        public void Dispose()
        {
            Szs.Dispose();
        }
    }
}
