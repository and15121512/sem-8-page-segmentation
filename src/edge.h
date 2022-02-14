/** @file
* @brief Модуль, реализующий структуру ребра неориентированного взвешенного графа (CCGraph).
*/

#ifndef EDGE_H
#define EDGE_H

#include <opencv2/opencv.hpp>

/**
\brief Структура ребра для графа на вершинах -- целых числах.
*/

struct Edge {
    size_t from;
    size_t to;
    double weight;
	/// Проверяет, что угол с горизонтальной осью для ребра меньше 45 градусов
	bool isEdgeHorizontal(cv::Mat stats) const;
	/// Проверяет, что угол с вертикальной осью для ребра меньше 45 градусов
	bool isEdgeVertical(cv::Mat stats) const;
};

bool operator<(const Edge& a, const Edge& b);
bool operator<(const Edge& a, double b);
bool operator<(double a, const Edge& b);

bool operator==(const Edge& a, const Edge& b);
bool operator==(const Edge& a, double b);
bool operator==(double a, const Edge& b);

bool operator>(const Edge& a, const Edge& b);
bool operator>(const Edge& a, double b);
bool operator>(double a, const Edge& b);

#endif // #ifndef EDGE_H
