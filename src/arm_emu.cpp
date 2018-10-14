#include "cpp_box/arm.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "rang.hpp"

template<typename Cont> void dump_rom(const Cont &c)
{
  std::size_t loc = 0;

  std::cerr << "Dumping Data At Loc: " << static_cast<const void *>(c.data()) << '\n';

  for (const auto byte : c) {
    std::cerr << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte) << ' ';
    if ((++loc % 16) == 0) { std::cerr << '\n'; }
  }
  std::cerr << '\n';
}

template<typename Map> void dump_memory_map(const Map &m)
{
  std::size_t loc = 0;
  for (const auto &memory : m) {
    std::cout << loc << ": " << memory.in_use << ' ' << std::hex << std::setfill('0') << static_cast<const void *>(memory.data) << ' ' << std::setw(8)
              << memory.start << ' ' << memory.end << '\n';
    ++loc;
  }
}

template<typename System, typename Registers> void dump_state(const System &sys, const Registers &last_registers)
{
  std::cout << ' ' << std::setw(8) << std::setfill('0') << std::hex << sys.PC();

  for (std::size_t reg = 0; reg < sys.registers.size(); ++reg) {
    if (sys.registers[reg] == last_registers[reg]) {
      std::cout << rang::style::dim;
    } else {
      std::cout << rang::style::reset;
    }
    std::cout << ' ' << std::setw(8) << std::setfill('0') << std::hex << sys.registers[reg];
  }

  std::cout << '\n';
}

int main(const int argc, const char *argv[])
{
  std::vector<std::string> args{ argv, argv + argc };

  if (args.size() == 2) {
    std::cerr << "Attempting to load file: " << args[1] << '\n';

    auto RAM = [&]() {
      if (std::ifstream ifs{ args.at(1), std::ios::binary }; ifs.good()) {
        const auto file_size = ifs.seekg(0, std::ios_base::end).tellg();
        ifs.seekg(0);
        std::vector<char> data;
        data.resize(static_cast<std::size_t>(file_size));
        ifs.read(data.data(), file_size);
        std::cerr << "Loaded file: '" << args.at(1) << "' of size: " << file_size << '\n';
        return std::vector<uint8_t>{ begin(data), end(data) };
      } else {
        std::cerr << "Error opening file: " << args.at(1) << '\n';
        exit(EXIT_FAILURE);
      }
    }();

    cpp_box::arm::System sys{ RAM };
    dump_rom(RAM);

    auto last_registers = sys.registers;
    int opcount         = 0;
    const auto tracer =
      [&opcount]([[maybe_unused]] const auto & /*t_sys*/, [[maybe_unused]] const auto /*t_pc*/, [[maybe_unused]] const auto /*t_ins*/) { ++opcount; };

    sys.run(0x00000000, tracer);

    std::cout << "Total instructions executed: " << opcount << '\n';

    dump_state(sys, last_registers);
    // if ((++opcount) % 1000 == 0) { std::cout << opcount << '\n'; }
  }
}
