#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <queue>
#include <QString>
#include <QStringList>

using namespace std;

// Типы узлов
enum class NodeType {
    INPUT = 1,
    OUTPUT = 2,
    SUM = 3,
    MEDIAN = 4,
    AVERAGE = 5,
    TO_UPPER = 6,
    TO_LOWER = 7
};

// Структура узла
struct Node {
    int nodeID;
    pair<int, int> coordinates; // кооржинаты узла на сцене
    NodeType type; // тип узла
    vector<int> prev_nodes; // список ID узлов предков
    string result; // результат вычисления узла
    bool calculated; // флаг проведения вычислений
    Node() : nodeID(0), type(NodeType::INPUT), calculated(false) {}
};

// Класс графа
class Graph {
private:
    vector<Node> nodes; // список всех узлов графа
    int next_NodeID; // следующий свободный ID

    vector<vector<int>> adjacencyMatrix() const; // функция постоения матрицы смежности
    bool hasCycle(int v, vector<bool>& visited, vector<bool>& recStack) const; // функция проверки циклов от определенного узла

public:
    Graph() : next_NodeID(0) {}

    void addNode(int x, int y, NodeType type); // функция добавления узла
    Node* findNodeById(int id); // функция поиска узла по ID
    void deleteNode(int id); // функция удаления узла
    void addRelation(int fromId, int toId); // функция добавления связи между узлами
    void deleteRelation(int fromId, int toId); // функция удаления связи между узлами
    bool isDAG() const; // функция проверки графа на ацикличность
    vector<int> topologicalSort(); // функция топологической сортировки
    void execute(); // функция вычисления графа
    int getNextId() const { return next_NodeID; }

    // Операции
    string sum(const string& data) const;
    string average(const string& data) const;
    string median(const string& data) const;
    string toUpper(const string& data) const;
    string toLower(const string& data) const;
};
