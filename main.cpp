#include <QCoreApplication>
#include <QDebug>
#include <QCommandLineParser>
#include "edflib.h"
#include "read_signal_file.h"

int hdl, i, j;

double buf[1000],
q,
sine_1,
sine_8,
sine_81777,
sine_85,
sine_15,
sine_17,
sine_50;

struct{
    long long samples;
    long long triggers[512];
    int index;
    int code;
    int bitposition;
    int smp_in_bit;
} dc_event_stat;


#define SMP_FREQ  (200)

#define SMP_FREQ_2  (256)

#define SMP_FREQ_3  (217)

#define FILE_DURATION  (600)

#ifndef M_PI
#define M_PI (3.14159265358979323846264338327)
#endif


int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("Cuculus - .SIG to .EDF converter");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Cuculus is .SIG (Brainlab) to .EDF converter based of EDFlib by Teuniz and sigtoedf by Frederik-D-Weber. Bashed together by Adam Kalina 2022, Motol University Hospital");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("input", QCoreApplication::translate("main", "Source *.SIG file"));
    parser.addPositionalArgument("output", QCoreApplication::translate("main", "Target *.EDF file"));


    qDebug() << "Hello, world!";

    // Process the actual command line arguments given by the user
    parser.process(a);

    const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)

    if(args.size() == 2){ //for now the app will require both arguments - TO DO - option for one argument, where the edf file is created in the same folder
        qDebug() << "Here!";
        qDebug() << args.at(0); // TO DO - must be SIG file
        qDebug() << args.at(1); // TO DO - must be EDF file

        //TO DO - if everything is oky, then read the SIG file


        QFileInfo info1(args.at(0));
        SignalFile signal = read_signal_file(info1); //check if the file is oky etc.
        qDebug() << QString::fromLocal8Bit(signal.measurement.name);
        qDebug() << signal.recorder_info.numberOfChannelsUsed;


        // TO DO - if everything is oky, write the EDF file
        int chns = 14; //signal.recorder_info.numberOfChannelsUsed;

        hdl = edfopen_file_writeonly(args.at(1).toLocal8Bit(), EDFLIB_FILETYPE_EDFPLUS, chns);

        if(hdl<0)
        {
            printf("error: edfopen_file_writeonly()\n");

            return(1);
        }

        for(i=0; i<chns; i++)
        {
            if(edf_set_samplefrequency(hdl, i, SMP_FREQ))
            {
                printf("error: edf_set_samplefrequency()\n");

                return(1);
            }
        }

        if(edf_set_samplefrequency(hdl, 3, SMP_FREQ_2))
        {
            printf("error: edf_set_samplefrequency()\n");

            return(1);
        }

        if(edf_set_samplefrequency(hdl, 4, SMP_FREQ_3))
        {
            printf("error: edf_set_samplefrequency()\n");

            return(1);
        }

        if(edf_set_samplefrequency(hdl, 13, 1000))
        {
            printf("error: edf_set_samplefrequency()\n");

            return(1);
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
            if(edf_set_physical_maximum(hdl, i, 1000.0))
            {
                printf("error: edf_set_physical_maximum()\n");

                return(1);
            }
        }

        if(edf_set_physical_maximum(hdl, 8, 262143.0))
        {
            printf("error: edf_set_physical_maximum()\n");

            return(1);
        }

        if(edf_set_physical_maximum(hdl, 13, 10.0))
        {
            printf("error: edf_set_physical_maximum()\n");

            return(1);
        }

        for(i=0; i<chns; i++)
        {
            if(edf_set_physical_minimum(hdl, i, -1000.0))
            {
                printf("error: edf_set_physical_minimum()\n");

                return(1);
            }
        }

        if(edf_set_physical_minimum(hdl, 8, -262144.0))
        {
            printf("error: edf_set_physical_minimum()\n");

            return(1);
        }

        if(edf_set_physical_minimum(hdl, 13, -10.0))
        {
            printf("error: edf_set_physical_minimum()\n");

            return(1);
        }

        for(i=0; i<chns; i++)
        {
            if(edf_set_physical_dimension(hdl, i, "uV"))
            {
                printf("error: edf_set_physical_dimension()\n");

                return(1);
            }
        }

        if(edf_set_physical_dimension(hdl, 13, "V"))
        {
            printf("error: edf_set_physical_dimension()\n");

            return(1);
        }

        i = 0;

        if(edf_set_label(hdl, i++, "squarewave"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "ramp"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "pulse 1"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "pulse 2"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "pulse 3"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "noise"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "sine 1 Hz"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "sine 8 Hz + DC"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "sine 8.1777 Hz + DC"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "sine 8.5 Hz"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "sine 15 Hz"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "sine 17 Hz"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "sine 50 Hz"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_label(hdl, i++, "DC 01"))
        {
            printf("error: edf_set_label()\n");

            return(1);
        }

        if(edf_set_equipment(hdl, "test generator"))
        {
            printf("edf_set_equipment()\n");

            return(1);
        }

        edf_set_birthdate(hdl, 1969, 6, 30);


        sine_1 = 0.0;
        sine_8 = 0.0;
        sine_81777 = 0.0;
        sine_85 = 0.0;
        sine_15 = 0.0;
        sine_17 = 0.0;
        sine_50 = 0.0;

        for(j=0; j<FILE_DURATION; j++)
        {
            if((j%10)<5)                    /* square */
            {
                for(i=0; i<SMP_FREQ; i++)
                {
                    buf[i] = 100.0;
                }
            }
            else
            {
                for(i=0; i<SMP_FREQ; i++)
                {
                    buf[i] = -100.0;
                }
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                       /* ramp */
            {
                buf[i] = -100.0 + (i * (200.0 / SMP_FREQ));
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                 /* pulse 1 */
            {
                buf[i] = 0.0;
            }

            buf[0] = 100.0;

            buf[SMP_FREQ - 2] = 100.0;

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ_2; i++)               /* pulse 2 */
            {
                buf[i] = 0.0;
            }

            buf[0] = 100.0;

            buf[SMP_FREQ_2 - 2] = 100.0;

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ_3; i++)              /* pulse 3 */
            {
                buf[i] = 0.0;
            }

            buf[0] = 100.0;

            buf[SMP_FREQ_3 - 2] = 100.0;

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                /* noise */
            {
                buf[i] = (int)(100.0 * (rand() / (RAND_MAX + 1.0)));
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                /* sine 1 Hz */
            {
                q = M_PI * 2.0;
                q /= SMP_FREQ;
                sine_1 += q;
                q = sin(sine_1);
                q *= 100.0;
                buf[i] = q;
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                /* sine 8 Hz */
            {
                q = M_PI * 2.0;
                q /= (SMP_FREQ / 8.0);
                sine_8 += q;
                q = sin(sine_8);
                q *= 100.0;
                buf[i] = q + 800.0;         /* add dc-offset */
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                /* sine 8.1777 Hz */
            {
                q = M_PI * 2.0;
                q /= (SMP_FREQ / 8.1777);
                sine_81777 += q;
                q = sin(sine_81777);
                q *= 100.0;
                buf[i] = q + 6000.0;       /* add dc-offset */
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                /* sine 8.5 Hz */
            {
                q = M_PI * 2.0;
                q /= (SMP_FREQ / 8.5);
                sine_85 += q;
                q = sin(sine_85);
                q *= 100.0;
                buf[i] = q;
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                /* sine 15 Hz */
            {
                q = M_PI * 2.0;
                q /= (SMP_FREQ / 15.0);
                sine_15 += q;
                q = sin(sine_15);
                q *= 100.0;
                buf[i] = q;
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                /* sine 17 Hz */
            {
                q = M_PI * 2.0;
                q /= (SMP_FREQ / 17.0);
                sine_17 += q;
                q = sin(sine_17);
                q *= 100.0;
                buf[i] = q;
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<SMP_FREQ; i++)                /* sine 50 Hz */
            {
                q = M_PI * 2.0;
                q /= (SMP_FREQ / 50.0);
                sine_50 += q;
                q = sin(sine_50);
                q *= 100.0;
                buf[i] = q;
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }

            for(i=0; i<1000; i++)                /* DC 01 */
            {
                if(dc_event_stat.bitposition)
                {
                    if(dc_event_stat.bitposition == 1)
                    {
                        buf[i] = 1.0;
                    }
                    else
                    {
                        if(dc_event_stat.code & (1 << (dc_event_stat.bitposition - 2)))
                        {
                            buf[i] = 1.0;
                        }
                        else
                        {
                            buf[i] = 0.0;
                        }
                    }

                    if(++dc_event_stat.smp_in_bit >= 10)
                    {
                        dc_event_stat.smp_in_bit = 0;

                        dc_event_stat.bitposition++;
                    }

                    if(dc_event_stat.bitposition > 10)
                    {
                        dc_event_stat.bitposition = 0;

                        dc_event_stat.smp_in_bit = 0;

                        dc_event_stat.code++;

                        dc_event_stat.code &= 255;

                        if(++dc_event_stat.index >= 512)
                        {
                            dc_event_stat.index = 0;

                            dc_event_stat.code = 0;
                        }
                    }
                }
                else
                {
                    if(dc_event_stat.samples == dc_event_stat.triggers[dc_event_stat.index])
                    {
                        /*          edfwrite_annotation_latin1(hdl, dc_event_stat.samples * 10LL, -1LL, "Trigger");  */

                        dc_event_stat.bitposition = 1;
                        dc_event_stat.smp_in_bit = 1;
                        buf[i] = 1.0;
                    }
                    else
                    {
                        buf[i] = 0.0;
                    }
                }

                dc_event_stat.samples++;
            }

            if(edfwrite_physical_samples(hdl, buf))
            {
                printf("error: edfwrite_physical_samples()\n");

                return(1);
            }
        }

        edfwrite_annotation_latin1(hdl, 0LL, -1LL, "Recording starts");

        edfwrite_annotation_latin1(hdl, 2980000LL, -1LL, "Test 1");

        edfwrite_annotation_latin1(hdl, 2940000LL + (long long)((10000.0 / SMP_FREQ) * (SMP_FREQ - 2)), -1LL, "pulse 1");

        edfwrite_annotation_latin1(hdl, 2950000LL + (long long)((10000.0 / SMP_FREQ_2) * (SMP_FREQ_2 - 2)), -1LL, "pulse 2");

        edfwrite_annotation_latin1(hdl, 2960000LL + (long long)((10000.0 / SMP_FREQ_3) * (SMP_FREQ_3 - 2)), -1LL, "pulse 3");

        edfwrite_annotation_latin1(hdl, FILE_DURATION * 10000LL, -1LL, "Recording ends");

        edfclose_file(hdl);
    }

    return a.exec();
}
