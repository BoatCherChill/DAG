#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <string>
#include <vector>
#include "graph.h"

using namespace std;


// Преобразование числа в строку
string toString(double d);

// Извлечение всех чисел из строки (разделитель - пробел)
vector<double> extract_nums(const string& data);

// Математические операции
string sum(const string& data);      // Сумма всех чисел
string average(const string& data);  // Среднее арифметическое
string median(const string& data);   // Медиана

// Строковые операции
string concat(const string& data);   // Конкатенация (склеивание) строк
string toUpper(const string& data);  // Преобразование в верхний регистр
string toLower(const string& data);  // Преобразование в нижний регистр

#endif
