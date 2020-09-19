#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <set>
#include <string>



namespace fs = std::filesystem;

// a structure that will store real time information about the fuzzer statistics
typedef struct  {
    int fuzz_cases;
    int crashes;

} stats;

//Global statistics for the fuzzer
extern stats statistics;

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

        int randomize(void) {
            int val = bite;
            //let's utilize a common technique to generate a deterministic random number
            bite ^= bite << 13;
            bite ^= bite << 17;
            bite ^= bite << 43;
            return val;
    }
};

int fuzz( char cmd[], unsigned char input[]) {

    //Hash the input, creating a tempoary filename
    std::hash<unsigned char*> ptr_hash;
     size_t temp = ptr_hash(input);
     std::string temp_string = std::to_string(temp);

    //Create a tempoary file
    std::ofstream fileWriter(temp_string);
    fileWriter(temp_string, ios::binary | ios::out);
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
    //Get the return code
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    //Cleanup and Update
    statistics.fuzz_cases++;
    std::remove(temp_string);
    return result;
    
}

int thd(std::set<std::string> corpus, char command[]) {
        //Create RNG
        rng random =  rng();
     
        

        unsigned char[] input;
        //infinite loop
        for (;;) {
            //pick a random entry from the corpus
            int index = random.randomize() % corpus.size();
            char filename[] = corpus[index];

            //Write this file into a bytearray
            ifstream fileReader;
            fileReader(filename, ios::binary | ios::in);
            fileReader.seekg(0, ios::end);
            size_t len = fileReader.tellg();
            char *ret = new char[len];
            fileReader.read(ret, len);
            fileReader.close();


            int exitcode =fuzz(command, ret);

            if(exitcode >0){
                statistics.crashes++;
            }
        }
}

//input: argv[1]: fuzz command, argv[2]: path to corpus directory, argv[3] number of threads
int main(int argc, char * argv[])
{
    stats live;

    std::set<std::string> corpus;
    
    std::string path = argv[2];

    //get all the files in the corpus directory
    for (const auto& entry : fs::directory_iterator(path)) {
        corpus.insert(entry.path);
    }
    //Create threads too fuzz concurrently
    for(int id=0; id< argv[3]; id++){
        std::thread(thd, corpus, argv[1]);
    }

    for(;;){
        io::cout << "Number of fuzz cases" <<statistics.fuzz_cases << endl;
        io::cout << "Number of crashes:" << statistics.crashes <<endl;
    }

}
