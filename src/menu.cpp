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
		auto container = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());
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
			BSML::Lite::CreateText(container->get_transform(), stream.str(), TMPro::FontStyles::Underline, {0, 0}, {0, 25});
		}
		// sliders for NPS, currently disabled because implementing them would be more of a pain than I thought it'd be
		// BSML::Lite::CreateSliderSetting(container->get_transform(), "Minimum NPS", 0.1, getModConfig().min_nps.GetValue(), 0.0, 20.0, [](float val) {
		// 	getModConfig().min_nps.SetValue(val);
		// });
		// BSML::Lite::CreateSliderSetting(container->get_transform(), "Maximum NPS", 0.1, getModConfig().max_nps.GetValue(), 0.0, 20.0, [](float val) {
		// 	getModConfig().max_nps.SetValue(val);
		// });
		BSML::Lite::CreateToggle(container->get_transform(), "Continue on Fail", getModConfig().continue_on_fail.GetValue(), [](bool value) {
			getModConfig().continue_on_fail.SetValue(value);
		});
		// I don't know if this is what you're supposed to do but it compiles
		std::vector<std::string_view> difficulties{"Easy", "Normal", "Hard", "Expert", "Expert+"};
		std::span difficulties_span { difficulties };
		BSML::Lite::CreateDropdown(container->get_transform(), "Difficulty", getModConfig().difficulty.GetValue(), difficulties_span, [](StringW string) {
			getModConfig().difficulty.SetValue(string);
		});
		BSML::Lite::CreateUIButton(container->get_transform(), "Start!", []() {
			start_endless();
		});
	}
}
