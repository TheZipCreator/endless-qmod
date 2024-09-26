#pragma once

#include "config-utils/shared/config-utils.hpp"
// #include "rapidjson-macros/shared/macros.hpp"

namespace endless {
	DECLARE_JSON_CLASS(PlaysetBeatmap,
		VALUE(std::string, id);
		VALUE(std::string, difficulty);
		VALUE(std::string, characteristic);
	);
	DECLARE_JSON_CLASS(Playset,
		VALUE(std::string, name);
		VECTOR(PlaysetBeatmap, beatmaps);
	);
}

DECLARE_CONFIG(ModConfig,
	CONFIG_VALUE(min_nps, double, "min_nps", 5.0);
	CONFIG_VALUE(max_nps, double, "max_nps", 8.0);
	CONFIG_VALUE(difficulty, std::string, "difficulty", "Expert+");
	CONFIG_VALUE(characteristic, std::string, "characteristic", "Standard");
	CONFIG_VALUE(noodle_extensions, std::string, "noodle_extensions", "Allowed");
	CONFIG_VALUE(chroma, std::string, "chroma", "Allowed");
	CONFIG_VALUE(continue_on_fail, bool, "continue_on_fail", false);
	CONFIG_VALUE(hud_enabled, bool, "hud_enabled", true);
	CONFIG_VALUE(playsets, std::vector<endless::Playset>, "playsets", {});
);
