#include <iostream>
#include <fstream>
#include <random>
#include <concepts>
#include <thread>
#include <future>
#include "src/gamers.cpp"
#include <ranges>

#define DEBUG_LOG true


int assignUserRole(std::map<int, SmartPointer<Player>>& alivePlayers, std::mt19937& gen) {
    // Выбираем случайный ID из доступных игроков
    std::vector<int> playerIds;
    for (const auto& [id, player] : alivePlayers) {
        playerIds.push_back(id);
    }

    std::shuffle(playerIds.begin(), playerIds.end(), gen); // Перемешиваем ID
    int userId = 2; // Берем первый ID

    std::cout << "Вы играете за игрока с ID: " << userId << ". Ваша роль: " << alivePlayers[userId]->role() << std::endl;
    return userId;
}
void userNightAction(std::map<int, SmartPointer<Player>>& alivePlayers, int userId, Logger& logger, int round) {
    // Проверяем, что пользователь (игрок) жив
    if (alivePlayers.find(userId) == alivePlayers.end()) {
        std::cout << "Вы не можете выполнить действие, так как ваш персонаж больше не жив." << std::endl;
        return; // Завершаем функцию, если игрок мертв
    }
    else{
        std::cout << "Игрок жив." << std::endl;
        if (alivePlayers[userId].get()->role() == "Мирный"){
            std::cout << "Вы мирный, пропуск ночной фазы."<<std::endl;
            return;
        }
        if (alivePlayers[userId].get()->role() == "Комиссар"){
            for (const auto& [id, player] : alivePlayers) {
                if (id != userId) {
                    std::cout << "Игрок " << id << " (" << player->role() << ")" << std::endl;
                }
            }
            std::cout << "1 - Проверить игрока." << std::endl;
            std::cout << "2 - Выстрелить в игрока." << std::endl;
            int chois;
            std::cin >> chois;
            while (chois != 1 && chois != 2)
            {
                std::cout << "1 - Проверить игрока." << std::endl;
                std::cout << "2 - Выстрелить в игрока." << std::endl;
                std::cin >> chois;
            }
            if (chois == 1){
                std::cout << "Выберите ID игрока для проверки: " << std::endl;
                int target;
                std::cin >> target;
                while(alivePlayers.find(target) == alivePlayers.end() || target == userId){
                    std::cout << "Неверный ID игрока." << std::endl;
                    std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                    std::cin >> target;
                }
                if (alivePlayers[target].get()->role() == "Мафия" || alivePlayers[target].get()->role() == "Дон мафии"){
                    std::cout << "Выбранный игрок: мафия."<< std::endl;
                }
                else{
                    std::cout << "Выбранный игрок: мирный."<< std::endl;
                }
                return;
            }
            else if (chois == 2){
                std::cout << "Выберите ID игрока для убийства: " << std::endl;
                int target;
                std::cin >> target;
                while(alivePlayers.find(target) == alivePlayers.end() || target == userId){
                    std::cout << "Неверный ID игрока." << std::endl;
                    std::cout << "Выберите ID игрока для ночного действия: " << std::endl;
                    std::cin >> target;
                }
                alivePlayers[userId].get()->setKill(target);
                return;
            }
        }
        std::cout << "Выберите ID игрока для ночного действия: " << std::endl;

        // Отображение игроков для выбора
        for (const auto& [id, player] : alivePlayers) {
            if (id != userId) {
                std::cout << "Игрок " << id << " (" << player->role() << ")" << std::endl;
            }
        }
        
        int targetId;
        std::cin >> targetId;

        // Проверяем, что выбранная цель существует и жива
        if (alivePlayers.find(targetId) != alivePlayers.end() && targetId != userId) {
            // Если цель жива и это не сам пользователь, устанавливаем цель
            alivePlayers[userId].get()->setTarget(targetId); // Устанавливаем цель пользователя
            logger.logRound(round, "Игрок " + std::to_string(userId) + " выбрал цель: игрок " + std::to_string(targetId));
        } else if (targetId == userId) {
            std::cout << "Нельзя выбирать себя в качестве цели. Действие пропущено." << std::endl;
        } else {
            std::cout << "Некорректный выбор. Действие пропущено." << std::endl;
        }
    }
}
void userVotePhase(std::map<int, SmartPointer<Player>>& alivePlayers, int userId, Logger& logger, int round) {
    // Проверяем, что пользователь (игрок) жив
    if (alivePlayers.find(userId) == alivePlayers.end()) {
        std::cout << "Вы не можете голосовать, так как ваш персонаж больше не жив." << std::endl;
        return; // Завершаем функцию, если игрок мертв
    }

    std::cout << "Ваш голос! Выберите игрока для голосования (введите ID):" << std::endl;

    // Отображение игроков для голосования
    for (const auto& [id, player] : alivePlayers) {
        if (id != userId) {
            std::cout << "Игрок " << id << " (" << player->role() << ")" << std::endl;
        }
    }

    int voteTarget;
    std::cin >> voteTarget;

    // Проверяем, что выбранный игрок существует и жив
    if (alivePlayers.find(voteTarget) != alivePlayers.end() && voteTarget != userId) {
        // Если игрок жив и не является самим пользователем, выполняем голосование
        alivePlayers[userId].get()->setTarget(voteTarget); // Пользователь голосует за игрока
        logger.logRound(round, "Игрок " + std::to_string(userId) + " голосует за игрока " + std::to_string(voteTarget));
    } else if (voteTarget == userId) {
        std::cout << "Нельзя голосовать за себя. Голосование пропущено." << std::endl;
    } else {
        std::cout << "Некорректный выбор. Голосование пропущено." << std::endl;
    }
}

void nightPhase(std::map<int, SmartPointer<Player>>& alivePlayers, Logger& logger, int round, bool user_in_game, int userId) {
    std::map<int, int> voteCount;
    std::vector<std::future<void>> futures;
    int victimMafia = -1; 
    int victimManiac = -1;
    int victimCom = -1;
    int donId = -1;
    std::vector<int> mafiaIds;
    // Используем ranges для фильтрации ролей
    for (const auto& [id, player] : alivePlayers) {
        if (player->role() == "Дон мафии") {
            donId = id;  // Если нашли Дона, запоминаем его ID
        }
        if (player->role() == "Мафия") {
            mafiaIds.push_back(id);  // Добавляем всех мафиози в список
        }
    }

    // Если Дона нет (он убит), выбираем нового Дона случайным образом
    if (donId == -1 && !mafiaIds.empty()) {
        donId = choose_random(mafiaIds);  // Выбираем нового Дона из списка мафиози

        // Убедимся, что выбранный игрок существует в alivePlayers
        auto it = alivePlayers.find(donId);
        if (it != alivePlayers.end()) {  // Проверяем, что такой игрок есть
            Mafia* newDon = dynamic_cast<Mafia*>(it->second.get());  // Безопасно получаем указатель на мафиози

            // Проверяем успешность dynamic_cast
            if (newDon) {
                newDon->isDon = true;  // Назначаем игрока новым Доном мафии
                logger.logRound(round, "Игрок " + std::to_string(donId) + " назначен новым Доном мафии.");
            } else {
                logger.logRound(round, "Ошибка: не удалось назначить нового Дона мафии.");
            }
        } else {
            logger.logRound(round, "Ошибка: не удалось найти игрока для назначения Доном.");
        }
    }

    // Используем параллельные задачи для действий игроков
    for (const auto& [id, player] : alivePlayers) {
        if (user_in_game && id == userId) {
                userNightAction(alivePlayers, userId, logger, round); 
        }else{
            futures.push_back(std::async(std::launch::async, [&]() {
                
                if (player->role() == "Мафия" || player->role() == "Дон мафии" || 
                    player->role() == "Маньяк" || player->role() == "Комиссар") {
                    PlayerAction action = player->act(alivePlayers, id, logger, round, true);
                    action.handle.resume();
                }
            }));
        }
    }

    // Ожидаем завершения действий всех игроков
    for (auto& future : futures) {
        future.get();
    }
    futures.clear();

    // Действие Дона
    //Не вызываем действия дона, если игрок и есть Дон
    if (donId != -1 && (userId != donId)) {
        futures.push_back(std::async(std::launch::async, [&]() {
            PlayerAction action = alivePlayers[donId].get()->act(alivePlayers, donId, logger, round, false);
            action.handle.resume();
        }));
    }
    // Ждем завершения действия Дона
    for (auto& future : futures) {
        future.get();
    }
    futures.clear();
    // Обработка выбора жертв
    if(donId != -1){
        Mafia* donMafiaPlayer = dynamic_cast<Mafia*>(alivePlayers[donId].get());
        victimMafia = donMafiaPlayer->target;
    }
    // Поиск маньяка и комиссара
    auto maniacPlayer = alivePlayers | std::views::filter([](const auto& entry) {
        return entry.second->role() == "Маньяк";
    });

    auto commissionerPlayer = alivePlayers | std::views::filter([](const auto& entry) {
        return entry.second->role() == "Комиссар";
    });

    if (!std::ranges::empty(maniacPlayer)) {
        victimManiac = dynamic_cast<Maniac*>(maniacPlayer.begin()->second.get())->target;
    }

    if (!std::ranges::empty(commissionerPlayer)) {
        victimCom = dynamic_cast<Commissioner*>(commissionerPlayer.begin()->second.get())->killTarget;
    }
    // else{
    //     std::cout << "Пустой комиссар" << std::endl;
    // }
    // std::cout << victimCom << std::endl;

    // Убираем жертв
    if (victimMafia != -1) {
        alivePlayers.erase(victimMafia);
        logger.logRound(round, "Игрок " + std::to_string(victimMafia) + " был убит мафией.");
    }
    if (victimManiac != -1) {
        alivePlayers.erase(victimManiac);
        logger.logRound(round, "Игрок " + std::to_string(victimManiac) + " был убит маньяком.");
    }
    if (victimCom != -1) {
        alivePlayers.erase(victimCom);
        logger.logRound(round, "Игрок " + std::to_string(victimCom) + " был убит комиссаром.");
    }
}

// Фаза дня — голосование
void dayPhase(std::map<int, SmartPointer<Player>>& alivePlayers, Logger& logger, int round, bool user_in_game, int userId) {
    std::map<int, int> voteCount;
    std::vector<std::future<void>> futures;

    // Параллельное голосование всех игроков
    for (const auto& [id, player] : alivePlayers) {
        futures.push_back(std::async(std::launch::async, [&]() {
            if (user_in_game && id == userId) {
                userVotePhase(alivePlayers, userId, logger, round); // Голосование для пользователя
            } else {
                PlayerAction action = player->vote(alivePlayers, id, logger, round);
                action.handle.resume();
            }
        }));
    }

    for (auto& future : futures) {
        future.get(); // Ждем завершения голосования
    }

    // Считаем голоса
    for (const auto& [id, player] : alivePlayers) {
        int target = player->getTarget();
        if (target != -1) {
            voteCount[target]++;
        }
    }

    // Поиск игрока с наибольшим количеством голосов
    if (!voteCount.empty()) {
        auto maxVotesIt = std::max_element(voteCount.begin(), voteCount.end(), 
            [](const auto& a, const auto& b) {
                return a.second < b.second; // Сравниваем количество голосов
            });

        int victim = maxVotesIt->first;

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
        if (player->role() == "Мафия" || player->role() == "Дон мафии") {
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




void run_game(std::map<int, SmartPointer<Player>>& alivePlayers, Logger& logger, bool user_in_game, int userId) {
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
        nightPhase(alivePlayers, logger, round, user_in_game, userId);
        if (checkGameEnd(alivePlayers, logger)) break;

        // Фаза дня
        if(DEBUG_LOG){
            for (const auto& [id, player] : alivePlayers) {
                logger.logRound(round, "Игрок " + std::to_string(id) + ": " + player->role());
            }
        }
        dayPhase(alivePlayers, logger, round, user_in_game, userId);
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
    int userId = -1; // ID пользователя
    std::cout << "Хотите ли вы участвовать в игре? (y/n): ";
    std::cin >> user_in_game_choice;
    if (user_in_game_choice == 'y' || user_in_game_choice == 'Y') {
        user_in_game = true;
    }

    // Расчет количества мафии
    int k = 5; // Можно изменить значение k
    int mafia_count = N / k;
    if (mafia_count == 0) mafia_count = 1;

    // Инициализация игроков
    std::map<int, SmartPointer<Player>> alivePlayers;

    // Добавляем мафию
    for (int i = 1; i <= mafia_count; ++i) {
        alivePlayers[i] = SmartPointer<Player>(new Mafia());
    }
    // Назначаем первую мафию доном
    dynamic_cast<Mafia*>(alivePlayers[1].get())->isDon = true;

    // Добавляем комиссара
    alivePlayers[mafia_count + 1] = SmartPointer<Player>(new Commissioner());

    // Добавляем маньяка
    alivePlayers[mafia_count + 2] = SmartPointer<Player>(new Maniac());

    // Добавляем мирных жителей
    int civilians_count = N - mafia_count - 2; // -2, т.к. комиссар и маньяк уже добавлены
    for (int i = 0; i < civilians_count; ++i) {
        alivePlayers[mafia_count + 3 + i] = SmartPointer<Player>(new Civilian());
    }

    // Если пользователь участвует, назначаем ему случайную роль
    if (user_in_game) {
        userId = assignUserRole(alivePlayers, gen);
    }

    Logger logger;

    // Логирование ролей игроков
    for (const auto& [id, player] : alivePlayers) {
        logger.logRound(0, "Игрок " + std::to_string(id) + ": " + player->role());
    }

    logger.logRound(0, "Игра началась!");

    // Запуск игры
    run_game(alivePlayers, logger, user_in_game, userId);

    return 0;
}