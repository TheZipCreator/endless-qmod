#include "main.hpp"

#include "scotland2/shared/modloader.h"
#include "bsml/shared/BSML.hpp"
#include "modconfig.hpp"

#include "menu.hpp"
#include "endless.hpp"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

Configuration &getConfig() {
  static Configuration config(modInfo);
  return config;
}

MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
  *info = modInfo.to_c();

  getConfig().Load();

  // File logging
  Paper::Logger::RegisterFileContextId(PaperLogger.tag);

  PaperLogger.info("Completed setup!");
}


MOD_EXTERN_FUNC void late_load() noexcept {
  il2cpp_functions::Init();
	BSML::Init();
	BSML::Register::RegisterGameplaySetupTab("Endless", &endless::did_activate);
	getModConfig().Init(modInfo);

  PaperLogger.info("Installing hooks...");
	endless::register_hooks();
  PaperLogger.info("Installed all hooks!");

	endless::check_for_incompatible_mods();
}
