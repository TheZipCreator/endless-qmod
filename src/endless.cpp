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
#include "GlobalNamespace/ScoreUIController.hpp"
#include "GlobalNamespace/CoreGameHUDController.hpp"
#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/ScoreFormatter.hpp"

#include "Zenject/DiContainer.hpp"

#include "UnityEngine/Transform.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Canvas.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "UnityEngine/WaitForSecondsRealtime.hpp"

#include "TMPro/TextMeshProUGUI.hpp"

#include "HMUI/CurvedTextMeshPro.hpp"

#include "System/Action_1.hpp"
#include "System/Collections/IEnumerator.hpp"

#include "custom-types/shared/delegate.hpp"

#include "songcore/shared/SongCore.hpp"
#include "songcore/shared/SongLoader/CustomBeatmapLevel.hpp"

#include "bsml/shared/BSML.hpp"

#include "endless.hpp"
#include "main.hpp"
#include "modconfig.hpp"
#include "menu.hpp"


// whether endless mode should be continued
static bool endless_should_continue(GlobalNamespace::LevelCompletionResults *lcr) {
	return 
		(lcr->levelEndStateType == GlobalNamespace::LevelCompletionResults::LevelEndStateType::Cleared) ||
		(lcr->levelEndStateType == GlobalNamespace::LevelCompletionResults::LevelEndStateType::Failed && getModConfig().continue_on_fail.GetValue());
}

MAKE_HOOK_MATCH(PauseMenuManager_Start, &GlobalNamespace::PauseMenuManager::Start, void, GlobalNamespace::PauseMenuManager *self) {
	PauseMenuManager_Start(self);
	static SafePtrUnity<UnityEngine::UI::Button> button = SafePtrUnity<UnityEngine::UI::Button>();
	if(!button) {
		// create skip button
		auto canvas = self->_levelBar->transform->parent->parent->GetComponent<UnityEngine::Canvas *>();
		RETURN_IF_NULL(canvas,);
		button.emplace(BSML::Lite::CreateUIButton(canvas->transform, "Skip", {86, -55}, [self]() {
			auto mth = UnityEngine::Object::FindObjectOfType<GlobalNamespace::MenuTransitionsHelper *>();
			RETURN_IF_NULL(mth,);
			self->enabled = false;
			mth->_gameScenesManager->PopScenes(0.f, nullptr, custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>(std::function([self](Zenject::DiContainer *unused) {
				endless::next_level();
			})));
		}));
	}
	button->gameObject->SetActive(endless::state.activated);
}

MAKE_HOOK_MATCH(PauseMenuManager_MenuButtonPressed, &GlobalNamespace::PauseMenuManager::MenuButtonPressed, void, GlobalNamespace::PauseMenuManager *self) {
	PauseMenuManager_MenuButtonPressed(self);
	endless::state.activated = false;
}


MAKE_HOOK_MATCH(ScoreUIController_Start, &GlobalNamespace::ScoreUIController::Start, void, GlobalNamespace::ScoreUIController *self) {
	ScoreUIController_Start(self);
	if(!endless::state.time_text) {
		auto canvas = UnityEngine::Object::FindObjectOfType<GlobalNamespace::CoreGameHUDController *>()->_energyPanelGO->GetComponentInChildren<UnityEngine::Canvas *>();
		RETURN_IF_NULL(canvas,);
		endless::state.time_text.emplace(BSML::Lite::CreateText(canvas->transform, "", TMPro::FontStyles::Normal, 14.f, {-50, 208}, {100, 5}));
		endless::state.score_text.emplace(BSML::Lite::CreateText(canvas->transform, "", TMPro::FontStyles::Normal, 14.f, {70, 208}, {100, 5}));
	}
	bool hud_enabled = endless::state.activated && getModConfig().hud_enabled.GetValue();
	endless::state.time_text->gameObject->SetActive(hud_enabled);
	endless::state.score_text->gameObject->SetActive(hud_enabled);
	if(hud_enabled)
		endless::set_score_text(0);
}

MAKE_HOOK_MATCH(ScoreUIController_UpdateScore, &GlobalNamespace::ScoreUIController::UpdateScore, void,
	GlobalNamespace::ScoreUIController *self,
	int multipliedScore,
	int modifiedScore
) {
	ScoreUIController_UpdateScore(self, multipliedScore, modifiedScore);
	if(!endless::state.activated)
		return;
	endless::set_score_text(modifiedScore);
}

MAKE_HOOK_MATCH(MenuTransitionsHelper_HandleMainGameSceneDidFinish, &GlobalNamespace::MenuTransitionsHelper::HandleMainGameSceneDidFinish, void, 
	GlobalNamespace::MenuTransitionsHelper *self,
	GlobalNamespace::StandardLevelScenesTransitionSetupDataSO *slstsdSO,
	GlobalNamespace::LevelCompletionResults *lcr
) {
	// TODO: this doesn't record scores or anything
	if(endless::state.activated && endless_should_continue(lcr)) {
		if(lcr->levelEndStateType == GlobalNamespace::LevelCompletionResults::LevelEndStateType::Cleared)
			endless::state.score += lcr->modifiedScore;
		self->_gameScenesManager->PopScenes(0.f, nullptr, custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>(std::function([self](Zenject::DiContainer *unused) {
			endless::next_level();
		})));
		return;
	}
	// endless::state.activated = false;
	MenuTransitionsHelper_HandleMainGameSceneDidFinish(self, slstsdSO, lcr);
}

MAKE_HOOK_MATCH(ResultsViewController_SetDataToUI, &GlobalNamespace::ResultsViewController::SetDataToUI, void, GlobalNamespace::ResultsViewController *self) {
	
	// objects to deactivate/reactivate
	std::vector<UnityEngine::GameObject *> objects = {
		self->_levelBar->gameObject,
		self->_newHighScoreText->gameObject
	};
	// add things that aren't the score to the list of objects 
	// This also removes the label that literally says "Score". FIXME
	{
		auto child_to_keep = self->_scoreText->gameObject->transform->parent;
		auto parent = child_to_keep->parent;
		for(std::size_t i = 0; i < parent->childCount; i++) {
			auto child = parent->GetChild(i);
			if(child != child_to_keep)
				objects.push_back(child->gameObject);
		}
	}
	
	// reactivate things in case they were deactivated
	for(UnityEngine::GameObject *go : objects)
		go->SetActive(true);
	
	// call original
	ResultsViewController_SetDataToUI(self);

	// modify menu in endless
	if(endless::state.activated && self->_levelCompletionResults->levelEndStateType == GlobalNamespace::LevelCompletionResults::LevelEndStateType::Failed) {
		// set score text
		self->_scoreText->text = GlobalNamespace::ScoreFormatter::Format(endless::state.score+self->_levelCompletionResults->modifiedScore);

		// activate/deactivate things
		self->_scoreText->gameObject->transform->parent->parent->gameObject->SetActive(true);
		self->_scoreText->gameObject->transform->parent->gameObject->SetActive(true);
		self->_scoreText->gameObject->SetActive(true);
		
		for(UnityEngine::GameObject *go : objects)
			go->SetActive(false);

		// deactivate endless now that we're done
		endless::state.activated = false;
	}
}



namespace endless {
	State state;
	
	// coroutine to update the time text
	static custom_types::Helpers::Coroutine update_time_coroutine() {
		while(true) {
			if(endless::state.score_text) {
				float elapsed = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now()-state.start_time).count()/1000.f;
				int32_t seconds = static_cast<int32_t>(elapsed)%60;
				int32_t minutes = static_cast<int32_t>(elapsed/60.f)%60;
				// I wonder if anyone will actually ever get to hours
				int32_t hours = static_cast<int32_t>(elapsed/3600.f);
				std::stringstream stream;
				stream << "Total Time ";
				if(hours == 0) {
					stream << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
				} else {
					stream << hours << ":" << std::setfill('0') << std::setw(2) << minutes << std::setfill('0') << std::setw(2) << seconds;
				}
				state.time_text->text = stream.str();
			}
			if(!state.activated)
				co_return;
			else
				co_yield reinterpret_cast<System::Collections::IEnumerator *>(UnityEngine::WaitForSecondsRealtime::New_ctor(1));
		}
	}
	
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

	void calculate_levels(bool automatic) {
		PaperLogger.info("Calculating levels...");
		state.levels = {};
		if(!SongCore::API::Loading::AreSongsLoaded())
			return;
		if(automatic) {
			// get info
			auto min_nps = getModConfig().min_nps.GetValue();
			auto max_nps = getModConfig().max_nps.GetValue();
			auto characteristic = get_characteristic(getModConfig().characteristic.GetValue());
			RETURN_IF_NULL(characteristic,);
			auto difficulty = string_to_difficulty(getModConfig().difficulty.GetValue());
			// get levels in the selected playlist
			std::vector<GlobalNamespace::BeatmapLevel *> levels;
			if(selected_playlist == nullptr)
				for(auto level : SongCore::API::Loading::GetAllLevels())
					levels.push_back(level);
			else {
				for(auto level : selected_playlist->playlistCS->beatmapLevels)
					levels.push_back(level);
			}
			// filter levels by if they have the correct parameters
			std::vector<GlobalNamespace::BeatmapLevel *> filtered_levels;
			std::copy_if(levels.begin(), levels.end(), std::back_inserter(filtered_levels), [difficulty, characteristic, min_nps, max_nps](GlobalNamespace::BeatmapLevel *level) {
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
				auto custom_level_opt = il2cpp_utils::try_cast<SongCore::SongLoader::CustomBeatmapLevel>(level);
				auto custom_level = custom_level_opt == std::nullopt ? nullptr : custom_level_opt.value();
				
				// check requirements/suggestions are met
				{
					bool has_noodle = false;
					bool has_chroma = false;
					if(custom_level != nullptr) {
						auto csdi = custom_level->CustomSaveDataInfo;
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
				return;
			std::transform(filtered_levels.begin(), filtered_levels.end(), std::back_inserter(state.levels), [=](GlobalNamespace::BeatmapLevel *level) {
				return LevelParams{level, characteristic, difficulty};
			});	
		} else {
			// TODO
		}
	}

	void start_endless(void) {
		// start endless
		PaperLogger.info("Starting endless mode...");
		if(!next_level())
			return;
		state.activated = true;
		state.start_time = std::chrono::steady_clock::now();
		state.score = 0;
		// start timer
		{
			auto mth = UnityEngine::Object::FindObjectOfType<GlobalNamespace::MenuTransitionsHelper *>();
			if(mth != nullptr)
				mth->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(update_time_coroutine()));
			else
				PaperLogger.warn("mth is null");
		}
		PaperLogger.info("Endless mode started!");
	}	
	
	void set_score_text(int score) {
		if(!endless::state.score_text)
			return;
		int total_score = score+endless::state.score;
		std::string text = "Total Score ";
		text += std::to_string(total_score);
		endless::state.score_text->text = text;
	}

	std::optional<LevelParams> get_next_level() {
		// pick random level
		return state.levels[std::rand()%state.levels.size()];
	}
	void register_hooks() {
		INSTALL_HOOK(PaperLogger, PauseMenuManager_Start);
		INSTALL_HOOK(PaperLogger, PauseMenuManager_MenuButtonPressed);

		INSTALL_HOOK(PaperLogger, ScoreUIController_Start);
		INSTALL_HOOK(PaperLogger, ScoreUIController_UpdateScore);

		INSTALL_HOOK(PaperLogger, MenuTransitionsHelper_HandleMainGameSceneDidFinish);

		INSTALL_HOOK(PaperLogger, ResultsViewController_SetDataToUI);
	}
}
