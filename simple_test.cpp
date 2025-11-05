#include<iostream>

#include "DependencyGraph.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    rv32_ooo::DependencyGraph graph;

    // Always nodes added with edges to predecessors
    // add implicit
    // graph.add_edge_RAW(0,1); // Added edge from root node if no predecessor exists
    graph.add_node(1, "V1 (MUL)",2);
    graph.add_node(2, "V2 (MUL)",2);
    graph.add_node(6, "V6 (MUL)",2);
    graph.add_node(8, "V8 (MUL)",2);
    graph.add_node(10, "V10 (ADD)");

    graph.add_node(3, "V3 (MUL)",2);
    graph.add_edge_RAW(1,3);
    graph.add_edge_RAW(2,3);


    graph.add_node(7, "V7 (MUL)",2);
    graph.add_edge_RAW(6,7);
    
    graph.add_node(9, "V9 (ADD)");
    graph.add_edge_RAW(8,9);

    graph.add_node(11, "V11 (LT)");
    graph.add_edge_RAW(10,11);

    graph.add_node(4, "V4 (SUB)");
    graph.add_edge_RAW(3,4);

    graph.add_node(5, "V5 (SUB)");
    graph.add_edge_RAW(4,5);
    graph.add_edge_RAW(7,5);

    // add implicit
    //graph.add_node(12, "SINK", 0);
    //graph.add_edge_RAW(5,12);
    //graph.add_edge_RAW(9,12);
    //graph.add_edge_RAW(11,12);

    graph.update_priorities();

    for (size_t i = 1; i < 12; i++)
    {
        std::cout << "id:" << i << "\t,prio:" <<  graph.get_priority(i) << "\n";
    }

    graph.schedule();

    return 0;
}