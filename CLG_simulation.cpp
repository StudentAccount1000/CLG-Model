//7_28_23
#include <iostream>
#include <vector>
//#include <array>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <chrono>
//g++ -o model CLGmodel.cpp -O

using std::array, std::unordered_map, std::cin, std::cout, std::vector, std::endl, std::pair;

//ubuntu vector max size: 9,223,372,036,854,775,744 (or apptx. 8EX of data)


class CLG{
    private:
        int L = 10;  //will need to specify before compile time
        int initialN;   //keeping initial amount may be important
        int N = 0;
        int N00pairs = 0;
        int N11pairs = 0;
    public:
        vector<bool> lattice;    //true is occupied, false is empty
        unordered_map<int, bool> active_sites;  //hash map, abs(key) is the position in lattice that is an edge, 
                                                //key < 0 left side edge, key > 0 right side edge
                                                //possible issue: key is 0, cannot have -0 as a key
        

        float generationProbability = 0.0;

        //simulation flags:
        bool generateExactStartingN = false;    //forces the generateLattice to start with exactly n sites. n should be >l/2 NOT IMPLEMENTED
        bool printStepHeaders = false;

    CLG(int l, int n){     //n should be >L/2
        initialN = n;
        L = l;
	
	lattice.reserve(L);	//can overallowcate in order to keep block size
        cout << "Capacity: " << lattice.capacity() << " Size: " << lattice.size() << endl;

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
                    active_sites[i*-1] = false;
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
                    if(active_sites.find((i-1) * -1) != active_sites.end()){    //contains only exists in C++20 or newer, but is option
                        active_sites[(i-1) * -1] = true;
                        active_sites[i-1] = true;
                    }else{
                        active_sites[i-1] = false;
                    }
                    
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
    for(int i = 0; i < 3; i++){
    const auto start = std::chrono::steady_clock::now();
    CLG sim = CLG(l, n);
    sim.generateLattice(sim.generationProbability);
    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> total_time = end - start;
    cout << total_time.count() << " seconds of runtime for generation" << endl;	//must use .count() before C++20
    /*char * out;
    out = sim.printArray(sim.lattice);
    delete[] out;*/
    cout << "N11s: " << sim.getNPairs().first << endl << "N00s: " << sim.getNPairs().second << endl;
    }
}