#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <limits>
#include <chrono>
#include <memory>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <condition_variable>

// Абстрактный класс для представления решения
class Solution
{
public:
    // Абстрактный метод для получения стоимости текущего решения
    virtual double getCost() const = 0;
    // Абстрактный метод для печати текущего решения
    virtual void print() const = 0;
    // Метод для создания копии текущего решения
    virtual std::shared_ptr<Solution> clone() const = 0;
    // Метод для обновления решения
    virtual void setSolution(const Solution &other) = 0;
};

// Абстрактный класс для операции изменения (мутации) решения
class MutationOperation
{
public:
    // Абстрактный метод для выполнения мутации решения
    virtual void mutate(Solution &solution) = 0;
};

// Класс для представления решения задачи планирования
class SchedulingSolution : public Solution
{
public:
    SchedulingSolution(int numJobs, int numProcessors, const std::vector<int> &jobDurations)
        : numJobs(numJobs), numProcessors(numProcessors), jobDurations(jobDurations), distribution(0, numProcessors - 1)
    {
        // Инициализация генератора случайных чисел
        std::random_device rd;
        rng.seed(rd());

        // Инициализация случайного начального решения
        schedule.assign(numJobs, std::vector<int>(numProcessors, 0));
        processorLoads.resize(numProcessors, 0);
        for (int i = 0; i < numJobs; ++i)
        {
            int processor = distribution(rng); // Выбираем случайный процессор для каждой работы
            schedule[i][processor] = 1;
            processorLoads[processor] += jobDurations[i];
        }
    }

    // Копирующий конструктор
    SchedulingSolution(const SchedulingSolution &other)
        : numJobs(other.numJobs), numProcessors(other.numProcessors), jobDurations(other.jobDurations),
          schedule(other.schedule), processorLoads(other.processorLoads), rng(other.rng), distribution(other.distribution) {}

    double getCost() const override
    {
        // Критерий K1: Разбалансированность расписания
        int Tmax = *std::max_element(processorLoads.begin(), processorLoads.end()); // Максимальная нагрузка
        int Tmin = *std::min_element(processorLoads.begin(), processorLoads.end()); // Минимальная нагрузка
        return static_cast<double>(Tmax - Tmin);                                    // Разница между максимальной и минимальной нагрузкой
    }

    void print() const override
    {
        // Печать расписания работ
        // for (int i = 0; i < numJobs; ++i)
        // {
        //     std::cout << "Job " << i << ": ";
        //     for (int j = 0; j < numProcessors; ++j)
        //     {
        //         std::cout << schedule[i][j] << " ";
        //     }
        //     std::cout << std::endl;
        // }
        // Печать нагрузки на каждом процессоре
        for (int i = 0; i < numProcessors; ++i)
        {
            std::cout << "Processor " << i << ": Load = " << processorLoads[i] << std::endl;
        }
    }

    std::shared_ptr<Solution> clone() const override
    {
        // Создание копии текущего решения
        return std::make_shared<SchedulingSolution>(*this);
    }

    void setSolution(const Solution &other) override
    {
        const SchedulingSolution &otherSolution = dynamic_cast<const SchedulingSolution &>(other);
        numJobs = otherSolution.numJobs;
        numProcessors = otherSolution.numProcessors;
        jobDurations = otherSolution.jobDurations;
        schedule = otherSolution.schedule;
        processorLoads = otherSolution.processorLoads;
        rng = otherSolution.rng;
        distribution = otherSolution.distribution;
    }

    void updateSchedule(int jobIndex, int oldProcessor, int newProcessor)
    {
        // Обновляем нагрузку процессоров и матрицу расписания
        schedule[jobIndex][oldProcessor] = 0; // Убираем работу с текущего процессора
        schedule[jobIndex][newProcessor] = 1; // Перемещаем работу на новый процессор
        processorLoads[oldProcessor] -= jobDurations[jobIndex];
        processorLoads[newProcessor] += jobDurations[jobIndex];
    }

    int getNumJobs() const { return numJobs; }
    int getNumProcessors() const { return numProcessors; }
    int getJobProcessor(int jobIndex) const
    {
        for (int j = 0; j < numProcessors; ++j)
        {
            if (schedule[jobIndex][j] == 1)
            {
                return j;
            }
        }
        return -1;
    }
    std::mt19937 &getRng() { return rng; }
    std::uniform_int_distribution<int> &getDistribution() { return distribution; }

private:
    int numJobs;                                     // Количество работ
    int numProcessors;                               // Количество процессоров
    std::vector<int> jobDurations;                   // Длительности работ
    std::vector<std::vector<int>> schedule;          // Матрица расписания
    std::vector<int> processorLoads;                 // Нагрузки на процессоры
    mutable std::mt19937 rng;                        // Генератор случайных чисел
    std::uniform_int_distribution<int> distribution; // Распределение для выбора процессора
};

// Класс для операции мутации решения задачи планирования
class SchedulingMutation : public MutationOperation
{
public:
    void mutate(Solution &solution) override
    {
        SchedulingSolution &schedSolution = dynamic_cast<SchedulingSolution &>(solution);
        std::mt19937 &rng = schedSolution.getRng();
        std::uniform_int_distribution<int> &distribution = schedSolution.getDistribution();
        std::uniform_int_distribution<int> jobDist(0, schedSolution.getNumJobs() - 1);

        int jobIndex = jobDist(rng); // Выбираем случайную работу
        int oldProcessor = schedSolution.getJobProcessor(jobIndex);
        int newProcessor = distribution(rng); // Выбираем новый случайный процессор
        while (newProcessor == oldProcessor)
        {
            newProcessor = distribution(rng); // Убеждаемся, что новый процессор отличается от старого
        }

        schedSolution.updateSchedule(jobIndex, oldProcessor, newProcessor); // Обновляем расписание
    }
};

// Абстрактный класс для закона понижения температуры
class CoolingSchedule
{
public:
    // Абстрактный метод для получения следующей температуры
    virtual double getNextTemperature(double currentTemperature, int iteration) const = 0;
};

// Класс для закона T = T_0 * ln(1 + i) / (1 + i)
class LogarithmicCooling : public CoolingSchedule
{
public:
    LogarithmicCooling(double initialTemperature) : initialTemperature(initialTemperature) {}

    double getNextTemperature(double currentTemperature, int iteration) const override
    {
        return initialTemperature * std::log(1 + iteration + 1) / (1 + iteration); // Логарифмическое уменьшение температуры
    }

private:
    double initialTemperature; // Начальная температура
};

// Основной класс для алгоритма имитации отжига
class SimulatedAnnealing
{
public:
    SimulatedAnnealing(std::shared_ptr<Solution> solution, MutationOperation *mutationOperation, CoolingSchedule *coolingSchedule, double initialTemperature, int maxIterations, int maxNoImprovementCount)
        : currentSolution(solution), mutationOperation(mutationOperation), coolingSchedule(coolingSchedule), initialTemperature(initialTemperature), temperature(initialTemperature), maxIterations(maxIterations), maxNoImprovementCount(maxNoImprovementCount)
    {
        // Инициализация генератора случайных чисел
        std::random_device rd;
        rng.seed(rd());
        bestSolution = currentSolution->clone();
        bestCost = bestSolution->getCost();
    }

    // Метод для установки текущего решения (например, после получения лучшего решения от других потоков)
    void setCurrentSolution(const std::shared_ptr<Solution> &solution)
    {
        std::lock_guard<std::mutex> lock(solutionMutex);
        currentSolution->setSolution(*solution);
    }

    // Получение лучшего найденного решения
    std::shared_ptr<Solution> getBestSolution()
    {
        std::lock_guard<std::mutex> lock(solutionMutex);
        return bestSolution->clone();
    }

    double getBestCost()
    {
        std::lock_guard<std::mutex> lock(solutionMutex);
        return bestCost;
    }

    // Основной метод для запуска алгоритма на определенное число итераций
    void runIteration(int iterations)
    {
        int iteration = 0;
        int noImprovementCount = 0;

        while (iteration < iterations && totalIterations < maxIterations && noImprovementCount < maxNoImprovementCount)
        {
            mutationOperation->mutate(*currentSolution);     // Мутация текущего решения
            double currentCost = currentSolution->getCost(); // Стоимость мутированного решения

            std::lock_guard<std::mutex> lock(solutionMutex);

            if (currentCost < bestCost)
            {                                           // Если новое решение лучше
                bestCost = currentCost;                 // Обновляем наилучшую стоимость
                noImprovementCount = 0;                 // Сбрасываем счетчик итераций без улучшений
                bestSolution = currentSolution->clone(); // Сохраняем новое лучшее решение
            }
            else
            {
                // Вероятность принятия ухудшающего решения
                double acceptanceProbability = std::exp(-(currentCost - bestCost) / temperature);
                std::uniform_real_distribution<double> dist(0.0, 1.0);
                if (acceptanceProbability >= dist(rng))
                {
                    // Принять ухудшающее решение
                    noImprovementCount = 0; // Сбрасываем счетчик
                }
                else
                {
                    // Откатываемся к лучшему решению
                    currentSolution->setSolution(*bestSolution);
                    noImprovementCount++; // Увеличиваем счетчик итераций без улучшений
                }
            }
            temperature = coolingSchedule->getNextTemperature(temperature, totalIterations); // Обновляем температуру
            iteration++;
            totalIterations++;
        }
    }

private:
    std::shared_ptr<Solution> currentSolution;         // Текущее решение
    std::shared_ptr<Solution> bestSolution;            // Лучшее найденное решение
    double bestCost;                                   // Стоимость лучшего решения
    MutationOperation *mutationOperation;              // Операция мутации решения
    CoolingSchedule *coolingSchedule;                  // План понижения температуры
    double initialTemperature;                         // Начальная температура
    double temperature;                                // Текущая температура
    int maxIterations;                                 // Максимальное количество итераций
    int maxNoImprovementCount;                         // Условие останова или максимально число итераций без улучшений
    int totalIterations = 0;                           // Общее количество итераций
    std::mutex solutionMutex;                          // Мьютекс для защиты доступа к решениям
    std::mt19937 rng;                                  // Генератор случайных чисел
};

// Класс для параллельного запуска алгоритма имитации отжига
class ParallelSimulatedAnnealing
{
public:
    ParallelSimulatedAnnealing(int numThreads, std::shared_ptr<Solution> initialSolution, MutationOperation *mutationOperation, CoolingSchedule *coolingSchedule, double initialTemperature, int maxIterations, int maxNoImprovementCount)
        : numThreads(numThreads), initialSolution(initialSolution), mutationOperation(mutationOperation), coolingSchedule(coolingSchedule), initialTemperature(initialTemperature), maxIterations(maxIterations), maxNoImprovementCount(maxNoImprovementCount)
    {
        globalBestSolution = initialSolution->clone();
        globalBestCost = globalBestSolution->getCost();
    }

    void run()
    {
        int outerIterationsWithoutImprovement = 0;
        int outerIteration = 0;
        while (outerIterationsWithoutImprovement < 100)
        {
            outerIteration++;
            std::vector<std::thread> threads;
            std::vector<std::shared_ptr<SimulatedAnnealing>> saInstances;

            // Создаем и запускаем экземпляры имитации отжига
            for (int i = 0; i < numThreads; ++i)
            {
                // Создаем копию начального решения для каждого потока
                std::shared_ptr<Solution> solutionCopy = initialSolution->clone();
                auto sa = std::make_shared<SimulatedAnnealing>(solutionCopy, mutationOperation, coolingSchedule, initialTemperature, maxIterations, maxNoImprovementCount);
                saInstances.push_back(sa);

                threads.emplace_back(&ParallelSimulatedAnnealing::threadFunction, this, sa);
            }

            // Ожидаем завершения всех потоков
            for (auto &thread : threads)
            {
                thread.join();
            }

            // Сбор лучших решений от каждого потока
            bool improved = false;
            for (auto &sa : saInstances)
            {
                std::shared_ptr<Solution> localBestSolution = sa->getBestSolution();
                double localBestCost = sa->getBestCost();

                if (localBestCost < globalBestCost)
                {
                    std::lock_guard<std::mutex> lock(globalMutex);
                    if (localBestCost < globalBestCost)
                    {
                        globalBestCost = localBestCost;
                        globalBestSolution = localBestSolution->clone();
                        improved = true;
                    }
                }
            }

            if (improved)
            {
                outerIterationsWithoutImprovement = 0;
                std::cout << "Global best cost improved to: " << globalBestCost << " at outer iteration " << outerIteration << std::endl;
            }
            else
            {
                outerIterationsWithoutImprovement++;
                std::cout << "No improvement in outer iteration " << outerIteration << std::endl;
            }

            // Рассылка лучшего решения всем экземплярам
            initialSolution->setSolution(*globalBestSolution);
        }

        std::cout << "\nFinal best solution found with cost: " << globalBestCost << std::endl;
        globalBestSolution->print();
    }

private:
    void threadFunction(std::shared_ptr<SimulatedAnnealing> sa)
    {
        int iterationsPerThread = 100000; // Количество итераций для каждого потока
        sa->runIteration(iterationsPerThread);
    }

    int numThreads;                                     // Количество потоков
    std::shared_ptr<Solution> initialSolution;          // Начальное решение
    MutationOperation *mutationOperation;               // Операция мутации
    CoolingSchedule *coolingSchedule;                   // План понижения температуры
    double initialTemperature;                          // Начальная температура
    int maxIterations;                                  // Максимальное число итераций
    int maxNoImprovementCount;                          // Максимальное число итераций без улучшений

    std::shared_ptr<Solution> globalBestSolution;       // Глобальное лучшее решение
    double globalBestCost;                              // Стоимость глобального лучшего решения
    std::mutex globalMutex;                             // Мьютекс для защиты глобальных данных
};

std::vector<int> loadJobDurationsFromCSV(const std::string &filename)
{
    std::vector<int> jobDurations;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open file " + filename);
    }

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line))
    {
        if (isHeader)
        {
            // Пропускаем заголовок
            isHeader = false;
            continue;
        }
        std::stringstream ss(line);
        std::string jobId;
        std::string durationStr;

        std::getline(ss, jobId, ',');
        std::getline(ss, durationStr, ',');

        int duration = std::stoi(durationStr);
        jobDurations.push_back(duration);
    }

    file.close();
    return jobDurations;
}

int main()
{
    try
    {
        // Загружаем длительности работ из CSV файла
        std::vector<int> jobDurations = loadJobDurationsFromCSV("jobs.csv");
        int numJobs = jobDurations.size();
        int numProcessors = 8;

        // Инициализируем начальное решение
        std::shared_ptr<Solution> initialSolution = std::make_shared<SchedulingSolution>(numJobs, numProcessors, jobDurations);
        SchedulingMutation mutationOperation;
        LogarithmicCooling coolingSchedule(100.0); // Используем логарифмическое понижение температуры

        double initialTemperature = 100.0;
        int maxIterations = 1000000;
        int maxNoImprovementCount = 10000;

        int numThreads = 8; // Количество параллельных потоков

        ParallelSimulatedAnnealing parallelSA(numThreads, initialSolution, &mutationOperation, &coolingSchedule, initialTemperature, maxIterations, maxNoImprovementCount);
        parallelSA.run(); // Запуск параллельного алгоритма имитации отжига
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
