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
    // works fine on my computer, but it actually didnt need this correction in Motol XP computers
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

std::vector<Event> read_events(std::fstream &file, long offset, long size, long nevents)
{
    file.seekg(offset);
    //std::cout << "beginning of read_events: "; whereAmI(file);
    //nevents = 10240;
    std::vector<Event> events;
    short tcount;
    short h;
    unsigned char B;
    unsigned int I;
    file.read(reinterpret_cast<char *>(&tcount), sizeof(tcount));
    //cout << tcount << " tcount" << endl;
    long currsize = sizeof(tcount) + sizeof(h) + sizeof(B) + 6 * sizeof(I);
    //cout << "currsize: " << currsize << endl;
    if (currsize > size)
    {
    }
    for (int i = 0; i < nevents; i++)
    {
        if (i < tcount)
        {
            Event event;
            //whereAmI(file);
            file.read(reinterpret_cast<char *>(&event.ev_type), sizeof(event.ev_type));
            //cout << "event: " << event.desc1;
            file.read(reinterpret_cast<char *>(&event.sub_type), sizeof(event.sub_type));
            //cout << ", " << event.desc2;
            file.read(reinterpret_cast<char *>(&I), sizeof(I));
            event.page = I >> 16;
            event.page_time = (I & 0x0000ffff) / 1000.0;
            //cout << event.page << ", " << event.page_time << endl;
            file.read(reinterpret_cast<char *>(&event.time), sizeof(event.time));
            //cout << ", " << event.ev2;
            file.read(reinterpret_cast<char *>(&event.duration), sizeof(event.duration));
            //cout << ", " << event.ev3;
            file.read(reinterpret_cast<char *>(&event.duration_in_ms), sizeof(event.duration_in_ms));
            //cout << ", " << event.ev4;
            file.read(reinterpret_cast<char *>(&event.channels), sizeof(event.channels));
            //cout << ", " << event.ev5;
            file.read(reinterpret_cast<char *>(&event.info), sizeof(event.info));
            //cout << ", " << event.ev6;
            events.push_back(event);
            //cout << endl;
        }
    }
    long current = file.tellg();
    //cout << current << endl;
    //cout << "nevents-tcount = " << nevents-tcount << endl;
    file.seekg(current + (nevents - tcount) * 27);
    //file.seekg(offset + sizeof(tcount) + nevents*27);
    return events;
}

std::vector<EventDesc> read_event_descs(std::fstream &file)
{
    //std::cout << "read_event_descs: "; whereAmI(file);
    // Careful on this - need to count for empty cycles in previous struct - function read_events
    //cout << "top of read_event_desc "; whereAmI(file);
    std::vector<EventDesc> types;
    short tcount;
    short h = 0;
    char dss[20];
    char lss[6];

    file.read(reinterpret_cast<char *>(&tcount), sizeof(tcount));

    int ssize = 32;
    std::vector<std::string> desc = readChannelChar(dss, file, ssize);
    std::vector<std::string> labels = readChannelChar(lss, file, ssize);
    std::vector<short> values = readChannel(h, file, ssize);
    std::vector<short> dtypes = readChannel(h, file, ssize);

    for (int i = 0; i < tcount; i++)
    {
        EventDesc eventdesc;
        //cout << desc[i] << " " << labels[i] << " " << values[i] << " " << dtypes[i] << endl;
        eventdesc.desc = desc[i];
        eventdesc.label = labels[i];
        eventdesc.d_type = dtypes[i];
        eventdesc.value = values[i];
        types.push_back(eventdesc);
    }

    return types;
}

std::vector<Note>  read_notes(std::fstream &file, long offset, long size){

    file.seekg(offset);
    std::vector<Note> notes;
    unsigned int I;
    short tcount; // = no of notes, size of one note = 264 bytes, 256 bytes = description, 2x4 bytes (int)
    file.read(reinterpret_cast<char *>(&tcount), sizeof(tcount));
    //qDebug() << "tcount" << tcount;

    for (int i = 1; i <= tcount; i++){
        Note note;
        file.read(reinterpret_cast<char *>(&note.desc), sizeof(note.desc));
        file.read(reinterpret_cast<char *>(&I), sizeof(I));
        note.page = I >> 16;
        file.read(reinterpret_cast<char *>(&note.time), sizeof(note.time));
        notes.push_back(note);
    }
    return notes;
};

std::vector<Event> get_selected_events_4_types(std::vector<Event> events, int type)
{
    std::vector<Event> selected;

    for (int i = 0; i < events.size(); i++)
    {
        if (events[i].ev_type == type)
        {
            selected.push_back(events[i]);
        }
    }
    return selected;
}

void read_display_montages(std::fstream &file, long offset, long size){
    file.seekg(offset);
    //std::cout << "display montages start at " << offset<<endl;
    //std::cout << "display montages size: " << size<<endl;

    //std::cout << "beginning read_display_montages: "; whereAmI(file);
    //nevents = 10240;
    short tcount;
    file.read(reinterpret_cast<char *>(&tcount), sizeof(tcount)); // some kind of buffer?

    //cout << "tcount: "<<tcount << endl;
    //whereAmI(file);

    char montageName[33];
    char lead[9];

    file.read(reinterpret_cast<char *>(&montageName), sizeof(montageName));

    //cout << "montage name: " << montageName << endl;

    file.seekg(1120,std::ios_base::cur);
    std::vector<std::string> leads = readChannelChar(lead, file, 24);

    for(int i = 0; i < leads.size(); i++){
        //file.read(reinterpret_cast<char *>(&lead), sizeof(lead));
        //cout << leads[i] << endl;
    }
    //std::cout << endl;

}

Spages read_signal_pages(std::fstream &file, bool read_signal_data, long file_size, long offset, int page_size, int epoch_length, int channels_used, std::vector<Channel> channels)
{
    //cout << "Begining of read_signal_pages: "; whereAmI(file);
    //cout << "Channels used: " << channels_used << endl;

    int header_length = 6;
    short h = 0;
    int num_pages = int((file_size - offset) / page_size);
    //std::cout << "save_buffer_size " << channels[1].save_buffer_size << endl;
    //std::vector<std::vector<double>> esignals(channels_used, std::vector<double>(channels[1].save_buffer_size * num_pages, 0)); // this is not allocating
    std::vector<std::vector<double>> esignals(channels_used, std::vector<double>()); //originally "signals"

    // allocating done right
    for(int ch = 0; ch < channels_used; ch++){
        esignals[ch].reserve(channels[1].save_buffer_size * num_pages);
    }

    std::vector<SignalPage> pages;
    //std::cout << "num pages: " << num_pages << std::endl;

    header_length = 6;

    int current_offset = offset;
    bool stop = false;
    file.seekg(current_offset);

    //std::cout << "Begining of signal reading read_signal_pages: "; whereAmI(file);

    int curr_page = -1;

    while (!stop && !file.eof())
    {
        curr_page += 1;
        //std::cout << "Current page: " << curr_page << std::endl;
        SignalPage page;
        file.read(reinterpret_cast<char *>(&page.filling), sizeof(page.filling));
        file.read(reinterpret_cast<char *>(&page.time), sizeof(page.time));
        pages.push_back(page); // or should I use insert?
        //std::cout << "page filling: " << page.filling << " page time: " << page.time << std::endl;
        //std::cout << "page time: " << page.time << endl;

        if (page.filling != 0)
        {
            stop = true;
            //std::cout << "Stop is true!" << std::endl;
        }
        else
        {
            int data_size = page_size - header_length; // what is this for?
        }
        if (!stop)
        {
            current_offset += page_size;
            //cout << "Current offset: " << current_offset <<endl;
            if (read_signal_data)
            {
                for (int i = 0; i < channels_used; i++)
                {
                    //std::cout << "Channel buffer size in channel no "<< i+1 << " is " << channels[i].save_buffer_size << std::endl;
                    std::vector<short> b = readChannel(h, file, channels[i].save_buffer_size);
                    int start_index = curr_page * channels[i].save_buffer_size;
                    esignals[i].insert(esignals[i].begin() + start_index, b.begin(), b.end());
                }
            }
            else
            {
                file.seekg(current_offset);
            }
        }
        //qDebug() << esignals[0].size();
    }

    if (read_signal_data)
    {
        for (int i = 0; i < channels_used; i++)
        {
            double cal_factor = channels[i].cal_factor;
            double cal_offset = channels[i].cal_offset;
            std::transform(esignals[i].begin(), esignals[i].end(), esignals[i].begin(), [&cal_factor](double &c) { return c * cal_factor; });
            std::transform(esignals[i].begin(), esignals[i].end(), esignals[i].begin(), [&cal_offset](double &c) { return c + cal_offset; });
        }
    }

    //qDebug() << esignals[0].size() << "at the end";

    Spages spages;
    spages.pages = pages;
    spages.esignals = esignals;
    return spages;
}

SignalFile read_signal_file(QFileInfo fileInfo){

    // get file size
    const long long file_size = fileInfo.size();

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

    //Events
    //cout << "Moving to: " << data_table.events_info.offset << endl;
    //cout << "Block size: " << data_table.events_info.size << endl;
    signal.events = read_events(file, signal.data_table.events_info.offset, signal.data_table.events_info.size, 2048);


    //Event desc
    // get to 61776 by reading additional 27 bytes for every missing event (maximum is 2048) in read_events
    signal.events_desc = read_event_descs(file);

    //whereAmI(file);
    signal.notes = read_notes(file, signal.data_table.notes_info.offset, signal.data_table.notes_info.size);


    std::map<int, std::string> EventDict;

    EventDict[1] = "ET_SAVESKIPEVENT";
    EventDict[2] = "ET_SYSTEMEVENT";
    EventDict[3] = "ET_USEREVENT";
    EventDict[4] = "ET_DIGINPEVENT";
    EventDict[5] = "ET_RECORDEREVENT";
    EventDict[6] = "ET_RESPIRATIONEVENTS";
    EventDict[7] = "ET_SATURATIONEVENTS";
    EventDict[8] = "ET_ECGEVENTS";
    EventDict[9] = "ET_EMGEVENTS";
    EventDict[10] = "ET_EEG_DELTAEVENTS";
    EventDict[11] = "ET_EEG_SPINDLEEVENTS";
    EventDict[12] = "ET_EEG_ALPHAEVENTS";
    EventDict[13] = "ET_EOGEVENTS";
    EventDict[14] = "ET_EEG_THETAEVENTS";
    EventDict[15] = "ET_EEG_BETAEVENTS";
    EventDict[16] = "ET_AROUSALEVENTS";
    EventDict[17] = "ET_SOUNDEVENTS";
    EventDict[18] = "ET_BODYPOSITIONEVENTS";
    EventDict[19] = "ET_CPAPEVENTS";

    std::vector<Event> store_events_list = get_selected_events_4_types(signal.events, 1);

    signal.store_events = store_events_list.size();

    // TO DO - read additional montages at position signal.data_table.display_montages_info.offset;
    read_display_montages(file, signal.data_table.display_montages_info.offset, signal.data_table.display_montages_info.size);

    //spages
    // TO DO - construct time vector from signal_pages
    Spages spages = read_signal_pages(file, true, file_size, signal.data_table.signal_info.offset, signal.data_table.signal_info.size, 30, signal.recorder_info.numberOfChannelsUsed, signal.recorder_info.channels);
    signal.signal_pages = spages.pages;
    signal.signal_data = spages.esignals;

    file.close();

    signal.check = true;
    return signal;
}
