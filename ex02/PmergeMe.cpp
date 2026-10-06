#include "PmergeMe.hpp"
#include <algorithm>
#include <ctime>    // For std::clock_t and std::clock
#include <iostream> // For std::cout and std::endl

static const std::size_t NO_PAIR = static_cast<std::size_t>(-1);

PmergeMe::PmergeMe() {}

PmergeMe::PmergeMe(const PmergeMe &other) { (void)other; }

PmergeMe::~PmergeMe() {}

PmergeMe &PmergeMe::operator=(const PmergeMe &other) {
  (void)other;
  return *this;
}

std::vector<std::size_t> PmergeMe::makeIndexOrder(std::size_t size) const {
  /*
   * We use Jacobsthal sequence
   * J₀ = 0 and J₁ = 1
   * Jn = Jn-1 + 2 * jn-2
   * 0, 1, 1, 3, 5, 11, 21, 43, 85, 171, 341
   * 1, 3, 5, 11, 21, 43, 85, 171, 341
   * Example:
   * smalls = b2, b3, b4, b5
   * ↓
   * result = 3, 2, 5, 4
   * ↓
   * mean   = b3, b2, b5, b4
   * ↓
   * indexOrder = [1, 0, 3, 2]
   */
  std::vector<std::size_t> indexOrder;
  std::size_t previous = 1;
  std::size_t current = 3;

  while (indexOrder.size() < size) {
    std::size_t value = current;
    // Example: size=7, size+1= 8, 8-2=6, 6 is max index.
    if (value > size + 1)
      value = size + 1;

    // 0: previous = 1, value = 3 → [3, 2]
    // 1: previous = 3, value = 5 → [3, 2, 5, 4]
    while (value > previous && indexOrder.size() < size) {
      // We have to - 2 to change the index order because 2 is the minimum value
      // [3, 2, 5, 4] → [1, 0, 3, 2]
      indexOrder.push_back(value - 2);
      --value;
    }

    // Jn = Jn-1 + 2 * jn-2
    std::size_t next = current + 2 * previous;
    previous = current;
    current = next;
  }

  return indexOrder;
}

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

void PmergeMe::insertSmall(std::vector<int> &larges,
                           std::vector<std::size_t> &largePairIds,
                           const std::vector<int> &smalls) {
  std::vector<std::size_t> indexOrder;
  indexOrder = makeIndexOrder(smalls.size());

  for (std::size_t i = 0; i < indexOrder.size(); ++i) {
    std::size_t smallsIndex = indexOrder[i];

    if (smallsIndex >= smalls.size())
      continue;

    int value = smalls[smallsIndex];
    std::size_t pairId = smallsIndex + 1;
    std::size_t partnerIndex = 0;
    while (partnerIndex < largePairIds.size() &&
           largePairIds[partnerIndex] != pairId)
      ++partnerIndex;
    if (partnerIndex == largePairIds.size())
      continue;

    std::vector<int>::iterator limit;
    limit =
        std::lower_bound(larges.begin(), larges.begin() + partnerIndex, value);

    std::size_t insertIndex = static_cast<std::size_t>(limit - larges.begin());
    larges.insert(limit, value);
    largePairIds.insert(largePairIds.begin() + insertIndex, NO_PAIR);
  }
}

void PmergeMe::sortVector(std::vector<int> &values) {
  if (values.size() <= 1)
    return;

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

  std::vector<int> larges;
  std::vector<std::size_t> largePairIds;
  std::vector<int> smalls;
  larges.push_back(sortedPairs[0].small);
  largePairIds.push_back(NO_PAIR);
  larges.push_back(sortedPairs[0].large);
  largePairIds.push_back(0);
  for (std::size_t i = 1; i < sortedPairs.size(); i++) {
    larges.push_back(sortedPairs[i].large);
    largePairIds.push_back(i);
    smalls.push_back(sortedPairs[i].small);
  }

  insertSmall(larges, largePairIds, smalls);

  if (hasOddValue) {
    std::vector<int>::iterator position;
    position = std::lower_bound(larges.begin(), larges.end(), oddValue);
    larges.insert(position, oddValue);
  }

  values = larges;
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
  // sortDeque(dequeValues);
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
