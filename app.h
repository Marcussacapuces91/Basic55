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
#include "interpreter.h"

#pragma once

class App {
public:

/**
 * Setup UART device over USB.
 */
  void setup_uart();

/**
 * Setup LittleFS.
 * @param[in] label spiffs partition's label.
 * @param[in] path path defined to access the local FS.
 * @return error code or ESP_OK if no error.
 */
  int setup_FS(const std::string& label, const std::string& path = "/FS");

/**
 * Setup ESP_Console.
 */
  void setup_console();

/**
 * Display a Copyright information and some technicals informations like free memory.
 */
  void copyright() const;

/**
 * Provide a prompt and treat the answer accordingly.
 * @param[in] prompt Prompt given before each command.
 */
  void run_console(const std::string& prompt);

  Interpreter inter;

protected:
/**
 * Treat "FILES" command and return a list of files, following the given filter.
 * @param path Current path for list of files.
 * @param filter a filter to reduce the list of files.
 * @return 0 if OK, any value in case of an error.
 */
  int cmd_files(const std::string& path, const std::string& filter = "*.*");

/**
 * Treat "DATE" command and return or set date & time.
 * @return 0 if OK, any value in case of an error.
 */
  int cmd_date(const std::string& date = "", const std::string& time = "");

  int cmd_write_text_file(const std::string& filename);

  int cmd_type_file(const std::string& filename);

/**
 * Scan the known commands and call the right callback in regard.
 * @param argc Number of valid args.
 * @param argv array of args.
 * @return 0 if OK, any value in case of an error.
 */
  int exec_command(const int argc, const char *const argv[]);
  
private:

  static constexpr const char* SETUP_TAG = "SETUP";
  static constexpr const char* LOOP_TAG = "LOOP";

};
