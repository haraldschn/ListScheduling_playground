#ifndef RESOURCE_SCHEDULING_H
#define RESOURCE_SCHEDULING_H

#include <iostream>
#include <cstdint>

#include <sstream>
#include <string>

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "ResourceDebug.h"

class ResourceGraph {
   private:
    bool debug;

    int type_length = 1;

    struct Node {
        uint64_t parent = 0;  // parent stage (0 if none)
        // ordering asscending by (operands_ready, instr_idx, node_id)
        uint64_t operands_ready = UINT64_MAX;
        uint64_t instr_idx = 0;
        uint64_t id = 0;

        // Enum in CPU specific PerformanceModel used for type identifier
        uint32_t type = 0;
        uint32_t capacity = 1;

        // Latency by default set to a large value (has to be set in any of the later usages)
        uint64_t latency = UINT16_MAX;
        uint64_t t_LR = 0;

        uint32_t children_unfinished = 0;

        std::vector<uint64_t> preds;
        std::vector<uint64_t> succs;
    };

    // this is the structure which implements the
    // operator overloading for Node comparision using std::greater
    struct CompareNodes {
        bool operator()(const Node& a, const Node& b) {
            // operands_ready in ascending order.
            if (a.operands_ready != b.operands_ready) {
                return a.operands_ready > b.operands_ready;  // lower first
            }
            // issue_ready in ascending order.
            if (a.instr_idx != b.instr_idx) {
                return a.instr_idx > b.instr_idx;  // lower second
            }
            // id in ascending order.
            return a.id > b.id;  // lower third
        }
    };

    // this is the structure which implements the
    // operator overloading for Node comparision using std::greater
    struct CompareOpReady {
        bool operator()(const Node& a, const Node& b) {
            // operands_ready in descending order.
            if (a.operands_ready != b.operands_ready) {
                return a.operands_ready < b.operands_ready;  // greater first
            }
            // id in ascending order.
            return a.id > b.id;  // lower second
        }
    };

    std::vector<Node> nodes;
    std::unordered_set<uint64_t> ready_nodes;

    std::vector<std::unordered_set<uint64_t>> U_act;
    std::vector<std::unordered_set<uint64_t>> T_act;
    std::vector<std::unordered_set<uint64_t>> S_act;

    uint64_t t_curr = 1;

   public:
    ResourceGraph(int type_len, bool debug_in = false) {
        type_length = type_len;
        debug = debug_in;

        Node root_node;
        root_node.id = 0;
        root_node.type = 0;
        root_node.latency = 0;
        nodes.push_back(root_node);

        for (int i = 0; i < type_len; i++) {
            std::unordered_set<uint64_t> U_0;
            std::unordered_set<uint64_t> T_0;
            std::unordered_set<uint64_t> S_0;

            U_act.push_back(U_0);
            T_act.push_back(T_0);
            S_act.push_back(S_0);
        }
    };
    ~ResourceGraph() = default;

    uint64_t add_parent_node(int type, uint64_t instr_idx, uint8_t cap = 1) {
        Node node_new;

        node_new.instr_idx = instr_idx;

        node_new.type = type;
        node_new.capacity = cap;

        nodes.push_back(node_new);
        uint64_t id = nodes.size() - 1;
        nodes[id].id = id;

        ready_nodes.insert(id);

        nodes[id].children_unfinished = 1;

        return id;
    }

    uint64_t add_node(int type, uint64_t instr_idx, uint64_t latency = 1, uint8_t cap = 1, uint64_t parent_idx = 0) {
        Node node_new;

        node_new.instr_idx = instr_idx;

        node_new.type = type;
        node_new.latency = latency;
        node_new.capacity = cap;

        nodes.push_back(node_new);
        uint64_t id = nodes.size() - 1;
        nodes[id].id = id;

        if (parent_idx != 0) {
            nodes[id].parent = parent_idx;
            nodes[parent_idx].children_unfinished += 1;
        }

        return id;
    }

    void add_edge(const uint64_t& from, const uint64_t& to) {
        if(from != 0) {
            nodes[to].preds.push_back(from);
            nodes[from].succs.push_back(to);
        } else {
            ready_nodes.insert(to);
        }
    }

    void find_candidate_operations(uint32_t k, uint64_t t) {
        std::vector<uint64_t> ans;
        for (uint64_t id : ready_nodes) {
            auto& v = nodes[id];

            uint64_t t_parent = 1;
            if (v.parent != 0) {
                t_parent = nodes[v.parent].t_LR;
            }

            if (v.type == k && v.t_LR == 0 && t_parent > 0) {
                bool all_finished = true;
                uint64_t max_pred_finished = 0;
                for (uint64_t p : v.preds) {
                    uint64_t t_pred_finish = nodes[p].t_LR + nodes[p].latency;
                    max_pred_finished = std::max(max_pred_finished, t_pred_finish);

                    if (nodes[p].t_LR == 0 || t_pred_finish > t) {
                        all_finished = false;
                    }
                }

                if (all_finished) {
                    U_act[k].insert(v.id);
                    v.operands_ready = max_pred_finished;
                }
            }
        }
    }

    void find_running_operations(uint32_t k, uint64_t t) {
        std::unordered_set<uint64_t> ans;
        std::vector<uint64_t> delete_stack;

        for (uint64_t id : ready_nodes) {
            auto& v = nodes[id];
            if (v.type == k && v.t_LR > 0) {
                uint64_t node_finish = v.t_LR + v.latency;
                if (node_finish > t) {
                    ans.insert(v.id);
                } else {
                    delete_stack.push_back(v.id);
                }
            }
        }

        for (uint64_t id : delete_stack) {
            ready_nodes.erase(id);
        }

        T_act[k] = ans;
    }

    void schedule(uint64_t t_enter, bool finish_schedule = false) {
        nodes[0].t_LR = 1;

        while (t_curr <= t_enter || (finish_schedule && !ready_nodes.empty())) {
            for (int k = 1; k < type_length; k++) {
                find_candidate_operations(k, t_curr);
                find_running_operations(k, t_curr);

                // pq could be represented by using a priority_queue for U_act ?
                std::priority_queue<Node, std::vector<Node>, CompareNodes> pq;
                for (auto& u : U_act[k]) {
                    pq.push(nodes[u]);
                }

                // TODO: use S_act in next iteration ??
                S_act[k].clear();
                while (!pq.empty() && (S_act[k].size() + T_act[k].size() < pq.top().capacity)) {
                    int id_max = pq.top().id;
                    pq.pop();

                    S_act[k].insert(id_max);
                    nodes[id_max].t_LR = t_curr;

                    // Add succesors to ready nodes
                    for (uint64_t s : nodes[id_max].succs) {
                        ready_nodes.insert(s);
                    }

                    if (nodes[id_max].parent != 0) {
                        uint64_t parent_id = nodes[id_max].parent;
                        nodes[parent_id].children_unfinished -= 1;

                        if (nodes[parent_id].children_unfinished == 1) {
                            nodes[parent_id].latency = t_curr - nodes[parent_id].t_LR;
                        }
                    }
                }

                if (debug && (!U_act[k].empty() || !T_act[k].empty() || !S_act[k].empty())) {
                    std::cout << "t:" << t_curr << "\n";
                    std::cout << "k:" << k << "\n";
                    std::cout << "\t\tU_act:";
                    print_set(U_act[k]);
                    std::cout << "T_act:";
                    print_set(T_act[k]);
                    std::cout << "S_act:";
                    print_set(S_act[k]);
                    std::cout << "\n";
                }

                for (auto it = S_act[k].begin(); it != S_act[k].end(); ++it) {
                    U_act[k].erase(*it);
                }
            }

            t_curr += 1;
        }
    }

    /// helper functions for std output

    size_t get_nodes_len() {
        return nodes.size();
    }

    std::string get_priority_str(uint64_t id) {
        std::stringstream ret_strs;

        ret_strs << "(";
        ret_strs << nodes[id].operands_ready;
        ret_strs << ",";
        ret_strs << nodes[id].instr_idx;
        ret_strs << ",";
        ret_strs << nodes[id].id;
        ret_strs << ")";

        return ret_strs.str();
    }
};

#endif