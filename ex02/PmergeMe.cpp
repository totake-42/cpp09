#include "PmergeMe.hpp"
#include <algorithm>
#include <ctime>    // For std::clock_t and std::clock
#include <iostream> // For std::cout and std::endl

static const std::size_t NO_PAIR = static_cast<std::size_t>(-1);

PmergeMe::PmergeMe() {}

PmergeMe::PmergeMe(const PmergeMe &other) { (void)other; }

PmergeMe::~PmergeMe() {}

PmergeMe &PmergeMe::operator=(const PmergeMe &other)
{
  (void)other;
  return *this;
}

std::vector<std::size_t> PmergeMe::makeIndexOrder(std::size_t size) const
{
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

  while (indexOrder.size() < size)
  {
    std::size_t value = current;
    // Do not generate an index larger than size - 1.
    if (value > size + 1)
      value = size + 1;

    // Add the current group in reverse order.
    while (value > previous && indexOrder.size() < size)
    {
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

struct Pair
{
  int small;
  int large;

  Pair(int a, int b)
  {
    if (a < b)
    {
      small = a;
      large = b;
    }
    else
    {
      small = b;
      large = a;
    }
  }
};

std::size_t PmergeMe::findPartner(
    const std::vector<std::size_t> &pairIds,
    std::size_t pairId) const
{
  std::size_t i = 0;

  while (i < pairIds.size())
  {
    if (pairIds[i] == pairId)
      return i;
    ++i;
  }

  return NO_PAIR;
}

void PmergeMe::insertSmall(std::vector<int> &larges,
                           std::vector<std::size_t> &pairIds,
                           const std::vector<int> &smalls)
{
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

  for (std::size_t i = 0; i < indexOrder.size(); i++)
  {
    std::size_t smallsIndex = indexOrder[i];
    int value = smalls[smallsIndex];
    /*
     * smalls[i] and the large value from the same pair
     * have the same pair ID.
     *
     * Find where that partner large value is currently located.
     */
    std::size_t pairId = smallsIndex;
    std::size_t partnerIndex = findPartner(pairIds, pairId);

    /*
     * Because small < large within each pair,
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
     * larges and pairIds must always have the same size
     * and correspond to each other.
     */
    std::size_t insertIndex = static_cast<std::size_t>(limit - larges.begin());

    // Insert the small value into larges.
    larges.insert(limit, value);

    /*
     * The inserted value is a small value, not a large value,
     * so it does not have a large-value pair ID.
     */
    pairIds.insert(pairIds.begin() + insertIndex, NO_PAIR);
  }
}

void PmergeMe::sortVector(std::vector<int> &values)
{
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
  for (std::size_t i = 0; i + 1 < values.size(); i += 2)
  {
    Pair pair(values[i], values[i + 1]);
    pairs.push_back(pair);
  }

  /*
   * Extract only the large values from each pair.
   *
   * Example:
   * pairs = [(3, 6), (1, 5), (2, 4)]
   *
   * largeValues = [6, 5, 4]
   */
  std::vector<int> largeValues;
  for (std::size_t i = 0; i < pairs.size(); i++)
  {
    largeValues.push_back(pairs[i].large);
  }

  /*
   * Recursively sort the large values.
   *
   * After this:
   * largeValues is sorted.
   */
  sortVector(largeValues);

  /*
   * Rebuild the original pairs in the same order
   * as the sorted large values.
   *
   * We use 'used' because large values can be duplicated.
   *
   * Example:
   * pairs       = [(3, 6), (1, 5), (2, 6)]
   * largeValues = [6, 6, 5]
   *
   * The two pairs containing 6 must be matched
   * with different elements.
   */
  std::vector<bool> used(pairs.size(),
                         false); // Because of duplicate values included
  std::vector<Pair> sortedPairs;
  for (std::size_t i = 0; i < largeValues.size(); i++)
  {
    for (std::size_t j = 0; j < pairs.size(); j++)
    {
      if (pairs[j].large == largeValues[i] && !used[j])
      {
        sortedPairs.push_back(pairs[j]);
        used[j] = true;
        break;
      }
    }
  }

  /*
   * Build three parallel vectors from sortedPairs:
   *
   * larges:
   *   sorted large values
   *
   * pairIds:
   *   ID of the pair that each large value belongs to
   *
   * smalls:
   *   small value belonging to each pair
   *
   * Example:
   * sortedPairs = [(3, 4), (1, 5), (2, 6)]
   *
   * larges  = [4, 5, 6]
   * pairIds = [0, 1, 2]
   * smalls  = [3, 1, 2]
   *
   * So:
   * pair 0 -> small 3 / large 4
   * pair 1 -> small 1 / large 5
   * pair 2 -> small 2 / large 6
   */
  std::vector<int> larges;
  std::vector<std::size_t> pairIds;
  std::vector<int> smalls;
  for (std::size_t i = 0; i < sortedPairs.size(); i++)
  {
    larges.push_back(sortedPairs[i].large);
    pairIds.push_back(i);
    smalls.push_back(sortedPairs[i].small);
  }

  /*
   * Insert all small values using the Jacobsthal order.
   *
   * Each small value is inserted only before
   * the large value from the same pair.
   */
  insertSmall(larges, pairIds, smalls);

  /*
   * If there was an odd value, insert it normally.
   * There is no pair partner, so it can be inserted
   * anywhere in the whole sorted range.
   */
  if (hasOddValue)
  {
    std::vector<int>::iterator position;
    position = std::lower_bound(larges.begin(), larges.end(), oddValue);
    larges.insert(position, oddValue);
  }

  /*
   * larges now contains the completely sorted result.
   */
  values = larges;
}

void PmergeMe::run(const std::vector<int> &input)
{
  std::vector<int> vectorValues(input.begin(), input.end());
  std::deque<int> dequeValues(input.begin(), input.end());

  std::clock_t start;
  std::clock_t end;
  double vectorTime;
  double dequeTime;

  start = std::clock();

  /*
   * The sorting process for the vector is as follows:
   *
   * 1. Form pairs from the input values.
   * 2. Extract the large values from each pair.
   * 3. Recursively sort the large values.
   * 4. Rebuild the sorted pairs.
   * 5. Create three parallel vectors: larges, pairIds, and smalls.
   * 6. Insert all small values using the Jacobsthal order.
   * 7. If there is an odd value, insert it normally.
   */
  sortVector(vectorValues);

  end = std::clock();
  vectorTime = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000000.0;

  start = std::clock();
  // sortDeque(dequeValues);
  end = std::clock();
  dequeTime = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000000.0;

  std::cout << "After: ";
  std::size_t i = 0;
  while (i < vectorValues.size())
  {
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
