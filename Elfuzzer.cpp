// Elfuzzer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <stdlib.h>
#include <set>
#include <filesystem>
#include <fstream>
#include <string>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

namespace fs = std::filesystem;

// a structure that will store real time information about the fuzzer statistics
struct stats {
    int fuzz_cases;
    int crashes;

};

//Get the number of cycles since startup, thus creating a random number
uint64_t rdtsc()
{
    uint32_t hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

//Create a random byte 
class rng {

    public:
        
        int bite;

        //This will prevent the fuzzer from being deterministic
        int make_random() {
            bite^= rdtsc();
        }

        void randomize(void) {
            int val = bite;
            //let's utilize a common technique to generate a deterministic random number
            bite ^= bite << 13;
            bite ^= bite << 17;
            bite ^= bite << 43;
            return val;
    }
};

int fuzz( char cmd[], unsinged char[] input, char[] filename) {

    //Create a tempoary file
    ofstream fileWriter;
    fileWriter(filename, ios::binary | ios::out);
    fileWriter.write((char*)&input, sizeof(input));
    fileWriter.close();

    //Concatonate the command with the filename
    cmd += filename;
   
    std::string result;

    //Execute command
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
    
}

int thd(std::set<std::string> corpus, int thread_num) {
        //Create RNG
        rng random =  rng();
     
        

        unsigned char[] input;
        //infinite loop
        for (;;) {
            //pick a random entry from the corpus
            int index = random.randomize() % corpus.size();
            char[] filename = corpus[index];
            ifstream fileReader;
            fileReader(filename, ios::binary | ios::out);
            while (fileReader) {
                filename << fileReader;
            }

            fuzz()
        }
}

//input: argv[1]: fuzz command, argv[2]: path to corpus directory
int main(int argc, char * argv[])
{

    std::set<std::string> corpus;
    
    std::string path = argv[2];

    //get all the files in the corpus directory
    for (const auto& entry : fs::directory_iterator(path)) {
        corpus.insert(entry.path);
    }

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
