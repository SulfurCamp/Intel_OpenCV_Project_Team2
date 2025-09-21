#ifndef STITCH_IMAGES_H
#define STITCH_IMAGES_H

#include <opencv2/core.hpp>
#include <vector>

cv::Mat stitchImages(const std::vector<cv::Mat>& images, bool& success);

#endif // STITCH_IMAGES_H
