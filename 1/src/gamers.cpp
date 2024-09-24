// Coroutine structure for player actions
// Функция для случайного выбора из нескольких кандидатов
template <typename T>
T choose_random(const std::vector<T>& candidates) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, candidates.size() - 1);
    return candidates[dist(gen)];
}


struct PlayerAction {
    struct promise_type {
        PlayerAction get_return_object() {
            return PlayerAction{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> handle;

    ~PlayerAction() { handle.destroy(); }
};


// Концепт для определения необходимых методов игрока
template<typename T>
concept PlayerConcept = requires(T a) {
    { a.act() } -> std::same_as<void>;
    { a.vote() } -> std::same_as<void>;
    { a.role() } -> std::same_as<std::string>;
};

class Player {
public:
    virtual ~Player() = default;
    virtual PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const = 0; // Действие игрока
    virtual PlayerAction vote(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const = 0; // Голосование
    virtual std::string role() const = 0; // Название роли
};

// Мировые роли
class Civilian : public Player {
public:
    int target = -1;
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const override {
        co_return; // no action for civilians
    }

    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const override {
        // std::cout << "Мирный житель голосует." << std::endl;
        int chosenTarget = -1; 
        std::vector<int> targets;
        // Получение списка доступных целей
        for (const auto& [targetId, targetPlayer] : alivePlayers) {
            targets.push_back(targetId);
        }
        chosenTarget = choose_random(targets);
        const_cast<Civilian*>(this)->target = chosenTarget;
        logger.logRound(round, "Мирный "+ std::to_string(id) +" выбрал игрока " + std::to_string(target) + " на голосовании.");
        co_return;
    }

    std::string role() const override { return "Мирный житель"; }
};

class Mafia : public Player {
public:
    int target = -1; // выбранная жертва

    PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const override {
        std::vector<int> targets;
        // Получение списка доступных целей
        for (const auto& [targetId, targetPlayer] : alivePlayers) {
            if (targetPlayer->role() != "Мафия") {
                targets.push_back(targetId);
            }
        }

        int chosenTarget = -1; // Локальная переменная для хранения выбранной цели

        if (!targets.empty()) {
            // Выбор случайной цели
            chosenTarget = choose_random(targets);
        }

        // Если требуется сохранить цель для дальнейшего использования, вы можете это сделать
        // Например, если это для голосования в следующем раунде:
        const_cast<Mafia*>(this)->target = chosenTarget; // Приведение const для изменения значения
        logger.logRound(round, "Мафия "+ std::to_string(id) +" выбрала игрока " + std::to_string(target) + " для убийства.");
        co_return; // завершение действия
    }

    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const override {
        int chosenTarget = -1; 
        std::vector<int> targets;
        // Получение списка доступных целей
        for (const auto& [targetId, targetPlayer] : alivePlayers) {
            targets.push_back(targetId);
        }
        chosenTarget = choose_random(targets);
        const_cast<Mafia*>(this)->target = chosenTarget;
        logger.logRound(round, "Мафия "+ std::to_string(id) +" выбрала игрока " + std::to_string(target) + " на голосовании.");
        co_return;
    }

    std::string role() const override { return "Мафия"; }
};

class Commissioner : public Player {
public:
    mutable int comTarget = -1; // Запоминаем ID мафии, обнаруженной комиссаром в предыдущем раунде
    mutable int killTarget = -1;
    mutable int target = -1;    // Цель для голосования или убийства

    // Действие комиссара: проверка случайного игрока ночью
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const override {
        // Если в прошлом раунде был найден мафия, выбираем его целью
        if (comTarget != -1 && alivePlayers.find(comTarget) != alivePlayers.end()) {
            killTarget = comTarget;
            comTarget = -1;
            logger.logRound(round, "Комиссар знает, что игрок " + std::to_string(target) + " мафия.");
            co_return;
        }
        else{
            // Если мафия ещё не найдена, проверяем случайного игрока
            std::vector<int> targets;
            for (const auto& [targetId, targetPlayer] : alivePlayers) {
                if (targetId != id) { // Комиссар не проверяет сам себя
                    targets.push_back(targetId);
                }
            }

            if (!targets.empty()) {
                int checkedPlayerId = choose_random(targets); // Выбираем случайного игрока для проверки
                const std::string& role = alivePlayers.at(checkedPlayerId)->role();
                
                if (role == "Мафия") {
                    // Комиссар обнаружил мафию и запоминает его ID в `comTarget`
                    comTarget = checkedPlayerId;
                    logger.logRound(round, "Комиссар проверил игрока " + std::to_string(checkedPlayerId) + " и обнаружил, что он мафия.");
                } else {
                    logger.logRound(round, "Комиссар проверил игрока " + std::to_string(checkedPlayerId) + ", он не мафия.");
                }
            }
            }
        co_return;
    }

    // Голосование комиссара: если мафия обнаружена, комиссар целенаправленно голосует против неё
    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const override {
        killTarget = -1;
        if (comTarget != -1 && alivePlayers.find(comTarget) != alivePlayers.end()) {
            // Если мафия обнаружена и она ещё жива, голосуем за неё
            target = comTarget;
            logger.logRound(round, "Комиссар " + std::to_string(id) + " голосует за игрока " + std::to_string(target) + " (обнаруженный мафия).");
        } else {
            // Если мафия не обнаружена или её больше нет, голосуем случайным образом
            std::vector<int> targets;
            for (const auto& [targetId, targetPlayer] : alivePlayers) {
                if (targetId != id) {
                    targets.push_back(targetId);
                }
            }
            if (!targets.empty()) {
                target = choose_random(targets);
                logger.logRound(round, "Комиссар " + std::to_string(id) + " голосует за случайного игрока " + std::to_string(target) + ".");
            }
        }

        co_return;
    }

    std::string role() const override { return "Комиссар"; }
};

class Maniac : public Player {
public:
    int target = -1;
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const override {
        std::vector<int> targets;
        // Получение списка доступных целей
        for (const auto& [targetId, targetPlayer] : alivePlayers) {
            targets.push_back(targetId);
        }

        int chosenTarget = -1; // Локальная переменная для хранения выбранной цели

        if (!targets.empty()) {
            // Выбор случайной цели
            chosenTarget = choose_random(targets);
        }

        // Если требуется сохранить цель для дальнейшего использования, вы можете это сделать
        // Например, если это для голосования в следующем раунде:
        const_cast<Maniac*>(this)->target = chosenTarget; // Приведение const для изменения значения
        logger.logRound(round, "Маньяк "+ std::to_string(id) +" выбрала игрока " + std::to_string(target) + " для убийства.");
        co_return;
    }

    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const override {
        int chosenTarget = -1; 
        std::vector<int> targets;
        // Получение списка доступных целей
        for (const auto& [targetId, targetPlayer] : alivePlayers) {
            targets.push_back(targetId);
        }
        chosenTarget = choose_random(targets);
        const_cast<Maniac*>(this)->target = chosenTarget;
        logger.logRound(round, "Маньяк "+ std::to_string(id) +" выбрал игрока " + std::to_string(target) + " на голосовании.");
        co_return;
    }

    std::string role() const override { return "Маньяк"; }
};