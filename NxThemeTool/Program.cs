using NxThemeTool;
using NxThemeTool.Nxtheme2;
using SARCExt;
using SwitchThemes.Common;

Console.WriteLine("NxThemeTool - https://github.com/exelix11/SwitchThemeInjector");
Console.WriteLine();

if (args.Length == 0 || args.Any(x => x == "help" || x == "-h" || x == "--help" || x == "-help"))
{
    PrintHelp();
    return 0;
}

if (args[0] == "new")
{
    var theme = new NxTheme2();
    foreach (var partInfo in CommonInfo.Parts)
    {
        if (!partInfo.AllowImages)
            continue;

        var part = new NxTheme2.Part(partInfo.Name);
        part.MainImage = Util.CreateEmpty720PJPG();
        theme.Parts.Add(part);
    }

    using var writer = new DirectoryContentWriter(args[1]);
    theme.Pack(writer);
}
else if (args[0] == "validate")
{
    IContentProvider provider;
    if (Directory.Exists(args[1]))
        provider = new DirectoryContentProvider(args[1]);
    else if (File.Exists(args[1]))
    {
        using var stream = File.OpenRead(args[1]);
        if (stream.Length < 10)
        {
            Console.WriteLine("File is too small to be a valid nxtheme.");
            return 1;
        }

        if (ManagedYaz0.IsYaz0(PeekFormat(stream)))
        {
            Console.WriteLine("This file is an old-style nxtheme and it is not supported for validation");
            return 1;
        }

        provider = new ZipContentProvider(stream);
    }
    else
    {
        Console.WriteLine("Target file or directory does not exist.");
        return 1;
    }

    var validation = new ProcessResult();
    _ = new NxTheme2(provider, validation);
    provider.Dispose();

    PrintValidation(validation);
}
else if (args[0] == "pack")
{
    if (args.Length < 3)
    {
        Console.WriteLine("Not enough arguments for packing.");
        return 1;
    }

    if (!Directory.Exists(args[1]))
    {
        Console.WriteLine("Target directory does not exist.");
        return 1;
    }

    using var provider = new DirectoryContentProvider(args[1]);
    using var writer = new ZipContentWriter(new FileStream(args[2], FileMode.Create, FileAccess.Write));
    var validation = new ProcessResult();

    var theme = new NxTheme2(provider, validation);
    PrintValidation(validation);

    theme.Pack(writer);
}
else if (args[0] == "apply")
{
    if (args.Length < 4)
    {
        Console.WriteLine("Not enough arguments.");
        return 1;
    }

    var source = args[1];
    var szs = args[2];
    var output = args[3];

    var result = new ProcessResult();

    using var patcher = ThemeApply.FromFiles(source, szs, result);
    using var outputFolder = new DirectoryContentWriter(output);
    
    patcher.Apply(outputFolder, result);

    PrintValidation(result);
}
else if (args[0] == "unpack" || File.Exists(args[0]))
{
    if (args[0] == "unpack" && args.Length < 3)
    {
        Console.WriteLine("Not enough arguments for unpacking.");
        return 1;
    }

    var source = args[0] == "unpack" ? args[1] : args[0];
    var dest = args[0] == "unpack" ? args[2] : Path.GetFileNameWithoutExtension(args[0]) + "_unpacked";

    using var sourceStream = File.OpenRead(source);
    if (ManagedYaz0.IsYaz0(PeekFormat(sourceStream)))
    {
        // Compatibility with nxtheme format 1, unpack anyway
        Console.ForegroundColor = ConsoleColor.Red;
        Console.WriteLine("This file is an old-style nxtheme or a plain SZS file.");
        Console.WriteLine("The file will be unpacked but it is not possible to repack it as-is");
        Console.WriteLine();
        Console.ResetColor();

        sourceStream.Dispose();

        var decompressed = ManagedYaz0.Decompress(File.ReadAllBytes(source));
        var sarc = SARC.Unpack(decompressed);

        foreach (var entry in sarc.Files)
        {
            var filePath = Path.Combine(dest, entry.Key);

            if (filePath.Contains("../") || filePath.Contains("..\\"))
                continue;

            var fileDir = Path.GetDirectoryName(filePath);
            if (fileDir != null)
                Directory.CreateDirectory(fileDir);

            Console.WriteLine($"Writing {filePath} ...");
            File.WriteAllBytes(filePath, entry.Value);
        }
    }
    else
    {
        using var sourceProvider = new ZipContentProvider(sourceStream);
        using var destWriter = new DirectoryContentWriter(dest);

        var theme = new NxTheme2(sourceProvider, null);
        theme.Pack(destWriter);
    }
}
else if (args[0] == "install")
{
    if (args.Length < 3)
    {
        Console.WriteLine("Not enough arguments.");
        return 1;
    }

    var result = RemoteInstall.DoRemoteInstall(args[1], File.ReadAllBytes(args[2]));
    if (result != null)
    {
        Console.WriteLine(result);
        return 1;
    }
}
else
{
    Console.WriteLine("Invalid commandline.");
    PrintHelp();
    return 1;
}

return 0;

void PrintValidation(ProcessResult result) 
{
    if (result.Warnings.Count == 0 && result.Errors.Count == 0)
    {
        Console.ForegroundColor = ConsoleColor.Green;
        Console.WriteLine("Completed. No warnings were generated.");
    }

    foreach (var warning in result.Warnings)
    {
        Console.ForegroundColor = ConsoleColor.Yellow;
        Console.WriteLine($"Warning [{warning.Source}]: {warning.Message}");
    }

    foreach (var warning in result.Errors)
    {
        Console.ForegroundColor = ConsoleColor.Red;
        Console.WriteLine($"Error [{warning.Source}]: {warning.Message}");
    }

    Console.ResetColor();
}

void PrintHelp()
{
    Console.WriteLine("Usage: NxThemeTool <command> [options]");
    Console.WriteLine("Commands:");
    Console.WriteLine("  new <target directory>             Creates a new theme structure in the given folder");
    Console.WriteLine("  validate <target>                  Ensures the selected nxtheme or folder is valid");
    Console.WriteLine("  pack <target directory> <output>   Packs a folder to an nxtheme file");
    Console.WriteLine("  unpack <file> <output directory>   Extracts the content of an nxtheme file to the given directory");
    Console.WriteLine("  install <file> <ip address>        Perform remote install to NXThemesInstaller running on a console");
    Console.WriteLine("  apply <nxtheme> <szs> <output>     Apply an nxthme file to one or more szs files. Szs must be the the path to the systemData folder of the theme installer.");
    Console.WriteLine("Extra:");
    Console.WriteLine("  <nxtheme file>                     If the only specified argument is a valid nxtheme file it will be unpacked");
}

byte[] PeekFormat(Stream stream)
{
    var bytes = new byte[10];
    stream.Read(bytes, 0, bytes.Length);
    stream.Seek(0, SeekOrigin.Begin);
    return bytes;
}