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

#pragma once

class TokenSpaces;

class Token {
public:
  Token(const std::string& aToken): token(aToken) {};
  virtual ~Token() = default;

  static std::unique_ptr<const TokenSpaces> parserSpaces(const std::string& line, size_t& end);
  static std::unique_ptr<const Token> parserNum(const std::string& line, size_t& end);

protected:
  const std::string token;

  static long int parserInt(const std::string& line, size_t& end) {
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
  TokenSpaces(const std::string& aToken) : Token(aToken) {};

};

class TokenInteger : public Token {
public:
  TokenInteger(const std::string& aToken, const int aValue) : Token(aToken), value(aValue) {};

private:
  const int value;

};

class TokenCommand : public Token {
public:
  TokenCommand(const std::string& aToken, const int aCmd) : Token(aToken), cmd(aCmd) {};

private:
  const int cmd;
};
