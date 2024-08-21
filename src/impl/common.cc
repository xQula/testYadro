#include <impl/common.hh>

#include <algorithm>
#include <thread>
#include <random>

namespace yuliy_test_task::common
{
  [[nodiscard]] auto trimmed(std::string_view str) -> std::string {
    auto owned = std::string(str);
    owned.erase(
      std::remove_if(
        owned.begin(),
        owned.end(),
        [](char c) { return std::isspace(c); }
      ),
      owned.end()
    );
    return owned;
  }

  auto delay(std::chrono::microseconds duration) -> void {
    std::this_thread::sleep_for(duration);
  }

  auto canonicalize(std::string_view path) -> std::filesystem::path {
    if(not std::filesystem::exists(path))
      return std::filesystem::current_path() / path;
    return std::filesystem::weakly_canonical(std::filesystem::path(path));
  }

  auto random_string(std::size_t length) -> std::string {
    static auto& char_set = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
    thread_local static auto rg = std::mt19937(std::random_device{}());
    thread_local static auto pick = std::uniform_int_distribution<std::size_t>(0, sizeof(char_set) - 2);
    auto str = std::string();
    str.reserve(length);
    while(length--)
      str += char_set[pick(rg)];
    return str;
  }
} // namespace yuliy_test_task::common

