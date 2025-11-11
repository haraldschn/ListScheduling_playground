#ifndef DEPGRAPH_H
#define DEPGRAPH_H

#include <cstdint>

#include <sstream>
#include <string>

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>

// Functional Unit set at Decode stage
// (using enum F_Type as numeric value)
enum F_Type {
    EMPTY,
    DIV,
    MUL,
    BR,
    ALU,
    LSU,
    F_SIZE
};

const std::string F_Names[] = {
    "EMPTY",
    "DIV",
    "MUL",
    "BR",
    "ALU",
    "LSU",
    "F_SIZE"};

class DependencyGraph {
   private:
    bool debug;

    void print_set_vec(std::vector<uint64_t>& input) {
        std::cout << "{";

        if (input.empty()) {
            std::cout << "}\t";
            return;
        }

        for (size_t i = 0; i < input.size() - 1; i++) {
            std::cout << input[i] << ",";
        }
        std::cout << input[input.size() - 1] << "}\t";
    };

    struct Node {
        // ordering asscending by (operands_ready, issue_ready, id)
        uint64_t operands_ready = UINT64_MAX;
        uint64_t issue_ready = 0;
        uint64_t id = 0;
        // node information
        F_Type type = F_Type::EMPTY;
        uint64_t latency = 1;
        uint64_t t_LR = 0;

        uint8_t a_k = 1; // parallel instructions in functional unit
        uint8_t s_k = 1; // starting instruction per cycle
        // Vector of predecessor nodes
        std::vector<uint64_t> predc;
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
            if (a.issue_ready != b.issue_ready) {
                return a.issue_ready > b.issue_ready;  // lower second
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

    std::priority_queue<Node, std::vector<Node>, CompareOpReady> prio_queue_nodes;

    std::unordered_set<uint64_t> ready_nodes;
    std::unordered_set<uint64_t> active_nodes;

   public:
    DependencyGraph(bool debug_in = false) {
        debug = debug_in;

        Node root_node;
        root_node.id = 0;
        root_node.type = F_Type::EMPTY;
        root_node.latency = 0;
        nodes.push_back(root_node);
    };
    ~DependencyGraph() = default;

    uint64_t add_node(F_Type type, uint64_t issue_ready, uint8_t a_k = 1, uint8_t s_k = 1) {
        Node node_new;
        
        node_new.issue_ready = issue_ready;

        node_new.type = type;
        node_new.latency = 1;
        node_new.a_k = a_k;
        node_new.s_k = s_k;

        nodes.push_back(node_new);
        uint64_t id = nodes.size()-1;
        nodes[id].id = id;

        ready_nodes.insert(id);

        add_edge_RAW(0, id);

        return id;
    }

    void add_edge_RAW(const uint64_t& from, const uint64_t& to) {
        // Remove dummy edges to source node (if other edge is added)
        if (!nodes[to].predc.empty()) {
            nodes[to].predc.pop_back();
        }

        nodes[to].predc.push_back(from);
    }

    void purge_finished_nodes(uint64_t t_curr) {
        std::vector<uint64_t> remove_active;

        for (uint64_t id : active_nodes) {
            auto& v = nodes[id];
            if (v.t_LR > 0) {
                uint64_t node_finish = v.t_LR + v.latency;
                if (node_finish < t_curr) {
                    remove_active.push_back(v.id);
                }
            }
        }

        for (uint64_t id : remove_active) {
            active_nodes.erase(id);
        }
    }

    std::vector<uint64_t> find_candidate_operations(int k, uint64_t t_act) {
        std::vector<uint64_t> ans;
        for (uint64_t id : ready_nodes) {
            auto& v = nodes[id];
            if (v.type == k && v.t_LR == 0) {
                bool all_finished = true;
                uint64_t max_pred_finished = 0;
                for (uint64_t p : v.predc) {
                    uint64_t t_pred_finish = nodes[p].t_LR + nodes[p].latency;
                    max_pred_finished = std::max(max_pred_finished, t_pred_finish);

                    if (nodes[p].t_LR == 0 || t_pred_finish > t_act) {
                        all_finished = false;
                    }
                }

                if (all_finished) {
                    ans.push_back(id);
                    nodes[id].operands_ready = max_pred_finished;
                    prio_queue_nodes.push(nodes[id]);
                }
            }
        }
        return ans;
    }

    std::vector<uint64_t> find_running_operations(int k, uint64_t t_act) {
        std::vector<uint64_t> ans;

        for (uint64_t id : active_nodes) {
            auto& v = nodes[id];
            if (v.type == k && v.t_LR > 0) {
                uint64_t node_finish = v.t_LR + v.latency;
                if (node_finish > t_act) {
                    ans.push_back(id);
                }
            }
        }

        return ans;
    }

    uint64_t schedule(uint64_t curr_node, uint64_t t_curr) {
        purge_finished_nodes(t_curr);

        // Handling nodes with operands_ready > t_curr 
        // priority queue ordered by deacreasing operands_ready -> currently usable (DON'T TRY to copy this prio_queue_nodes)
        while (!prio_queue_nodes.empty()) {
            uint64_t id = prio_queue_nodes.top().id;
            if (nodes[id].operands_ready > t_curr) {
                ready_nodes.insert(id);
                nodes[id].t_LR = 0;
            } else {
                break;
            }
            prio_queue_nodes.pop();
        }

        nodes[0].t_LR = 1;
        ready_nodes.erase(0);
        uint64_t t_act = t_curr;

        while (nodes[curr_node].t_LR == 0) {
            for (int k = F_Type::DIV; k < F_Type::F_SIZE; k++) {
                std::vector<uint64_t> U_act = find_candidate_operations(k, t_act);
                std::vector<uint64_t> T_act = find_running_operations(k, t_act);

                // pq could be represented by using a priority_queue for U_act ?
                std::priority_queue<Node, std::vector<Node>, CompareNodes> pq;
                for (auto& u : U_act) {
                    pq.push(nodes[u]);
                }

                std::vector<uint64_t> S_act;

                while (!pq.empty() && (S_act.size() < pq.top().s_k) && (S_act.size() + T_act.size() < pq.top().a_k)) {
                    int id_max = pq.top().id;
                    pq.pop();

                    S_act.push_back(id_max);
                    nodes[id_max].t_LR = t_act;

                    ready_nodes.erase(id_max);
                    active_nodes.insert(id_max);
                }

                if (nodes[curr_node].type == k && debug) {
                    std::cout << "t_act:" << t_act << "\n";
                    std::cout << "k: " << F_Names[k];
                    std::cout << "\tU_act:";
                    print_set_vec(U_act);
                    std::cout << "T_act:";
                    print_set_vec(T_act);
                    std::cout << "S_act:";
                    print_set_vec(S_act);
                    std::cout << "\n";
                }
            }
            t_act += 1;
            
        }

        return nodes[curr_node].t_LR;
    }

    void get_latency(uint64_t curr_node, uint64_t t_curr) {
        nodes[curr_node].latency = t_curr;
    }

    void get_WB_time(uint64_t curr_node, uint64_t t_curr) {
        if (t_curr > nodes[curr_node].t_LR + 1) {
            nodes[curr_node].latency = t_curr - (nodes[curr_node].t_LR + 1);
            //std::cout << "latency: " << nodes[curr_node].latency << "\n";
        } else {
            //std::cout << "latency: 1 (else)\n";
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
        ret_strs << nodes[id].issue_ready;
        ret_strs << ",";
        ret_strs << nodes[id].id;
        ret_strs << ")";

        return ret_strs.str();
    }
};

#endif  // DEPGRAPH_H