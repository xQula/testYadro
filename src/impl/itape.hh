#pragma once

#include <cstdint>
#include <concepts>
#include <expected>
#include <vector>
#include <impl/config.hh>

namespace yuliy_test_task
{
  template <typename T>
  concept TapeElement = std::equality_comparable<T>
    and std::is_standard_layout_v<T>
    and std::is_trivial_v<T>;

  template <TapeElement T>
  class ITape
  {
   public:
    template <typename U>
    using result_type = std::expected<U, std::string>;
    using value_type = T;

    enum class Direction
    {
      Left,
      Right
    };

    virtual ~ITape() = default;

    /**
    * Reads a single value from the tape.
    *
    * @return The read value.
    */
    [[nodiscard]] virtual auto read() const -> value_type = 0;

    /**
    * Reads and shifts a single value from the tape.
    *
    * @return The read value.
    */
    [[nodiscard]] virtual auto read_and_shift() -> value_type = 0;

    /**
    * Reads and shifts n values from the tape.
    *
    * @param n The number of values to read and shift.
    * @return The read values.
    */
    [[nodiscard]] virtual auto read_and_shift_n(std::size_t n) -> result_type<std::vector<value_type>> = 0;

    /**
    * Shifts the tape in the specified direction.
    *
    * @param direction The direction to shift the tape.
    * @return `true` if the shift was successful, `false` otherwise.
    */
    [[nodiscard]] virtual auto shift(Direction direction) -> bool = 0;

    /**
    * Writes a single value to the tape.
    *
    * @param value The value to write.
    */
    virtual auto write(value_type value) -> void = 0;

    /**
    * Writes and shifts a single value to the tape.
    *
    * @param value The value to write and shift.
    */
    virtual auto write_and_shift(value_type value) -> void = 0;

    /**
    * Writes and shifts n values to the tape.
    *
    * @param values The values to write and shift.
    */
    [[nodiscard]] virtual auto write_and_shift_n(std::vector<value_type> const& values) -> result_type<void> = 0;

    /**
    * Rewinds the tape to the beginning.
    */
    virtual auto rewind() -> void = 0;

    /**
    * Checks if the tape is at the end of the file.
    *
    * @return `true` if the tape is at the end of the file, `false` otherwise.
    */
    [[nodiscard]] virtual auto eof() const -> bool = 0;

    /**
    * Checks if the tape is empty.
    *
    * @return `true` if the tape is empty, `false` otherwise.
    */
    [[nodiscard]] virtual auto empty() const -> bool = 0;

    /**
    * Returns the size of the tape.
    *
    * @return The size of the tape.
    */
    [[nodiscard]] virtual auto size() const -> std::size_t = 0;


    /**
    * Returns the filename of the tape.
    *
    * @return The filename of the tape.
    */
    [[nodiscard]] virtual auto filename() const -> std::filesystem::path const& = 0;

    /**
    * Returns the configuration of the tape.
    *
    * @return The configuration of the tape.
    */
    [[nodiscard]] virtual auto config() const -> Config const& = 0;
  };
} // namespace yuliy_test_task
