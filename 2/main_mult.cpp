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
};

std::shared_ptr<Solution> globalBestSolution;
std::mutex globalMutex;

class MutationOperation {
public:
    virtual void mutate(Solution &solution) = 0;
};

class SchedulingSolution : public Solution {
public:
    SchedulingSolution(int numJobs, int numProcessors, const std::vector<int> &jobDurations, unsigned int seed)
        : numJobs(numJobs), numProcessors(numProcessors), jobDurations(jobDurations), distribution(0, numProcessors - 1) {
        rng.seed(seed);

        schedule.assign(numJobs, std::vector<int>(numProcessors, 0));
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
    std::vector<int> jobDurations;
    std::vector<std::vector<int>> schedule;
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

class LogarithmicCooling : public CoolingSchedule {
public:
    LogarithmicCooling(double initialTemperature) : initialTemperature(initialTemperature) {}

    double getNextTemperature(double currentTemperature, int iteration) const override {
        return initialTemperature * std::log(1 + iteration + 1) / (1 + iteration);
    }

private:
    double initialTemperature;
};

class ParallelSimulatedAnnealing {
public:
    ParallelSimulatedAnnealing(Solution *solution, MutationOperation *mutationOperation, CoolingSchedule *coolingSchedule, double initialTemperature, int maxIterations, int threadID, unsigned int seed)
        : solution(solution->cloneWithNewSeed(seed)), mutationOperation(mutationOperation), coolingSchedule(coolingSchedule), temperature(initialTemperature), maxIterations(maxIterations), threadID(threadID) {}

    void run() {
        int iteration = 0;
        double bestCost = solution->getCost();
        auto bestSolution = solution->cloneWithNewSeed(std::chrono::system_clock::now().time_since_epoch().count());

        while (iteration < maxIterations) {
            mutationOperation->mutate(*solution);
            double currentCost = solution->getCost();

            if (currentCost < bestCost) {
                bestCost = currentCost;
                bestSolution = solution->cloneWithNewSeed(std::chrono::system_clock::now().time_since_epoch().count());
            }
            temperature = coolingSchedule->getNextTemperature(temperature, iteration);
            iteration++;
        }

        std::lock_guard<std::mutex> lock(globalMutex);
        if (!globalBestSolution || bestSolution->getCost() < globalBestSolution->getCost()) {
            globalBestSolution = bestSolution;
        }
    }

private:
    std::shared_ptr<Solution> solution;
    MutationOperation *mutationOperation;
    CoolingSchedule *coolingSchedule;
    double temperature;
    int maxIterations;
    int threadID;
};

std::vector<int> loadJobDurationsFromCSV(const std::string &filename) {
    std::vector<int> jobDurations;
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

        int duration = std::stoi(durationStr);
        jobDurations.push_back(duration);
    }

    file.close();
    return jobDurations;
}

int main() {
    try {
        std::vector<int> jobDurations = loadJobDurationsFromCSV("jobs.csv");
        int numJobs = jobDurations.size();
        int numProcessors = 8;
        int numThreads = 14;
        int numIterations = 20;

        SchedulingMutation mutationOperation;
        LogarithmicCooling coolingSchedule(100.0);
        double initialTemperature = 100.0;
        int maxIterations = 100;

        for (int iter = 0; iter < numIterations; ++iter) {
            std::vector<std::thread> threads;

            for (int i = 0; i < numThreads; ++i) {
                threads.emplace_back([&, i]() {
                    unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count() + i;
                    std::shared_ptr<Solution> initialSolution;
                    
                    {
                        std::lock_guard<std::mutex> lock(globalMutex);
                        if (globalBestSolution) {
                            initialSolution = globalBestSolution->cloneWithNewSeed(seed);
                        } else {
                            initialSolution = std::make_shared<SchedulingSolution>(numJobs, numProcessors, jobDurations, seed);
                        }
                    }

                    ParallelSimulatedAnnealing sa(initialSolution.get(), &mutationOperation, &coolingSchedule, initialTemperature, maxIterations, i, seed);
                    sa.run();
                });
            }

            for (auto &t : threads) {
                t.join();
            }

            if (globalBestSolution) {
                std::cout << "Iteration " << iter + 1 << ": Best solution found with cost: " << globalBestSolution->getCost() << std::endl;
                globalBestSolution->print();
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
