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

    public:

    vector<account> accounts;
        auto create_account(int min_amount) {
            accounts.emplace_back();
            auto item = accounts.rbegin();
            item->deposit(min_amount);
            return accounts.size() - 1;
        }

        void transfer(auto from, auto to, int amount) {
		lock(accounts[from].get_mutex(), accounts[to].get_mutex());
		lock_guard<mutex> from_lock(accounts[from].get_mutex(), adopt_lock);
		lock_guard<mutex> to_lock(accounts[to].get_mutex(), adopt_lock);

            if (accounts[from].balance() >= amount) {
		    assert_positive();
                accounts[from].deposit(-amount);
                accounts[to].deposit(amount);
		assert_positive();
            }
        }

	void assert_positive() const {
		for (auto &acc : accounts)
			if (acc.balance() < 0 )
				throw invalid_argument("Negative balance!");
	}
};


auto trans(Bank& bank, auto from, auto to, int amount) {
    return async(launch::async, [&, from, to] (int amount) {
        bank.transfer(from, to, amount);
    }, amount);
}

int main() {

    try {
        Bank bank;

        auto one = bank.create_account(100);
        auto two = bank.create_account(200);

        vector<future<void>> futures;

        for (int i = 0 ; i < 1000 ; i++ ) {
            futures.emplace_back(trans(bank, one, two, 100));
            futures.emplace_back(trans(bank, two, one, 100));
        }

        for (auto & f : futures) {
            f.get();
        }


        cout << "one balance: " << bank.accounts[one].balance() << endl;
        cout << "two balance: " << bank.accounts[two].balance() << endl;

    } catch (exception &err) {
        cerr << err.what() << endl;
    }
}
