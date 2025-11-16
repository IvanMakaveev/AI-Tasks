// Ivan Makaveev, 2MI0600203
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <numeric>
#include <chrono>

/*
    Settings
*/
const size_t POPULATION_SIZE = 1000;
const size_t GENERATIONS = 500;
const size_t MUTATION_PERCENT = 70;
const size_t ELITISM_COUNT = 5;

/*
    Random generators
*/

std::random_device deviceSeed;
std::mt19937 randomGenerator(deviceSeed());

int getRandomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(randomGenerator);
}

double getRandomDouble(double min, double max)
{
    std::uniform_real_distribution<double> dist(min, max);
    return dist(randomGenerator);
}

double eucledianDistance(const std::pair<double, double>& first, const std::pair<double, double>& second)
{
    double dx = first.first - second.first;
    double dy = first.second - second.second;
    return sqrt(dx * dx + dy * dy);
}

std::vector<int> getRandomRoute(size_t citiesCount)
{
    std::vector<int> route(citiesCount);
    std::iota(route.begin(), route.end(), 0);
    std::shuffle(route.begin(), route.end(), randomGenerator);

    return route;
}

/*
    Input parser
*/

bool isInteger(const std::string& str)
{
    if (str.empty())
        return false;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (!std::isdigit(static_cast<unsigned char>(str[i])))
            return false;
    }

    return true;
}

void generateRandomDataset(
    const std::string& datasetName,
    size_t& citiesCount,
    std::vector<std::string>& cityNames,
    std::vector<std::pair<double, double>>& cityCoords
)
{
    citiesCount = std::stoi(datasetName);
    cityCoords.reserve(citiesCount);
    for (size_t i = 0; i < citiesCount; i++)
    {
        cityCoords.push_back({ getRandomDouble(-100, 100), getRandomDouble(-100, 100) });
    }
}

void readDataset(size_t& citiesCount, std::vector<std::string>& cityNames, std::vector<std::pair<double, double>>& cityCoords)
{
    std::cin >> citiesCount;
    cityNames.resize(citiesCount);
    cityCoords.resize(citiesCount);

    for (size_t i = 0; i < citiesCount; i++)
    {
        std::cin >> cityNames[i];
        std::cin >> cityCoords[i].first;
        std::cin >> cityCoords[i].second;
    }
}

/*
    Speciment structure
*/

struct Speciment
{
    std::vector<int> route;
    double fitnessScore;

    Speciment(std::vector<int>&& route) : route(std::move(route)), fitnessScore(0)
    {

    }

    Speciment(const std::vector<std::vector<double>>& matrix, std::vector<int>&& route)
        : route(std::move(route)), fitnessScore(calculateFitness(matrix))
    {

    }

    double calculateFitness(const std::vector<std::vector<double>>& matrix)
    {
        double result = 0;
        for (size_t i = 0; i < route.size() - 1; i++)
            result += matrix[route[i]][route[(i + 1) % route.size()]];

        return result;
    }

    bool operator<(const Speciment& other) const
    {
        return fitnessScore < other.fitnessScore;
    }

    double weight() const
    {
        return 1 / fitnessScore;
    }
};

/*
    Selection strategies:
*/

double findFitnessSum(const std::vector<Speciment>& population)
{
    double fitnessSum = 0;
    for (const auto& speciment : population)
        fitnessSum += speciment.weight();

    return fitnessSum;
}

const Speciment& rouletteWheelSelection(const std::vector<Speciment>& population, double fitnessSum)
{
    double target = getRandomDouble(0, fitnessSum);
    double cumulative = 0.0;

    for (const auto& speciment : population)
    {
        cumulative += speciment.weight();
        if (cumulative >= target)
            return speciment;
    }

    return population.back();
}

const Speciment& tournamentSelection(const std::vector<Speciment>& population, size_t tournamentSize)
{
    const Speciment* winner = &population[getRandomInt(0, population.size() - 1)];
    for (int i = 0; i < tournamentSize; i++)
    {
        const Speciment& contender = population[getRandomInt(0, population.size() - 1)];
        if (contender < *winner)
            winner = &contender;
    }

    return *winner;
}

void prepareRankSelection(std::vector<Speciment>& sortedPopulation)
{
    std::reverse(sortedPopulation.rbegin(), sortedPopulation.rend());
}

const Speciment& rankSelection(const std::vector<Speciment>& sortedPopulationDescendingFitness)
{
    size_t size = sortedPopulationDescendingFitness.size();
    static std::vector<double> probs(size);
    static bool isInitialized = false;
    static std::vector<double> cumulative(size);

    if (!isInitialized)
    {
        double multiplier = 2 / (size * (size - 1.0));
        for (size_t rank = 1; rank <= size; rank++)
        {
            double prob = multiplier * rank;
            probs[rank - 1] = prob;
        }

        cumulative[0] = probs[0];
        for (size_t i = 1; i < size; i++)
            cumulative[i] = cumulative[i - 1] + probs[i];
    }

    double target = getRandomDouble(0, 1);
    int chosenRank = 0;
    while (chosenRank < size - 1 && target > cumulative[chosenRank])
        chosenRank++;

    return sortedPopulationDescendingFitness[chosenRank];
}

const Speciment& truncationSelection(const std::vector<Speciment>& sortedPopulation, size_t topK)
{
    return sortedPopulation[getRandomInt(0, topK - 1)];
}

/*
    Crossover strategies:
*/

Speciment twoPointCrossover(const Speciment& firstParent, const Speciment& secondParent)
{
    size_t size = firstParent.route.size();
    std::vector<int> childRoute(size, -1);
    std::vector<bool> isUsed(size, 0);

    size_t beg = getRandomInt(0, size - 1);
    size_t end = getRandomInt(0, size - 1);
    if (beg > end)
        std::swap(beg, end);

    for (size_t i = beg; i <= end; i++)
    {
        childRoute[i] = firstParent.route[i];
        isUsed[firstParent.route[i]] = true;
    }

    size_t index = 0;
    for (size_t i = 0; i < size; i++)
    {
        if (childRoute[i] == -1)
        {
            while (isUsed[secondParent.route[index]])
            {
                index++;
            }

            childRoute[i] = secondParent.route[index];
            isUsed[secondParent.route[index]] = true;
            index++;
        }
    }

    return Speciment(std::move(childRoute));
}

Speciment partiallyMappedCrossover(const Speciment& firstParent, const Speciment& secondParent)
{
    size_t size = firstParent.route.size();
    std::vector<int> childRoute(size, -1);

    size_t beg = getRandomInt(0, size - 1);
    size_t end = getRandomInt(0, size - 1);
    if (beg > end)
        std::swap(beg, end);

    std::vector<bool> isUsed(size, false);

    for (size_t i = beg; i <= end; i++)
    {
        childRoute[i] = firstParent.route[i];
        isUsed[firstParent.route[i]] = true;
    }

    std::vector<int> secondParentPositions(size);
    for (int i = 0; i < size; i++)
    {
        secondParentPositions[secondParent.route[i]] = i;
    }

    for (size_t i = beg; i <= end; i++)
    {
        int currentGene = secondParent.route[i];
        if (isUsed[currentGene])
            continue;

        size_t pos = i;
        while (pos >= beg && pos <= end)
        {
            int conflictGene = firstParent.route[pos];
            pos = secondParentPositions[conflictGene];
        }

        childRoute[pos] = currentGene;
        isUsed[currentGene] = true;
    }

    for (int i = 0; i < size; ++i)
    {
        if (childRoute[i] == -1)
            childRoute[i] = secondParent.route[i];
    }

    return Speciment(std::move(childRoute));
}

void buildAdjacency(const std::vector<int>& route, std::vector<std::vector<int>>& adj)
{
    int size = route.size();

    for (int i = 0; i < size; ++i)
    {
        int u = route[i];
        int v = route[(i + 1) % size];
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
}

Speciment alternatingEdgesCrossover(const Speciment& firstParent, const Speciment& secondParent)
{
    size_t size = firstParent.route.size();

    std::vector<std::vector<int>> firstAdj, secondAdj;
    firstAdj.assign(size, {});
    buildAdjacency(firstParent.route, firstAdj);
    secondAdj.assign(size, {});
    buildAdjacency(secondParent.route, secondAdj);

    std::vector<int> childRoute;
    childRoute.reserve(size);
    std::vector<bool> isUsed(size, false);

    int curr = getRandomInt(0, size - 1);

    childRoute.push_back(curr);
    isUsed[curr] = 1;

    bool useFirst = true;
    while (childRoute.size() < size)
    {
        int next = -1;

        const auto& adjPrimary = useFirst ? firstAdj[curr] : secondAdj[curr];
        const auto& adjSecondary = useFirst ? secondAdj[curr] : firstAdj[curr];

        for (int neighbour : adjPrimary)
        {
            if (!isUsed[neighbour])
            {
                next = neighbour;
                break;
            }
        }

        if (next == -1)
        {
            for (int neighbour : adjSecondary)
            {
                if (!isUsed[neighbour])
                {
                    next = neighbour;
                    break;
                }
            }
        }

        if (next == -1)
        {
            for (size_t fallback = 0; fallback < size; fallback++)
            {
                if (!isUsed[fallback]) {
                    next = fallback;
                    break;
                }
            }
        }

        curr = next;
        childRoute.push_back(curr);
        isUsed[curr] = true;
        useFirst = !useFirst;
    }

    return Speciment(std::move(childRoute));
}

Speciment edgeRecombinationCrossover(const Speciment& firstParent, const Speciment& secondParent)
{
    size_t size = firstParent.route.size();

    std::vector<std::vector<int>> edges;
    edges.assign(size, {});
    buildAdjacency(firstParent.route, edges);
    buildAdjacency(secondParent.route, edges);

    std::vector<int> childRoute;
    childRoute.reserve(size);
    std::vector<bool> isUsed(size, false);

    int current = getRandomInt(0, size - 1);
    int next = -1;
    for (int step = 0; step < size; step++)
    {
        childRoute.push_back(current);
        isUsed[current] = 1;

        if (step == size - 1)
        {
            break;
        }

        for (auto& neighbour : edges[current])
        {
            auto& adj = edges[neighbour];
            adj.erase(std::remove(adj.begin(), adj.end(), current), adj.end());
        }

        auto& currAdj = edges[current];
        if (!currAdj.empty())
        {
            int bestSize = INT_MAX;
            std::vector<int> bestCandidates;

            for (int neighbour : currAdj)
            {
                size_t s = edges[neighbour].size();
                if (s < bestSize)
                {
                    bestSize = s;
                    bestCandidates.clear();
                    bestCandidates.push_back(neighbour);
                }
                else if (s == bestSize)
                {
                    bestCandidates.push_back(neighbour);
                }
            }

            next = bestCandidates[getRandomInt(0, bestCandidates.size() - 1)];
        }
        else
        {
            int bestSize = INT_MAX;
            std::vector<int> bestCandidates;

            for (int city = 0; city < size; city++)
            {
                if (!isUsed[city])
                {
                    size_t s = edges[city].size();
                    if (s < bestSize)
                    {
                        bestSize = s;
                        bestCandidates.clear();
                        bestCandidates.push_back(city);
                    }
                    else if (s == bestSize)
                    {
                        bestCandidates.push_back(city);
                    }
                }
            }

            next = bestCandidates[getRandomInt(0, bestCandidates.size() - 1)];
        }

        current = next;
    }

    return Speciment(std::move(childRoute));
}

/*
    Mutation strategies:
*/

void swapMutation(Speciment& speciment)
{
    size_t size = speciment.route.size();
    int i = getRandomInt(0, size - 1);
    int j = getRandomInt(0, size - 1);

    while (i == j)
        j = getRandomInt(0, size - 1);

    std::swap(speciment.route[i], speciment.route[j]);
}

void inversionMutation(Speciment& speciment)
{
    size_t size = speciment.route.size();
    int start = getRandomInt(0, size - 1);
    int end = getRandomInt(0, size - 1);

    if (start > end)
        std::swap(start, end);

    std::reverse(speciment.route.begin() + start, speciment.route.begin() + end + 1);
}

void insertionMutation(Speciment& speciment)
{
    size_t size = speciment.route.size();
    int from = getRandomInt(0, size - 1);
    int to = getRandomInt(0, size - 1);

    while (from == to)
        to = getRandomInt(0, size - 1);

    int city = speciment.route[from];
    speciment.route.erase(speciment.route.begin() + from);

    if (to > from)
        to--;

    speciment.route.insert(speciment.route.begin() + to, city);
}

void displacementMutation(Speciment& speciment)
{
    size_t size = speciment.route.size();
    int start = getRandomInt(0, size - 1);
    int end = getRandomInt(0, size - 1);

    if (start > end)
        std::swap(start, end);

    if (start == end)
        return;

    std::vector<int> segment(speciment.route.begin() + start, speciment.route.begin() + end + 1);
    speciment.route.erase(speciment.route.begin() + start, speciment.route.begin() + end + 1);

    int pos = getRandomInt(0, speciment.route.size());

    speciment.route.insert(speciment.route.begin() + pos, segment.begin(), segment.end());
}

void shuffleMutation(Speciment& speciment)
{
    size_t size = speciment.route.size();
    int start = getRandomInt(0, size - 1);
    int end = getRandomInt(0, size - 1);

    if (start > end)
        std::swap(start, end);

    if (start + 1 >= end)
        return;

    std::shuffle(speciment.route.begin() + start, speciment.route.begin() + end + 1, randomGenerator);
}

void twoOptMutation(Speciment& speciment, const std::vector<std::vector<double>>& matrix)
{
    size_t size = speciment.route.size();

    int i, j;
    while (true)
    {
        i = getRandomInt(0, size - 1);
        j = getRandomInt(0, size - 1);

        if (i == j)
            continue;
        if (i > j)
            std::swap(i, j);
        if (j == i + 1)
            continue;
        if (i == 0 && j == size - 1)
            continue;

        break;
    }

    int iNext = (i + 1) % size;
    int jNext = (j + 1) % size;
    int a = speciment.route[i];
    int b = speciment.route[iNext];
    int c = speciment.route[j];
    int d = speciment.route[jNext];

    double oldLen = matrix[a][b] + matrix[c][d];
    double newLen = matrix[a][c] + matrix[b][d];

    if (newLen < oldLen)
        std::reverse(speciment.route.begin() + iNext, speciment.route.begin() + j + 1);
}

void orOptMutation(Speciment& speciment)
{
    int size = speciment.route.size();
    int length = getRandomInt(0, std::min(3, size - 1));
    int start = getRandomInt(0, size - length);
    int end = start + length;

    std::vector<int> segment(speciment.route.begin() + start, speciment.route.begin() + end);
    speciment.route.erase(speciment.route.begin() + start, speciment.route.begin() + end);

    int pos = getRandomInt(0, speciment.route.size());

    speciment.route.insert(speciment.route.begin() + pos, segment.begin(), segment.end());
}

void combinedMutation(Speciment& speciment, const std::vector<std::vector<double>>& matrix)
{
    int mutationType = getRandomInt(0, 100);
    if (mutationType <= 25)
    {
        inversionMutation(speciment);
    }
    else if (mutationType <= 50)
    {
        insertionMutation(speciment);
    }
    else if (mutationType <= 60)
    {
        twoOptMutation(speciment, matrix);
    }
    else if (mutationType <= 65)
    {
        orOptMutation(speciment);
    }
    else if (mutationType <= 80)
    {
        swapMutation(speciment);
    }
    else if (mutationType <= 90)
    {
        displacementMutation(speciment);
    }
    else
    {
        shuffleMutation(speciment);
    }
}

/*
    Genetic algorithm
*/

Speciment produceChild(
    const Speciment& firstParent,
    const Speciment& secondParent,
    const std::vector<std::vector<double>>& matrix,
    Speciment(*crossoverStrategy) (const Speciment&, const Speciment&)
)
{
    Speciment child = crossoverStrategy(firstParent, secondParent);
    if (getRandomInt(0, 100) < MUTATION_PERCENT)
        combinedMutation(child, matrix);
    child.fitnessScore = child.calculateFitness(matrix);

    return child;
}

Speciment geneticAlgorithm(std::vector<Speciment>& population, std::vector<double>& fitnessProgression, const std::vector<std::vector<double>>& matrix)
{
    size_t populationSize = population.size();
    std::vector<Speciment> nextGeneration;

    for (size_t currGen = 0; currGen < GENERATIONS; currGen++)
    {
        std::sort(population.begin(), population.end());
        if (!(currGen % 10))
            fitnessProgression.push_back(population.front().fitnessScore);

        nextGeneration.clear();
        nextGeneration.insert(nextGeneration.begin(), population.begin(), population.begin() + ELITISM_COUNT);

        for (size_t i = 0; i < ELITISM_COUNT && nextGeneration.size() < populationSize; i++)
        {
            const Speciment& firstParent = truncationSelection(population, 20);
            const Speciment* secondParent = &truncationSelection(population, 20);
            while (secondParent == &firstParent)
                secondParent = &truncationSelection(population, 20);

            nextGeneration.push_back(produceChild(firstParent, *secondParent, matrix, edgeRecombinationCrossover));
            nextGeneration.push_back(produceChild(*secondParent, firstParent, matrix, edgeRecombinationCrossover));
        }

        while (nextGeneration.size() < populationSize)
        {
            const Speciment& firstParent = tournamentSelection(population, 5);
            const Speciment* secondParent = &tournamentSelection(population, 5);
            while (secondParent == &firstParent)
                secondParent = &tournamentSelection(population, 5);

            nextGeneration.push_back(produceChild(firstParent, *secondParent, matrix, twoPointCrossover));
            nextGeneration.push_back(produceChild(*secondParent, firstParent, matrix, twoPointCrossover));
        }

        if (nextGeneration.size() > populationSize)
        {
            nextGeneration.pop_back();
        }

        population = std::move(nextGeneration);
    }

    Speciment result = *min_element(population.begin(), population.end());
    fitnessProgression.push_back(result.fitnessScore);
    return result;
}


void printOutput(const std::vector<double>& fitnessProgression, const std::vector<std::string>& cityNames, const Speciment& result)
{
    std::cout << std::setprecision(10);
    for (size_t i = 0; i < fitnessProgression.size(); i++)
        std::cout << fitnessProgression[i] << std::endl;

    std::cout << std::endl;

    if (!cityNames.empty())
    {
        for (size_t i = 0; i < result.route.size(); i++)
        {
            std::cout << cityNames[result.route[i]];
            if (i != result.route.size())
            {
                std::cout << " -> ";
            }
        }
        std::cout << std::endl;
    }

    std::cout << result.fitnessScore << std::endl;
}

int main()
{
    std::string datasetName;
    std::cin >> datasetName;

    size_t citiesCount;
    std::vector<std::string> cityNames;
    std::vector<std::pair<double, double>> cityCoords;

    if (isInteger(datasetName))
    {
        generateRandomDataset(datasetName, citiesCount, cityNames, cityCoords);
    }
    else
    {
        readDataset(citiesCount, cityNames, cityCoords);
    }

    std::vector<std::vector<double>> distanceMatrix(citiesCount, std::vector<double>(citiesCount, 0));
    for (size_t i = 0; i < citiesCount; i++)
    {
        for (size_t j = 0; j < citiesCount; j++)
        {
            distanceMatrix[i][j] = distanceMatrix[j][i] = eucledianDistance(cityCoords[i], cityCoords[j]);
        }
    }

    std::vector<Speciment> population;
    for (size_t i = 0; i < POPULATION_SIZE; i++)
    {
        population.emplace_back(distanceMatrix, getRandomRoute(citiesCount));
    }

    std::vector<double> fitnessProgression;

    auto start = std::chrono::high_resolution_clock::now();
    Speciment resultSpeciment = geneticAlgorithm(population, fitnessProgression, distanceMatrix);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "# TIMES_MS: alg=" << duration.count() << std::endl;

    printOutput(fitnessProgression, cityNames, resultSpeciment);
}
