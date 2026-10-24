// 功能：筛选 Espri.root/SimulatedTree，按左右臂输出 Processed_Left.root 和
//       Processed_right.root；每个文件只有一个 tree，含 ENaI、ETrue、RDCX、RDCY 四个 Double_t 分支。
// 方法：读取 Espri、InitialConditions 和 InteractionCoordinates；要求同臂塑闪和 NaI 有正能量读出，
//       ENaI 取该臂能量最大的 NaI 棒，ETrue 取初始质子的发射动能。
//       遍历 RDC 中面命中，直接读取 GetRdcPosX/Y；仅保留符合臂中恰好一个有效命中的事件。
//       臂号 0 对应 Left/+X，臂号 1 对应 Right/-X；每个保留事件只写入所属臂的文件。
//       与实验比较时 Left 取 espriSide=1，Right 取 espriSide=0 对应的 rdcX/rdcY 数组元素。
//       新增近似未击穿筛选：同臂唯一 NaI 质子记录的累计沉积能 >= 首步后动能。
// 注意事项：能量单位 MeV，坐标单位 mm；依赖已配置的 NPTool/ROOT。
//       输入须已保存实验坐标约定的 RDC 中面位置，不拟合丝读出，也不再次转换坐标。
//       有效坐标须有限且不等于 kInvalidValue；无有效唯一初始质子或符合命中数不为 1 时不写树。
//       塑闪/NaI 读出不含粒子身份和 TrackID，符合依据是同臂正能量，不能作逐轨迹真值匹配。
//       ENaI 是保存的模拟读出能量，包含 NaI 分辨率；ETrue 不做能损反推。
//       RDC 命中按臂号关联能量，不按粒子身份或 TrackID 进一步筛选；多命中不作合并。
//       当前几何用距靶半径 >1100 mm 区分 NaI 与 RDC 交互记录，再按世界 X 正负区分臂。
//       此判据不是严格停止真值，可能接收临界击穿事件；累计沉积能跨 NaI 阵列，不保证单棒阻停。
//       停止筛选使用未展宽交互能量；输出 ENaI 仍是最大棒的展宽读出，不增加输出分支。
//       运行会覆盖当前目录的两个输出文件；只写树，不保存图或原始对象分支。

#include <cmath>
#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TEspriData.h"
#include "TInitialConditions.h"
#include "TInteractionCoordinates.h"

namespace EspriProcessInternal {

const char* const kInputFileName = "Espri.root";
const char* const kOutputLeftFileName = "Processed_Left.root";
const char* const kOutputRightFileName = "Processed_right.root";
const Double_t kInvalidValue = -999.0;
// 当前输入中 RDC 记录半径约 1010--1044 mm，NaI 约 1254--1455 mm。
// 只用于识别记录来自哪个敏感体；改变安装几何后须重新核对，不能把它当作通用阈值。
const Double_t kNaIInteractionMinimumRadiusMM = 1100.0;

// RDC/NaI 臂号为 0/1，塑闪探测器号为 1/2。
Bool_t HasPlasticHit(TEspriData* espri, Int_t arm) {
  for (UInt_t hit = 0; hit < espri->GetPlasMultEnergy(); ++hit) {
    if (espri->GetPlasEDetectorNbr(hit) != arm + 1) continue;
    const Double_t energy = espri->GetPlasEnergy(hit);
    if (std::isfinite(energy) && energy > 0.0) return kTRUE;
  }
  return kFALSE;
}

Bool_t GetMaximumNaIEnergy(TEspriData* espri, Int_t arm, Double_t& energy) {
  energy = kInvalidValue;
  Bool_t found = kFALSE;
  for (Int_t hit = 0; hit < espri->GetNaIEMult(); ++hit) {
    if (espri->GetNaIEDetectorNbr(hit) != arm) continue;
    const Double_t hitEnergy = espri->GetNaIEEnergy(hit);
    if (!std::isfinite(hitEnergy) || hitEnergy <= 0.0) continue;
    if (!found || hitEnergy > energy) {
      found = kTRUE;
      energy = hitEnergy;
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

void Process() {
  using namespace EspriProcessInternal;

  TFile* inputFile = TFile::Open(kInputFileName, "READ");
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

  TFile* outputLeftFile = TFile::Open(kOutputLeftFileName, "RECREATE");
  outputLeftFile->cd();
  TTree* outputLeftTree = new TTree("tree", "Left-arm coincidence with approximate NaI stopping selection and experimental-convention RDC coordinates");
  outputLeftTree->Branch("ENaI", &ENaI, "ENaI/D");
  outputLeftTree->Branch("ETrue", &ETrue, "ETrue/D");
  outputLeftTree->Branch("RDCX", &RDCX, "RDCX/D");
  outputLeftTree->Branch("RDCY", &RDCY, "RDCY/D");

  TFile* outputRightFile = TFile::Open(kOutputRightFileName, "RECREATE");
  outputRightFile->cd();
  TTree* outputRightTree = new TTree("tree", "Right-arm coincidence with approximate NaI stopping selection and experimental-convention RDC coordinates");
  outputRightTree->Branch("ENaI", &ENaI, "ENaI/D");
  outputRightTree->Branch("ETrue", &ETrue, "ETrue/D");
  outputRightTree->Branch("RDCX", &RDCX, "RDCX/D");
  outputRightTree->Branch("RDCY", &RDCY, "RDCY/D");

  const Long64_t entries = inputTree->GetEntries();
  Long64_t validTruthEvents = 0;
  Long64_t coincidenceEvents = 0;
  Long64_t missingRDCEvents = 0;
  Long64_t multipleRDCEvents = 0;
  Long64_t beforeStoppingCutEvents = 0;
  Long64_t missingNaITruthEvents = 0;
  Long64_t rejectedStoppingCutEvents = 0;
  Long64_t keptEvents = 0;
  Long64_t keptLeftEvents = 0;
  Long64_t keptRightEvents = 0;

  for (Long64_t entry = 0; entry < entries; ++entry) {
    inputTree->GetEntry(entry);
    ENaI = ETrue = RDCX = RDCY = kInvalidValue;

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
      if (!HasPlasticHit(espri, arm)) continue;
      if (!GetMaximumNaIEnergy(espri, arm, maximumNaIEnergy)) continue;
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

  std::cout << "RDC coordinates: direct GetRdcPosX/Y, no fit or coordinate transform (mm)\n"
            << "Input events: " << entries << '\n'
            << "Valid proton truth: " << validTruthEvents << '\n'
            << "Same-arm Plastic-NaI coincidence: " << coincidenceEvents << '\n'
            << "Coincidence without valid RDC hit: " << missingRDCEvents << '\n'
            << "Coincidence with multiple valid RDC hits: " << multipleRDCEvents << '\n'
            << "Before approximate NaI stopping cut: " << beforeStoppingCutEvents << '\n'
            << "Missing or ambiguous NaI proton record: " << missingNaITruthEvents << '\n'
            << "Rejected by Edep_NaI < K_after_first_step: " << rejectedStoppingCutEvents << '\n'
            << "Approximate NaI stopping cut: Edep_NaI >= K_after_first_step (not strict stopping truth)\n"
            << "Saved events: " << keptEvents << '\n'
            << "Left events: " << keptLeftEvents << " -> " << kOutputLeftFileName << "/tree\n"
            << "Right events: " << keptRightEvents << " -> " << kOutputRightFileName << "/tree" << std::endl;
}
