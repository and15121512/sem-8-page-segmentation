/** @file
* @brief Модуль, реализующий вспомогательную структуру для алгоритма Крускала нахождения MST.
*/

#ifndef DSU_H
#define DSU_H

#include <vector>

/**
\brief Вспомогательная структура данных для алгоритма Крускала нахождения MST.
*/
class DSU
{
public:

    DSU(int vertices_count) : 
        parents(std::vector<int>(vertices_count))
        , rank(std::vector<int>(vertices_count, 0))
    {
        for (size_t i = 0; i < vertices_count; ++i)
            parents[i] = i;
    }

    void Create(int vertex);
    int Find(int vertex);
    bool Union(int vertex_a, int vertex_b);

private:
    std::vector<int> parents;
    std::vector<int> rank;
};

#endif // #ifndef DSU_H
