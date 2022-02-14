/** @file
* @brief Модуль осуществления оценки решения задачи сегментации, часть утилиты main_eval.
*/

#ifndef EVALUATION_H
#define EVALUATION_H

#include <opencv2/opencv.hpp>

/**
\brief Структура о значениями ошибок различных типов для решения задачи сегментации документов.
*/
struct EvaluationResult {
    /// Значение ошибки типа False Detection
    double falseDetectionErr;
    /// Значение ошибки типа Miss
    double missErr;
    /// Значение ошибки типа Partial Miss
    double prtialMissErr;
    /// Значение ошибки типа Split
    double splitErr;
    /// Значение ошибки типа Merger
    double mergerErr;
    /// Изображение с выделенными компонентами истинной разметки (для дебага)
    cv::Mat gtImg;
    /// Изображение с выделенными компонентами найденной разметки (для дебага)
    cv::Mat segImg;
};

/**
* @brief Сравнивает найденную и истинную разметки и возвращает структуру со значениями ошибок.
* @param templateImg Исходная страница документа
* @param groundTruth Истинная разметка (полученная из дтасета через утилиту main_parser)
* @param segmResult Найденная разметка (утилитой main_seg)
* @return Структура со значениями ошибок
*/
EvaluationResult pageSegmEvaluation(cv::Mat templateImg
                                  , const std::vector<std::vector<cv::Point>>& groundTruth
                                  , const std::vector<std::vector<cv::Point>>& segmResult);

#endif // #ifndef EVALUATION_H
