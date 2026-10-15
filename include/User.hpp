#pragma once
#include <string>

class User {
private:
    std::string name;
    double balance;

public:
    // constructor
    User(std::string name, double intitialBalance);

    // methods
    bool deductBalance(double amount); // validasi apakah saldo cukup sebelum berkurang
    void topUp(double amount); // jumlah <= 0 diabaikan
    double getBalance() const;
    const std::string& getName() const;
};