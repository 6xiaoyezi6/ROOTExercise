#include "Rtypes.h"
#include "TString.h"

#include <iostream>

// ============================================================
// 父类：粒子
// ============================================================
class Particle {
protected:
    TString  Name;
    Double_t Mass;
    Double_t Energy;

public:
    // 构造函数
    Particle(const TString& particleName,Double_t particleMass,Double_t particleEnergy)
        : Name(particleName),Mass(particleMass),Energy(particleEnergy)// 初始化列表形式的构造函数
  // name(particleName)左边 name：类的成员变量 右边 particleName：传入的参数
    {
        std::cout << "调用 Particle 构造函数："<< Name << std::endl;
    }

    // 输出粒子信息
    void PrintInfo() const
    {
        std::cout << "名称：" << Name << std::endl;
        std::cout << "质量：" << Mass<< " MeV/c^2" << std::endl;
        std::cout << "动能：" << Energy<< " MeV" << std::endl;
    }

    // 设置动能
    void SetEnergy(Double_t newEnergy)
    {
        if (newEnergy >= 0.0) {
            Energy = newEnergy;
        }
    }

    // 获取动能
    Double_t GetEnergy() const
    {
        return Energy;
    }
};

// ============================================================
// 子类：质子
// ============================================================
class Proton : public Particle {
private:
    Int_t Charge;

public:
    Proton(Double_t protonEnergy)
        : Particle("Proton", 938.272, protonEnergy),Charge(1)
    {
        std::cout << "调用 Proton 构造函数"<< std::endl;
    }

    void PrintCharge() const
    {
        std::cout << "电荷数："<< Charge << std::endl;
    }
};

// ============================================================
// 子类：中子
// ============================================================
class Neutron : public Particle {
private:
    Int_t Charge;

public:
    Neutron(Double_t neutronEnergy)
        : Particle("Neutron", 939.565, neutronEnergy),Charge(0)
    {
        std::cout << "调用 Neutron 构造函数"<< std::endl;
    }

    void PrintCharge() const
    {
        std::cout << "电荷数："<< Charge << std::endl;
    }
};

// ============================================================
// ROOT 宏入口函数
// 文件名应为 ParticleExample.C
// ============================================================
void ParticleExample()
{
    std::cout << "===== 创建质子对象 ====="
              << std::endl;

    Proton proton(150.0);//建一个名为 proton 的 Proton 对象，并将初始动能设置为 150.0 MeV

    proton.PrintInfo();
    proton.PrintCharge();

    proton.SetEnergy(200.0);//调用 proton 对象的 SetEnergy() 函数，把质子的动能从 150 MeV 修改成 200 MeV

    std::cout << "修改后的质子动能："<< proton.GetEnergy()<< " MeV" << std::endl;

    std::cout << "\n===== 创建中子对象 ====="
              << std::endl;

    Neutron neutron(80.0);

    neutron.PrintInfo();
    neutron.PrintCharge();
}