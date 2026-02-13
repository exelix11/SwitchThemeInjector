using Newtonsoft.Json;
using SwitchThemes.Common;
using System.Collections;
using System.Reflection;
using System.Text;

namespace NxThemeTool
{
    class CodeBuilder
    {
        readonly StringBuilder Sb = new();
        int IndentationLevel = 0;

        void ApplyIndentation()
        {
            for (int i = 0; i < IndentationLevel; i++)
                Sb.Append('\t');
        }

        public void StartLine(string line = "")
        {
            ApplyIndentation();
            Sb.Append(line);
        }

        public void FinishLine(string line = "")
        {
            Sb.AppendLine(line);
        }

        public void AppendFullLine(string line = "")
        {
            if (string.IsNullOrWhiteSpace(line))
            {
                Sb.AppendLine();
                return;
            }

            ApplyIndentation();
            Sb.AppendLine(line);
        }

        public void Append(object text)
        {
            Sb.Append(text);
        }

        public override string ToString()
        {
            return Sb.ToString();
        }

        public void Indent() => IndentationLevel++;

        public void Unindent()
        {
            if (IndentationLevel > 0)
                IndentationLevel--;
        }

        public IndentationHandle WithIndentation()
        {
            Indent();
            return new IndentationHandle(this);
        }

        public class IndentationHandle(CodeBuilder Builder) : IDisposable
        {
            readonly CodeBuilder Builder = Builder;

            public void Dispose() =>
                Builder.Unindent();
        }
    }

    internal class CppGen(string RootDir)
    {
        readonly string RootDir = RootDir;

        void MakeIncludes(CodeBuilder sb)
        {
            sb.AppendFullLine("#include \"../Layouts/Patches.hpp\"");
            sb.AppendFullLine("#include \"../json.hpp\"");
            sb.AppendFullLine("#include \"../NXTheme.hpp\"");

            sb.AppendFullLine();
            sb.AppendFullLine("using namespace std;");
            sb.AppendFullLine();
        }

        void WriteCppValue(CodeBuilder sb, object value)
        {
            if (value is string)
            {
                sb.Append('"');
                foreach (var ch in (string)value)
                {
                    if (ch == '\\' || ch == '"')
                        sb.Append('\\');

                    sb.Append(ch);
                }
                sb.Append('"');
            }
            else if (value is IList list)
            {
                sb.Append("{ ");
                for (int i = 0; i < list.Count; i++)
                {
                    WriteCppValue(sb, list[i]);
                    if (i != list.Count - 1)
                        sb.Append(", ");
                }
                sb.Append("} ");
            }
            else if (value is bool b)
            {
                sb.Append(b ? "true" : "false");
            }
            else if (value is int or long or short or byte or uint or ulong or ushort)
            {
                sb.Append(value);
            }
            else if (value is float or double)
            {
                sb.Append(value);
            }
            else if (value is Enum)
            {
                sb.Append($"static_cast<{value.GetType().Name}>(");
                sb.Append((int)value);
                sb.Append(")");
            }
            else if (value is IDictionary dict)
            {
                sb.AppendFullLine("{");
                bool first = true;
                foreach (DictionaryEntry item in dict)
                {
                    if (!first)
                        sb.FinishLine(", ");

                    first = false;

                    sb.StartLine("{ ");
                    WriteCppValue(sb, item.Key);
                    sb.Append(", ");
                    WriteCppValue(sb, item.Value);
                    sb.Append(" }");
                }
                sb.FinishLine();
                sb.AppendFullLine("}");
            }
            // Special cases
            else
            {
                var type = value.GetType();
                Console.WriteLine($"Writing object of type {type.Name}");
                WriteObjectType(sb, value, 
                    type.GetFields(BindingFlags.Instance | BindingFlags.Public), 
                    type.GetProperties(BindingFlags.Instance | BindingFlags.Public));
            }
        }

        void WriteObjectType(CodeBuilder sb, object obj, FieldInfo[] fields, PropertyInfo[] properties)
        {
            sb.FinishLine("{ ");
            using (var _ = sb.WithIndentation())
                for (int i = 0; i < fields.Length; i++)
                {
                    var field = fields[i];
                    var value = field.GetValue(obj);
                    if (value == null)
                        continue;

                    sb.StartLine($".{field.Name} = ");
                    WriteCppValue(sb, value);

                    if (i != fields.Length - 1 || properties.Length != 0)
                        sb.FinishLine(",");
                    else
                        sb.FinishLine();
                }
            sb.StartLine("}");
        }

        public void GeneratePatchTemplates()
        {
            CodeBuilder sb = new();

            MakeIncludes(sb);
            sb.StartLine("vector<PatchTemplate> Patches::DefaultTemplates = ");

            WriteCppValue(sb, DefaultTemplates.Templates);

            File.WriteAllText(Path.Join(RootDir, "PatchTemplates.g.cpp"), sb.ToString());
        }

        public void GenerateTextureReplacementTable()
        {
            CodeBuilder sb = new();

            MakeIncludes(sb);
            sb.StartLine("unordered_map<string, vector<TextureReplacement>> Patches::textureReplacement::NxNameToList = ");

            WriteCppValue(sb, TextureReplacement.NxNameToList);

            File.WriteAllText(Path.Join(RootDir, "TextureReplacement.g.cpp"), sb.ToString());
        }

        public void GenerateLayoutJsons() 
        {
            CodeBuilder sb = new();

            foreach (var fix in NewFirmFixes.FixResources.Keys)
            {
                var res = NewFirmFixes.LoadFromFixName(fix);
                var minijson = res.AsJson(Formatting.None);

                sb.StartLine($"constexpr std::string_view {fix} = ");
                WriteCppValue(sb, minijson);
                sb.FinishLine("sv;");
            }

            File.WriteAllText(Path.Join(RootDir, "NewFirmFixes.g.hpp"), sb.ToString());
        }
    }
}
