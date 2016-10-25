// Program awget by Chen Wang

#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <sys/socket.h>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

using namespace std;

int sockfd = -1; // Used to store the socket file descriptor, used for cleanup, -1 for sockfd not initialized

void self_exit(int exitcode)
{
    if (sockfd != -1)
    {
        // Need to close sockfd
        close(sockfd);
    }
    exit(exitcode);
}

string tolower(string src)
{
    // Helper function that converts a string to lowercase
    string res = "";
    unsigned int len_src = src.length();
    for (unsigned int i = 0; i < len_src;i++)
    {
        if ((src[i] >= 'A') && (src[i] <= 'Z'))
            res += (src[i] + 'a' - 'A');
        else
            res += src[i];
    }
    return res;
}

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

int parse_nonnegative_int(string input)
{
    if (input.length() == 0)
        return -1;
    int num = 0;
    for (int i = 0; i < input.length(); i++)
    {
        if ((input[i] > '9') || (input[i] < '0'))
            return -1;
        num *= 10;
        num += (input[i] - '0') ;
    }
    return num;
}

int parse_first_line(string line)
{
    int num = parse_nonnegative_int(line);
    if (num == 0)
        return -1;
    return num;
}

int parse_ip_segment(string line)
{
    int num = parse_nonnegative_int(line);
    if (num > 255)
        return -1;
    return 0;
}

int validate_ip(string ip)
{
    if (ip.length() > 15)
        return -1;
    if (tolower(ip) == "localhost")
        return 0;
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
    int num = parse_nonnegative_int(port);
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
        getline(chainfile, line);
        if (line != "")
        {
            cerr << "Wrong number specified in the first line." << endl;
            return -1;
        }
    }

    chainfile.close();
    return 0;
}

int sendout(int sockfd, string url, vector<string> stepstones)
{
    // Send out url
    url = "U: " + url;
    ssize_t ret = send(sockfd, url.c_str(), url.length() + 1, 0);
    if (ret == -1)
    {
        cerr << "Error: Failed to send out url. Program will now terminate.";
        self_exit(1);
    }

    // Send out each step stone ip and port
    vector<string>::iterator iter = stepstones.begin();

    if (stepstones.size() == 0)
    {
        // No Port IP pair to send
        string msgtosend = "P:**";
        ssize_t ret = send(sockfd, msgtosend.c_str(), msgtosend.length() + 1, 0);

        if (ret == -1)
        {
            cerr << "Error: Failed to send out ip_port pair. Program will now terminate.";
            self_exit(1);
        }
        return 0;
    }

    // Have Port IP pair to send
    while (iter != stepstones.end())
    {
        char temp[22];
        strcpy(temp, (*iter).c_str());
        string ip(strtok(temp, ":"));
        string portstr(strtok(NULL, ":"));

        string msgtosend = "P: " + portstr + " S: " + ip + " /P";

        ssize_t ret = send(sockfd, msgtosend.c_str(), msgtosend.length() + 1, 0);

        if (ret == -1)
        {
            cerr << "Error: Failed to send out ip_port pair. Program will now terminate.";
            self_exit(1);
        }

        iter++;
    }
    return 0;
}

int process_sending_receving(string url, vector<string> stepstones)
{
    int num_of_stepstones = stepstones.size();

    // Select random stepstone
    srand(time(NULL));
    int selected = rand() % num_of_stepstones;

    vector<string>::iterator iter = stepstones.begin();
    for (int i = 0; i < selected; i ++)
        iter++;

    string ip_port = *iter;
    stepstones.erase(iter);

    // Get IP and port from string
    char temp[22];
    strcpy(temp, ip_port.c_str());

    string ip(strtok(temp, ":"));
    string portstr(strtok(NULL, ":"));

    int port = parse_nonnegative_int(portstr);

    // Start creating socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        // Failed to get file descriptor
        cout << "Error: Failed to get socket/file descriptor. Program will now terminate." << endl;
        self_exit(-1);
    }

    // Create struct sockaddr_in
    struct sockaddr_in server_address;
    memset(&server_address,0,sizeof(server_address)); // Initialize localAddress (with all zero)

    // Prepare the IP
    inet_pton(AF_INET, ip.c_str(), &(server_address.sin_addr));

    // Prepare the port number
    uint16_t portnumber = (uint16_t) port; // port should be between 0 and 65535, so conversion is safe
    uint16_t network_portnumber = htons(portnumber); // Convert port number into network byte order

    // Prepare the struct sockaddr_in
    server_address.sin_family = AF_INET;
    server_address.sin_port = network_portnumber;

    // Start to connect
    int ret = connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address));

    if (ret == -1)
    {
        cerr << "Error: Failed to connect. Program will now terminate." << endl;
        self_exit(1);
    }

    if (sendout(sockfd, url, stepstones))
        return -1;
    // TODO: receive and reconstruct packets
    // TODO: save the reconstructed binary to the localdisk

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

        return process_sending_receving(url, stepstones);
    }
    else
    {
        // Wrong number of parameters
        commandline_error_message();
        return -1;
    }
    return 0;
}