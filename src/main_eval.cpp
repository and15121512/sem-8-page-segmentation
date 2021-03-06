/** @file
* @brief Исполняемый файл утилиты для оценки качества сегментации. Сравнивает данные о найденной разметке (полученной утилитой main_seg) с истинной разметкой,
* выделенной из XML-файла датасета (утилитой main_parser).
*/

#include "evaluation.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>
#include <regex>



/**
* @brief Отрисовывает разметку документа segm поверх изображения img цветом color.
* @param img Исходное изображение документа
* @param segm Разметка для отображения (массив компонент -- массивов вершин её многоугольника)
* @param color Цвет разметки
* @return Изображение с отрисованной поверх разметкой
*/
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

/**
* @brief Считывает разметку документа из файла.
* @param fin Открытый поток для файла с разметкой (от утилит main_seg или main_parser)
* @return Разметка (массив компонент -- массивов вершин её многоугольника)
*/
std::vector<std::vector<cv::Point>> readBlocks(std::ifstream& fin) {
	std::vector<std::vector<cv::Point>> res;
	bool in_block = false;
	for (std::string line; getline(fin, line); ) {
		if (line.empty()) {
			if (in_block) {
				in_block = false;
				continue;
			}
			continue;
		}
		if (!in_block) {
			in_block = true;
			res.push_back(std::vector<cv::Point>());
		}
		cv::Point p;
		std::istringstream sin;
		sin.str(line);
		sin >> p.x >> p.y;
		res.back().push_back(p);
	}
	return res;
}

/// \private
int main(int argc, char** argv)
{
	if (5 != argc)
	{
		std::cout << "[FATAL] Four arguments required: source image for segmentation, segmentation result path, ground-truth path, output dir." << std::endl;
		return 0;
	}

	std::filesystem::path in_file_img = std::filesystem::path(argv[1]).lexically_normal();
	std::filesystem::path in_file_seg = std::filesystem::path(argv[2]).lexically_normal();
	std::filesystem::path in_file_gt = std::filesystem::path(argv[3]).lexically_normal();
	std::filesystem::path out_dir = std::filesystem::path(argv[4]).lexically_normal();
	std::filesystem::path out_gt = out_dir / ("gt_" + in_file_img.filename().string());
	std::filesystem::path out_eval = out_dir / ("eval_" + in_file_img.filename().string());

	std::ifstream fin_seg(in_file_seg.string());
	if (!fin_seg) {
		std::cout << "[FATAL] Can't open segmentation result file: " + in_file_seg.string() << std::endl;
		return 0;
	}
	std::vector<std::vector<cv::Point>> segm = readBlocks(fin_seg);
	std::ifstream fin_gt(in_file_gt.string());
	if (!fin_gt) {
		std::cout << "[FATAL] Can't open ground-truth file: " + in_file_gt.string() << std::endl;
		return 0;
	}
	std::vector<std::vector<cv::Point>> gt = readBlocks(fin_gt);

	cv::Mat page = cv::imread(in_file_img.string());
	if (page.empty()) {
		std::cout << "[FATAL] Can't open input image: " + in_file_img.string() << std::endl;
		return 0;
	}
	EvaluationResult res_so = pageSegmEvaluation(page, gt, segm);
	std::cout << "Errors: " << std::endl
		<< "    False Detection: " << res_so.falseDetectionErr << std::endl
		<< "    Miss: " << res_so.missErr << std::endl
		<< "    Partial Miss: " << res_so.prtialMissErr << std::endl
		<< "    Split: " << res_so.splitErr << std::endl
		<< "    Merger: " << res_so.mergerErr << std::endl
		<< "Total error: " << (res_so.falseDetectionErr
							+ res_so.missErr
							+ res_so.prtialMissErr
							+ res_so.splitErr
							+ res_so.mergerErr) / 5. << std::endl;

	cv::Mat gt_res = page.clone();
	gt_res = drawSegments(gt_res, gt, cv::Scalar(0, 0, 255));
	cv::imwrite(out_gt.string(), gt_res);
	gt_res = cv::imread(out_gt.string());
	if (gt_res.empty()) {
		std::cout << "[FATAL] Can't save output image: " + out_gt.string() << std::endl;
		return 0;
	}

	cv::Mat eval_res = page.clone();
	eval_res = drawSegments(eval_res, gt, cv::Scalar(0, 0, 255));
	eval_res = drawSegments(eval_res, segm, cv::Scalar(255, 0, 0));
	cv::imwrite(out_eval.string(), eval_res);
	eval_res = cv::imread(out_eval.string());
	if (eval_res.empty()) {
		std::cout << "[FATAL] Can't save output image: " + out_eval.string() << std::endl;
		return 0;
	}
	return 0;
}

// for (std::string line; getline(fin, line); ) {
// 	if (!in_block) {
// 		if (std::regex_search(line, coords_begin)) {
// 			res.push_back(std::vector<cv::Point>());
// 			in_block = true;
// 		}
// 	}
// 	else {
// 		std::smatch x_match;
// 		bool x_found = std::regex_search(line, x_match, x_val);
// 		std::smatch y_match;
// 		bool y_found = std::regex_search(line, y_match, y_val);
// 		if (x_found != y_found) { std::cout << "[info] Wrong XML format. Further reading stopped."; return res; }
// 		if (x_found && y_found) {
// 			res.back().push_back(cv::Point(std::stoi(x_match[1]), std::stoi(y_match[1])));
// 		}
// 
// 		if (std::regex_search(line, coords_end)) { in_block = false; }
// 	}
// }

// std::ifstream fin(xmlLocation);
// if (!fin) { std::cout << "[fatal] Wrong ground-truth XML location." << std::endl; return res; }
// std::vector<std::vector<cv::Point>> gt = xmlParser(fin);
// EvaluationResult eval_res = pageSegmEvaluation(page, gt, segm);
// res.falseDetectionErr = eval_res.falseDetectionErr;
// res.missErr = eval_res.missErr;
// res.prtialMissErr = eval_res.prtialMissErr;
// res.splitErr = eval_res.splitErr;
// res.mergerErr = eval_res.mergerErr;
// res.segImg = page_res.clone();
// res.segImg = drawSegments(res.segImg, gt, cv::Scalar(0, 0, 255));
// res.segImg = drawSegments(res.segImg, segm, cv::Scalar(255, 0, 0));
// res.mstImg = page_mst;
