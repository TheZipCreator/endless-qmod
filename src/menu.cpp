#include "TMPro/FontStyles.hpp"

#include "bsml/shared/BSML.hpp"
#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/Transform.hpp"

#include "modconfig.hpp"
#include "menu.hpp"
#include "main.hpp"
#include "endless.hpp"
#include "misc.hpp"

namespace endless {
	PlaylistCore::Playlist *selected_playlist = nullptr;
	int selected_playset = -1;
	
	template<class T>
	static T tab_add_parent(std::shared_ptr<std::vector<UnityEngine::GameObject *>> tab, T object) {
		tab->push_back(object->transform->parent->gameObject);
		return object;
	}
	
	template<class T>
	static T tab_add(std::shared_ptr<std::vector<UnityEngine::GameObject *>> tab, T object) {
		tab->push_back(object->gameObject);
		return object;
	}

	void did_activate(UnityEngine::GameObject *self, bool firstActivation) {
		if(!firstActivation)
			return;
		auto container = BSML::Lite::CreateScrollableSettingsContainer(self->transform);
		// add incompatible mods warning if applicable
		if(enabled_incompatible_mods.size() > 0) {
			std::stringstream stream;
			stream << "The following incompatible mods are enabled:\n";
			for(std::size_t i = 0; i < enabled_incompatible_mods.size(); i++) {
				if(i != 0)
					stream << ", ";
				stream << enabled_incompatible_mods[i];
			}
			stream << "\nThese mods are known to cause crashes or other problems with Endless.";
			stream << "\nIt is HIGHLY recommended to disable them before continuing.";
			BSML::Lite::CreateText(container->transform, stream.str(), TMPro::FontStyles::Underline, {0, 0}, {0, 25});
		}
		// sliders for NPS, currently disabled because implementing them would be more of a pain than I thought it'd be
		// BSML::Lite::CreateSliderSetting(container->transform, "Minimum NPS", 0.1, getModConfig().min_nps.GetValue(), 0.0, 20.0, [](float val) {
		// 	getModConfig().min_nps.SetValue(val);
		// });
		// BSML::Lite::CreateSliderSetting(container->transform, "Maximum NPS", 0.1, getModConfig().max_nps.GetValue(), 0.0, 20.0, [](float val) {
		// 	getModConfig().max_nps.SetValue(val);
		// });
		BSML::Lite::CreateToggle(container->transform, "Continue on Fail", getModConfig().continue_on_fail.GetValue(), [](bool value) {
			getModConfig().continue_on_fail.SetValue(value);
		});
		BSML::Lite::CreateToggle(container->transform, "Endless HUD Enabled", getModConfig().hud_enabled.GetValue(), [](bool value) {
			getModConfig().hud_enabled.SetValue(value);
		});	
		
		auto automatic_tab = std::make_shared<std::vector<UnityEngine::GameObject *>>();
		auto playset_tab = std::make_shared<std::vector<UnityEngine::GameObject *>>();
		
		std::vector<std::string_view> tabs = {"Automatic", "Playset"};
		std::span<std::string_view> tab_span { tabs }; // don't know why doing this is necessary /here/ but nowhere else.
		auto tsc = BSML::Lite::CreateTextSegmentedControl(container->transform, tab_span, [=](int idx){
			for(auto go : *automatic_tab)
				go->active = idx == 0;
			for(auto go : *playset_tab)
				go->active = idx == 1;
		});
		// automatic
		{
			// playlist
			std::vector<std::string_view> playlist_names = {"All"};
			std::vector<PlaylistCore::Playlist *> playlists = PlaylistCore::GetLoadedPlaylists();
			for(PlaylistCore::Playlist *playlist : playlists) {
				playlist_names.push_back(playlist->name);
			}
			// this could break if playlists are updated while in-game. FIXME
			tab_add_parent(automatic_tab, BSML::Lite::CreateDropdown(container->transform, "Playlist", "All", playlist_names, [playlists](StringW string) {
				selected_playlist = nullptr;
				if(string == "All")
					return;
				for(PlaylistCore::Playlist *playlist : playlists) {
					if(playlist->name != string)
						continue;
					selected_playlist = playlist;
				}
			}));

			// difficulty
			std::vector<std::string_view> difficulties{"Easy", "Normal", "Hard", "Expert", "Expert+"};
			tab_add_parent(automatic_tab, BSML::Lite::CreateDropdown(container->transform, "Difficulty", getModConfig().difficulty.GetValue(), difficulties, [](StringW string) {
				getModConfig().difficulty.SetValue(string);
			}));

			// mods
			std::vector<std::string_view> allow_state{"Allowed", "Required", "Forbidden"};
			tab_add_parent(automatic_tab, BSML::Lite::CreateDropdown(container->transform, "Noodle Extensions", getModConfig().noodle_extensions.GetValue(), allow_state, [](StringW string) {
				getModConfig().noodle_extensions.SetValue(string);
			}));
			tab_add_parent(automatic_tab, BSML::Lite::CreateDropdown(container->transform, "Chroma", getModConfig().chroma.GetValue(), allow_state, [](StringW string) {
				getModConfig().chroma.SetValue(string);
			}));

			// start button
			tab_add(automatic_tab, BSML::Lite::CreateUIButton(container->transform, "Start!", []() {
				calculate_levels(true);
				start_endless();
			}));
		}
		// playset
		{
			std::vector<std::string_view> _names = {"<None>"};

			// start button
			tab_add(playset_tab, BSML::Lite::CreateUIButton(container->transform, "Start!", []() {
				calculate_levels(false);
				start_endless();
			}));
			
		}
		#undef ADD
		#undef ADD_PARENT
		for(auto go : *playset_tab)
			go->active = false;
	}
}
