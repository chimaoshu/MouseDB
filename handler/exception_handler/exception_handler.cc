#include "exception_handler.h"

void raise_exception(status_code code)
{
    throw "error code:" + std::to_string(code);
}