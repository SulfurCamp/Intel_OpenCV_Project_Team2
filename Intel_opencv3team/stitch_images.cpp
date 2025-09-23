#include "stitch_images.h"
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <vector>


// Largest rectangle of ones in a binary image (histogram method)
static cv::Rect largestAxisAlignedRect(const cv::Mat& bin) {
    CV_Assert(bin.type() == CV_8U);
    const int rows = bin.rows, cols = bin.cols;
    std::vector<int> h(cols, 0);
    int bestArea = 0; cv::Rect best(0,0,cols,rows);
    for (int i = 0; i < rows; ++i) {
        const uchar* p = bin.ptr<uchar>(i);
        for (int j = 0; j < cols; ++j) h[j] = p[j] ? h[j] + 1 : 0;
        std::vector<std::pair<int,int>> st; // (start, height)
        for (int j = 0; j <= cols; ++j) {
            int height = (j == cols) ? 0 : h[j];
            int start = j;
            while (!st.empty() && st.back().second > height) {
                int idx = st.back().first; int ht = st.back().second; st.pop_back();
                int area = ht * (j - idx);
                if (area > bestArea) { bestArea = area; best = cv::Rect(idx, i - ht + 1, j - idx, ht); }
                start = idx;
            }
            if (st.empty() || st.back().second < height) st.emplace_back(start, height);
        }
    }
    return best;
}


static cv::Mat crop_nonblack(const cv::Mat& pano) {
    CV_Assert(pano.type() == CV_8UC3 || pano.type() == CV_8UC4);
    cv::Mat bgr; if (pano.channels() == 4) cv::cvtColor(pano, bgr, cv::COLOR_BGRA2BGR); else bgr = pano;
    cv::Mat mask; cv::inRange(bgr, cv::Scalar(1,1,1), cv::Scalar(255,255,255), mask);
    int k = std::max(3, ((bgr.cols + bgr.rows) / 2) / 30); if (k % 2 == 0) ++k;
    cv::Mat kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(k,k));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel, cv::Point(-1,-1), 1);
    if (cv::countNonZero(mask) == 0) return pano.clone();
    cv::Rect r = largestAxisAlignedRect(mask);
    r &= cv::Rect(0,0,pano.cols,pano.rows);
    if (r.width <= 0 || r.height <= 0) return pano.clone();
    return pano(r).clone();
}


cv::Mat stitchImages(const std::vector<cv::Mat>& images, bool& success) {
    success = false; cv::Mat pano;
    if (images.size() < 2) return pano;
    auto stitcher = cv::Stitcher::create(cv::Stitcher::PANORAMA);
    auto status = stitcher->stitch(images, pano);
    if (status == cv::Stitcher::OK && !pano.empty()) { success = true; return crop_nonblack(pano); }
    return pano;
}
