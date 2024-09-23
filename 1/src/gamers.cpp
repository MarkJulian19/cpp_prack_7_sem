#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include <memory>
#include "src/smart_pointer.cpp"

class Player
{
public:
    virtual void act() = 0;
    virtual void vote() = 0;
    virtual ~Player() = default;
};

class Mafia : public Player
{
public:
    void act() override
    {
        // Реализация действий мафии
    }
    void vote() override
    {
        // Реализация голосования мафии
    }
};

class Citizen : public Player
{
public:
    void act() override
    {
        // Реализация действий мирного жителя
    }
    void vote() override
    {
        // Реализация голосования мирного жителя
    }
};

class Commissioner : public Player
{
public:
    void act() override
    {
        // Реализация действий комиссара
    }
    void vote() override
    {
        // Реализация голосования комиссара
    }
};

class Maniac : public Player
{
public:
    void act() override
    {
        // Реализация действий маньяка
    }
    void vote() override
    {
        // Реализация голосования маньяка
    }
};

class Game
{
private:
    std::vector<SmartPointer<Player>> players;
    std::mutex mtx;
    std::condition_variable cv;
    bool game_over = false;

public:
    Game(int num_players)
    {
        // Инициализация игроков
        for (int i = 0; i < num_players; ++i)
        {
            if (i % 4 == 0)
            {
                players.push_back(SmartPointer<Player>(new Mafia()));
            }
            else if (i % 4 == 1)
            {
                players.push_back(SmartPointer<Player>(new Citizen()));
            }
            else if (i % 4 == 2)
            {
                players.push_back(SmartPointer<Player>(new Commissioner()));
            }
            else
            {
                players.push_back(SmartPointer<Player>(new Maniac()));
            }
        }
    }

    void start()
    {
        std::vector<std::thread> threads;
        for (auto &player : players)
        {
            threads.emplace_back([this, &player]()
                                 {
                while (!game_over) {
                    player->act();
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    player->vote();
                } });
        }

        for (auto &thread : threads)
        {
            thread.join();
        }
    }

    void end_game()
    {
        std::lock_guard<std::mutex> lock(mtx);
        game_over = true;
        cv.notify_all();
    }
};