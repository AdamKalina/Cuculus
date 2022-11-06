/*
*****************************************************************************
*
* Cuculus is command line application for converting EEG *.SIG files (Schwartzer BrainLab) to *.EDF
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

#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include "write_edf_file.cpp"

QString EdfPath;

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("Cuculus - .SIG to .EDF converter");
    QCoreApplication::setApplicationVersion("0.3");

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
"Cuculus is .SIG (Brainlab) to .EDF converter based of EDFlib by Teuniz and sigtoedf by Frederik-D-Weber.\n"
"Cuculus  Copyright (C) 2022  Adam Kalina\n"
"This program comes with ABSOLUTELY NO WARRANTY\n"
"This is free software, and you are welcome to redistribute it\n");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption anonymizeOption("a", QCoreApplication::translate("main", "Anonymize output"));
    parser.addOption(anonymizeOption);
    QCommandLineOption shortenOption("s", QCoreApplication::translate("main", "Shorten events labels"));
    parser.addOption(shortenOption);
    QCommandLineOption systemEventsOption("y", QCoreApplication::translate("main", "Export recorder system events in annotations"));
    parser.addOption(systemEventsOption);
    parser.addPositionalArgument("input", QCoreApplication::translate("main", "Source *.SIG file"));
    parser.addPositionalArgument("output", QCoreApplication::translate("main", "Target *.EDF file - optional"));

    // Process the actual command line arguments given by the user
    parser.process(a);

    const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)

    bool anonymize = parser.isSet(anonymizeOption); // disable adding patients info in the header
    bool shorten = parser.isSet(shortenOption); // use shortened labels for events, e.g. ZO/OO
    bool exportSystemEvents = parser.isSet(systemEventsOption);

    //    qDebug() << "anonymize is " << anonymize;
    //    qDebug() << "shorten is " << shorten;
    //    qDebug() << args;

    if(args.size()==0){
        parser.showHelp();
    }
    else{
        QFileInfo infoSig(args.at(0));
        if(args.size() == 2){ // two arguments provided
            EdfPath = args.at(1);
        }else{ // only one (or more than two) arguments provided
            EdfPath = infoSig.path() + "/" + infoSig.completeBaseName() + ".EDF";
            //qDebug() << "Exporting to the same folder";
            std::cout << "Exporting to the same folder" << std::endl;
        }
        QFileInfo infoEdf(EdfPath);

        if(infoEdf.completeSuffix() != "EDF"){ //must be EDF file
            //qDebug() << "The path to output does not end with .EDF - exiting";
            std::cout << "The path to output does not end with .EDF - exiting" << std::endl;
            return 1;
        }

        if(infoSig.completeSuffix().toLower() == "sig" && infoSig.exists()){ // check if input is .sig or .SIG file
            SignalFile signal = read_signal_file(infoSig);
            if(signal.check){ // check if the file is oky etc
                write_edf_file(&signal,infoEdf, anonymize, shorten,exportSystemEvents);
                std::cout << "Finished successfully!" << std::endl;
                return 0;
            }
            else{
                //qDebug() << "Error reading .SIG file";
                std::cout << "Error reading .SIG file" << std::endl;
                return 1;
            }
        }
        else{
            //qDebug() << "The input file does not exist or it is not a .SIG file - exiting";
            std::cout << "The input file does not exist or it is not a .SIG file - exiting" << std::endl;
            return 1;
        }


    }

    //return a.exec();
}
