#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

#include <cstdlib>

namespace {

std::string
fileToString(const char* path)
{
  std::ifstream file(path);
  if (!file.good())
    return std::string("#error Failed to open '") + path + "'.\n";

  std::ostringstream stream;
  stream << file.rdbuf();
  return stream.str();
}

} // namespace

int
main(int argc, char** argv)
{
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0]
              << " <vertShader> <fragShader> <header in> <header out>"
              << std::endl;
    return EXIT_FAILURE;
  }

  auto vertShader = fileToString(argv[1]);
  auto fragShader = fileToString(argv[2]);

  auto header = fileToString(argv[3]);

  std::regex vertRegex("@vertShader@");
  std::regex fragRegex("@fragShader@");

  header = std::regex_replace(header, vertRegex, vertShader);
  header = std::regex_replace(header, fragRegex, fragShader);

  std::ofstream headerOut(argv[4]);

  headerOut << header;

  return EXIT_SUCCESS;
}
