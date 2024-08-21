#pragma once

#include <utility>
#include <fstream>
#include <ranges>
#include <queue>
#include <impl/itape.hh>
#include <impl/common.hh>

namespace yuliy_test_task::algorithm
{
  template <typename T>
  using result_type = std::expected<T, std::string>;

  namespace detail
  {
    template <typename T>
    requires (sizeof(T) > 0)
    struct TempFile
    {
      explicit TempFile(std::vector<T> const& values) {
        auto const name = common::random_string(32);
        this->path = std::filesystem::temp_directory_path() / "yuliy_test_task_temp_file_sorter" / name;
        this->path.replace_extension(".tmp");
        create_directories(this->path.parent_path());
        if(not exists(this->path))
          auto const stream = std::ofstream(this->path);
        this->stream_ = std::fstream(this->path, std::ios::binary | std::ios::out | std::ios::in);
        if(not this->stream_)
          return;
        this->write(values);
      }

      ~TempFile() noexcept {
        [[maybe_unused]] auto dummy = std::error_code();
        std::filesystem::remove(this->path, dummy);
      }

      TempFile(TempFile const&) = delete;
      TempFile& operator=(TempFile const&) = delete;
      TempFile(TempFile&&) noexcept = default;
      TempFile& operator=(TempFile&&) noexcept = default;


      /**
       * Reads a single value of type T from the temporary file.
       *
       * This function reads a single value of type T from the temporary file
       * and returns it wrapped in a std::optional. The value is read from the
       * current position of the file and the position is not changed after the
       * read operation. The function returns std::nullopt if the file is not
       * open or if the end of the file has been reached.
       *
       * \returns The value read from the file or std::nullopt if there is no
       * value to read.
       */
      [[nodiscard]] auto read_one() -> std::optional<T> {
        if(not this->stream_)
          return std::nullopt;
        if(this->stream_.eof())
          return std::nullopt;
        auto value = T();
        this->stream_.read(reinterpret_cast<char*>(&value), sizeof(T));
        return value;
      }


      /**
       * Reads all values of type T from the temporary file.
       *
       * This function reads all values of type T from the temporary file and
       * returns them wrapped in a std::vector. The values are read from the
       * current position of the file and the position is not changed after the
       * read operation. The function returns an empty std::vector if the file
       * is not open.
       *
       * \returns The values read from the file.
       */
      [[nodiscard]] auto read() -> std::vector<T> {
        auto values = std::vector<T>();
        if(not this->stream_)
          return values;
        while(true) {
          auto const value = this->read_one();
          if(not value)
            break;
          values.push_back(*value);
        }
        return values;
      }


      /**
       * Writes values of type T to the temporary file.
       *
       * This function writes values of type T to the temporary file. The values
       * are written to the current position of the file and the position is
       * not changed after the write operation. The function also applies a delay
       * specified in the config.
       *
       * \param values The values to write.
       */
      auto write(std::vector<T> const& values) -> void {
        if(not this->stream_)
          return;
        this->stream_.write(reinterpret_cast<char const*>(values.data()), values.size() * sizeof(T));
        this->stream_.flush();
        this->stream_.seekp(0, std::ios_base::beg);
      }

      std::filesystem::path path;

      private:
        std::fstream stream_;
    };
  } // namespace detail


  /**
   * Sorts the input tape in ascending order and writes it to the output tape.
   *
   * This function uses a temporary file for sorting and therefore it
   * requires enough free space on the disk. The function also applies a
   * delay specified in the config.
   *
   * \param in The input tape.
   * \param out The output tape.
   * \param progress If true, the function prints progress information.
   * \returns An empty result if the function was successful, otherwise
   * an std::unexpected with an error message.
   */
  template <typename T>
  [[nodiscard]] auto sort_into(
    ITape<T>& in,
    ITape<T>& out,
    bool progress = false
  ) -> result_type<void> {
    auto const max_elems_in_ram = in.config().template ram_limit_elems<typename ITape<T>::value_type>();
    auto const size = in.size();
    if(size == 0)
      return {};
    auto const tmp_file_count = size / max_elems_in_ram + 1;
    auto tmp_files = std::vector<detail::TempFile<T>>();
    tmp_files.reserve(tmp_file_count);

    if(progress)
      common::println("\nReading tape...");
    auto read = 0;
    for(std::size_t i = 0; i < tmp_file_count; ++i) {
      auto data = in.read_and_shift_n(max_elems_in_ram);
      if(not data)
        return std::unexpected(data.error());
      if(progress)
        common::print_progress(++read, tmp_file_count);
      std::sort(data->begin(), data->end());
      tmp_files.emplace_back(*data);
    }
    if(progress)
      common::println();

    using value_index_type = std::pair<typename ITape<T>::value_type, int>;
    auto compare = [](
      value_index_type const& lhs,
      value_index_type const& rhs
    ) -> bool { return lhs.first > rhs.first; };
    auto min_heap = std::priority_queue<
      value_index_type,
      std::vector<value_index_type>,
      decltype(compare)
    >(compare);
    for(int i = 0; i < std::ssize(tmp_files); ++i) {
      auto& tmp_file = tmp_files[i];
      auto const value = tmp_file.read_one();
      if(value)
        min_heap.emplace(*value, i);
    }
    if(progress)
      common::println("\nSorting...");
    auto written = 0;
    while(not min_heap.empty()) {
      auto const [val, idx] = min_heap.top();
      min_heap.pop();
      out.write_and_shift(val);
      if(progress)
        common::print_progress(++written, size);
      auto new_val = tmp_files[idx].read_one();
      if(new_val and *new_val)
        min_heap.emplace(*new_val, idx);
    }
    if(progress)
      common::println();
    return {};
  }
} // namespace yuliy_test_task::algorithm