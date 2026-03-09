#include "harmoniereader.h"
#include <algorithm>
#include <numeric>
#include <deque>

#include <QTextCodec>
#include "edflib.h"

HarmonieReader::HarmonieReader()
{

}

/**
 * @brief Detects the physical layout of the .SIG file.
 *
 * Harmonie files vary in how many channels they physically store vs what is shown in the montage.
 * This function also detects if the data blocks use an 8-byte or 16-byte header.
 */
void HarmonieReader::readRecordChannelCount() {
    // 1. Get hint from STS file (look for CStdCodec marker)
    std::string pattern = "CStdCodec";
    char currentChar;
    std::string buffer;
    sts_file.clear();
    sts_file.seekg(0);

    while (sts_file.get(currentChar)) {
        buffer += currentChar;
        if (buffer.size() > pattern.size())
            buffer.erase(0, 1);
        if (buffer == pattern) break;
    }

    int hintChannels = 0;
    if (buffer == pattern) {
        short dummy, count;
        sts_file.read(reinterpret_cast<char*>(&dummy), 2);
        sts_file.read(reinterpret_cast<char*>(&count), 2);
        hintChannels = count;
    } else {
        // Fallback to montage channel count if marker not found
        hintChannels = numChannels;
    }

    // 2. Cross-verify with actual SIG file size to find exact block alignment
    if (!sig_file.is_open()) {
        sig_file.open(sig_file_info.filePath().toLocal8Bit(), std::ios::in | std::ios::binary);
    }

    if (sig_file.is_open()) {
        double t1, t2;
        sig_file.seekg(0, std::ios::beg);
        sig_file.read(reinterpret_cast<char*>(&t1), 8);

        // Check for 8-byte vs 16-byte block headers: (Channels * 128 bytes of data) + Header
        size_t sizes[] = { (size_t)hintChannels * 128 + 8, (size_t)hintChannels * 128 + 16 };
        bool found = false;

        for (size_t s : sizes) {
            sig_file.seekg(s, std::ios::beg);
            if (sig_file.read(reinterpret_cast<char*>(&t2), 8)) {
                // If the next timestamp is reasonably close (within 2 seconds), we found the alignment
                if (std::abs(t2 - t1) < 2.0 && std::abs(t2 - t1) > 0.0001) {
                    recordChannels = hintChannels;
                    recordHeaderSize = s - (hintChannels * 128);
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            recordChannels = hintChannels;
            recordHeaderSize = 8;
        }
        sig_file.close();
    }
    
    if (recordChannels <= 0) recordChannels = hintChannels;
    qDebug() << "Physical records detected:" << recordChannels << "channels," << recordHeaderSize << "byte header.";
}

/**
 * @brief Exports the loaded signal data to EDF+ files.
 *
 * Each recording section (continuous data segment) is exported to a separate file
 * to maintain timeline integrity.
 */
int HarmonieReader::exportToEdf(QFileInfo infoEdf, bool exportSystemAnnotations, bool anonymize) {
    if (channels.empty()) return 1;

    int fs = channels[0].sampling_rate;
    std::string fullName;
    std::string patientCode;
    std::vector<int> birthdate;

    if (anonymize) {
        fullName = "Anonymous";
        patientCode = "X";
        birthdate = {1901, 1, 1}; // Standard anonymized birthdate
    } else {
        fullName = patient_info[2] + " " + patient_info[3];
        patientCode = patient_info[0];
        birthdate = birthDateFromID(patient_info[0]);
    }

    // Loop through each recording section and create a separate EDF file
    for(int j = 0; j < (int)sections.size(); j++){

        std::string outputPath_section = infoEdf.absoluteFilePath().toLocal8Bit();
        // Append "_part_N" before the .edf extension
        outputPath_section.insert(outputPath_section.size() - 4, "_part_" + std::to_string(j + 1));

        std::cout << "Exporting section " << (j + 1) << " to: " << outputPath_section << std::endl;

        int hdl = edfopen_file_writeonly(outputPath_section.c_str(), EDFLIB_FILETYPE_EDFPLUS, numChannels);
        if (hdl < 0) {
            std::cout << "ERROR: edfopen_file_writeonly failed with code " << hdl << std::endl;
            return 1;
        }

        // Set Header Metadata
        edf_set_patientname(hdl, fullName.c_str());
        edf_set_patientcode(hdl, patientCode.c_str());
        edf_set_birthdate(hdl, birthdate[0], birthdate[1], birthdate[2]);
        edf_set_equipment(hdl, "Harmonie");

        QDateTime sectionStart = sections[j].start;
        edf_set_startdatetime(hdl, sectionStart.date().year(), sectionStart.date().month(), sectionStart.date().day(),
                              sectionStart.time().hour(), sectionStart.time().minute(), sectionStart.time().second());

        // Configure Channels
        for (int i = 0; i < numChannels; ++i) {
            std::string sensor = channels[i].label;
            
            // Determine signal type (EEG or ECG)
            std::string type = "EEG";
            double physMax = 3276.7; // Standard EEG range in uV
            
            // Check if it's ECG based on type code (2) or label content
            if (channels[i].type == 2 || sensor.find("EKG") != std::string::npos || sensor.find("ECG") != std::string::npos) {
                type = "ECG";
                physMax = 10000.0; // Wider range for ECG to prevent truncation of R-peaks
            }
            
            std::string edfLabel = type + " " + sensor;
            edf_set_samplefrequency(hdl, i, channels[i].sampling_rate);
            edf_set_label(hdl, i, edfLabel.c_str());
            
            // Set physical and digital ranges
            edf_set_physical_maximum(hdl, i, physMax);
            edf_set_physical_minimum(hdl, i, -physMax);
            edf_set_digital_maximum(hdl, i, 32767);
            edf_set_digital_minimum(hdl, i, -32768);
            edf_set_physical_dimension(hdl, i, "uV");
        }

        // Sample-accurate data extraction using queues
        std::vector<std::deque<double>> sampleQueues(numChannels);
        std::vector<double> exportBuf(fs);
        size_t blockSizeBytes = recordChannels * 128 + recordHeaderSize;

        if (!sig_file.is_open()) {
            sig_file.open(sig_file_info.filePath().toLocal8Bit(), std::ios::in | std::ios::binary);
        }
        
        // Seek to the start block of this specific recording section
        sig_file.clear();
        sig_file.seekg(sections[j].start_block * blockSizeBytes, std::ios::beg);

        size_t blocksToRead = sections[j].no_of_blocks;
        qDebug() << "Exporting section" << (j + 1) << ":" << blocksToRead << "blocks";

        for (size_t b = 0; b < blocksToRead; ++b) {
            sig_file.seekg(recordHeaderSize, std::ios::cur); // Skip block header/timestamp
            for (int ch = 0; ch < recordChannels; ++ch) {
                short blockData[64];
                sig_file.read(reinterpret_cast<char*>(blockData), 128);
                
                // If this channel is part of the display montage, buffer the samples
                if (ch < numChannels) {
                    // Normalizing polarity: EEG is physically correct (1.0), ECG (type 2) needs inversion (-1.0)
                    double pol = 1.0;
                    if (channels[ch].type == 2) {
                        pol = -1.0;
                    }
                    
                    for (int s = 0; s < 64; ++s) {
                        sampleQueues[ch].push_back(static_cast<double>(blockData[s]) * pol);
                    }
                }
            }

            // Flush queues: when we have enough samples for 1 full second, write them to EDF
            while (!sampleQueues.empty() && sampleQueues[0].size() >= (size_t)fs) {
                for (int ch = 0; ch < numChannels; ++ch) {
                    for (int s = 0; s < fs; ++s) {
                        exportBuf[s] = sampleQueues[ch].front();
                        sampleQueues[ch].pop_front();
                    }
                    edfwrite_physical_samples(hdl, exportBuf.data());
                }
            }
        }

        // Export Annotations for this section
        for (const auto& ann : annotations) {
            if (!exportSystemAnnotations && ann.system) continue;

            // Only export annotations that occur within this section's time window
            if (sections[j].start <= ann.start && ann.end <= sections[j].end){
                // onset and duration are in units of 100 microSeconds (resolution is 0.0001 second)
                long long onset = sectionStart.msecsTo(ann.start) * 10;
                long long duration = ann.start.msecsTo(ann.end) * 10;
                if (duration <= 0) duration = -1;

                if (edfwrite_annotation_utf8(hdl, onset, duration, ann.text.toUtf8().constData()) != 0) {
                    qDebug() << "Warning: failed to write annotation:" << ann.text;
                }
            }
        }

        edfclose_file(hdl);
    }
    
    if (sig_file.is_open()) sig_file.close();
    qDebug() << "EDF export finished successfully.";
    return 0;
}

/**
 * @brief Maps the recording sections by analyzing block timestamps.
 */
void HarmonieReader::reconstructTimeline() {
    sig_file.open(sig_file_info.filePath().toLocal8Bit(), std::ios::in | std::ios::binary);
    if (!sig_file.is_open()) {
        std::cout << "ERROR: Cannot open the SIG file..." << std::endl;
        return;
    }

    size_t blockSizeBytes = recordChannels * 128 + recordHeaderSize;
    sig_file.seekg(0, std::ios::end);
    size_t fileSize = sig_file.tellg();
    size_t numBlocks = fileSize / blockSizeBytes;
    sig_file.seekg(0);

    qDebug() << "Reading timeline from SIG file..." << numBlocks << "blocks";

    decimatedTimeline.clear();
    decimatedTimeline.reserve(numBlocks);

    // Read only the timestamp at the beginning of each block
    for (size_t i = 0; i < numBlocks; ++i) {
        double ts;
        sig_file.read(reinterpret_cast<char*>(&ts), sizeof(double));
        decimatedTimeline.push_back(ts);
        sig_file.seekg(blockSizeBytes - 8, std::ios::cur);
    }
    sig_file.close();

    if (decimatedTimeline.empty()) return;

    // Detect recording gaps (cuts) by comparing consecutive timestamps
    sections.clear();
    if (decimatedTimeline.size() < 2) {
        TimelineSection s;
        s.start = decodeTime(decimatedTimeline[0]);
        double blockDuration = 0;
        if (!channels.empty()) {
            blockDuration = 64.0 / channels.at(0).sampling_rate;
        }
        s.end = s.start.addMSecs(blockDuration * 1000.0);
        s.durationSeconds = blockDuration;
        sections.push_back(s);
        return;
    }

    std::vector<double> diffs;
    for (size_t i = 1; i < decimatedTimeline.size(); ++i) {
        diffs.push_back(decimatedTimeline[i] - decimatedTimeline[i-1]);
    }
    std::sort(diffs.begin(), diffs.end());
    double medianDiff = diffs[diffs.size() / 2];
    double threshold = medianDiff * 2.0;

    double blockDuration = 0;
    if (!channels.empty()) {
        blockDuration = 64.0 / channels.at(0).sampling_rate;
    }

    size_t sectionStartIdx = 0;
    for (size_t i = 1; i < decimatedTimeline.size(); ++i) {
        // If the gap between blocks exceeds the threshold, it's a new section
        if ((decimatedTimeline[i] - decimatedTimeline[i-1]) > threshold) {
            TimelineSection s;
            s.start = decodeTime(decimatedTimeline[sectionStartIdx]);
            s.end = decodeTime(decimatedTimeline[i-1]).addMSecs(blockDuration * 1000.0);
            s.durationSeconds = (i - sectionStartIdx) * blockDuration;
            s.start_block = sectionStartIdx;
            s.end_block = i;
            s.no_of_blocks = i - sectionStartIdx;
            sections.push_back(s);
            sectionStartIdx = i;
        }
    }

    // Finalize the last section
    TimelineSection s;
    s.start = decodeTime(decimatedTimeline[sectionStartIdx]);
    s.end = decodeTime(decimatedTimeline.back()).addMSecs(blockDuration * 1000.0);
    s.durationSeconds = (decimatedTimeline.size() - sectionStartIdx) * blockDuration;
    s.no_of_blocks = decimatedTimeline.size() - sectionStartIdx;
    s.start_block = sectionStartIdx;
    s.end_block = decimatedTimeline.size();
    sections.push_back(s);

    for (const auto& sec : sections) {
        qDebug() << "Section:" << sec.start.toString() << "to" << sec.end.toString() << "Duration:" << sec.durationSeconds << "s";
    }
}

/**
 * @brief Decodes Harmonie timestamps (seconds since Jan 1, 0001).
 */
QDateTime HarmonieReader::decodeTime(double start_time){
    QDateTime beginning_of_epoch(QDate(1, 1, 1), QTime(0, 0, 0));
    // Adjustment needed because QDateTime epoch logic differs from Harmonie's
    QDateTime record_start = beginning_of_epoch.addSecs(start_time-365*24*60*60-24*60*60);
    if(record_start.isDaylightTime()) record_start = record_start.addSecs(-3600);
    return record_start;
}

/**
 * @brief Extracts patient birthdate from ID (Czech Rodné číslo).
 */
std::vector<int> HarmonieReader::birthDateFromID(std::string id){
    if(id == "0" || id.length() < 6) return std::vector<int> {0,0,0};

    std::string year = id.substr(0, 2);
    std::string month = id.substr(2, 2);
    std::string day = id.substr(4, 2);

    int m = atoi(month.c_str());
    int y = atoi(year.c_str());
    int d = atoi(day.c_str());

    // Adjustment for females (+50 in month)
    if(m > 12){
        m -= 50;
    }

    // Century detection (rodná čísla were 9 digits before 1954)
    if(y < 54 && id.length() == 10){
        y += 2000;
    }else{
        y += 1900;
    }

    return std::vector<int> {y,m,d};
}

/**
 * @brief Scans the STS file for patient information.
 */
void HarmonieReader::readPatientInfo(){
    char currentChar;
    std::string pattern = "CPatientInfo";
    std::string buffer;
    patient_info.clear();
    while (sts_file.get(currentChar)) {
        buffer += currentChar;
        if (buffer.size() > pattern.size()) buffer.erase(0, 1);
        if (buffer == pattern) break;
    }
    while (sts_file.get() != 0) {} // Skip padding

    // now there is patient info divided into four fields: [w][id1][x][id2][y][surname][z][name][null null null] where w,x,y,z are numbers of characters in the next field.
    // Since it can be zero, the whole header is terminated by three consecutive nulls.
    
    int nullCount = 0;
    while (sts_file.get(currentChar)) {
        if (currentChar == '\0') {
            if (++nullCount == 3) break; // Terminated by three nulls
        } else nullCount = 0;
        
        int length = (unsigned char)currentChar;
        std::string result = "";
        for (int i = 0; i < length; i++) result += sts_file.get();
        patient_info.push_back(result);
    }
}

/**
 * @brief Parses the complex annotation structure in the .STS file.
 */
void HarmonieReader::readAnnotations(){
    unsigned short tmp;
    double f;
    char currentChar;
    std::string pattern = "CStatusItem";
    std::string buffer;

    // Reset pointer to find annotations marker regardless of previous reads
    sts_file.clear();
    sts_file.seekg(0);

    while (sts_file.get(currentChar)) {
        buffer += currentChar;
        if (buffer.size() > pattern.size()) buffer.erase(0, 1);
        if (buffer == pattern) break;
    }

    if (buffer != pattern) {
        qDebug() << "Warning: CStatusItem marker not found in STS file.";
        return;
    }

    // Skip the 80-byte header block following CStatusItem
    for (int i = 0; i < 4; i++) sts_file.read(reinterpret_cast<char*>(&tmp), 2);
    sts_file.read(reinterpret_cast<char*>(&f), 8);
    for (int i = 0; i < 2; i++) sts_file.read(reinterpret_cast<char*>(&tmp), 2);
    sts_file.read(reinterpret_cast<char*>(&f), 8);
    for (int i = 0; i < 9; i++) sts_file.read(reinterpret_cast<char*>(&tmp), 2);
    sts_file.read(reinterpret_cast<char*>(&f), 8);
    for (int i = 0; i < 2; i++) sts_file.read(reinterpret_cast<char*>(&tmp), 2);
    sts_file.read(reinterpret_cast<char*>(&f), 8);
    for (int i = 0; i < 9; i++) sts_file.read(reinterpret_cast<char*>(&tmp), 2);

    QTextCodec *codec = QTextCodec::codecForName("Windows-1250");
    double ttin, ttout;
    short type_arr[2];
    unsigned char num_char;


    // Loop through individual annotation records
    while (sts_file.read(reinterpret_cast<char*>(&ttin), 8)) {
        Annotation an;
        an.start = decodeTime(ttin);
        sts_file.read(reinterpret_cast<char*>(type_arr), 4); // Metadata/flags
        sts_file.read(reinterpret_cast<char*>(&ttout), 8); // End time
        an.end = decodeTime(ttout);
        sts_file.read(reinterpret_cast<char*>(type_arr), 4); // Type Flag

        if (type_arr[1] == -1) { // Branch A: Complex/User Annotations
            an.system = false;
            sts_file.read(reinterpret_cast<char*>(&num_char), 1);
            QByteArray textData(num_char, '\0');
            sts_file.read(textData.data(), num_char);
            an.text = codec->toUnicode(textData);
            
            unsigned char tmmp;
            sts_file.read(reinterpret_cast<char*>(&tmmp), 1);
            if (tmmp == 255) { // Extension marker
                unsigned short numm;
                sts_file.read(reinterpret_cast<char*>(&numm), 2);
                QByteArray extra(numm, '\0');
                sts_file.read(extra.data(), numm);
                an.text += " " + QString::fromLocal8Bit(extra);
            } else if (tmmp != 0) {
                QByteArray extra(tmmp, '\0');
                sts_file.read(extra.data(), tmmp);
                an.text += " " + QString::fromLocal8Bit(extra);
            }
            
            // Skip metadata tail
            unsigned char tmmp_arr[2];
            sts_file.read(reinterpret_cast<char*>(tmmp_arr), 2);
            if (tmmp_arr[0]) {
                unsigned char recnum;
                sts_file.read(reinterpret_cast<char*>(&recnum), 1);
                sts_file.seekg(recnum, std::ios::cur);
                sts_file.read(reinterpret_cast<char*>(&recnum), 1);
                sts_file.seekg(recnum, std::ios::cur);
                sts_file.read(reinterpret_cast<char*>(&num_char), 1);
                sts_file.seekg(num_char, std::ios::cur);
                sts_file.read(reinterpret_cast<char*>(&num_char), 1);
                int count = 0;
                while (count < num_char) if (sts_file.get() == 70) count++; else count = 0;
                sts_file.seekg(10, std::ios::cur);
            } else sts_file.seekg(10, std::ios::cur);
            
        } else { // Branch B: Simple/System Annotations
            sts_file.read(reinterpret_cast<char*>(&num_char), 1);
            QByteArray t1(num_char, '\0');
            sts_file.read(t1.data(), num_char);
            sts_file.read(reinterpret_cast<char*>(&num_char), 1);
            QByteArray t2(num_char, '\0');
            sts_file.read(t2.data(), num_char);
            an.text = QString::fromLocal8Bit(t1) + QString::fromLocal8Bit(t2);
            an.system = true;
            sts_file.seekg(12, std::ios::cur); // Standard system padding
        }
        
        if (!an.text.isEmpty()) annotations.push_back(an);
        if (sts_file.eof()) break;
    }
}

void HarmonieReader::readHarmonieChannelsInfo(){
    channels.clear();
    std::string pattern = "CRecordingMontage";
    char currentChar;
    std::string buffer;
    while (sts_file.get(currentChar)) {
        buffer += currentChar;
        if (buffer.size() > pattern.size()) buffer.erase(0, 1);
        if (buffer == pattern) break;
    }
    sts_file.seekg(2, std::ios::cur);
    int name_length = sts_file.get();
    sts_file.seekg(name_length, std::ios::cur);
    short no, r_INF1_NUM, sf;
    sts_file.read((char *)&no, 2);
    numChannels = no;
    sts_file.read((char *)&r_INF1_NUM, 2);
    std::vector<int> sfs;
    for (int i = 0; i < r_INF1_NUM; i++) {
        sts_file.read((char *)&sf, 2);
        sfs.push_back(sf);
    }
    
    // Skip hardware settings blocks
    short r_INF2_NUM;
    sts_file.read((char *)&r_INF2_NUM, 2);
    sts_file.seekg(r_INF2_NUM * 2, std::ios::cur);
    
    // Read channel types (1 for EEG, 2 for ECG)
    short r_INF3_NUM;
    std::vector<int> types;
    sts_file.read((char *)&r_INF3_NUM, 2);
    for (int i = 0; i < r_INF3_NUM; i++) {
        short pol;
        sts_file.read((char *)&pol, 2);
        types.push_back(pol);
    }
    
    // Skip more internal metadata blocks
    for (int j = 4; j <= 5; ++j) {
        short n;
        sts_file.read((char *)&n, 2);
        sts_file.seekg(n * 2, std::ios::cur);
    }
    short n6;
    sts_file.read((char *)&n6, 2);
    sts_file.seekg(n6 * 4, std::ios::cur);
    sts_file.seekg(6, std::ios::cur);
    
    // Read final channel labels
    short n8;
    sts_file.read((char *)&n8, 2);
    for (int i = 0; i < n8; i++) {
        int len = sts_file.get();
        std::string label(len, '\0');
        sts_file.read(&label[0], len);
        ChannelInfo ci;
        ci.label = label;
        ci.sampling_rate = (i < (int)sfs.size()) ? sfs[i] : 0;
        ci.type = (i < (int)types.size()) ? types[i] : 0;
        channels.push_back(ci);
    }
}

void HarmonieReader::readHeader(){
    sts_file.open(sts_file_info.filePath().toLocal8Bit(), std::ios::in | std::ios::binary);
    if (!sts_file.is_open()) {
        std::cout << "ERROR: Cannot open the STS file..." << std::endl;
        return;
    }

    // Magic number check for Harmonie STS files: 4294901761 (0xFFFF0001)
    unsigned int magic;
    sts_file.read(reinterpret_cast<char*>(&magic), 4);
    if (magic != 4294901761U) {
        std::cout << "ERROR: Not a valid Harmonie STS file (Invalid magic number: " << magic << ")" << std::endl;
        sts_file.close();
        return;
    }

    readPatientInfo();
    readHarmonieChannelsInfo();
    readRecordChannelCount();
    readAnnotations();
    sts_file.close();
    reconstructTimeline();
}

int HarmonieReader::readHarmonieFile(QFileInfo info){
    if(!info.exists()) return 1;
    if (info.completeSuffix().toLower() == "sig"){
        sig_file_info = info;
        sts_file_info = info.canonicalPath() + "/" + info.baseName() + ".STS";
    } else {
        sig_file_info = info.canonicalPath() + "/" + info.baseName() + ".SIG";
        sts_file_info = info;
    }
    readHeader();
    return 0;
}
