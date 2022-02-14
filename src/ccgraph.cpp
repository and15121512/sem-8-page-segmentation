#include "ccgraph.h"
#include "dsu.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <exception>



CCGraph CCGraph::MST() {
    CCGraph mst(this->verticesCnt_);
    
    std::vector<Edge> edges = edges_;
    std::sort(edges.begin(), edges.end());
    DSU dsu(mst.verticesCnt());
    for (const auto& edge : edges)
    {
        if (dsu.Union(edge.from, edge.to)) {
            mst.InsertEdge(edge);
        }
    }
    return mst;
}

//std::vector<CCGraph> CCGraph::pageSegments(size_t kMinEdgesToSplit, cv::Mat stats) const {
//    std::vector<CCGraph> components;
//    //try {
//		pageSegmentsRec(components, *this, kMinEdgesToSplit, stats);
//    //} catch (std::exception e) {
//	//	std::cout << "[ERROR] " << e.what() << std::endl;
//	//}
//	return components;
//}

std::vector<CCGraph> CCGraph::pageSegments(double maxEdgeWidth, double maxEdgeHeight, cv::Mat stats) const {
    //std::vector<CCGraph> components;
    CCGraph cpy = *this;
    std::vector<size_t> to_erase;
    for (size_t i = 0; i < cpy.edges_.size(); ++i) {
        if (cpy.edges_[i].isEdgeHorizontal(stats) && cpy.edges_[i].weight >= maxEdgeWidth) { to_erase.push_back(i); }
        if (cpy.edges_[i].isEdgeVertical(stats) && cpy.edges_[i].weight >= maxEdgeHeight) { to_erase.push_back(i); }
    }
    for (int i = to_erase.size() - 1; i >= 0; --i) {
        cpy.edges_.erase(cpy.edges_.begin() + to_erase[i]);
    }
    return cpy.segmentsByBFS(stats);
}

cv::Rect CCGraph::BBoxByBFS(cv::Mat stats) const {
    cv::Rect bbox(0, 0, 0, 0); // (x, y, w, h)
    if (edges_.empty()) {
        return bbox;
    }
    size_t initVertex = edges_[0].from;

    int leftmost = 1e7;
    int rightmost = 0;
    int upmost = 1e7;
    int bottommost = 0;

    std::queue<size_t> q;
    q.push(initVertex);

    size_t currVertex = initVertex;
    std::vector<char> added_edges(this->edgesCnt(), 0);
    while (!q.empty()) {
        currVertex = q.front();
        q.pop();

        int width = stats.at<int>(currVertex, 2);
        int height = stats.at<int>(currVertex, 3);
        if (width / double(height) > 1e2) { continue; }
        if (height / double(width) > 1e2) { continue; }
        leftmost = std::min(leftmost
            , stats.at<int>(currVertex, 0));
        rightmost = std::max(rightmost
            , stats.at<int>(currVertex, 0)
            + width);
        upmost = std::min(upmost
            , stats.at<int>(currVertex, 1));
        bottommost = std::max(bottommost
            , stats.at<int>(currVertex, 1)
            + height);

        size_t i = 0;
        for (const auto& edge : this->edges_) {
            if (edge.from == currVertex
                && 0 == added_edges[i]) {
                added_edges[i] = 1;
                q.push(edge.to);
            }
            else if (edge.to == currVertex
                && 0 == added_edges[i]) {
                added_edges[i] = 1;
                q.push(edge.from);
            }
            ++i;
        }
    }

    double x_c = (leftmost + rightmost) / 2.;
    double y_c = (upmost + bottommost) / 2.;
    double width = (rightmost - leftmost);
    double height = (bottommost - upmost);
    bbox = cv::Rect(x_c - width / 2.
                , y_c - height / 2.
                , width
                , height);
    return bbox;
}

size_t CCGraph::horizontalEdgesCnt(cv::Mat stats) const {
	size_t res = 0;
	for (const auto& edge : edges_) {
		if (edge.isEdgeHorizontal(stats)) { ++res; }
	}
	return res;
}

std::vector<cv::Rect> CCGraph::splittedByVertical(cv::Mat stats) const {
	std::vector<CCGraph> segments;
	
	CCGraph graph_cpy = *this;
	std::vector<size_t> to_remove;
	for (size_t i = 0; i < graph_cpy.edgesCnt(); ++i) {
		if (edges_[i].isEdgeVertical(stats)) { to_remove.push_back(i); }
	}
	
	for (int i = to_remove.size() - 1; i >= 0; --i) {
		(graph_cpy.edges_).erase(graph_cpy.edges_.begin() + to_remove[i]);
	}
	
	return graph_cpy.componentsByBFS(stats);
}

//void CCGraph::printGraph() const {
//    std::cout << "Vertices: " << verticesCnt_ << std::endl;
//    std::cout << "Edges: " << edges_.size() << std::endl;
//    std::cout << "Graph: " << std::endl;
//    for (const auto& edge : edges_) {
//        std::cout << edge.from << " "
//                  << edge.to << " "
//                  << edge.weight << std::endl;
//    }
//}

void CCGraph::drawGraph(cv::Mat im, cv::Mat coords, cv::Scalar color) const {
    for (const auto& edge : edges_) {
        double x1 = coords.at<double>(edge.from, 0);
        double y1 = coords.at<double>(edge.from, 1);
        double x2 = coords.at<double>(edge.to, 0);
        double y2 = coords.at<double>(edge.to, 1);
        cv::line(im, cv::Point(x1, y1), cv::Point(x2, y2), color, 2);
    }
}

//void CCGraph::saveGraph(cv::Mat stats) const {
//    std::ofstream fout_h("weights_h.txt");
//	std::ofstream fout_v("weights_v.txt");
//    if (!fout_h || !fout_v) { std::cout << "[ERROR] Can't save graph." << std::endl; }
//    for (const auto& edge : edges_) {
//		double x_len = std::abs(stats.at<int>(edge.from, 0) - stats.at<int>(edge.to, 0));
//		double y_len = std::abs(stats.at<int>(edge.from, 1) - stats.at<int>(edge.to, 1));
//		if (y_len < x_len || edge.weight < 1e-3) { fout_h << edge.weight << std::endl; }
//		else { fout_v << edge.weight << std::endl; }
//    }
//}

//void CCGraph::pageSegmentsRec(std::vector<CCGraph>& components
//                       , CCGraph currComponent
//                       , size_t kMinEdgesToSplit
//                       , cv::Mat stats) const {
//    double k_max_area_ratio = 1.5;
//
//    if (currComponent.edgesCnt() < kMinEdgesToSplit) {
//        if (0 != currComponent.edgesCnt()) { components.push_back(currComponent); }
//        return;
//    }
//	//if (0 == currComponent.edgesCnt()) { return; }
//    auto max_it = std::max_element(currComponent.edges_.begin(), currComponent.edges_.end());
//    //auto max_it = currComponent.edges_.begin();
//    //for (auto it = currComponent.edges_.begin(); it != currComponent.edges_.end(); ++it) {
//    //    double from_area = stats.at<int>(it->from, 2) * stats.at<int>(it->from, 3) + 1e-3;
//    //    double to_area = stats.at<int>(it->to, 2) * stats.at<int>(it->to, 3) + 1e-3;
//    //    bool parse_cond = false; //(from_area / to_area > k_max_area_ratio || to_area / from_area > k_max_area_ratio);
//    //    if (parse_cond || *it > *max_it) { max_it = it; }
//    //}
//    size_t from = max_it->from;
//    size_t to = max_it->to;
//    currComponent.edges_.erase(max_it);
//    pageSegmentsRec(components, currComponent.copyTreeByBFS(from), kMinEdgesToSplit, stats);
//    pageSegmentsRec(components, currComponent.copyTreeByBFS(to), kMinEdgesToSplit, stats);
//}

CCGraph CCGraph::copyTreeByBFS(size_t initVertex) const {
    CCGraph newTree(this->verticesCnt());

    std::queue<size_t> q;
    q.push(initVertex);

    size_t currVertex = initVertex;
    std::vector<char> added_edges(this->edgesCnt(), 0);
    while (!q.empty()) {
        currVertex = q.front();
        q.pop();
        size_t i = 0;
        for (const auto& edge : this->edges_) {
            if (edge.from == currVertex
                && 0 == added_edges[i]) {
                newTree.InsertEdge(edge);
                added_edges[i] = 1;
                q.push(edge.to);
            }
            else if (edge.to == currVertex
                && 0 == added_edges[i]) {
                newTree.InsertEdge(edge);
                added_edges[i] = 1;
                q.push(edge.from);
            }
            ++i;
        }
    }
    return newTree;
}

std::vector<CCGraph> CCGraph::segmentsByBFS(cv::Mat stats) const {
    std::vector<CCGraph> segments;
    std::vector<char> visited(verticesCnt_, 0);
    for (size_t init = 0; init < verticesCnt_; ++init) {
        if (1 == visited[init]) { continue; }
        CCGraph component(this->verticesCnt());
        std::queue<size_t> q;
        q.push(init);
        visited[init] = 1;

        size_t currVertex = init;
        while (!q.empty()) {
            currVertex = q.front();
            q.pop();
            for (const auto& edge : this->edges_) {
                if (edge.from == currVertex) {
                    component.InsertEdge(edge);
                    if (0 == visited[edge.to]) {
                        visited[edge.to] = 1;
                        q.push(edge.to);
                    }
                }
                else if (edge.to == currVertex) {
                    component.InsertEdge(edge);
                    if (0 == visited[edge.from]) {
                        visited[edge.from] = 1;
                        q.push(edge.from);
                    }
                }
            }
        }
        if (component.edgesCnt() > 0) {
            segments.push_back(component);
        }
    }
    return segments;
}

std::vector<cv::Rect> CCGraph::componentsByBFS(cv::Mat stats) const {
	std::vector<cv::Rect> bboxes;
	std::vector<char> visited(verticesCnt_, 0);
	for (size_t init = 0; init < verticesCnt_; ++init) {
        if (1 == visited[init]) { continue; }
		CCGraph component(this->verticesCnt());
		std::queue<size_t> q;
		q.push(init);
		visited[init] = 1;
	
		size_t currVertex = init;
		while (!q.empty()) {
			currVertex = q.front();
			q.pop();
			for (const auto& edge : this->edges_) {
				if (edge.from == currVertex) {
					component.InsertEdge(edge);
                    if (0 == visited[edge.to]) {
                        visited[edge.to] = 1;
                        q.push(edge.to);
                    }
				}
				else if (edge.to == currVertex) {
					component.InsertEdge(edge);
                    if (0 == visited[edge.from]) {
                        visited[edge.from] = 1;
                        q.push(edge.from);
                    }
				}
			}
		}
        if (component.edgesCnt() > 0) {
            bboxes.push_back(component.BBoxByBFS(stats));
        }
	}
	return bboxes;
}
