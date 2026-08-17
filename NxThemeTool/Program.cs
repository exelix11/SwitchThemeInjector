using NxThemeTool;
using SARCExt;
using SwitchThemes.Common;
using System.ComponentModel.DataAnnotations;

Console.WriteLine("NxThemeTool - https://github.com/exelix11/SwitchThemeInjector");
Console.WriteLine($"Using ThemesCommon {CommonInfo.CoreVer}");

if (args.Length == 0 || args.Any(x => x == "help" || x == "-h" || x == "--help" || x == "-help"))
{
    PrintHelp();
    return 0;
}

void NeedArgs(int count) 
{
    if (args.Length < count + 1) // Account for the command itself being the first argument
    {
        Console.WriteLine("Not enough arguments.");
        Environment.Exit(1);
    }
}

if (args[0] == "list")
{
    Console.WriteLine("Supported theme parts:");
    foreach (var item in CommonInfo.Parts)
    {
        Console.WriteLine($"- Part name: {item.Name}");
        Console.WriteLine($"\t {item.Description} is {item.SzsName} in {item.TitleId}");
    }
}
else if (args[0] == "new")
{
    NeedArgs(2);

    var theme = NxTheme.CreateNew(args[1]);
    theme.MainImageFile = Util.CreateEmpty720PJPG();

    using var writer = new DirectoryContentWriter(args[2]);
    theme.Pack(writer);
}
else if (args[0] == "validate")
{
    NeedArgs(1);

    var validation = new ProcessResult();

    using var provider = ProviderHelper.OpenFor(args[1]);
    new NxTheme(provider, validation);

    PrintValidation(validation);
}
else if (args[0] == "pack")
{
    NeedArgs(2);

    if (!Directory.Exists(args[1]))
    {
        Console.WriteLine("Target directory does not exist.");
        return 1;
    }

    using var provider = new DirectoryContentProvider(args[1]);
    using var writer = new ZipContentWriter(new FileStream(args[2], FileMode.Create, FileAccess.Write));
    var validation = new ProcessResult();

    var theme = new NxTheme(provider, validation);
    PrintValidation(validation);

    theme.Pack(writer);
}
else if (args[0] == "apply")
{
    NeedArgs(3);

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
    if (args[0] == "unpack")
        NeedArgs(2);

    var source = args[0] == "unpack" ? args[1] : args[0];
    var dest = args[0] == "unpack" ? args[2] : Path.GetFileNameWithoutExtension(args[0]) + "_unpacked";

    using var provider = ProviderHelper.OpenFor(source);
    using var destWriter = new DirectoryContentWriter(dest);

    var theme = new NxTheme(provider, null);
    theme.Pack(destWriter);
}
else if (args[0] == "install")
{
    NeedArgs(2);

    var result = RemoteInstall.DoRemoteInstall(args[2], File.ReadAllBytes(args[1]));
    if (result != null)
    {
        Console.WriteLine(result);
        return 1;
    }
}
else if (args[0] == "cppgen")
{
    NeedArgs(1);

    var gen = new CppGen(args[1]);
    gen.GeneratePatchTemplates();
    gen.GenerateTextureReplacementTable();
    gen.GenerateLayoutJsons();
}
else if (args[0] == "diff")
{
    NeedArgs(3);
    var source = args[1];
    var modified = args[2];
    var outpput = args[3];

    var differ = new LayoutDiff(
        SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(source))),
        SARC.Unpack(ManagedYaz0.Decompress(File.ReadAllBytes(modified)))
    );

    var res = differ.ComputeDiff();
    
    if (!string.IsNullOrWhiteSpace(differ.OutputLog))
        Console.WriteLine(differ.OutputLog);

    File.WriteAllText(outpput, res.AsJson());
}
else if (args[0] == "szs")
{
    NeedArgs(2);
    var source = args[1];
    var output = args.Last();

    var dds = args.FirstOrDefault(x => x.EndsWith(".dds"));
    var json = args.FirstOrDefault(x => x.EndsWith(".json"));

    if (dds is null && json is null)
    {
        Console.WriteLine("At least a dds or a json file must be provided.");
        return 1;
    }

    if (output == dds || output == json)
    {
        Console.WriteLine("Missing output file name.");
        return 1;
    }

    var res = ThemeApply.ApplySimple(
        File.ReadAllBytes(source),
        dds is null ? null : File.ReadAllBytes(dds),
        json is null ? null : File.ReadAllText(json)
    );

    File.WriteAllBytes(output, res);
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
    Console.WriteLine("  new <theme part> <directory>       Creates a new theme structure in the specified folder for the provided theme part.");
    Console.WriteLine("  list                               Shows the list of valid theme parts");
    Console.WriteLine("  validate <target>                  Ensures the selected nxtheme or folder is valid");
    Console.WriteLine("  pack <target directory> <output>   Packs a folder to a nxtheme file");
    Console.WriteLine("  unpack <file> <output directory>   Extracts the content of a nxtheme file to the specified directory");
    Console.WriteLine("  install <file> <ip address>        Perform remote install to NXThemesInstaller running on a console");
    Console.WriteLine("  apply <nxtheme> <szs> <output>     Apply an nxthme file to one or more szs files. Szs must be the the path to the systemData folder of the theme installer.");
    Console.WriteLine("  diff <source szs> <modified szs> <output json>     Produce a json diff from the two provided szs files.");
    Console.WriteLine("  szs <target szs> [optional dds file] [optional json layout] <output szs>     Patch an szs file with the provided dds image and json layout.");
    Console.WriteLine("Extra:");
    Console.WriteLine("  <nxtheme file>                     If the only specified argument is a valid nxtheme file it will be unpacked. This is a convenience feature which allows dragging nxtheme files on this binary to unpack them automatically.");
    Console.WriteLine("Development:");
    Console.WriteLine("  cppgen <output foler>              Generate C++ data tables needed by the NxThemesInstaller codebase");
}