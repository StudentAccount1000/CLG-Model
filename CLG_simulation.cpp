#include <iostream>
#include <vector>
//#include <array>
#include <stdlib.h>
#include <string>
//#include <unordered_map>
#include <map>
#include <chrono>

using std::array, std::multimap, std::cin, std::cout, std::vector, std::endl, std::pair;

//consider using #pragma for storing bools into single bytes, similar to the bitset solution
//Compile command: g++ -o model CLGmodel.cpp -O

class CLG{
    private:
        int L = 10;  //will need to specify before compile time
        int initialN;   //keeping initial amount may be important
        int N = 0;
        int N00pairs = 0;
        int N11pairs = 0;
    public:
        vector<bool> lattice;    //true is occupied, false is empty
        multimap<int, bool> edge_sites;  //multi map (no collisions), (key) is the position in lattice that is an edge, 
                                                //solves issue of using - and + of int for the key, and -0 vs +0
                                                //true is right side, flase is left side edge
                                                //note: could reduce to vector, but vector has worse insertion/deletion times (if not i/d from back)
                                                //check gaurentee of order
        

        float generationProbability = 0.0;

        //simulation flags:
        bool generateExactStartingN = false;    //forces the generateLattice to start with exactly n sites. n should be >l/2 NOT IMPLEMENTED
        bool printStepHeaders = false;

    CLG(int l, int n){     //n should be >L/2, error checking not yet implemented
        initialN = n;
        L = l;


        generationProbability = float(initialN)/float(L);
        //cout << "N: " << initialN << endl << "L: " << L << endl << "P: " << generationProbability << endl;
        
        srand(time(NULL));  //set rand() seed, otherwise it is based on compilation time
    }

    float generateLattice(float P = 0.5){
        float generationPercent = ((P - 0.5)*2) ;  //since in the state N>L/2, which is what we care about, there are no N00 states
        //cout << "Generation percent: " << generationPercent << endl;
        bool previous_site_val = true;
        lattice.push_back(true);
        for (int i = 1; i < L; i++){
            int rand_n = rand();
        
            if (i % 2 == 0){
                
                if (!previous_site_val){     // is last position is vacant, so this is an edge
                    edge_sites.insert(pair{i, false});
                }
                else{
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
                    edge_sites.insert(pair{i-1, true});
                    
                }
                
            }
            
        }

        if(lattice[L-1] & L > 1){        // since the lattice wraps, this check to see if the first and last element for a N11 pair
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
        }while(!attemptMoveResult);
        return moveTryCounter;
    }

    bool attemptMove(){ //look into parallelizing
        int rand_index = rand() % edge_sites.size();  //random edge, this is the site to be moved
        auto i = edge_sites.begin(); 
        
        //iterator placed on the selected edge
        //FIX THIS SECTION LATER
        int counter = 0;
        while(counter < rand_index){
            counter++;
            i++;
        }
        //get a second iterator 

        //for the move it will be in position (i - 1)%edge_sites.size() or (i + 1)%edge_sites.size()
        return false;
    }

    void swap(int pos1, int pos2){ //switches the positions of pos1 and pos2, and updates edge_sites
        
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
        cout << endl;
        return outputArr;
    }

    pair<int,int> getNPairs(){  //N11pairs first, then N00pairs
        return pair<int,int> {N11pairs, N00pairs};
    }

    int getN(){
        return N;
    }
};


int main(){
    cout << "Enter the length of the lattice" << endl;
    std::string inputL;
    cin >> inputL;
    cout << "Enter the number of particles in the lattice" << endl;
    std::string inputN;
    cin >> inputN;
    const int l = stoi(inputL);
    const int n = stoi(inputN);
    const auto start = std::chrono::steady_clock::now();
    CLG sim = CLG(l, n);
    sim.generateLattice(sim.generationProbability);
    const auto end = std::chrono::steady_clock::now();
    cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000 << " seconds of runtime for generation" << endl;
    //char * out;
    //out = sim.printArray(sim.lattice);
    //delete[] out;
    cout << "N11s: " << sim.getNPairs().first << endl << "N00s: " << sim.getNPairs().second << endl;
}