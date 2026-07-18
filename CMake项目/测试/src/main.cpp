/*
常见内容包括：

  1. 包含头文件。
  2. 接收命令行参数。
  3. 设置输入、输出路径或程序配置。
  4. 创建需要的对象。
  5. 调用真正实现功能的函数或类。
  6. 处理错误。
  7. 返回程序运行状态。


./build/bin/draw_energy_loss input.root result.pdf

  对应关系为：

  argc = 3

  argv[0] = "./build/bin/draw_energy_loss"
  argv[1] = "input.root"
  argv[2] = "result.pdf"

  注意，程序名本身也算一个参数，因此不传额外参数时：

  argc = 1
*/
#include "GraphPlotter.h"
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  std::string inputFile =
      "/Users/yemingxin/ROOT_Exercise/CMake项目/测试/"
      "EnergyLossTable1HInH_3.root";
  std::string outputPdf =
      "/Users/yemingxin/ROOT_Exercise/CMake项目/测试/output/"
      "EnergyLossTable1HInH_3.pdf";

  if (argc > 1) {
    inputFile = argv[1];
  }

  if (argc > 2) {
    outputPdf = argv[2];
  }

  try {
    DrawGraphToPdf(inputFile, "histograms/EnergyToRangeModel4", outputPdf);//把root文件和TGraph的路径传给DrawGraphToPdf
    std::cout << "PDF written to: " << outputPdf << '\n';
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << '\n';
    return 1;
  }

  return 0;
}

