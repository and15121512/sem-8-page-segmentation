/** @file
* @brief Исполняемый файл утилиты для осуществления сегментации. По входной странице документа возвращает 
* изображение с MST, изображение с найденной разметкой и саму разметку (в отдельном файле).
*/

#include "page_segmentation.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>


/// \private
int main(int argc, char** argv)
{
    if (3 != argc)
    {
        std::cout << "[FATAL] Two arguments required: input image path, output dir." << std::endl;
        return 0;
    }

    std::filesystem::path in_file = std::filesystem::path(argv[1]).lexically_normal();
    std::filesystem::path out_dir = std::filesystem::path(argv[2]).lexically_normal();
    std::filesystem::path out_file = out_dir / in_file.filename().string();
    out_file.replace_extension(".txt");
	std::filesystem::path out_mst = out_dir / ("mst_" + in_file.filename().string());
    std::filesystem::path out_res = out_dir / ("res_" + in_file.filename().string());

    cv::Mat page = cv::imread(in_file.string());
    if (page.empty()) {
        std::cout << "[FATAL] Can't open input image: " + in_file.string() << std::endl;
        return 0;
    }
    SegResult res_so = segmentedPage(page);
    cv::imwrite(out_mst.string(), res_so.mstImg);
	cv::imwrite(out_res.string(), res_so.segImg);

    std::vector<std::vector<cv::Point>> segms = res_so.segm;
    std::ofstream fout(out_file.string());
    if (!fout) {
        std::cout << "[FATAL] Can't create output file: " + out_file.string() << std::endl;
        return 0;
    }
    for (const auto& segm : segms) {
        for (const auto& p : segm) {
            fout << p.x << " " << p.y << std::endl;
        }
        fout << std::endl;
    }

	cv::Mat res = cv::imread(out_mst.string());
    if (res.empty()) {
        std::cout << "[FATAL] Can't save output image: " + out_mst.string() << std::endl;
        return 0;
    }
    res = cv::imread(out_res.string());
    if (res.empty()) {
        std::cout << "[FATAL] Can't save output image: " + out_res.string() << std::endl;
        return 0;
    }
    return 0;
}
