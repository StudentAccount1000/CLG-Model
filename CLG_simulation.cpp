#include <iostream>
#include <vector>
//#include <array>
#include <stdlib.h>
#include <string>
//#include <unordered_map>
#include <map>
#include <chrono>

#include <boost/program_options.hpp>
#include <fstream>
#include <iterator>

using std::array, std::multimap, std::cin, std::cout, std::vector, std::endl, std::pair;

//consider using #pragma for storing bools into single bytes, similar to the bitset solution
//useing libboost-all-dev
//Compile command: g++ -o model CLGmodel.cpp -O -lboost_program_options


class CLG{
    private:
        int L = 10;  //L is the number of particle sites to simulate, default value 10
        int initialN = 5;   //number of initial particles/occupied sites (approximate)
        int N = 0;          //actual number of particles (not generated yet)
        int N00pairs = 0;   //since in the state N>L/2, which is what we care about, there are no N00 states
        int N11pairs = 0;
    public:
        vector<bool> lattice;    //lattice data structure true is occupied by a particle, false is empty
        vector<pair<int, bool>> edge_sites;  //(no collisions), (key) is the position in lattice that is an edge, 
                                                //solves issue of using - and + of int for the key, and -0 vs +0
                                                //true is right side, false is left side edge
                                                //note: could reduce to vector, but vector has worse insertion/deletion times (if not i/d from back)
                                                //***check gaurentee of order 
        

        float generationProbability = 0.0;

        //simulation flags: https://www.boost.org/doc/libs/1_83_0/doc/html/program_options/tutorial.html#id-1.3.30.4.3
        //may prefer https://stackoverflow.com/questions/6892754/creating-a-simple-configuration-file-and-parser-in-c ntg/sbi solution, but has no command line help
        bool generateExactStartingN = false;    //forces the generateLattice to start with exactly n sites. n should be >l/2 NOT IMPLEMENTED
        bool printStepHeaders = true;          //print information about the end of each step
        bool useFixedSeed = false;              //uses the same seed for random number generation, which allows for replicability
        int maxTryMoves = 200;              //depricated, update later
        int maxNumMoves = 100;                  //maximum number of moves

    CLG(int l, int n){     //n should be >L/2, L should be > 2 error checking not yet implemented
        initialN = n;
        L = l;


        generationProbability = float(initialN)/float(L);
        //cout << "N: " << initialN << endl << "L: " << L << endl << "P: " << generationProbability << endl;
        
        srand(time(NULL));  //set rand() seed, otherwise the seed does not change, IMPLEMENT useFixedSeed
    }

    float generateLattice(float P = 0.5){
        float generationPercent = ((P - 0.5)*2) ;  
        //cout << "Generation percent: " << generationPercent << endl;
        bool previous_site_val = true;  // used to track 11 pairs and find edges
        lattice.push_back(true);
        for (int i = 1; i < L; i++){
            int rand_n = rand();
        
            if (i % 2 == 0){
                
                if (!previous_site_val){     // last position is vacant, and the current is occupied, so add it as a left edge
                    edge_sites.push_back(pair{i, false});
                }
                else{   // last position is not vacant, so it forms a pair with the current occupied site
                    N11pairs++;
                }
                lattice.push_back(true);
                previous_site_val = true;
                N++;
            }else{
            	//cout << rand_n % 10 << endl;
                if(generationPercent * 10 > (rand_n % 10)){ //in order to compare need to multiply by generationPercent by 10 in order to have correct comparison
                    lattice.push_back(true);
                    previous_site_val = true;  //can remove this line using current generation methods
                    N++;
                    N11pairs++;
                }else{
                    lattice.push_back(false);
                    previous_site_val = false;
                    edge_sites.push_back(pair{i-1, true}); // current site is false, so previous site is an edge
                    
                }
                
            }
            
        }

        if(lattice[L-1] & L > 1){        // since the lattice wraps, this check to see if the first and last element form a N11 pair
        	N11pairs++;
        } 
        
        lattice.shrink_to_fit();    //done adding, so free memory if able. NON-BINDING, INTERNAL IMPLEMENTATION DEPENDENT
        //cout << "Lattice generated: " << endl;
        return generationPercent;
    }

    int move(){
        int moveTryCounter = 0;
        bool attemptMoveResult = false;
        do{
            attemptMoveResult = attemptMove();
            moveTryCounter++;
        }while(!attemptMoveResult && moveTryCounter < maxTryMoves);

        if(attemptMoveResult == maxTryMoves){
            cout << "Max move attempts reached" << endl;
        }
        if(attemptMoveResult && printStepHeaders){
            cout << ". Took " << moveTryCounter << " attempts" << endl;
        }
        return moveTryCounter;
    }

    bool attemptMove(){ //look into parallelizing
        int rand_index = rand() % edge_sites.size();  //random edge, this is the site to be moved
        pair<int,bool> i = edge_sites[rand_index];
        
        
        if(!i.second){ //need to look to the left
            if(lattice[(i.first + 1) % lattice.size()] == true){    //check to see if moving to the left would create a 00 pair
                if(lattice[(i.first - 1) % lattice.size()] == true){
                    cout << "Possible logic error at position: " << i.first << ", tried to move left and found no 0";
                    return false;
                }
                //swap position *i.first and *i.first-1 % size
                lattice[(i.first - 1) % lattice.size()] = true;
                lattice[i.first] = false;
                //update edges
                    //current edge is removed, edge for occupied site 2 to the left of i (which is assumedly occipied since there are no 00 pairs)
                    //where selected is moved to becomes an edge (in this case a right edge), 1 right of selected init position becomes an edge
                edge_sites[(rand_index - 1) % edge_sites.size()] = pair{(i.first - 1) % int(lattice.size()), true};
                edge_sites[rand_index] = pair{(i.first + 1) % int(lattice.size()), false};
                if(printStepHeaders){
                    cout << "Move made left on <" << i.first << ", " << i.second << "> and was successful, updated edge_sites: " << edge_sites[(rand_index - 1) % edge_sites.size()].first << " and " << edge_sites[rand_index].first;
                }
                return true;
            }
        }else{  //trying to switch with right particle
            if(lattice[(i.first - 1) % lattice.size()] == true){    //check for move creating 00 pair
                if(lattice[(i.first + 1) % lattice.size()] == true){
                    cout << "Possible logic error at position: " << i.first << ", tried to move right and found no 0";
                    return false;
                }
                //swap position *i.first and *i.first+1 % size
                lattice[(i.first + 1) % lattice.size()] = true;
                lattice[i.first] = false;
                //update edges
                edge_sites[(rand_index + 1) % edge_sites.size()] = pair{(i.first + 1) % int(lattice.size()), false};
                edge_sites[rand_index] = pair{(i.first - 1) % int(lattice.size()), true};
                if(printStepHeaders){
                    cout << "Move made right on <" << i.first << ", " << i.second << "> and was successful, updated edge_sites: " << edge_sites[rand_index].first << " and " << edge_sites[(rand_index - 1) % edge_sites.size()].first;
                }
                return true;
            }
        }
        
        return false;
    }


    char* printArray(vector<bool> lat){
        int ind = 0;
        char *outputArr = new char[L];
        for (bool i : lat){
            if(i){
                cout << 1;
                outputArr[ind++] = '1';
            }else{
                cout << 0;
                outputArr[ind++] = '0';
            }
        }
        //cout << endl;
        return outputArr;
    }

    pair<int,int> getNPairs(){  //N11pairs first, then N00pairs
        return pair<int,int> {N11pairs, N00pairs};
    }

    int getN(){
        return N;
    }
};



template<class T>
std::ostream& operator<<(std::ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " ")); 
    return os;
}


//Note: var_map stores the values as the any type, so they need to explicitly recast every time
int main(int argc, char * argv[]){
    //configuation file option variables
    bool use_fixed_seed;
    
    boost::program_options::variables_map var_map;
    try{    //https://www.boost.org/doc/libs/1_81_0/libs/program_options/example/multiple_sources.cpp
        int opt;
        std::string config_file;
        boost::program_options::options_description cmdline("Command Line Options");
        cmdline.add_options()
            ("version,v,ver", "print version")
            ("help,h", "show help message")
            ("config,cfg", boost::program_options::value<std::string>(&config_file)->default_value("CLGconfig.cfg"), "file name of configuration file")
        ;

        boost::program_options::options_description shared("Options for config file and command line");
        shared.add_options()
            ("generate-exact-n", boost::program_options::value<std::string>(), "uses a generation which has the lattice start with exactly N occupied sites. CURRENTLY UNSUPPORTED")
            ("print-steps", "Boolean, prints information after each step/move in the simulation")
            ("use-fixed-seed,fixed-seed", "Boolean, whether or not to use fixed random seed so that the result from multiple runs will be the same")
            ("max-move-tries", "maximum number of tries to make a valid move")
            ("max-num-moves", "maximum number of moves to make in the simulation")
            ("initial-n,n", boost::program_options::value<uint32_t>(), "value of n, the number of initial occupied sites in the lattice")
            ("l-value,l", boost::program_options::value<uint32_t>(), "value of l, the length of the lattice")
        ;

        boost::program_options::options_description hidden_opt("Hidden options");

        hidden_opt.add_options()
            ("input-file", boost::program_options::value<vector<std::string>>(), "input file")
        ;

        boost::program_options::options_description cmdline_options;
        cmdline_options.add(cmdline).add(shared).add(hidden_opt);
        boost::program_options::options_description config_file_options;
        config_file_options.add(shared).add(hidden_opt);
        boost::program_options::options_description visible("Allowed options");
        visible.add(cmdline).add(shared);

        boost::program_options::positional_options_description pos;
        pos.add("input-file", -1);

        //var_map olf location
        store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(pos).run(), var_map);
        notify(var_map);

        std::ifstream config_ifs(config_file.c_str());
        if(config_ifs){
            store(parse_config_file(config_ifs, config_file_options), var_map);
            notify(var_map);
        }else{    
            //false, means file failed to open
            std::string err = "Cannot open config file: " + config_file;
            throw std::invalid_argument(err);
        }


        if(var_map.count("help")){
            cout << visible << endl;
            return 0;   //want to exit the program, not run it if someone asks for help
        }
        if(var_map.count("version")){
            cout << "Version is: 1.0.5, updated 9/11/23\n";
            return 0;
        }
        if(var_map.count("input-file")){
            cout << "Input files are: " << var_map["input-file"].as<vector<std::string>>() << endl;
        }
        
        
        if(var_map.count("use-fixed-seed")){
            //update the setting directly from within here, do not try and edit var_map
            use_fixed_seed = true;
        }
        
        //cout << "n value is: " << var_map["initial-n"] << endl;
        
        
    }
    catch(const std::exception &e){
        cout << e.what() << endl;
        return 1;
    }
    
    
    
    //*********************
    
    const int l = var_map["l-value"].as<uint32_t>();
    const int n = var_map["initial-n"].as<uint32_t>();
    
    cout << "L: " << l << " N: " << n << endl;	//temp line to check conversion and if reading from cfg correctly
    
    const auto start = std::chrono::steady_clock::now();
    CLG sim = CLG(l, n);
    sim.generateLattice(sim.generationProbability);
    const auto end = std::chrono::steady_clock::now();
    cout << float(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000 << " seconds of runtime for lattice generation" << endl;
    if(l <= 20){
        char * out;
        out = sim.printArray(sim.lattice);
        delete[] out;
        cout << "\tN: " << sim.getN() << " N11s: " << sim.getNPairs().first << " N00s: " << sim.getNPairs().second << endl;

    }else{
            cout << "\tN: " << sim.getN() << " N11s: " << sim.getNPairs().first << endl << "N00s: " << sim.getNPairs().second << endl;
    }
    
}