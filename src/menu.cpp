#include "TMPro/FontStyles.hpp"

#include "bsml/shared/BSML.hpp"
#include "modconfig.hpp"

#include "menu.hpp"
#include "main.hpp"
#include "endless.hpp"
#include "misc.hpp"

namespace endless {
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

		// difficulty
		// I don't know if this is what you're supposed to do but it compiles
		std::vector<std::string_view> difficulties{"Easy", "Normal", "Hard", "Expert", "Expert+"};
		std::span difficulties_span{ difficulties };
		BSML::Lite::CreateDropdown(container->transform, "Difficulty", getModConfig().difficulty.GetValue(), difficulties_span, [](StringW string) {
			getModConfig().difficulty.SetValue(string);
		});

		// mods
		std::vector<std::string_view> allow_state{"Allowed", "Required", "Forbidden"};
		std::span allow_state_span{ allow_state };
		BSML::Lite::CreateDropdown(container->transform, "Noodle Extensions", getModConfig().noodle_extensions.GetValue(), allow_state, [](StringW string) {
			getModConfig().noodle_extensions.SetValue(string);
		});
		BSML::Lite::CreateDropdown(container->transform, "Chroma", getModConfig().chroma.GetValue(), allow_state, [](StringW string) {
			getModConfig().chroma.SetValue(string);
		});

		// start button
		BSML::Lite::CreateUIButton(container->transform, "Start!", []() {
			start_endless();
		});
	}
}
