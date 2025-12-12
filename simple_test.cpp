#include<iostream>

#include "ResourceScheduling.h"
#include "ResourceDefinition.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    bool debug_output = true;

    ResourceGraph graph(RES_Type::SIZE, debug_output);

    std::vector<uint64_t> nodes_IF;
    std::vector<uint64_t> nodes_ID;
    std::vector<uint64_t> nodes_EX;
    std::vector<uint64_t> nodes_WB;

    // Always nodes added with edges to their predecessors
    // 

    nodes_IF.push_back(graph.add_parent_node(IF_stage, 1));
    graph.add_edge(0, nodes_IF.back());

    uint64_t IF1_idx = graph.add_node(IF1_substage, 1, 1, 1, nodes_IF.back());
    graph.add_edge(0, IF1_idx);

    uint64_t IF2_idx = graph.add_node(IF2_substage, 1, 1, 1, nodes_IF.back());
    graph.add_edge(IF1_idx, IF2_idx);

    nodes_ID.push_back(graph.add_node(ID_stage, 1));
    graph.add_edge(0, nodes_ID.back());
    graph.add_edge(nodes_IF.back(), nodes_ID.back());

    graph.add_edge(IF2_idx, nodes_ID.back());

    nodes_EX.push_back(graph.add_node(EX_stage, 1, 4));
    graph.add_edge(0, nodes_EX.back());
    graph.add_edge(nodes_ID.back(), nodes_EX.back());

    nodes_WB.push_back(graph.add_node(WB_stage, 1));
    graph.add_edge(0, nodes_WB.back());
    graph.add_edge(nodes_EX.back(), nodes_WB.back());

    nodes_IF.push_back(graph.add_node(IF_stage, 1));
    graph.add_edge(nodes_IF[0], nodes_IF.back());
    
    nodes_ID.push_back(graph.add_node(ID_stage, 1));
    graph.add_edge(nodes_ID[0], nodes_ID.back());
    graph.add_edge(nodes_IF.back(), nodes_ID.back());

    nodes_EX.push_back(graph.add_node(EX_stage, 1));
    graph.add_edge(nodes_EX[0], nodes_EX.back());
    graph.add_edge(nodes_ID.back(), nodes_EX.back());

    nodes_WB.push_back(graph.add_node(WB_stage, 1));
    graph.add_edge(nodes_WB[0], nodes_WB.back());
    graph.add_edge(nodes_EX.back(), nodes_WB.back());

    graph.schedule(1, true);

    for (size_t i = 1; i < graph.get_nodes_len() && debug_output; i++)
    {
        std::cout << "id:" << i << "\t,prio:" <<  graph.get_priority_str(i) << "\n";
    }

    return 0;
}