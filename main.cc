#include <algorithm>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using MemInfoEntry = std::pair<std::string, uint64_t>;
using MemInfo = std::vector<MemInfoEntry>;

constexpr char g_whiteSpace[] = " \t\n\r\f\v";

MemInfo parseProcMeminfo()
{
    const char* path = "/proc/meminfo";

    std::ifstream inFile(path);

    if (!inFile.is_open()) {
        std::cerr << "Failed to open file: " << path << '\n';
        return {};
    }

    MemInfo meminfo;

    // The file /proc/meminfo usually contains about 50 lines.
    // To avoid over-allocation, we can reserve memory:
    meminfo.reserve(64);

    // /proc/meminfo file format:
    //
    // <ParamName>:    <Size> [Unit]
    // ...

    std::string line;
    while (std::getline(inFile, line)) {
        auto pos1 = line.find(':');
        if (pos1 != std::string::npos) {
            auto pos2 = line.find_first_not_of(g_whiteSpace, pos1 + 1);
            if (pos2 != std::string::npos) {
                const auto valuePtr = line.data() + pos2;
                uint64_t value;

                auto [ptr, ec] = std::from_chars(valuePtr, line.data() + line.size(), value);

                if (ec == std::errc::invalid_argument) {
                    std::cerr << "The string \"" << valuePtr << "\" does not contain a number\n";
                    continue;
                }
                else if (ec == std::errc::result_out_of_range) {
                    std::cerr << "The number from the string " << valuePtr << " is too large\n";
                    continue;
                }

                meminfo.emplace_back(line, value);
            }
            else {
                std::cerr << "Line \"" << line << "\" has incorrect format\n";
            }
        }
        else {
            std::cerr << "Line \"" << line << "\" has incorrect format\n";
        }
    }

    // inFile.close(); // Not necessary: ​​RAII will do everything automatically.
    return meminfo;
}

void sortMemInfoBySize(MemInfo& meminfo)
{
    auto compareBySize = [](const auto& a, const auto& b) { return a.second > b.second; };

    std::stable_sort(meminfo.begin(), meminfo.end(), compareBySize);
}

bool modifyAndPrintToFile(MemInfo& meminfo, const char* path)
{
    if (path == nullptr || !path[0]) {
        std::cerr << "File path is empty!\n";
        return false;
    }

    std::ofstream outFile(path);

    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << path << '\n';
        return false;
    }

    for (const auto& item : meminfo) {
        auto line = item.first;
        auto size = item.second;

        line.erase(std::remove(line.begin(), line.end(), ':'), line.end());

        if (size == 0) {
            auto pos1 = line.find_first_of(g_whiteSpace);
            if (pos1 != std::string::npos) {
                auto pos2 = line.find_first_not_of(g_whiteSpace, pos1);
                if (pos2 != std::string::npos) {
                    line[pos2++] = '-';
                    line.erase(line.begin() + pos2, line.end());
                }
            }
        }
        outFile << line << '\n';
    }

    // outFile.close(); // Not necessary.
    return true;
}

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        std::cout << "Run: " << argv[0] << " <output_file>\n";
        return 0;
    }

    auto meminfo = parseProcMeminfo();
    sortMemInfoBySize(meminfo);
    if (!modifyAndPrintToFile(meminfo, argv[1])) {
        return 1;
    }

    std::cout << "OK\n";
    return 0;
}
