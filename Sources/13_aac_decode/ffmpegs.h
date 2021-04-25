#ifndef FFMPEGS_H
#define FFMPEGS_H


extern "C" {
#include <libavformat/avformat.h>
}


typedef struct {
    const char* filename;
    int sampleRate;
    AVSampleFormat sampleFmt;
    int chLayout;
} AudioDecodeSpec;

class FFmpegs {

public:
    FFmpegs();

    static void accDecode(const char* inFilename, AudioDecodeSpec &out);
};

#endif // FFMPEGS_H
