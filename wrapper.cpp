#include "wrapper.h"

wrapper::wrapper()
{
}

int wrapper::readAndSaveFile(QFileInfo infoSig, QFileInfo infoEdf, bool anonymize, bool shorten, bool exportSystemEvents){
    read_signal_file signalReader;
    read_signal_file::SignalFile signal = signalReader.read_signal_file_all(infoSig, true);
    if(signal.check){ // check if the file is oky etc
        write_edf EdfWriter;
        if(EdfWriter.write_edf_file(&signal,infoEdf, anonymize, shorten, exportSystemEvents)){
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        std::cout << "Error reading .SIG file" << std::endl;
        return 1;
    }
}

int wrapper::readAndSaveFileChunks(QFileInfo infoSig, QFileInfo infoEdf, bool anonymize, bool shorten, bool exportSystemEvents){
    read_signal_file signalReader;
    read_signal_file::SignalFile signal = signalReader.read_signal_file_all(infoSig, false); // reads the header and the pages, not the real EEG data
    if(!signal.check){
        std::cout << "Error reading .SIG file" << std::endl;
        return 1;
    }

    write_edf EdfWriter;
    // write the header
    EdfWriter.write_edf_header(&signal, infoEdf, anonymize);
    // write the chunks of EEG signal
    int numberOfChannelsUsed = signal.recorder_info.numberOfChannelsUsed;
    int SMP_FREQ = int(signal.recorder_info.channels[0].sampling_rate);
    long current_offset = signal.data_table.signal_info.offset;
    long page_size = signal.data_table.signal_info.size;
    std::vector<std::vector<double>> esignals_buffer(numberOfChannelsUsed); // Allocate the 2D vector ONCE outside the loop
    //for page in pages
    for(int i = 0;i < signal.signal_pages.size();i++){
        //read_signal_file::Spage spage = signalReader.read_signal_page(current_offset, numberOfChannelsUsed, signal.recorder_info.channels); // read one page
        signalReader.read_signal_page_into(current_offset, numberOfChannelsUsed, signal.recorder_info.channels, esignals_buffer);
        current_offset += page_size; // move the pointer (or maybe I do not need it after setting the first one...
        // write data records
        //EdfWriter.set_data_chunk(spage.esignals,SMP_FREQ,numberOfChannelsUsed);
        EdfWriter.set_data_chunk(esignals_buffer, SMP_FREQ, numberOfChannelsUsed);
    }
    // write the annotations
    EdfWriter.set_annotations(&signal, shorten, exportSystemEvents);
    // finish writing the file
    EdfWriter.close_file();
    return 0;

}
