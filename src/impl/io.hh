#pragma once

#include <fstream>
#include <sstream>
#include <filesystem>
#include <impl/itape.hh>

namespace yuliy_test_task
{
  template <TapeElement T>
  class IO
  {
   public:
    virtual ~IO() = default;

    [[nodiscard]] virtual auto read() const -> T = 0;
    [[nodiscard]] virtual auto shift(ITape<T>::Direction direction) -> bool = 0;
    virtual auto write(T value) -> void = 0;
    virtual auto rewind() -> void = 0;

    [[nodiscard]] virtual auto end() const -> bool = 0;
    [[nodiscard]] virtual auto size() const -> std::size_t = 0;
    [[nodiscard]] virtual auto name() const -> std::filesystem::path const& = 0;
  };

  namespace detail
  {
    template <typename T>
    concept ConvertibleToPathOrString = std::convertible_to<T, std::filesystem::path>
        or std::convertible_to<T, std::string>;
  } // namespace detail

  template <typename T, typename U>
  concept TapeIO = requires(T t, U value) {
    { t.read() } -> std::convertible_to<U>;
    { t.write(value) } -> std::same_as<void>;
    { t.rewind() } -> std::same_as<void>;
    { t.end() } -> std::same_as<bool>;
    { t.size() } -> std::same_as<std::size_t>;
    { t.name() } -> detail::ConvertibleToPathOrString;
  };

  template <TapeElement T>
  class AbstractFileIO : public IO<T>
  {
    public:
      explicit AbstractFileIO(std::filesystem::path filename)
        : filename_(std::move(filename))
        , position_(0) {
        if(not exists(this->filename_.parent_path()))
          create_directories(this->filename_.parent_path());
      }

      ~AbstractFileIO() override = default;

      auto rewind() -> void override {
        this->position_ = 0;
        this->handle_.seekp(this->position_);
        this->handle_.clear();
      }

      [[nodiscard]] auto end() const -> bool override { return this->handle_.eof(); }
      [[nodiscard]] auto name() const -> std::filesystem::path const& override { return this->filename_; }

      // own
      [[nodiscard]] auto position() const -> std::streampos { return this->position_; }

    protected:
      std::filesystem::path filename_;
      mutable std::fstream handle_;
      std::streampos position_;
  };

  template <TapeElement T>
  class BinaryFileIO : public AbstractFileIO<T>
  {
    public:
      explicit BinaryFileIO(std::filesystem::path filename)
        : AbstractFileIO<T>(std::move(filename)) {
        if(not exists(this->filename_)) {
          this->handle_.open(this->filename_, std::ios::out | std::ios::binary);
          if(not this->handle_)
            throw std::runtime_error(std::format("failed to open file {}", this->filename_.generic_string()));
          this->handle_.close();
        }
        this->handle_.open(this->filename_, std::ios::in | std::ios::out | std::ios::binary);
        if(not this->handle_)
          throw std::runtime_error(std::format("failed to open file {}", this->filename_.generic_string()));
      }

      [[nodiscard]] auto read() const -> T override {
        auto value = T();
        this->handle_.read(reinterpret_cast<char*>(&value), sizeof(T));
        this->handle_.seekp(this->position_, std::ios_base::beg);
        return static_cast<int>(value);
      }

      [[nodiscard]] auto shift(ITape<T>::Direction direction) -> bool override {
        if(not this->handle_)
          return false;
        if(direction == ITape<T>::Direction::Left)
          this->position_ -= sizeof(T);
        else
          this->position_ += sizeof(T);
        this->handle_.seekp(this->position_);
        return true;
      }

      auto write(T value) -> void override {
        auto const value_ = static_cast<T>(value);
        this->handle_.write(reinterpret_cast<char const*>(&value_), sizeof(T));
        this->handle_.seekp(this->position_, std::ios_base::beg);
      }

      [[nodiscard]] auto size() const -> std::size_t override {
        this->handle_.seekp(0, std::ios_base::end);
        auto const size = static_cast<std::size_t>(this->handle_.tellp()) / sizeof(T);
        this->handle_.seekp(this->position_, std::ios_base::beg);
        return size;
      }
  };
} // namespace yuliy_test_task