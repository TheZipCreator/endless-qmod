#pragma once

#include "UnityEngine/GameObject.hpp"

#include "playlistcore/shared/PlaylistCore.hpp"

namespace endless {
	// Selected playlist. `nullptr` if "All" is selected.
	extern PlaylistCore::Playlist *selected_playlist;
	// Selected playset. -1 if "<None>" is selected.
	extern int selected_playset;
	// did_activate method for BSML
	void did_activate(UnityEngine::GameObject *self, bool firstActivation);
	
	// registers menu hooks
	void register_menu_hooks(void);
}
