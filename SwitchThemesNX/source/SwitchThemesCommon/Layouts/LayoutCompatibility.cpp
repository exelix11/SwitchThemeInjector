#include "LayoutCompatibility.hpp"
#include "Bflyt/Bflyt.hpp"
#include "Bflyt/Grp1Pane.hpp"

#include <unordered_set>

using namespace std::string_literals;
using namespace SwitchThemesCommon::Compatibility;

CompatIssue SwitchThemesCommon::Compatibility::CompatIssue::MissingPane(std::string_view fileName, std::string_view paneName, std::string_view additional, bool critical)
{
	return
	{
		.FileName = std::string(fileName),
		.ItemName = std::string(paneName),
		.AdditionalInfo = std::string(additional),
		.Type = ProblemType::MissingPane,
		.Severity = critical ? ProblemSeverity::Critical : ProblemSeverity::AutoIgnored
	};
}

CompatIssue SwitchThemesCommon::Compatibility::CompatIssue::Uncertain(std::string_view fileName, std::string_view itemName, std::string_view additional)
{
	return
	{
		.FileName = std::string(fileName),
		.ItemName = std::string(itemName),
		.AdditionalInfo = std::string(additional),
		.Type = ProblemType::Uncertain,
		.Severity = ProblemSeverity::AutoIgnored
	};
}

CompatIssue SwitchThemesCommon::Compatibility::CompatIssue::MissingGroup(std::string_view fileName, std::string_view groupName)
{
	return
	{
		.FileName = std::string(fileName),
		.ItemName = std::string(groupName),
		.AdditionalInfo = ""s,
		.Type = ProblemType::MissingGroup,
		.Severity = ProblemSeverity::Critical
	};
}

std::string SwitchThemesCommon::Compatibility::LayoutNameForAnimation(std::string_view animationName)
{
	auto parts = animationName.find_first_of('/');
	if (parts == std::string_view::npos)
		return ""s;

	auto onlyName = animationName.substr(parts + 1);
	parts = onlyName.find_first_of('_');
	if (parts == std::string_view::npos)
		return ""s;

	return "blyt/" + std::string(onlyName.substr(0, parts)) + ".bflyt";
}

void SwitchThemesCommon::Compatibility::CheckAnimationCompatibility(std::vector<CompatIssue>& res, const LayoutPatch& layout, const SARC::SarcData& szs, std::string_view animName, const Bflan& bflan)
{
	auto bflytName = LayoutNameForAnimation(animName);
	if (bflytName.empty() || !szs.files.count(bflytName))
	{
		res.push_back(CompatIssue::Uncertain(animName, bflytName, "Unknown bflyt file"));
		return;
	}

	std::unordered_set<std::string> paneNames = {};
	std::unordered_set<std::string> groupNames = {};

	auto bflyt = std::make_unique<BflytFile>(szs.files.at(bflytName));
	auto paneIterator = bflyt->PanesBegin();
	while (paneIterator != bflyt->PanesEnd())
	{
		auto pane = *paneIterator;
		if (pane->PaneName != "")
			paneNames.insert(pane->PaneName);
		
		auto asGrp = std::dynamic_pointer_cast<Panes::Grp1Pane>(pane);
		if (asGrp)
			groupNames.insert(asGrp->GroupName);

		++paneIterator;
	}

	// Target groups might also be added via a patch
	std::unordered_set<std::string> addedGroups = {};
	for (const auto& patch : layout.Files)
	{
		if (patch.FileName != bflytName)
			continue;

		for (auto& grp : patch.AddGroups)
			addedGroups.insert(grp.GroupName);
	}

	auto patData = bflan.FindSectionByType<Pat1Section>();
	if (patData)
		for (const auto& group : patData->Groups) {
			if (groupNames.count(group) == 0 && addedGroups.count(group) == 0)
				res.push_back(CompatIssue::MissingGroup(bflytName, group));
		}

	auto paiData = bflan.FindSectionByType<Pai1Section>();
	if (paiData)
		for (const auto& entry : paiData->Entries)
		{
			if (entry.Target == PaiEntry::AnimationTarget::Pane)
				if (!paneNames.count(entry.Name))
					res.push_back(CompatIssue::MissingPane(animName, entry.Name, "Animation target", true));
			// TODO: Materials
			// TODO: Textures
		}

}
