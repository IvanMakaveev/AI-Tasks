// Ivan Makaveev, 2MI0600203

#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "KDTree.hpp"

#include <matplot/matplot.h>

const std::string DATA_PATH = "D:\\FMI All\\AI-Tasks\\Homeworks\\Homework 05\\Iris.csv";
const char SEPARATOR = ',';
std::random_device random;
std::mt19937 generator(random());

/*
	Data reading
*/

template <class F, size_t K, class L>
std::vector<DataRecord<F, K, L>> readData(
	const std::string& file,
	char separator = ',',
	bool skipHeaders = true,
	bool skipFirstColumn = true
)
{
	std::vector<DataRecord<F, K, L>> result;
	std::ifstream ifs(file);

	if (!ifs.is_open())
		throw std::runtime_error("File could not be opened");

	std::string line;

	if (skipHeaders)
		std::getline(ifs, line);

	while (std::getline(ifs, line))
	{
		if (line.empty())
			continue;

		std::stringstream ss(line);
		std::string token;

		DataRecord<F, K, L> record;
		if (skipFirstColumn)
			std::getline(ss, token, separator);

		for (size_t i = 0; i < K; i++)
		{
			if (!std::getline(ss, token, separator))
				throw std::runtime_error("Invalid data format - not enough features");

			record.features[i] = std::stod(token);
		}

		if (!std::getline(ss, token))
			throw std::runtime_error("Invalid data format - label missing");

		record.label = token;
		result.push_back(record);
	}

	ifs.close();

	return result;
}

/*
	Normalization
*/
template <class F, size_t K, class L>
void minMaxNormalization(std::vector<DataRecord<F, K, L>>& dataset) {
	for (size_t i = 0; i < K; i++)
	{
		F minVal = std::min_element(dataset.begin(), dataset.end(), [i](const DataRecord<F, K, L>& lhs, const DataRecord<F, K, L>& rhs) {return lhs.features[i] < rhs.features[i]; })->features[i];
		F maxVal = std::max_element(dataset.begin(), dataset.end(), [i](const DataRecord<F, K, L>& lhs, const DataRecord<F, K, L>& rhs) {return lhs.features[i] < rhs.features[i]; })->features[i];

		F denom = maxVal - minVal;

		for (auto& record : dataset)
			record.features[i] = denom ? (record.features[i] - minVal) / denom : 0;
	}
}

void evaluateDistribution(const std::vector<double>& data, double& mean, double& sd)
{
	mean = 0;
	for (size_t i = 0; i < data.size(); i++)
		mean += data[i];

	mean /= data.size();

	sd = 0;
	for (size_t i = 0; i < data.size(); i++)
		sd += (data[i] - mean) * (data[i] - mean);

	sd /= (data.size() - 1);
	sd = sqrt(sd);
}

template <size_t K, class L>
void zScoreNormalization(std::vector<DataRecord<double, K, L>>& dataset)
{
	for (size_t i = 0; i < K; i++)
	{
		std::vector<double> featureData(dataset.size());
		std::transform(dataset.begin(), dataset.end(), featureData.begin(), [i](const DataRecord<double, K, L>& record) {return record.features[i]; });

		double mean, sd;
		evaluateDistribution(featureData, mean, sd);

		for (auto& record : dataset)
			record.features[i] = (record.features[i] - mean) / sd;
	}
}

/*
	KNN Model - using KD-Tree
*/

template <class F, size_t K, class L>
class KNN
{
	KDTree<F, K, L> kdtree;

public:
	KNN(const std::vector<DataRecord<F, K, L>>& dataset) : kdtree(dataset)
	{ }

	L predict(const std::array<F, K>& query, size_t k) const
	{
		if (k == 0)
			throw std::runtime_error("Invalid k value");

		std::vector<DataRecord<F, K, L>> closest = kdtree.getNearest(query, k);
		std::unordered_map<L, size_t> count;
		size_t bestCount = 0;
		L* bestCountLabel = nullptr;

		for (size_t i = 0; i < closest.size(); i++)
		{
			count[closest[i].label]++;

			if (bestCount < count[closest[i].label])
			{
				bestCount = count[closest[i].label];
				bestCountLabel = &closest[i].label;
			}
		}

		return *bestCountLabel;
	}
};

/*
	Data separation
*/

template <class F, size_t K, class L>
void stratifiedTrainTest(
	std::vector<DataRecord<F, K, L>> dataset,
	std::vector<DataRecord<F, K, L>>& train,
	std::vector<DataRecord<F, K, L>>& test,
	size_t testPercentage
)
{
	if (testPercentage > 100)
		throw std::runtime_error("Invalid test percentage");
	
	train.clear();
	test.clear();

	std::unordered_map<L, size_t> count;
	for (const auto& el : dataset)
		count[el.label]++;

	std::shuffle(dataset.begin(), dataset.end(), generator);

	std::unordered_map<L, size_t> testCount;
	for (const auto& el : dataset)
	{
		if (testCount[el.label] < (count[el.label] * testPercentage) / 100)
		{
			test.push_back(el);
			testCount[el.label]++;
		}
		else
		{
			train.push_back(el);
		}
	}
}

template <class F, size_t K, class L, class Iter>
void separateFolds(
	const std::vector<DataRecord<F, K, L>>& train,
	std::vector<DataRecord<F, K, L>>& trainFold,
	std::vector<DataRecord<F, K, L>>& validateFold,
	Iter beg,
	Iter end
)
{
	trainFold.insert(trainFold.end(), train.begin(), beg);
	validateFold.insert(validateFold.end(), beg, end);
	trainFold.insert(trainFold.end(), end, train.end());
}

template <class F, size_t K, class L>
size_t testPredictKNN(
	const KNN<F, K, L>& model,
	const std::vector<DataRecord<F, K, L>>& test,
	size_t k
)
{
	size_t correct = 0;
	for (const auto& el : test)
	{
		auto res = model.predict(el.features, k);
		if (res == el.label)
			correct++;
	}

	return correct;
}

template <class F, size_t K, class L>
std::vector<double> performCrossValidationKNN(
	const std::vector<DataRecord<F, K, L>>& train,
	size_t folds,
	size_t k
)
{
	if (folds > train.size() || folds < 2)
	{
		throw std::runtime_error("Invalid folds count");
	}

	std::vector<double> accuracies;

	size_t foldSize = train.size() / folds;
	for (size_t i = 1; i <= folds; i++)
	{
		std::vector<DataRecord<F, K, L>> trainFold;
		std::vector<DataRecord<F, K, L>> validateFold;

		auto beg = train.begin() + foldSize * (i - 1);
		auto end = (i == folds ? train.end() : train.begin() + foldSize * i);

		separateFolds(train, trainFold, validateFold, beg, end);

		size_t correct = testPredictKNN<F, K, L>(KNN<F, K, L>(trainFold), validateFold, k);

		accuracies.push_back((correct * 100.0) / validateFold.size());
	}

	return accuracies;
}

template <class F, size_t K, class L>
void analyzeKNN(const std::vector<DataRecord<F, K, L>>& dataset, size_t k)
{
	std::vector<DataRecord<F, K, L>> train;
	std::vector<DataRecord<F, K, L>> test;
	stratifiedTrainTest<F, K, L>(dataset, train, test, 20);

	KNN model(train);
	size_t trainCorrect = testPredictKNN<F, K, L>(model, train, k);
	double trainAccuracy = (trainCorrect * 100.0) / train.size();
	std::cout << "Train set accuracy: " << trainAccuracy << "%" << std::endl << std::endl;

	std::vector<double> crossValidationAccuracies = performCrossValidationKNN<F, K, L>(train, 10, k);
	double mean, sd;
	evaluateDistribution(crossValidationAccuracies, mean, sd);
	for (size_t i = 0; i < crossValidationAccuracies.size(); i++)
		std::cout << "Accuracy fold[" << i + 1 << "]: " << crossValidationAccuracies[i] << "%" << std::endl;
	std::cout << "Average: " << mean << "%" << std::endl;
	std::cout << "SD: " << sd << std::endl << std::endl;


	size_t testCorrect = testPredictKNN<F, K, L>(model, test, k);
	double testAccuracy = (testCorrect * 100.0) / test.size();
	std::cout << "Test set accuracy: " << testAccuracy << "%" << std::endl;
}

template <class F, size_t K, class L>
void generalAnalysisKNN(const std::vector<DataRecord<F, K, L>>& dataset)
{
	std::vector<DataRecord<F, K, L>> train;
	std::vector<DataRecord<F, K, L>> test;
	stratifiedTrainTest<F, K, L>(dataset, train, test, 20);

	KNN model(train);
	std::vector<size_t> x;
	std::vector<double> testAcc;
	for (size_t k = 1; k <= 80; k++)
	{
		x.push_back(k);
		size_t testCorrect = testPredictKNN<F, K, L>(model, test, k);
		testAcc.push_back((testCorrect * 100.0) / test.size());
	}

	matplot::plot(x, testAcc, "b-o");
	matplot::title("Test Accuracy");
	matplot::xlabel("Value for K");
	matplot::ylabel("Accuracy %");
	matplot::ylim({ 50, 101 });

	matplot::show();
}

int main()
{
	size_t k = 0;
	std::cout << "Enter value for K (or 0 for general analysis):" << std::endl;
	std::cin >> k;

	auto dataset = readData<double, 4, std::string>(DATA_PATH, SEPARATOR);

	//minMaxNormalization<double, 4, std::string>(dataset);
	//zScoreNormalization<4, std::string>(dataset);

	if (k != 0)
	{
		analyzeKNN<double, 4, std::string>(dataset, k);
	}
	else
	{
		generalAnalysisKNN<double, 4, std::string>(dataset);
	}
}