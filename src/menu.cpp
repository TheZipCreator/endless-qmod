#include "bsml/shared/BSML.hpp"
#include "modconfig.hpp"

#include "menu.hpp"
#include "main.hpp"
#include "endless.hpp"

namespace endless {
	void did_activate(UnityEngine::GameObject *self, bool firstActivation) {
		if(!firstActivation)
			return;
		auto container = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());
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
