
#include "app.h"

App app;

void setup() {
  esp_log_level_set("*", ESP_LOG_INFO);

  app.setup_uart();
  app.setup_console();
  ESP_ERROR_CHECK( app.setup_FS("/FS") );
  app.copyright();
}

void loop() {
  app.run_console("ESP32> ");
}
