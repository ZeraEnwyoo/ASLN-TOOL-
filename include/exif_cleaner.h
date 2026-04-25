#ifndef ASLN_EXIF_CLEANER_H
#define ASLN_EXIF_CLEANER_H

int strip_exif(const char* input_path, const char* output_path);
int batch_strip_exif(const char* directory);
int has_metadata(const char* image_path);

#endif
