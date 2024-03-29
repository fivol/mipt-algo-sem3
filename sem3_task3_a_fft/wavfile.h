#pragma once

#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <complex>
#include <vector>
#include <cmath>
#include "wav_header.h"
#include "constants.h"

typedef std::complex<double> complex_t;


class WAVFile {
    WAVHEADER header{};
    char *data = nullptr;

    [[nodiscard]] size_t GetDataLength() const {
        return header.subchunk2Size;
    }

public:
    WAVFile() = default;

    ~WAVFile() {
        delete[] data;
    };

    bool ReadFile(const char *filename);

    bool SaveFile(const char *filename) {
        FILE *file = fopen(filename, "wb");
        if (!file) {
            return false;
        }
        fwrite(&header, sizeof(WAVHEADER), 1, file);
        fwrite(data, GetDataLength(), 1, file);
        fclose(file);
        return true;
    }

    void MakeFFTCompression(double compression_ratio, const std::string& compress_type, int page_size);

    [[nodiscard]] WAVHEADER GetHeader() const {
        return header;
    }

private:
    static void CopyToVector(char *ptr, std::vector<complex_t> &array, int item_size, int n) {
        for (int i = 0; i < n; ++i) {
            array[i] = GetNBytes(ptr + i * item_size, item_size);
        }
    }

    static void CopyFromVector(char *ptr, std::vector<complex_t> &array, int item_size, int n) {
        for (int i = 0; i < n; ++i) {
            SetNBytes(ptr + i * item_size, array[i].real(), item_size);
        }
    }

    void fft(std::vector<complex_t> &array, bool is_invert);

    static uint64_t GetNBytes(void *ptr, int n) {
        uint64_t value = 0;
        memcpy(&value, ptr, n);
        return value;
    }

    static void SetNBytes(void *ptr, uint64_t value, int n) {
        memcpy(ptr, &value, n);
    }
};


bool WAVFile::ReadFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return false;
    }
    fread(&header, sizeof(WAVHEADER), 1, file);

    data = (char *) malloc(header.subchunk2Size);
    fread(data, header.subchunk2Size, 1, file);
    fclose(file);
    return true;
}

void WAVFile::fft(std::vector<complex_t> &array, bool is_invert) {
    int size = (int) array.size();
    if (size == 1){
        return;
    };

    std::vector<complex_t> oven(size / 2), odd(size / 2);
    for (int i = 0, j = 0; i < size; i += 2, ++j) {
        oven[j] = array[i];
        odd[j] = array[i + 1];
    }
    fft(oven, is_invert);
    fft(odd, is_invert);

    double angle = 2 * M_PI / size * (is_invert ? -1 : 1);
    complex_t w(1);
    complex_t wn(cos(angle), sin(angle));
    for (int i = 0; i < size / 2; ++i) {
        array[i] = oven[i] + w * odd[i];
        array[i + size / 2] = oven[i] - w * odd[i];
        if (is_invert){
            array[i] /= 2, array[i + size / 2] /= 2;
        }
        w *= wn;
    }
}

void WAVFile::MakeFFTCompression(double compression_ratio, const std::string& compress_type, int page_size) {
    std::vector<complex_t> sequence_page(page_size);
    int pointer = 0;
    int data_size = GetDataLength();
    while (pointer < data_size) {
        sequence_page.assign(page_size, 0);
        int count = std::min(page_size, (data_size - pointer) / header.blockAlign);
        CopyToVector(data + pointer, sequence_page, header.blockAlign, count);
        fft(sequence_page, false);
        size_t compression_begin_pointer = 0;
        size_t compression_size = compression_ratio * page_size;
        if(COMPRESS_MIDDLE == compress_type){
            compression_begin_pointer = (page_size - compression_size) / 2;
        }else{
            compression_begin_pointer = page_size - compression_size;
        }
        memset(&sequence_page[compression_begin_pointer], 0, compression_size);
        fft(sequence_page, true);
        CopyFromVector(data + pointer, sequence_page, header.blockAlign, count);

        pointer += count * header.blockAlign;
    }
}
