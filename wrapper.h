#ifndef WRAPPER_H
#define WRAPPER_H

#include "write_edf_file.h"
#include "read_signal_file.h"

class wrapper
{
public:
    wrapper();
    int readAndSaveFile(QFileInfo fileInfo, QFileInfo infoEdf, bool anonymize, bool shorten, bool exportSystemEvents);
    int readAndSaveFileChunks(QFileInfo fileInfo, QFileInfo infoEdf, bool anonymize, bool shorten, bool exportSystemEvents);
};

#endif // WRAPPER_H
