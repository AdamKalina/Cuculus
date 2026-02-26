#ifndef WRITE_EDF_FILE_H
#define WRITE_EDF_FILE_H

#include "wrapper.h"
#include "read_signal_file.h"
#include <QFileInfo>

class read_signal_file;

class write_edf{

    int hdl;
    std::string chName;

    const char* recorderName;

    QMap<int, QMap<int, QString>> event_map;
    QMap<int, QString> systemEvent_map;
    QMap<int, QString> recorderEvent_map;
    QMap<int, QString> saveSkipEvent_map;


public:
    int write_edf_file(read_signal_file::SignalFile *signal, QFileInfo file2write, bool anonymize, bool shorten, bool exportSystemEvents);
    int set_header_info(read_signal_file::SignalFile *signal, bool anonymize, QFileInfo file2write);
    int set_channel_properties(read_signal_file::SignalFile *signal);
    int set_data(read_signal_file::SignalFile *signal);
    int set_data_chunk(std::vector<std::vector<double>> esignals, int SMP_FREQ, int numberOfChannelsUsed);
    int set_annotations(read_signal_file::SignalFile *signal, bool shorten, bool exportSystemEvents);
    int write_edf_header(read_signal_file::SignalFile *signal, QFileInfo file2write, bool anonymize);
    int close_file();
};

#endif // WRITE_EDF_FILE_H
