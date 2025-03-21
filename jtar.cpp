#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <sstream>
#include "file.h"

using namespace std;

void printOptions();
void tf(string tarfilename);
void xf(string tarfilename);
void cf(int &argc, char* args[]);
void buildTarfile(string tarfilename, vector<string> &filenames);
void setBitset(bitset<4> &argScenarios, int &argc);
void two_args(int argc, char* args[]);
void tf(string tarfilename);
void xf(string tarfilename, string &dirname, fstream &tarfile);
bool isDir (string filename);
void setContents(string filename, string &pmode, string &size, string &stamp);
void setCanonical(string &stamp, fstream &scratch);
void extractDir(string dirname, vector<File> &files);
void writeToTar(File file, string tarfilename);
void checkErrors(int argc, char* args[]);

int main(int argc, char* args[]) {

        checkErrors(argc, args); //Runs through several error scenarios

        bitset<4> argScenarios; 

        setBitset(argScenarios, argc); //Sets up the bitset

        //This order of these if else statements is required for the program to function
        //If the user enters more than three
        if (argScenarios[3]) 
                cf(argc, args);

        //If the user enters three arguments
        else if (argScenarios[2]) 
                two_args(argc, args);

        //If the user enters two arguments
        else if (argScenarios[1])
                printOptions();
}

void checkErrors(int argc, char* args[]) {

        //Exit program if the user enters less than two args
        if (argc < 2) {
                cerr << "Invalid number of args." << endl;
                exit(1);
        }

        string com = args[1]; //Command should always be this

        //Only allowed command for two args is --help
        if (argc == 2) {
                if (com != "--help") {
                        cerr << "Invalid command or format." << endl;
                        exit(1);
                }
        }

        //Only allowed commands for three args are -tf and -xf
        else if (argc == 3) {
                if (com != "-tf" && com != "-xf") {
                        cerr << "Invalid command or format." << endl;
                        exit(1);
                }

                string tar = args[2];

                fstream tarfile(tar.c_str(), ios::in);

                //Must enter an existing tarfile
                if (!tarfile) {
                        cerr << "You've entered a tar file that does not exist." << endl;
                        exit(1);
                }

                tarfile.close();
        }

        //Only allowed command for more than three args is -cf
        else {
                string tar = args[2];

                if (com != "-cf") {
                        cerr << "Invalid command or format." << endl;
                        exit(1);
                }

                //If the user enters an existing directory as the tarfile, the program will inform them of this
                if (isDir(tar)) {
                        cerr << "You can't make your tarfile a directory." << endl;
                        exit(1);
                }
        }

        //Searches through the entered files and checks if all of them exist
        for (int i = 3; i < argc; i++) {
                string arg = args[i];

                fstream file(arg.c_str(), ios::in);

                if (!file) {
                        cerr << "You've entered a file/directory that does not exist." << endl;
                        exit(1);
                }

                file.close();
        }

}

void two_args(int argc, char* args[]) {

        string com = args[1];
        string tarfilename = args[2];
        string dirname;

        //If the command is xf
        if (com == "-xf") {
                fstream tarfile(tarfilename.c_str(), ios::in | ios::binary); //Declared here to make the recursion work in xf
                int num;
                tarfile.read((char*) &num, sizeof(int));
                xf(tarfilename, dirname, tarfile);
        }

        //If the command is tf
        else if (com == "-tf")
                tf(tarfilename);
}

void tf(string tarfilename) {

        fstream tarfile(tarfilename.c_str(), ios::in | ios::binary);

        File file;
        int num;

        tarfile.read((char*) &num, sizeof(int));

        //Loops through the tarfile to extract all names
        while(tarfile.read((char*) &file, sizeof(File))) {
                cout << file.getName() << endl;
                if (!file.isADir()) {
                        char c;

                        //Skip through the contents of files
                        for (int i = 0; i < stoi(file.getSize()); i++)
                                tarfile.read(&c, sizeof(char));
                }
        }

        tarfile.close();
}

void xf(string tarfilename, string &dirname, fstream &tarfile) {

        File file;

        //Loops through the tarfile and extracts its contents
        while (tarfile.read((char*) &file, sizeof(File))) {

                //Recreates files with their orignal access times, pmodes, and contents
                if (!file.isADir()) { 
                        char c;
                        fstream file_writer(file.getName().c_str(), ios::out | ios::app);

                        for (int i = 0; i < stoi(file.getSize()) && tarfile.read(&c, sizeof(char)); i++)
                                file_writer << c;

                        string command1 = "chmod " + file.getPmode() + " " + file.getName();
                        string command2 = "touch -t " + file.getStamp() + " " + file.getName();

                        system(command1.c_str());
                        system(command2.c_str());

                        file_writer.close();
                }

                //Recreates directories with their original access times, pmodes, and contents
                else {
                        dirname = file.getName();

                        fstream exists(dirname.c_str(), ios::in);

                        if (!exists) {
                                string command1 = "mkdir " + dirname;
                                string command2 = "chmod " + file.getPmode() + " " + dirname;
                                string command3 = "touch -t " + file.getStamp() + " " + dirname; 

                                system(command1.c_str());
                                system(command2.c_str());
                                system(command3.c_str());

                                xf(tarfilename, dirname, tarfile); //Recursive call to access the directory's contents and recreate those
                        }

                        exists.close();
                }
        }

        tarfile.close();
}

void setBitset(bitset<4> &argScenarios, int &argc) {

        //Sets all the slots on the command line that the user entered on
        for (int i = 1; i < argScenarios.size() && i < argc; i++)
                argScenarios[i] = 1;
}

void printOptions() {

        //Displays the user's command options
        cout << "-cf\tMakes a tarfile based on the files and directories entered." << endl;
        cout << "-tf\tLists files and directories part of the tarfile." << endl;
        cout << "-xf\tRecreates the files and directories in the tarfile exactly how they were before." << endl;
}

void cf(int &argc, char* args[]) {

        //Filenames vector filled with the user's entered filenames to store them for the tarfile when creating it
        vector<string> filenames;

        for (int i = 3; i < argc; i++) {
                string filename = args[i];
                filenames.push_back(filename);
        }

        string tarfilename = args[2];

        buildTarfile(tarfilename, filenames); //Construct tarfile
}

void buildTarfile(string tarfilename, vector<string> &filenames) {

        vector<File> files; //Store file objects

        //Loops through the filenames vector to create file objects
        for (int i = 0; i < filenames.size(); i++) {
                string pmode;
                string size;
                string stamp;

                setContents(filenames[i], pmode, size, stamp); //Stores the main credentials of the file in variables

                File file(filenames[i].c_str(), pmode.c_str(), size.c_str(), stamp.c_str()); //New File constructor

                //Flag the File object as a directory, put it in the File object vector, and fish out its contents
                if (isDir(filenames[i])) {
                        file.flagAsDir();
                        files.push_back(file);
                        extractDir(filenames[i], files);
                }

                //Files added to vector
                else
                        files.push_back(file);
        }

        //Write the number of files at the start of the tarfile
        int numFiles = files.size();
        fstream writer(tarfilename.c_str(), ios::out | ios::binary | ios::app);
        writer.write((char*) &numFiles, sizeof(int));
        writer.close();

        //Write the vector of File objects to the tarfile
        for (int i = 0; i < files.size(); i++) 
                writeToTar(files[i], tarfilename);
}

void writeToTar(File file, string tarfilename) {

        fstream tarfile(tarfilename.c_str(), ios::out | ios::binary | ios::app);
        tarfile.write((char*) &file, sizeof(File)); //Write the File object

        //If not a directory, write the file's inner contents to the tarfile
        if (!file.isADir()) {
                string tempfile = "scratch1";
                string command = "cat " + file.getName() + " > " + tempfile;

                system(command.c_str());

                fstream scratch(tempfile.c_str(), ios::in);

                string line;

                while (getline(scratch, line)) {
                        tarfile.write(line.c_str(), line.length());
                        tarfile.write("\n", 1);
                }

                scratch.close();
        }

        tarfile.close();
}

void extractDir(string dirname, vector<File> &files) {

        string ogName = dirname; //Important for going backwards in the directory graph

        string tempfile = "scratch2";
        string command = "ls " + dirname + " > " + tempfile;

        system(command.c_str());

        fstream scratch(tempfile.c_str(), ios::in);

        string line;
        string pmode;
        string size;
        string stamp;

        //Loops through the directory's contents and extracts info from them
        while(getline(scratch, line)) {
                dirname += '/'; //Separation delimeter for system calls
                dirname += line; //Add on the next node to go down to

                setContents(dirname, pmode, size, stamp); //Store the directory's contents in variables

                command = "ls " + ogName + " > " + tempfile; //Reset command to let us go up 

                File file(dirname.c_str(), pmode.c_str(), size.c_str(), stamp.c_str());

                if (isDir(dirname)) {
                        file.flagAsDir(); //Flag as a directory
                        files.push_back(file); //Add to vector of objects
                        extractDir(dirname, files); //Recursive call to go down in the graph
                        system(command.c_str()); //Go up in the graph
                } 

                //Put file into vector
                else
                        files.push_back(file);

                dirname = ogName; //Move back up the graph
        }

        scratch.close();
}

void setContents(string filename, string &pmode, string &size, string &stamp) {

        string tempfile = "scratch3";
        string command = "stat " + filename + " > " + tempfile;

        system(command.c_str()); //Store the statted contents in a file

        fstream scratch(tempfile.c_str(), ios::in);
        string line;

        //Move to the Size line
        getline(scratch, line);
        getline(scratch, line);

        //Break down the string extracted to get the size
        stringstream str1(line);
        getline(str1, line, 'S');
        getline(str1, line, ':');
        getline(str1, line, ' ');
        getline(str1, line, ' ');
        size = line;

        //Move down to the pmode line
        getline(scratch, line);
        getline(scratch, line);

        //Break down the string to get the pmode
        stringstream str2(line);
        getline(str2, line, '(');
        getline(str2, line, '/');
        pmode = line;

        setCanonical(stamp, scratch); //Getting the cannonical form of the access time is long enough to warrant a method

        scratch.close();
}

void setCanonical(string &stamp, fstream &scratch) {

        string line;

        //Move down to the Access Time
        getline(scratch, line);
        getline(scratch, line);

        stringstream date(line); //Let's break down this string

        string canon = ""; //String to be in canonical form

        //Year
        getline(date, line, ' ');
        getline(date, line, '-');
        canon += line;

        //Month
        getline(date, line, '-');
        canon += line;

        //Day
        getline(date, line, ' ');
        canon += line;

        //Hour
        getline(date, line, ':');
        canon += line;

        //Minute
        getline(date, line, ':');
        canon += line;

        //Second
        getline(date, line, '.');
        canon += '.';
        canon += line;

        stamp = canon; //Set the stamp to canon
}

bool isDir (string filename) {

        string tempfile = "scratch4";
        string command = "stat " + filename + " > " + tempfile;

        fstream exists(filename.c_str(), ios::in);

        string line = "";

        //Only reads through an existing directory
        if (exists) {
                system(command.c_str());

                fstream reader(tempfile.c_str(), ios::in);

                //Go down to the line that would possibly contain "directory"
                getline(reader, line);
                getline(reader, line);

                reader.close();
        }

        exists.close();

        return line.find("directory") != -1; //Returns true if the word directory is in the line, false otherwise
}
