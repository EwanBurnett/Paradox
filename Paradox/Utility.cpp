#include "Utility.h"

static const std::unordered_map<Paradox::ParadoxError, const char*> kErrorMappings{
    {Paradox::ParadoxError::Success, "Success"},
    {Paradox::ParadoxError::Failed, "Failed"},
    {Paradox::ParadoxError::InitializationFailed, "Initialization Failed"},
    {Paradox::ParadoxError::NotImplemented, "Not Implemented"},
    {Paradox::ParadoxError::ParadoxError_MAX, "Undefined"},
    
};

Paradox::ParadoxError Paradox::CheckError(const Paradox::ParadoxError err) {
    if (err <= Paradox::ParadoxError::Failed) {
        printf("Uh Oh!\n");
    }

    return err;
}

std::string Paradox::GetErrorString(const ParadoxError err)
{
    if (err > ParadoxError::ParadoxError_MAX) {
        return { "Undefined" }; 
    }

    return kErrorMappings.at(err); 
}
