#include <impl/config.hh>

#include <fstream>
#include <impl/common.hh>

namespace yuliy_test_task
{
  auto Config::from_pwd() -> std::expected<Config, std::string> {
    return Config::load(std::filesystem::current_path() / Config::default_filename);
  }

  auto Config::load(const std::filesystem::path &path) -> std::expected<Config, std::string> {
    if(not exists(path))
      return std::unexpected(std::format("file {} doesn't exist", path.generic_string()));
    auto ifs = std::ifstream(path);
    auto self = Config();
    try {
      for(std::string line; std::getline(ifs, line);) {
        auto pos = line.find('=');
        if(pos == std::string::npos)
          continue;
        auto key = common::trimmed(line.substr(0, pos));
        auto value = common::trimmed(line.substr(pos + 1));
        if(key == "ram_limit")
          self.ram_limit_ = std::stoull(value);
        else if(key == "read_delay")
          self.read_delay_ = std::chrono::microseconds{std::stoll(value)};
        else if(key == "write_delay")
          self.write_delay_ = std::chrono::microseconds{std::stoll(value)};
        else if(key == "tape_shift_delay")
          self.tape_shift_delay_ = std::chrono::microseconds{std::stoll(value)};
        else if(key == "tape_rewind_delay")
          self.tape_rewind_delay_ = std::chrono::microseconds{std::stoll(value)};
      }
    } catch(...) {
      return std::unexpected(std::format(R"(failed to parse config file '{}')", path.generic_string()));
    }
    return self;
  }

  auto operator<<(std::ostream &os, const Config &self) -> std::ostream & {
    os << std::format("ram limit    = {} bytes\n", self.ram_limit_);
    os << std::format("read delay   = {}\n", self.read_delay_);
    os << std::format("write delay  = {}\n", self.write_delay_);
    os << std::format("tape shift   = {}\n", self.tape_shift_delay_);
    os << std::format("tape rewind  = {}\n", self.tape_rewind_delay_);
    return os;
  }
} // namespace yuliy_test_task