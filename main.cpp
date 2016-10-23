// Program awget

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

using namespace std;

void commandline_error_message()
{
    cerr << "Error: Invalid parameter received." << endl;
    cerr << "Usage: awget <URL> [-c chainfile]" << endl;
    cerr << "If chainfile is not specified, program will look for chaingang.txt under the current directory." << endl;
}

void chainfile_error_message()
{
    cerr << "Error reading the chain file. Please check chain file format." << endl;
}

int parse_first_line(string line)
{
    int num = 0;
    for (int i = 0; i < line.length(); i++)
    {
        if ((line[i] > '9') || (line[i] < '0'))
            return -1;
        num *= 10;
        num += (line[i] - '0') ;
    }
    return num;
}

int process_chain_file(string chainfilename, vector<string>& stepstones)
{
    ifstream chainfile;
    chainfile.open(chainfilename);
    string line;
    if (!chainfile.is_open())
    {
        cerr << "Failed to open the chainfile. Does the file exist?" << endl;
        return -1;
    }

    getline(chainfile, line);

    if (chainfile)
    {
        chainfile_error_message();
        return -1;
    }

    if (parse_first_line(line) == -1)
    {
        chainfile_error_message();
        return -1;
    }

    while (!chainfile.eof())
    {
        getline(chainfile, line);
        if (chainfile)
            cout << line << endl;
    }
    chainfile.close();
    return 0;
}

int main(int argc, char* argv[]) {
    if ((argc == 2) || (argc == 4))
    {
        string url(argv[1]);

        if ((argc == 4) && (strcmp("-c", argv[2]) != 0))
        {
            // Unexpected parameter at "-c"
            commandline_error_message();
            return -1;
        }

        string chainfile = "chaingang.txt";

        if (argc == 4)
        {
            string temp(argv[3]);
            chainfile = temp;
        }

        vector<string> stepstones;
        stepstones.reserve(20);

        if (!process_chain_file(chainfile, stepstones))
            return -1;



    }
    else
    {
        // Wrong number of parameters
        commandline_error_message();
        return -1;
    }
    return 0;
}