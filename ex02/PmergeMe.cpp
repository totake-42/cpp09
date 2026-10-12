#include "PmergeMe.hpp"
#include <algorithm>
#include <cstdio>   // For std::perror
#include <iomanip>  // For std::setprecision
#include <iostream> // For std::cout and std::endl
#include <time.h>   // For clock_gettime and CLOCK_MONOTONIC

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
   * Create the insertion order of small values using the Jacobsthal sequence.
   *
   * smalls starts from b1:
   *
   * smalls = [b1, b2, b3, b4, b5, ...]
   *
   * The Jacobsthal sequence is:
   * J0 = 0, J1 = 1
   * Jn = Jn-1 + 2 * Jn-2
   *
   * 0, 1, 1, 3, 5, 11, 21, 43, ...
   *
   * We use the sequence starting from 3.
   * Each group is processed backwards.
   *
   * Example:
   *
   * smalls = [b1, b2, b3, b4, b5]
   *
   * b1 is inserted first.
   *
   * The remaining values are inserted in Jacobsthal order:
   * b3, b2, b5, b4
   *
   * Convert them to zero-based indexes:
   *
   * b1 -> index 0
   * b2 -> index 1
   * b3 -> index 2
   * b4 -> index 3
   * b5 -> index 4
   *
   * Therefore:
   * b3, b2, b5, b4
   *   ↓
   * 2, 1, 4, 3
   *
   * Finally, b1 (index 0) is added at the beginning:
   *
   * [2, 1, 4, 3]
   *      ↓
   * [0, 2, 1, 4, 3]
   */
  std::vector<std::size_t> indexOrder;
  std::size_t previous = 1;
  std::size_t current = 3;

  while (indexOrder.size() < size) {
    std::size_t value = current;
    // Do not generate an index larger than size - 1.
    if (value > size + 1)
      value = size + 1;

    // Add the current group in reverse order.
    while (value > previous && indexOrder.size() < size) {
      indexOrder.push_back(value - 1);
      --value;
    }

    // Generate the next Jacobsthal number.
    std::size_t next = current + 2 * previous;
    previous = current;
    current = next;
  }

  // b1 (smalls[0]) is inserted first.
  indexOrder.insert(indexOrder.begin(), 0);
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

std::size_t PmergeMe::findPartner(const std::vector<std::size_t> &largePairIds,
                                  std::size_t pairId) const {
  std::size_t i = 0;

  while (i < largePairIds.size()) {
    if (largePairIds[i] == pairId)
      return i;
    ++i;
  }

  return NO_PAIR;
}

void PmergeMe::insertSmall(std::vector<int> &larges,
                           std::vector<std::size_t> &largePairIds,
                           const std::vector<int> &smalls) {
  if (smalls.empty())
    return;

  /*
   * Insert small values in the Jacobsthal order.
   *
   * smalls = [b1, b2, b3, b4, ...]
   *
   * makeIndexOrder() returns the order in which smalls should be inserted:
   *
   * [0, 2, 1, 4, 3, ...]
   *  ↓  ↓  ↓  ↓  ↓
   * [b1, b3, b2, b5, b4, ...]
   */

  std::vector<std::size_t> indexOrder;
  /*
   * makeIndexOrder() receives the number of small values
   * excluding b1, because b1 is always inserted first.
   */
  indexOrder = makeIndexOrder(smalls.size() - 1);

  for (std::size_t i = 0; i < indexOrder.size(); i++) {
    std::size_t smallsIndex = indexOrder[i];
    int value = smalls[smallsIndex];
    /*
     * smalls[i] and the large value from the same pair
     * have the same pair ID.
     *
     * Find where that partner large value is currently located.
     */
    std::size_t pairId = smallsIndex;
    std::size_t partnerIndex = findPartner(largePairIds, pairId);

    /*
     * Because small <= large within each pair,
     * value must be inserted before its partner large value.
     *
     * Therefore, we only search the range before partnerIndex.
     */
    std::vector<int>::iterator limit;
    limit =
        std::lower_bound(larges.begin(), larges.begin() + partnerIndex, value);

    /*
     * Save the position where the small value will be inserted.
     *
     * larges and largePairIds must always have the same size
     * and correspond to each other.
     */
    std::size_t insertIndex = static_cast<std::size_t>(limit - larges.begin());

    // Insert the small value into larges.
    larges.insert(limit, value);

    /*
     * The inserted value is a small value, not a large value,
     * so it does not have a large-value pair ID.
     */
    largePairIds.insert(largePairIds.begin() + insertIndex, NO_PAIR);
  }
}

void PmergeMe::sortVector(std::vector<int> &values) {
  if (values.size() <= 1)
    return;

  /*
   * If the number of values is odd,
   * keep the last value aside and insert it at the end.
   */
  bool hasOddValue = (values.size() % 2 != 0);
  int oddValue = 0;
  if (hasOddValue)
    oddValue = values[values.size() - 1];

  /*
   * Make pairs from the input.
   *
   * Each Pair stores:
   *   small = smaller value
   *   large = larger value
   *
   * Example:
   * [6, 3] -> Pair(3, 6)
   * [1, 5] -> Pair(1, 5)
   */
  std::vector<Pair> pairs;
  for (std::size_t i = 0; i + 1 < values.size(); i += 2) {
    Pair pair(values[i], values[i + 1]);
    pairs.push_back(pair);
  }

  /*
   * Extract only the large values from each pair.
   *
   * Example:
   * pairs = [(3, 6), (1, 5), (2, 4)]
   *
   * larges = [6, 5, 4]
   */
  std::vector<int> larges;
  for (std::size_t i = 0; i < pairs.size(); i++) {
    larges.push_back(pairs[i].large);
  }

  /*
   * Recursively sort the large values.
   *
   * After this:
   * larges is sorted.
   */
  sortVector(larges);

  /*
   * Build pair IDs and small values in the same order
   * as the sorted large values.
   *
   * Equal large values may belong to different pairs.
   * Mark each matched pair as used so every sorted large value keeps
   * exactly one corresponding small value.
   */
  std::vector<std::size_t> largePairIds;
  std::vector<int> smalls;
  std::vector<bool> usedPairs(pairs.size(), false);
  for (std::size_t i = 0; i < larges.size(); i++) {
    for (std::size_t j = 0; j < pairs.size(); j++) {
      if (!usedPairs[j] && pairs[j].large == larges[i]) {
        largePairIds.push_back(i);
        smalls.push_back(pairs[j].small);
        usedPairs[j] = true;
        break;
      }
    }
  }

  /*
   * Insert all small values using the Jacobsthal order.
   *
   * Each small value is inserted only before
   * the large value from the same pair.
   */
  insertSmall(larges, largePairIds, smalls);

  /*
   * If there was an odd value, insert it normally.
   * There is no pair partner, so it can be inserted
   * anywhere in the whole sorted range.
   */
  if (hasOddValue) {
    std::vector<int>::iterator position;
    position = std::lower_bound(larges.begin(), larges.end(), oddValue);
    larges.insert(position, oddValue);
  }

  /*
   * larges now contains all values in sorted order.
   */
  values = larges;
}

std::size_t PmergeMe::findPartner(const std::deque<std::size_t> &largePairIds,
                                  std::size_t pairId) const {
  std::size_t i = 0;

  while (i < largePairIds.size()) {
    if (largePairIds[i] == pairId)
      return i;
    ++i;
  }

  return NO_PAIR;
}

void PmergeMe::insertSmall(std::deque<int> &larges,
                           std::deque<std::size_t> &largePairIds,
                           const std::deque<int> &smalls) {
  if (smalls.empty())
    return;

  std::vector<std::size_t> indexOrder;
  indexOrder = makeIndexOrder(smalls.size() - 1);

  for (std::size_t i = 0; i < indexOrder.size(); i++) {
    std::size_t smallsIndex = indexOrder[i];
    int value = smalls[smallsIndex];
    std::size_t pairId = smallsIndex;
    std::size_t partnerIndex = findPartner(largePairIds, pairId);

    std::deque<int>::iterator limit;
    limit =
        std::lower_bound(larges.begin(), larges.begin() + partnerIndex, value);

    std::size_t insertIndex = static_cast<std::size_t>(limit - larges.begin());
    larges.insert(limit, value);
    largePairIds.insert(largePairIds.begin() + insertIndex, NO_PAIR);
  }
}

void PmergeMe::sortDeque(std::deque<int> &values) {
  if (values.size() <= 1)
    return;

  bool hasOddValue = (values.size() % 2 != 0);
  int oddValue = 0;
  if (hasOddValue)
    oddValue = values[values.size() - 1];

  std::deque<Pair> pairs;
  for (std::size_t i = 0; i + 1 < values.size(); i += 2) {
    Pair pair(values[i], values[i + 1]);
    pairs.push_back(pair);
  }

  std::deque<int> larges;
  for (std::size_t i = 0; i < pairs.size(); i++) {
    larges.push_back(pairs[i].large);
  }

  sortDeque(larges);

  std::deque<std::size_t> largePairIds;
  std::deque<int> smalls;
  std::vector<bool> usedPairs(pairs.size(), false);
  for (std::size_t i = 0; i < larges.size(); i++) {
    for (std::size_t j = 0; j < pairs.size(); j++) {
      if (!usedPairs[j] && pairs[j].large == larges[i]) {
        largePairIds.push_back(i);
        smalls.push_back(pairs[j].small);
        usedPairs[j] = true;
        break;
      }
    }
  }

  insertSmall(larges, largePairIds, smalls);

  if (hasOddValue) {
    std::deque<int>::iterator position;
    position = std::lower_bound(larges.begin(), larges.end(), oddValue);
    larges.insert(position, oddValue);
  }

  values = larges;
}

bool PmergeMe::run(const std::vector<int> &input) {
  std::vector<int> vectorValues(input.begin(), input.end());
  std::deque<int> dequeValues(input.begin(), input.end());

  struct timespec start;
  struct timespec end;
  double vectorTime;
  double dequeTime;

  /*
   * The sorting process for the vector is as follows:
   *
   * 1. Form pairs from the input values.
   * 2. Extract the large values from each pair.
   * 3. Recursively sort the large values.
   * 4. Build largePairIds and smalls by matching the sorted large values to
   * pairs.
   * 5. Insert all small values into larges using the Jacobsthal order.
   * 6. If there is an odd value, insert it normally.
   */

  /*
   * CLOCK_MONOTONIC measures elapsed time without wall-clock adjustments.
   * Convert the seconds and nanoseconds difference to microseconds.
   */
  if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
    std::cerr << "Error" << std::endl;
    return (false);
  }
  sortVector(vectorValues);
  if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
    std::cerr << "Error" << std::endl;
    return (false);
  }
  vectorTime = static_cast<double>(end.tv_sec - start.tv_sec) * 1000000.0 +
               static_cast<double>(end.tv_nsec - start.tv_nsec) / 1000.0;

  if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
    std::cerr << "Error" << std::endl;
    return (false);
  }
  sortDeque(dequeValues);
  if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
    std::cerr << "Error" << std::endl;
    return (false);
  }
  dequeTime = static_cast<double>(end.tv_sec - start.tv_sec) * 1000000.0 +
              static_cast<double>(end.tv_nsec - start.tv_nsec) / 1000.0;

  // Verify that the sorted vector and deque have the same values
  if (vectorValues.size() != dequeValues.size()) {
    std::cerr << "Error" << std::endl;
    return (false);
  }
  for (std::size_t i = 0; i < vectorValues.size(); i++) {
    if (i != 0 && vectorValues[i - 1] > vectorValues[i]) {
      std::cerr << "Error" << std::endl;
      return (false);
    }
    if (i != 0 && dequeValues[i - 1] > dequeValues[i]) {
      std::cerr << "Error" << std::endl;
      return (false);
    }
    if (vectorValues[i] != dequeValues[i]) {
      std::cerr << "Error" << std::endl;
      return (false);
    }
  }

  // Print the sorted vector and time information
  std::cout << "After: ";
  for (std::size_t i = 0; i < vectorValues.size(); i++)
    std::cout << " " << vectorValues[i];
  std::cout << std::endl;
  std::cout << std::fixed << std::setprecision(5);
  std::cout << "Time to process a range of " << vectorValues.size()
            << " elements with std::vector: " << vectorTime << " us"
            << std::endl;
  std::cout << "Time to process a range of " << dequeValues.size()
            << " elements with std::deque : " << dequeTime << " us"
            << std::endl;

  return (true);
}
