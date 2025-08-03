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

#include <string>
#include <iostream>
#include <cctype>
#include <esp_log.h>

#pragma once

class Interpreter {

public:
  void lexer(const std::string& aLine) {
    for (c = aLine.begin() ; !aLine.end() ;) {
      if (std::isspace(c) {
        ESP_LOGI("LEXER", "Space (%c)", c);
      } else if (std::isalpha(c)) {
        ESP_LOGI("LEXER", "Alpha (%c)", c);
      } else if (std::isdigit(c)) {
        ESP_LOGI("LEXER", "Digit (%c)", c);



      } else if (std::ispunct(c)) {
        ESP_LOGI("LEXER", "Ponctu (%c)", c);
      } else {
        ESP_LOGW("LEXER", "Unkonwn type (%c)", c);
      }
    }
  }


};
