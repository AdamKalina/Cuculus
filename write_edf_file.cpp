/*
*****************************************************************************
*
* Cuculus is command line application for converting EEG *.SIG files (Schwarzer BrainLab) to *.EDF
* Copyright (C) 2022  Adam Kalina
* email: adam.kalina89@gmail.com
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
*****************************************************************************
*/

#include "edflib.h"
#include "read_signal_file.h"

inline int write_edf_file(SignalFile *signal, QFileInfo file2write, bool anonymize, bool shorten, bool exportSystemEvents) //is that inline a hack - TO DO?
{
    int hdl;
    std::string chName;

    const char* recorderName;

    QMap<int, QMap<int, QString>> event_map;
    QMap<int, QString> systemEvent_map;
    QMap<int, QString> recorderEvent_map;
    QMap<int, QString> saveSkipEvent_map;


    // I am putting here only events not related to PSG. And even from those I use only few

    saveSkipEvent_map[0] = "Store event";
    systemEvent_map[77] = "Montage change";
    recorderEvent_map[86] = "Start video recording";
    recorderEvent_map[118] = "Stop video recording";

    event_map[1] = saveSkipEvent_map; // ET_SAVESKIPEVENT = whole recording?
    event_map[2] = systemEvent_map;//"ET_SYSTEMEVENT";
    //event_map[3] = "ET_USEREVENT";
    //event_map[4] = "ET_DIGINPEVENT";
    event_map[5] = recorderEvent_map;//"ET_RECORDEREVENT";

    qDebug() << QString::fromLocal8Bit(signal->measurement.name);

    hdl = edfopen_file_writeonly(file2write.absoluteFilePath().toLocal8Bit(), EDFLIB_FILETYPE_EDFPLUS, signal->recorder_info.numberOfChannelsUsed);

    if(hdl<0)
    {
        printf("error: edfopen_file_writeonly()\n");

        return(1);
    }

    // ======== SETTING HEADER INFO ========

    if(!anonymize){


        if(edf_set_patientname(hdl, signal->measurement.name)){
            printf("error: edf_set_patientname()\n");

            return(1);
        }

        if(edf_set_patientcode(hdl, signal->measurement.id)){
            printf("error: edf_set_patientcode()\n");

            return(1);
        }

        if(edf_set_gender(hdl, signal->measurement.sex)){
            printf("error: edf_set_gender()\n");

            return(1);
        }


        /* Sets the birthdate.
   * year: 1800 - 3000, month: 1 - 12, day: 1 - 31
   * This function is optional
   * Returns 0 on success, otherwise -1
   * This function is optional and can be called only after opening a file in writemode and before the first sample write action
   */

        std::string str = signal->measurement.id;
        std::string year = str.substr(0, 2);
        std::string month = str.substr(2, 2);
        std::string day = str.substr(4, 2);

        int m = atoi(month.c_str());
        int y = atoi(year.c_str());
        int d = atoi(day.c_str());

        if(m > 12){
            m -= 50;
        }

        if(y < 54 && str.length() == 10){ // rodna cisla byla devitimistna do roku 1954
            y += 2000;
        }else{
            y += 1900;
        }

        if(edf_set_birthdate(hdl, y, m, d)){
            printf("error: edf_set_birthdate()\n");

            return(1);
        }

        //QDateTime startTime =  decode_date_time(signal->measurement.start_date,signal->measurement.start_hour); // this is the startdatetime stored in the header - but actually the file starts with the first store event

        QDateTime startTime = decode_date_time(signal->measurement.start_date, signal->signal_pages[0].time);


        if(edf_set_startdatetime(hdl, startTime.date().year(), startTime.date().month(), startTime.date().day(), startTime.time().hour(), startTime.time().minute(), startTime.time().second())){
            printf("error: edf_set_startdatetime()\n");

            return(1);
        }

        /* Sets the startdate and starttime.
   * year: 1985 - 2084, month: 1 - 12, day: 1 - 31
   * hour: 0 - 23, minute: 0 - 59, second: 0 - 59
   * If not called, the library will use the system date and time at runtime
   * Returns 0 on success, otherwise -1
   * This function is optional and can be called only after opening a file in writemode
   * and before the first sample write action
   * Note: for anonymization purposes, the consensus is to use 1985-01-01 00:00:00 for the startdate and starttime.
   */

        if(edf_set_patient_additional(hdl, signal->measurement.doctor)){ // this might not make sense, but in Motol we use this field for record labeling
            printf("error: edf_set_patient_additional\n");

            return(1);
        }

        if(edf_set_technician(hdl, signal->measurement.technician)){
            printf("error: edf_set_technician()\n");

            return(1);
        }

    }


    if(edf_set_admincode(hdl, file2write.baseName().toLocal8Bit())){
        printf("error: edf_set_admincode\n");

        return(1);
    }

    if (strcmp(signal->recorder_info.name, "Schwarzer PTMS1") == 0){ // truncating this so there is enough space for measurement.class_code
        recorderName = "Schwarzer";
    }
    else{
        recorderName = signal->recorder_info.name;
    }


    if(edf_set_equipment(hdl, recorderName)){
        printf("error: edf_set_equipment()\n");

        return(1);
    }

    if(edf_set_recording_additional(hdl, signal->measurement.class_code)){ // max 9 characters in Brainlab header
        printf("error: edf_set_recording_additional\n");

        return(1);
    }

    // ======== SETTING CHANNEL PROPERTIES ========

    for(int ch_index = 0; ch_index < signal->recorder_info.numberOfChannelsUsed; ch_index++){

        Channel *recinf = &signal->recorder_info.channels[ch_index];

        //QString chLabel = QString::fromStdString(recinf->channel_desc);

        QString chLabel = QString::fromStdString(signal->recorder_info.displayMontage.leads[ch_index]); //use display montage labels

        chLabel.replace("/","-");

        if(chLabel == "In1a-In1b"){ // I guess this is recorder specific
            chName = "ECG";
        }
        else{
            chName = recinf->signal_type + " " + chLabel.toStdString();
        }

        //qDebug() << QString::fromStdString(chName);

        if(edf_set_samplefrequency(hdl, ch_index, recinf->sampling_rate))
        {
            printf("error: edf_set_samplefrequency() in channel %d \n", ch_index);

            return(1);
        }

        if(edf_set_physical_maximum(hdl, ch_index, 3277))
        {
            printf("error: edf_set_physical_maximum() in channel %d \n", ch_index);

            return(1);
        }

        if(edf_set_physical_minimum(hdl, ch_index, -3277))
        {
            printf("error: edf_set_physical_minimum() in channel %d \n", ch_index);

            return(1);
        }

        if(edf_set_digital_maximum(hdl, ch_index, 32767))
        {
            printf("error: edf_set_digital_maximum() in channel %d \n", ch_index);

            return(1);
        }

        if(edf_set_digital_minimum(hdl, ch_index, -32767))
        {
            printf("error: edf_set_digital_minimum() in channel %d \n", ch_index);

            return(1);
        }

        if(edf_set_label(hdl, ch_index, chName.c_str()))
        {
            printf("error: edf_set_label() in channel %d \n", ch_index);

            return(1);
        }

        if(edf_set_physical_dimension(hdl, ch_index, recinf->unit.c_str()))
        {
            printf("error: edf_set_physical_dimension() in channel %d \n", ch_index);

            return(1);
        }

        // not used: edf_set_prefilter, edf_set_transducer
        // temp_filterStringHeader = 'HP ' + str(sf.recorder_info.highFilter[recinf.high_filter_index]) + ' ' + 'LP ' + str(sf.recorder_info.lowFilter[recinf.low_filter_index])

    }

    // ======== WRITING SAMPLES ========

    // calculate number of blocks, one block length = samplefrequency
    //qDebug() << "length of signal" << signal->signal_data[1].size();

    //long long nBlocks = (signal->signal_data[1].size() - std::count(signal->signal_data[1].begin(), signal->signal_data[1].end(), 0))/SMP_FREQ;//number of non-zero samples/sampling frequency
    //qDebug() << "lengthOfFile" << nBlocks;

    int SMP_FREQ = int(signal->recorder_info.channels[0].sampling_rate); // I hope that sampling rate is same in all signals... SMP_FREQ used to be hardcoded to 250 Hz but it changes ()

    int nBlocks = signal->signal_data[0].size()/SMP_FREQ;
    //double buf[SMP_FREQ];

    //std::vector<double> buf;
    //buf.reserve(SMP_FREQ);

    std::vector<double> buf (SMP_FREQ, 0);

    for(int block_index = 0; block_index < nBlocks; block_index++){ // iterate over blocks
        double iBeg = block_index*SMP_FREQ;

        for(int ch_index = 0; ch_index < signal->recorder_info.numberOfChannelsUsed; ch_index++){ // iterate over channels

            Channel *recinf = &signal->recorder_info.channels[ch_index];

            //qDebug() << recinf->sampling_rate;

            for(int buf_index = 0; buf_index < recinf->sampling_rate; buf_index++){
                //qDebug() << "buf index: " << buf_index << "| ch_index: "<< ch_index << "| iBeg: " << iBeg;
                buf[buf_index] = signal->signal_data[ch_index][iBeg+buf_index]* 100; //* chFactor - why?
            }

            if(edfwrite_physical_samples(hdl, buf.data()))
            {
                printf("error: edfwrite_physical_samples() block %d, channel %d\n",block_index,ch_index);

                return(1);
            }
        }
    }

    // ======== WRITING ANNOTATIONS ========

    //int edfwrite_annotation_utf8(int handle, long long onset, long long duration, const char *description);
    /* writes an annotation/event to the file
     * onset is relative to the start of the file
     * onset and duration are in units of 100 microSeconds!     resolution is 0.0001 second!
     * for example: 34.071 seconds must be written as 340710
     * if duration is unknown or not applicable: set a negative number (-1)
     * description is a null-terminated UTF8-string containing the text that describes the event
     * This function is optional and can be called only after opening a file in writemode
     * and before closing the file
     */

    //qDebug() << "events size" << signal->events.size();
    QString label;

    for(unsigned int j = 0; j < signal->events.size(); j++){

        if(signal->events.at(j).ev_type == 3){


            int ev_subtype = signal->events.at(j).sub_type;
            ev_subtype -= 100; // turns 102 into 2 and so on, corresponds to events_desc
            int onset_in_s = (signal->events.at(j).page - 1) * signal->recorder_info.epochLengthInSamples / float(signal->recorder_info.highestRate) + signal->events.at(j).page_time;

            if(shorten){ // option to write just shortened labels of events, e.g. OO, ZO, HVn, HVu
                label = QString::fromLocal8Bit(signal->events_desc.at(ev_subtype).label.data());
            }else{
                label = QString::fromLocal8Bit(signal->events_desc.at(ev_subtype).desc.data());
            }
            edfwrite_annotation_utf8(hdl, onset_in_s*10000, signal->events.at(j).duration_in_ms*10, label.toUtf8());


        }
        else{
            if(exportSystemEvents)
            {

                //                                                qDebug() << "event no: " << j;
                //                                                qDebug() << "info: "<<signal->events.at(j).info; // always one
                //                                                qDebug() << "channels: " << signal->events.at(j).channels;
                //                                                qDebug() << "page: "<<signal->events.at(j).page << ", page_time: "<<signal->events.at(j).page_time;
                //                                                qDebug() << "time: "<<signal->events.at(j).time;
                //                                                qDebug() << "duration: " << signal->events.at(j).duration << ", duration in ms: " <<signal->events.at(j).duration_in_ms;
                //                                                qDebug() << "end time: " <<signal->events.at(j).end_time;
                //                                                qDebug() << "ev type: " << signal->events.at(j).ev_type << ", sub type: "<<signal->events.at(j).sub_type;

                //                                                char character = char(signal->events.at(j).sub_type); //unsigned char (ASCII value) to char - used in original convertSIGtoEDF.py
                //                                                qDebug()<<character;
                //                                                qDebug() << "----";
                int onset_in_s = (signal->events.at(j).page - 1) * signal->recorder_info.epochLengthInSamples / float(signal->recorder_info.highestRate) + signal->events.at(j).page_time;
                //label = event_map[signal->events.at(j).ev_type][signal->events.at(j).sub_type]; // but this not allow using default value
                label = event_map.value((signal->events.at(j).ev_type)).value(signal->events.at(j).sub_type,"unknown event");

                if(signal->events.at(j).ev_type == 1){ // do not use "Store event" - it colors the whole recording
                    continue;
                }
                edfwrite_annotation_utf8(hdl, onset_in_s*10000, signal->events.at(j).duration_in_ms*10, label.toUtf8());


            }
        }
    }

    for(unsigned int k = 0; k < signal->notes.size(); k++){
        int onset_in_s = (signal->notes.at(k).page - 1) * signal->recorder_info.epochLengthInSamples / float(signal->recorder_info.highestRate);
        QString label = QString::fromLocal8Bit(signal->notes[k].desc); // while truncating it replaces last character with <?> - sometimes?
        edfwrite_annotation_utf8(hdl, onset_in_s*10000, -1, label.toUtf8());
    }

    edfclose_file(hdl);

    //qDebug() << "finished";

    return(0);
}
