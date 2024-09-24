#include <filesystem>

// Логирование
class Logger {
    std::filesystem::path log_directory;

public:
    Logger() {
        log_directory = std::filesystem::current_path() / "game_logs";
        std::filesystem::create_directory(log_directory);
    }

    void logRound(int round, const std::string& log) {
        std::ofstream log_file(log_directory / ("round_" + std::to_string(round) + ".log"), std::ios::app);
        log_file << log << std::endl;
        log_file.close();
    }

    void logFinal(const std::string& final_log) {
        std::ofstream final_file(log_directory / "final_results.log", std::ios::app);
        final_file << final_log << std::endl;
        final_file.close();
    }
};