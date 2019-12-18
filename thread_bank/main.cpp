#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <functional>
#include <vector>


using namespace std;


class account {
    private:
        int _balance = 0;
        mutable mutex mtex;

    public:

        account() noexcept = default;

        account(account&& acc)
            : _balance(std::move(acc._balance))
        {
            // do we need to think about the mutex here?
        }

        void deposit(int amount) {
            lock_guard<mutex> locked(mtex);
            _balance += amount;
        }

        int balance() const noexcept {
            lock_guard<mutex> locked(mtex);
            return _balance;
        }

        mutex& get_mutex() { return mtex; }

};

class Bank {
    vector<account> accounts;

    public:

        auto create_account(int min_amount) {
            accounts.emplace_back();
            auto item = accounts.rbegin();
            item->deposit(min_amount);
            return item;
        }

        void transfer(auto from, auto to, int amount) {
            if (from->balance() >= amount) {
                from->deposit(-amount);
                to->deposit(amount);
            }
        }
};


auto trans(Bank& bank, auto from, auto to, int amount) {
    return async(launch::async, [&] (int amount) {
        bank.transfer(from, to, amount);
    }, amount);
}

int main() {

    try {
        Bank bank;

        auto one = bank.create_account(100);
        auto two = bank.create_account(200);

        vector<future<void>> futures;

        for (int i = 0 ; i < 100 ; i++ ) {
            futures.emplace_back(trans(bank, one, two, 100));
            futures.emplace_back(trans(bank, two, one, 100));
        }

        for (auto & f : futures) {
            f.wait();
        }


        cout << "one balance: " << one->balance() << endl;
        cout << "two balance: " << two->balance() << endl;

    } catch (exception &err) {
        cerr << err.what() << endl;
    }
}
