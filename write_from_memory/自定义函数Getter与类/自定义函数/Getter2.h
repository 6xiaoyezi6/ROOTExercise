
#ifndef GETTER2_H
#define GETTER2_H

class EventData {
private:
    double energy = 150.0;

public:
    double GetEnergy() const {return energy;}//普通成员函数
};


class TTwoBody {
public:
    static double Energy(double kineticEnergy) {return kineticEnergy + 938.272;}//静态成员函数
};

#endif
