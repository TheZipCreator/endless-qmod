#pragma once

#include <functional>
#include <optional>

#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/LevelBar.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "UnityEngine/Transform.hpp"

#include "modconfig.hpp"

#define RETURN_IF_NULL(VALUE, RETURN) do { \
	if(VALUE == nullptr) { \
		PaperLogger.warn(#VALUE " is null."); \
		return RETURN; \
	} \
} while(0)


namespace endless {
	// level with difficulty and characteristic
	struct LevelParams {
		GlobalNamespace::BeatmapLevel *level;
		GlobalNamespace::BeatmapCharacteristicSO *characteristic;
		GlobalNamespace::BeatmapDifficulty difficulty;

		static std::optional<LevelParams> from_playset_beatmap(PlaysetBeatmap psb);
		static std::optional<LevelParams> from_beatmap_key(GlobalNamespace::BeatmapKey key);
	};
	// converts a string to a difficulty
	GlobalNamespace::BeatmapDifficulty string_to_difficulty(std::string string);
	// converts a difficulty to a string
	std::string difficulty_to_string(GlobalNamespace::BeatmapDifficulty dif); 	
	// gets a characteristic by name. returns `null` if it can't be found.
	GlobalNamespace::BeatmapCharacteristicSO *get_characteristic(std::string name);
	// starts playing a level. Returns `true` if the level was successfully started, `false` otherwise.
	bool start_level(LevelParams params);
	// Mods that are known to cause issues
	extern const std::vector<std::string> incompatible_mods;
	// Anything in `incompatible_mods` that is enabled.
	extern std::vector<std::string> enabled_incompatible_mods;
	// Checks for incompatible mods
	void check_for_incompatible_mods(void);
	// gets a beatmap by an id (returns null if not found)
	GlobalNamespace::BeatmapLevel *get_beatmap(std::string id);
	// Creates a level bar
	GlobalNamespace::LevelBar *create_level_bar(UnityEngine::Transform *parent, LevelParams params);
	
	
}
