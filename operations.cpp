#include "operations.h"
#include "dag.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <QString>

string toString(double d) {
    stringstream ss;
    ss << d;
    return ss.str();
}

vector<double> extract_nums(const string& data) {
    vector<double> numbers;
    stringstream ss(data);
    double num;
    while (ss >> num) {
        numbers.push_back(num);
    }
    return numbers;
}

vector<string> extract_strings(const string& data) {
    vector<string> strings;
    stringstream ss(data);
    string token;
    while (ss >> token) {
        strings.push_back(token);
    }
    return strings;
}

string sum(const string& data) {
    double result = 0;
    vector<double> nums = extract_nums(data);
    for (double num : nums) result += num;
    return toString(result);
}

string average(const string& data) {
    double sumVal = 0;
    int count = 0;
    vector<double> nums = extract_nums(data);
    for (double num : nums) {
        sumVal += num;
        count++;
    }
    return count != 0 ? toString(sumVal / count) : "0";
}

string median(const string& data) {
    vector<double> nums = extract_nums(data);
    if (nums.empty()) return "0";
    sort(nums.begin(), nums.end());
    size_t n = nums.size();
    if (n % 2 == 0) {
        return toString((nums[n / 2 - 1] + nums[n / 2]) / 2.0);
    }
    else {
        return toString(nums[n / 2]);
    }
}

string concat(const string& data) {
    // Объединяем все строки без пробелов
    stringstream ss(data);
    string token, result;
    while (ss >> token) {
        result += token;
    }
    return result;
}

string toUpper(const string& data) {
    string result;
    stringstream ss(data);
    string token;
    bool first = true;
    while (ss >> token) {
        if (!first) result += " ";
        first = false;
        for (char& c : token) {
            result += toupper(c);
        }
    }
    return result;
}

string toLower(const string& data) {
    string result;
    stringstream ss(data);
    string token;
    bool first = true;
    while (ss >> token) {
        if (!first) result += " ";
        first = false;
        for (char& c : token) {
            result += tolower(c);
        }
    }
    return result;
}
