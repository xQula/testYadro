#pragma once

#include <cstdlib>
#include <string>
#include <string_view>
#include <format>
#include <iostream>
#include <chrono>
#include <filesystem>

namespace yuliy_test_task::common
{

  /**
   * Trims leading and trailing whitespace from a string.
   *
   * @param str The string to trim.
   *
   * @return The trimmed string.
   *
   * @throws None.
   */
  [[nodiscard]] auto trimmed(std::string_view str) -> std::string;

  /**
   * Terminates the program with an error message.
   *
   * @param code The exit code of the program.
   * @param message The error message to be displayed.
   * @param args The arguments to be formatted into the error message.
   *
   * @return None.
   *
   * @throws None.
   */
  template <typename... Args>
  [[noreturn]] inline auto panic(int code, std::format_string<Args...> message, Args&&... args) -> void {
    std::cerr << std::format(message, std::forward<Args>(args)...) << std::endl;
    std::cin.get();
    std::exit(code);
  }

  /**
   * Prints a formatted message to the standard output.
   *
   * @param message The format string to be used for formatting the message.
   * @param args The arguments to be formatted into the message.
   *
   * @return None.
   *
   * @throws None.
   */
  template <typename... Args>
  inline auto println(std::format_string<Args...> message, Args&&... args) -> void {
    std::cout << std::format(message, std::forward<Args>(args)...) << std::endl;
  }

  /**
   * Prints a newline character to the standard output.
   *
   * This function is equivalent to `std::cout << std::endl;`.
   *
   * @return None.
   *
   * @throws None.
   */
  inline auto println() -> void {
    std::cout << std::endl;
  }

  /**
   * Prints the progress of a task to the standard output.
   *
   * @param current The current progress of the task.
   * @param total The total amount of work for the task.
   *
   * @return None.
   *
   * @throws None.
   */
  inline auto print_progress(int current, int total) -> void {
    auto const percent = static_cast<double>(current) / static_cast<double>(total) * 100.0;
    std::cout << "\r\033[2K" << std::format("Progress: \033[1;32m{:>5.2f}%\033[0m (\033[0;34m{}/{}\033[0m)", percent, current, total);
  }

  /**
   * Delays the execution of the current thread for a specified amount of time.
   *
   * @param duration The duration of the delay in microseconds.
   *
   * @return None.
   *
   * @throws None.
   */
  auto delay(std::chrono::microseconds duration) -> void;

  /**
   * Canonicalizes a path.
   *
   * If the path is absolute, the function returns it as is.
   * If the path is relative, the function returns the absolute path that is
   * relative to the current working directory.
   *
   * @param path The path to canonicalize.
   *
   * @return The canonicalized path.
   *
   * @throws None.
   */
  [[nodiscard]] auto canonicalize(std::string_view path) -> std::filesystem::path;

  /**
   * Generates a random string of a specified length.
   *
   * The generated string contains only characters from the following set:
   * - Uppercase letters (A-Z)
   * - Lowercase letters (a-z)
   * - Digits (0-9)
   * - Hyphen (-)
   * - Underscore (_)
   *
   * @param length The length of the string to generate.
   *
   * @return The generated string.
   *
   * @throws None.
   */
  [[nodiscard]] auto random_string(std::size_t length) -> std::string;
} // namespace yuliy_test_task::common
