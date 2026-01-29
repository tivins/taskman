/**
 * Implémentation rules:generate.
 */

#include "rules.hpp"
#include <cxxopts.hpp>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

// Include the generated header with embedded rules
#include "rules.generated.h"

namespace fs = std::filesystem;

namespace taskman {

int cmd_rules_generate(int argc, char* argv[]) {
    cxxopts::Options opts("taskman rules:generate", "Generate .cursor/rules/ files");
    opts.add_options()
        ("output", "Output directory (default: .cursor/rules)", cxxopts::value<std::string>()->default_value(".cursor/rules"))
        ("help", "Show help");

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            std::cout << opts.help() << '\n';
            return 0;
        }
    }

    cxxopts::ParseResult result;
    try {
        result = opts.parse(argc, argv);
    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "taskman: " << e.what() << "\n";
        return 1;
    }

    std::string output_dir = result["output"].as<std::string>();

    try {
        // Create output directory if it doesn't exist
        fs::path output_path(output_dir);
        fs::create_directories(output_path);

        if (!fs::is_directory(output_path)) {
            std::cerr << "taskman: output path is not a directory: " << output_dir << "\n";
            return 1;
        }

        // Write each rule file
        int count = 0;
        for (std::size_t i = 0; i < NUM_RULE_FILES; ++i) {
            const RuleFile& rule = RULE_FILES[i];
            fs::path file_path = output_path / rule.filename;

            std::ofstream out_file(file_path, std::ios::binary);
            if (!out_file) {
                std::cerr << "taskman: cannot write file: " << file_path << "\n";
                return 1;
            }

            out_file.write(rule.content, std::strlen(rule.content));
            if (!out_file) {
                std::cerr << "taskman: error writing file: " << file_path << "\n";
                return 1;
            }

            out_file.close();
            ++count;
        }

        std::cout << "Generated " << count << " rule file(s) in " << output_dir << "\n";
        return 0;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "taskman: filesystem error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "taskman: error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace taskman
