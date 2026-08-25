#pragma once

#include <string>
#include <string_view>

#include "../../SwitchThemesCommon/Common.hpp"

namespace RemoteInstall
{
	inline constexpr std::string_view ImageTarget = "__image";

	inline bool IsImage(std::string_view target) noexcept
	{
		return target == ImageTarget;
	}

	inline std::string TargetLabel(std::string_view target)
	{
		if (IsImage(target))
			return "Plain Image";

		auto info = ThemeTargetInfo::Find(std::string(target));
		return info ? info->PartName : "Unknown part name";
	}
}
