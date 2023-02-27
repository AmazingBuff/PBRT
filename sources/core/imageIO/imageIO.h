#pragma once
#include"../pbrt.h"

namespace pbrt
{
    std::unique_ptr<RGBSpectrum[]> ReadImage(const std::string& name, Point2i* resolution);
    void WriteImage(const std::string& name, const Float* rgb, const Bound2i& outputBounds, const Point2i& totalResolution);
}