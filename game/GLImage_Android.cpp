#include "GLImage.h"
#include <android/imagedecoder.h>
#include <android/asset_manager.h>
#include <android/imagedecoder.h>
#include <cassert>
#include <vector>

void GL::Image::load(const unsigned char *buf, size_t bufSize) {
    if (isLoaded()) return;

    // Make a decoder to turn it into a texture
    AImageDecoder *pAndroidDecoder = nullptr;
    auto result = AImageDecoder_createFromBuffer(buf, bufSize, &pAndroidDecoder);
    assert(result == ANDROID_IMAGE_DECODER_SUCCESS);

    // make sure we get 8 bits per channel out. RGBA order.
    AImageDecoder_setAndroidBitmapFormat(pAndroidDecoder, ANDROID_BITMAP_FORMAT_RGBA_8888);

    // Get the image header, to help set everything up
    const AImageDecoderHeaderInfo *pAndroidHeader = nullptr;
    pAndroidHeader = AImageDecoder_getHeaderInfo(pAndroidDecoder);

    // important metrics for sending to GL
    auto width = AImageDecoderHeaderInfo_getWidth(pAndroidHeader);
    auto height = AImageDecoderHeaderInfo_getHeight(pAndroidHeader);
    auto stride = AImageDecoder_getMinimumStride(pAndroidDecoder);

    width_ = width;
    height_ = height;

    // Get the bitmap data of the image
    auto upAndroidImageData = std::make_unique<std::vector<uint8_t>>(height * stride);
    auto decodeResult = AImageDecoder_decodeImage(
            pAndroidDecoder,
            upAndroidImageData->data(),
            stride,
            upAndroidImageData->size());
    assert(decodeResult == ANDROID_IMAGE_DECODER_SUCCESS);

    loadTextureData_(upAndroidImageData->data(), GL_RGBA, true);
}
