#include "edflib.h"
#include "read_signal_file.h"


#define SMP_FREQ  (2048)

#ifndef M_PI
#define M_PI (3.14159265358979323846264338327)
#endif




inline int write_edf_file(SignalFile *signal, QFileInfo file2write) //is that inline a hack - TO DO?
{
    int i, j,
            hdl,
            buf2[SMP_FREQ],
            chns;

    double buf[SMP_FREQ],
            q;

    chns = 2;

    const char* recorderName;

    qDebug() << QString::fromLocal8Bit(signal->measurement.name);

    hdl = edfopen_file_writeonly(file2write.absoluteFilePath().toLocal8Bit(), EDFLIB_FILETYPE_EDFPLUS, chns);

    if(hdl<0)
    {
        printf("error: edfopen_file_writeonly()\n");

        return(1);
    }

    // ======== WRITING HEADER INFO ========

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


    int edf_set_birthdate(int handle, int birthdate_year, int birthdate_month, int birthdate_day); // birthday not used in Motol?
    /* Sets the birthdate.
   * year: 1800 - 3000, month: 1 - 12, day: 1 - 31
   * This function is optional
   * Returns 0 on success, otherwise -1
   * This function is optional and can be called only after opening a file in writemode
   * and before the first sample write action
   */

    if(edf_set_patient_additional(hdl, signal->measurement.doctor)){ // this might not make sense, but in Motol we use this field for record labeling
        printf("error: edf_set_patient_additional\n");

        return(1);
    }


    if(edf_set_admincode(hdl, file2write.baseName().toLocal8Bit())){
        printf("error: edf_set_admincode\n");

        return(1);
    }

    if(edf_set_technician(hdl, signal->measurement.technician)){
        printf("error: edf_set_technician()\n");

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

    // ======== WRITING SAMPLES ========

    for(i=0; i<chns; i++)
    {
        if(edf_set_samplefrequency(hdl, i, SMP_FREQ))
        {
            printf("error: edf_set_samplefrequency()\n");

            return(1);
        }
    }

    for(i=0; i<chns; i++)
    {
        if(edf_set_physical_maximum(hdl, i, 400))
        {
            printf("error: edf_set_physical_maximum()\n");

            return(1);
        }
    }

    for(i=0; i<chns; i++)
    {
        if(edf_set_digital_maximum(hdl, i, 32767))
        {
            printf("error: edf_set_digital_maximum()\n");

            return(1);
        }
    }

    for(i=0; i<chns; i++)
    {
        if(edf_set_digital_minimum(hdl, i, -32768))
        {
            printf("error: edf_set_digital_minimum()\n");

            return(1);
        }
    }

    for(i=0; i<chns; i++)
    {
        if(edf_set_physical_minimum(hdl, i, -400))
        {
            printf("error: edf_set_physical_minimum()\n");

            return(1);
        }
    }

    if(edf_set_label(hdl, 0, "sine"))
    {
        printf("error: edf_set_label()\n");

        return(1);
    }

    if(edf_set_label(hdl, 1, "ramp"))
    {
        printf("error: edf_set_label()\n");

        return(1);
    }

    for(i=0; i<chns; i++)
    {
        if(edf_set_physical_dimension(hdl, i, "mV"))
        {
            printf("error: edf_set_physical_dimension()\n");

            return(1);
        }
    }

    for(j=0; j<10; j++)
    {
        for(i=0; i<SMP_FREQ; i++)
        {
            q = M_PI * 2.0;
            q /= SMP_FREQ;
            q *= i;
            q = sin(q);
            q *= 400;
            buf[i] = q;
        }

        if(edfwrite_physical_samples(hdl, buf))
        {
            printf("error: edfwrite_physical_samples()\n");

            return(1);
        }

        for(i=0; i<SMP_FREQ; i++)
        {
            buf[i] = -400 + (i * 2.9296875);
        }

        if(edfwrite_physical_samples(hdl, buf))
        {
            printf("error: edfwrite_physical_samples()\n");

            return(1);
        }
    }

    for(j=0; j<10; j++)
    {
        for(i=0; i<SMP_FREQ; i++)
        {
            q = M_PI * 2.0;
            q /= SMP_FREQ;
            q *= i;
            q = sin(q);
            q *= 32767;
            buf2[i] = q;
        }

        if(edfwrite_digital_samples(hdl, buf2))
        {
            printf("error: edfwrite_digital_samples()\n");

            return(1);
        }

        for(i=0; i<SMP_FREQ; i++)
        {
            buf2[i] = -32768 + (i * 81);
        }

        if(edfwrite_digital_samples(hdl, buf2))
        {
            printf("error: edfwrite_digital_samples()\n");

            return(1);
        }
    }

    edfwrite_annotation_latin1(hdl, 0LL, -1LL, "Recording starts");

    edfwrite_annotation_latin1(hdl, 200000LL, -1LL, "Recording ends");

    edfclose_file(hdl);

    return(0);
}
