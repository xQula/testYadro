#pragma once

#include <fstream>
#include <vector>
#include <memory>
#include <type_traits>
#include <impl/itape.hh>
#include <impl/config.hh>
#include <impl/common.hh>

namespace yuliy_test_task
{
  template <TapeElement T>
  class Tape : public ITape<T>
  {
   public:
    [[nodiscard]] static auto create(
      std::filesystem::path filename,
      Config const& config
    ) -> ITape<T>::template result_type<std::unique_ptr<ITape<T>>> try {
      return std::unique_ptr<ITape<T>>(new Tape<T>(std::move(filename), config));
    } catch(std::exception const& e) {
      return std::unexpected(e.what());
    }

    ~Tape() override = default;


    /** \brief Reads a value from the tape.
    *
    * This function reads a value of type T from the tape and returns it. The
    * value is read from the current position of the tape and the position is
    * not changed after the read operation. The function also applies a delay
    * specified in the config.
    *
    * \returns The value read from the tape.
    */
    [[nodiscard]] auto read() const -> T override {
      auto value = T();
      this->handle_.read(reinterpret_cast<char*>(&value), sizeof(T));
      this->handle_.seekp(this->position_, std::ios_base::beg);
      common::delay(this->config().read_delay());
      return static_cast<int>(value);
    }

    /** \brief Reads and shifts a value from the tape.
    *
    * This function reads a value of type T from the tape and returns it. The
    * value is read from the current position of the tape and the position is
    * shifted to the next position. The function also applies a delay
    * specified in the config.
    *
    * \returns The value read from the tape.
    */
    [[nodiscard]] auto read_and_shift() -> T override {
      auto const res = this->read();
      std::ignore = this->shift(ITape<T>::Direction::Right);
      return res;
    }

    /** \brief Reads and shifts n values from the tape.
    *
    * This function reads n values of type T from the tape and returns them. The
    * values are read from the current position of the tape and the position is
    * shifted to the next position. The function also applies a delay
    * specified in the config.
    *
    * \param n The number of values to read and shift.
    * \returns The values read from the tape.
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

    /** \brief Shifts the tape in the specified direction.
    *
    * This function shifts the tape in the specified direction. The function
    * applies a delay specified in the config.
    *
    * \param direction The direction to shift the tape.
    * \returns `true` if the shift was successful, `false` otherwise.
    */
    [[nodiscard]] auto shift(ITape<T>::Direction direction) -> bool override {
      common::delay(this->config().tape_shift_delay());
      if(not this->handle_)
        return false;
      if(direction == ITape<T>::Direction::Left)
        this->position_ -= sizeof(T);
      else
        this->position_ += sizeof(T);
      this->handle_.seekp(this->position_);
      return true;
    }

    /** \brief Writes a value to the tape.
    *
    * This function writes a value of type T to the tape. The value is written
    * to the current position of the tape and the position is not changed
    * after the write operation. The function also applies a delay
    * specified in the config.
    *
    * \param value The value to write.
    */
    auto write(T value) -> void override {
      auto const value_ = static_cast<T>(value);
      this->handle_.write(reinterpret_cast<char const*>(&value_), sizeof(T));
      this->handle_.seekp(this->position_, std::ios_base::beg);
      common::delay(this->config().write_delay());
    }

    /** \brief Writes and shifts a value to the tape.
    *
    * This function writes a value of type T to the tape. The value is written
    * to the current position of the tape and the position is shifted to the
    * next position. The function also applies a delay
    * specified in the config.
    *
    * \param value The value to write and shift.
    */
    auto write_and_shift(T value) -> void override {
      this->write(value);
      std::ignore = this->shift(ITape<T>::Direction::Right);
    }

    /** \brief Writes and shifts n values to the tape.
    *
    * This function writes n values of type T to the tape. The values are
    * written to the current position of the tape and the position is shifted
    * to the next position. The function also applies a delay
    * specified in the config.
    *
    * \param values The values to write and shift.
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

    /** \brief Rewinds the tape to the beginning.
    *
    * This function rewinds the tape to the beginning. The function also
    * applies a delay specified in the config.
    */
    auto rewind() -> void override {
      common::delay(this->config().tape_rewind_delay());
      this->position_ = 0;
      this->handle_.seekp(this->position_);
      this->handle_.clear();
    }


    /**
     * Checks if the end of the file has been reached.
     *
     * @return `true` if the end of the file has been reached, `false` otherwise.
     */
    [[nodiscard]] auto eof() const -> bool override {
      return this->handle_.eof();
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
     * Returns the number of elements in the tape.
     *
     * This function calculates the number of elements in the tape by
     * seeking to the end of the file, calculating the file size, and then
     * dividing it by the size of a single element.
     *
     * @return The number of elements in the tape.
     */
    [[nodiscard]] auto size() const -> std::size_t override {
      this->handle_.seekp(0, std::ios_base::end);
      auto const size = static_cast<std::size_t>(this->handle_.tellp()) / sizeof(T);
      this->handle_.seekp(this->position_, std::ios_base::beg);
      return size;
    }

    /**
     * Returns the current position of the tape.
     *
     * @return The current position of the tape.
     */
    [[nodiscard]] auto position() const -> std::size_t override {
      return this->position_;
    }


    /**
     * Returns the filename associated with the tape.
     *
     * @return The filename of the tape.
     */
    [[nodiscard]] auto filename() const -> std::filesystem::path const& override {
      return this->filename_;
    }

    /**
     * Returns the config associated with the tape.
     *
     * @return The config of the tape.
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
      , filename_(std::move(filename))
      , position_(0) {
      if(not exists(this->filename().parent_path()))
        create_directories(this->filename().parent_path());
      if(not exists(this->filename())) {
        this->handle_.open(this->filename(), std::ios::out | std::ios::binary);
        if(not this->handle_)
          throw std::runtime_error(std::format("failed to open file {}", this->filename().generic_string()));
        this->handle_.close();
      }
      this->handle_.open(this->filename(), std::ios::in | std::ios::out | std::ios::binary);
      if(not this->handle_)
        throw std::runtime_error(std::format("failed to open file {}", this->filename().generic_string()));
    }

    Config const& config_;
    std::filesystem::path filename_;
    mutable std::fstream handle_;
    std::streampos position_;
  };
} // namespace yuliy_test_task


#if defined UNIT_TESTS
#include <gtest/gtest.h>
#include <chrono>
#include <format>

TEST(Tape, check_config)
{
  using namespace std::chrono_literals;
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto tape = *yuliy_test_task::Tape<int32_t>::create(yuliy_test_task::common::canonicalize("../tests/test_input.tape"), config);
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
  const auto tape = *yuliy_test_task::Tape<int32_t>::create(yuliy_test_task::common::canonicalize("../tests/test_input.tape"), config);
  ASSERT_TRUE(tape->empty());
}

TEST(Tape, check_file_name)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input.tape");
  const auto tape = *yuliy_test_task::Tape<int32_t>::create(path, config);
  ASSERT_EQ(tape->filename() , path);
}

TEST(Tape, check_eof)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input.tape");
  const auto tape = *yuliy_test_task::Tape<int32_t>::create(path, config);
  ASSERT_FALSE(tape->eof());
}

TEST(Tape, check_position)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input.tape");
  const auto tape = *yuliy_test_task::Tape<int32_t>::create(path, config);
  ASSERT_EQ(tape->position(), 0);
}

TEST(Tape, check_size)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input.tape");
  const auto tape = *yuliy_test_task::Tape<int32_t>::create(path, config);
  ASSERT_EQ(tape->size(), 0);
}

TEST(Tape, check_read)
{
  auto const config = *yuliy_test_task::Config::from_pwd();
  const auto path = yuliy_test_task::common::canonicalize("../tests/test_input1.tape");
  const auto tape = *yuliy_test_task::Tape<int32_t>::create(path, config);
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
  const auto tape = *yuliy_test_task::Tape<int32_t>::create(path, config);
  const auto tape1 = *yuliy_test_task::Tape<int32_t>::create(path1, config);
  const auto tape3 = *yuliy_test_task::Tape<int32_t>::create(path3, config);
  const auto tape4 = *yuliy_test_task::Tape<int32_t>::create(path4, config);
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