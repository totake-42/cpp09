#include "PmergeMe.hpp"
#include <ctime>    // For std::clock_t and std::clock
#include <iostream> // For std::cout and std::endl

PmergeMe::PmergeMe() {}

PmergeMe::PmergeMe(const PmergeMe &other) { (void)other; }

PmergeMe::~PmergeMe() {}

PmergeMe &PmergeMe::operator=(const PmergeMe &other) {
  (void)other;
  return *this;
}

void PmergeMe::sortVector(std::vector<int> &values) {
  if (values.size() <= 1)
    return;

  struct Pair {
    int small;
    int large;
  };

  std::vector<Pair> pairs;
  bool hasOddValue = (values.size() % 2 != 0);
  int oddValue = 0;

  std::size_t i = 0;
  while (i + 1 < values.size()) {
    Pair pair;

    if (values[i] < values[i + 1]) {
      pair.small = values[i];
      pair.large = values[i + 1];
    } else {
      pair.small = values[i + 1];
      pair.large = values[i];
    }

    pairs.push_back(pair);

    i += 2;
  }

  if (hasOddValue)
    oddValue = values[values.size() - 1];

  std::vector<int> largeValues;

  i = 0;
  while (i < pairs.size()) {
    largeValues.push_back(pairs[i].large);
    i++;
  }

  sortVector(largeValues);

  std::vector<bool> used(pairs.size(), false);
  std::vector<Pair> sortedPairs;

  i = 0;
  while (i < largeValues.size()) {
    std::size_t j = 0;
    while (j < pairs.size()) {
      if (!used[j] && pairs[j].large == largeValues[i]) {
        sortedPairs.push_back(pairs[j]);
        used[j] = true;
        break;
      }
      j++;
    }
    i++;
  }

  std::vector<int> chain;
  std::vector<int> pending;

  chain.push_back(sortedPairs[0].small);
  chain.push_back(sortedPairs[0].large);

  i = 1;
  while (i < sortedPairs.size()) {
    chain.push_back(sortedPairs[i].large);
    pending.push_back(sortedPairs[i].small);
    i++;
  }

  insertPending(chain, pending);

  if (hasOddValue) {
    std::vector<int>::iterator position;
    position = std::lower_bound(chain.begin(), chain.end(), oddValue);
    chain.insert(position, oddValue);
  }

  values = chain;
}

void PmergeMe::run(const std::vector<int> &input) {
  std::vector<int> vectorValues(input.begin(), input.end());
  std::deque<int> dequeValues(input.begin(), input.end());

  std::clock_t start;
  std::clock_t end;
  double vectorTime;
  double dequeTime;

  start = std::clock();
  sortVector(vectorValues);
  end = std::clock();
  vectorTime = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000000.0;

  start = std::clock();
  sortDeque(dequeValues);
  end = std::clock();
  dequeTime = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000000.0;

  std::cout << "After: ";
  std::size_t i = 0;
  while (i < vectorValues.size()) {
    std::cout << " " << vectorValues[i];
    i++;
  }
  std::cout << std::endl;

  std::cout << "Time to process a range of " << input.size()
            << " elements with std::vector: " << vectorTime << " us"
            << std::endl;
  std::cout << "Time to process a range of " << input.size()
            << " elements with std::deque: " << dequeTime << " us" << std::endl;
}