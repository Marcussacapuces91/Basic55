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

#include <esp_log.h>
#include <string>
#include <memory>
#include "tokens.h"


std::unique_ptr<const TokenSpaces> Token::parserSpaces(const std::string& line, size_t& end) {
  std::string s;
  size_t pos = 0;
  while (pos < line.size()) {
    if (std::isspace(line[pos])) s += line[pos];
    else break;
    ++pos;
  }
  end = pos;
  if (pos > 0) return std::make_unique<TokenSpaces>(s); else return nullptr;
};

std::unique_ptr<const Token> Token::parserNum(const std::string& line, size_t& end) {
  if (!line.size()) return nullptr;
  if (std::isdigit(line[0]) || (line[0] == '.') || (std::toupper(line[0]) == 'E')) {
    size_t end_int;
    const auto res = parserInt(line, end_int);
    std::string s{line.substr(0, end_int)};

    if (end_int < line.size()) {
      if (line[end_int+1] == '.') {  // Float
        ESP_LOGE("LEXER", "UN POINT DECIMAL! TODO!");
        return nullptr;
      } else if (std::toupper(line[end_int+1]) == 'E') { // Float with Exp
        ESP_LOGE("LEXER", "UN EXPOSANT! TODO!");
        return nullptr;
      }      
    }
    end = s.size();
    return std::unique_ptr<const Token>(new TokenInteger{s, res});
  } else {
    return nullptr;
  }
};
