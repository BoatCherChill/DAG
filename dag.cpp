#include "dag.h"
#include <map>
#include <queue>

// построение матрицы смежности
vector<vector<int>> adjacency_matrix(const vector<Node>& nodes)
{
    int n = nodes.size();
    vector<vector<int>> adj(n, vector<int>(n, 0));

    // Создаем отображение: ID узла -> индекс в векторе
    map<int, int> idToIndex;
    for (int i = 0; i < n; i++) {
        idToIndex[nodes[i].nodeID] = i;
    }

    // Заполняем матрицу смежности
    for (int i = 0; i < n; i++) {
        for (int prevId : nodes[i].prevNodes) {
            int from = idToIndex[prevId];
            int to = i;
            if (from >= 0 && to >= 0) {
                adj[from][to] = 1;  // Ребро from -> to
            }
        }
    }
    return adj;
}

// поиск цикла
bool dfs_cycle(int v, const vector<vector<int>>& adj, vector<bool>& visited, vector<bool>& recStack)
{
    visited[v] = true;   // Отмечаем вершину как посещенную
    recStack[v] = true;  // Добавляем вершину в стек рекурсии

    // Проверяем всех соседей текущей вершины
    for (int to = 0; to < adj.size(); to++) {
        if (adj[v][to]) {  // Если есть ребро v -> to
            if (!visited[to]) {
                // Рекурсивно обходим непосещенного соседа
                if (dfs_cycle(to, adj, visited, recStack)) {
                    return true;  // Цикл найден
                }
            }
            else if (recStack[to]) {
                return true;  // Вершина уже в стеке рекурсии - цикл!
            }
        }
    }
    recStack[v] = false;  // Убираем вершину из стека рекурсии
    return false;         // Цикл не найден
}

// проверка на ацикличность
bool is_DAG(const vector<Node>& nodes)
{
    int n = nodes.size();
    if (n == 0) return true;  // Пустой граф считается DAG

    vector<vector<int>> adj = adjacency_matrix(nodes);
    vector<bool> visited(n, false);  // Посещенные вершины
    vector<bool> recStack(n, false); // Стек рекурсии

    // Проверяем каждую непосещенную вершину
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            if (dfs_cycle(i, adj, visited, recStack)) {
                return false;  // Найден цикл
            }
        }
    }
    return true;  // Циклов нет - это DAG
}

/**
 * @brief Топологическая сортировка узлов графа (алгоритм Кана)
 * @param nodes Вектор узлов графа
 * @return Вектор ID узлов в топологическом порядке
 *
 * Упорядочивает узлы так, что для каждого ребра u->v, u идет раньше v.
 * Это позволяет выполнять вычисления в правильной последовательности
 */
vector<int> top_sort(const vector<Node>& nodes)
{
    vector<int> result;
    int n = nodes.size();
    if (n == 0) return result;

    vector<vector<int>> adj = adjacency_matrix(nodes);

    // Создаем отображения между индексами и ID
    vector<int> indexToId(n);
    map<int, int> idToIndex;
    for (int i = 0; i < n; i++) {
        indexToId[i] = nodes[i].nodeID;
        idToIndex[nodes[i].nodeID] = i;
    }

    // Вычисляем степень входа для каждой вершины
    vector<int> inDegree(n, 0);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adj[i][j]) {
                inDegree[j]++;  // Увеличиваем степень входа для вершины j
            }
        }
    }

    // Очередь для вершин с нулевой степенью входа
    queue<int> q;
    for (int i = 0; i < n; i++) {
        if (inDegree[i] == 0) {
            q.push(i);
        }
    }

    // Алгоритм Кана: удаляем вершины с нулевой степенью входа
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        result.push_back(indexToId[v]);  // Добавляем вершину в результат

        // Уменьшаем степень входа для всех соседей
        for (int to = 0; to < n; to++) {
            if (adj[v][to]) {
                inDegree[to]--;
                if (inDegree[to] == 0) {
                    q.push(to);  // Добавляем вершину с нулевой степенью входа
                }
            }
        }
    }
    return result;
}
