/** @file
* @brief Модуль, реализующий абстрактный неориентиованный взвешенный граф (для построения MST и т.п.).
*/

#ifndef CCGRAPH_H
#define CCGRAPH_H

#include "edge.h"

#include <opencv2/opencv.hpp>

#include <vector>

/**
\brief Описание класса, реализующего неориентированный взвешенный граф (хранит рёбра как массив дуг).
*/
class CCGraph {
public:
    typedef std::vector<Edge>::const_iterator Iterator;

    /// Конструирует пустой граф на вершинах - целых числах от 0 до verticesCnt
    CCGraph(size_t verticesCnt) :
        verticesCnt_(verticesCnt) {
        edges_.reserve(verticesCnt * (verticesCnt - 1) / 2.);
    }
    /// Конструирует граф на вершинах - целых числах от 0 до verticesCnt со рёбрами из edges
    CCGraph(size_t verticesCnt, const std::vector<Edge>& edges) :
        verticesCnt_(verticesCnt)
        , edges_(edges) { }
    /// Вставляет ребро между вершинами from и to с весом weight
    void InsertEdge(size_t from, size_t to, double weight) {
        edges_.push_back(Edge{ from, to, weight });
    }
    /// Вставляет ребро, используя готовую структуру ребра edge
    void InsertEdge(const Edge& edge) {
        edges_.push_back(edge);
    }
    /// Находит минимальное остовное дерево для графа, возвращает его в виде нового графа
    CCGraph MST();
    /// Выделяет первичные компоненты страницы в виде графов, принимает граничные значения для удаления рёбер и информацию о компонентах связности stats
    std::vector<CCGraph> pageSegments(double maxEdgeWidth, double maxEdgeHeight, cv::Mat stats) const;
    /// Помещает граф в минимальный bounding box и возвращает его, используя информацию о компонентах связности stats
    cv::Rect BBoxByBFS(cv::Mat stats) const;
    /// Считает количество рёбер в графе, угол с горизонтальной осью которых меньше 45 градусов
	size_t horizontalEdgesCnt(cv::Mat stats) const;
    /// Возвращает граф, в котором удалены рёбра, , угол с вертикальной осью которых меньше 45 градусов
	std::vector<cv::Rect> splittedByVertical(cv::Mat stats) const;
    /// Возвращает количество вершин в графе
    size_t verticesCnt() const { return verticesCnt_; }
    /// Возвращает текущее количество рёбер в графе
    size_t edgesCnt() const { return edges_.size(); }

    /// Отрисовывает граф на изобр-е im по инф-ции о КС coords с цветом color
    void drawGraph(cv::Mat im, cv::Mat coords, cv::Scalar color) const;
private:
    std::vector<Edge> edges_;
    size_t verticesCnt_;
    /// Возвращает дерево, полученное обходом исходного дерева из вершины initVertex и добавлением всех рёбер перехода
    CCGraph copyTreeByBFS(size_t initVertex) const;
    /// Возвращает массив компонент связности графа (запуская BFS от каждой вершины)
    std::vector<CCGraph> segmentsByBFS(cv::Mat stats) const;
    /// Возвращает массив boundin box'ов, построенных для каждой компоненты связности графа
    std::vector<cv::Rect> componentsByBFS(cv::Mat stats) const;
};

#endif // #ifndef CCGRAPH_H
