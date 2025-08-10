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

#include "esp_system.h"
#include <esp_idf_version.h>

#include "interpreter.h"

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
  int setup_FS(const std::string& partitionLabel, const std::string& path = "/FS");

/**
 * Setup ESP_Console.
 */
  void setup_console();

/**
 * Display a Copyright information and some technicals informations like free memory.
 */
  void displayCopyright() const;

/**
 * Provide a prompt and treat the answer accordingly.
 * @param[in] prompt Prompt given before each command.
 */
  void run_console(const std::string& prompt);

  void displayResetReason() {
    const auto reason = esp_reset_reason();
    switch (reason) {
      ESP_RST_UNKNOWN:
        std::cout << "Reset reason can not be determined." << std::endl;
        break;
      ESP_RST_POWERON:
        std::cout << "Reset due to power-on event." << std::endl;
        break;
      ESP_RST_EXT:
        std::cout << "Reset by external pin (not applicable for ESP32)" << std::endl;
        break;
      ESP_RST_SW:
        std::cout << "Software reset via esp_restart." << std::endl;
        break;
      ESP_RST_PANIC:
        std::cout << "Software reset due to exception/panic." << std::endl;
        break;
      ESP_RST_INT_WDT:
        std::cout << "Reset (software or hardware) due to interrupt watchdog." << std::endl;
        break;
      ESP_RST_TASK_WDT:
        std::cout << "Reset due to task watchdog." << std::endl;
        break;
      ESP_RST_WDT:
        std::cout << "Reset due to other watchdogs." << std::endl;
        break;
      ESP_RST_DEEPSLEEP:
        std::cout << "Reset after exiting deep sleep mode." << std::endl;
        break;
      ESP_RST_BROWNOUT:
        std::cout << "Brownout reset (software or hardware)" << std::endl;
        break;
      ESP_RST_SDIO:
        std::cout << "Reset over SDIO." << std::endl;
        break;
      ESP_RST_USB:
        std::cout << "Reset by USB peripheral." << std::endl;
        break;
      ESP_RST_JTAG:
        std::cout << "Reset by JTAG." << std::endl;
        break;
      ESP_RST_EFUSE:
        std::cout << "Reset due to efuse error." << std::endl;
        break;
      ESP_RST_PWR_GLITCH:
        std::cout << "Reset due to power glitch detected." << std::endl;
        break;
      ESP_RST_CPU_LOCKUP:
        std::cout << "Reset due to CPU lock up (double exception)" << std::endl;
        break;
      // 0x0C:
      //   std::cout << "Software CPU Reset." << std::endl;
      //   break;
      default:
        std::cout << "Other reason (" << reason << ")" << std::endl;
    }

  }

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

  int cmd_reboot();

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
