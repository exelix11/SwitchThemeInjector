using SwitchThemes.Common;
using System.Collections;
using System.Reflection;
using System.Text;

namespace NxThemeTool
{
    internal class CppGen(string RootDir)
    {
        readonly string RootDir = RootDir;

        void MakeIncludes(StringBuilder sb)
        {
            sb.AppendLine("#include \"../Layouts/Patches.hpp\"");
            sb.AppendLine("#include \"../json.hpp\"");
            sb.AppendLine("#include \"../NXTheme.hpp\"");

            sb.AppendLine();
            sb.AppendLine("using namespace std;");
            sb.AppendLine();
        }

        void CppValue(StringBuilder sb, object value) 
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
                sb.Append("{");
                for (int i = 0; i < list.Count; i++)
                {
                    CppValue(sb, list[i]);
                    if (i != list.Count - 1)
                        sb.Append(", ");
                }
                sb.Append("}");
            }
            else if (value is bool b)
            {
                sb.Append(b ? "true" : "false");
            }
            else
            {
                sb.Append(value);
            }
        }

        void WriteDotnetType(StringBuilder sb, object obj, FieldInfo[] fields, PropertyInfo[] properties)
        {
            sb.AppendLine("{");
            for (int i = 0; i < fields.Length; i++)
            {
                var field = fields[i];
                var value = field.GetValue(obj);
                if (value == null)
                    continue;
                
                sb.Append($"\t.{field.Name} = ");
                CppValue(sb, value);

                if (i != fields.Length - 1 || properties.Length != 0)
                    sb.AppendLine(",");
                else
                    sb.AppendLine();
            }
            sb.Append("}");
        }

        public void GeneratePatchTemplates() 
        {
            var info = typeof(PatchTemplate);

            StringBuilder sb = new();

            MakeIncludes(sb);
            sb.AppendLine("vector<PatchTemplate> Patches::DefaultTemplates = {");
            for (int i = 0; i < DefaultTemplates.Templates.Length; i++)
            {
                WriteDotnetType(sb, DefaultTemplates.Templates[i], info.GetFields(), []);

                if (i != DefaultTemplates.Templates.Length - 1)
                    sb.AppendLine(",");
                else
                    sb.AppendLine();
            }
            sb.AppendLine("};");

            File.WriteAllText(Path.Join(RootDir, "PatchTemplates.g.cpp"), sb.ToString());
        }
    }
}
