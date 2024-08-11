#pragma once

#include "UnityEngine/GameObject.hpp"

#include "playlistcore/shared/PlaylistCore.hpp"

namespace endless {
	// Selected playlist. `nullptr` if "All" is selected.
	extern PlaylistCore::Playlist *selected_playlist;
	// did_activate method for BSML
	void did_activate(UnityEngine::GameObject *self, bool firstActivation);
}
