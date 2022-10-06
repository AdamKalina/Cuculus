#include "edflib.h"
#include "read_signal_file.h"

#define SMP_FREQ   (250) // hardcoded for Motol BrainLab signals

inline int write_edf_file(SignalFile *signal, QFileInfo file2write, bool anonymize) //is that inline a hack - TO DO?
{
    int hdl;

    const char* recorderName;

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

        QDateTime startTime =  decode_date_time(signal->measurement.start_date,signal->measurement.start_hour);

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

        std::string chName = recinf->signal_type + "-" + recinf->channel_desc;
        //qDebug() << QString::fromStdString(chName);

        // TO DO - nice format of channel label

        if(edf_set_samplefrequency(hdl, ch_index, recinf->sampling_rate)) // TO DO - replace by SMP_FREQ?
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

    int nBlocks = signal->signal_data[0].size()/SMP_FREQ;
    double buf[SMP_FREQ];

    for(int block_index = 0; block_index < nBlocks; block_index++){
        double iBeg = block_index*SMP_FREQ;

        for(int ch_index = 0; ch_index < signal->recorder_info.numberOfChannelsUsed; ch_index++){

            Channel *recinf = &signal->recorder_info.channels[ch_index];

            for(int buf_index = 0; buf_index < recinf->sampling_rate; buf_index++){
                buf[buf_index] = signal->signal_data[ch_index][iBeg+buf_index]* 100; //* chFactor - why?
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples() block %d, channel %d\n",block_index,ch_index);

                return(1);
            }
        }

    }

    // ======== WRITING ANNOTATIONS ========

    qDebug() << "events size" << signal->events.size();

    for(unsigned int j = 0; j < signal->events.size(); j++){
        // TO DO - other types of events
        if(signal->events.at(j).ev_type == 3){


            int ev_subtype = signal->events.at(j).sub_type;
            ev_subtype -= 100; // turns 102 into 2 and so on, corresponds to events_desc
            int onset_in_s = (signal->events.at(j).page - 1) * signal->recorder_info.epochLengthInSamples / float(signal->recorder_info.highestRate) + signal->events.at(j).page_time;
            QString label = QString::fromLocal8Bit(signal->events_desc.at(ev_subtype).desc.data());
            // TO DO - option to write just shortened labels of events, e.g. OO, ZO, HVn, HVu

            edfwrite_annotation_utf8(hdl, onset_in_s*10000, signal->events.at(j).duration_in_ms*10, label.toUtf8());
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
//            qDebug() << "event no: " << j;
//            //qDebug() << "info: "<<signal->events.at(j).info; // always one
//            //qDebug() << "channels: " << signal->events.at(j).channels;
//            qDebug() << "page: "<<signal->events.at(j).page << ", page_time: "<<signal->events.at(j).page_time;
//            qDebug() << "time: "<<signal->events.at(j).time;
//            qDebug() << "duration: " << signal->events.at(j).duration << ", duration in ms: " <<signal->events.at(j).duration_in_ms;
//            qDebug() << "end time: " <<signal->events.at(j).end_time;
//            qDebug() << "ev type: " << signal->events.at(j).ev_type << ", sub type: "<<signal->events.at(j).sub_type;
//            qDebug() << "----";
        }

    }

//    qDebug() << "events desc size" << signal->events_desc.size(); // predefinovane eventy

//    for(unsigned int k = 0; k < signal->events_desc.size(); k++){
//        qDebug() << k;
//        qDebug() << signal->events_desc.at(k).desc.data();
//        qDebug() << signal->events_desc.at(k).label.data();
//        qDebug() << signal->events_desc.at(k).value;
//        qDebug() << "---";

//    }

    for(int i = 0; i < signal->events.size(); i++){
        //        st = Event.ST_DICT.get((evt.ev_type, evt.sub_type))
        //                eventName = Event.ET_DICT.get(evt.ev_type) + '_' + (st if st is not None else evt.sub_type)
        //                second = (evt.page - 1) * sf.recorder_info.epochLengthInSamples / float(sf.recorder_info.highestRate) + evt.page_time
        //                duration = -1
        //                edfWriteAnnotation(edfWriter, second, duration, eventName.encode("utf-8"))
    }

//    for(int i = 0; i < signal->notes.size(); i++){
//        qDebug() << "note no: " << i+1;
//        qDebug() << signal->notes[i].desc;
//        qDebug() << signal->notes[i].page;
//    }

    edfclose_file(hdl);

    qDebug() << "finished";

    return(0);
}
