#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <type_traits>
#include <impl/itape.hh>
#include <impl/config.hh>
#include <impl/common.hh>
#include <impl/io.hh>

namespace yuliy_test_task
{
  template <TapeElement T, TapeIO<T> Io>
  class Tape : public ITape<T>
  {
   public:
    /**
     * Creates a new instance of the Tape class.
     *
     * @param filename The path to the file used by the Tape.
     * @param config   The configuration for the Tape.
     *
     * @return A unique pointer to the newly created Tape instance.
     */
    [[nodiscard]] static auto create(
      std::filesystem::path filename,
      Config const& config
    ) -> ITape<T>::template result_type<std::unique_ptr<ITape<T>>> {
      return std::unique_ptr<ITape<T>>(new Tape<T, Io>(std::move(filename), config));
    }

    ~Tape() override = default;

    /**
     * Reads a single value from the tape.
     *
     * @return The read value.
     */
    [[nodiscard]] auto read() const -> T override {
      common::delay(this->config().read_delay());
      return this->io_.read();
    }

    /**
     * Reads a single value from the tape and shifts the tape to the right.
     *
     * @return The read value.
     */
    [[nodiscard]] auto read_and_shift() -> T override {
      auto const res = this->read();
      std::ignore = this->shift(ITape<T>::Direction::Right);
      return res;
    }

    /**
     * Reads and shifts n values from the tape.
     *
     * @param n The number of values to read and shift.
     *
     * @return The read values.
     *
     * @throws std::runtime_error if the number of values to read exceeds the RAM limit.
     */
    [[nodiscard]] auto read_and_shift_n(std::size_t n)
      -> ITape<T>::template result_type<std::vector<T>> override {
      if(n == 0)
        return std::vector<T>();
      if(n > this->config().template ram_limit_elems<T>())
        return std::unexpected(std::format("ram limit exceeded on read: {} bytes, requested {} bytes",
          this->config().ram_limit_bytes(),
          this->config().template ram_limit_elems<T>()
        ));
      auto values = std::vector<T>();
      for(auto i = 0; i < n; ++i) {
        if(this->eof())
          break;
        values.push_back(this->read_and_shift());
      }
      if(not values.back())
        values.pop_back();
      return values;
    }

    /**
     * Shifts the tape in the specified direction.
     *
     * @param direction The direction to shift the tape.
     *
     * @return `true` if the shift was successful, `false` otherwise.
     */
    [[nodiscard]] auto shift(ITape<T>::Direction direction) -> bool override {
      common::delay(this->config().tape_shift_delay());
      return this->io_.shift(direction);
    }

    /**
     * Writes a single value to the tape.
     *
     * @param value The value to write.
     */
    auto write(T value) -> void override {
      common::delay(this->config().write_delay());
      this->io_.write(value);
    }

    /**
     * Writes a single value to the tape and shifts the tape to the right.
     *
     * @param value The value to write.
     *
     * @return None
     */
    auto write_and_shift(T value) -> void override {
      this->write(value);
      std::ignore = this->shift(ITape<T>::Direction::Right);
    }

    /**
     * Writes and shifts n values from the tape.
     *
     * @param values The values to write and shift.
     *
     *         write exceeds the RAM limit.
     *
     * @throws std::runtime_error if the number of values to write exceeds the RAM limit.
     */
    [[nodiscard]] auto write_and_shift_n(std::vector<T> const& values)
      -> ITape<T>::template result_type<void> override {
      if(values.empty())
        return {};
      if(values.size() > this->config().template ram_limit_elems<T>())
        return std::unexpected(std::format("ram limit exceeded on write: {} bytes, requested {} bytes",
          this->config().ram_limit_bytes(),
          this->config().template ram_limit_elems<T>()
        ));
      for(auto const& value : values) {
        if(this->eof())
          break;
        this->write_and_shift(value);
      }
      return {};
    }

    /**
     * Rewinds the tape to its beginning.
     *
     * @note This function is blocking and will delay the execution of the program by the configured tape rewind delay.
     */
    auto rewind() -> void override {
      common::delay(this->config().tape_rewind_delay());
      this->io_.rewind();
    }

    /**
     * Checks if the tape has reached its end.
     *
     * @return `true` if the tape has reached its end, `false` otherwise.
     */
    [[nodiscard]] auto eof() const -> bool override {
      return this->io_.end();
    }

    /**
     * Checks if the tape is empty.
     *
     * @return `true` if the tape is empty, `false` otherwise.
     */
    [[nodiscard]] auto empty() const -> bool override {
      return this->size() == 0;
    }

    /**
     * Returns the size of the tape.
     *
     * @return The number of elements in the tape.
     */
    [[nodiscard]] auto size() const -> std::size_t override {
      return this->io_.size();
    }

    /**
     * Returns the filename of the tape.
     *
     * @return The filename of the tape.
     */
    [[nodiscard]] auto filename() const -> std::filesystem::path const& override {
      return this->io_.name();
    }

    /**
     * Returns the configuration for the tape.
     *
     * @return The configuration for the tape.
     */
    [[nodiscard]] auto config() const -> Config const& override {
      return this->config_;
    }

   private:
    Tape(
      std::filesystem::path filename,
      Config const& config
    ) noexcept(false)
      : config_(config)
      , io_(std::move(filename))
    {}

    Config const& config_;
    mutable Io io_;
  };

  template <typename T>
  using BinaryTape = Tape<T, BinaryFileIO<T>>;
} // namespace yuliy_test_task


#if defined UNIT_TESTS
#include <gtest/gtest.h>
#include <chrono>
#include <format>

TEST(Tape, check_config)
{
  using namespace std::chrono_literals;
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto tape = *yuliy_test_task::BinaryTape<int32_t>::create(yuliy_test_task::common::canonicalize("../tests/test_input.tape"), config);
  const auto read_delay_ = 0us;
  const auto write_delay_ = 0us;
  const auto tape_shift_delay_ = 1us;
  const auto tape_rewind_delay_ = 100us;
  ASSERT_EQ(tape->config().ram_limit_bytes(), 10240);
  ASSERT_EQ(tape->config().read_delay(), read_delay_);
  ASSERT_EQ(tape->config().write_delay(), write_delay_);
  ASSERT_EQ(tape->config().tape_shift_delay(), tape_shift_delay_);
  ASSERT_EQ(tape->config().tape_rewind_delay(), tape_rewind_delay_);
}

TEST(Tape, check_empty_tape)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto tape = *yuliy_test_task::BinaryTape<int32_t>::create(yuliy_test_task::common::canonicalize("../tests/test_input.tape"), config);
  ASSERT_TRUE(tape->empty());
}

TEST(Tape, check_file_name)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input.tape");
  const auto tape = *yuliy_test_task::BinaryTape<int32_t>::create(path, config);
  ASSERT_EQ(tape->filename() , path);
}

TEST(Tape, check_eof)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input.tape");
  const auto tape = *yuliy_test_task::BinaryTape<int32_t>::create(path, config);
  ASSERT_FALSE(tape->eof());
}

// TEST(Tape, check_position)
// {
//   auto const config = *yuliy_test_task::Config::from_pwd();
//   const auto path = yuliy_test_task::common::canonicalize("../tests/test_input.tape");
//   const auto tape = *yuliy_test_task::BinaryTape<int32_t>::create(path, config);
//   ASSERT_EQ(tape->position(), 0);
// }

TEST(Tape, check_size)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input.tape");
  const auto tape = *yuliy_test_task::BinaryTape<int32_t>::create(path, config);
  ASSERT_EQ(tape->size(), 0);
}

TEST(Tape, check_read)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input1.tape");
  const auto tape = *yuliy_test_task::BinaryTape<int32_t>::create(path, config);
  const auto data = *tape->read_and_shift_n(4);
  ASSERT_EQ(data[0], 892);
  ASSERT_EQ(data[1], 262);
  ASSERT_EQ(data[2], 799);
  ASSERT_EQ(data[3], 202);
}

TEST(Tape, check_is_sorted)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input1.tape");
  const auto path1 = yuliy_test_task::common::canonicalize("../tests/test_output1.tape");
  const auto path3 = yuliy_test_task::common::canonicalize("../tests/test_input2.tape");
  const auto path4 = yuliy_test_task::common::canonicalize("../tests/test_output2.tape");
  const auto tape = *yuliy_test_task::BinaryTape<int32_t>::create(path, config);
  const auto tape1 = *yuliy_test_task::BinaryTape<int32_t>::create(path1, config);
  const auto tape3 = *yuliy_test_task::BinaryTape<int32_t>::create(path3, config);
  const auto tape4 = *yuliy_test_task::BinaryTape<int32_t>::create(path4, config);
  const auto data = *tape->read_and_shift_n(tape->size());
  const auto data1 = *tape1->read_and_shift_n(tape->size());
  const auto data3 = *tape3->read_and_shift_n(tape->size());
  const auto data4 = *tape4->read_and_shift_n(tape->size());
  ASSERT_FALSE(std::is_sorted(data.begin(), data.end()));
  ASSERT_TRUE(std::is_sorted(data1.begin(), data1.end()));
  ASSERT_FALSE(std::is_sorted(data3.begin(), data3.end()));
  ASSERT_TRUE(std::is_sorted(data4.begin(), data4.end()));
  ASSERT_EQ(tape->size(), tape1->size());
  ASSERT_EQ(tape3->size(), tape3->size());
}

#endif