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
 
#include <ostream>
#include "app.h"

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#endif
#include <esp_log.h>
#include <esp_console.h>
#include <linenoise/linenoise.h>
#include "soc/soc_caps.h"
#include "driver/uart_vfs.h"
#include "driver/uart.h"
#include "esp_vfs_cdcacm.h"
#include "esp_littlefs.h"
#include <cerrno>
#include <cctype>
#include <fnmatch.h>
#include <string>
#include <ctime>
#include <filesystem>
#include <iostream>
// #include <format>
#include <fstream>
#include <cerrno>

void App::setup_uart() {
  /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
  ESP_ERROR_CHECK( uart_vfs_dev_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR) );
  /* Move the caret to the beginning of the next line on '\n' */
  ESP_ERROR_CHECK( uart_vfs_dev_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF) );

  /* Configure UART. Note that REF_TICK is used so that the baud rate remains
   * correct while APB frequency is changing in light sleep mode.
   */
  const uart_config_t uart_config = {
          .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
          .data_bits = UART_DATA_8_BITS,
          .parity = UART_PARITY_DISABLE,
          .stop_bits = UART_STOP_BITS_1,
#if SOC_UART_SUPPORT_REF_TICK
          .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
          .source_clk = UART_SCLK_XTAL,
#endif
  };
  /* Install UART driver for interrupt-driven reads and writes */
  ESP_ERROR_CHECK( uart_driver_install((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0) );
  ESP_ERROR_CHECK( uart_param_config((uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );

  /* Tell VFS to use UART driver */
  uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);
  /* Disable buffering on stdin */
  ESP_ERROR_CHECK( setvbuf(stdin, NULL, _IONBF, 0) );
}

int App::setup_FS(const std::string& partitionLabel, const std::string& path) {
  const esp_vfs_littlefs_conf_t config = {
    .base_path = path.c_str(),
    .partition_label = partitionLabel.c_str(),
    .format_if_mount_failed = true,
    .dont_mount = false
  };
  ESP_LOGI(SETUP_TAG, "Registering %s partition as %s", config.partition_label, config.base_path);
  const auto reg = esp_vfs_littlefs_register(&config);
  ESP_LOGI(SETUP_TAG, "Registered littlefs %i", reg);
  switch (reg) {
    case ESP_OK:
      ESP_LOGI(SETUP_TAG, "FS mounted.");
      break;
    case ESP_FAIL:
      ESP_LOGE(SETUP_TAG, "Failed to mount or format filesystem");
      return reg;
    case ESP_ERR_NOT_FOUND:
      ESP_LOGE(SETUP_TAG, "Failed to find %s partition", config.partition_label);
      return reg;
    default:
      ESP_LOGE(SETUP_TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(reg));
      return reg;
  }

  size_t total, used = 0;
  const auto ret = esp_littlefs_info(config.partition_label, &total, &used);
  if (ret == ESP_OK) ESP_LOGI(SETUP_TAG, "Partition size: total: %d, used: %d", total, used);
  else {
    ESP_LOGE(SETUP_TAG, "Failed to get %s partition information (%s). Formatting...", config.partition_label, esp_err_to_name(ret));
    const auto ret = esp_littlefs_format(config.partition_label);
    ESP_ERROR_CHECK(ret);
    return ret;
  }

  return ESP_OK;
}

void App::setup_console() {
  esp_console_config_t console_config = ESP_CONSOLE_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_init(&console_config));

  linenoiseSetMultiLine(1);
  linenoiseSetCompletionCallback(&esp_console_get_completion);
  linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);
  linenoiseHistorySetMaxLen(100);
  linenoiseSetMaxLineLen(console_config.max_cmdline_length);
  linenoiseAllowEmpty(false);

#if CONFIG_CONSOLE_STORE_HISTORY
  /* Load command history from filesystem */
  linenoiseHistoryLoad(history_path);
#endif // CONFIG_CONSOLE_STORE_HISTORY

  /* Figure out if the terminal supports escape sequences */
  if (linenoiseProbe()) {         /* zero indicates success */
    linenoiseSetDumbMode(1);
    std::cout << "\n\n"
              << "Your terminal application does not support escape sequences.\n"
              << "Line editing and history features are disabled.\n"
              << "On Windows, try using Putty instead.\n\n";
  } else {
    std::cout << "\n"
              << "This is a BASIC-55 console, welcome!\n\n"
              << "Type 'help' to get the list of commands.\n"
              << "Use UP/DOWN arrows to navigate through command history.\n"
              << "Press TAB when typing command name to auto-complete.\n\n";
  }

}

void App::displayCopyright() const {
  std::cout << "MS-BASIC 0.1" << std::endl
            << "(C) Copyright M. SIBERT 2025" << std::endl;

  const auto now = time(NULL);
  char ct[30];
  if (std::strftime(ct, sizeof ct, "%c", localtime(&now))) {
    std::cout << "Time is " << ct << std::endl;
  } else {
    std::cout << "Invalid local time!" << std::endl;
  }
  std::cout << "It might be changed using the DATE command or by connecting WiFi." << std::endl
            << esp_get_free_heap_size() << " Bytes free." << std::endl
            << "Log level set to " << LOG_LOCAL_LEVEL << ". Default level " << esp_log_get_default_level() << std::endl
            << "Ok" << std::endl << std::endl;
}

int App::cmd_files(const std::string& path, const std::string& filter) {
  if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
    std::cout << "Path " << path << " unknown or not a dir!" << std::endl;
    return ESP_ERR_NOT_FOUND;
  }

  std::cout << " Directory of " << path << std::endl << std::endl;

  int files = 0;
  long sizes = 0;
  const std::filesystem::path dir{path};
  for (auto const& dir_entry : std::filesystem::directory_iterator(dir)) {
    if (!dir_entry.is_regular_file()) continue;
    const std::string fname{ std::string(dir_entry.path().stem()) + std::string(dir_entry.path().has_extension() ? dir_entry.path().extension() : "") };
    if (fnmatch(filter.c_str(), fname.c_str(), 0)) continue;

    const auto fsize = dir_entry.file_size();
    const auto ftime = dir_entry.last_write_time();
    std::time_t cftime = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(ftime));
    char mtime[25] = "undefined";
    std::strftime(mtime, sizeof(mtime), "%c", std::localtime(&cftime));

    const auto ftype = std::string(dir_entry.is_regular_file() ? "FILE" : "--- ");

    char size[20];
    if (fsize < 10240L) {
      snprintf(size, sizeof(size), "%5u bytes", (int)fsize);
    } else if (fsize < 10240L*1024) {
      snprintf(size, sizeof(size), "%5f kBytes", fsize / 1024.0);
    } else if (fsize < 10240LL*1024*1024) {
      snprintf(size, sizeof(size), "%5f MBytes", fsize / 1024 / 1024.0);
    }

    std::cout << std::setw(25) << mtime
              << std::setw(5) << ftype << " "
              << std::setw(15) << size << " "
              << fname << std::endl;
    sizes += fsize;
    ++files;
  }
  std::cout << std::endl << "\t" << files << " File(s), " << sizes << " bytes" << std::endl;
  size_t total, used;
  const auto ret = esp_littlefs_info("spiffs", &total, &used);
  ESP_ERROR_CHECK(ret);
  if (ret == ESP_OK) std::cout << "\t" << "Total size: " << total << " bytes, used: " << used << " bytes" << std::endl;

  return EXIT_SUCCESS;
}

int App::cmd_date(const std::string& date, const std::string& time) {
  struct timeval tv;
  gettimeofday(&tv, NULL);  // now -> tv
  struct tm tm;
  localtime_r(&tv.tv_sec, &tm); // now -> tm

  if (date != "") {
    const int sc = sscanf(date.c_str(), "%d/%d/%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
    tm.tm_year = tm.tm_year >= 1900 ? tm.tm_year - 1900 : tm.tm_year + 2000 - 1900;
    if (sc >= 2) --tm.tm_mon;  // 0..11
    tv.tv_sec = mktime(&tm);
    if (settimeofday(&tv, NULL)) return errno;
  }

  if (time != "") {
    const int sc = sscanf(time.c_str(), "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tv.tv_sec = mktime(&tm);
    if (settimeofday(&tv, NULL)) return errno;
  }

  std::cout << std::endl << "Time is: " << asctime(&tm) << std::endl;
  return ESP_OK;
}

int App::cmd_write_text_file(const std::string& filename) {
  std::ofstream file{filename};
  if (!file.is_open()) {
    ESP_LOGE("WRITE FILE", "Can't open file %s", filename.c_str());
    return file.rdstate();
  }

  while (true) {
    std::string s;
    std::getline(std::cin, s);
    if (s != "") {
      std::cout << s << std::endl;
      file << s << std::endl; 
    } else break;
  }

  file.close();
  return ESP_OK;
}

int App::cmd_type_file(const std::string& filename) {
  std::ifstream file{filename};
  if (!file.is_open()) {
    ESP_LOGE("WRITE FILE", "Can't open file %s", filename.c_str());
    return errno; // file.rdstate();
  }

  std::string s;
  while (std::getline(file, s)) {
    std::cout << s << std::endl;
  }

  file.close();
  return ESP_OK;
}

int App::exec_command(const int argc, const char *const argv[]) {
  if (!argc) return ESP_ERR_INVALID_SIZE;

  ESP_LOGI(LOOP_TAG, "argv[0]: %s", argv[0]);

  char* end;
  errno = 0;  // reset error
  const int num = strtoul(argv[0], &end, 10);
  if (num || errno) { // It's a number (line number)!
    ESP_LOGI(LOOP_TAG, "Line number: %i", num);
    
    return ESP_ERR_NOT_SUPPORTED;
  }

  std::string cmd_upper{argv[0]};
  for (auto& c: cmd_upper) c = (char)toupper(c);

  if (cmd_upper == "HELP") {  // Ask for help
    ESP_LOGI(LOOP_TAG, "HELP (%s)", argv[0]);
    return ESP_ERR_NOT_SUPPORTED;
  }

  if (cmd_upper == "FILES") {
    if (argc == 1) return cmd_files("/FS");
    if (argc == 2) return cmd_files("/FS", argv[1]);
    return ESP_ERR_INVALID_SIZE;
  }

  if (cmd_upper == "DATE") {
    if (argc == 1) return cmd_date();
    if (argc == 2) return cmd_date(argv[1]);
    if (argc == 3) return cmd_date(argv[1], argv[2]);
    return ESP_ERR_INVALID_SIZE;
  }

  if (cmd_upper == "WRITEFILE") {
    if (argc == 2) return cmd_write_text_file(std::string{"/FS/"} + argv[1]);
    return ESP_ERR_INVALID_SIZE;
  }

  if (cmd_upper == "TYPE") {
    if (argc == 2) return cmd_type_file(std::string{"/FS/"} + argv[1]);
    return ESP_ERR_INVALID_SIZE;
  }

  return ESP_ERR_NOT_ALLOWED;
}

void App::run_console(const std::string& prompt) {
  auto const line = linenoise(prompt.c_str());
  if (line == NULL) return;

  if (strlen(line) > 0) {
    linenoiseHistoryAdd(line);

    char* argv[10];
    const int argc = esp_console_split_argv(line, argv, sizeof argv);
    switch (const auto err = exec_command(argc, argv)) {
      case ESP_OK:
        std::cout << "Ok" << std::endl << std::endl;
        break;
      case ESP_ERR_NOT_ALLOWED:
        std::cout << "Unknown command \"" << argv[0] << "\"!" << std::endl << std::endl;
        break;
      case ESP_ERR_NOT_SUPPORTED:
        std::cout << "Command \"" << argv[0] << "\" not yet supported!" << std::endl << std::endl;
        break;
      default:
        std::cout << "Error " << err << ": " << esp_err_to_name(err) << " / " << strerror(err) << "!" << std::endl << std::endl;
    };
  }

  /* linenoise allocates line buffer on the heap, so need to free it */
  linenoiseFree(line);
}
