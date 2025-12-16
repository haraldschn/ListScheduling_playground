#ifndef RESOURCE_DEFINITION_H
#define RESOURCE_DEFINITION_H

// Definition for Node Types (Resource, Stage or Substage)
enum RES_Type {
    EMPTY,
    IF_stage,
    ID_stage,
    EX_stage,
    WB_stage,
    IF1_1_substage,
    IF1_2_substage,
    IF2_substage,
    SIZE
};

// Definition for Node Types (Resource, Stage or Substage)
uint32_t RES_Cap[]{
    1,  // EMPTY,
    1,  // IF_stage,
    1,  // ID_stage,
    1,  // EX_stage,
    1,  // WB_stage,
    1,  // IF1_1_substage,
    1,  // IF1_2_substage,
    1,  // IF2_substage,
    1,  // SIZE
};

// Definition for Node Types (Resource, Stage or Substage)
uint32_t RES_Latency[]{
    1,  // EMPTY,
    1,  // IF_stage,
    1,  // ID_stage,
    1,  // EX_stage,
    1,  // WB_stage,
    1,  // IF1_1_substage,
    1,  // IF1_2_substage,
    1,  // IF2_substage,
    1,  // SIZE
};

#endif