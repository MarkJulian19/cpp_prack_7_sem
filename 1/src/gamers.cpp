#include <set>
#include <coroutine>
#include <vector>
#include <map>
#include "smart_pointer.cpp"
#include "logger.cpp"
// Функция для случайного выбора из нескольких кандидатов
template <typename T>
T choose_random(const std::vector<T>& candidates) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, candidates.size() - 1);
    return candidates[dist(gen)];
}


//тип возврата для корутин. Она содержит внутреннюю структуру promise_type, которая определяет поведение корутины,
//и корутинный хэндл (std::coroutine_handle), который контролирует выполнение этой корутины (приостановка, возобновление, завершение).
struct PlayerAction {
    //обязательная часть корутины в C++. Этот объект отвечает за управление состоянием корутины. Он определяет методы, которые управляют началом, завершением, возвратом значений и обработкой исключений.
    struct promise_type {
        //создаёт и возвращает объект PlayerAction. Этот метод вызывается автоматически при создании корутины и связывает объект с корутиной через std::coroutine_handle. Этот хэндл даёт возможность управлять выполнением корутины (например, возобновлять её работу).
        PlayerAction get_return_object() {
            return PlayerAction{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        //указывает, как корутина должна вести себя при запуске. В данном случае возвращает std::suspend_always, что означает, что корутина приостановит своё выполнение сразу после запуска.
        std::suspend_always initial_suspend() { return {}; }
        //указывает, что корутина должна сделать при завершении. В данном случае снова возвращается std::suspend_always, что означает, что корутина будет приостановлена и после завершения.
        std::suspend_always final_suspend() noexcept { return {}; }
        //метод вызывается, когда корутина завершает выполнение и не возвращает значение (так как корутина возвращает void).
        void return_void() {}
        //вызывается, если в корутине произошло необработанное исключение.
        void unhandled_exception() {}
    };
    //хэндл корутины, позволяющий контролировать её выполнение, возобновление и завершение.
    std::coroutine_handle<promise_type> handle;
    //в деструкторе вызывается метод handle.destroy(), который завершает корутину и освобождает ресурсы, связанные с её состоянием.
    ~PlayerAction() { handle.destroy(); }
};




class Player {
public:
    int target = -1;
    int killTarget = -1;
    int previousTarget = -1;
    virtual ~Player() = default;
    virtual PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round, bool mafiaR) const = 0; // Действие игрока
    virtual PlayerAction vote(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const = 0; // Голосование
    virtual std::string role() const = 0; // Название роли
    virtual int getTarget() const {
        return -1; // По умолчанию возвращаем -1
    }
    virtual void setTarget(int t){
        target = t;
    }
    virtual void setKill(int t){
        killTarget = t;
    }
    virtual int getPrevTarget() const{
        return -1;
    }
    virtual void setPrevTarget(int t){
        previousTarget = t;
    }
};

// Мировые роли
class Civilian : public Player {
public:
    int target = -1;
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round, bool mafiaR) const override {
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
    int getTarget() const override {
        return target;
    }
    void setTarget(int t){
        target = t;
    }
};

class Mafia : public Player {
public:
    int target = -1; // выбранная жертва
    bool isDon = false;

    PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round, bool mafiaR) const override {
        if (mafiaR){
            std::vector<int> targets;
            // Получение списка доступных целей
            for (const auto& [targetId, targetPlayer] : alivePlayers) {
                if (targetPlayer->role() != "Мафия" && targetPlayer->role() != "Дон мафии") {
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
        else {
            // Дон мафии собирает цели мафиози
            std::map<int, int> targetCount;  // Счетчик для каждого игрока-цели
            for (const auto& [targetId, targetPlayer] : alivePlayers) {
                if (targetPlayer->role() == "Мафия" || targetPlayer->role() == "Дон мафии") {
                    Mafia* mafiaPlayer = dynamic_cast<Mafia*>(targetPlayer.get());
                    if (mafiaPlayer && mafiaPlayer->target != -1) {
                        targetCount[mafiaPlayer->target]++;  // Увеличиваем счетчик для выбранной цели
                    }
                }
            }

            if (!targetCount.empty()) {
                // Поиск максимального числа голосов
                int maxVotes = 0;
                std::vector<int> mostVotedTargets;

                for (const auto& [target, count] : targetCount) {
                    if (count > maxVotes) {
                        maxVotes = count;
                        mostVotedTargets = { target };  // Обновляем список целей
                    } else if (count == maxVotes) {
                        mostVotedTargets.push_back(target);  // Добавляем, если количество голосов одинаковое
                    }
                }

                // Если несколько целей, выбираем случайную из них
                int chosenTarget = choose_random(mostVotedTargets);
                const_cast<Mafia*>(this)->target = chosenTarget;  // Дон выбирает цель

                logger.logRound(round, "Дон мафии " + std::to_string(id) + " выбрал игрока " + std::to_string(chosenTarget) + " на основе голосов мафии.");
            }

            co_return;  // Завершаем действие Дона мафии
        }
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

    std::string role() const override { return isDon ? "Дон мафии" : "Мафия"; }
    int getTarget() const override {
        return target;
    }
    void setTarget(int t){
        target = t;
    }
};

class Commissioner : public Player {
public:
    mutable int comTarget = -1;  // Запоминаем ID мафии, обнаруженной комиссаром в предыдущем раунде
    mutable int killTarget = -1; // Цель на убийство
    mutable int target = -1;     // Цель для голосования или проверки
    mutable std::set<int> knownCivilians; // Набор известных мирных жителей (не мафия)

    // Действие комиссара: проверка случайного игрока ночью
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round, bool mafiaR) const override {
        // Если в прошлом раунде был найден мафия, выбираем его целью
        if (comTarget != -1 && alivePlayers.find(comTarget) != alivePlayers.end()) {
            killTarget = comTarget;
            comTarget = -1;
            logger.logRound(round, "Комиссар знает, что игрок " + std::to_string(killTarget) + " мафия.");
            co_return;
        } else {
            // Если мафия ещё не найдена, проверяем случайного игрока
            std::vector<int> targets;
            for (const auto& [targetId, targetPlayer] : alivePlayers) {
                if (targetId != id && knownCivilians.find(targetId) == knownCivilians.end()) {
                    // Комиссар не проверяет сам себя и тех, кто уже проверен как мирный
                    targets.push_back(targetId);
                }
            }

            if (!targets.empty()) {
                int checkedPlayerId = choose_random(targets); // Выбираем случайного игрока для проверки
                const std::string& role = alivePlayers.at(checkedPlayerId)->role();
                
                if (role == "Мафия" || role == "Дон мафии") {
                    // Комиссар обнаружил мафию и запоминает его ID в `comTarget`
                    comTarget = checkedPlayerId;
                    logger.logRound(round, "Комиссар проверил игрока " + std::to_string(checkedPlayerId) + " и обнаружил, что он мафия.");
                } else {
                    // Если это мирный житель, добавляем его в список известных мирных
                    knownCivilians.insert(checkedPlayerId);
                    logger.logRound(round, "Комиссар проверил игрока " + std::to_string(checkedPlayerId) + ", он не мафия.");
                }
            } else {
                logger.logRound(round, "Комиссару некого проверять, все возможные игроки уже известны.");
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
            logger.logRound(round, "Комиссар " + std::to_string(id) + " воздержался от голоса.");
            target = -1;
        }

        co_return;
    }

    // Определение роли комиссара
    std::string role() const override { return "Комиссар"; }

    // Возвращаем текущую цель комиссара
    int getTarget() const override {
        return target;
    }
    void setTarget(int t){
        target = t;
    }
    void setKill(int t){
        killTarget = t;
    }
};


class Maniac : public Player {
public:
    int target = -1;
    PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round, bool mafiaR) const override {
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
    int getTarget() const override {
        return target;
    }
    void setTarget(int t){
        target = t;
    }
};
class Doctor : public Player {
public:
    mutable int previousTarget = -1;
    mutable int target = -1;

    PlayerAction act(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round, bool mafiaR) const override {
        std::vector<int> targets;
        for (const auto& [targetId, targetPlayer] : alivePlayers) {
            if (targetId != previousTarget) {
                targets.push_back(targetId);
            }
        }

        if (!targets.empty()) {
            int chosenTarget = choose_random(targets);
            const_cast<Doctor*>(this)->target = chosenTarget;
            const_cast<Doctor*>(this)->previousTarget = chosenTarget;
            logger.logRound(round, "Доктор " + std::to_string(id) + " лечит игрока " + std::to_string(chosenTarget) + ".");
        } else {
            const_cast<Doctor*>(this)->target = -1;
            logger.logRound(round, "Доктор " + std::to_string(id) + " не нашел, кого лечить и пропускает ход.");
        }

        co_return;
    }

    PlayerAction vote(const std::map<int, SmartPointer<Player>>& alivePlayers, int id, Logger& logger, int round) const override {
        int chosenTarget = -1;
        std::vector<int> targets;
        for (const auto& [targetId, targetPlayer] : alivePlayers) {
            targets.push_back(targetId);
        }
        chosenTarget = choose_random(targets);
        const_cast<Doctor*>(this)->target = chosenTarget;
        logger.logRound(round, "Доктор " + std::to_string(id) + " выбрал игрока " + std::to_string(target) + " на голосовании.");
        co_return;
    }

    std::string role() const override { return "Доктор"; }
    int getTarget() const override {
        return target;
    }
    void setTarget(int t) {
        target = t;
    }
    int getPrevTarget() const {
        return previousTarget;
    }
    void setPrevTarget(int t){
        previousTarget = t;
    }
};
// Концепт для определения необходимых методов игрока
// template<typename T>
// concept PlayerConcept = requires(T player) {
//     { player.role() } -> std::convertible_to<std::string>;
//     { player.act(std::declval<std::map<int, SmartPointer<Player>>&, int, Logger&, int, bool>()) } -> std::same_as<PlayerAction>;
//     { player.vote(std::declval<std::map<int, SmartPointer<Player>>&, int, Logger&, int>()) } -> std::same_as<PlayerAction>;
//     { player.setTarget(std::declval<int>()) };
//     { player.getTarget() } -> std::convertible_to<int>;
//     { player.setKill(std::declval<int>()) };
// };