#include "imagestructs.h"

namespace Image
{

    uint32_t bitsPerPixelForFormat(ColorFormat format)
    {
        switch (format)
        {
        case ColorFormat::Paletted1:
            return 1;
        case ColorFormat::Paletted2:
            return 2;
        case ColorFormat::Paletted4:
            return 4;
        case ColorFormat::Paletted8:
            return 8;
        case ColorFormat::RGB555:
            return 15;
        case ColorFormat::RGB565:
            return 16;
        case ColorFormat::RGB888:
            return 24;
        default:
            throw std::runtime_error("Bad color format");
        }
    }

    std::string to_string(ColorFormat format)
    {
        switch (format)
        {
        case ColorFormat::Paletted1:
            return "paletted 1-bit";
        case ColorFormat::Paletted2:
            return "paletted 2-bit";
        case ColorFormat::Paletted4:
            return "paletted 4-bit";
        case ColorFormat::Paletted8:
            return "paletted 8-bit";
        case ColorFormat::RGB555:
            return "RGB555";
        case ColorFormat::RGB565:
            return "RGB565";
        case ColorFormat::RGB888:
            return "RGB888";
        default:
            throw std::runtime_error("Bad color format");
        }
    }

}