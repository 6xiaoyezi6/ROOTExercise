// 功能：依次筛选 Espri.root 和 Espri_Evaluation.root 的 SimulatedTree；训练样本按左右臂
//       输出 Processed_Left.root、Processed_right.root，独立评估样本输出
//       Processed_Left_Eval.root、Processed_Right_Eval.root。
//       每个文件只有一个 tree，含 ENaI、ETrue、RDCX、RDCY 四个 Double_t 分支及 Int_t NaIBarID。
// 方法：读取 Espri、InitialConditions 和 InteractionCoordinates；要求同臂塑闪和 NaI 有正能量读出，
//       ENaI 和 NaIBarID 取同一条最大能量 NaI 读出；ETrue 取初始质子的发射动能。
//       模拟 Left/arm 0 的原始棒号与实验反序，NaIBarID=7-GetNaIEId(hit)，输出 1--7；
//       Right/arm 1 同序，NaIBarID=8+GetNaIEId(hit)，输出 8--14。
//       实验 espriNaiBarId 为 0--6，统一编号为 7*(1-espriSide)+espriNaiBarId+1，不再反序。
//       两端统一棒号均按 RDCY 从高到低排列；Left 反序与模拟 RDCY 读出符号相符。
//       遍历 RDC 中面命中，直接读取 GetRdcPosX/Y；仅保留符合臂中恰好一个有效命中的事件。
//       臂号 0 对应 Left/+X，臂号 1 对应 Right/-X；每个保留事件只写入所属臂的文件。
//       与实验比较时 Left 取 espriSide=1，Right 取 espriSide=0 对应的 rdcX/rdcY 数组元素。
//       按 CheckPosition.C 的几何将 RDC 读出坐标还原为世界中面位置，以真实顶点到该点
//       的方向计算实验室极角，只保留 55<=theta_p<=70 度（含端点）；不使用初始发射角。
//       近似未击穿筛选：同臂唯一 NaI 质子记录的累计沉积能 >= 首步后动能。
// 注意事项：能量单位 MeV，坐标单位 mm；依赖已配置的 NPTool/ROOT。
//       输入须已保存实验坐标约定的 RDC 中面位置，不拟合丝读出，也不再次转换坐标。
//       有效坐标须有限且不等于 kInvalidValue；无有效唯一初始质子或符合命中数不为 1 时不写树。
//       塑闪/NaI 读出不含粒子身份和 TrackID，符合依据是同臂正能量，不能作逐轨迹真值匹配。
//       ENaI 是保存的模拟读出能量，包含 NaI 分辨率；ETrue 不做能损反推。
//       RDC 命中按臂号关联能量，不按粒子身份或 TrackID 进一步筛选；多命中不作合并。
//       角度计算依赖当前 RDC 安装角、半径和读出原点；改变几何须同步核对 CheckPosition.C。
//       世界坐标仅用于角度门，输出 RDCX/Y 保持原读出坐标；本宏不处理实验文件。
//       当前几何用距靶半径 >1100 mm 区分 NaI 与 RDC 交互记录，再按世界 X 正负区分臂。
//       此判据不是严格停止真值，可能接收临界击穿事件；累计沉积能跨 NaI 阵列，不保证单棒阻停。
//       停止筛选使用未展宽交互能量；输出 ENaI 仍是最大棒的展宽读出，棒号不是该臂任意命中的编号。
//       两份输入使用完全相同的事件筛选和输出结构；相对路径均以启动目录为准。
//       运行会覆盖当前目录的四个输出文件；只写树，不保存图或原始对象分支。

#include <cmath>
#include <iostream>

#include "TFile.h"
#include "TMath.h"
#include "TTree.h"
#include "TVector3.h"
#include "TEspriData.h"
#include "TInitialConditions.h"
#include "TInteractionCoordinates.h"

namespace EspriProcessInternal {

const char* const kInputFileName = "Espri.root";
const char* const kOutputLeftFileName = "Processed_Left.root";
const char* const kOutputRightFileName = "Processed_right.root";
const char* const kEvaluationInputFileName = "Espri_Evaluation.root";
const char* const kEvaluationOutputLeftFileName = "Processed_Left_Eval.root";
const char* const kEvaluationOutputRightFileName = "Processed_Right_Eval.root";
const Double_t kInvalidValue = -999.0;
const Double_t kProtonThetaMinDeg = 55.0;
const Double_t kProtonThetaMaxDeg = 70.0;
// 与 CheckPosition.C 的 RDC 中面几何和实验读出原点保持一致。
const Double_t kRdcThetaDeg[2] = {60.333284806151, 60.0};
const Double_t kRdcRadiusMM[2] = {1172.434333449206 - 162.0, 1173.19 - 162.0};
const Double_t kRdcReadoutCenterX[2] = {227.5 - 1.40, 227.5 - 3.35};
const Double_t kRdcReadoutCenterY = 227.5 - 3.0;
// 当前输入中 RDC 记录半径约 1010--1044 mm，NaI 约 1254--1455 mm。
// 只用于识别记录来自哪个敏感体；改变安装几何后须重新核对，不能把它当作通用阈值。
const Double_t kNaIInteractionMinimumRadiusMM = 1100.0;

TVector3 RdcToWorld(Int_t arm, Double_t rdcX, Double_t rdcY) {
  const Double_t theta = kRdcThetaDeg[arm] * TMath::DegToRad();
  const Double_t x = rdcX - kRdcReadoutCenterX[arm];
  const Double_t y = rdcY - kRdcReadoutCenterY;
  if (arm == 0) {
    return TVector3(kRdcRadiusMM[arm]*std::sin(theta) + x*std::cos(theta),
                    y, kRdcRadiusMM[arm]*std::cos(theta) - x*std::sin(theta));
  }
  return TVector3(-kRdcRadiusMM[arm]*std::sin(theta) + x*std::cos(theta),
                  y, kRdcRadiusMM[arm]*std::cos(theta) + x*std::sin(theta));
}

// RDC/NaI 臂号为 0/1，塑闪探测器号为 1/2。
Bool_t HasPlasticHit(TEspriData* espri, Int_t arm) {
  for (UInt_t hit = 0; hit < espri->GetPlasMultEnergy(); ++hit) {
    if (espri->GetPlasEDetectorNbr(hit) != arm + 1) continue;
    const Double_t energy = espri->GetPlasEnergy(hit);
    if (std::isfinite(energy) && energy > 0.0) return kTRUE;
  }
  return kFALSE;
}

Bool_t GetMaximumNaIEnergy(TEspriData* espri, Int_t arm, Double_t& energy, Int_t& barID) {
  energy = kInvalidValue;
  barID = static_cast<Int_t>(kInvalidValue);
  Bool_t found = kFALSE;
  for (Int_t hit = 0; hit < espri->GetNaIEMult(); ++hit) {
    if (espri->GetNaIEDetectorNbr(hit) != arm) continue;
    const Double_t hitEnergy = espri->GetNaIEEnergy(hit);
    if (!std::isfinite(hitEnergy) || hitEnergy <= 0.0) continue;
    if (!found || hitEnergy > energy) {
      found = kTRUE;
      energy = hitEnergy;
      const Int_t localBarID = espri->GetNaIEId(hit);
      if (arm == 0) barID = 7 - localBarID;
      else barID = 8 + localBarID;
    }
  }
  return found;
}

Bool_t GetNaIStoppingEnergies(TInteractionCoordinates* interactions, Int_t arm,
                             Double_t& deposit, Double_t& kineticAfterFirstStep) {
  deposit = kineticAfterFirstStep = kInvalidValue;
  Int_t matchedRecords = 0;
  for (Int_t hit = 0; hit < interactions->GetDetectedMultiplicity(); ++hit) {
    if (interactions->GetParticleName(hit) != "proton") continue;
    const Double_t x = interactions->GetDetectedPositionX(hit);
    const Double_t y = interactions->GetDetectedPositionY(hit);
    const Double_t z = interactions->GetDetectedPositionZ(hit);
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) continue;
    if (arm == 0 && x <= 0.0) continue;
    if (arm == 1 && x >= 0.0) continue;
    const Double_t radiusSquared = x*x + y*y + z*z;
    if (radiusSquared <= kNaIInteractionMinimumRadiusMM*kNaIInteractionMinimumRadiusMM) continue;
    ++matchedRecords;
    deposit = interactions->GetEnergy(hit);
    kineticAfterFirstStep = interactions->GetKineticEnergy(hit);
  }
  if (matchedRecords == 1 && std::isfinite(deposit) && deposit > 0.0 &&
      std::isfinite(kineticAfterFirstStep) && kineticAfterFirstStep >= 0.0) return kTRUE;
  deposit = kineticAfterFirstStep = kInvalidValue;
  return kFALSE;
}

} // namespace EspriProcessInternal

void ProcessEspriFile(const char* inputFileName, const char* outputLeftFileName,
                      const char* outputRightFileName) {
  using namespace EspriProcessInternal;

  TFile* inputFile = TFile::Open(inputFileName, "READ");
  TTree* inputTree = (TTree*)inputFile->Get("SimulatedTree");

  TEspriData* espri = nullptr;
  TInitialConditions* initialConditions = nullptr;
  TInteractionCoordinates* interactions = nullptr;

  inputTree->SetBranchAddress("Espri", &espri);
  inputTree->SetBranchAddress("InitialConditions", &initialConditions);
  inputTree->SetBranchAddress("InteractionCoordinates", &interactions);

  Double_t ENaI = kInvalidValue;
  Double_t ETrue = kInvalidValue;
  Double_t RDCX = kInvalidValue;
  Double_t RDCY = kInvalidValue;
  Int_t NaIBarID = static_cast<Int_t>(kInvalidValue);

  TFile* outputLeftFile = TFile::Open(outputLeftFileName, "RECREATE");
  outputLeftFile->cd();
  TTree* outputLeftTree = new TTree("tree", "Left-arm Plastic-NaI coincidence, 55<=theta_p<=70 deg, approximate NaI stop");
  outputLeftTree->Branch("ENaI", &ENaI, "ENaI/D");
  outputLeftTree->Branch("ETrue", &ETrue, "ETrue/D");
  outputLeftTree->Branch("RDCX", &RDCX, "RDCX/D");
  outputLeftTree->Branch("RDCY", &RDCY, "RDCY/D");
  outputLeftTree->Branch("NaIBarID", &NaIBarID, "NaIBarID/I");

  TFile* outputRightFile = TFile::Open(outputRightFileName, "RECREATE");
  outputRightFile->cd();
  TTree* outputRightTree = new TTree("tree", "Right-arm Plastic-NaI coincidence, 55<=theta_p<=70 deg, approximate NaI stop");
  outputRightTree->Branch("ENaI", &ENaI, "ENaI/D");
  outputRightTree->Branch("ETrue", &ETrue, "ETrue/D");
  outputRightTree->Branch("RDCX", &RDCX, "RDCX/D");
  outputRightTree->Branch("RDCY", &RDCY, "RDCY/D");
  outputRightTree->Branch("NaIBarID", &NaIBarID, "NaIBarID/I");

  const Long64_t entries = inputTree->GetEntries();
  Long64_t validTruthEvents = 0;
  Long64_t coincidenceEvents = 0;
  Long64_t missingRDCEvents = 0;
  Long64_t multipleRDCEvents = 0;
  Long64_t beforeAngleCutEvents = 0;
  Long64_t invalidAngleEvents = 0;
  Long64_t rejectedAngleCutEvents = 0;
  Long64_t beforeStoppingCutEvents = 0;
  Long64_t missingNaITruthEvents = 0;
  Long64_t rejectedStoppingCutEvents = 0;
  Long64_t keptEvents = 0;
  Long64_t keptLeftEvents = 0;
  Long64_t keptRightEvents = 0;

  for (Long64_t entry = 0; entry < entries; ++entry) {
    inputTree->GetEntry(entry);
    ENaI = ETrue = RDCX = RDCY = kInvalidValue;
    NaIBarID = static_cast<Int_t>(kInvalidValue);

    // 初始粒子名称定位质子；不把探测器沉积能或 TrackInfo 下游动能当作顶点真值。
    Int_t protonIndex = -1;
    Int_t protonCount = 0;
    for (Int_t particle = 0; particle < initialConditions->GetParticleMultiplicity(); ++particle) {
      if (initialConditions->GetParticleName(particle) != "proton") continue;
      protonIndex = particle;
      ++protonCount;
    }
    if (protonCount != 1) continue;
    ETrue = initialConditions->GetKineticEnergy(protonIndex);
    if (!std::isfinite(ETrue) || ETrue <= 0.0) continue;
    ++validTruthEvents;

    Bool_t hasCoincidence = kFALSE;
    Int_t validRDCHits = 0;
    Int_t selectedArm = -1;
    for (Int_t arm = 0; arm < 2; ++arm) {
      Double_t maximumNaIEnergy = kInvalidValue;
      Int_t maximumNaIBarID = static_cast<Int_t>(kInvalidValue);
      if (!HasPlasticHit(espri, arm)) continue;
      if (!GetMaximumNaIEnergy(espri, arm, maximumNaIEnergy, maximumNaIBarID)) continue;
      hasCoincidence = kTRUE;

      for (UInt_t hit = 0; hit < espri->GetRdcMultPosition(); ++hit) {
        if (espri->GetRdcPosDetectorNbr(hit) != arm) continue;
        const Double_t x = espri->GetRdcPosX(hit);
        const Double_t y = espri->GetRdcPosY(hit);
        if (!std::isfinite(x) || !std::isfinite(y)) continue;
        if (x == kInvalidValue || y == kInvalidValue) continue;
        ++validRDCHits;
        selectedArm = arm;
        ENaI = maximumNaIEnergy;
        NaIBarID = maximumNaIBarID;
        // 模拟已按实验读出原点和轴方向保存，原值写出，不能再次平移或翻转。
        RDCX = x;
        RDCY = y;
      }
    }

    if (hasCoincidence) ++coincidenceEvents;
    if (hasCoincidence && validRDCHits == 0) ++missingRDCEvents;
    if (hasCoincidence && validRDCHits > 1) ++multipleRDCEvents;
    // 在所有符合臂中合计命中数；两臂各有一个命中也属于多命中事件，予以排除。
    if (validRDCHits != 1) continue;
    ++beforeAngleCutEvents;

    const TVector3 vertex(initialConditions->GetIncidentPositionX(),
                          initialConditions->GetIncidentPositionY(),
                          initialConditions->GetIncidentPositionZ());
    if (!std::isfinite(vertex.X()) || !std::isfinite(vertex.Y()) || !std::isfinite(vertex.Z()) ||
        vertex.X() == kInvalidValue || vertex.Y() == kInvalidValue || vertex.Z() == kInvalidValue) {
      ++invalidAngleEvents;
      continue;
    }
    // 沿用比较图的几何角度定义；变换仅服务于角度计算，不改写输出 RDCX/Y。
    const TVector3 direction = RdcToWorld(selectedArm, RDCX, RDCY) - vertex;
    const Double_t protonTheta = direction.Theta() * TMath::RadToDeg();
    if (!std::isfinite(direction.Mag2()) || direction.Mag2() <= 0.0 || !std::isfinite(protonTheta)) {
      ++invalidAngleEvents;
      continue;
    }
    if (protonTheta < kProtonThetaMinDeg || protonTheta > kProtonThetaMaxDeg) {
      ++rejectedAngleCutEvents;
      continue;
    }
    ++beforeStoppingCutEvents;
    Double_t naIDeposit = kInvalidValue;
    Double_t naIKineticAfterFirstStep = kInvalidValue;
    if (!GetNaIStoppingEnergies(interactions, selectedArm, naIDeposit, naIKineticAfterFirstStep)) {
      ++missingNaITruthEvents;
      continue;
    }
    if (naIDeposit < naIKineticAfterFirstStep) {
      ++rejectedStoppingCutEvents;
      continue;
    }
    if (selectedArm == 0) {
      outputLeftTree->Fill();
      ++keptLeftEvents;
    }
    else {
      outputRightTree->Fill();
      ++keptRightEvents;
    }
    ++keptEvents;
  }

  outputLeftFile->cd();
  outputLeftTree->Write();
  outputLeftFile->Close();
  outputRightFile->cd();
  outputRightTree->Write();
  outputRightFile->Close();
  inputFile->Close();
  delete outputLeftFile;
  delete outputRightFile;
  delete inputFile;

  std::cout << "Input file: " << inputFileName << '\n'
            << "RDC coordinates: direct GetRdcPosX/Y, no fit or coordinate transform (mm)\n"
            << "NaIBarID: same maximum-energy readout; Left=7-localID, Right=8+localID; experiment-aligned\n"
            << "Input events: " << entries << '\n'
            << "Valid proton truth: " << validTruthEvents << '\n'
            << "Same-arm Plastic-NaI coincidence: " << coincidenceEvents << '\n'
            << "Coincidence without valid RDC hit: " << missingRDCEvents << '\n'
            << "Coincidence with multiple valid RDC hits: " << multipleRDCEvents << '\n'
            << "Before proton angle cut: " << beforeAngleCutEvents << '\n'
            << "Invalid vertex or RDC direction: " << invalidAngleEvents << '\n'
            << "Rejected outside proton angle range: " << rejectedAngleCutEvents << '\n'
            << "Proton angle cut: " << kProtonThetaMinDeg << " <= theta_p <= " << kProtonThetaMaxDeg
            << " deg (true vertex to RDC mid-plane, as in CheckPosition.C)\n"
            << "Before approximate NaI stopping cut: " << beforeStoppingCutEvents << '\n'
            << "Missing or ambiguous NaI proton record: " << missingNaITruthEvents << '\n'
            << "Rejected by Edep_NaI < K_after_first_step: " << rejectedStoppingCutEvents << '\n'
            << "Approximate NaI stopping cut: Edep_NaI >= K_after_first_step (not strict stopping truth)\n"
            << "Saved events: " << keptEvents << '\n'
            << "Left events: " << keptLeftEvents << " -> " << outputLeftFileName << "/tree\n"
            << "Right events: " << keptRightEvents << " -> " << outputRightFileName << "/tree" << std::endl;
}

void PreProcess() {
  using namespace EspriProcessInternal;
  ProcessEspriFile(kInputFileName, kOutputLeftFileName, kOutputRightFileName);
  ProcessEspriFile(kEvaluationInputFileName, kEvaluationOutputLeftFileName, kEvaluationOutputRightFileName);
}
