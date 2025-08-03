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

App app;

void setup() {
  esp_log_level_set("*", ESP_LOG_INFO);

  app.setup_uart();
  app.setup_console();
  ESP_ERROR_CHECK( app.setup_FS("spiffs", "/FS") );
  app.copyright();

  ESP_LOGI("SETUP", "Openging P001.BAS");
  std::ifstream file{"/FS/P001.BAS"};
  if (!file.is_open()) {
    ESP_LOGE("SETUP", "Error opening file!");
    return;
  }
  std::string s;
  while (getline(file, s)) {
    std::cout << s << std::endl;
    app.inter.lexer(s);
  }
  file.close();
}

void loop() {
  app.run_console("ESP32> ");
}
