#include "read_signal_file.h"

// function definitions

QDateTime decode_date_time(long date, long time)
{
    //date = days since BC - but with some constraints to account for year with 31*12 days (and 31 days in month)
    int y = floor(date / (31 * 12));
    int m = floor(date / 31);
    int d = date % 31;
    m = m % 12;

    //qDebug() << y << " " << m << " " << d;

    if (d == 0)
    {
        d = 31;
        m--;
    }

    if (m == 0)
    {
        m = 12;
        y--;
    }

    //qDebug() << y << " " << m << " " << d;

    // time = ms since midnight

    int h = floor(time / (60 * 60 * 1000));
    int min = floor(time / (60 * 1000));
    min = min % 60;
    int s = floor(time / 1000);
    s = s % 60;
    int ms = time % 1000;
    //qDebug() << h << " " << min << " " << s << " " << ms;


    QDate startDate(y, m, d);
    QTime startTime(h, min, s, ms);
    QDateTime startDateTime = QDateTime(startDate,startTime).toLocalTime();

    // this is kinda hack - old BrainLab does not account for time transition to daylight saving time, I need to correct it manually
    // works fine on my computer
    //if(startDateTime.isDaylightTime()){
        //qDebug() << "before " << startDateTime;
      //  startDateTime = startDateTime.addSecs(3600);
        //qDebug() << "after " << startDateTime;
    //}

    return startDateTime;
};

template <typename T>
std::vector<T> readChannel(T tch, std::fstream &file, int nch)
{
    std::vector<T> x;
    for (int i = 0; i < nch; i++)
    {
        file.read(reinterpret_cast<char *>(&tch), sizeof(tch));
        x.push_back(tch);
        //cout << x[i] << " ";
    }
    //cout <<endl;
    return x;
}

template <typename ByteArray>
std::vector<std::string> readChannelChar(ByteArray &a, std::fstream &file, int nch)
{
    std::vector<std::string> x;
    for (int i = 0; i < nch; i++)
    {
        file.read(reinterpret_cast<char *>(&a), sizeof(a));
        x.push_back(a);
    }
    return x;
}

void whereAmI(std::fstream &file)
{
    int testing = 1;
    if(testing){
        std::cout << "Position in file: " << file.tellg() << std::endl;
    }
}

SignalHeader read_signal_header(std::fstream &file)
{
    SignalHeader stud;
    file.read(reinterpret_cast<char *>(&stud.program_id), sizeof(stud.program_id));
    file.read(reinterpret_cast<char *>(&stud.signal_id), sizeof(stud.signal_id));
    file.read(reinterpret_cast<char *>(&stud.version_id), sizeof(stud.version_id));
    file.read(reinterpret_cast<char *>(&stud.read_only), sizeof(stud.read_only));
    return stud;
}

DataTable read_data_table(std::fstream &file)
{
    DataTable data_table;
    long y = 0;
    std::vector<long> dt = readChannel(y, file, 17);
    data_table.measurement_info = Block{dt[0], dt[1], 0};
data_table.recorder_montage_info = Block{dt[2], dt[3],0};
data_table.events_info = Block{dt[4], dt[5],0};
data_table.notes_info = Block{dt[6], dt[7],0};
data_table.impedance_info = Block{dt[8], dt[9],0};
data_table.display_montages_info = Block{dt[10], dt[11],0};
data_table.stimulator_info = Block{dt[12], dt[13],0};
data_table.signal_info = Block{dt[14], dt[15], dt[16]};
return data_table;
}

Measurement read_measurement(std::fstream &file, long offset)
{
    Measurement measurement;
    file.seekg(offset);
    file.read(measurement.id, sizeof(measurement.id));
    file.read(measurement.name, sizeof(measurement.name));
    file.read(measurement.street, sizeof(measurement.street));
    file.read(measurement.zip_code, sizeof(measurement.zip_code));
    file.read(measurement.city, sizeof(measurement.city));
    file.read(measurement.state, sizeof(measurement.street));
    file.read(measurement.country, sizeof(measurement.street));
    file.read(reinterpret_cast<char *>(&measurement.birthday), sizeof(measurement.birthday));
    file.read(reinterpret_cast<char *>(&measurement.sex), sizeof(measurement.sex));
    file.read(reinterpret_cast<char *>(&measurement.start_date), sizeof(measurement.start_date));
    file.read(reinterpret_cast<char *>(&measurement.start_hour), sizeof(measurement.start_hour));
    file.read(measurement.room, sizeof(measurement.room));
    file.read(measurement.doctor, sizeof(measurement.doctor));
    file.read(measurement.technician, sizeof(measurement.technician));
    file.read(measurement.class_code, sizeof(measurement.class_code));
    file.read(measurement.clin_info, sizeof(measurement.clin_info));
    file.read(measurement.backup_flag, sizeof(measurement.backup_flag));
    file.read(reinterpret_cast<char *>(&measurement.archive_flag), sizeof(measurement.archive_flag));
    file.read(reinterpret_cast<char *>(&measurement.vcr_timing_correction), sizeof(measurement.vcr_timing_correction));
    file.read(measurement.referring_doctor_name, sizeof(measurement.referring_doctor_name));
    file.read(measurement.referring_doctor_code, sizeof(measurement.referring_doctor_code));
    file.read(reinterpret_cast<char *>(&measurement.weight), sizeof(measurement.weight));
    file.read(reinterpret_cast<char *>(&measurement.height), sizeof(measurement.height));
    file.read(reinterpret_cast<char *>(&measurement.weight_unit), sizeof(measurement.weight_unit));
    file.read(reinterpret_cast<char *>(&measurement.height_unit), sizeof(measurement.height_unit));
    file.read(measurement.protocol, sizeof(measurement.protocol));
    file.read(reinterpret_cast<char *>(&measurement.maximum_voltage), sizeof(measurement.maximum_voltage));
    file.read(reinterpret_cast<char *>(&measurement.maximum_amplitude), sizeof(measurement.maximum_amplitude));
    return measurement;
}

RecorderMontageInfo read_recorder_info(std::fstream &file, long offset)
{
    RecorderMontageInfo recorder_info;
    file.seekg(offset);
    file.read(recorder_info.name, sizeof(recorder_info.name));
    file.read(reinterpret_cast<char *>(&recorder_info.nRecChannels), sizeof(recorder_info.nRecChannels));
    file.read(reinterpret_cast<char *>(&recorder_info.invertedACChannels), sizeof(recorder_info.invertedACChannels));
    file.read(reinterpret_cast<char *>(&recorder_info.maximumVoltage), sizeof(recorder_info.maximumVoltage));
    file.read(reinterpret_cast<char *>(&recorder_info.normalVoltage), sizeof(recorder_info.normalVoltage));
    file.read(reinterpret_cast<char *>(&recorder_info.calibrationSignal), sizeof(recorder_info.calibrationSignal));
    file.read(reinterpret_cast<char *>(&recorder_info.calibrationScale), sizeof(recorder_info.calibrationScale));
    file.read(reinterpret_cast<char *>(&recorder_info.videoControl), sizeof(recorder_info.videoControl));
    file.read(reinterpret_cast<char *>(&recorder_info.nSensitivities), sizeof(recorder_info.nSensitivities));
    file.read(reinterpret_cast<char *>(&recorder_info.nLowFilters), sizeof(recorder_info.nLowFilters));
    file.read(reinterpret_cast<char *>(&recorder_info.nHighFilters), sizeof(recorder_info.nHighFilters));

    float z = 0;
    recorder_info.sensitivity = readChannel(z, file, 20);
    recorder_info.lowFilter = readChannel(z, file, 20);
    recorder_info.highFilter = readChannel(z, file, 20);

    // <33s2bhH
    unsigned char b;
    file.read(reinterpret_cast<char *>(&recorder_info.montageName), sizeof(recorder_info.montageName)); // 33s
    file.read(reinterpret_cast<char *>(&b), sizeof(b)); // b
    recorder_info.numberOfChannelsUsed = b;
    file.read(reinterpret_cast<char *>(&b), sizeof(b)); //b
    recorder_info.globalSens = b;
    file.read(reinterpret_cast<char *>(&recorder_info.epochLengthInSamples), sizeof(recorder_info.epochLengthInSamples)); //h
    file.read(reinterpret_cast<char *>(&recorder_info.highestRate), sizeof(recorder_info.highestRate)); //H

    // channels
    int nch = 32;
    short h = 0;
    unsigned short H = 0;
    char uch[5];
    char st[9];
    char stth[13];

    std::vector<unsigned short> sampling_rate = readChannel(H, file, nch);
    std::vector<std::string> signal_type = readChannelChar(st, file, nch);
    std::vector<std::string> signal_sub_type = readChannelChar(st, file, nch);
    std::vector<std::string> channel_desc = readChannelChar(stth, file, nch);
    std::vector<unsigned short> sensitivity_index = readChannel(H, file, nch);
    std::vector<unsigned short> low_filter_index = readChannel(H, file, nch);
    std::vector<unsigned short> high_filter_index = readChannel(H, file, nch);
    std::vector<unsigned short> delay = readChannel(H, file, nch);
    std::vector<std::string> unit = readChannelChar(uch, file, nch);
    std::vector<short> artefact_level = readChannel(h, file, nch);
    std::vector<short> cal_type = readChannel(h, file, nch);
    std::vector<float> cal_factor = readChannel(z, file, nch);
    std::vector<float> cal_offset = readChannel(z, file, nch);
    std::vector<unsigned short> save_buffer_size = readChannel(H, file, nch);


    for (int i = 0; i < nch; i++)
    {
        Channel channel;
        channel.sampling_rate = sampling_rate[i];
        channel.signal_type = signal_type[i];
        channel.signal_sub_type = signal_sub_type[i];
        channel.channel_desc = channel_desc[i];
        channel.sensitivity_index = sensitivity_index[i];
        channel.low_filter_index = low_filter_index[i];
        channel.high_filter_index = high_filter_index[i];
        channel.delay = delay[i];
        channel.unit = unit[i];
        channel.cal_type = cal_type[i];
        channel.cal_factor = cal_factor[i];
        channel.cal_offset = cal_offset[i];
        channel.artefact_level = artefact_level[i];
        channel.save_buffer_size = save_buffer_size[i];
        recorder_info.channels.push_back(channel);
    }
    //cout << "end of read_recorder_info: ";
    //whereAmI(file);
    return recorder_info;
}

SignalFile read_signal_file(QFileInfo fileInfo){

    // get file size
    //const long long file_size = fileInfo.size();

    // Signal struct
    SignalFile signal;

    // READ THE FILE
    std::fstream file(fileInfo.filePath().toLocal8Bit(), std::ios::in | std::ios::out | std::ios::binary);

    //qDebug() << fileInfo.filePath();

    if (file.fail())
    {
        qDebug() << "ERROR: Cannot open the file...";
        return signal;
    }

    // Signal header
    signal.header = read_signal_header(file);

    // check if it is BrainLab *.SIG file
    if (signal.header.program_id != 1096045395){
        qDebug() << fileInfo.filePath() << " is not valid BrainLab file, skipping";
        return signal;
    }

    // Data table
    signal.data_table = read_data_table(file);

    //whereAmI(file); // should be 80
    // Measurement
    signal.measurement = read_measurement(file, signal.data_table.measurement_info.offset);

    //Recorder info
    signal.recorder_info = read_recorder_info(file, signal.data_table.recorder_montage_info.offset);

    file.close();

    return signal;
}
