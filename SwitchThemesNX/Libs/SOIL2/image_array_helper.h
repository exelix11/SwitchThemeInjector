#ifndef SOIL_TYPES_H
#define SOIL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Structure to hold an array of images */
typedef struct SOIL_ImageArray {
    int width;
    int height;
    int layers;
    int channels;
    unsigned char **data;
} SOIL_ImageArray;

#ifdef __cplusplus
}
#endif

#endif /* SOIL_TYPES_H */
