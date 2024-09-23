#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include <memory>
#include "src/gamers.cpp"



int main()
{
    int num_players = 10;
    Game game(num_players);
    game.start();
    return 0;
}