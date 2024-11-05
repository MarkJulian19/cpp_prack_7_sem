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

class Solution {
public:
    virtual double getCost() const = 0;
    virtual void print() const = 0;
    virtual std::shared_ptr<Solution> cloneWithNewSeed(unsigned int seed) const = 0;
    virtual std::shared_ptr<Solution> clone() const = 0;
};

std::shared_ptr<Solution> globalBestSolution;
std::mutex globalMutex;

class MutationOperation {
public:
    virtual void mutate(Solution &solution) = 0;
};

class SchedulingSolution : public Solution {
public:
    SchedulingSolution(int numJobs, int numProcessors, const std::vector<uint8_t> &jobDurations, unsigned int seed)
        : numJobs(numJobs), numProcessors(numProcessors), jobDurations(jobDurations), distribution(0, numProcessors - 1) {
        rng.seed(seed);

        schedule.assign(numJobs, std::vector<uint8_t>(numProcessors, 0));
        processorLoads.resize(numProcessors, 0);
        for (int i = 0; i < numJobs; ++i) {
            int processor = distribution(rng);
            schedule[i][processor] = 1;
            processorLoads[processor] += jobDurations[i];
        }
    }

    double getCost() const override {
        int Tmax = *std::max_element(processorLoads.begin(), processorLoads.end());
        int Tmin = *std::min_element(processorLoads.begin(), processorLoads.end());
        return static_cast<double>(Tmax - Tmin);
    }

    void print() const override {
        for (int i = 0; i < numProcessors; ++i) {
            std::cout << "Processor " << i << ": Load = " << processorLoads[i] << std::endl;
        }
    }

    std::shared_ptr<Solution> cloneWithNewSeed(unsigned int seed) const override {
        auto cloned = std::make_shared<SchedulingSolution>(*this);
        cloned->rng.seed(seed); // Устанавливаем новый seed
        return cloned;
    }

    std::shared_ptr<Solution> clone() const override { // Реализация метода clone()
        return std::make_shared<SchedulingSolution>(*this);
    }

    void updateSchedule(int jobIndex, int oldProcessor, int newProcessor) {
        schedule[jobIndex][oldProcessor] = 0;
        schedule[jobIndex][newProcessor] = 1;
        processorLoads[oldProcessor] -= jobDurations[jobIndex];
        processorLoads[newProcessor] += jobDurations[jobIndex];
    }

    int getNumJobs() const { return numJobs; }
    int getNumProcessors() const { return numProcessors; }
    int getJobProcessor(int jobIndex) const {
        for (int j = 0; j < numProcessors; ++j) {
            if (schedule[jobIndex][j] == 1) {
                return j;
            }
        }
        return -1;
    }

    std::mt19937 &getRng() { return rng; }
    std::uniform_int_distribution<int> &getDistribution() { return distribution; }

private:
    int numJobs;
    int numProcessors;
    std::vector<uint8_t> jobDurations;
    std::vector<std::vector<uint8_t>> schedule;
    std::vector<int> processorLoads;
    mutable std::mt19937 rng;
    std::uniform_int_distribution<int> distribution;
};

class SchedulingMutation : public MutationOperation {
public:
    void mutate(Solution &solution) override {
        SchedulingSolution &schedSolution = dynamic_cast<SchedulingSolution &>(solution);
        std::mt19937 &rng = schedSolution.getRng();
        std::uniform_int_distribution<int> &distribution = schedSolution.getDistribution();
        std::uniform_int_distribution<int> jobDist(0, schedSolution.getNumJobs() - 1);

        int jobIndex = jobDist(rng);
        int oldProcessor = schedSolution.getJobProcessor(jobIndex);
        int newProcessor = distribution(rng);
        while (newProcessor == oldProcessor) {
            newProcessor = distribution(rng);
        }

        schedSolution.updateSchedule(jobIndex, oldProcessor, newProcessor);
    }
};

class CoolingSchedule {
public:
    virtual double getNextTemperature(double currentTemperature, int iteration) const = 0;
};

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

class ParallelSimulatedAnnealing {
public:
    ParallelSimulatedAnnealing(Solution *solution, MutationOperation *mutationOperation, CoolingSchedule *coolingSchedule, double initialTemperature, int maxNoImprovementCount, int threadID, unsigned int seed)
        : initialSolution(solution->cloneWithNewSeed(seed)), mutationOperation(mutationOperation), coolingSchedule(coolingSchedule), temperature(initialTemperature),  maxNoImprovementCount(maxNoImprovementCount), threadID(threadID), rng(seed) {}

    void run() {
        int iteration = 0;
        double bestCost = initialSolution->getCost(); // Изначальная стоимость решения
        auto bestSolution = initialSolution->clone(); // Копия наилучшего решения
        int noImprovementCount = 0;                   // Счетчик количества итераций без улучшения
        // std::uniform_real_distribution<double> realDist(0.0, 1.0);

        while (noImprovementCount < maxNoImprovementCount) {
            // Клонируем лучшее решение и применяем к нему мутацию
            auto currentSolution = bestSolution->clone();
            mutationOperation->mutate(*currentSolution);
            double currentCost = currentSolution->getCost(); // Стоимость мутированного решения

            if (currentCost < bestCost) {
                // Если новое решение лучше, обновляем наилучшее решение
                bestCost = currentCost;
                noImprovementCount = 0;
                bestSolution = currentSolution;
            } else {
                // Если решение хуже, то принимаем его с некоторой вероятностью (правило Метрополиса)
                double acceptanceProbability = std::exp(-(currentCost - bestCost) / temperature);
                if (acceptanceProbability >= static_cast<double>(rand()) / RAND_MAX) {
                    // Принять ухудшающее решение и обновить лучшее решение
                    noImprovementCount = 0;
                    bestSolution = currentSolution;
                } else {
                    // Если решение не принято, увеличиваем счетчик итераций без улучшений
                    noImprovementCount++;
                }
            }
            // Обновляем температуру согласно закону понижения температуры
            temperature = coolingSchedule->getNextTemperature(temperature, iteration);
            iteration++;
        }
        // Сохраняем локально лучшее решение
        localBestSolution = bestSolution;
    }

    std::shared_ptr<Solution> getLocalBestSolution() const {
        return localBestSolution;
    }

private:
    std::shared_ptr<Solution> initialSolution;
    std::shared_ptr<Solution> localBestSolution;
    MutationOperation *mutationOperation;
    CoolingSchedule *coolingSchedule;
    double temperature;
    int maxIterations;
    int maxNoImprovementCount;
    int threadID;
    std::mt19937 rng;
};

std::vector<uint8_t> loadJobDurationsFromCSV(const std::string &filename) {
    std::vector<uint8_t> jobDurations;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file " + filename);
    }

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (isHeader) {
            isHeader = false;
            continue;
        }
        std::stringstream ss(line);
        std::string jobId;
        std::string durationStr;

        std::getline(ss, jobId, ',');
        std::getline(ss, durationStr, ',');

        uint8_t duration = std::stoi(durationStr);
        jobDurations.push_back(duration);
    }

    file.close();
    return jobDurations;
}

int main(int argc, char *argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <numThreads>" << std::endl;
            return 1;
        }

        int numThreads = std::stoi(argv[1]);
        std::vector<uint8_t> jobDurations = loadJobDurationsFromCSV("jobs.csv");
        int numJobs = jobDurations.size();
        int numProcessors = 40;
        int maxNoImprovementCount = 100;
        int maxGlobalNoImprovementCount = 10;

        SchedulingMutation mutationOperation;
        BoltzmannCooling coolingSchedule(100.0);
        double initialTemperature = 100.0;

        int globalNoImprovementCount = 0;

        {
            std::lock_guard<std::mutex> lock(globalMutex);
            if (!globalBestSolution) {
                globalBestSolution = std::make_shared<SchedulingSolution>(numJobs, numProcessors, jobDurations, std::chrono::system_clock::now().time_since_epoch().count());
            }
        }

        while (globalNoImprovementCount < maxGlobalNoImprovementCount) {
            std::vector<std::thread> threads;
            std::vector<std::shared_ptr<Solution>> localBestSolutions(numThreads);

            for (int i = 0; i < numThreads; ++i) {
                threads.emplace_back([&, i]() {
                    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count() + i;
                    std::shared_ptr<Solution> initialSolution;

                    {
                        std::lock_guard<std::mutex> lock(globalMutex);
                        initialSolution = globalBestSolution->cloneWithNewSeed(seed);
                    }

                    ParallelSimulatedAnnealing sa(initialSolution.get(), &mutationOperation, &coolingSchedule, initialTemperature, maxNoImprovementCount, i, seed);
                    sa.run();

                    localBestSolutions[i] = sa.getLocalBestSolution();
                });
            }

            for (auto &t : threads) {
                t.join();
            }

            bool improved = false;
            for (const auto &localBest : localBestSolutions) {
                if (localBest->getCost() < globalBestSolution->getCost()) {
                    std::lock_guard<std::mutex> lock(globalMutex);
                    globalBestSolution = localBest;
                    // std::cout << "Found Improved Solution" << std::endl;
                    improved = true;
                }
            }
            // std::cout << "Improved: " << improved << std::endl;
            if (improved) {
                globalNoImprovementCount = 0;
            } else {
                globalNoImprovementCount++;
            }

            // std::cout << "Current best solution cost: " << globalBestSolution->getCost() << std::endl;
        }
        std::cout << "Current best solution cost: " << globalBestSolution->getCost() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
