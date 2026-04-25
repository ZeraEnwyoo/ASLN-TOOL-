#include "../include/asln.h"
#include "../include/exif_cleaner.h"
#include <jpeglib.h>
#include <png.h>
#include <dirent.h>
#include <errno.h>

static int is_image_file(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return 0;
    return (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0 || strcasecmp(ext, ".png") == 0);
}

static int remove_jpeg_metadata(const char* input, const char* output) {
    FILE* infile = fopen(input, "rb");
    if (!infile) return -1;
    
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return -1;
    }
    
    for (int m = 0; m < 16; m++) jpeg_save_markers(&cinfo, JPEG_APP0 + m, 0xFFFF);
    jpeg_save_markers(&cinfo, JPEG_COM, 0xFFFF);
    
    if (!jpeg_start_decompress(&cinfo)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return -1;
    }
    
    FILE* outfile = fopen(output, "wb");
    if (!outfile) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return -1;
    }
    
    struct jpeg_compress_struct cinfo_out;
    jpeg_create_compress(&cinfo_out);
    jpeg_stdio_dest(&cinfo_out, outfile);
    
    cinfo_out.image_width = cinfo.image_width;
    cinfo_out.image_height = cinfo.image_height;
    cinfo_out.input_components = cinfo.num_components;
    cinfo_out.in_color_space = cinfo.out_color_space;
    jpeg_set_defaults(&cinfo_out);
    jpeg_start_compress(&cinfo_out, TRUE);
    
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, 
                         cinfo.output_width * cinfo.output_components, 1);
    
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        jpeg_write_scanlines(&cinfo_out, buffer, 1);
    }
    
    jpeg_finish_compress(&cinfo_out);
    jpeg_destroy_compress(&cinfo_out);
    fclose(outfile);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
}

static int remove_png_metadata(const char* input, const char* output) {
    FILE* infile = fopen(input, "rb");
    if (!infile) return -1;
    
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) { fclose(infile); return -1; }
    
    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_read_struct(&png, NULL, NULL); fclose(infile); return -1; }
    
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(infile);
        return -1;
    }
    
    png_init_io(png, infile);
    png_read_info(png, info);
    
    FILE* outfile = fopen(output, "wb");
    if (!outfile) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(infile);
        return -1;
    }
    
    png_structp png_out = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_out) {
        fclose(outfile);
        png_destroy_read_struct(&png, &info, NULL);
        fclose(infile);
        return -1;
    }
    
    png_infop info_out = png_create_info_struct(png_out);
    if (!info_out) {
        png_destroy_write_struct(&png_out, NULL);
        fclose(outfile);
        png_destroy_read_struct(&png, &info, NULL);
        fclose(infile);
        return -1;
    }
    
    png_init_io(png_out, outfile);
    png_set_IHDR(png_out, info_out,
                 png_get_image_width(png, info),
                 png_get_image_height(png, info),
                 png_get_bit_depth(png, info),
                 png_get_color_type(png, info),
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_out, info_out);
    
    int height = png_get_image_height(png, info);
    png_bytep* row_pointers = png_get_rows(png, info);
    if (row_pointers) png_write_image(png_out, row_pointers);
    png_write_end(png_out, NULL);
    
    png_destroy_write_struct(&png_out, &info_out);
    fclose(outfile);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(infile);
    return 0;
}

int strip_exif(const char* input_path, const char* output_path) {
    if (!input_path || !output_path) return -1;
    const char* ext = strrchr(input_path, '.');
    if (!ext) return -1;
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0) {
        return remove_jpeg_metadata(input_path, output_path);
    } else if (strcasecmp(ext, ".png") == 0) {
        return remove_png_metadata(input_path, output_path);
    }
    return -1;
}

int batch_strip_exif(const char* directory) {
    if (!directory) return -1;
    DIR* dir = opendir(directory);
    if (!dir) return -1;
    struct dirent* entry;
    char input_path[1024];
    char output_path[1024];
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(input_path, sizeof(input_path), "%s/%s", directory, entry->d_name);
        if (!is_image_file(input_path)) continue;
        snprintf(output_path, sizeof(output_path), "%s.clean", input_path);
        if (strip_exif(input_path, output_path) == 0) {
            rename(output_path, input_path);
            count++;
        }
    }
    closedir(dir);
    return count;
}

int has_metadata(const char* image_path) {
    if (!image_path) return 0;
    FILE* fp = fopen(image_path, "rb");
    if (!fp) return 0;
    unsigned char buffer[1024];
    size_t bytes = fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);
    for (size_t i = 0; i < bytes - 1; i++) {
        if (buffer[i] == 0xFF && buffer[i+1] == 0xE1) return 1;
    }
    return 0;
}
