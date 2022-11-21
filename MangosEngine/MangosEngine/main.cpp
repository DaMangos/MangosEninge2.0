#include "mgo_application.hpp"
int main()
{
    try
    {
        mgo::Application application;
        application.run();
    }
    catch (const std::exception& errorMessage)
    {
        MGO_LOG_ERROR(errorMessage.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
