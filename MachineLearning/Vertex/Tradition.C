/*
 * 功能：遍历本宏目录中当前 TOGAXSI 生成的 alphap.root/alphap_test.root，直接从 SSD 条带坐标重建
 *       世界顶点，只将成功事件写入 Tradition.root/Tradition_test.root，保留 EventID。
 * 方法：与 PreProcess 相同的七层同轨迹 alpha-proton 和左右配对选择；仿照
 *       ReconstrucionTOGAXSI_Hits.C，Cluster 两投影拟合→Recoil 局域系→与
 *       Recoil X1/X2 的 X-Z 直线求交→世界系；局域几何来自源文件配置。
 * 注意事项：不读取 Processd.root，不预先按机器学习事件筛选；每个源事件最多一行。
 *           Cluster 使用世界条带位置，Recoil 使用局域条带位置，单位 mm。
 *           EventID 取原始 SSD 模拟事件号，在每个文件内唯一，两文件不可混用。
 *           真实世界顶点只作评估标签和残差计算；Recoil Y1 不参与几何求交。
 *           不额外施加 GAGG/能损/运动学筛选；无效、多击中、错配和拟合失败跳过。
 *           输出及配置快照覆盖同名文件；输入错误报错返回，不当作成功重建。
 *           依赖本机 NPTool 字典和 ROOT；训练与测试必须使用同一模拟响应和不同随机种子。
 */
R__ADD_INCLUDE_PATH(/Users/yemingxin/nptool/NPLib/include)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPCore.dylib)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPClusterSSD.dylib)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPRecoilSSD.dylib)
R__LOAD_LIBRARY(/Users/yemingxin/nptool/NPLib/lib/libNPReactionConditions.dylib)
#include "TAsciiFile.h"
#include "TClusterSSDData.h"
#include "TRecoilSSDData.h"
#include "TReactionConditions.h"
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include "TString.h"
#include "TVector3.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompLU.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>

namespace VertexTradition {
const double kInvalidValue = -999.0;
struct Geometry {
  bool enabled = false;
  TVector3 origin, x, y, z;
};
bool Valid(double v) { return std::isfinite(v) && v != kInvalidValue; }
void Require(bool ok, const std::string& message) {
  if (!ok) throw std::runtime_error(message);
}
// 与参考程序相同，仅从各组 X1 的 POS 定义局域原点与坐标轴。
std::map<std::string, Geometry> ReadGeometry(const char* path) {
  std::ifstream input(path);
  Require(input.is_open(), "Cannot read DetectorConfiguration snapshot");
  std::map<std::string, Geometry> groups;
  std::string line;
  TString type;
  TVector3 position;
  bool inBlock = false, hasType = false, hasPosition = false;
  while (std::getline(input, line)) {
    const auto comment = line.find('%');
    if (comment != std::string::npos) line.erase(comment);
    TString compact(line.c_str());
    compact.ReplaceAll(" ", ""); compact.ReplaceAll("\t", ""); compact.ReplaceAll("\r", "");
    if (compact == "RecoilSSD") {
      inBlock = true; hasType = hasPosition = false;
      continue;
    }
    if (!inBlock) continue;
    TString upper = compact; upper.ToUpper();
    if (upper.BeginsWith("TYPE=")) {
      type = compact(5, compact.Length() - 5); type.ToUpper(); hasType = true;
    }
    TString source(line.c_str()), upperSource(source); upperSource.ToUpper();
    if (upperSource.Contains("POS=")) {
      const auto equal = source.First('=');
      TString value = source(equal + 1, source.Length() - equal - 1);
      double x, y, z;
      if (std::sscanf(value.Data(), "%lf %lf %lf", &x, &y, &z) == 3) {
        position.SetXYZ(x, y, z); hasPosition = true;
      }
    }
    if (!hasType || !hasPosition) continue;
    if (type.Contains("X1")) {
      for (const char* name : {"LU", "LD", "RU", "RD"}) {
        if (!type.Contains(name)) continue;
        Geometry g;
        g.origin = position;
        g.x.SetXYZ(0, 0, -1);
        g.z.SetXYZ(position.X(), position.Y(), 0);
        if (Valid(position.X()) && Valid(position.Y()) && Valid(position.Z()) && g.z.Mag() > 0) {
          g.z = g.z.Unit(); g.y = g.z.Cross(g.x).Unit(); g.enabled = true;
        }
        Require(groups.count(name) == 0, "Duplicate Recoil X1 geometry");
        groups[name] = g;
      }
    }
    inBlock = false;
  }
  Require(!groups.empty(), "No Recoil X1 geometry in DetectorConfiguration");
  return groups;
}
// 两点版本与参考 LeastSquares 使用相同正规方程及 |det| < 1e-5 判断。
bool Fit(double z1, double x1, double z2, double x2, double& a, double& b) {
  if (!Valid(z1) || !Valid(z2) || !Valid(x1) || !Valid(x2)) return false;
  TMatrixD matrix(2, 2);
  matrix(0, 0) = z1*z1 + z2*z2;
  matrix(0, 1) = matrix(1, 0) = z1 + z2; matrix(1, 1) = 2.0;
  TVectorD rhs(2); rhs(0) = z1*x1 + z2*x2; rhs(1) = x1+x2;
  if (std::fabs(matrix(0, 0)*2.0 - matrix(0, 1)*matrix(1, 0)) < 1e-5) return false;
  TDecompLU lu(matrix);
  if (!lu.Solve(rhs)) return false;
  a = rhs(0); b = rhs(1);
  return std::isfinite(a) && std::isfinite(b);
}
// 直接从原始七层击中提取条带输入，保持与 PreProcess 相同的事件选择。
bool BuildInput(Long64_t& id, std::array<double, 14>& v,
              TClusterSSDData* cluster, TRecoilSSDData* recoil,
              std::string& group, double& z1, double& z2) {
  if (!cluster || !recoil || cluster->fClusterSSDHits.size() != 4 || recoil->fRecoilSSDHits.size() != 3) return false;
  const auto& firstC = cluster->fClusterSSDHits.front();
  const auto& firstR = recoil->fRecoilSSDHits.front();
  id = firstC.EventID;
  if (id < 0) return false;
  const TClusterSSDHit* c[4] = {};
  const TRecoilSSDHit* r[3] = {};
  for (const auto& hit : cluster->fClusterSSDHits) {
    if (hit.EventID != id || hit.ParticleName != "alpha" || hit.TrackID != firstC.TrackID ||
        hit.ClusterSSDGroup != firstC.ClusterSSDGroup || !std::isfinite(hit.StripEnergy) || hit.StripEnergy <= 0) return false;
    int index = hit.ClusterSSDType == "X1" ? 0 : hit.ClusterSSDType == "Y1" ? 1 :
                hit.ClusterSSDType == "X2" ? 2 : hit.ClusterSSDType == "Y2" ? 3 : -1;
    if (index < 0 || c[index]) return false;
    c[index] = &hit;
  }
  for (const auto& hit : recoil->fRecoilSSDHits) {
    if (hit.EventID != id || hit.ParticleName != "proton" || hit.TrackID != firstR.TrackID ||
        hit.RecoilSSDGroup != firstR.RecoilSSDGroup || !std::isfinite(hit.StripEnergy) || hit.StripEnergy <= 0) return false;
    int index = hit.RecoilSSDType == "X1" ? 0 : hit.RecoilSSDType == "X2" ? 1 : hit.RecoilSSDType == "Y1" ? 2 : -1;
    if (index < 0 || r[index]) return false;
    r[index] = &hit;
  }
  group = firstR.RecoilSSDGroup;
  if (!(firstC.ClusterSSDGroup == "Left" && (group == "RU" || group == "RD")) &&
      !(firstC.ClusterSSDGroup == "Right" && (group == "LU" || group == "LD"))) return false;
  std::array<double, 11> source = {{
    c[0]->StripXYPositionWorld, c[0]->StripZPositionWorld, c[1]->StripXYPositionWorld, c[1]->StripZPositionWorld,
    c[2]->StripXYPositionWorld, c[2]->StripZPositionWorld, c[3]->StripXYPositionWorld, c[3]->StripZPositionWorld,
    r[0]->StripXYPositionLocus, r[1]->StripXYPositionLocus, r[2]->StripXYPositionLocus}};
  for (int i = 0; i < 11; ++i)
    if (!Valid(source[i])) return false;
  for (int i = 0; i < 11; ++i) v[i] = source[i];
  z1 = r[0]->StripZPositionLocus; z2 = r[1]->StripZPositionLocus;
  return Valid(z1) && Valid(z2);
}
bool Reconstruct(const std::array<double, 14>& v, const Geometry& g, double z1, double z2, TVector3& vertex) {
  if (!g.enabled) return false;
  double a, b, c, d, e, f;
  if (!Fit(v[1], v[0], v[5], v[4], a, b) ||
      !Fit(v[3], v[2], v[7], v[6], c, d) || !Fit(z1, v[8], z2, v[9], e, f)) return false;
  TVector3 direction(a, c, 1), point(b, d, 0);
  TVector3 u(direction.Dot(g.x), direction.Dot(g.y), direction.Dot(g.z));
  const TVector3 delta = point - g.origin;
  TVector3 p(delta.Dot(g.x), delta.Dot(g.y), delta.Dot(g.z));
  if (std::fabs(u.Z()) < 1e-9) return false;
  a = u.X()/u.Z(); b = p.X()-a*p.Z();
  c = u.Y()/u.Z(); d = p.Y()-c*p.Z();
  const double denom = a-e;
  if (std::fabs(denom) < 1e-9) return false;
  const double z = (f-b)/denom;
  TVector3 local((a*f-b*e)/denom, c*z+d, z);
  vertex = g.origin + g.x*local.X() + g.y*local.Y() + g.z*local.Z();
  return std::isfinite(vertex.X()) && std::isfinite(vertex.Y()) && std::isfinite(vertex.Z());
}
void Process(const TString& rawPath, const TString& outputPath) {
  std::unique_ptr<TFile> raw(TFile::Open(rawPath, "READ"));
  Require(raw && !raw->IsZombie(), "Cannot open " + std::string(rawPath.Data()));
  TTree* tree = dynamic_cast<TTree*>(raw->Get("SimulatedTree"));
  Require(tree != nullptr, "Missing SimulatedTree");
  TClusterSSDData* cluster = nullptr;
  TRecoilSSDData* recoil = nullptr;
  TReactionConditions* reaction = nullptr;
  // 保持全部分支启用：对象的 split 子分支未必以 ClusterSSD/RecoilSSD 命名。
  Require(tree->SetBranchAddress("ClusterSSD", &cluster) >= 0, "Missing ClusterSSD branch");
  Require(tree->SetBranchAddress("RecoilSSD", &recoil) >= 0, "Missing RecoilSSD branch");
  Require(tree->SetBranchAddress("ReactionConditions", &reaction) >= 0, "Missing ReactionConditions branch");
  TAsciiFile* config = dynamic_cast<TAsciiFile*>(raw->Get("DetectorConfiguration"));
  Require(config && !config->IsEmpty(), "Missing source DetectorConfiguration");
  config->WriteToFile((outputPath + ".detector").Data());
  const auto groups = ReadGeometry((outputPath + ".detector").Data());
  std::unique_ptr<TFile> output(TFile::Open(outputPath, "RECREATE"));
  Require(output && !output->IsZombie(), "Cannot create traditional output");
  output->cd();
  TTree result("tree", "Successful traditional world vertices and original EventID; mm");
  Long64_t eventID = -1;
  std::array<double, 14> values;
  double reconstructed[3], residual[3];
  result.Branch("EventID", &eventID, "EventID/L");
  const char* axes[3] = {"X", "Y", "Z"};
  for (int i = 0; i < 3; ++i) {
    TString label = TString("Vertex") + axes[i];
    TString prediction = TString("TraditionVertex") + axes[i];
    TString diff = TString("Delta") + axes[i];
    result.Branch(label, &values[11+i], label + "/D");
    result.Branch(prediction, &reconstructed[i], prediction + "/D");
    result.Branch(diff, &residual[i], diff + "/D");
  }
  std::set<Long64_t> writtenIDs;
  Long64_t selected = 0, geometryFailed = 0;
  for (Long64_t entry = 0; entry < tree->GetEntries(); ++entry) {
    Require(tree->GetEntry(entry) > 0 && cluster && recoil && reaction,
            "Cannot read source event: entry=" + std::to_string(entry));
    std::string group;
    double z1 = 0, z2 = 0;
    if (!BuildInput(eventID, values, cluster, recoil, group, z1, z2)) continue;
    if (reaction->GetParticleMultiplicity() != 2) continue;
    int alphaCount = 0, protonCount = 0;
    for (int i = 0; i < 2; ++i) {
      if (reaction->GetParticleName(i) == "alpha") ++alphaCount;
      if (reaction->GetParticleName(i) == "proton") ++protonCount;
    }
    if (alphaCount != 1 || protonCount != 1) continue;
    values[11] = reaction->GetVertexPositionX();
    values[12] = reaction->GetVertexPositionY();
    values[13] = reaction->GetVertexPositionZ();
    if (!Valid(values[11]) || !Valid(values[12]) || !Valid(values[13])) continue;
    ++selected;
    const auto geometry = groups.find(group);
    TVector3 vertex(kInvalidValue, kInvalidValue, kInvalidValue);
    if (geometry == groups.end() || !Reconstruct(values, geometry->second, z1, z2, vertex)) {
      ++geometryFailed;
      continue;
    }
    Require(writtenIDs.insert(eventID).second, "Duplicate successful EventID; cannot align uniquely");
    reconstructed[0] = vertex.X(); reconstructed[1] = vertex.Y(); reconstructed[2] = vertex.Z();
    for (int i = 0; i < 3; ++i) residual[i] = reconstructed[i] - values[11+i];
    result.Fill();
  }
  output->cd(); result.Write();
  std::cout << rawPath << " -> " << outputPath << ": input=" << tree->GetEntries()
            << ", selected=" << selected << ", geometryFailed=" << geometryFailed
            << ", written=" << result.GetEntries() << std::endl;
  // result 为栈对象；在其析构前保持所属 TFile 存活。
}
} // namespace VertexTradition

void Tradition() {
  const TString dir = gSystem->DirName(__FILE__);
  try {
    VertexTradition::Process(dir + "/alphap.root", dir + "/Tradition.root");
    VertexTradition::Process(dir + "/alphap_test.root", dir + "/Tradition_test.root");
  } catch (const std::exception& error) {
    std::cerr << "Tradition stopped: " << error.what() << std::endl;
  }
}
