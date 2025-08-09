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

#pragma once

#include <string>
#include <iostream>
#include <cctype>
#include <vector>
#include <memory>

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#endif
#include <esp_log.h>

#include "tokens.h"

using ListTokens = std::vector<std::unique_ptr<const Token>>;

class Interpreter {

public:
/**
 * Lexer
 * @param line a string contening all char to be traeted by the lexer.
 */
  std::unique_ptr<ListTokens> tokenizeLine(const std::string_view line) {

    auto tokens = std::make_unique<ListTokens>();

    size_t start = 0;
    while (start < line.size()) {
      size_t end = 0;
      std::unique_ptr<const Token> token;
//      const auto sub = std::string{line}.substr(start);
      const std::string_view sub = line.substr(start);
      ESP_LOGD("LEXER", "start: %i, sub '%.*s'", start, int(sub.size()), sub.data());
      if ((token = Token::parseSpaces(sub, end)) ||
//          (token = Token::parseInstruction(sub, end)) ||
// Ajouter ici le parsing des fonctions
          (token = Token::parseIdentifier(sub, end)) ||

          (token = Token::parseNum(sub, end)) ||
          (token = Token::parseString(sub, end)) ||
          (token = Token::parseSeparator(sub, end)) ||
          (token = Token::parseOperator(sub, end)))
      {
        start += end;
        ESP_LOGD("LEXER", "Token: '%s'", token->to_string().c_str());
        tokens->emplace_back(std::move(token));
      } else {
        ESP_LOGE("LEXER", "No token detected in '%.*s'", int(sub.size()), sub.data());
        return nullptr;
      }
    }
    ESP_LOGD("LEXER", "Tokenization complete. %zu tokens found.", tokens->size());
    return tokens;
  }


};
