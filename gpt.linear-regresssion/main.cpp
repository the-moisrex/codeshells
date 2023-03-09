#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

constexpr double mean(const vector<double>& arr) {
    double sum = 0;
    for (int i = 0; i < arr.size(); i++) {
        sum += arr[i];
    }
    return sum / arr.size();
}

double slope(const vector<double>& x, const vector<double>& y) {
    double x_mean = mean(x);
    double y_mean = mean(y);
    double num = 0, den = 0;
    for (int i = 0; i < x.size(); i++) {
        num += (x[i] - x_mean) * (y[i] - y_mean);
        den += pow(x[i] - x_mean, 2);
    }
    return num / den;
}

double intercept(const vector<double>& x, const vector<double>& y) {
    double x_mean = mean(x);
    double y_mean = mean(y);
    return y_mean - (slope(x, y) * x_mean);
}

double predict(double x, double slope, double intercept) {
    return slope * x + intercept;
}


constexpr int NUM_VALUES = 1000;

vector<double> generate_x_values(int num_values) {
    vector<double> x(num_values);
    for (int i = 0; i < num_values; i++) {
        x[i] = i;
    }
    return x;
}

vector<double> generate_y_values(const vector<double>& x) {
    vector<double> y(x.size());
    for (int i = 0; i < x.size(); i++) {
        y[i] = x[i] * 2;
    }
    return y;
}


int main() {
    vector<double> x = generate_x_values(NUM_VALUES);
    vector<double> y = generate_y_values(x);

    double b = intercept(x, y);
    double m = slope(x, y);

    cout << "Intercept: " << b << endl;
    cout << "Slope: " << m << endl;

    // Get the x value to predict from user input
    double x_to_predict;
    cout << "Enter an x value to predict: ";
    cin >> x_to_predict;

    double y_pred = predict(x_to_predict, m, b);
    cout << "Predicted value for x = " << x_to_predict << ": " << y_pred << endl;

    return 0;
}

