#include "ffmpegs.h"

#include <QFile>
#include <QDebug>

FFmpegs::FFmpegs()
{

}

void FFmpegs::pcm2wav(WAVHeader &header, const char *pcmFilename, const char *wavFilename) {
    header.blockAlign = header.bitsPerSample * header.numChannels >> 3;
    header.byteRate = header.sampleRate * header.blockAlign;

    QFile pcmFile(pcmFilename);
    if (!pcmFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << pcmFilename;
        return;
    }
    header.dataChunkDataSize = pcmFile.size();
//    header.chunkDataSize = header.dataChunkDataSize + sizeof (WAVHeader) - 8;
    header.chunkDataSize = header.dataChunkDataSize + sizeof (WAVHeader) - sizeof (header.chunkDataSize) - sizeof (header.chunkId);


    QFile wavFile(wavFilename);
    if (!wavFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << wavFilename;
        pcmFile.close();
        return;
    }

    wavFile.write((const char *) &header, sizeof (header));

    char buffer[1024];
    int size;
    while ((size = pcmFile.read(buffer, sizeof (buffer))) > 0) {
        wavFile.write(buffer, size);
    }

    pcmFile.close();
    wavFile.close();
}
