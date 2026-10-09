#include "User.hpp"

User::User(std::string name, double initialBalance)
    : name(std::move(name)), balance(initialBalance < 0 ? 0 : initialBalance) {}

bool User::deductBalance(double amount) {
    if(amount < 0 || amount > balance){
        return false;
    }
    balance -= amount;
    return true;
}

void User::topUp(double amount) {
    if(amount > 0){
        balance += amount;
    }
}

double User::getBalance() const { return balance; }

const std::string& User::getName() const { return name;}