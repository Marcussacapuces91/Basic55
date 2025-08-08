#include <memory>
#include <string_view>
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

// #include <string>
// #include <memory>
#include <vector>
#include <iostream>

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#endif
#include <esp_log.h>

#include "tokens.h"


std::unique_ptr<const TokenSpaces> Token::parseSpaces(const std::string_view line, size_t& end) {
  ESP_LOGD("LEXER", "Trying to parse spaces...");
  if (line.empty()) return nullptr; // empty

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

std::unique_ptr<const Token> Token::parseNum(const std::string_view line, size_t& end) {
  ESP_LOGD("LEXER", "Trying to parse numeric...");
  if (line.empty()) return nullptr; // empty
  if (!std::isdigit(line[0]) && (line[0] != '.') && (line[0] != '-')) return nullptr; // not a number

  size_t end_int;
  const auto res = parseInt(line, end_int);
  std::string s{line.substr(0, end_int)};

  ESP_LOGD("PARSE NUM", "s=%s, res=%i", s.c_str(), res);

  if (end_int < line.size()) {
    if (line[end_int] == '.') {  // Float
      ESP_LOGE("LEXER", "UN POINT DECIMAL! TODO!");
      return nullptr;
    } else if (std::toupper(line[end_int]) == 'E') { // Float with Exp
      ESP_LOGE("LEXER", "UN EXPOSANT! TODO!");
      return nullptr;
    }
  }

  end = s.size();
  return std::unique_ptr<const Token>(new TokenInteger{s, res});
};

std::unique_ptr<const TokenString> Token::parseString(const std::string_view line, size_t& end) {
  ESP_LOGD("LEXER", "Trying to parse string...");
  if (line.empty() || (line[0] != '"')) return nullptr; // empty
  std::string s;
  size_t pos = 1;
  while (pos < line.size()) {
    const auto c = line[pos];
    if ((c == '\\') && (pos + 1 < line.size())) {
      if (line[pos + 1] == '\\') { s += c; pos += 2; continue; }
      else if (line[pos + 1] == '"') { s += '"'; pos += 2; continue; }
      ESP_LOGE("LEXER", "Unknown escaped sequence!");
      return nullptr;
    } else if (c == '"') break;
    else { s += c; ++pos; }
  }

//  ESP_LOGI("PARSE STR", "s=%s, pos=%i, c=%c", s.c_str(), pos, line[pos]);

  if (line[pos] == '"') {
    end = pos + 1;
    return std::make_unique<const TokenString>(s);
  } else {
    ESP_LOGE("LEXER", "String unclosed!");
    return nullptr;
  }
};

std::unique_ptr<const TokenInstruction> Token::parseInstruction(const std::string_view line, size_t& end) {
  ESP_LOGD("LEXER", "Trying to parse instruction...");

  static const std::vector<std::vector<std::pair<int, std::string_view>>> cmds = {
    { }, // A
    { { 21, "BASE" } }, // B
    { }, // C
    { { 18, "DATA" }, { 19, "DIM" }, { 24, "DEF" } }, // D
    { { 2, "END" }, { 9, "ELSE" } }, // E
    { { 12, "FOR" } }, // F
    { { 4, "GO" } }, // G
    { }, // H
    { { 7, "IF" }, { 15, "INPUT" } }, // I
    { }, // J
    { }, // K
    { { 3, "LET" } }, // L
    { }, // M
    { { 14, "NEXT" } }, // N
    { { 10, "ON" }, { 20, "OPTION" } }, // O
    { { 1, "PRINT" } },
    { }, // Q
    { { 16, "READ" }, { 17, "RESTORE" }, { 22, "REM" }, { 23, "RANDOMIZE" }, { 25, "RETURN" } }, // R
    { { 6, "SUB" }, { 11, "STOP" }, { 13, "STEP" } }, // S
    { { 5, "TO" }, { 8, "THEN" } }, // T
    { }, // U
    { }, // V
    { }, // W
    { }, // X
    { }, // Y
    { }, // Z
  };

  if (!line.size() || !std::isalpha(line[0])) return nullptr;

  const auto c = std::toupper(line[0]);
  for (auto& cm : cmds[c - 'A']) {
    std::string ins;
    for(const char& c: line.substr(0, cm.second.size())) ins += std::toupper(c);
    if (cm.second == ins) {
      end = cm.second.size();
      return std::make_unique<const TokenInstruction>(line.substr(0, cm.second.size()), cm.first);
    }
  }
  return nullptr;
};

std::unique_ptr<const Token> Token::parseSeparator(const std::string_view line, size_t& end) {
  ESP_LOGD("LEXER", "Trying to parse separator...");
  if (line.empty()) return nullptr; // empty

  if (line[0] == ';' || line[0] == ',' || line[0] == ':') {
    end = 1;
    return std::make_unique<const TokenSeparator>(line[0]);
  }
  return nullptr;
};
