#include "ReportGenerator.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <pugixml.hpp>

void ReportGenerator::generateReport(const std::string &reportName, Format format) {
    Logger::log(Logger::INFO, "Generating report: " + reportName);

    try {
        collectData();
        formatReport(format);
        saveReport(reportName, format);
        logReportGeneration(reportName);
    } catch (const std::exception &e) {
        Logger::log(Logger::ERROR, "Failed to generate report: " + std::string(e.what()));
        throw;
    }

    Logger::log(Logger::INFO, "Report generated successfully: " + reportName);
}

std::string ReportGenerator::getCurrentDateTime() {
    std::time_t now = std::time(nullptr);
    std::tm *ltm = std::localtime(&now);
    std::ostringstream oss;
    oss << 1900 + ltm->tm_year << "-"
        << std::setw(2) << std::setfill('0') << 1 + ltm->tm_mon << "-"
        << std::setw(2) << std::setfill('0') << ltm->tm_mday << " "
        << std::setw(2) << std::setfill('0') << ltm->tm_hour << ":"
        << std::setw(2) << std::setfill('0') << ltm->tm_min << ":"
        << std::setw(2) << std::setfill('0') << ltm->tm_sec;
    return oss.str();
}

void ReportGenerator::collectData() {
    //  Need logic data collector
    reportData.push_back("Data point 1: Value 1");
    reportData.push_back("Data point 2: Value 2");
    reportData.push_back("Data point 3: Value 3");
}

void ReportGenerator::formatReport(Format format) {
    std::ostringstream oss;
    if (format == Format::TEXT) {
        oss << "Report generated on: " << getCurrentDateTime() << "\n\n";
        oss << "Report Data:\n";
        for (const auto &data : reportData) {
            oss << data << "\n";
        }
    } else if (format == Format::JSON) {
        nlohmann::json j;
        j["generated_on"] = getCurrentDateTime();
        j["data"] = reportData;
        oss << j.dump(4);
    } else if (format == Format::XML) {
        pugi::xml_document doc;
        auto root = doc.append_child("report");
        root.append_child("generated_on").text() = getCurrentDateTime().c_str();
        auto data = root.append_child("data");
        for (const auto &item : reportData) {
            data.append_child("item").text() = item.c_str();
        }
        std::ostringstream xml_stream;
        doc.save(xml_stream, "  ");
        oss << xml_stream.str();
    } else if (format == Format::CSV) {
        oss << "generated_on," << getCurrentDateTime() << "\n";
        for (const auto &data : reportData) {
            oss << data << "\n";
        }
    }
    formattedReport = oss.str();
}

void ReportGenerator::saveReport(const std::string &reportName, Format format) {
    std::string extension;
    if (format == Format::TEXT) {
        extension = ".txt";
    } else if (format == Format::JSON) {
        extension = ".json";
    } else if (format == Format::XML) {
        extension = ".xml";
    } else if (format == Format::CSV) {
        extension = ".csv";
    }

    std::ofstream file(reportName + extension);
    if (file.is_open()) {
        file << formattedReport;
        file.close();
    } else {
        Logger::log(Logger::ERROR, "Failed to save report: " + reportName);
        throw std::runtime_error("Failed to save report");
    }
}

void ReportGenerator::logReportGeneration(const std::string &reportName) {
    Logger::log(Logger::INFO, "Report saved: " + reportName);
}

