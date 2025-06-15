#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "../SarcLib/Sarc.hpp"
#include "Bflan.hpp"
#include "Patches.hpp"

namespace SwitchThemesCommon::Compatibility 
{
	enum class ProblemType 
	{
        // The layout references a file that does not exist anymore
        MissingFile,
        // The layout or animation references a pane that does not exist anymore
        MissingPane,
        // The animation references a group that does not exist anymore
        MissingGroup,
        // The animation references a texture that does not exist anymore
        MissingTexture,
        // The heuristic failed to determine the severity of the issue
        Uncertain
	};

    enum class ProblemSeverity
    {
        // This issue won't crash the theme, it will be automatically ignored by the theme installer
        // it is the case for layout patches that refer to non-existing panes
        AutoIgnored,
        // This issue will crash the console if the theme is installed
        // It is the case for custom animations that refer to non-existing panes or groups
        // To prevent crashes during the installation process any file that has a Critical issue will be dropped
        Critical
    };

    struct CompatIssue
    {
        std::string FileName;
        std::string ItemName;
        std::string AdditionalInfo;
		ProblemType Type;
		ProblemSeverity Severity;

        static CompatIssue MissingPane(std::string_view fileName, std::string_view paneName, std::string_view additional = "", bool critical = false);
        static CompatIssue Uncertain(std::string_view fileName, std::string_view itemName, std::string_view additional);
        static CompatIssue MissingGroup(std::string_view fileName, std::string_view groupName);
    };

    std::string LayoutNameForAnimation(std::string_view animationName);
    void CheckAnimationCompatibility(std::vector<CompatIssue>& res, const LayoutPatch& layout, const SARC::SarcData& szs, std::string_view animName, const Bflan& bflan);

    // TODO: The c++ implementation is not complete. We do not implement bflyt checks since those are not currently used in the install process
}