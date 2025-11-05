#ifndef RV32_DEPGRAPH_H
#define RV32_DEPGRAPH_H

#include <cstdint>
#include <vector>
#include <string>
#include <queue>
#include <utility>
#include <unordered_map>
#include <unordered_set>

#include "helpers.h"

namespace rv32_ooo {

enum class Edge_Type {
    EMPTY,
    RAW,
    WAR,
    WAW
};

enum Node_Type {
    MULT,
    ALU,
    SOURCE,
    SINK,
};

const size_t a_k[] = {2, 1};

class DependencyGraph {
   private:
    struct Edge {
        uint64_t from, to;
        Edge_Type type;
    };

    struct Node {
        std::string label;
        Node_Type type;
        uint64_t latency = 1;
        uint64_t priority = 0;
        uint64_t t_LR = 0;

        std::vector<uint64_t> predc;
    };

    std::unordered_map<uint64_t, Node> nodes;
    std::vector<Edge> edges;

    std::unordered_set<uint64_t> sink_nodes;

   public:
    DependencyGraph() {
        nodes[0].label = "V0";
        nodes[0].type = Node_Type::SOURCE;
        nodes[0].latency = 0;
    };
    ~DependencyGraph() = default;

    void add_node(uint64_t id, const std::string& label, uint64_t latency = 1) {
        nodes[id].label = label;
        if (label.find("MUL") != std::string::npos) {
            nodes[id].type = Node_Type::MULT;
        } else {
            nodes[id].type = Node_Type::ALU;
        }
        nodes[id].latency = latency;

        sink_nodes.insert(id);
    }

    void add_edge_RAW(const uint64_t& from, const uint64_t& to) {
        edges.push_back({from, to, Edge_Type::RAW});
        nodes[to].predc.push_back(from);

        sink_nodes.erase(from);
    }
    void add_edge_WAR(const uint64_t& from, const uint64_t& to) {
        edges.push_back({from, to, Edge_Type::WAR});
        nodes[to].predc.push_back(from);

        sink_nodes.erase(from);
    }
    void add_edge_WAW(const uint64_t& from, const uint64_t& to) {
        edges.push_back({from, to, Edge_Type::WAW});
        nodes[to].predc.push_back(from);

        sink_nodes.erase(from);
    }

    void update_priorities() {
        // Reset all priorities
        for (auto& x : nodes) {
            x.second.priority = 0;
        }

        for (int i = edges.size() - 1; i >= 0; --i) {
            uint64_t dest = edges[i].to;
            uint64_t src = edges[i].from;
            if (nodes[dest].priority == 0) {
                nodes[dest].priority = nodes[dest].latency;
            }
            nodes[src].priority = nodes[dest].priority + nodes[src].latency;
        }
    }

    uint64_t get_priority(uint64_t id) {
        return nodes[id].priority;
    }

    std::vector<uint64_t> find_candidate_operations(int k, uint64_t t_act) {
        std::vector<uint64_t> ans;
        for (auto& v : nodes) {
            if (v.second.type == k && v.second.t_LR == 0) {
                bool all_finished = true;
                for (uint64_t p : v.second.predc) {
                    uint64_t t_pred_finish = nodes[p].t_LR + nodes[p].latency;
                    if (nodes[p].t_LR == 0 || t_pred_finish > t_act) {
                        all_finished = false;
                    }
                }

                if (all_finished) {
                    ans.push_back(v.first);
                }
            }
        }
        return ans;
    }

    std::vector<uint64_t> find_running_operations(int k, uint64_t t_act) {
        std::vector<uint64_t> ans;
        for (auto& v : nodes) {
            if (v.second.type == k && v.second.t_LR > 0) {
                uint64_t node_finish = v.second.t_LR + v.second.latency;
                if (node_finish > t_act) {
                    ans.push_back(v.first);
                }
            }
        }
        return ans;
    }

    bool sink_nodes_set() {
        bool all_set = true;
        for (auto& v : sink_nodes) {
            all_set &= (nodes[v].t_LR != 0);
        }
        return all_set;
    }

    void schedule() {
        // Connect all unconnected nodes to root node
        for (auto& x : nodes) {
            if (x.first != 0 && x.second.predc.empty()) {
                add_edge_RAW(0,x.first);
            }
        }

        nodes[0].t_LR = 1;
        uint64_t t_act = 1;

        while (!sink_nodes_set()) {
            std::cout << "t_act:" << t_act << "\n";
            for (int k = 0; k < 2; k++) {
                std::vector<uint64_t> U_act = find_candidate_operations(k, t_act);
                std::vector<uint64_t> T_act = find_running_operations(k, t_act);

                // pq could be represented by using a priority_queue for U_act ?
                std::priority_queue<std::pair<uint64_t, uint64_t>, std::vector<std::pair<uint64_t, uint64_t>>, ComparePairs> pq;
                for (auto& u : U_act) {
                    Node& v = nodes[u];
                    pq.push({v.priority, u});
                }

                std::vector<uint64_t> S_act;

                while (!pq.empty() && (S_act.size() + T_act.size() < a_k[k])) {
                    int id_max = pq.top().second;
                    pq.pop();

                    S_act.push_back(id_max);
                    nodes[id_max].t_LR = t_act;
                }

                std::cout << "k: " << k;
                std::cout << "\tU_act:";
                print_vec(U_act);
                std::cout << "T_act:";
                print_vec(T_act);
                std::cout << "S_act:";
                print_vec(S_act);
                std::cout << "\n";
            }
            t_act += 1;
            std::cout << "\n";
        }
    }
};

}  // namespace rv32_ooo

#endif  // RV32_DEPGRAPH_H