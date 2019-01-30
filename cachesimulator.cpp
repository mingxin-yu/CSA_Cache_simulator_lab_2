/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss

struct config{
       int L1blocksize;
       int L1setsize;
       int L1size;
       int L2blocksize;
       int L2setsize;
       int L2size;
       };

unsigned long shiftbits(bitset<32> inst, int start)
{
  unsigned long ulonginst;
  return ((inst.to_ulong())>>start);
}

unsigned long find_index(bitset<32> addr, int tag, int index, int offset)
{
	bitset<32> buffer = (addr.to_ulong())<< tag;
	return (buffer.to_ulong()>> (tag+offset));
}
      
//----------------------------------------------- Cache ---------------------------------------------
class Cache
{
    public: 
        int cache_data;
        int num_data;

     	Cache(int cache_size)
    	{ 
			cache.resize(cache_size);
			cout << "The total size of the cache is " << cache_size << endl;
			num.resize(cache_size);
        }
	
        bool read_Cache(int cache_addr, int tag, int index, int offset, int associavity, int counter_value)
        {   
        	int value_of_index = find_index(cache_addr,tag,index,offset);
        	for (int set = 0; set < associavity; set ++){
        		if (cache[ set * cache.size()/associavity + value_of_index] != 0){
        			if (bitset<32> (shiftbits(cache[set * cache.size()/associavity + value_of_index], offset)) == bitset<32> (shiftbits(cache_addr,offset)))
        			{
        				num[ set * cache.size()/associavity + value_of_index] = counter_value;
        				return true;
        			}
        		}
        	}
        	return false;
        }
    
        void write_Cache(int cache_addr, int tag, int index, int offset, int associavity, int counter_value)
        {
        	int value_of_index = find_index(cache_addr, tag, index, offset);
        	for (int set = 0; set < associavity; set ++){

        		if (bitset<32> (shiftbits(cache[set * cache.size()/associavity + value_of_index], offset)) == bitset<32> (shiftbits(cache_addr,offset))){
        			num[ set * cache.size()/associavity + value_of_index] = counter_value;
        			return;
        		}
        		else if (cache[ set * cache.size()/associavity + value_of_index] == 0){
        			cache[ set * cache.size()/associavity + value_of_index] = cache_addr;
        			num[ set * cache.size()/associavity + value_of_index] = counter_value;
        			return;
        		}
        		if (set == associavity - 1)
        		{
        			int smallest = num[value_of_index];
        			int buffer = 0;
        			for (int q = 0; q < associavity; q++){
        				if (num[q * cache.size()/associavity + value_of_index] < smallest){
        					smallest = num[q * cache.size()/associavity + value_of_index];
        					buffer = q;
        				}
        			}
        			cache[buffer * cache.size()/associavity + value_of_index ] = cache_addr;
        			num[buffer * cache.size()/associavity + value_of_index ] = counter_value;
        			return;
        		}
        	}
        }
	private:
		vector<int> cache;
		vector<int> num;
};


int main(int argc, char* argv[]){

    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while(!cache_params.eof())  // read config file
    {
      cache_params>>dummyLine;
      cache_params>>cacheconfig.L1blocksize;
      cache_params>>cacheconfig.L1setsize;              
      cache_params>>cacheconfig.L1size;
      cache_params>>dummyLine;              
      cache_params>>cacheconfig.L2blocksize;           
      cache_params>>cacheconfig.L2setsize;        
      cache_params>>cacheconfig.L2size;
    }

    cout << "L1 block size " << cacheconfig.L1blocksize << endl;
    cout << "L1 set size " << cacheconfig.L1setsize << endl;
    cout << "L1 size " << cacheconfig.L1size << endl;

    cout << "L2 block size " << cacheconfig.L2blocksize << endl;
    cout << "L2 set size " << cacheconfig.L2setsize << endl;
    cout << "L2 size " << cacheconfig.L2size << endl;

//---------------------------- calculation for tag index and offset -----------------------------

    int L1_offset = log2(cacheconfig.L1blocksize);
    int L1_index;
    if (cacheconfig.L1setsize == 0){
    	L1_index = 0;
    	cacheconfig.L1setsize = cacheconfig.L1size*1024/cacheconfig.L1blocksize;
    }
    else L1_index = log2(cacheconfig.L1size * 1024 / cacheconfig.L1blocksize / cacheconfig.L1setsize);
    int L1_tag = 32 - L1_offset - L1_index;

    cout << "L1 offset " << L1_offset << endl;
    cout << "L1 index " << L1_index << endl;
    cout << "L1 tag " << L1_tag << endl;

    int L2_offset = log2(cacheconfig.L2blocksize);
    int L2_index;

    if (cacheconfig.L2setsize == 0){
    	L2_index = 0;
    	cacheconfig.L2setsize = cacheconfig.L2size*1024/cacheconfig.L2blocksize;
    }
    else L2_index = log2(cacheconfig.L2size * 1024 / cacheconfig.L2blocksize / cacheconfig.L2setsize);
    int L2_tag = 32 - L2_offset - L2_index;

    cout << "L2 offset " << L2_offset << endl;
    cout << "L2 index " << L2_index << endl;
    cout << "L2 tag " << L2_tag << endl;

//---------------------------- calculation for tag index and offset -----------------------------

//---------------------------- create the L1 and L2 cache -----------------------------
    Cache L1(cacheconfig.L1size*1024/cacheconfig.L1blocksize);
    Cache L2(cacheconfig.L2size*1024/cacheconfig.L2blocksize);
//---------------------------- create the L1 and L2 cache -----------------------------

//---------------------------- define the counter -----------------------------
    int counter = 0;
    int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
   
   
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    
    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;        
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;
    
    int cycle = 0;
    if (traces.is_open()&&tracesout.is_open()){    
        while (getline (traces,line))
        {   // read mem access file and access Cache
        	cycle++;
        	counter++; 
            istringstream iss(line); 
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);

            if (accesstype.compare("R")==0){	
            	if (L1.read_Cache(accessaddr.to_ulong(),L1_tag,L1_index,L1_offset,cacheconfig.L1setsize,counter)){
            		L1AcceState = RH;
            		L2AcceState = NA;
            	}
            	else if (L2.read_Cache(accessaddr.to_ulong(),L2_tag,L2_index,L2_offset,cacheconfig.L2setsize,counter)){
            		L1.write_Cache(accessaddr.to_ulong(),L1_tag,L1_index,L1_offset,cacheconfig.L1setsize,counter);
            		L1AcceState = RM;
            		L2AcceState = RH;
            	}
            	else{
            		L1AcceState = RM;
            		L2AcceState = RM;
            		L1.write_Cache(accessaddr.to_ulong(),L1_tag,L1_index,L1_offset,cacheconfig.L1setsize,counter);
            		L2.write_Cache(accessaddr.to_ulong(),L2_tag,L2_index,L2_offset,cacheconfig.L2setsize,counter);
            	}
            }
            else 
            {   
            	if (L1.read_Cache(accessaddr.to_ulong(),L1_tag,L1_index,L1_offset,cacheconfig.L1setsize,counter)){
            		L1.write_Cache(accessaddr.to_ulong(),L1_tag,L1_index,L1_offset,cacheconfig.L1setsize,counter);
            		L1AcceState = WH;
            	}
            	else{
            		L1AcceState = WM;
            	}
            	if (L2.read_Cache(accessaddr.to_ulong(),L2_tag,L2_index,L2_offset,cacheconfig.L2setsize,counter)){
            		L2.write_Cache(accessaddr.to_ulong(),L2_tag,L2_index,L2_offset,cacheconfig.L2setsize,counter);
            		L2AcceState = WH;
            	}
            	else{
            		L2AcceState = WM;
            	}
            }
            tracesout<< L1AcceState << " " << L2AcceState << endl;  // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close(); 
    }
    else cout<< "Unable to open trace or traceout file ";
   
    return 0;
}