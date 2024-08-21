#pragma once

#include <filesystem>
#include <expected>
#include <chrono>
#include <string>
#include <string_view>
#include <format>
#include <sstream>

namespace yuliy_test_task
{
  using namespace std::chrono_literals;
  class Config
  {
    public:
      static constexpr inline auto default_filename = std::string_view("config.ini");

      [[nodiscard]] static auto from_pwd() -> std::expected<Config, std::string>;
      [[nodiscard]] static auto load(std::filesystem::path const& path) -> std::expected<Config, std::string>;

      /**
       * Calculates the maximum number of elements of type T that can fit within the configured RAM limit.
       *
       * @tparam T The type of elements to calculate the limit for.
       *
       * @return The maximum number of elements of type T that can fit within the configured RAM limit.
       */
      template <typename T>
      constexpr auto ram_limit_elems() const noexcept -> T {
        return this->ram_limit_ / sizeof(T);
      }

      /**
       * Returns the configured RAM limit in bytes.
       *
       * @return The configured RAM limit in bytes.
       */
      [[nodiscard]] constexpr auto ram_limit_bytes() const noexcept -> std::size_t { return this->ram_limit_; }

      /**
       * Returns the configured read delay.
       *
       * @return The configured read delay in microseconds.
       */
      [[nodiscard]] constexpr auto read_delay() const noexcept -> std::chrono::microseconds { return this->read_delay_; }

      /**
       * Returns the configured write delay.
       *
       * @return The configured write delay in microseconds.
       */
      [[nodiscard]] constexpr auto write_delay() const noexcept -> std::chrono::microseconds { return this->write_delay_; }

      /**
       * Returns the configured tape shift delay.
       *
       * @return The configured tape shift delay in microseconds.
       */
      [[nodiscard]] constexpr auto tape_shift_delay() const noexcept -> std::chrono::microseconds { return this->tape_shift_delay_; }

      /**
       * Returns the configured tape rewind delay.
       *
       * @return The configured tape rewind delay in microseconds.
       */
      [[nodiscard]] constexpr auto tape_rewind_delay() const noexcept -> std::chrono::microseconds { return this->tape_rewind_delay_; }

      friend auto operator<<(std::ostream& os, Config const& self) -> std::ostream&;

    private:
      Config() = default;

      std::size_t ram_limit_ = 1024 * 1024 * 1024;
      std::chrono::microseconds read_delay_ = 2us;
      std::chrono::microseconds write_delay_ = 2us;
      std::chrono::microseconds tape_shift_delay_ = 10us;
      std::chrono::microseconds tape_rewind_delay_ = 100us;
  };
} // namespace yuliy_test_task

template <>
struct std::formatter<yuliy_test_task::Config, char>
{
  /**
   * Parses a Config object from a format-parse context.
   *
   * Since the Config object is not formattable from a string, this function simply returns the beginning of the context.
   *
   * @param ctx The format-parse context.
   * @return The beginning of the context.
   */
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx) -> ParseContext::iterator { return ctx.begin(); }

  /**
   * Formats a Config object into a format context.
   *
   * @param config The Config object to be formatted.
   * @param ctx The format context to write to.
   *
   * @return The end iterator of the format context.
   */
  template <typename FormatContext>
  auto format(yuliy_test_task::Config config, FormatContext& ctx) const -> FormatContext::iterator {
    auto os = std::stringstream();
    os << config;
    return std::ranges::copy(std::move(os).str(), ctx.out()).out;
  }
};