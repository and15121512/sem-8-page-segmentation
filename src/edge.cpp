#include "edge.h"

#include <algorithm>

bool operator<(const Edge& a, const Edge& b) { return a.weight < b.weight; }
bool operator<(const Edge& a, double b) { return a.weight < b; }
bool operator<(double a, const Edge& b) { return a < b.weight; }

bool operator==(const Edge& a, const Edge& b) { return std::abs(a.weight - b.weight) < 1e-7; }
bool operator==(const Edge& a, double b) { return std::abs(a.weight - b) < 1e-7; }
bool operator==(double a, const Edge& b) { return std::abs(a - b.weight) < 1e-7; }

bool operator>(const Edge& a, const Edge& b) { return a.weight > b.weight; }
bool operator>(const Edge& a, double b) { return a.weight > b; }
bool operator>(double a, const Edge& b) { return a > b.weight; }

bool Edge::isEdgeHorizontal(cv::Mat stats) const {
	double x_len = std::abs(stats.at<int>(from, 0) - stats.at<int>(to, 0));
	double y_len = std::abs(stats.at<int>(from, 1) - stats.at<int>(to, 1));
	return (y_len < x_len || weight < 1e-3);
}

bool Edge::isEdgeVertical(cv::Mat stats) const {
	return !isEdgeHorizontal(stats);
}
