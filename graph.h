#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <queue>
#include <cctype>
#include <QString>
#include <QStringList>

using namespace std;

// Типы узлов
enum class NodeType { INPUT = 1, OUTPUT = 2, SUM = 3, MEDIAN = 4, AVERAGE = 5, TO_UPPER = 6, TO_LOWER = 7 };

// Структура узла
struct Node {
    int nodeID;
    pair<int, int> coordinates;
    NodeType type;
    vector<int> prev_nodes;
    string result;
    bool calculated;
    Node() : nodeID(0), type(NodeType::INPUT), calculated(false) {}
};

// Класс графа
class Graph {
private:
    vector<Node> nodes;
    int next_NodeID;

    vector<vector<int>> adjacencyMatrix() const;
    bool hasCycle(int v, vector<bool>& visited, vector<bool>& recStack) const;

public:
    Graph() : next_NodeID(0) {}

    void addNode(int x, int y, NodeType type);
    Node* findNodeById(int id);
    void deleteNode(int id);
    void addRelation(int fromId, int toId);
    void deleteRelation(int fromId, int toId);
    bool isDAG() const;
    vector<int> topologicalSort();
    void execute();
    int getNextId() const { return next_NodeID; }

    // Операции
    string sum(const string& data) const;
    string average(const string& data) const;
    string median(const string& data) const;
    string toUpper(const string& data) const;
    string toLower(const string& data) const;
};

#endif
