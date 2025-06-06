#include <iostream>

class Euro;

class USD {
    double m_amount;
public:
    explicit USD(double amount) : m_amount(amount){}

    double value() const{
        return m_amount;
    }

    operator Euro();

};

class Euro{
    double m_amount;
public:
    explicit Euro(double amount) : m_amount(amount){}

    double value() const{
        return m_amount;
    }

    explicit Euro(USD usd); // converting operator, given a USD object how to convert to a Euro

};

USD::operator Euro(){
    return Euro(m_amount / .85);
}

Euro::Euro(USD usd) : m_amount(usd.value() * .85) {}