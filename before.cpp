#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

const std::array<uint8_t, 8> MAGIC_NUMBER_1 {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x66, 0x74};
const std::array<uint8_t, 8> MAGIC_NUMBER_2 {0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x66, 0x74};

void processFile(const fs::path& filepath, const fs::path& outputDir) {

  std::cout << "Process Begin: " << filepath << "\n";

  fs::path outfile = outputDir / filepath.filename();
  outfile.replace_extension(".mp4");

  std::ifstream in(filepath, std::ios::binary);
  std::ofstream out(outfile, std::ios::binary);

  std::cout << "Extracting...\n";
  in.ignore(2);

  std::cout << "Writing to output file: " << outfile << "\n";
  out << in.rdbuf();

  std::cout << "Complete.\n";
}

bool matchMagicNumber(const fs::path& filepath) {

  std::ifstream in(filepath, std::ios::binary);
  std::array<uint8_t, 8> buffer;
  in.read((char*)buffer.data(), buffer.size());

  return (buffer == MAGIC_NUMBER_1) || (buffer == MAGIC_NUMBER_2);
}

int main() {

  fs::path inputDir = fs::current_path();
  fs::path outputDir = inputDir / "output";

  std::cout << "PATH:\tDefault Input Mode:" << inputDir << "\n";

  if (!fs::exists(outputDir)) {
    std::cout << "Create input path:" << outputDir << "\n";
    fs::create_directory(outputDir);
  } else {
    std::cout << "Output path exist.\n";
  }

  std::cout << "Find NDF file...\n";

  std::vector<fs::path> ndfFiles;
  for (auto& entry : fs::directory_iterator(inputDir)) {
    if (entry.path().extension() == ".ndf") {
      ndfFiles.push_back(entry.path());
    }
  }

  if (ndfFiles.empty()) {
    std::cout << "NDF file disappear.\n";
    return 0;
  }

  std::cout << "Match magic number...\n";

  std::vector<fs::path> matchedFiles;
  for (auto& file : ndfFiles) {
    if (matchMagicNumber(file)) {
      matchedFiles.push_back(file);
    }
  }

  if (matchedFiles.empty()) {
    std::cout << "No match.\n";
    return 0;
  }

  std::cout << "Match success with begin...\n";

  if (!matchedFiles.empty()) {
    fs::path fileToProcess = matchedFiles[0];
    processFile(fileToProcess, outputDir);
  }

  std::cout << "All success!\n";
    system("Pause");
  return 0;
}