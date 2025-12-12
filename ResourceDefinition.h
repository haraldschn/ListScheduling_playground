#ifndef RESOURCE_DEFINITION_H
#define RESOURCE_DEFINITION_H

// Definition for Node Types (Resource, Stage or Substage)
enum RES_Type {
    EMPTY,
    IF_stage,
    ID_stage,
    EX_stage,
    WB_stage,
    IF1_substage,
    IF2_substage,
    SIZE
};

#endif