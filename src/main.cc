/*
 * Tape is a data storage device that can consequently read and write.
 * The read/write magnetic head is stationary during reading and writing, and the tape can move in both directions.
 * Writing and reading information is possible in the tape cell on which the magnetic head is currently located.
 * Moving magnetic head is an expensive operation: tape doesn't have random access.
 *
 * There is an input tape of length N (where N is large), containing int32_t numbers.
 * There is also an output tape of same length N.
 * You must write the numbers from the input tape to the output tape, sorted in ascending order.
 * There is also RAM limit (represented as constant named M), so you can't store more than M numbers in RAM at any time.
 * To implement the algorithm, you can use a reasonable number of time tapes, i.e. tapes on which you can store
 * some temporary information necessary during work algorithm.
 *
 * You must create an C++ console program, which implements the algorithm with this follow-up description:
 * - Define an interface for Tape collection/range.
 * - Implement the interface. The following fields must be configurable through configuration file:
 *   - M (RAM limit)
 *   - N (size of input/output tapes)
 *   - Read/write delay (in milliseconds)
 *   - Tape shift delay (in milliseconds)
 *   - Tape rewind delay (in milliseconds)
 * - You can store temporary files in %AppData%/Local/Temp directory (or in /tmp/ directory on Linux).
 * - Console program must accept input and output files as command line arguments.
 */

#include <impl/common.hh>
#include <impl/config.hh>
#include <impl/tape.hh>
#include <impl/sort.hh>

using namespace yuliy_test_task;

auto main(int argc, char* argv[]) -> int try {
  if(argc != 3)
    common::panic(1, "usage: {} <input tape> <output tape>", argv[0]);
  auto const config = *Config::from_pwd();
  common::println("{}", config);
  *algorithm::sort_into(
    **BinaryTape<int32_t>::create(common::canonicalize(argv[1]), config),
    **BinaryTape<int32_t>::create(common::canonicalize(argv[2]), config),
    true
  );
  common::println("Done.");
  return 0;
} catch(std::exception const& e) {
  common::panic(1, "Error: {}", e.what());
}
