#include <algorithm>

#include "UnityEngine/Object.hpp"

#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"
#include "GlobalNamespace/BeatmapCharacteristicCollection.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/SinglePlayerLevelSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/RecordingToolManager.hpp"
#include "GlobalNamespace/GameplaySetupViewController.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"

#include "System/Action_2.hpp"
#include "System/Nullable_1.hpp"

#include "custom-types/shared/delegate.hpp"

#include "misc.hpp"
#include "main.hpp"


namespace endless {
	GlobalNamespace::BeatmapDifficulty string_to_difficulty(std::string string) {
		// don't know why C++ doesn't have string switches
		if(string == "Easy")
			return GlobalNamespace::BeatmapDifficulty::Easy;
		if(string == "Normal")
			return GlobalNamespace::BeatmapDifficulty::Normal;
		if(string == "Hard")
			return GlobalNamespace::BeatmapDifficulty::Hard;
		if(string == "Expert")
			return GlobalNamespace::BeatmapDifficulty::Expert;
		if(string == "Expert+")
			return GlobalNamespace::BeatmapDifficulty::ExpertPlus;
		PaperLogger.warn("string_to_difficulty() called on invalid difficulty '{}', defaulting to ExpertPlus.", string);
		return GlobalNamespace::BeatmapDifficulty::ExpertPlus;
	}
	std::string difficulty_to_string(GlobalNamespace::BeatmapDifficulty dif) {
		if(dif == GlobalNamespace::BeatmapDifficulty::Easy)
			return "Easy";
		if(dif == GlobalNamespace::BeatmapDifficulty::Normal)
			return "Normal";
		if(dif == GlobalNamespace::BeatmapDifficulty::Hard)
			return "Hard";
		if(dif == GlobalNamespace::BeatmapDifficulty::Expert)
			return "Expert";
		if(dif == GlobalNamespace::BeatmapDifficulty::ExpertPlus)
			return "Expert+";
		PaperLogger.warn("difficulty_to_string() called on invalid difficulty, with value {}.", static_cast<int32_t>(dif));
		return "Unknown";
	}
	GlobalNamespace::BeatmapCharacteristicSO *get_characteristic(std::string name) {
		auto obj = UnityEngine::Object::FindObjectOfType<GlobalNamespace::PlayerDataModel *>();
		RETURN_IF_NULL(obj, nullptr);
		return obj->_playerDataFileModel->_beatmapCharacteristicCollection->GetBeatmapCharacteristicBySerializedName(name);
	}	
	bool start_level(LevelParams params) {
		PaperLogger.info("Starting level '{}'...", params.level->songName);
		auto mth = UnityEngine::Object::FindObjectOfType<GlobalNamespace::MenuTransitionsHelper *>();
		RETURN_IF_NULL(mth, false);
		auto splsfc = UnityEngine::Object::FindObjectOfType<GlobalNamespace::SinglePlayerLevelSelectionFlowCoordinator *>();
		RETURN_IF_NULL(splsfc, false);
		auto beatmapKey = GlobalNamespace::BeatmapKey(params.characteristic, params.difficulty, params.level->levelID);
		mth->StartStandardLevel(
			"Endless",
			beatmapKey,
			params.level,
			splsfc->_gameplaySetupViewController->environmentOverrideSettings,
			splsfc->_gameplaySetupViewController->colorSchemesSettings->GetOverrideColorScheme(),
			params.level->GetColorScheme(params.characteristic, params.difficulty),
			splsfc->gameplayModifiers,
			splsfc->playerSettings,
			nullptr,
			splsfc->_environmentsListModel,
			"Exit Endless",
			false,
			false,
			nullptr,
			nullptr,
			// ::System::Action_2< ::UnityW< ::GlobalNamespace::StandardLevelScenesTransitionSetupDataSO>, ::GlobalNamespace::LevelCompletionResults *> *
			custom_types::MakeDelegate<
				System::Action_2<UnityW<GlobalNamespace::StandardLevelScenesTransitionSetupDataSO>, GlobalNamespace::LevelCompletionResults *>*
			>(
				std::function([splsfc](UnityW<GlobalNamespace::StandardLevelScenesTransitionSetupDataSO> slstss, GlobalNamespace::LevelCompletionResults *lcr) -> void {
					splsfc->HandleStandardLevelDidFinish(slstss, lcr);
					// on_level_end(lcr);
				})
			),
			nullptr,
			System::Nullable_1<GlobalNamespace::RecordingToolManager::SetupData>(false, {})
		);
		PaperLogger.info("Level started.");
		return true;
	}
	const std::vector<std::string> incompatible_mods = {"Replay"};
	std::vector<std::string> enabled_incompatible_mods = {};
	void check_for_incompatible_mods(void) {
		enabled_incompatible_mods.clear();
		for(auto &mod_result : modloader::get_all()) {
			if(auto mod_info = std::get_if<modloader::ModData>(&mod_result)) {
				auto id = mod_info->info.id;
				for(std::string iid : incompatible_mods) {
					if(id == iid) {
						enabled_incompatible_mods.push_back(id);
						PaperLogger.warn("Incompatible mod {} found.", id);
						goto continue_outer; // no labelled continue in C++
					}
				}
			}
			continue_outer:;
		}
	}
}
