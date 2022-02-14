#include "page_segmentation.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <list>
#include <string>
#include <regex>



bool BBoxInclusion(cv::Rect what, cv::Rect where) {
	const double k_scale = 1.1;
	
	cv::Rect where_mod(where.x + where.width / 2. * (1 - k_scale)
					   , where.y + where.height / 2. * (1 - k_scale)
					   , where.width * k_scale
					   , where.height * k_scale);
	
	cv::Point tl(what.x, what.y);
	cv::Point tr(what.x + what.width, what.y);
	cv::Point bl(what.x, what.y + what.height);
	cv::Point br(what.x + what.width, what.y + what.height);
	return where_mod.contains(tl)
           && where_mod.contains(tr)
		   && where_mod.contains(bl)
		   && where_mod.contains(br);
};

inline double distance(cv::Rect rect1, cv::Rect rect2) {
	double res;
	cv::Point cent1(rect1.x + rect1.width / 2., rect1.y + rect1.height / 2.);
	cv::Point cent2(rect2.x + rect2.width / 2., rect2.y + rect2.height / 2.);
	double dist_x = std::abs(cent1.x - cent2.x) - (rect1.width + rect2.width) / 2.;
	double dist_y = std::abs(cent1.y - cent2.y) - (rect1.height + rect2.height) / 2.;
	
	if (dist_x < 0. && dist_y < 0.) { res = 0.; }
	else if (dist_x > 0. && dist_y > 0.) {
		res = std::sqrt(std::pow(dist_x, 2) + 1.3 * std::pow(dist_y, 2)); 
		//res = std::abs(dist_x) + std::abs(dist_y); //std::sqrt(std::pow(dist_x, 2) + 1.3 * std::pow(dist_y, 2));
	}
	else {
		res = std::max(dist_x, 1.3 * dist_y);
	}
    return res;
	//return (std::abs(x1 - x2) + 1.5 * std::abs(y1 - y2));
    //return std::sqrt(std::pow(x1 - x2, 2) + 1.3 * std::pow(y1 - y2, 2));
}

cv::Mat preprocesseing(cv::Mat page) {
    const int k_downscale_width = 500;

    cv::Mat page_gray;
    cv::cvtColor(page, page_gray, cv::COLOR_BGR2GRAY);
    cv::Mat page_down;
    double scale = k_downscale_width / double(page_gray.size().width);
    //resize(
    //    page_gray
    //    , page_down
    //    , Size(k_downscale_width, page_gray.size().height * scale)
    //    , 0
    //    , 0
    //    , INTER_LINEAR
    //);
    page_down = page_gray;

	cv::Mat page_denoised;
	cv::fastNlMeansDenoising(page_down, page_denoised);
    cv::Mat page_bin;
    double res_threshold = cv::threshold(page_denoised, page_bin, 127, 255, cv::THRESH_OTSU);
    return page_bin;
}

void initCCGraph(CCGraph* graph, cv::Mat stats, cv::Mat centroids, cv::Size imgSz) {
    for (size_t i = 0; i < graph->verticesCnt(); ++i) {
        int x1 = stats.at<int>(i, 0);
        int y1 = stats.at<int>(i, 1);
        if (0 == x1 && 0 == y1) { continue; } // full-image label
        int w1 = stats.at<int>(i, 2);
        int h1 = stats.at<int>(i, 3);
		if (w1 > imgSz.width * 0.05 && h1 > imgSz.height * 0.05) { continue; }
        for (size_t j = i + 1; j < graph->verticesCnt(); ++j) {
            int x2 = stats.at<int>(j, 0);
            int y2 = stats.at<int>(j, 1);
            if (0 == x2 && 0 == y2) { continue; } // full-image label
            int w2 = stats.at<int>(j, 2);
            int h2 = stats.at<int>(j, 3);
			if (w2 > imgSz.width * 0.05 && h2 > imgSz.height * 0.05) { continue; }
            graph->InsertEdge(i, j, distance(cv::Rect(x1, y1, w1, h1), cv::Rect(x2, y2, w2, h2)));
        }
    }
}

void drawCCs(cv::Mat page, cv::Mat stats) {
    // Draws bounding boxes for each graph vertex (CC)
    for (size_t i = 0; i < stats.size().height; ++i) {
        int x = stats.at<int>(i, 0);
        int y = stats.at<int>(i, 1);
        int w = stats.at<int>(i, 2);
        int h = stats.at<int>(i, 3);
        cv::rectangle(page, cv::Rect(x, y, w, h), cv::Scalar(0, 127, 0));
    }
}

std::vector<cv::Rect> CCGraphsToBBoxes(std::vector<CCGraph>& segments, cv::Mat stats) {
	std::vector<cv::Rect> bboxes;
	for (const auto& segment : segments) {
		bboxes.push_back(segment.BBoxByBFS(stats));
    }
	
	std::vector<size_t> bboxes_to_erase;
	for (auto it1 = bboxes.begin(); it1 != bboxes.end(); ++it1) {
		for (auto it2 = bboxes.begin(); it2 != bboxes.end(); ++it2) {
			if (it1 != it2 && BBoxInclusion(*it1, *it2)) { bboxes_to_erase.push_back(it1 - bboxes.begin()); }
		}
	}
	std::sort(bboxes_to_erase.begin(), bboxes_to_erase.end());
	
	for (int i = bboxes_to_erase.size() - 1; i >= 0; --i) {
		segments.erase(segments.begin() + bboxes_to_erase[i]);
		bboxes.erase(bboxes.begin() + bboxes_to_erase[i]);
	}
	return bboxes;
}

inline cv::Rect mergedBBoxes(cv::Rect bbox1, cv::Rect bbox2) {
	double x = std::min(bbox1.x, bbox2.x);
	double y = std::min(bbox1.y, bbox2.y);
	double w = std::max(bbox1.x + bbox1.width, bbox2.x + bbox2.width) - x;
	double h = std::max(bbox1.y + bbox1.height, bbox2.y + bbox2.height) - y;
	return cv::Rect(x, y, w, h);
}

double CCWidthAvg(cv::Mat stats) {
	double res = 0;
	for (size_t j = 0; j < stats.size().height; ++j) {
		res += stats.at<int>(j, 2);
	}
	res = res / stats.size().height;
	return res;
}

double CCHeightAvg(cv::Mat stats) {
	double res = 0;
	for (size_t j = 0; j < stats.size().height; ++j) {
		res += stats.at<int>(j, 3);
	}
	res = res / stats.size().height;
	return res;
}

bool gapCond(cv::Rect bbox1, cv::Rect bbox2) {
	double k_allowable_gap = 0.4;
	double y1_c = bbox1.y + bbox1.height / 2.;
	double y2_c = bbox2.y + bbox2.height / 2.;
	return (std::abs(y1_c - y2_c) < k_allowable_gap * bbox1.height
		|| std::abs(y1_c - y2_c) < k_allowable_gap * bbox2.height);
}

std::vector<cv::Rect> splitSegmentToLines(CCGraph& segment, cv::Mat stats) {
	const double k_dist_to_merge = CCWidthAvg(stats) * 6.;
	
	std::cout << "[info] Splitting segment by vertical edges..." << std::endl;
	std::vector<cv::Rect> bboxes = segment.splittedByVertical(stats);
	std::cout << "[info] Done." << std::endl;
	
	std::cout << "[info] Merging bboxes into lines..." << std::endl;
	std::list<cv::Rect> bboxes_lst(bboxes.begin(), bboxes.end());
	bool found = true;
	while (found) {
		found = false;
		auto it1 = bboxes_lst.begin();
		auto it2 = bboxes_lst.begin();
		for (it1 = bboxes_lst.begin(); it1 != bboxes_lst.end(); ++it1) {
			it2 = it1;
			++it2;
			for (; it2 != bboxes_lst.end(); ++it2) {
				found = (distance(*it1, *it2) < k_dist_to_merge && gapCond(*it1, *it2));
				if (found) { break; }
			}
			if (found) { break; }
		}
		if (found) {
			cv::Rect newBBox = mergedBBoxes(*it1, *it2);
			bboxes_lst.erase(it1);
			bboxes_lst.erase(it2);
			bboxes_lst.push_back(newBBox);
		}
	}
	bboxes = std::vector<cv::Rect>(bboxes_lst.begin(), bboxes_lst.end());
	std::cout << "[info] Done." << std::endl;
	return bboxes;
}

cv::Scalar randRGB() {
	return cv::Scalar(
		(rand() + 30) % 220
		, (rand() + 30) % 220
		, (rand() + 30) % 220
	);
}

class LineComp {
public:
	bool operator()(cv::Rect line1, cv::Rect line2) { return line1.y < line2.y; }
};

std::vector<std::vector<cv::Rect>> splittedParagraphs(const std::vector<cv::Rect>& lines) {
	std::vector<std::vector<cv::Rect>> paragraphs;
	
	std::vector<cv::Rect> lines_cpy = lines;
	std::sort(lines_cpy.begin(), lines_cpy.end(), LineComp());
	std::vector<std::vector<cv::Rect>::iterator> par_separators;
	par_separators.push_back(lines_cpy.begin());
	auto it1 = lines_cpy.begin();
	auto it2 = lines_cpy.begin();
	++it2;
	for (; it1 != lines_cpy.end() && it2 != lines_cpy.end(); ++it1, ++it2) {
		double left_gap = std::max(it1->width, it2->width) * 0.05;
		double right_gap = std::max(it1->width, it2->width) * 0.05;
		if (it2->x - it1->x >= left_gap
			&& (it2->x - it1->x) + (it2->width - it2->width) >= right_gap) {
			par_separators.push_back(it2);
		}
	}
	par_separators.push_back(lines_cpy.end());

	for (size_t i = 0; i + 1 < par_separators.size(); ++i) {
		paragraphs.push_back(std::vector<cv::Rect>(par_separators[i], par_separators[i + 1]));
	}
	return paragraphs;
}

std::vector<cv::Point> blockByLines(const std::vector<cv::Rect>& lines) {
	std::vector<cv::Point> res;

	std::vector<cv::Rect> lines_cpy = lines;
	std::sort(lines_cpy.begin(), lines_cpy.end(), LineComp());
	bool first = true;
	double prev_y = -1.;
	for (auto it = lines_cpy.begin(); it != lines_cpy.end(); ++it) {
		if (!first) {
			res.push_back(cv::Point(it->x + it->width, prev_y));
		}
		prev_y = it->y + it->height;
		first = false;
		res.push_back(cv::Point(it->x + it->width, it->y));
		res.push_back(cv::Point(it->x + it->width, it->y + it->height));
	}
	first = true;
	prev_y = -1.;
	for (auto it = lines_cpy.rbegin(); it != lines_cpy.rend(); ++it) {
		if (!first) {
			res.push_back(cv::Point(it->x, prev_y));
		}
		prev_y = it->y;
		first = false;
		res.push_back(cv::Point(it->x, it->y + it->height));
		res.push_back(cv::Point(it->x, it->y));
	}
	return res;
}

cv::Mat drawSegments(cv::Mat img
	, const std::vector<std::vector<cv::Point>>& segm
	, cv::Scalar color) {
	const double alpha = 0.7;
	cv::Mat res = img.clone();
	for (size_t i = 0; i < segm.size(); ++i) {
		cv::Mat roi = res.clone();
		cv::fillPoly(roi, std::vector<std::vector<cv::Point>>(1, segm[i]), color);
		cv::addWeighted(res.clone(), alpha, roi, 1 - alpha, 0.0, res);

		auto it1 = segm[i].begin();
		auto it2 = segm[i].begin();
		++it2;
		for (; it1 != segm[i].end() && it2 != segm[i].end(); ++it1, ++it2) {
			cv::line(res, *it1, *it2, color, 2);
		}
		cv::line(res, segm[i].front(), segm[i].back(), color, 2);
	}
	return res;
}

SegResult segmentedPage(cv::Mat page) {
	SegResult res;
	
    cv::Mat page_prep = preprocesseing(page);
    std::cout << "[info] Preprocessing done." << std::endl;

    cv::Mat labels, stats, centroids;
    cv::connectedComponentsWithStats(~page_prep, labels, stats, centroids);
    std::cout << "[info] Connected components found: " << centroids.size().height - 1 << "." << std::endl;

    CCGraph graph(centroids.size().height);
    initCCGraph(&graph, stats, centroids, page_prep.size());
    CCGraph mst = graph.MST();
    std::cout << "[info] MST constructed." << std::endl;
	cv::Mat page_mst;
    cv::cvtColor(page_prep, page_mst, cv::COLOR_GRAY2RGB);
	drawCCs(page_mst, stats);
	mst.drawGraph(page_mst, centroids, cv::Scalar(0, 255, 0));
	res.mstImg = page_mst;
	
    //std::vector<CCGraph> segments = mst.pageSegments(mst.verticesCnt() * 0.5, stats);
	std::vector<CCGraph> segments = mst.pageSegments(CCWidthAvg(stats) * 2, CCHeightAvg(stats) * 1.75, stats);
	std::cout << "[info] Page segments found: " << segments.size() << std::endl;
	
    cv::Mat page_res;
    //cv::cvtColor(page_prep, page_res, cv::COLOR_GRAY2RGB);
	page_res = page_mst.clone();
	std::vector<std::vector<cv::Point>> segm;
	for (size_t i = 0; i < segments.size(); ++i) {
		std::cout << "Current segment: " << i << std::endl;
		std::vector<std::vector<cv::Rect>> curr_paragraphs = splittedParagraphs(splitSegmentToLines(segments[i], stats));
		for (const auto& paragraph : curr_paragraphs) {
			std::vector<cv::Point> block = blockByLines(paragraph);
			if (!block.empty()) {
				segm.push_back(blockByLines(paragraph));
			}
		}
	}
	res.segm = segm;
	res.segImg = page_res.clone();
	res.segImg = drawSegments(res.segImg, segm, cv::Scalar(255, 0, 0));
	res.mstImg = page_mst;
    return res;
}
