// Ivan Makaveev, 2MI0600203

#pragma once
#include <array>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>

template <class F, size_t K, class L>
struct DataRecord
{
	std::array<F, K> features;
	L label;

	bool operator==(const DataRecord<F,K,L>& other) const
	{
		return features == other.features && label == other.label;
	}
};

template <class F, size_t K, class L>
class KDTree
{
	struct Node
	{
		std::array<F, K> point;
		L label;
		size_t axis;
		Node* left;
		Node* right;

		Node(const std::array<F, K>& point, const L& label, size_t axis) :
			point(point), label(label), axis(axis)
		{ }
	};

	Node* buildTree(std::vector<DataRecord<F, K, L>>& dataset, size_t left, size_t right, size_t depth)
	{
		if (left == right)
		{
			return nullptr;
		}

		size_t axis = depth % K;
		size_t mid = left + (right - left) / 2;
		std::nth_element(
			dataset.begin() + left,
			dataset.begin() + mid,
			dataset.begin() + right,
			[axis](const DataRecord<F, K, L>& lhs, const DataRecord<F, K, L>& rhs)
			{
				return lhs.features[axis] < rhs.features[axis];
			}
		);

		auto target = (dataset.begin() + mid);
		Node* curr = new Node(target->features, target->label, axis);
		curr->left = buildTree(dataset, left, mid, depth + 1);
		curr->right = buildTree(dataset, mid + 1, right, depth + 1);
		return curr;
	}

	void deepCopy(Node* other)
	{
		if (!other)
		{
			return nullptr;
		}

		Node* curr = new Node(other->point, other->axis);
		curr->left = deepCopy(other->left);
		curr->right = deepCopy(other->right);

		return curr;
	}

	void copyFrom(const KDTree<F, K, L>& other)
	{
		root = deepCopy(other.root);
	}

	void moveFrom(KDTree<F, K, L>&& other) noexcept
	{
		root = other.root;
		other.root = nullptr;
	}

	void free(Node* curr) noexcept
	{
		if (!curr)
		{
			return;
		}

		free(curr->left);
		free(curr->right);
		delete curr;
	}

	void kNearestRec(const Node* curr, const std::array<F, K>& query, size_t k, std::priority_queue<std::pair<double, const Node*>>& pq) const
	{
		if (!curr)
			return;

		double dist = getDistSquared(query, curr->point);

		pq.push({ dist, curr });
		if (pq.size() > k)
		{
			pq.pop();
		}

		size_t axis = curr->axis;
		double axisDiff = query[axis] - curr->point[axis];

		Node* primary = nullptr;
		Node* secondary = nullptr;
		if (axisDiff < 0)
		{
			primary = curr->left;
			secondary = curr->right;
		}
		else
		{
			primary = curr->right;
			secondary = curr->left;
		}

		kNearestRec(primary, query, k, pq);

		if (pq.size() < k || axisDiff * axisDiff < pq.top().first)
		{
			kNearestRec(secondary, query, k, pq);
		}
	}

public:
	static double getDistSquared(const std::array<F, K>& lhs, const std::array<F, K>& rhs)
	{
		double res = 0;
		for (size_t i = 0; i < K; i++)
		{
			double delta = lhs[i] - rhs[i];
			res += delta * delta;
		}

		return res;
	}

	KDTree(const std::vector<DataRecord<F, K, L>>& dataset)
	{
		auto datasetCopy = dataset;
		root = buildTree(datasetCopy, 0, datasetCopy.size(), 0);
	}

	KDTree(const KDTree<F, K, L>& other)
	{
		copyFrom(other);
	}

	KDTree(KDTree<F, K, L>&& other) noexcept
	{
		moveFrom(std::move(other));
	}

	KDTree<F, K, L>& operator=(const KDTree<F, K, L>& other)
	{
		if (this != &other)
		{
			free(root);
			copyFrom(other);
		}

		return *this;
	}

	KDTree<F, K, L>& operator=(KDTree<F, K, L>&& other) noexcept
	{
		if (this != &other)
		{
			free(root);
			moveFrom(std::move(other));
		}

		return *this;
	}

	~KDTree() noexcept
	{
		free(root);
	}

	std::vector<DataRecord<F, K, L>> getNearest(const std::array<F, K>& query, size_t nearestCount) const
	{
		if (!root)
			throw std::runtime_error("Tree is empty");

		if (nearestCount == 0)
			return { };

		std::priority_queue<std::pair<double, const Node*>> heap;

		kNearestRec(root, query, nearestCount, heap);

		std::vector<DataRecord<F, K, L>> result;
		result.reserve(heap.size());
		while (!heap.empty())
		{
			result.push_back(DataRecord<F, K, L> {heap.top().second->point, heap.top().second->label});
			heap.pop();
		}

		return result;
	}

private:
	Node* root = nullptr;
};

