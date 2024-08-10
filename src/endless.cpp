#include <cstdlib>
#include <vector>

#include "UnityEngine/Object.hpp"

#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapBasicData.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/GameScenesManager.hpp"
#include "GlobalNamespace/LevelBar.hpp"

#include "Zenject/DiContainer.hpp"

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/UI/Button.hpp"

#include "System/Action_1.hpp"

#include "custom-types/shared/delegate.hpp"

#include "songcore/shared/SongCore.hpp"
#include "songcore/shared/SongLoader/CustomBeatmapLevel.hpp"

#include "bsml/shared/BSML.hpp"

#include "endless.hpp"
#include "main.hpp"
#include "modconfig.hpp"


// whether endless mode should be continued
static bool endless_should_continue(GlobalNamespace::LevelCompletionResults *lcr) {
	return 
		(lcr->levelEndStateType == GlobalNamespace::LevelCompletionResults::LevelEndStateType::Cleared) ||
		(lcr->levelEndStateType == GlobalNamespace::LevelCompletionResults::LevelEndStateType::Failed && getModConfig().continue_on_fail.GetValue());
}

MAKE_HOOK_MATCH(pause_start_hook, &GlobalNamespace::PauseMenuManager::Start, void, GlobalNamespace::PauseMenuManager *self) {
	pause_start_hook(self);
	static SafePtrUnity<UnityEngine::UI::Button> button = SafePtrUnity<UnityEngine::UI::Button>();
	if(!button) {
		// create skip button
		auto canvas = self->_levelBar->get_transform()->get_parent()->get_parent()->GetComponent<UnityEngine::Canvas *>();
		RETURN_IF_NULL(canvas,);
		button.emplace(BSML::Lite::CreateUIButton(canvas->get_transform(), "Skip", {86, -55}, [self]() {
			auto mth = UnityEngine::Object::FindObjectOfType<GlobalNamespace::MenuTransitionsHelper *>();
			RETURN_IF_NULL(mth,);
			self->enabled = false;
			mth->_gameScenesManager->PopScenes(0.f, nullptr, custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>(std::function([self](Zenject::DiContainer *unused) {
				endless::next_level();
			})));
		}));
	}
	button.ptr()->get_gameObject()->SetActive(endless::state.activated);
}

MAKE_HOOK_MATCH(game_finish_hook, &GlobalNamespace::MenuTransitionsHelper::HandleMainGameSceneDidFinish, void, 
	GlobalNamespace::MenuTransitionsHelper *self,
	GlobalNamespace::StandardLevelScenesTransitionSetupDataSO *slstsdSO,
	GlobalNamespace::LevelCompletionResults *lcr
) {
	// TODO: this doesn't record scores or anything
	if(endless::state.activated && endless_should_continue(lcr)) {
		self->_gameScenesManager->PopScenes(0.f, nullptr, custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>(std::function([self](Zenject::DiContainer *unused) {
			endless::next_level();
		})));
		return;
	}
	endless::state.activated = false;
	game_finish_hook(self, slstsdSO, lcr);
}

namespace endless {
	State state;
	
	bool next_level(void) {
		auto levelParams = get_next_level();
		if(levelParams == std::nullopt) {
			PaperLogger.warn("No available levels matching given criteria");
			return false;
		}
		PaperLogger.info("Starting next level...");
		if(!start_level(levelParams.value())) {
			PaperLogger.warn("Level could not be started.");
			return false;
		}
		return true;
	}

	void start_endless(void) {
		PaperLogger.info("Starting endless mode...");
		if(!next_level())
			return;
		state.activated = true;
		PaperLogger.info("Endless mode started!");
	}	

	std::optional<LevelParams> get_next_level() {
		if(!SongCore::API::Loading::AreSongsLoaded())
			return std::nullopt;
		// get info
		auto min_nps = getModConfig().min_nps.GetValue();
		auto max_nps = getModConfig().max_nps.GetValue();
		auto characteristic = get_characteristic(getModConfig().characteristic.GetValue());
		RETURN_IF_NULL(characteristic, std::nullopt);
		auto difficulty = string_to_difficulty(getModConfig().difficulty.GetValue());
		// get all levels
		auto levels = SongCore::API::Loading::GetAllLevels();
		// filter levels by if they have the correct parameters
		std::vector<GlobalNamespace::BeatmapLevel *> filtered_levels;
		std::copy_if(levels.begin(), levels.end(), std::back_inserter(filtered_levels), [difficulty, characteristic, min_nps, max_nps](SongCore::SongLoader::CustomBeatmapLevel *level) {
			// make sure combination exists
			auto data = level->GetDifficultyBeatmapData(characteristic, difficulty);
			if(data == nullptr)
				return false;
			// check NPS
			// {
			// 	if(level->songDuration == 0.f)
			// 		return false; // I doubt this will ever be true but if some evil bastard decides to make 0s long map this will make sure it doesn't crash
			// 	float nps = static_cast<float>(data->notesCount)/level->songDuration;
			// 	PaperLogger.debug("nps: {}/{} = {}", data->notesCount, level->songDuration, nps);
			// 	if(nps < min_nps || nps > max_nps)
			// 		return false;
			// }

			// check requirements/suggestions are met
			{
				bool has_noodle = false;
				bool has_chroma = false;
				auto csdi = level->CustomSaveDataInfo;
				auto bcdbd = csdi->get().TryGetCharacteristicAndDifficulty(characteristic->_serializedName, difficulty);
				if(bcdbd != std::nullopt) {
					for(std::string requirement : bcdbd.value().get().requirements) {
						if(!SongCore::API::Capabilities::IsCapabilityRegistered(requirement)) {
							return false;
						}
						if(requirement == "Noodle Extensions")
							has_noodle = true;
						else if(requirement == "Chroma")
							has_chroma = true;
					}
					for(std::string suggestion : bcdbd.value().get().suggestions) {
						if(suggestion == "Noodle Extensions")
							has_noodle = true;
						else if(suggestion == "Chroma")
							has_chroma = true;
					}
				}
				auto is_fine = [](bool has_mod, std::string allow_state) -> bool {
					if(allow_state == "Forbidden")
						return !has_mod;
					if(allow_state == "Required")
						return has_mod;
					return true;
				};
				if(!is_fine(has_noodle, getModConfig().noodle_extensions.GetValue()))
					return false;
				if(!is_fine(has_chroma, getModConfig().chroma.GetValue()))
					return false;
			}
			return true;
		});
		if(filtered_levels.size() == 0)
			return std::nullopt;
		
		// pick random level
		auto level = filtered_levels[std::rand()%filtered_levels.size()];
		return LevelParams{level, characteristic, difficulty};
	}
	void register_hooks() {
		INSTALL_HOOK(PaperLogger, pause_start_hook);
		INSTALL_HOOK(PaperLogger, game_finish_hook);
	}
}
