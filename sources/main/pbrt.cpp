
#include<vector>
#include"../core/API/function.h"

int main(int argc, char* argv[])
{
    //parse command-line
    Options options;
    std::vector<std::string> filenames;
    for(int i = 0; i < argc; i++)
    {
        filenames.push_back(argv[i]);
    }

    //initialize
    pbrtInit(options);
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
    pbrtCleanup();
    return 0;
}