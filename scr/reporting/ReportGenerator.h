#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <string>
#include <vector>

class ReportGenerator {
public:
    enum class Format { TEXT, JSON, XML, CSV };

    void generateReport(const std::string &reportName, Format format = Format::TEXT);

private:
    std::string getCurrentDateTime();
    void collectData();
    void formatReport(Format format);
    void saveReport(const std::string &reportName, Format format);
    void logReportGeneration(const std::string &reportName);

    std::vector<std::string> reportData;
    std::string formattedReport;
};

#endif // REPORTGENERATOR_H

