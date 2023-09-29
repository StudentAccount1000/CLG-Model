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
#include <stdexcept>

using std::array, std::multimap, std::cin, std::cout, std::vector, std::endl, std::pair, std::to_string;

//consider using #pragma for storing bools into single bytes, similar to the bitset solution
//useing libboost-all-dev
//Compile command: g++ -o model CLGmodel.cpp -O -lboost_program_options


class CLG{
    private:
        int L = 16;  //L is the number of particle sites to simulate, default value 10
        int initialN = 9;   //number of initial particles/occupied sites (approximate)
        int N = 0;          //actual number of particles (not generated yet)
        int N00pairs = 0;   //since in the state N>L/2, which is what we care about, there are no N00 states
        int N11pairs = 0;
        
        //configuation file option variables
    	
        int max_move_tries = 50;
        int max_num_moves = 5;
    	bool generate_exact_n = false;
        bool print_step_headers = false;
        bool use_fixed_seed = false;
        bool print_initial_lattice = false;
        bool print_execution_time = false;
        
        
    public:
        vector<bool> lattice;    //lattice data structure true is occupied by a particle, false is empty
        vector<int> empty_sites;  //indexed on lattice, contains location of all empty sites
        //sites next to it are not necessarily active, still need to check
        

        float generationProbability = 0.0;



    CLG(boost::program_options::variables_map var_map){
        
        int setup_result = setValues(var_map);
        if(setup_result != 0){
            throw std::invalid_argument("A variable option is formatted wrong or is the wrong type");
        }


        if(L < 4 || initialN < 2){  //range checking N and L
            std::string err = "Error: L must be >= 4 and N must be >= 2. L is " + to_string(L) + " and N is " + to_string(initialN);
            throw std::domain_error(err);
        }
        if(L < initialN || initialN <= L / 2){   //error checking relative ranges on N and L
            std::string err = "N must be > L/2 and N <= L. L is " + to_string(L) + " and N is " + to_string(initialN);
            throw std::domain_error(err);
        }

        lattice.reserve(L);

        generationProbability = float(initialN)/float(L);
        //cout << "N: " << initialN << endl << "L: " << L << endl << "P: " << generationProbability << endl;
        if(use_fixed_seed){
            srand(77777);   //value here does not matter, as long as it is the same every time
        }else{
            srand(time(NULL));
        }
    }



    int setValues(boost::program_options::variables_map var_map){
    
        L = var_map["l-value"].as<uint32_t>();
    	initialN = var_map["initial-n"].as<uint32_t>();
    	max_move_tries = var_map["max-move-tries"].as<uint32_t>();
        max_num_moves = var_map["max-num-moves"].as<uint32_t>();
        
        
        
        if(var_map.count("generate-exact-n")){
            if(var_map["use-fixed-seed"].as<std::string>().compare("false") == 0){
                generate_exact_n = false;
            }else{  //if the cfg file had true or if the flag was specificied from the command line
                generate_exact_n = true;
            }
        }
        if(var_map.count("print-steps")){
            if(var_map["print-steps"].as<std::string>().compare("false") == 0){
                print_step_headers = false;
            }else{  //if the cfg file had true or if the flag was specificied from the command line
                print_step_headers = true;
            }
        }
        if(var_map.count("use-fixed-seed")){
            if(var_map["use-fixed-seed"].as<std::string>().compare("false") == 0){
                use_fixed_seed = false;
            }else{  //if the cfg file had true or if the flag was specificied from the command line
                use_fixed_seed = true;
            }
        }
        if(var_map.count("print-lattice")){
            if(var_map["print-lattice"].as<std::string>().compare("false") == 0){
                print_initial_lattice = false;
            }else{  //if the cfg file had true or if the flag was specificied from the command line
                print_initial_lattice = true;
            }
        }
        if(var_map.count("print-execution-time")){
            if(var_map["print-execution-time"].as<std::string>().compare("false") == 0){
                print_execution_time = false;
            }else{  //if the cfg file had true or if the flag was specificied from the command line
                print_execution_time = true;
            }
        }
        
        cout << "L: " << L << "\ninitialN: " << initialN << "\nmax move tries: " << max_move_tries << "\nmax num moves: " << max_num_moves << "\n\
generate exact n: " << generate_exact_n << "\nprint step headers: " << print_step_headers << "\nuse fixed seed: " << use_fixed_seed << "\n\
print initial lattice: " << print_initial_lattice << "\nprint execution time: " << print_execution_time << endl;
        return 0;
    }



    //accessors
    bool getprint_step_headers(){ return print_step_headers; }
    bool getprint_initial_lattice(){ return print_initial_lattice; }
    bool getprint_execution_time(){ return print_execution_time; }
    int getmax_num_moves(){ return max_num_moves; }



    float generateLattice(float P = 0.5){
        float generationPercent = ((P - 0.5)*2) ;  
        //cout << "Generation percent: " << generationPercent << endl;
        bool previous_site_val = true;  // used to track 11 pairs and find edges
        lattice.push_back(true);
        for (int i = 1; i < L; i++){
            int rand_n = rand();
        
            if (i % 2 == 0){
                
                
                // last position is not vacant, so it forms a pair with the current occupied site
                if(previous_site_val){
                    N11pairs++;
                }
                
                
                lattice.push_back(true);
                previous_site_val = true;
                N++;
            }else{
            	//cout << rand_n % 10 << endl;
                if(generationPercent * 10 > (rand_n % 10)){ //in order to compare need to multiply by generationPercent by 10 in order to have correct comparison
                    lattice.push_back(true);
                    previous_site_val = true;  //line can be removed
                    N++;
                    N11pairs++; //previous is always true using this generation, do not need to check it
                }else{
                    lattice.push_back(false);
                    previous_site_val = false;
                    empty_sites.push_back(i); // current site is false, so it is empty
                    
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
        //check for edge conditions
        if(N == L || empty_sites.size() == 0){  
            cout << "N = L, so no moves can be made" << endl;
            return -1;
        }
        //condition N = L/2 where no moves could be made has already been checked

        int move_try_counter = 0;
        bool attempt_move_result = false;
        int current_move = 0;
        do{
            if(attempt_move_result){    //if this is true, the previous move was a success so the counter needs to be reset
                move_try_counter = 0;
                current_move++;
            }
            attempt_move_result = attemptMove();
            move_try_counter++;
        }while(move_try_counter < max_move_tries && current_move < max_num_moves);

        if(move_try_counter == max_move_tries){
            cout << "Max move attempts reached: " << move_try_counter  << " without finding valid move on move #" << current_move << endl;
        }
        if(attempt_move_result && print_step_headers){
            cout << "Ended moves at " << current_move << " of " << max_num_moves << " moves" << endl;
        }
        return current_move;
    }



    bool attemptMove(){ //look into parallelizing
        int rand_index = rand() % empty_sites.size();  //random edge, this is the site to be moved
        int ind = empty_sites[rand_index];
        
        //random empty site chosen, now choose direction
        char direction = ' ';
        if (rand() % 2 == 0){
            direction = 'l';
        }else{
            direction = 'r';
        }

        if(direction == 'l'){ //check left
            if(lattice[(ind + lattice.size() - 2) % lattice.size()]){    //if the site 2 to the left is not 0, then the site 1 to the left is active
                //cout << (ind-1) << ' ' << ind << ' ' << swap((ind-1)%lattice.size(), ind) << endl;
                if (!swap((ind + lattice.size() - 1) % lattice.size(), ind)){
                    cout << "Move logic fail on " << (ind+lattice.size()-1) % lattice.size() << ',' << ind << "Where rand index is: " << rand_index << ", ind1 and ind2: " << lattice[(ind+lattice.size()-1) % lattice.size()] << ' ' << lattice[ind] << endl;
                    cout << "Array state: ";
                    printArray(lattice);
                    cout << "Empty sites: ";
                    for(int i : empty_sites){ cout << i << ','; }
                    cout << endl;
                    return false;
                }
                empty_sites[rand_index] = (ind + lattice.size() - 1) % lattice.size();	//update empty site position
                return true;    //move succeeded
            }
        }
        //check right
        if (lattice[ind + 2 % lattice.size()]){
            //cout << (ind+1) << ' ' << ind << ' ' << swap((ind+1)%lattice.size(), ind) << endl;
            if(!swap(ind, (ind + 1) % lattice.size())){
                cout << "Move logic fail on " << (ind) << ',' << (ind + 1) % lattice.size() << "Where rand index is: " << rand_index << ", ind1 and ind2: " << lattice[ind] << ' ' << lattice[(ind+1) % lattice.size()] << endl;
                cout << "Array state: ";
                printArray(lattice);
                cout << "Empty sites: ";
                for(int i : empty_sites){ cout << i << ','; }
                    cout << endl;
                return false;
            }
            empty_sites[rand_index] = (ind + 1) % lattice.size();
            return true;
        }
        //neither direction works
        return false;
    }



    bool insert_or_delete(){
        char action = ' ';
        if(N <= L/2){   //if N is L/2 or less no valid deletions can be made 
            action = 'i';
        }else if (N == L){  //if N = L then there are no valid insertions
            action = 'd';
        }else{
            if(rand() % 2 == 0){
                action = 'i';
            }else{
                action = 'd';
            }
        }

        if (action == 'i'){
            int ind = rand() % empty_sites.size();
            lattice[empty_sites[ind]] = true;
            N++;
            N11pairs += 2;  //two new pairs exist because 101 becomes 111
            //remove the empty site since it is not longer active
            empty_sites[ind] = empty_sites[empty_sites.size()-1];
            empty_sites.pop_back();
        }else if(action == 'd'){
            int ind = -1;
            for(int i = 0; i < max_move_tries; i++){
                ind = rand() % lattice.size();
                if(lattice[(ind-1) % lattice.size()] && lattice[ind] && lattice[(ind+1) % lattice.size()]){ //tests to see if ind and ind +- 1 are occupied, so ind can be removed
                    lattice[ind] = false;
                    N11pairs -= 2;
                    empty_sites.push_back(ind); //the sites are not ordered with i/d
                    return true;
                }
            }
        }

        return false;
    }



    bool swap(int a, int b){
        if(lattice[a] ^ lattice[b]){    //xor sites a and b, makes sure you are not swapping two empty or two occupied
            bool t = lattice[a];
            lattice[a] = lattice[b];
            lattice[b] = t;
            return true;
        }
        return false;
    }



    void printArray(vector<bool> lat){
        int ind = 0;
        for (bool i : lat){
            if(i){
                cout << 1;
            }else{
                cout << 0;
            }
        }
        cout << endl;
    }
    
    void printEmptySites(){
        cout << "Empty sites: ";
        for(int i : empty_sites){ cout << i << ','; }
        cout << endl;
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



int main(int argc, char * argv[]){
    
    /*while(*argv != NULL){
        cout << *argv << '\t';
        argv++;
    }
    cout << "\nargc: " << argc << endl;  */  

    //Note: var_map stores the values as the any type, so they need to explicitly recast every time
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
            ("generate-exact-n", "Uses a generation which has the lattice start with exactly N occupied sites. Default is random generation near N. CURRENTLY UNSUPPORTED")
            ("print-steps", "true/false, prints information after each step/move in the simulation")
            ("use-fixed-seed,fixed-seed", "true/false, whether or not to use fixed random seed so that the result from multiple runs will be the same")
            ("print-lattice", "true/false, prints out lattice out after initial generation. Suggested to turn off for any lattice size > 50")
            ("print-execution-time", "true/false, prints out the generation time and time to run all moves")
            ("max-move-tries", boost::program_options::value<uint32_t>(), "Maximum number of tries to make a valid move")
            ("max-num-moves", boost::program_options::value<uint32_t>(), "Maximum number of moves to make in the simulation")
            ("initial-n,n", boost::program_options::value<uint32_t>(), "Value of n, the number of initial occupied sites in the lattice")
            ("l-value,l", boost::program_options::value<uint32_t>(), "Value of l, the length of the lattice")
        ;

        boost::program_options::options_description hidden_opt("Hidden options");

        hidden_opt.add_options()
            ("input-file", boost::program_options::value<vector<std::string>>(), "input file");  //functions from command line but no logic implemented
        

        boost::program_options::options_description cmdline_options;
        cmdline_options.add(cmdline).add(shared).add(hidden_opt);
        boost::program_options::options_description config_file_options;
        config_file_options.add(shared).add(hidden_opt);
        boost::program_options::options_description visible("Allowed options");
        visible.add(cmdline).add(shared);

        boost::program_options::positional_options_description pos;
        pos.add("input-file", -1);
        
        
        //read data from command line arguments
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(pos).run(), var_map);
        boost::program_options::notify(var_map);
        
        //read data from config file (will not overwrite command line options)
        std::ifstream config_ifs(config_file.c_str());
        if(config_ifs){
            boost::program_options::store(parse_config_file(config_ifs, config_file_options), var_map);
            boost::program_options::notify(var_map);
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
            cout << "Version is: 1.0.8, updated 9/28/23\n";
            return 0;
        }
        if(var_map.count("input-file")){ 
            cout << "Input files are: " << var_map["input-file"].as<vector<std::string>>() << endl;
        }

        //timing generation of lattice
        const auto start_generation_timestamp = std::chrono::steady_clock::now();CLG sim = CLG(var_map);
    

        sim.generateLattice(sim.generationProbability);

        const auto end_generation_timestamp = std::chrono::steady_clock::now();  
        if(sim.getprint_execution_time()){
            cout << float(std::chrono::duration_cast<std::chrono::milliseconds>(end_generation_timestamp - start_generation_timestamp).count()) / 1000 << " seconds of runtime for lattice generation" << endl;
        }
        
        //print lattice before moves are made
        if(sim.getprint_initial_lattice()){
            sim.printArray(sim.lattice);
            cout << "\tN: " << sim.getN() << " N11s: " << sim.getNPairs().first << " N00s: " << sim.getNPairs().second << endl;
            sim.printEmptySites();
        }else{
            cout << "\tN: " << sim.getN() << " N11s: " << sim.getNPairs().first << endl << "N00s: " << sim.getNPairs().second << endl;
        }
        
        //timing total time to make moves
        const auto start_move_timestamp = std::chrono::steady_clock::now();
        int actual_moves = sim.move();
        const auto end_move_timestamp = std::chrono::steady_clock::now();
    
        //print move execution time
        if(sim.getprint_execution_time()){
            cout << float(std::chrono::duration_cast<std::chrono::milliseconds>(end_move_timestamp - start_move_timestamp).count()) / 1000 << " seconds of runtime for all moves" << endl;
        }
    
        
        //logging: to be added later
    
    }
    catch(const std::exception &e){
        cout << e.what() << endl;
        return 1;
    }
    
    //*********************
    return 0;
}