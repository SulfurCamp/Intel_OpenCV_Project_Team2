
#include <opencv2/opencv.hpp>
#include <opencv2/stitching.hpp>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <algorithm>

using namespace cv;
using namespace std;

static Rect largestAxisAlignedRect(const Mat& bin) {
    CV_Assert(bin.type() == CV_8U);
    const int rows = bin.rows, cols = bin.cols;

    vector<int> h(cols, 0);
    int bestArea = 0;
    Rect best(0, 0, cols, rows);

    for (int i = 0; i < rows; ++i) {
        const uchar* p = bin.ptr<uchar>(i);
        for (int j = 0; j < cols; ++j) h[j] = p[j] ? h[j] + 1 : 0;

        vector<pair<int,int>> st;
        for (int j = 0; j <= cols; ++j) {
            int height = (j == cols) ? 0 : h[j];
            int start = j;
            while (!st.empty() && st.back().second > height) {
                int idx = st.back().first;
                int ht  = st.back().second;
                st.pop_back();
                int area = ht * (j - idx);
                if (area > bestArea) {
                    bestArea = area;
                    best = Rect(idx, i - ht + 1, j - idx, ht);
                }
                start = idx;
            }
            if (st.empty() || st.back().second < height) st.emplace_back(start, height);
        }
    }
    return best;
}

static Mat crop_nonblack(const Mat& pano,
                         int threshold = 6,  
                         bool dumpDebug = false)
{
    CV_Assert(pano.type() == CV_8UC3 || pano.type() == CV_8UC4);

    Mat bgr;
    if (pano.channels() == 4) cvtColor(pano, bgr, COLOR_BGRA2BGR);
    else bgr = pano;

    Mat mask;
    inRange(bgr, Scalar(threshold, threshold, threshold), Scalar(255,255,255), mask);

    int k = max(3, ((bgr.cols + bgr.rows) / 2) / 30);
    if (k % 2 == 0) k++;                            
    Mat kernel = getStructuringElement(MORPH_RECT, Size(k, k));
    morphologyEx(mask, mask, MORPH_CLOSE, kernel, Point(-1,-1), 1);

    if (dumpDebug) imwrite("debug_mask.png", mask);

    if (countNonZero(mask) == 0) return pano.clone();

    Rect r = largestAxisAlignedRect(mask);
    r &= Rect(0, 0, pano.cols, pano.rows);
    if (r.width <= 0 || r.height <= 0) return pano.clone();

    return pano(r).clone();
}

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <out_base> <img1> <img2> [img3 ...] [--debug]\n";
        return 1;
    }

    string outBase = argv[1];
    bool debug = false;

    vector<string> paths;
    for (int i = 2; i < argc; ++i) {
        string a = argv[i];
        if (a == "--debug") { debug = true; continue; }
        paths.push_back(a);
    }
    if (paths.size() < 2) {
        cerr << "Need at least two input images.\n";
        return 1;
    }

    vector<Mat> imgs;
    for (auto& p : paths) {
        Mat m = imread(p, IMREAD_COLOR);
        if (m.empty()) {
            cerr << "Failed to read: " << p << "\n";
            return 2;
        }
        imgs.push_back(m);
    }

    Ptr<Stitcher> stitcher = Stitcher::create(Stitcher::PANORAMA);
    Mat pano;
    Stitcher::Status status = stitcher->stitch(imgs, pano);
    if (status != Stitcher::OK) {
        cerr << "Stitching failed. Status = " << int(status) << "\n";
        return 3;
    }

    string panoPath = outBase + "_pano.jpg";
    if (!imwrite(panoPath, pano)) {
        cerr << "Failed to write: " << panoPath << "\n";
        return 4;
    }

    Mat cropped = crop_nonblack(pano, /*threshold=*/6, /*dumpDebug=*/debug);
    string cropPath = outBase + "_cropped.jpg";
    if (!imwrite(cropPath, cropped)) {
        cerr << "Failed to write: " << cropPath << "\n";
        return 5;
    }

    cout << "Saved pano    : " << panoPath   << " (" << pano.cols   << "x" << pano.rows   << ")\n";
    cout << "Saved cropped : " << cropPath   << " (" << cropped.cols<< "x" << cropped.rows<< ")\n";
    if (debug) cout << "Saved mask   : debug_mask.png\n";
    return 0;
}

