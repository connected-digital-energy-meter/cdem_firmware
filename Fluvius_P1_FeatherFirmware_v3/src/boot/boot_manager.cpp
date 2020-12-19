#include "boot_manager.h"
#include "../../hardware.h"
#include "boot_config.h"

namespace SmartMeter {

  BootManager::BootManager(void) {
    pinMode(BOOT_PIN, INPUT_PULLUP);
  }

  Configuration BootManager::boot(void) {
    // Loading the EEPROM or factory default configuration settings 
    SerialDebug.println("Loading configuration ...");
    if (configManager.load_configuration()) {
      SerialDebug.println("Loaded existing configuration");
    } else {
      SerialDebug.println("No existing configuration could be loaded");
      SerialDebug.println("Saving factory default configuration");
      configManager.factory_default();
      save_config();
    }

    SerialDebug.println("Hold the touch if you wish to boot into boot menu");
    SerialDebug.print("Booting in");
    for (int i = BOOT_MENU_TIME; i >= 0; i--) {
      SerialDebug.print(" ... " + String(i));
      if (!digitalRead(BOOT_PIN)) {
        SerialDebug.println("");
        show_boot_menu();
        break;
      }
      delay(1000);
    }

    SerialDebug.println("");

    return configManager.current_config();
  }

  void BootManager::show_boot_menu(void) {
    BootConfig bootConfig(configManager.current_config(), &SerialDebug);
    Configuration config = bootConfig.Enable_Bootmenu();    // TODO - Save config if newer

    if (config != this->configManager.current_config()) {
      SerialDebug.println("User has change config. Saving it ...");
      configManager.current_config(config);
      save_config();
    } else {
      SerialDebug.println("Config not altered. No need for saving it.");
    }
  }

  void BootManager::save_config(void) {
    if (configManager.save_configuration()) {
      SerialDebug.println("Successfully saved configuration");
    } else {
      SerialDebug.println("Something went wrong. Could not save configuration");
    }
  }

};