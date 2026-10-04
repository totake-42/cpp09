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

std::vector<size_t> PmergeMe::makeJacobsthalOrder(std::size_t size) const {
  std::vector<size_t> order;
  size_t previous = 1;
  size_t current = 3;

  while (order.size() < size) {
    size_t value = current;

    while (value > previous && order.size() < size) {
      /*
       * pending[0] is b2.
       * b3, b2, b5, b4, b11, ...
       */
      order.push_back(value - 2);
      --value;
    }

    size_t next = current + 2 * previous;
    previous = current;
    current = next;
  }

  return order;
}

void PmergeMe::insertSmall(std::vector<int> &result,
                           const std::vector<int> &smalls) {
  std::vector<size_t> order;
  order = makeJacobsthalOrder(smalls.size());

  for (size_t i = 0; i < order.size(); ++i) {
    size_t smallsIndex = order[i];

    if (smallsIndex >= smalls.size())
      continue;

    int value = smalls[smallsIndex];

    std::vector<int>::iterator limit;
    limit = std::lower_bound(result.begin(), result.end(), value);

    result.insert(limit, value);
  }
}

void PmergeMe::sortVector(std::vector<int> &values) {
  if (values.size() <= 1)
    return;

  struct Pair {
    int small;
    int large;

    Pair(int a, int b) {
      if (a < b) {
        small = a;
        large = b;
      } else {
        small = b;
        large = a;
      }
    }
  };

  bool hasOddValue = (values.size() % 2 != 0);
  int oddValue = 0;
  if (hasOddValue)
    oddValue = values[values.size() - 1];

  // Make pairs vector
  std::vector<Pair> pairs;
  for (std::size_t i = 0; i + 1 < values.size(); i += 2) {
    Pair pair(values[i], values[i + 1]);
    pairs.push_back(pair);
  }

  // Make large vlues vector
  std::vector<Pair> pairs;
  std::vector<int> largeValues;
  for (std::size_t i = 0; i < pairs.size(); i++) {
    largeValues.push_back(pairs[i].large);
  }

  // recursive
  sortVector(largeValues);

  // Sort pairs depends on large values
  std::vector<bool> used(pairs.size(),
                         false); // Because of duplicate values included
  std::vector<Pair> sortedPairs;
  for (std::size_t i = 0; i < largeValues.size(); i++) {
    for (std::size_t j = 0; j < pairs.size(); j++) {
      if (pairs[j].large == largeValues[i] && !used[j]) {
        sortedPairs.push_back(pairs[j]);
        used[j] = true;
        break;
      }
    }
  }

  std::vector<int> result;
  std::vector<int> smalls;
  result.push_back(sortedPairs[0].small);
  result.push_back(sortedPairs[0].large);
  for (std::size_t i = 1; i < sortedPairs.size(); i++) {
    result.push_back(sortedPairs[i].large);
    smalls.push_back(sortedPairs[i].small);
    i++;
  }

  insertSmall(result, smalls);

  if (hasOddValue) {
    std::vector<int>::iterator position;
    position = std::lower_bound(result.begin(), result.end(), oddValue);
    result.insert(position, oddValue);
  }

  values = result;
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