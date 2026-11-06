#include "Rtypes.h"
#include "yaml-cpp/yaml.h"
#include "TString.h"
#include "TVector3.h"
#include <iostream>
#include <string>
#include <vector>

/*
 * YAML::Node config
 * YAML 是 yaml-cpp 库中的命名空间。
 * Node 是 YAML 命名空间中定义的类。
 * config 是用户创建的 YAML::Node 对象。
 * as<T>() 是 yaml-cpp 库提供的成员函数模板。
 */

void ReadConfig() {

  ///////////////// 输入 YAML 文件 /////////////////
  TString InputYAMLPath = "config.yaml";
  YAML::Node config = YAML::LoadFile(InputYAMLPath.Data());

  ///////////////// 1. 整数 /////////////////
  Int_t detectorID = config["DetectorID"].as<Int_t>();
  Int_t numberOfLayers = config["NumberOfLayers"].as<Int_t>();

  std::cout << "\n===== 整数 =====" << std::endl;
  std::cout << "DetectorID: " << detectorID << std::endl;
  std::cout << "NumberOfLayers: " << numberOfLayers << std::endl;

  ///////////////// 2. 浮点数 /////////////////
  Double_t beamEnergy = config["BeamEnergy"].as<Double_t>();
  Double_t sigmaEnergy = config["SigmaEnergy"].as<Double_t>();
  Double_t threshold = config["Threshold"].as<Double_t>();

  std::cout << "\n===== 浮点数 =====" << std::endl;
  std::cout << "BeamEnergy: " << beamEnergy << std::endl;
  std::cout << "SigmaEnergy: " << sigmaEnergy << std::endl;
  std::cout << "Threshold: " << threshold << std::endl;

  ///////////////// 3. 字符串：TString /////////////////
  TString detectorName = config["DetectorName"].as<std::string>().c_str();
  TString particle = config["Particle"].as<std::string>().c_str();

  std::cout << "\n===== 字符串 =====" << std::endl;
  std::cout << "DetectorName: " << detectorName << std::endl;
  std::cout << "Particle: " << particle << std::endl;

  ///////////////// 4. 布尔值 /////////////////
  Bool_t enableDetector = config["EnableDetector"].as<Bool_t>();
  Bool_t enableSmearing = config["EnableSmearing"].as<Bool_t>();

  std::cout << "\n===== 布尔值 =====" << std::endl;
  std::cout << std::boolalpha;
  std::cout << "EnableDetector: " << enableDetector << std::endl;
  std::cout << "EnableSmearing: " << enableSmearing << std::endl;

  ///////////////// 5. 空值 /////////////////
  YAML::Node calibration = config["CalibrationFactor"];

  std::cout << "\n===== 空值 =====" << std::endl;
  if (calibration.IsNull()) {
    std::cout << "CalibrationFactor: null" << std::endl;
  }

  ///////////////// 6. 数组：转换为 TVector3 /////////////////
  std::vector<Double_t> position = config["Position"].as<std::vector<Double_t>>();
  TVector3 detectorPosition(position[0], position[1], position[2]);

  std::cout << "\n===== Position (TVector3) =====" << std::endl;
  std::cout << "X: " << detectorPosition.X() << std::endl;
  std::cout << "Y: " << detectorPosition.Y() << std::endl;
  std::cout << "Z: " << detectorPosition.Z() << std::endl;

  ///////////////// 7. 数组：多行写法 /////////////////
  std::vector<Double_t> energyLevels = config["EnergyLevels"].as<std::vector<Double_t>>();

  std::cout << "\n===== EnergyLevels 数组 =====" << std::endl;
  for (size_t i = 0; i < energyLevels.size(); ++i) {
    std::cout << "EnergyLevels[" << i << "]: " << energyLevels[i] << std::endl;
  }

  ///////////////// 8. 字典 / 映射 /////////////////
  //相当于两层的
  Double_t sigmaX = config["Resolution"]["SigmaX"].as<Double_t>();
  Double_t sigmaY = config["Resolution"]["SigmaY"].as<Double_t>();
  Double_t sigmaZ = config["Resolution"]["SigmaZ"].as<Double_t>();

  std::cout << "\n===== Resolution 字典 =====" << std::endl;
  std::cout << "SigmaX: " << sigmaX << std::endl;
  std::cout << "SigmaY: " << sigmaY << std::endl;
  std::cout << "SigmaZ: " << sigmaZ << std::endl;

  ///////////////// 9. 嵌套结构：TString + TVector3 /////////////////
  YAML::Node detectors = config["Detectors"];

  std::cout << "\n===== Detectors 嵌套结构 =====" << std::endl;

  for (const auto& detector : detectors) {

    Int_t id = detector["ID"].as<Int_t>();
    TString name = detector["Name"].as<std::string>().c_str();

    std::vector<Double_t> pos = detector["Position"].as<std::vector<Double_t>>();

    if (pos.size() != 3) {
      std::cerr << "Error: Detector " << id << " Position must contain exactly 3 values." << std::endl;
      return;
    }

    TVector3 detectorPos(pos[0], pos[1], pos[2]);

    std::cout << "ID: " << id << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Position X: " << detectorPos.X() << std::endl;
    std::cout << "Position Y: " << detectorPos.Y() << std::endl;
    std::cout << "Position Z: " << detectorPos.Z() << std::endl;
    std::cout << "------------------------" << std::endl;
  }

}
