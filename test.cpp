#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

std::string findN0vaDesktopDirectory(const fs::path& currentPath) {
    // 组合完整的文件夹路径
    fs::path fullPath = currentPath / "N0vaDesktop";

    // 判断路径是否存在
    if (fs::exists(fullPath)) {
        return fullPath.string();
    }

    // 递归遍历当前路径下的子目录
    for (const auto& entry : fs::directory_iterator(currentPath)) {
        if (entry.is_directory()) {
            std::string foundPath = findN0vaDesktopDirectory(entry.path());
            if (!foundPath.empty()) {
                return foundPath; // 找到了，直接返回路径
            }
        }
    }

    return ""; // 返回空字符串表示未找到
}

int main() {
    for (char driveLetter = 'C'; driveLetter <= 'Z'; ++driveLetter) {
        // 构造盘符路径
        fs::path drivePath = fs::path(std::string(1, driveLetter) + ":/");

        // 检查盘符是否存在
        if (fs::exists(drivePath)) {
            std::string fullPath = findN0vaDesktopDirectory(drivePath);
            if (!fullPath.empty()) {
                std::cout << "FIND PATH " << fullPath << std::endl;
                break; // 找到目标文件夹，停止搜索
            }
        }
    }

    return 0;
}


