#include "PmergeMe.hpp"
#include <cerrno>   // For errno
#include <climits>  // For INT_MAX
#include <iostream> // For std::cerr and std::endl
#include <vector>   // For std::vector

static bool parsePositiveInteger(const char *str, int &value) {
  char *end;
  long parsed;

  errno = 0;
  parsed = std::strtol(str, &end, 10);

  if (str[0] == '\0' || *end != '\0')
    return false;
  if (errno != 0 || parsed <= 0 || parsed > INT_MAX)
    return false;

  value = static_cast<int>(parsed);
  return true;
}

int main(int argc, char **argv) {
  if (argc <= 1) {
    std::cerr << "Error" << std::endl;
    return (1);
  }

  std::vector<int> input;
  std::size_t i = 1;
  while (i < static_cast<std::size_t>(argc)) {
    int value;

    if (!parsePositiveInteger(argv[i], value)) {
      std::cerr << "Error" << std::endl;
      return (1);
    }

    input.push_back(value);

    i++;
  }

  std::cout << "Before:";
  i = 0;
  while (i < input.size()) {
    std::cout << " " << input[i];
    i++;
  }
  std::cout << std::endl;

  PmergeMe sorter;
  std::vector<int> vectorValues;
  std::deque<int> dequeValues;

  sorter.run(input);

  return (0);
}
