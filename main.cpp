#include <QCoreApplication>
#include <QDebug>
#include <QCommandLineParser>
#include "write_edf_file.cpp"

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
        SignalFile signal = read_signal_file(info1); //TO DO - check if the file is oky etc.
        qDebug() << QString::fromLocal8Bit(signal.measurement.name);
        qDebug() << signal.recorder_info.numberOfChannelsUsed;

        QFileInfo info2(args.at(1));

        // TO DO - if everything is oky, write the EDF file
        write_edf_file(&signal,info2);

    }

    return a.exec();
}
