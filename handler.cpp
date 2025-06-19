// protocol_handler.cpp
#include "Server.h"
#include "FileHandler.h"
#include "Information_handler.h"
#include <iostream>
#include <thread>
#include <chrono>

class Information_handler {
public:
    std::string IP;
    int port;
    std::string path;
    std::string lcl_filename;
    std::string lcs_filename;
    short counter = 0;

    void setContext(const std::string& ip_, int port_, const std::string& path_,
                    const std::string& lcl, const std::string& lcs) {
        IP = ip_;
        port = port_;
        path = path_;
        lcl_filename = lcl;
        lcs_filename = lcs;
    }

    bool generateLCS() {
        FileHandler lcs;
        lcs.protocol_version = 60 + counter;
        lcs.no_of_thw = 1;

        FileHandler::ThwEntry thw;
        thw.l_n = "LineStatus";
        thw.l_n_len = thw.l_n.length();
        thw.ser_num = "StatusSerial";
        thw.ser_num_len = thw.ser_num.length();
        thw.num_p_n = 1;

        FileHandler::PnEntry pn;
        pn.p_n = "Status";
        pn.p_n_len = pn.p_n.length();
        pn.ame = "Generated";
        pn.ame_len = pn.ame.length();
        pn.p_d_text = "Status Description";
        pn.p_d_len = pn.p_d_text.length();

        thw.pn_entries.push_back(pn);
        lcs.thw_entries.push_back(thw);

        return lcs.generateFile(path, lcs_filename);
    }

    bool sendLCLFile() {
        TFTPClient client(IP, port);
        return client.request_upload(lcl_filename);
    }

    bool sendLCSFile() {
        TFTPClient client(IP, port);
        return client.request_upload(lcs_filename);
    }

    bool launchInfoHandler() {
        sendLCLFile();
        do {
            generateLCS();
            sendLCSFile();

            FileHandler parsed;
            if (parsed.parseFile(path, lcs_filename)) {
                std::cout << "[INFO] LCS Parsed. Version: " << parsed.protocol_version << "\n";
                if (parsed.protocol_version == 0x0003) {
                    std::cout << "[INFO] Status 0x0003 reached.\n";
                    break;
                }
            }
            counter = (counter + 1) % 16;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        } while (true);

        return true;
    }
};

void dataloading(int port, const std::string& path) {
    Server server(port, path);

    // Generate test.LCL
    FileHandler handler;
    handler.protocol_version = 59;
    handler.no_of_thw = 2;

    FileHandler::ThwEntry thw1;
    thw1.l_n = "Line001"; thw1.l_n_len = thw1.l_n.length();
    thw1.ser_num = "Serial001"; thw1.ser_num_len = thw1.ser_num.length();
    thw1.num_p_n = 2;
    thw1.pn_entries.push_back({5, "PartA", 9, "AssemblyA", 18, "Part Description A"});
    thw1.pn_entries.push_back({5, "PartB", 9, "AssemblyB", 18, "Part Description B"});

    FileHandler::ThwEntry thw2;
    thw2.l_n = "Line002"; thw2.l_n_len = thw2.l_n.length();
    thw2.ser_num = "Serial002"; thw2.ser_num_len = thw2.ser_num.length();
    thw2.num_p_n = 1;
    thw2.pn_entries.push_back({5, "PartC", 9, "AssemblyC", 18, "Part Description C"});

    handler.thw_entries = {thw1, thw2};
    handler.generateFile(path, "test.LCL");

    while (true) {
        std::string filename = server.waitForRequest();
        if (filename.empty()) continue;

        std::string ext = filename.substr(filename.find_last_of('.') + 1);
        std::cout << "Received RRQ: " << filename << " (." << ext << ")\n";

        if (ext == "LCI") {
            if (server.shouldAbort()) {
                std::cout << "[SERVER] Aborting this request.\n";
                continue;
            }

            // Generate LCI dynamically
            FileHandler lci;
            lci.protocol_version = 59;
            lci.no_of_thw = 2;
            lci.generateFile(path, filename);

            server.sendFile(filename);

            Information_handler info;
            info.setContext(server.getClientIP(), port, path, "test.LCL", "test.LCS");
            info.launchInfoHandler();
        }
    }
}
