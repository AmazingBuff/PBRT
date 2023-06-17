
#include<vector>
#include"core/API/api.h"

int main(int argc, char* argv[])
{
    //parse command-line
    pbrt::Options options;
    std::vector<std::string> filenames;
    for(int i = 0; i < argc; i++)
    {
        filenames.push_back(argv[i]);
    }

    //initialize
    pbrt::pbrtInit(options);
    //process scene description
    if(filenames.size() == 0)
    {
        //parse scene from standard input


    }
    else
    {
        //parse scene from input files
    }
    //cleanup
    pbrt::pbrtCleanup();
    return 0;
}