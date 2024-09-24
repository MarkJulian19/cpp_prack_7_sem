#include <iostream>
#include <vector>
#include <string>

#include <fstream>
#include <random>
#include <concepts>
#include <coroutine>
#include <utility> // Для std::swap
#include <map>
#include <thread>
#include <future>
#include "src/logger.cpp"
#include "src/smart_pointer.cpp"
#include "src/gamers.cpp"

#define DEBUG_LOG false


void nightPhase(std::map<int, SmartPointer<Player>>& alivePlayers, Logger& logger, int round) {
    std::map<int, int> voteCount;
    std::vector<std::future<void>> futures;
    int victimMafia = -1; 
    int victimManiac = -1;
    int victimCom = -1;
    for (const auto& [id, player] : alivePlayers) {
        if (player->role() == "Мафия") {
            futures.push_back(std::async(std::launch::async, [&]() {
                PlayerAction action = player->act(alivePlayers, id, logger, round);
                action.handle.resume();
            }));
        }
        else if (player->role() == "Маньяк") {
            futures.push_back(std::async(std::launch::async, [&]() {
                PlayerAction action = player->act(alivePlayers, id, logger, round);
                action.handle.resume();
            }));
        }
        else if (player->role() == "Комиссар") {
            futures.push_back(std::async(std::launch::async, [&]() {
                PlayerAction action = player->act(alivePlayers, id, logger, round);
                action.handle.resume();
            }));
        }
    }

    for (auto& future : futures) {
        future.get();
    }
    for (const auto& [id, player] : alivePlayers) {
        if (player->role() == "Мафия") {
            Mafia* mafiaPlayer = dynamic_cast<Mafia*>(player.get());
            if (mafiaPlayer) {
                int target = mafiaPlayer->target;
                if (target != -1) {
                    voteCount[target]++;
                }
            }
        }
    }
    if (!voteCount.empty()) {
        int maxVotes = 0;
        std::vector<int> candidates;
        for (const auto& [playerIndex, votes] : voteCount) {
            if (votes > maxVotes) {
                maxVotes = votes;
                candidates = {playerIndex};
            } else if (votes == maxVotes) {
                candidates.push_back(playerIndex);
            }
        }

        victimMafia = choose_random(candidates);
        // alivePlayers.erase(victim);
        // logger.logRound(round, "Игрок " + std::to_string(victim) + " был убит мафией.");
    }
    for (const auto& [id, player] : alivePlayers) {
        if (player->role() == "Маньяк") {
            Maniac* ManiacPlayer = dynamic_cast<Maniac*>(player.get());
            if (ManiacPlayer) {
                int target = ManiacPlayer->target;
                if (target != -1) {
                    victimManiac = target;
                }
            }
            break;
        }
    }
    for (const auto& [id, player] : alivePlayers) {
        if (player->role() == "Комиссар") {
            Commissioner* CommissionerPlayer = dynamic_cast<Commissioner*>(player.get());
            if (CommissionerPlayer) {
                int target = CommissionerPlayer->killTarget;
                if (target != -1) {
                    victimCom = target;
                }
            }
            break;
        }
    }
    
    if (victimMafia != -1){
        alivePlayers.erase(victimMafia);
        logger.logRound(round, "Игрок " + std::to_string(victimMafia) + " был убит мафией.");
    }
    if (victimManiac != -1){
        alivePlayers.erase(victimManiac);
        logger.logRound(round, "Игрок " + std::to_string(victimManiac) + " был убит маньяком.");
    }
    if (victimCom != -1){
        alivePlayers.erase(victimCom);
        logger.logRound(round, "Игрок " + std::to_string(victimCom) + " был убит коммисаром.");
    }
    
}

// Фаза дня — голосование
void dayPhase(std::map<int, SmartPointer<Player>>& alivePlayers, Logger& logger, int round) {
    std::map<int, int> voteCount;
    std::vector<std::future<void>> futures;

    for (const auto& [id, player] : alivePlayers) {
        futures.push_back(std::async(std::launch::async, [&]() {
            PlayerAction action = player->vote(alivePlayers, id, logger, round); // Игрок голосует
            action.handle.resume(); // Возобновляем корутину

            // // Предположим, что игрок голосует за себя (можно улучшить логику)
            // voteCount[id]++;
            // logger.logRound(round, "Игрок " + std::to_string(id) + " голосует.");
        }));
    }

    for (auto& future : futures) {
        future.get(); // Ждем завершения голосования
    }
        for (const auto& [id, player] : alivePlayers) {
        if (player->role() == "Мафия") {
            Mafia* mafiaPlayer = dynamic_cast<Mafia*>(player.get());
            if (mafiaPlayer) {
                int target = mafiaPlayer->target;
                if (target != -1) {
                    voteCount[target]++;
                }
            }
        }
        else if(player->role() == "Мирный"){
            Civilian* CivilianPlayer = dynamic_cast<Civilian*>(player.get());
            if (CivilianPlayer) {
                int target = CivilianPlayer->target;
                if (target != -1) {
                    voteCount[target]++;
                }
            }
        }
        else if(player->role() == "Маньяк"){
            Maniac* ManiacPlayer = dynamic_cast<Maniac*>(player.get());
            if (ManiacPlayer) {
                int target = ManiacPlayer->target;
                if (target != -1) {
                    voteCount[target]++;
                }
            }
        }
        else if(player->role() == "Комиссар"){
            Commissioner* CommissionerPlayer = dynamic_cast<Commissioner*>(player.get());
            if (CommissionerPlayer) {
                int target = CommissionerPlayer->target;
                if (target != -1) {
                    voteCount[target]++;
                }
            }
        }
    }
    // Обработка голосов
    if (!voteCount.empty()) {
        int maxVotes = 0;
        std::vector<int> candidates;
        for (const auto& [playerIndex, votes] : voteCount) {
            if (votes > maxVotes) {
                maxVotes = votes;
                candidates = {playerIndex};
            } else if (votes == maxVotes) {
                candidates.push_back(playerIndex);
            }
        }

        int victim = choose_random(candidates);
        alivePlayers.erase(victim);
        logger.logRound(round, "Игрок " + std::to_string(victim) + " был убит после голосования.");
    }
}

// Проверка конца игры
bool checkGameEnd(const std::map<int, SmartPointer<Player>>& alivePlayers, Logger& logger) {
    int mafiaAlive = 0;
    int civiliansAlive = 0;
    int maniacAlive = 0;
    for (const auto& [id, player] : alivePlayers) {
        if (player->role() == "Мафия") {
            mafiaAlive++;
        } else if(player->role() == "Маньяк") {
            maniacAlive++;
            civiliansAlive++;
        }else{
            civiliansAlive++;
        }
    }

    if (mafiaAlive >= civiliansAlive) {
        logger.logFinal("Мафия победила!");
        return true; // Мафия победила
    }



    if (mafiaAlive == 0 && maniacAlive == 0) {
        logger.logFinal("Мирные жители победили!");
        return true; // Мирные победили
    }

    if (maniacAlive == 1 && civiliansAlive == 1){
        logger.logFinal("Маньяк победил!");
        return true; // Маньяк победил
    }
    return false; // Игра продолжается
}


void run_game(std::map<int, SmartPointer<Player>>& alivePlayers, Logger& logger) {
    int round = 1;
    while (true) {
        logger.logRound(round, "Начало раунда " + std::to_string(round));

        // Логирование ролей игроков
        if(DEBUG_LOG){
            for (const auto& [id, player] : alivePlayers) {
                logger.logRound(round, "Игрок " + std::to_string(id) + ": " + player->role());
            }
        }

        // Фаза ночи
        nightPhase(alivePlayers, logger, round);
        if (checkGameEnd(alivePlayers, logger)) break;

        // Фаза дня
        if(DEBUG_LOG){
            for (const auto& [id, player] : alivePlayers) {
                logger.logRound(round, "Игрок " + std::to_string(id) + ": " + player->role());
            }
        }
        dayPhase(alivePlayers, logger, round);
        if (checkGameEnd(alivePlayers, logger)) break;

        round++;
    }
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    int N; // Количество игроков
    std::cout << "Введите количество игроков (N > 4): ";
    std::cin >> N;
    if (N <= 4) {
        std::cout << "Количество игроков должно быть больше 4." << std::endl;
        return 1;
    }

    // Будет ли пользователь участвовать в игре
    char user_in_game_choice;
    bool user_in_game = false;
    std::cout << "Хотите ли вы участвовать в игре? (y/n): ";
    std::cin >> user_in_game_choice;
    if (user_in_game_choice == 'y' || user_in_game_choice == 'Y') {
        user_in_game = true;
    }

    // Расчет количества мафии
    int k = 5; // Можно изменить значение k
    int C = 10;
    int mafia_count = N / k;
    int com_count = N / C;
    if (mafia_count == 0) mafia_count = 1;
    if (com_count == 0) com_count = 1;
    // Инициализация игроков
    std::map<int, SmartPointer<Player>> alivePlayers;

    // Добавляем роли в зависимости от количества игроков
    // Добавляем мафию
    for (int i = 1; i <= mafia_count; ++i) {
        alivePlayers[i] = SmartPointer<Player>(new Mafia());
    }

    // Добавляем комиссара
    // alivePlayers[mafia_count + 1] = SmartPointer<Player>(new Commissioner());

    // Добавляем маньяка
    alivePlayers[mafia_count + 1] = SmartPointer<Player>(new Maniac());
    for (int i = 0; i < com_count; ++i) {
        alivePlayers[mafia_count + 2 + i] = SmartPointer<Player>(new Commissioner());
    }
    // Добавляем мирных жителей
    int civilians_count = N - mafia_count - 1 - com_count; // -2, т.к. комиссар и маньяк уже добавлены
    for (int i = 0; i < civilians_count; ++i) {
        alivePlayers[mafia_count + com_count+ 2 + i] = SmartPointer<Player>(new Civilian());
    }

    Logger logger;

    // Логирование ролей игроков
    for (const auto& [id, player] : alivePlayers) {
        logger.logRound(0, "Игрок " + std::to_string(id) + ": " + player->role());
    }

    logger.logRound(0, "Игра началась!");

    // Запуск игры
    run_game(alivePlayers, logger);

    return 0;
}