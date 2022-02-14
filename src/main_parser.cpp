/** @file
* @brief Исполняемый файл утилиты, выделяющей нужную для утилиты оценки main_eval информацию из входного XML-файла разметки датасета.
* Сохраняет эту информацию в выходной файл.
*/

#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <regex>

/**
* @brief Парсит XML файл истинной разметки от датасета (см. отчёт), выделяя только вершины этой разметки.
* @param fin Открытый поток для файла с XML-разметкой для сэмплов датасета
* @return Извлечённая разметка (массив компонент, представленных массивом вершин компоненты)
*/
std::vector<std::vector<cv::Point>> xmlParser(std::ifstream& fin) {
	std::vector<std::vector<cv::Point>> res;

	std::regex coords_begin("<Coords>");
	std::regex x_val("x=\"(\\d+)\"");
	std::regex y_val("y=\"(\\d+)\"");
	std::regex coords_end("</Coords>");

	bool in_block = false;
	for (std::string line; getline(fin, line); ) {
		if (!in_block) {
			if (std::regex_search(line, coords_begin)) {
				res.push_back(std::vector<cv::Point>());
				in_block = true;
			}
		}
		else {
			std::smatch x_match;
			bool x_found = std::regex_search(line, x_match, x_val);
			std::smatch y_match;
			bool y_found = std::regex_search(line, y_match, y_val);
			if (x_found != y_found) { std::cout << "[info] Wrong XML format. Further reading stopped."; return res; }
			if (x_found && y_found) {
				res.back().push_back(cv::Point(std::stoi(x_match[1]), std::stoi(y_match[1])));
			}

			if (std::regex_search(line, coords_end)) { in_block = false; }
		}
	}

	return res;
}

/// \private
int main(int argc, char** argv)
{
	if (3 != argc)
	{
		std::cout << "[FATAL] Two arguments required: XML file from dataset, output dir." << std::endl;
		return 0;
	}

	std::filesystem::path in_file = std::filesystem::path(argv[1]).lexically_normal();
	std::filesystem::path out_dir = std::filesystem::path(argv[2]).lexically_normal();
	std::filesystem::path out_res = out_dir / in_file.filename().string();
	out_res.replace_extension(".txt");

	std::ifstream fin(in_file.string());
	if (!fin) {
		std::cout << "[FATAL] Can't open XML file: " + in_file.string() << std::endl;
		return 0;
	}
	std::vector<std::vector<cv::Point>> segms = xmlParser(fin);

	std::ofstream fout(out_res.string());
	if (!fout) {
		std::cout << "[FATAL] Can't create output file: " + out_res.string() << std::endl;
		return 0;
	}
	for (const auto& segm : segms) {
		for (const auto& p : segm) {
			fout << p.x << " " << p.y << std::endl;
		}
		fout << std::endl;
	}
	return 0;
}
