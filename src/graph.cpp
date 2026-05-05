#include "graph.h"

// функция заполнения матрицы смежности
vector<vector<int>> Graph::adjacencyMatrix() const {
    int n = nodes.size();
    vector<vector<int>> adj(n, vector<int>(n, 0)); // Матрица смежности n x n
    map<int, int> idToIndex;  // словарь id узла к индексу в векторе
    // заполнить словарь
    for (int i = 0; i < n; i++)
        idToIndex[nodes[i].nodeID] = i;
    // заполнить матрицу смежности
    for (int i = 0; i < n; i++) {
        for (int prevId : nodes[i].prev_nodes) {
            int from = idToIndex[prevId];
            int to = i;
            adj[from][to] = 1; // поставить 1, если связь есть
        }
    }
    return adj;
}

// функция проверки циклов от текущего узла
bool Graph::hasCycle(int v, vector<bool>& visited, vector<bool>& recStack) const {
    visited[v] = true; // отметить вершину как посещенную
    recStack[v] = true; // добавить в стек рекурсии

    vector<vector<int>> adj = adjacencyMatrix();
    for (size_t to = 0; to < adj[v].size(); to++) {
        if (adj[v][to]) {
            if (!visited[to] && hasCycle(to, visited, recStack))
                return true; //
            else if (recStack[to])
                return true;
        }
    }
    recStack[v] = false;
    return false;
}

// функция проверки циклов в графе
bool Graph::isDAG() const {
    int n = nodes.size();
    if (n == 0)
        return true;  // Пустой граф считается DAG
    vector<bool> visited(n, false);
    vector<bool> recStack(n, false);
    for (int i = 0; i < n; i++) {
        if (!visited[i] && hasCycle(i, visited, recStack))
            return false;
    }
    return true;
}

// топологическая сортировка
vector<int> Graph::topologicalSort() {
    vector<int> result;
    int n = nodes.size();
    if (n == 0) return result;
    vector<vector<int>> adj = adjacencyMatrix();
    vector<int> indexToId(n);
    map<int, int> idToIndex;
    for (int i = 0; i < n; i++) {
        indexToId[i] = nodes[i].nodeID;
        idToIndex[nodes[i].nodeID] = i;
    }
    vector<int> inDegree(n, 0); // Степень входа для каждой вершины
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            if (adj[i][j])
                inDegree[j]++;
        }
    }
    queue<int> q;
    for (int i = 0; i < n; i++) if (inDegree[i] == 0) q.push(i); // Вершины с нулевой степенью входа
    while (!q.empty()) {
        int v = q.front(); q.pop();
        result.push_back(indexToId[v]); // Добавляем вершину в результат
        for (int to = 0; to < n; to++) {
            if (adj[v][to]) {
                inDegree[to]--;
                if (inDegree[to] == 0) q.push(to);
            }
        }
    }
    return result;
}

// функция добавления узла в логический граф
void Graph::addNode(int x, int y, NodeType type) {
    Node newNode;
    newNode.nodeID = next_NodeID++;
    newNode.coordinates = { x, y };
    newNode.type = type;
    newNode.calculated = false;
    nodes.push_back(newNode);
}

// функция поиска узла по ID
Node* Graph::findNodeById(int id) {
    for (auto& node : nodes){
        if (node.nodeID == id)
            return &node;
    }
    return nullptr;
}

// функция удаления узла
void Graph::deleteNode(int id) {
    // Удалить все связи, где узел является предком
    for (auto& node : nodes) {
        if (node.nodeID != id) {
            node.prev_nodes.erase(
                std::remove(node.prev_nodes.begin(), node.prev_nodes.end(), id),
                node.prev_nodes.end()
            );
        }
    }

    // Удалить узел
    nodes.erase(
        std::remove_if(nodes.begin(), nodes.end(),
            [id](const Node& n) { return n.nodeID == id; }),
        nodes.end()
    );
}

// функция добавления связи
void Graph::addRelation(int fromId, int toId) {
    Node* toNode = findNodeById(toId);
    if (!toNode) return;
    for (int prev : toNode->prev_nodes) {
        if (prev == fromId)
            return; // Связь уже существует
    }
    toNode->prev_nodes.push_back(fromId);// добавить связь
}

// функция удаления связи
void Graph::deleteRelation(int fromId, int toId) {
    Node* toNode = findNodeById(toId);
    if (!toNode)
        return;
    auto& prevNodes = toNode->prev_nodes;
    auto it = find(prevNodes.begin(), prevNodes.end(), fromId);
    if (it != prevNodes.end())
        prevNodes.erase(it); // удалить связь
}

// функция вычисления суммы набора данных
string Graph::sum(const string& data) const {
    double result = 0;
    stringstream ss(data);
    double num;
    while (ss >> num)
        result += num;
    string str = to_string(result);
    str.erase(str.find_last_not_of('0') + 1, string::npos); // Удаление лишних нулей
    if (str.back() == '.')
        str.pop_back();
    return str;
}

// функция вычисления среднего значения набора данных
string Graph::average(const string& data) const {
    double sumVal = 0;
    int count = 0;
    stringstream ss(data);
    double num;
    while (ss >> num) {
        sumVal += num;
        count++;
    }
    if (count == 0)
        return "0";
    double avg = sumVal / count;
    string str = to_string(avg);
    str.erase(str.find_last_not_of('0') + 1, string::npos);
    if (str.back() == '.')
        str.pop_back();
    return str;
}

// функция вычисления медианы набора данных
string Graph::median(const string& data) const {
    vector<double> nums;
    stringstream ss(data);
    double num;
    while (ss >> num)
        nums.push_back(num);
    if (nums.empty())
        return "0";
    sort(nums.begin(), nums.end());
    size_t n = nums.size();
    double median;
    if (n % 2 == 0)
        median = (nums[n/2 - 1] + nums[n/2]) / 2.0; // для четного количества
    else
        median = nums[n/2]; // для нечетного количества
    string str = to_string(median);
    str.erase(str.find_last_not_of('0') + 1, string::npos);
    if (str.back() == '.') str.pop_back();
    return str;
}

// функция привдеения текста к верхнему регистру
string Graph::toUpper(const string& data) const {
    QString qstr = QString::fromStdString(data);
    QStringList parts = qstr.split(' ', Qt::SkipEmptyParts);
    QString result;
    for (int i = 0; i < parts.size(); ++i) {
        if (i > 0)
            result += " ";
        result += parts[i].toUpper();
    }
    return result.toStdString();
}

// функция привдеения текста к нижнему регистру
string Graph::toLower(const string& data) const {
    QString qstr = QString::fromStdString(data);
    QStringList parts = qstr.split(' ', Qt::SkipEmptyParts);
    QString result;
    for (int i = 0; i < parts.size(); ++i) {
        if (i > 0) result += " ";
        result += parts[i].toLower();
    }
    return result.toStdString();
}

// функция запуска вычислений
void Graph::execute() {
    if (!isDAG()) return; // проверить на ацикличность
    vector<int> order = topologicalSort(); // топологическая сортировка
    for (int nodeId : order) {
        Node* node = findNodeById(nodeId);
        if (!node) continue;

        // собрать данные от всех предков
        string allData;
        for (int prevId : node->prev_nodes) {
            Node* prev = findNodeById(prevId);
            if (prev && prev->calculated) {
                if (!allData.empty())
                    allData += " ";
                allData += prev->result;
            }
        }

        if (node->type != NodeType::INPUT && node->prev_nodes.empty()) continue; // Узел без входов

        // выполнить  операции в зависимости от типа узла
        switch (node->type) {
            case NodeType::INPUT:
                node->calculated = true;
                break;
            case NodeType::OUTPUT:
                node->result = allData;
                node->calculated = true;
                break;
            case NodeType::SUM:
                node->result = sum(allData);
                node->calculated = true;
                break;
            case NodeType::MEDIAN:
                node->result = median(allData);
                node->calculated = true;
                break;
            case NodeType::AVERAGE:
                node->result = average(allData);
                node->calculated = true;
                break;
            case NodeType::TO_UPPER:
                node->result = toUpper(allData);
                node->calculated = true;
                break;
            case NodeType::TO_LOWER:
                node->result = toLower(allData);
                node->calculated = true;
                break;
        }
    }
}
