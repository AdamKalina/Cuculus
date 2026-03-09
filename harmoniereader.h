#ifndef HARMONIEREADER_H
#define HARMONIEREADER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>

struct TimelineSection {
    QDateTime start;
    QDateTime end;
    double durationSeconds;
    size_t start_block;
    size_t end_block;
    size_t no_of_blocks;
};

struct Annotation {
    QDateTime start;
    QDateTime end;
    QString text;
    bool system;
};

struct ChannelInfo {
    std::string label;
    int sampling_rate;
    int type;
};

class HarmonieReader
{
    std::fstream sig_file;
    std::fstream sts_file;
    QFileInfo sig_file_info;
    QFileInfo sts_file_info;

    int numChannels = 0; // number of chanels used in montage
    int recordChannels = 0; // True Physical Channel Count
    int recordHeaderSize = 8; // Size of timestamp/metadata per block in .SIG file
    std::vector<int> samplingRates;
    std::vector<ChannelInfo> channels;
    std::vector<double> decimatedTimeline;
    std::vector<TimelineSection> sections;
    std::vector<Annotation> annotations;
    std::vector<std::string> patient_info;

public:
    HarmonieReader();
    int readHarmonieFile(QFileInfo info);
    void reconstructTimeline();
    int exportToEdf(QFileInfo infoEdf, bool exportSystemAnnotations = true, bool anonymize = false);

private:
    void readPatientInfo();
    void readHarmonieChannelsInfo();
    void readRecordChannelCount();
    QDateTime decodeTime(double start_time);
    int sexFromID(std::string id);
    double readStartingTime();
    void readAnnotations();
    void readHeader();
    std::vector<int> birthDateFromID(std::string id);
};

#endif // HARMONIEREADER_H
