using SwitchThemes.Common;
using SwitchThemes.Common.Images;
using System.Text.Json;

namespace NxThemeTool.Nxtheme2
{
    public class NxTheme2Manifest
    {
        public string ThemeName { get; set; } = "New theme";
        public string Author { get; set; } = "Your name here";

        public int FormatVersion { get; set; }
        public string? Id { get; set; }

        public string Serialize() => JsonSerializer.Serialize(this, new JsonSerializerOptions() { WriteIndented = true });
        public static NxTheme2Manifest Deserialize(string json) => JsonSerializer.Deserialize<NxTheme2Manifest>(json) ?? throw new JsonException("Failed to deserialize NxTheme2Manifest.");
    }

    public class NxTheme2
    {
        public record class Part(
            string PartName,
            Dictionary<string, byte[]> ExtraImages)
        {
            public string? LayoutJson { get; set; }
            public byte[]? MainImage { get; set; }

            public bool IsEmpty => LayoutJson == null && MainImage == null && ExtraImages.Count == 0;
            public bool HasMainImage => MainImage != null;
            public bool HasExtraImages => ExtraImages.Count > 0;
            public bool HasLayout => LayoutJson != null;

            public Part(string partName) : this(partName, new()) { }
        }

        public readonly List<Part> Parts = new();
        public readonly NxTheme2Manifest Manifest;

        public NxTheme2()
        {
            Manifest = new NxTheme2Manifest()
            {
                FormatVersion = CommonInfo.NxTheme2FormatVersion,
                Id = Guid.NewGuid().ToString()
            };
        }

        public NxTheme2(IContentProvider provider, ProcessResult? validation)
        {
            Manifest = provider.GetJson<NxTheme2Manifest>("manifest.json");

            foreach (var entry in provider.GetFiles())
            {
                if (entry == "manifest.json")
                    continue;

                var fileName = entry.ToLower();

                if (fileName != entry)
                    validation?.Warn(entry, "File names must be lowercase");

                var parts = fileName.Split('/');
                if (parts.Length != 2)
                {
                    validation?.Err(entry, "Invalid file name");
                    continue;
                }

                var part = GetOrAddPart(parts[0]);
                if (parts[1] == "layout.json")
                    part.LayoutJson = provider.GetString(entry);
                else if (parts[1] == "image.jpg" || parts[1] == "image.png" || parts[1] == "image.dds")
                {
                    if (part.MainImage != null)
                        validation?.Err(entry, "Multiple main images found for part '" + part.PartName + "'");

                    part.MainImage = provider.GetFile(entry);
                }
                else
                {
                    if (part.ExtraImages.ContainsKey(parts[1]))
                        validation?.Err(entry, "Multiple extra images with the same name found for part '" + part.PartName + "': " + parts[1]);

                    part.ExtraImages[parts[1]] = provider.GetFile(entry);
                }
            }

            // If construction went well, perform additional validation
            if (validation is not null)
                Validate(validation);
        }

        Part GetOrAddPart(string partName)
        {
            var part = Parts.FirstOrDefault(p => p.PartName == partName);
            if (part == null)
            {
                part = new Part(partName, new());
                Parts.Add(part);
            }
            return part;
        }

        public void Validate(ProcessResult validation)
        {
            // Manifest checks
            if (string.IsNullOrWhiteSpace(Manifest.ThemeName))
                validation.Err("manifest.json", "Theme name cannot be empty.");

            if (string.IsNullOrWhiteSpace(Manifest.Author))
                validation.Warn("manifest.json", "Consider setting the theme author.");

            if (string.IsNullOrWhiteSpace(Manifest.Id))
                validation.Warn("manifest.json", "Consider setting setting a unique ID for your theme.");

            if (Manifest.FormatVersion < CommonInfo.NxThemeFormatVersion)
                validation.Err("manifest.json", $"Invalid format version. Versions below {CommonInfo.NxThemeFormatVersion} are only valid for the old nxtheme format");

            if (Manifest.FormatVersion > CommonInfo.NxTheme2FormatVersion)
                validation.Err("manifest.json", $"The theme format version is newer than the one this program supports. The output might be wrong. This build supports up to {CommonInfo.NxTheme2FormatVersion}");

            // Ensure all parts are valid and non empty
            if (Parts.Count == 0)
                validation.Err("", "The theme is empty.");

            foreach (var part in Parts)
            {
                if (part.IsEmpty)
                    validation.Warn(part.PartName, "Part is empty. Each part must have at least a layout or a main image.");

                var target = CommonInfo.GetPart(part.PartName);
                if (target is null)
                {
                    validation.Err(part.PartName, "Unknown part name. This part will be ignored. Make sure the part name is correct and matches one of the parts defined in the documentation.");
                    continue;
                }

                if (part.HasLayout)
                {
                    if (!target.AllowLayout)
                        validation.Err(part.PartName + "/layout.json", "This theme part does not support custom layouts");

                    try
                    {
                        _ = JsonDocument.Parse(part.LayoutJson!);
                    }
                    catch (JsonException ex)
                    {
                        validation.Err(part.PartName + "/layout.json", "Invalid JSON: " + ex.Message);
                    }
                }

                if (part.HasMainImage)
                {
                    if (!target.AllowImages)
                        validation.Err(part.PartName, "This theme part does not support custom images");

                    try
                    {
                        var image = ImageUtil.ParseImage(part.MainImage!);

                        if (image is JpgInfo { IsProgressive: true })
                            validation.Err(part.PartName, $"The provided JPG image uses progressive encoding. This is not support and will fail to install.");

                        if (image is DDS dds && !dds.IsValidForBg)
                            validation.Err(part.PartName, $"The provided DDS image can't be used for wallpapers. Only DXT1 encoded images are valid.");

                        if (image is PngInfo)
                            validation.Err(part.PartName, $"Png images are not supported for wallpapers.");

                        if (image.Size.Width != 1280 || image.Size.Height != 720)
                            validation.Err(part.PartName, $"Invalid main image size {image.Size}. The main image must be 1280x720 pixels.");
                    }
                    catch (Exception ex)
                    {
                        validation.Err(part.PartName, "Invalid main image format. Supported formats are: jpg, dds. Error:" + ex.Message);
                        return;
                    }
                }

                HashSet<string> uniqueNames = new();
                foreach (var image in part.ExtraImages)
                {
                    if (!target.AllowImages)
                        validation.Err(part.PartName, "This theme part does not support custom images");

                    var withoutExtension = Path.GetFileNameWithoutExtension(image.Key);
                    if (!uniqueNames.Add(withoutExtension))
                        validation.Err(part.PartName + "/" + image.Key, $"Duplicate extra image name. Each extra image must have a unique name.");

                    if (!TextureReplacement.NxNameToList.TryGetValue(part.PartName, out var validTextures))
                    {
                        validation.Err(part.PartName + "/" + image.Key, $"This texture is not supported in the {part.PartName} theme part.");
                        continue;
                    }

                    var tex = validTextures.FirstOrDefault(t => t.NxThemeName == withoutExtension);
                    if (tex == null)
                    {
                        validation.Err(part.PartName + "/" + image.Key, $"This texture is not supported in the {part.PartName} theme part.");
                        continue;
                    }

                    try
                    {
                        var imageInfo = ImageUtil.ParseImage(image.Value);
                        
                        if (imageInfo is JpgInfo)
                            validation.Err(part.PartName + "/" + image.Key, $"JPG images are not allowed for icons.");

                        if (imageInfo is DDS dds && !dds.IsValidForIcons)
                            validation.Err(part.PartName + "/" + image.Key, $"The provided DDS image can't be used for icons. Try a different encoding.");

                        if (imageInfo.Size.Width != tex.W || imageInfo.Size.Height != tex.H)
                            validation.Err(part.PartName + "/" + image.Key, $"Invalid extra image size {imageInfo.Size}. The image must be {tex.W}x{tex.H} pixels.");
                    }
                    catch (Exception ex)
                    {
                        validation.Err(part.PartName + "/" + image.Key, "Invalid extra image format. Supported formats are: jpg, png, dds. Error:" + ex.Message);
                    }
                }
            }
        }

        public void Pack(IContentWriter writer)
        {
            writer.WriteString("manifest.json", Manifest.Serialize());

            foreach (var part in Parts)
            {
                if (part.IsEmpty)
                    continue;

                var partFolder = part.PartName + "/";

                if (part.HasLayout)
                    writer.WriteString(partFolder + "layout.json", part.LayoutJson!);

                if (part.HasMainImage)
                {
                    var format = ImageUtil.DetectImageExtension(part.MainImage!);
                    writer.WriteFile(partFolder + "image." + format, part.MainImage!);
                }

                foreach (var image in part.ExtraImages)
                    writer.WriteFile(partFolder + image.Key, image.Value);
            }
        }
    }
}
