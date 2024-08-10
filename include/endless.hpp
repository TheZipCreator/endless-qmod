#pragma once

#include <optional>

#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

#include "misc.hpp"

namespace endless {
	// state
	struct State {
		// whether endless mode is currently activated
		bool activated = false;
	};
	extern State state;
	// starts endless mode
	void start_endless(void);
	// starts the next level in endless mode. Returns whether it actually started
	bool next_level(void);
	// Returns the next level, if one can be found
	std::optional<LevelParams> get_next_level();
	// registers hooks
	void register_hooks(void);
}
