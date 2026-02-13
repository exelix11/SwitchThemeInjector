#include "Patches.hpp"
#include "../json.hpp"
#include "../NXTheme.hpp"

using namespace std;

using json = nlohmann::json;

using nlohmann::json;

#define get_s(s, f) j.at(s).get_to(f)
#define get(n) get_s(#n, p.n)

#define get_if_s_else(s, f, def) do { if(j.count(s)) j.at(s).get_to(f); else def; } while (0)
#define get_if_s(s, f) if(j.count(s)) j.at(s).get_to(f)

#define get_if(n) get_if_s(#n, p.n)
#define get_if_else(n, def) get_if_s_else(#n, p.n, p.n = def)

template<typename T>
static inline std::optional<T> GetOptionalHelper(const json& j, const char* name)
{
	if (j.count(name))
		return j.at(name).get<T>();
	return std::nullopt;
}

#define get_optional_s(s, f) f = GetOptionalHelper<decltype(f)::value_type>(j, s)
#define get_optional(n) get_optional_s(#n, p.n)

void from_json(const json& j, Vector2& p) {
	p = {};
	get_if(X);
	get_if(Y);
}

void from_json(const json& j, Vector3& p) {
	p = {};
	get_if(X);
	get_if(Y);
	get_if(Z);
}

void from_json(const json& j, UsdPatch& p) {
	p = {};
	get(PropName);
	get_if(PropValues);
	get_if(type);
}

void from_json(const json& j, PanePatch& p) {
	p = {};
	get(PaneName);

	#define get_assign_member(jname, field, flag) do { \
		if (j.count(jname)) { \
			j.at(jname).get_to(field); \
			p.ApplyFlags |= (u32)PanePatch::Flags::flag; \
		} } while(0)

	#define get_assign(name) get_assign_member(#name, p.name, name)
	
	get_assign(Position);
	get_assign(Rotation);
	get_assign(Scale);
	get_assign(Size);
	get_assign(Visible);

	get_assign(OriginX);
	get_assign(OriginY);
	get_assign(ParentOriginX);
	get_assign(ParentOriginY);

	get_assign_member("ColorTL", p.PaneSpecific0(), PaneSpecific0);
	get_assign_member("ColorTR", p.PaneSpecific1(), PaneSpecific1);
	get_assign_member("ColorBL", p.PaneSpecific2(), PaneSpecific2);
	get_assign_member("ColorBR", p.PaneSpecific3(), PaneSpecific3);

	get_assign(UsdPatches);

	#undef get_assign
	#undef get_assign_member
}

void from_json(const json& j, ExtraGroup& p) {
	p = {};	
	get(GroupName);
	get_if(Panes);
}

void from_json(const json& j, MaterialPatch::TexReference& p) {
	p = {};
	get(Name);
	get_optional(WrapS);
	get_optional(WrapT);
}

void from_json(const json& j, MaterialPatch::TexTransform& p) {
	p = {};
	get(Name);
	get_optional(X);
	get_optional(Y);
	get_optional(Rotation);
	get_optional(ScaleX);
	get_optional(ScaleY);
}

void from_json(const json& j, MaterialPatch& p) {
	p = {};
	get(MaterialName);
	get_if(ForegroundColor);
	get_if(BackgroundColor);
	get_if(Refs);
	get_if(Transforms);
}

void from_json(const json& j, LayoutFilePatch& p) {
	p = {};
	get(FileName);
	get_if(Patches);
	get_if(AddGroups);
	get_if(Materials);
	get_if(PushBackPanes);
	get_if(PullFrontPanes);
}

void from_json(const json& j, AnimFilePatch& p) {
	p = {};
	get(FileName);
	get(AnimJson);
}

void from_json(const json& j, LayoutPatch& p) {
	p = {};
	get_if(PatchName);
	get_if(AuthorName);
	get_if(Files);
	get_if(Anims);
	get_if(PatchAppletColorAttrib);
	get_if(ID);
	get_if_else(HideOnlineBtn, true);
	get_if_s("Ready8X", p.Obsolete_Ready8X);
	get_if_else(TargetFirmware, ((int)ConsoleFirmware::Fw11_0));
}

#undef get_s
#undef get
#undef get_if_s
#undef get_if
#undef get_optional_s
#undef get_optional

LayoutPatch Patches::LoadLayout(const string_view jsn)
{
	return json::parse(jsn).get<LayoutPatch>();
}