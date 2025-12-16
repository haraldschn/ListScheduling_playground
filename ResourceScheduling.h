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

    void print_set(std::unordered_set<uint64_t>& input) {
        std::cout << "{";

        if (input.empty()) {
            std::cout << "}\t";
            return;
        }

        for (auto it = input.begin(); it != input.end();) {
            std::cout << *it;
            if (++it != input.end()) {
                std::cout << ", ";
            }
        }

        std::cout << "}\t";
    };

    int type_length = 1;
    struct Node {
        uint64_t parent = 0;  // parent stage (0 if none)
        // ordering asscending by (operands_ready, node_id)
        uint64_t operands_ready = UINT64_MAX;
        uint64_t id = 0;

        // Enum in CPU specific PerformanceModel used for type identifier
        uint32_t type = 0;
        uint32_t capacity = 1;

        // Latency by default set to a large value (has to be set in any of the later usages)
        uint64_t latency = UINT16_MAX;
        uint64_t t_LR = 0;

        int children_unfinished = -1;

        std::vector<uint64_t> preds;
        std::vector<uint64_t> succs;

        std::vector<uint64_t> exit_cond;
    };

    // this is the structure which implements the
    // operator overloading for Node comparision using std::greater
    struct CompareNodes {
        bool operator()(const Node& a, const Node& b) {
            // operands_ready in ascending order.
            if (a.operands_ready != b.operands_ready) {
                return a.operands_ready > b.operands_ready;  // lower first
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

    uint64_t add_parent_node(int type, uint8_t cap = 1) {
        Node node_new;

        node_new.type = type;
        node_new.capacity = cap;

        nodes.push_back(node_new);
        uint64_t id = nodes.size() - 1;
        nodes[id].id = id;

        nodes[id].children_unfinished = 0;

        return id;
    }

    uint64_t add_node(int type, uint64_t latency = 1, uint8_t cap = 1, uint64_t parent_idx = 0) {
        Node node_new;

        node_new.type = type;
        node_new.latency = latency;
        node_new.capacity = cap;

        nodes.push_back(node_new);
        uint64_t id = nodes.size() - 1;
        nodes[id].id = id;

        if (parent_idx != 0) {
            nodes[id].parent = parent_idx;
            nodes[parent_idx].children_unfinished += 1;

            for (uint64_t p : nodes[parent_idx].preds) {
                add_edge(p, id);
            }

            if (nodes[parent_idx].preds.empty()) {
                ready_nodes.insert(id);
            }
        }

        return id;
    }

    void add_edge(const uint64_t& from, const uint64_t& to) {
        if (from != 0) {
            nodes[to].preds.push_back(from);
            nodes[from].succs.push_back(to);
        } else {
            ready_nodes.insert(to);
        }
    }

    void add_exit_cond(const uint64_t& id, const uint64_t& cond) {
        nodes[id].exit_cond.push_back(cond);
    }

    void find_candidate_operations(uint64_t t) {
        std::vector<uint64_t> delete_stack;

        for (uint64_t id : ready_nodes) {
            bool all_finished = true;
            uint64_t max_pred_finished = 0;
            for (uint64_t p : nodes[id].preds) {
                uint64_t t_pred_finish = nodes[p].t_LR + nodes[p].latency;
                // Predecessor order influences runtime
                if (nodes[p].t_LR == 0 || t_pred_finish > t) {
                    all_finished = false;
                    break;
                }
                max_pred_finished = std::max(max_pred_finished, t_pred_finish);
            }

            if (all_finished) {
                uint32_t k = nodes[id].type;
                U_act[k].insert(nodes[id].id);
                delete_stack.push_back(id);
                nodes[id].operands_ready = max_pred_finished;
            }
        }

        for (uint64_t id : delete_stack) {
            ready_nodes.erase(id);
        }
    }

    void find_running_operations(uint64_t t) {
        bool add_candidates = false;
        std::vector<uint64_t> delete_stack;

        for (int k = 1; k < type_length; k++) {
            for (uint64_t id : S_act[k]) {
                T_act[k].insert(id);
            }

            for (uint64_t id : T_act[k]) {
                uint64_t node_finish = nodes[id].t_LR + nodes[id].latency;
                if (node_finish <= t) {
                    delete_stack.push_back(id);

                    for (uint64_t cond_id : nodes[id].exit_cond) {
                        uint64_t cond_finish = nodes[cond_id].t_LR + nodes[cond_id].latency;

                        if (nodes[cond_id].t_LR == 0 || cond_finish > t) {
                            delete_stack.pop_back();
                            nodes[id].latency += 1;
                            break;
                        }
                    }
                }
            }
        }

        for (uint64_t id : delete_stack) {
            uint32_t k = nodes[id].type;
            T_act[k].erase(id);

            uint64_t parent_id = nodes[id].parent;
            if (parent_id != 0) {
                nodes[parent_id].children_unfinished -= 1;

                if (nodes[parent_id].children_unfinished == 0) {
                    nodes[parent_id].latency = t - nodes[parent_id].t_LR;
                    uint32_t parent_k = nodes[parent_id].type;
                    T_act[parent_k].erase(parent_id);
                    // Adding succesor of parent
                    add_candidates |= true;
                }
            }
        }

        if (add_candidates) {
            find_candidate_operations(t);
        }
    }

    void schedule(uint64_t t_enter, bool finish_schedule = false) {
        nodes[0].t_LR = 1;

        while (t_curr <= t_enter || (finish_schedule && ready_nodes.size() > 0)) {
            find_candidate_operations(t_curr);
            find_running_operations(t_curr);

            for (int k = 1; k < type_length; k++) {
                S_act[k].clear();

                // pq could be represented by using a priority_queue for U_act ?
                std::priority_queue<Node, std::vector<Node>, CompareNodes> pq;
                for (auto& u : U_act[k]) {
                    pq.push(nodes[u]);
                }

                while (!pq.empty() && (S_act[k].size() + T_act[k].size() < pq.top().capacity)) {
                    int id_max = pq.top().id;
                    pq.pop();

                    uint64_t parent_id = nodes[id_max].parent;
                    if (parent_id != 0 && nodes[parent_id].t_LR == 0) {
                        continue;
                    }

                    S_act[k].insert(id_max);
                    nodes[id_max].t_LR = t_curr;

                    // Add succesors to ready nodes
                    for (uint64_t s : nodes[id_max].succs) {
                        ready_nodes.insert(s);
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
        ret_strs << nodes[id].id;
        ret_strs << ")";

        return ret_strs.str();
    }

    uint64_t get_node_t_start(uint64_t curr_node_id) {
        return nodes[curr_node_id].t_LR - 1;
    }

    uint64_t get_node_t_end(uint64_t curr_node_id) {
        return nodes[curr_node_id].t_LR + nodes[curr_node_id].latency - 1;
    }
};

#endif