#include "dsu.h"

#include <vector>



void DSU::Create(int vertex)
{
    parents[vertex] = vertex;
    rank[vertex] = 0;
}

int DSU::Find(int vertex)
{
    int curr_vertex = vertex;
    while (curr_vertex != parents[curr_vertex])
        curr_vertex = parents[curr_vertex];

    int leader = curr_vertex;
    curr_vertex = vertex;
    while (curr_vertex != parents[curr_vertex])
    {
        int parent_vertex = parents[curr_vertex];
        parents[curr_vertex] = leader;
        curr_vertex = parent_vertex;
    }

    return leader;
}

bool DSU::Union(int vertex_a, int vertex_b)
{
    vertex_a = Find(vertex_a);
    vertex_b = Find(vertex_b);

    if (vertex_a != vertex_b)
    {
        if (rank[vertex_a] > rank[vertex_b]) {
            parents[vertex_b] = vertex_a;
        }
        else {
            parents[vertex_a] = vertex_b;
        }

        if (rank[vertex_a] == rank[vertex_b]) {
            ++rank[vertex_a];
        }
        return true;
    }
    return false;
}