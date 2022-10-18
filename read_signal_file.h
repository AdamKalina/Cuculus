#ifndef READ_SIGNAL_FILE_H
#define READ_SIGNAL_FILE_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <locale>
#include <cmath>
#include <math.h>
#include <time.h>
//#include <string.h>
#include <type_traits>
#include <cstdint>
#include <functional>
#include <algorithm>
//#include <QString>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

// structs definitions

struct SignalPage
{
    unsigned short filling;
    long time;
};

struct Spages
{
    std::vector<std::vector<double>> esignals;
    std::vector<SignalPage> pages;
};

struct Event
{
    short ev_type;
    unsigned char sub_type;
    unsigned int page;
    float page_time;
    unsigned int time;
    unsigned int duration;
    unsigned int duration_in_ms;
    unsigned int channels;
    unsigned int info;
    unsigned int end_time = time + duration_in_ms;
};

struct Note{
    char desc[256];
    int page;
    int time; // maybe?
};

struct EventDesc
{
    int DT_MEASURE = 0;
    int DT_EXTERNAL = 1;
    int DT_SCAN = 2;

    //DT_DICT = {DT_MEASURE: "DT_MEASURE", DT_EXTERNAL: "DT_EXTERNAL", DT_SCAN: "DT_SCAN"}

    std::string desc;
    std::string label;
    int d_type;
    int value;
};

struct SignalHeader
{
    long program_id;
    long signal_id;
    short version_id;
    short read_only;
};

struct Measurement
{
    char id[17];
    char name[33];
    char street[33];
    char zip_code[17];
    char city[33];
    char state[33];
    char country[33];
    long birthday;
    short sex;
    long start_date;
    long start_hour;
    char room[9];
    char doctor[33];
    char technician[33];
    char class_code[9];
    char clin_info[1963];
    char backup_flag[1];
    short status_flags;
    char archive_flag[1];
    float vcr_timing_correction;
    char referring_doctor_name[33];
    char referring_doctor_code[33];
    short weight;
    short height;
    short weight_unit;
    short height_unit;
    char protocol[33];
    short maximum_voltage;
    short maximum_amplitude;
};

struct Block
{
    long offset;
    long size;
    long header_size;
};

struct DataTable
{
    Block measurement_info;
    Block recorder_montage_info;
    Block events_info;
    Block notes_info;
    Block impedance_info;
    Block display_montages_info;
    Block stimulator_info;
    Block signal_info;
};

struct Channel
{
    double sampling_rate;
    std::string signal_type;
    std::string signal_sub_type;
    std::string channel_desc;
    float sensitivity_index;
    float low_filter_index;
    float high_filter_index;
    double delay;
    std::string unit;
    short artefact_level;
    short cal_type;
    float cal_factor;
    float cal_offset;
    double save_buffer_size;
};

struct RecorderMontageInfo
{
    char name[33];
    unsigned char nRecChannels[1];
    unsigned char invertedACChannels[1];
    short maximumVoltage;
    short normalVoltage;
    short calibrationSignal;
    short calibrationScale;
    short videoControl;
    unsigned short nSensitivities;
    unsigned short nLowFilters;
    unsigned short nHighFilters;
    std::vector<float> sensitivity; // vector of 20 floats
    std::vector<float> lowFilter;
    std::vector<float> highFilter;
    char montageName[33];
    int numberOfChannelsUsed;
    int globalSens;
    short epochLengthInSamples;
    unsigned short highestRate;
    std::vector<Channel> channels;
    int parameter = 0; // from here - not used?
    std::string displayMontageName = "";
    std::string dispCh;
    std::string dispChScale;
    std::string electrode = "";
    std::string lead;
    int gain;
    int offset;
    int nChannelsOnDisplay = 0;
    int sampleMap;
    std::string dummy = "";
};

struct SignalFile
{
    SignalHeader header;
    DataTable data_table;
    Measurement measurement;
    RecorderMontageInfo recorder_info;
    std::vector<Event> events;
    std::vector<EventDesc> events_desc;
    std::vector<Note> notes;
    int store_events;
    std::vector<std::vector<double>> signal_data;
    std::vector<SignalPage> signal_pages;
    bool check = false;
};

QDateTime decode_date_time(long date, long time);

SignalFile read_signal_file(QFileInfo fileInfo);

#endif
