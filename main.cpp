// Program awget

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>

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
    if (num == 0)
        return -1;
    return num;
}

int parse_ip_segment(string line)
{
    if (line.length() == 0)
        return -1;
    int num = 0;
    for (int i = 0; i < line.length(); i++)
    {
        if ((line[i] > '9') || (line[i] < '0'))
            return -1;
        num *= 10;
        num += (line[i] - '0') ;
    }
    if (num > 255)
        return -1;
    return 0;
}

int validate_ip(string ip)
{
    if (ip.length() > 15)
        return -1;
    char ip_str[16];
    strcpy(ip_str, ip.c_str());
    char* token = strtok(ip_str, ".");
    vector<string> ip_segment;
    while (token != NULL)
    {
        ip_segment.push_back(token);
        token = strtok(NULL, ".");
    }

    if (ip_segment.size() != 4)
    {
        // IP must have 4 segments
        return -1;
    }

    for (int i = 0 ; i < 4; i ++)
    {
        if (parse_ip_segment(ip_segment.at(i)))
            return -1;
    }
    return 0;
}

int validate_port(string port)
{
    if (port.length() == 0)
        return -1;
    int num = 0;
    for (int i = 0; i < port.length(); i++)
    {
        if ((port[i] > '9') || (port[i] < '0'))
            return -1;
        num *= 10;
        num += (port[i] - '0') ;
    }
    if (num > 65535)
        return -1;
    return 0;
}

int parse_ip_port(string line, string& ip_port)
{
    stringstream ss(line);
    string ip;
    ss >> ip;
    if (!ss || ss.eof())
        return -1;
    if (validate_ip(ip))
    {
        cerr << "Invalid IP detected." << endl;
        return -1;
    }

    // Start handling port
    string port;
    ss >> port;
    if (!ss || !ss.eof())
        return -1;
    if (validate_port(port))
    {
        cerr << "Invalid port detected." << endl;
        return -1;
    }

    ip_port += ip;
    ip_port += ':';
    ip_port += port;

    return 0;
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

    if (!chainfile)
    {
        chainfile_error_message();
        return -1;
    }

    int num_of_line = parse_first_line(line);

    if (num_of_line == -1)
    {
        chainfile_error_message();
        return -1;
    }

    for (int i = 0; i < num_of_line; i++)
    {
        getline(chainfile, line);
        if (!chainfile)
        {
            cerr << "Wrong number specified in the first line." << endl;
            return -1;
        }
        string ip_port;
        if (parse_ip_port(line, ip_port))
        {
            cerr << "Error parsing IP and port from line " << i+2 << "." << endl;
            return -1;
        }
        stepstones.push_back(ip_port);
    }

    if (!chainfile.eof())
    {
        cerr << "Wrong number specified in the first line." << endl;
        return -1;
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

        if (process_chain_file(chainfile, stepstones))
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