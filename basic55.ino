/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.  
 */

#include "app.h"

#include <fstream>
#include <iostream>
#include <driver/gpio.h>

#define BLINK_GPIO gpio_num_t(22)

App app;

void parse_file(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    ESP_LOGE("SETUP", "Error opening file %s!", filename.c_str());
    return;
  }

  std::string s;
  while (getline(file, s)) {
    gpio_set_level(BLINK_GPIO, 0);
    ESP_LOGD("SETUP", "getline: %s", s.c_str());
    const auto pTokens = app.inter.tokenizeLine(s);
    gpio_set_level(BLINK_GPIO, 1);
    if (pTokens) {
      // for (const auto& pToken: *pTokens)
      //   std::cout << pToken->to_string();
      // std::cout << std::endl;  
    } else {
      ESP_LOGE("SETUP", "Error parsing line %s", s.c_str());
    }
  }
  file.close();
}

void setup() {
  esp_log_level_set("*", ESP_LOG_INFO);

  app.setup_uart();
  app.setup_console();
  ESP_ERROR_CHECK( app.setup_FS("spiffs", "/FS") );
  app.displayCopyright();

// Configure the GPIO pin
  gpio_reset_pin(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

  char s[20];
  for (int i = 1; i <= 208; ++i) {
    snprintf(s, 20, "/FS/P%03d.BAS", i);
    ESP_LOGI("SETUP", "Parsing file %s", s);
    parse_file(s);
  };


}

void loop() {
  app.run_console("ESP32> ");
}
