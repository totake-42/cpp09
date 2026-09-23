#ifndef PMERGEME_HPP
#define PMERGEME_HPP

#include <deque>
#include <vector>

class PmergeMe {

private:
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
