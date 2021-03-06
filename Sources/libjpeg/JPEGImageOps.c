#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <turbojpeg.h>
#include "include/JPEGImageOps.h"

#define THROW_TJ(action)  THROW(action, tjGetErrorStr2(tjInstance))
#define THROW_UNIX(action)  THROW(action, strerror(errno))
#define DEFAULT_SUBSAMP  TJSAMP_444
#define DEFAULT_QUALITY  95

tjscalingfactor *scalingFactors = NULL;
int numScalingFactors = 0;

DLLEXPORT unsigned char* tjJPEGLoadCompressedImage(const char *filename, int *width,
                                     int align, int *height, int *pixelFormat, int *inSubsamp,
                                     int flags) {

    tjscalingfactor scalingFactor = { 1, 1 };
    char *inFormat = "jpg", *outFormat = "jpg";
    FILE *jpegFile = NULL;
    unsigned char *imgBuf = NULL, *jpegBuf = NULL;
    *pixelFormat = TJPF_UNKNOWN;
    tjhandle tjInstance = NULL;

    long size;
    unsigned long jpegSize;

    /* Read the JPEG file into memory. */
    jpegFile = fopen(filename, "rb");
    fseek(jpegFile, 0, SEEK_END);
    size = ftell(jpegFile);
    fseek(jpegFile, 0, SEEK_SET);
    jpegSize = (unsigned long)size;
    jpegBuf = (unsigned char *)tjAlloc(jpegSize);
    fread(jpegBuf, jpegSize, 1, jpegFile);
    fclose(jpegFile);  jpegFile = NULL;

    tjInstance = tjInitDecompress();
    tjDecompressHeader(tjInstance, jpegBuf, jpegSize, width, height);
    imgBuf = (unsigned char *)tjAlloc(*width * *height * 3);
    tjDecompress2(tjInstance, jpegBuf, jpegSize, imgBuf, *width, 0, *height, TJPF_RGB, 0);
    tjFree(jpegBuf);  jpegBuf = NULL;
    tjDestroy(tjInstance);  tjInstance = NULL;

    return imgBuf;
}

DLLEXPORT int tjJPEGSaveImage(const char *filename, unsigned char *buffer,
                          int width, int pitch, int height, int pixelFormat, int outSubsamp,
                          int flags) {
    
    FILE *jpegFile = NULL;
    unsigned char *imgBuf = NULL, *jpegBuf = NULL;
    tjhandle tjInstance = NULL;

    int inSubsamp, retVal = -1, outQual = DEFAULT_QUALITY;
    unsigned long jpegSize=0;
    
    tjInstance = tjInitCompress();
    tjCompress2(tjInstance, buffer, width, 0, height, pixelFormat,
                &jpegBuf, &jpegSize, outSubsamp, outQual, flags);
    tjDestroy(tjInstance);  tjInstance = NULL;
    
    jpegFile = fopen(filename, "wb");
    if (fwrite(jpegBuf, jpegSize, 1, jpegFile) == 1)
        retVal = 0;
    tjDestroy(tjInstance);  tjInstance = NULL;
    fclose(jpegFile);  jpegFile = NULL;
    tjFree(jpegBuf);  jpegBuf = NULL;
    
    return retVal;
    
}
