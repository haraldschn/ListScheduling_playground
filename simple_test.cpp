#include<iostream>

#include "DependencyGraph.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    rv32_ooo::DependencyGraph graph;

    // Always nodes added with edges to predecessors
    // add implicit
    // graph.add_edge_RAW(0,1); // Added edge from root node if no predecessor exists
    graph.add_node(1, rv32_ooo::LD, 2);
    graph.schedule(1, 2);
    graph.get_latency(1, 2);

    graph.add_node(2, rv32_ooo::LD, 3);
    graph.schedule(2, 3);
    graph.get_latency(2, 2);

    graph.add_node(3, rv32_ooo::DIV, 4);
    graph.add_edge_RAW(1,3);
    graph.add_edge_RAW(2,3);
    graph.schedule(3, 4);
    graph.get_latency(3, 4);

    graph.add_node(4, rv32_ooo::ALU, 5);
    graph.add_edge_RAW(1,4);
    graph.schedule(4, 5);
    graph.get_latency(4, 1);

    graph.add_node(5, rv32_ooo::MUL, 6);
    graph.add_edge_RAW(1,5);
    graph.add_edge_RAW(4,5);
    graph.schedule(5, 6);
    graph.get_latency(5, 2);

    graph.add_node(6, rv32_ooo::MUL, 7);
    graph.add_edge_RAW(3,6);
    graph.schedule(6, 7);
    graph.get_latency(6, 2);

    graph.add_node(7, rv32_ooo::ALU, 8);
    graph.add_edge_RAW(2,7);
    graph.add_edge_RAW(6,7);
    graph.schedule(7, 8);
    graph.get_latency(7, 1);

    // Store
    graph.add_node(8, rv32_ooo::ST, 9);
    graph.add_edge_RAW(7,8);
    graph.schedule(8, 9);
    graph.get_latency(8, 1);

    // Load
    graph.add_node(9, rv32_ooo::LD, 10);
    graph.schedule(9, 10);
    graph.get_latency(9, 2);

    graph.add_node(10, rv32_ooo::ALU, 11);
    graph.add_edge_RAW(9,10);
    graph.schedule(10, 11);
    graph.get_latency(10, 1);

    for (size_t i = 1; i < graph.get_nodes_len(); i++)
    {
        std::cout << "id:" << i << "\t,prio:" <<  graph.get_priority_str(i) << "\n";
    }

    

    return 0;
}