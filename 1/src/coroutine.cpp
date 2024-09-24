// #include "gamers.cpp"

// struct PlayerCoroutine {
//     struct promise_type {
//         auto get_return_object() { return PlayerCoroutine{}; }
//         std::suspend_always initial_suspend() { return {}; }
//         std::suspend_always final_suspend() noexcept { return {}; }
//         void return_void() {}
//         void unhandled_exception() {}
//     };
// };

// PlayerCoroutine player_action(Player& player) {
//     std::cout << player.role() << " начинает действовать..." << std::endl;
//     player.act();
//     co_await std::suspend_always{};
//     player.vote();
//     std::cout << player.role() << " завершает действия." << std::endl;
// }

// void run_game(std::vector<SmartPointer<Player>>& players) {
//     for (auto& player : players) {
//         if (player->isAlive()) {
//             player_action(*player);
//         }
//     }
// }