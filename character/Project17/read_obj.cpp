#pragma once
#include<iostream>
#include<fstream>

#define MAX_LINE_LENGTH 128
#include"read_obj.h"

void read_obj_file(const char* filename, Model* model) {
    std::ifstream in{ filename };
    if (not in) {
        std::cout << "파일 읽기 실패\n";
    }

    std::vector<Vertex> temp_vertex;
    std::vector<Normal> temp_norm;
    std::vector<Face> temp_vdx; // 정점 인덱스
    std::vector<Face> temp_ndx; // 노멀 인덱스

    while (in) {
        std::string line;
        in >> line;
        if (line == "v") {
            Vertex v;
            in >> v.x >> v.y >> v.z;
            temp_vertex.push_back(v);
        }else if (line == "vn") {
            Normal n;
            in >> n.x >> n.y >> n.z;
            temp_norm.push_back(n);
        }else if (line == "f") {
            char a;
            Face n;
            Face v;
            //Face t;
            Face vertexIndex, uvIndex, normalIndex;

            for (int i = 0; i < 3; i++)
            {
                in >> vertexIndex.v[i] >> a >> uvIndex.v[i] >> a >> normalIndex.v[i];
                --vertexIndex.v[i];
                --uvIndex.v[i];
                --normalIndex.v[i];
            }
            temp_vdx.push_back(vertexIndex);
            temp_ndx.push_back(normalIndex);

        }
    }

    for (int i = 0; i < temp_vdx.size(); ++i) {
        model->vertices.push_back(temp_vertex[temp_vdx[i].v[0]]);
        model->vertices.push_back(temp_vertex[temp_vdx[i].v[1]]);
        model->vertices.push_back(temp_vertex[temp_vdx[i].v[2]]);
    }
    for (int i = 0; i < temp_ndx.size(); ++i) {
        model->nvectors.push_back(temp_norm[temp_ndx[i].v[0]]);
        model->nvectors.push_back(temp_norm[temp_ndx[i].v[1]]);
        model->nvectors.push_back(temp_norm[temp_ndx[i].v[2]]);
    }

    
}
