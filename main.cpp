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

#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include "wrapper.h"
#include "harmoniereader.h"

QString EdfPath;

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "czech");
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("Cuculus - .SIG converter");
    QCoreApplication::setApplicationVersion("0.4");

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
                "Cuculus is .SIG converter based of EDFlib by Teuniz and supporting BrainLab and Harmonie formats.\n"
                "Cuculus  Copyright (C) 2022-2026  Adam Kalina\n"
                "This program comes with ABSOLUTELY NO WARRANTY\n"
                "This is free software, and you are welcome to redistribute it\n");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption anonymizeOption("a", QCoreApplication::translate("main", "Anonymize output"));
    parser.addOption(anonymizeOption);
    QCommandLineOption shortenOption("s", QCoreApplication::translate("main", "Shorten events labels (BrainLab only)"));
    parser.addOption(shortenOption);
    QCommandLineOption systemEventsOption("y", QCoreApplication::translate("main", "Export recorder system events in annotations"));
    parser.addOption(systemEventsOption);
    parser.addPositionalArgument("input", QCoreApplication::translate("main", "Source *.SIG file"));
    parser.addPositionalArgument("output", QCoreApplication::translate("main", "Target *.EDF file - optional"));

    // Process the actual command line arguments given by the user
    parser.process(a);

    const QStringList args = parser.positionalArguments();

    bool anonymize = parser.isSet(anonymizeOption); 
    bool shorten = parser.isSet(shortenOption); 
    bool exportSystemEvents = parser.isSet(systemEventsOption);

    if(args.size()==0){
        parser.showHelp();
    }
    else{
        QFileInfo infoSig(args.at(0));
        if(args.size() == 2){ // two arguments provided
            EdfPath = args.at(1);
        }else{ // only one (or more than two) arguments provided
            EdfPath = infoSig.path() + "/" + infoSig.completeBaseName() + ".EDF";
            std::string coutPath = EdfPath.toLocal8Bit();
            std::cout << "Exporting to " << coutPath << std::endl;
        }
        QFileInfo infoEdf(EdfPath);

        // --- BRAINLAB CHECK ---
        read_signal_file BrainlabReader;
        // read_signal_file_all checks program_id == 1096045395 internally
        read_signal_file::SignalFile signal = BrainlabReader.read_signal_file_all(infoSig, false);

        if (signal.check) {
            std::cout << "Detected BrainLab format. Exporting..." << std::endl;
            wrapper Wrapper;
            if (Wrapper.readAndSaveFileChunks(infoSig, infoEdf, anonymize, shorten, exportSystemEvents)){
                std::cout << "error processing BrainLab file" << std::endl;
                return 1;
            }
            std::cout << "BrainLab export finished successfully!" << std::endl;
            return 0;
        } 
        
        // --- HARMONIE CHECK ---
        std::cout << "Not a BrainLab file. Trying Harmonie format..." << std::endl;
        HarmonieReader Harmonie;
        if (Harmonie.readHarmonieFile(infoSig) == 0) {
            std::cout << "Detected Harmonie format. Exporting..." << std::endl;
            if (Harmonie.exportToEdf(infoEdf, exportSystemEvents, anonymize) == 0) {
                std::cout << "Harmonie export finished successfully!" << std::endl;
                return 0;
            } else {
                std::cout << "Error during Harmonie export." << std::endl;
                return 1;
            }
        }

        std::cout << "Error: File is neither a valid BrainLab nor Harmonie recording." << std::endl;
        return 1;
    }

    return 0;
}
