#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>

class DeadlockDetector {
private:
    std::unordered_map<int, std::vector<int>> wait_for_graph;
    std::mutex graph_mutex;
    bool running = true;

    bool dfs(int node, std::unordered_set<int>& visited, std::unordered_set<int>& recStack) {
        visited.insert(node);
        recStack.insert(node);

        for (int neighbor : wait_for_graph[node]) {
            if (visited.find(neighbor) == visited.end() && dfs(neighbor, visited, recStack))
                return true;
            else if (recStack.find(neighbor) != recStack.end())
                return true;
        }

        recStack.erase(node);
        return false;
    }

public:
    void addDependency(int waiting_thread, int holding_thread) {
        std::lock_guard<std::mutex> lock(graph_mutex);
        wait_for_graph[waiting_thread].push_back(holding_thread);
    }

    void removeDependency(int waiting_thread) {
        std::lock_guard<std::mutex> lock(graph_mutex);
        wait_for_graph.erase(waiting_thread);
    }

    void daemonLoop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::lock_guard<std::mutex> lock(graph_mutex);

            std::unordered_set<int> visited;
            std::unordered_set<int> recStack;

            for (auto const& pair : wait_for_graph) {
                int thread_id = pair.first;
                if (visited.find(thread_id) == visited.end()) {
                    if (dfs(thread_id, visited, recStack)) {
                        std::cerr << "[DEADLOCK DAEMON] ALERT: Deadlock detected!\n";
                        wait_for_graph.clear();
                        break;
                    }
                }
            }
        }
    }

    void stop() {
        running = false;
    }
};
