#include<iostream>

#include "DependencyGraph.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    bool debug_output = true;

    DependencyGraph graph(debug_output);

    uint64_t node_id = 0;

    // Always nodes added with edges to predecessors
    // add implicit
    // graph.add_edge_RAW(0,1); // Added edge from root node if no predecessor exists
    node_id = graph.add_node(LD, 2);
    graph.schedule(node_id, 2);
    graph.get_latency(node_id, 2);

    node_id = graph.add_node(LD, 3);
    graph.schedule(node_id, 3);
    graph.get_latency(node_id, 2);

    node_id = graph.add_node(DIV, 4);
    graph.add_edge_RAW(1,node_id);
    graph.add_edge_RAW(2,node_id);
    graph.schedule(node_id, 4);
    graph.get_latency(node_id, 4);

    node_id = graph.add_node(ALU, 5);
    graph.add_edge_RAW(1,node_id);
    graph.schedule(node_id, 5);
    graph.get_latency(node_id, 1);

    node_id = graph.add_node(MUL, 6);
    graph.add_edge_RAW(1,node_id);
    graph.add_edge_RAW(4,node_id);
    graph.schedule(node_id, 6);
    graph.get_latency(node_id, 2);

    node_id = graph.add_node(MUL, 7);
    graph.add_edge_RAW(3,node_id);
    graph.schedule(node_id, 7);
    graph.get_latency(node_id, 2);

    node_id = graph.add_node(ALU, 8);
    graph.add_edge_RAW(2,node_id);
    graph.add_edge_RAW(6,node_id);
    graph.schedule(node_id, 8);
    graph.get_latency(node_id, 1);

    // Store
    node_id = graph.add_node(ST, 9);
    graph.add_edge_RAW(7,node_id);
    graph.schedule(node_id, 9);
    graph.get_latency(node_id, 1);

    // Load
    node_id = graph.add_node(LD, 10);
    graph.schedule(node_id, 10);
    graph.get_latency(node_id, 2);

    node_id = graph.add_node(ALU, 11);
    graph.add_edge_RAW(9,node_id);
    graph.schedule(node_id, 11);
    graph.get_latency(node_id, 1);

    for (size_t i = 1; i < graph.get_nodes_len() && debug_output; i++)
    {
        std::cout << "id:" << i << "\t,prio:" <<  graph.get_priority_str(i) << "\n";
    }

    

    return 0;
}