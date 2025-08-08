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
#include <memory>

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#endif
#include <esp_log.h>

#pragma once

class TokenSpaces;
class TokenString;
class TokenInstruction;

class Token {
public:
  Token(const std::string_view aToken): token(aToken) {};
  virtual ~Token() = default;

  virtual std::string to_string() const {
    return token;
  }

  static std::unique_ptr<const TokenSpaces> parseSpaces(const std::string_view line, size_t& end);
  static std::unique_ptr<const Token> parseNum(const std::string_view line, size_t& end);
  static std::unique_ptr<const TokenString> parseString(const std::string_view line, size_t& end);
  static std::unique_ptr<const TokenInstruction> parseInstruction(const std::string_view line, size_t& end);
  static std::unique_ptr<const Token> parseSeparator(const std::string_view line, size_t& end);

protected:
  const std::string token;

  static long int parseInt(const std::string_view line, size_t& end) {
    if (!line.size()) { end = 0; return 0; }

    int res = 0;
    bool neg = false;
    int pos = 0;
    if (line[0] == '-') { neg = true; ++pos; }
    else if (line[0] == '+') ++pos;

    while (pos < line.size()) {
      if (std::isdigit(line[pos])) {
        res = res * 10 + (line[pos] - '0');
      } else break;
      ++pos;
    }
    end = pos;
    return (neg ? - res : res);
  };


};

class TokenSpaces : public Token {
public:
  TokenSpaces(const std::string_view aToken) : Token(aToken) {};

};

class TokenInteger : public Token {
public:
  TokenInteger(const std::string_view aToken, const int aValue) : Token(aToken), value(aValue) {};

private:
  const int value;

};

class TokenCommand : public Token {
public:
  TokenCommand(const std::string_view aToken, const int aCmd) : Token(aToken), cmd(aCmd) {};

private:
  const int cmd;
};

class TokenString : public Token {
public:
  TokenString(const std::string_view aToken) : Token(aToken) {};

  std::string to_string() const override {
    return '"' + token + '"';
  }

};

class TokenInstruction : public Token {
public:
  TokenInstruction(const std::string_view aToken, const unsigned aInstruction) : Token(aToken), instruction(aInstruction) {};

private:
  const unsigned instruction;
};

class TokenSeparator : public Token {
public:
  TokenSeparator(const char aToken) : Token(std::string{1, aToken}) {};

};
