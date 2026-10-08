#ifndef PMERGEME_HPP
#define PMERGEME_HPP

#include <cstddef> // For std::size_t
#include <deque>
#include <vector>

class PmergeMe
{

private:
  std::vector<std::size_t> makeIndexOrder(std::size_t size) const;
  std::size_t findPartner(
      const std::vector<std::size_t> &pairIds,
      std::size_t pairId) const;
  void insertSmall(std::vector<int> &larges,
                   std::vector<std::size_t> &pairIds,
                   const std::vector<int> &smalls);
  void sortVector(std::vector<int> &values);
  void sortDeque(std::deque<int> &values);

public:
  PmergeMe();
  PmergeMe(const PmergeMe &other);
  ~PmergeMe();

  PmergeMe &operator=(const PmergeMe &other);

  void run(const std::vector<int> &input);
};

#endif
