/** @file
* @brief Модуль осуществления сегментации страницы, часть утилиты main_seg.
*/

#ifndef PAGE_SEGMENTATION_H
#define PAGE_SEGMENTATION_H

#include "ccgraph.h"

#include <opencv2/opencv.hpp>

#include <vector>
#include <algorithm>

/**
\brief Структура ответа на задачу сегментации документов.
*/
struct SegResult {
    /// Исходное изображение с наложенным поверх MST
    cv::Mat mstImg;
    /// Исходное изображение с наложенным поверх сегментированными участками
    cv::Mat segImg;
    /// Массив компонент, заданных массивом вершин ограничивающего многоугольника
    std::vector<std::vector<cv::Point>> segm;
};

/**
* @brief Осуществляет сегментацию страницы документа.
* @param page Открытый поток для файла с разметкой (от утилит main_seg или main_parser)
* @return Структура ответа на задачу (изображение с MST, изображение с отрисованной разметкой, сама разметка)
*/
SegResult segmentedPage(cv::Mat page);

#endif // #ifndef PAGE_SEGMENTATION_H
