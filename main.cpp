#include <QCoreApplication>
#include <QDebug>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include "write_edf_file.cpp"

bool inputOky;
bool outputOky;

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("Cuculus - .SIG to .EDF converter");
    QCoreApplication::setApplicationVersion("0.2");

    QCommandLineParser parser;
    parser.setApplicationDescription(
"                                                :.\n"
"                                              ~?7.\n"
"                                           .~J5?:\n"
"                 .::::.                  .~J55?.\n"
"              .~7?77!77?!:              ~J55Y!.\n"
"          :~!7YJ: .7^  :!?777!~^.     ~J5557.\n"
"             .^??: .      ..:^!7??7^^J555?:\n"
"                7Y.              :~7J555!\n"
"                .Y7     .:.          :!J7:\n"
"                 :Y!     .:~~~^:....   .~??~.         . :^\n"
"     :7J?77!^     ^Y7.      .^~!7??????????7~       .7?!!7\n"
" :7?!^..^7?!^.     .7J!:           ..::..   .^7?7:...^7J?7.\n"
"   .^!77!~~!7?7!^.   :7JJ7!~^^^^^~~~^:..:~!??7~^~!777!~:.\n"
"       ^~~~!!7?JYYJ7!^::^!7????7!~~^~!?JYYJ?77!!~^.  :.\n"
"       ^77!~^::.:^^~!!!7?JYYYYYYYJ?7!~~^::.YYJ?7!.^!7?7^..\n"
"        :::^!?JJ?!~^^~~!!7??7!~~!77!!!!777?????7?JYYJ?777\n"
"            !!!!!777!!7?JJ??7!!~~~~~~!!77????7!~^:....\n"
"\n"
"Cuculus is .SIG (Brainlab) to .EDF converter based of EDFlib by Teuniz and sigtoedf by Frederik-D-Weber. Bashed together by Adam Kalina 2022, Motol University Hospital");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("input", QCoreApplication::translate("main", "Source *.SIG file"));
    parser.addPositionalArgument("output", QCoreApplication::translate("main", "Target *.EDF file"));

    // Process the actual command line arguments given by the user
    parser.process(a);

    const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)

    if(args.size() == 2){ //for now the app will require both arguments - TO DO - option for one argument, where the edf file is created in the same folder
        //qDebug() << "Here!";
        qDebug() << args.at(0); // TO DO - must be SIG file
        qDebug() << args.at(1); // TO DO - must be EDF file

        //TO DO - must be SIG file
        QFileInfo infoSig(args.at(0));

        if(infoSig.completeSuffix() == "SIG" && infoSig.exists()){
            inputOky = true;
        }
        else{
            qDebug() << "The input file does not exist or it is not a .SIG file - exiting";
            return 1;
        }

        // TO DO - must be EDF file
        QFileInfo infoEdf(args.at(1));


        if(infoEdf.completeSuffix() == "EDF"){
            outputOky = true;
        }else{
            qDebug() << "The path to output does not end with .EDF - exiting";
            return 1;
        }

        // If everything is oky, then read the SIG file
        if(outputOky && inputOky){
            SignalFile signal = read_signal_file(infoSig); //TO DO - check if the file is oky etc.
            write_edf_file(&signal,infoEdf);
        }
    }

    return a.exec();
}
