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
        for (int i = 0; i < numJobs; ++i)
        {
            std::cout << "Job " << i << ": ";
            for (int j = 0; j < numProcessors; ++j)
            {
                std::cout << schedule[i][j] << " ";
            }
            std::cout << std::endl;
        }
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

// Класс для закона Больцмана
class BoltzmannCooling : public CoolingSchedule
{
public:
    BoltzmannCooling(double initialTemperature) : initialTemperature(initialTemperature) {}

    double getNextTemperature(double currentTemperature, int iteration) const override
    {
        return initialTemperature / std::log(1 + iteration + 1); // Температура уменьшается по закону Больцмана
    }

private:
    double initialTemperature; // Начальная температура
};

// Класс для закона Коши
class CauchyCooling : public CoolingSchedule
{
public:
    CauchyCooling(double initialTemperature) : initialTemperature(initialTemperature) {}

    double getNextTemperature(double currentTemperature, int iteration) const override
    {
        return initialTemperature / (1 + iteration); // Температура уменьшается по закону Коши
    }

private:
    double initialTemperature; // Начальная температура
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
    SimulatedAnnealing(Solution *solution, MutationOperation *mutationOperation, CoolingSchedule *coolingSchedule, double initialTemperature, int maxIterations, int maxNoImprovementCount)
        : solution(solution), mutationOperation(mutationOperation), coolingSchedule(coolingSchedule), temperature(initialTemperature), maxIterations(maxIterations), maxNoImprovementCount(maxNoImprovementCount) {}

    void run()
    {
        int iteration = 0;
        double bestCost = solution->getCost(); // Изначальная стоимость решения
        auto bestSolution = solution->clone(); // Копия наилучшего решения
        int noImprovementCount = 0;            // Счетчик количества итераций без улучшения

        while (iteration < maxIterations && noImprovementCount < maxNoImprovementCount)
        {
            mutationOperation->mutate(*solution);     // Мутация текущего решения
            double currentCost = solution->getCost(); // Стоимость мутированного решения
            if (currentCost < bestCost)
            {                                     // Если новое решение лучше
                bestCost = currentCost;           // Обновляем наилучшую стоимость
                noImprovementCount = 0;           // Сбрасываем счетчик итераций без улучшений
                bestSolution = solution->clone(); // Сохраняем новое лучшее решение
            }
            else
            {
                // Вероятность принятия ухудшающего решения
                double acceptanceProbability = std::exp(-(currentCost - bestCost) / temperature);
                if (acceptanceProbability >= static_cast<double>(rand()) / RAND_MAX)
                {
                    noImprovementCount = 0; // Принять ухудшающее решение и сбросить счетчик
                }
                else
                {
                    noImprovementCount++; // Увеличиваем счетчик итераций без улучшений
                }
            }
            temperature = coolingSchedule->getNextTemperature(temperature, iteration); // Обновляем температуру
            iteration++;
        }
        bestSolution->print(); // Печатаем наилучшее найденное решение
        std::cout << "Best solution found with cost: " << bestCost << std::endl;
    }

private:
    Solution *solution;                   // Текущее решение
    MutationOperation *mutationOperation; // Операция мутации решения
    CoolingSchedule *coolingSchedule;     // План понижения температуры
    double temperature;                   // Текущая температура
    int maxIterations;                    // Максимальное количество итераций
    int maxNoImprovementCount;            // Условие останова или максимально число иттераций без улучшений
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

        SchedulingSolution solution(numJobs, numProcessors, jobDurations);
        SchedulingMutation mutationOperation;
        LogarithmicCooling coolingSchedule(100.0); // Используем логарифмическое понижение температуры

        double initialTemperature = 100.0;
        int maxIterations = 100000000;
        int maxNoImprovementCount = 100000000;
        SimulatedAnnealing sa(&solution, &mutationOperation, &coolingSchedule, initialTemperature, maxIterations, maxNoImprovementCount);
        sa.run(); // Запуск алгоритма имитации отжига
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
