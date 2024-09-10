#ifndef WRITE_EDF_FILE_H
#define WRITE_EDF_FILE_H

#include "read_signal_file.h"

class write_edf{

    int hdl;
    std::string chName;

    const char* recorderName;

    QMap<int, QMap<int, QString>> event_map;
    QMap<int, QString> systemEvent_map;
    QMap<int, QString> recorderEvent_map;
    QMap<int, QString> saveSkipEvent_map;


public:
    int write_edf_file(SignalFile *signal, QFileInfo file2write, bool anonymize, bool shorten, bool exportSystemEvents);

   private:
    int set_header_info(SignalFile *signal, bool anonymize, QFileInfo file2write);
    int set_channel_properties(SignalFile *signal);
    int set_data(SignalFile *signal);
    int set_annotations(SignalFile *signal, bool shorten, bool exportSystemEvents);


};

#endif // WRITE_EDF_FILE_H
