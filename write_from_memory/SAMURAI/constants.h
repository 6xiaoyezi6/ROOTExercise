#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Rtypes.h>

namespace tof_constants {
    //constexpr const char* input_file = "20260531_2218/hSBT1258.root";
    // energy units are MeV
    constexpr Double_t amu = 931.494095;

    // Z = 0
    constexpr Double_t m_n = 939.5654133; // neutron

    // Z = 1
    constexpr Int_t Z1 = 1 ;
    constexpr Int_t A_3H = 3 ;
    constexpr Double_t m_1H = 1.00728 * amu; // proton
    constexpr Double_t m_2H = 2.01355 * amu; // deuteron
    constexpr Double_t m_3H = 3.0155 * amu; // triton
    constexpr Double_t m_4H = 4.02588 * amu; // 4H
    constexpr Double_t m_5H = 5.03476 * amu; // 5H
    constexpr Double_t m_6H = 6.04441 * amu; // 6H
    constexpr Double_t m_7H = 7.0522 * amu; // 7H

    // Z = 2
    constexpr Int_t Z2 = 2 ;
    constexpr Int_t A_8He = 8 ;
    constexpr Int_t A_4He = 4 ;
    constexpr Int_t A_6He = 6 ;
    constexpr Double_t m_3He = 3.01493 * amu; // 3He
    constexpr Double_t m_4He = 4.00151 * amu; // alpha
    constexpr Double_t Ex1_4He = 20.210; // 1st excited state
    constexpr Double_t m_6He = 6.01779 * amu; // 6He
    constexpr Double_t m_8He = 8.03284 * amu; // 8He

    //光速，以及F7-F13距离
    constexpr Double_t length_f7f13 = 36014.1; //mm 
    constexpr Double_t velocity_light = 299.792458; // mm/ns

    //磁刚度
    constexpr Double_t Brho_D6_run1258_8He = 8.7413; // T*m
    constexpr Double_t Brho_D6_run1451_3H = 6.5371; // T*m
    constexpr Double_t Brho_D6_run1386_6He = 6.5596; // T*m
    constexpr Double_t Brho_D6_run1367_4He = 4.3886; // T*m
    constexpr Double_t Brho_D6_run1453_3H = 6.5371; // T*m

    



   
}

#endif // CONSTANTS_H
