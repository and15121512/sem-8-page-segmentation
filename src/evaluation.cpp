#include "evaluation.h"

#include <iostream>
#include <opencv2/opencv.hpp>



cv::Mat segmAreasImg(cv::Mat templateImg
    , const std::vector<std::vector<cv::Point>>& segm
    , size_t max_pixel_value) {
    cv::Mat res = templateImg.setTo(cv::Scalar(255));
    for (size_t i = 0; i < segm.size(); ++i) {
        cv::fillPoly(res, std::vector<std::vector<cv::Point>>(1, segm[i]), cv::Scalar(i));
    }
    return res.clone();
}

EvaluationResult pageSegmEvaluation(cv::Mat templateImg
    , const std::vector<std::vector<cv::Point>>& groundTruth
    , const std::vector<std::vector<cv::Point>>& segmResult) {
    const size_t max_pixel_value = 254;
    EvaluationResult res;
    res.falseDetectionErr = 0.;
    res.missErr = 0.;
    res.prtialMissErr = 0.;
    res.splitErr = 0.;
    res.mergerErr = 0.;

    cv::Mat gt_img = segmAreasImg(templateImg.clone(), groundTruth, max_pixel_value);
    cv::Mat seg_img = segmAreasImg(templateImg.clone(), segmResult, max_pixel_value);

    size_t gt_area_cnt = 0;
    size_t seg_area_cnt = 0;
    std::cout << "[info] GT processing started..." << std::endl;
    for (size_t i = 0; i < groundTruth.size(); ++i) {
        size_t area_cnt = 0;
        size_t intersect_cnt = 0;
        size_t seg_intersect_cnt = 0;
        std::vector<char> colors_met(max_pixel_value + 1, 1);
        for (size_t x = 0; x < gt_img.size().width; ++x) {
            for (size_t y = 0; y < gt_img.size().height; ++y) {
                area_cnt += (i == gt_img.at<uchar>(y, x)) ? 1 : 0;
                //std::cout << ( 255 != int(seg_img.at<uchar>(y, x)) ) << std::endl;
                if (i == gt_img.at<uchar>(y, x) && 255 != seg_img.at<uchar>(y, x)) {
                    ++intersect_cnt;
                    seg_intersect_cnt += colors_met[seg_img.at<uchar>(y, x)];
                    colors_met[seg_img.at<uchar>(y, x)] = 0;
                }
                //if (i == gt_img.at<uchar>(y, x) && 255 == int(seg_img.at<uchar>(y, x))) { std::cout << "Miss!"; }
            }
        }
        if (0 == seg_intersect_cnt) { res.missErr += area_cnt - intersect_cnt; }
        else if (1 == seg_intersect_cnt) { res.prtialMissErr += area_cnt - intersect_cnt; }
        else { res.prtialMissErr += area_cnt - intersect_cnt; res.splitErr += area_cnt; }
        gt_area_cnt += area_cnt;
        //std::cout << "[info] Segment " << i << ": " << intersect_cnt << " " << area_cnt << std::endl;
    }
    std::cout << "Done." << std::endl;
    std::cout << "[info] Seg processing started..." << std::endl;
    for (size_t i = 0; i < segmResult.size(); ++i) {
        size_t area_cnt = 0;
        size_t intersect_cnt = 0;
        size_t gt_intersect_cnt = 0;
        std::vector<char> colors_met(max_pixel_value + 1, 1);
        for (size_t x = 0; x < seg_img.size().width; ++x) {
            for (size_t y = 0; y < seg_img.size().height; ++y) {
                area_cnt += (i == seg_img.at<uchar>(y, x)) ? 1 : 0;
                if (i == seg_img.at<uchar>(y, x) && 255 != gt_img.at<uchar>(y, x)) {
                    ++intersect_cnt;
                    gt_intersect_cnt += colors_met[gt_img.at<uchar>(y, x)];
                    colors_met[gt_img.at<uchar>(y, x)] = 0;
                }
            }
        }
        if (0 == gt_intersect_cnt) { res.falseDetectionErr += area_cnt - intersect_cnt; }
        else if (gt_intersect_cnt > 1) { res.mergerErr += intersect_cnt; }
        seg_area_cnt += area_cnt;
        //std::cout << "Segment " << i << ": " << intersect_cnt << " " << area_cnt << std::endl;
    }
    std::cout << "Done." << std::endl;
    res.falseDetectionErr /= seg_area_cnt;
    res.missErr /= gt_area_cnt;
    res.prtialMissErr /= gt_area_cnt;
    res.splitErr /= gt_area_cnt;
    res.mergerErr /= gt_area_cnt;
    res.gtImg = gt_img;
    res.segImg = seg_img;
    return res;
}
