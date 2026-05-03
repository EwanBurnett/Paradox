#include "Utility.h"
#include "Logger.h"
#include "Profiler.h"

static const std::unordered_map<Paradox::ParadoxError, const char*> kErrorMappings{
    {Paradox::ParadoxError::Success, "Success"},
    {Paradox::ParadoxError::Failed, "Failed"},
    {Paradox::ParadoxError::InitializationFailed, "Initialization Failed"},
    {Paradox::ParadoxError::NotImplemented, "Not Implemented"},
    {Paradox::ParadoxError::ParadoxError_MAX, "Undefined"},

};

Paradox::ParadoxError Paradox::CheckError(const Paradox::ParadoxError err) {
    ParadoxZoneScoped;
    if (err <= Paradox::ParadoxError::Failed) {
        Log::Print(ELogColour::LightMagenta, "[Paradox] Internal Error - %s\n", GetErrorString(err).c_str());
    }

    return err;
}

std::string Paradox::GetErrorString(const ParadoxError err)
{
    ParadoxZoneScoped;
    if (err > ParadoxError::ParadoxError_MAX) {
        return { "Undefined" };
    }

    return kErrorMappings.at(err);
}
