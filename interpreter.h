#include <memory>
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
#include <vector>

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#endif
#include <esp_log.h>

#include "tokens.h"

#pragma once

using ListTokens = std::vector<std::unique_ptr<const Token>>;

class Interpreter {

public:
/**
 * Lexer
 * @param line a string contening all char to be traeted by the lexer.
 */
  std::unique_ptr<ListTokens> lexer(const std::string_view line) {

    auto tokens = std::make_unique<ListTokens>();

    size_t start = 0;
    while (start < line.size()) {
      size_t end = 0;
      std::unique_ptr<const Token> token;
      const auto sub = std::string{line}.substr(start);
      ESP_LOGD("LEXER", "start: %i, sub '%s'", start, sub.c_str());
      if ((token = Token::parseSpaces(sub, end)) ||
          (token = Token::parseNum(sub, end)) ||
          (token = Token::parseString(sub, end)) ||
          (token = Token::parseInstruction(sub, end)) ||
          (token = Token::parseSeparator(sub, end))) {
        start += end;
        if (!token) ESP_LOGI("LEXER", "Token NULL!");
        ESP_LOGD("LEXER", "Token: '%s'", token->to_string().c_str());
        tokens->emplace_back(std::move(token));
      } else {
        ESP_LOGE("LEXER", "No token detected in \"%s\"!", sub.c_str());
        return nullptr;
      }
    }

    return tokens;
  }


};
