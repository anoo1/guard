// SPDX-License-Identifier: Apache-2.0
#include "libguard/guard_interface.hpp"
#include "libguard/include/guard_record.hpp"

#include <config.h>

#include <CLI/CLI.hpp>

using namespace std;
using namespace openpower::guard;

void guardList(bool displayResolved)
{
    auto records = getAll();
    if (!records.size())
    {
        std::cout << "No Records to display" << std::endl;
        return;
    }

    bool isHeaderPrinted = false;
    for (const auto& elem : records)
    {
        // Not to print guard records with errorlog type set as GARD_Reconfig
        // As guard below type records are for internal usage only.
        if (elem.errType == GARD_Reconfig)
        {
            continue;
        }
        // To list resolved records as user wants to list resolved records
        else if (displayResolved && (elem.recordId != GUARD_RESOLVED))
        {
            continue;
        }
        // To list unresolved records as user wants to list unresolved records
        else if (!displayResolved && (elem.recordId == GUARD_RESOLVED))
        {
            continue;
        }

        // Don't print the header if already printed header since
        // records mixed of resolved and unresolved records so if have
        // only either one in retrieved records and user tried to see opposite
        // one then we should not print header else user will get confused.
        if (!isHeaderPrinted)
        {
            std::cout << "ID       | ERROR    |  Type  | Path " << std::endl;
            isHeaderPrinted = true;
        }

        std::cout << std::hex << std::setw(8) << std::setfill('0')
                  << elem.recordId;

        std::cout << " | ";
        std::cout << std::hex << std::setw(8) << std::setfill('0')
                  << elem.elogId;

        std::cout << " | ";
        std::optional<std::string> gReasonToStr =
            guardReasonToStr(elem.errType);
        std::cout << *gReasonToStr;

        std::cout << " | ";
        std::optional<std::string> physicalPath =
            getPhysicalPath(elem.targetId);
        if (!physicalPath)
        {
            std::cout << "Unknown ";
        }
        else
        {
            std::cout << *physicalPath;
        }
        std::cout << std::endl;
    }

    if (!isHeaderPrinted)
    {
        std::cout << "No "
                  << (displayResolved == true ? "resolved" : "unresolved")
                  << " records to display" << std::endl;
    }
}

void guardDelete(const std::string& physicalPath)
{
    std::optional<EntityPath> entityPath = getEntityPath(physicalPath);
    if (!entityPath)
    {
        std::cerr << "Unsupported physical path " << physicalPath << std::endl;
        return;
    }
    clear(*entityPath);
}

void guardClear()
{
    clearAll();
}

void guardInvalidateAll()
{
    invalidateAll();
}

void guardCreate(const std::string& physicalPath)
{
    std::optional<EntityPath> entityPath = getEntityPath(physicalPath);
    if (!entityPath)
    {
        std::cerr << "Unsupported physical path " << physicalPath << std::endl;
        return;
    }
    create(*entityPath);
    std::cout << "Success" << std::endl;
}

static void exitWithError(const std::string& help, const char* err)
{
    std::cerr << "ERROR: " << err << std::endl << help << std::endl;
    exit(-1);
}

int main(int argc, char** argv)
{
    try
    {
        CLI::App app{"Guard Tool"};
        std::optional<std::string> createGuardStr;
        std::optional<std::string> deleteGuardStr;
        bool listGuardRecords = false;
        bool clearAll = false;
        bool listResolvedGuardRecords = false;
        bool invalidateAll = false;
        bool gversion = false;

        app.set_help_flag("-h, --help", "Guard CLI tool options");
        app.add_option("-c, --create", createGuardStr,
                       "Create Guard record, expects physical path as input");
        app.add_option(
            "-i, --invalidate", deleteGuardStr,
            "Invalidate a single Guard record, expects physical path as input");
        app.add_flag("-I, --invalidate-all", invalidateAll,
                     "Invalidates all the Guard records");
        app.add_flag("-l, --list", listGuardRecords,
                     "List all the Guard'ed resources");
        app.add_flag("-r, --reset", clearAll, "Erase all the Guard records");
        app.add_flag("-a, --listresolvedrecords", listResolvedGuardRecords,
                     "List all the resolved Guard'ed resources")
            ->group("");
        app.add_flag("-v, --version", gversion, "Version of GUARD tool");

        CLI11_PARSE(app, argc, argv);

        libguard_init();

        if (createGuardStr)
        {
            guardCreate(*createGuardStr);
        }
        else if (deleteGuardStr)
        {
            guardDelete(*deleteGuardStr);
        }
        else if (clearAll)
        {
            guardClear();
        }
        else if (listGuardRecords)
        {
            guardList(false);
        }
        else if (listResolvedGuardRecords)
        {
            guardList(true);
        }
        else if (invalidateAll)
        {
            guardInvalidateAll();
        }
        else if (gversion)
        {
            std::cout << "Guard tool " << GUARD_VERSION << std::endl;
        }
        else
        {
            exitWithError(app.help("", CLI::AppFormatMode::All),
                          "Invalid option");
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Exception: " << ex.what() << std::endl;
    }
    return 0;
}
