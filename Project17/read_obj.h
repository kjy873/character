#pragma once
#include<vector>

#define MAX_LINE_LENGTH 128

typedef struct {
    float x, y, z;
} Vertex;
typedef struct {
    unsigned int v[3];
} Face;
typedef struct {
    float x, y, z;
}Normal;



typedef struct {

    std::vector<Vertex> vertices;

    std::vector<Normal> nvectors;

    
    
} Model;
void read_newline(char* str);
void read_obj_file(const char* filename, Model* model);