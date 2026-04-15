#ifndef DAG_H
#define DAG_H

#include "graph.h"
#include <vector>

using namespace std;

// Создание матрицы смежности из списка узлов
vector<vector<int>> adjacency_matrix(const vector<Node>& nodes);

// Проверка, является ли граф ациклическим (DAG)
bool is_DAG(const vector<Node>& nodes);

// Топологическая сортировка узлов графа
vector<int> top_sort(const vector<Node>& nodes);

#endif
