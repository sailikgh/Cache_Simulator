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

/* you can define the cache class here, or design your own data structure for L1 and L2 cache
class cache {

      }
*/

class Cache
{
public:
    int b;
    int s;
    int t;
    int noOfSets;
    int noOfSlots;
    int noOfWays;
    vector<u_long> data;
    vector<u_long> tag;
    vector<int> counter;
    int counterFA;
    vector<int> isValid;

    void structureCache(int blocksize, int setsize, int sizeKB)
    {
        unsigned long size = sizeKB * 1024;
        if (setsize == 0)
        {
            b = log2(blocksize);
            noOfSets = 0;
            s = 0;
            noOfSlots = size/blocksize;
            t = 32-b;
            counterFA = 0;
        }
        else if (setsize == 1)
        {
            b = log2(blocksize);
            noOfSets = size/blocksize;
            s = log2(noOfSets);
            t = 32 - s - b;
            counterFA = -1;
        }
        else if (setsize > 1)
        {
            b = log2(blocksize);
            noOfSets = size/(blocksize * setsize);
            s = log2(noOfSets);
            t = 32 - s - b;
            counterFA = -1;
            counter.resize(noOfSets);
            for (int i=0; i < noOfSets; i++)
            {
                counter[i] = 0;
            }
        }
        noOfWays = setsize;
        data.resize(size);
        if (noOfWays != 0)
        {
            tag.resize(noOfWays * noOfSets);
            isValid.resize(noOfWays * noOfSets);
        }
        else
        {
            tag.resize(noOfSlots);
            isValid.resize(noOfSlots);
        }

        for (int i=0; i < isValid.size(); i++)
        {
            isValid[i] = 0;
        }

    }
};

Cache l1;
Cache l2;

void updateCache(int l2AccState, bitset<32> accessaddr)
{
    unsigned long l2Tag;
    unsigned long l2Index;
    bool evictFroml1 = false;
    if ((l2AccState == 1) || (l2AccState == 2))
    {
        unsigned long l1CacheAddrIndex;
        //Write Into L1
        if (l1.noOfWays == 0)
        {
            l1CacheAddrIndex = std::stoul(accessaddr.to_string().substr(0, l1.t), nullptr, 2);
            if (l1.counterFA != -1)
            {
                if (l1.isValid[l1.counterFA] == 1)
                {
                    l2Index = l1.counterFA;
                    l2Tag = l1.tag[l1.counterFA];
                    evictFroml1 = true;
                }
                l1.tag[l1.counterFA] = l1CacheAddrIndex;
                l1.isValid[l1.counterFA] = 1;
                l1.counterFA++;
                if (l1.counterFA == l1.noOfSlots)
                {
                    l1.counterFA = 0;
                }
            }
        }
        else if (l1.noOfWays == 1)
        {
            l1CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l1.t, l1.s), nullptr, 2);
            unsigned long tag;
            tag = std::stoul(accessaddr.to_string().substr(0, l1.t), nullptr, 2);
            if (l1.isValid[l1CacheAddrIndex] == 1)
            {
                l2Index = l1CacheAddrIndex;
                l2Tag = l1.tag[l1CacheAddrIndex];
                evictFroml1 = true;
            }
            l1.tag[l1CacheAddrIndex] = tag;
            l1.isValid[l1CacheAddrIndex] = 1;
        }
        else if (l1.noOfWays > 1)
        {
            l1CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l1.t, l1.s), nullptr, 2);
            unsigned long tag;
            tag = std::stoul(accessaddr.to_string().substr(0, l1.t), nullptr, 2);
            unsigned long replaceSlotIndex;
            bool isEmpty = false;
            for (int i=0; i < l1.noOfWays; i++)
            {
                if (l1.isValid[(l1CacheAddrIndex * l1.noOfWays) + i] == 0)
                {
                    l1.tag[(l1CacheAddrIndex * l1.noOfWays) + i] = tag;
                    l1.isValid[(l1CacheAddrIndex * l1.noOfWays) + i] = 1;
                    isEmpty = true;
                    break;
                }
            }
            if (!isEmpty)
            {
                replaceSlotIndex = l1.counter[l1CacheAddrIndex];
                if (l1.isValid[(l1CacheAddrIndex * l1.noOfWays) + replaceSlotIndex] == 1)
                {
                    l2Index = (l1CacheAddrIndex * l1.noOfWays) + replaceSlotIndex;
                    l2Tag = l1.tag[(l1CacheAddrIndex * l1.noOfWays) + replaceSlotIndex];
                    evictFroml1 = true;
                }
                l1.tag[(l1CacheAddrIndex * l1.noOfWays) + replaceSlotIndex] = tag;
                l1.isValid[(l1CacheAddrIndex * l1.noOfWays) + replaceSlotIndex] = 1;
                l1.counter[l1CacheAddrIndex]++;
                if (l1.counter[l1CacheAddrIndex] == l1.noOfWays)
                {
                    l1.counter[l1CacheAddrIndex] = 0;
                }
            }
        }
        //Evicting From L2 : Handled in main

        //Adding To L2
        if (evictFroml1 == true)
        {
            bitset<32> l1T = bitset<32>(l2Tag);
            bitset<32> l1I = bitset<32>(l2Index);
            string l1TBits = l1T.to_string().substr(32 - l1.t, l1.t);
            string l1IBits = l1I.to_string().substr(32 - l1.s, l1.s);
            string l2Addr = l1TBits + l1IBits;
            unsigned long l2CacheAddrIndex;
            if (l2.noOfWays == 0) {
                l2CacheAddrIndex = std::stoul(l2Addr.substr(0, l2.t), nullptr, 2);
                if (l2.counterFA != -1) {
                    l2.tag[l2.counterFA] = l2CacheAddrIndex;
                    l2.isValid[l2.counterFA] = 1;
                    l2.counterFA++;
                    if (l2.counterFA == l2.noOfSlots) {
                        l2.counterFA = 0;
                    }
                }
            } else if (l2.noOfWays == 1) {
                l2CacheAddrIndex = std::stoul(l2Addr.substr(l2.t, l2.s), nullptr, 2);
                unsigned long tagl2;
                tagl2 = std::stoul(l2Addr.substr(0, l2.t), nullptr, 2);
                l2.tag[l2CacheAddrIndex] = tagl2;
                l2.isValid[l2CacheAddrIndex] = 1;
            } else if (l2.noOfWays > 1) {
                l2CacheAddrIndex = std::stoul(l2Addr.substr(l2.t, l2.s), nullptr, 2);
                unsigned long tagl2;
                tagl2 = std::stoul(l2Addr.substr(0, l2.t), nullptr, 2);
                unsigned long replaceSlotIndexl2;
                bool isEmptyl2 = false;
                for (int i = 0; i < l2.noOfWays; i++) {
                    if (l2.isValid[(l2CacheAddrIndex * l2.noOfWays) + i] == 0) {
                        l2.tag[(l2CacheAddrIndex * l2.noOfWays) + i] = tagl2;
                        l2.isValid[(l2CacheAddrIndex * l2.noOfWays) + i] = 1;
                        isEmptyl2 = true;
                        break;
                    }
                }
                if (!isEmptyl2) {
                    replaceSlotIndexl2 = l2.counter[l2CacheAddrIndex];
                    if (l2.isValid[(l2CacheAddrIndex * l2.noOfWays) + replaceSlotIndexl2] == 1) {
                        l2Index = (l2CacheAddrIndex * l2.noOfWays) + replaceSlotIndexl2;
                        l2Tag = l2.tag[(l2CacheAddrIndex * l2.noOfWays) + replaceSlotIndexl2];
                    }
                    l2.tag[(l2CacheAddrIndex * l2.noOfWays) + replaceSlotIndexl2] = tagl2;
                    l2.isValid[(l2CacheAddrIndex * l2.noOfWays) + replaceSlotIndexl2] = 1;
                    l2.counter[l2CacheAddrIndex]++;
                    if (l2.counter[l2CacheAddrIndex] == l2.noOfWays) {
                        l2.counter[l2CacheAddrIndex] = 0;
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
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



    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like

    l1.structureCache(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size);
    l2.structureCache(cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);


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

    if (traces.is_open()&&tracesout.is_open()){
        while (getline (traces,line)){   // read mem access file and access Cache

            int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
            int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);


            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R")==0)
            {
                //Implement by you:
                // read access to the L1 Cache,
                //  and then L2 (if required),
                //  update the L1 and L2 access state variable;
                unsigned long l1CacheAddrIndex;
                unsigned long l2CacheAddrIndex;

                //Read Hit On L1
                if (l1.noOfWays == 0)
                {
                    l1CacheAddrIndex = std::stoul(accessaddr.to_string().substr(0, l1.t), nullptr, 2);
                    for (int i=0 ; i < l1.noOfSlots; i++)
                    {
                        if ((l1.tag[i] == l1CacheAddrIndex) && (l1.isValid[i] == 1))
                        {
                            L1AcceState = 1;
                            L2AcceState = 0;
                            break;
                        }
                    }
                    //Read Miss On L1
                    if (L1AcceState != 1)
                    {
                        L1AcceState = 2;
                    }

                }
                else if (l1.noOfWays == 1)
                {
                    l1CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l1.t, l1.s), nullptr, 2);
                    unsigned long tag;
                    tag = std::stoul(accessaddr.to_string().substr(0, l1.t), nullptr, 2);
                    if ((l1.tag[l1CacheAddrIndex] == tag) && (l1.isValid[l1CacheAddrIndex] == 1))
                    {
                        L1AcceState = 1;
                        L2AcceState = 0;
                    }
                    //Read Miss On L1
                    else
                    {
                        L1AcceState = 2;
                    }
                }
                else if (l1.noOfWays > 1)
                {
                    l1CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l1.t, l1.s), nullptr, 2);
                    unsigned long tag;
                    tag = std::stoul(accessaddr.to_string().substr(0, l1.t), nullptr, 2);
                    for (int i=0; i < l1.noOfWays; i++)
                    {
                        if ((l1.tag[(l1CacheAddrIndex * l1.noOfWays) + i] == tag) && (l1.isValid[(l1CacheAddrIndex * l1.noOfWays) + i] == 1))
                        {
                            L1AcceState = 1;
                            L2AcceState = 0;
                            break;
                        }
                    }
                    //Read Miss On L1
                    if (L1AcceState != 1)
                    {
                        L1AcceState = 2;
                    }
                }
                //Read Miss On L1
                else
                {
                    L1AcceState = 2;
                }

                //Read Hit On L2
                if (L1AcceState == 2)
                {
                    if (l2.noOfWays == 0)
                    {
                        l2CacheAddrIndex = std::stoul(accessaddr.to_string().substr(0, l2.t), nullptr, 2);
                        for (int i = 0; i < l2.noOfSlots; i++)
                        {
                            if ((l2.tag[i] == l2CacheAddrIndex) && (l2.isValid[i] == 1))
                            {
                                L2AcceState = 1;
                                l2.isValid[i] = 0;
                                break;
                            }
                        }
                        //Read Miss On L2
                        if (L2AcceState != 1)
                        {
                            L2AcceState = 2;
                        }
                    }
                    else if (l2.noOfWays == 1)
                    {
                        l2CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l2.t, l2.s), nullptr, 2);
                        unsigned long tag;
                        tag = std::stoul(accessaddr.to_string().substr(0, l2.t), nullptr, 2);
                        if ((l2.tag[l2CacheAddrIndex] == tag) && (l2.isValid[l2CacheAddrIndex] == 1))
                        {
                            L2AcceState = 1;
                            l2.isValid[l2CacheAddrIndex] = 0;
                        }
                        //Read Miss On L2
                        else
                        {
                            L2AcceState = 2;
                        }
                    }
                    else if (l2.noOfWays > 1)
                    {
                        l2CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l2.t, l2.s), nullptr, 2);
                        unsigned long tag;
                        tag = std::stoul(accessaddr.to_string().substr(0, l2.t), nullptr, 2);
                        for (int i = 0; i < l2.noOfWays; i++)
                        {
                            if ((l2.tag[(l2CacheAddrIndex * l2.noOfWays) + i] == tag) &&
                                (l2.isValid[(l2CacheAddrIndex * l2.noOfWays) + i] == 1))
                            {
                                L2AcceState = 1;
                                l2.isValid[(l2CacheAddrIndex * l2.noOfWays) + i] = 0;
                                break;
                            }
                        }
                        //Read Miss On L2
                        if (L2AcceState != 1)
                        {
                            L2AcceState = 2;
                        }
                    }
                    //Read Miss On L2
                    else
                    {
                        L2AcceState = 2;
                    }
                    updateCache(L2AcceState, accessaddr);
                }
            }
            else
            {
                //Implement by you:
                // write access to the L1 Cache,
                //and then L2 (if required),
                //update the L1 and L2 access state variable;

                unsigned long l1CacheAddrIndex;
                unsigned long l2CacheAddrIndex;
                //Write Hit On L1
                if (l1.noOfWays == 0)
                {
                    l1CacheAddrIndex = std::stoul(accessaddr.to_string().substr(0, l1.t), nullptr, 2);
                    for (int i=0 ; i < l1.noOfSlots; i++)
                    {
                        if ((l1.tag[i] == l1CacheAddrIndex) && (l1.isValid[i] == 1))
                        {
                            L1AcceState = 3;
                            L2AcceState = 0;
                            break;
                        }
                    }
                    //Write Miss On L1
                    if (L1AcceState != 3)
                    {
                        L1AcceState = 4;
                    }
                }
                else if (l1.noOfWays == 1)
                {
                    l1CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l1.t, l1.s), nullptr, 2);
                    unsigned long tag;
                    tag = std::stoul(accessaddr.to_string().substr(0,l1.t), nullptr, 2);
                    if (((l1.tag[l1CacheAddrIndex]) == tag) && (l1.isValid[l1CacheAddrIndex] == 1))
                    {
                        L1AcceState = 3;
                        L2AcceState = 0;
                    }
                    //Write Miss On L1
                    else
                    {
                        L1AcceState = 4;
                    }
                }
                else if (l1.noOfWays > 1)
                {
                    l1CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l1.t, l1.s), nullptr, 2);
                    unsigned long tag;
                    tag = std::stoul(accessaddr.to_string().substr(0, l1.t), nullptr, 2);
                    for (int i=0; i < l1.noOfWays; i++)
                    {
                        if ((l1.tag[(l1CacheAddrIndex * l1.noOfWays) + i] == tag) && (l1.isValid[(l1CacheAddrIndex * l1.noOfWays) + i] == 1))
                        {
                            L1AcceState = 3;
                            L2AcceState = 0;
                            break;
                        }
                    }
                    //Write Miss On L1
                    if (L1AcceState != 3)
                    {
                        L1AcceState = 4;
                    }
                }
                else
                {
                    L1AcceState = 4;
                }

                //Write Hit On L2
                if (L1AcceState == 4)
                {
                    if (l2.noOfWays == 0)
                    {
                        l2CacheAddrIndex = std::stoul(accessaddr.to_string().substr(0, l2.t), nullptr, 2);
                        for (int i = 0; i < l2.noOfSlots; i++)
                        {
                            if ((l2.tag[i] == l2CacheAddrIndex) && (l2.isValid[i] == 1))
                            {
                                L2AcceState = 3;
                                break;
                            }
                        }
                        //Write Miss On L2
                        if (L2AcceState != 3)
                        {
                            L2AcceState = 4;
                        }
                    }
                    else if (l2.noOfWays == 1)
                    {
                        l2CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l2.t, l2.s), nullptr, 2);
                        unsigned long tag;
                        tag = std::stoul(accessaddr.to_string().substr(0, l2.t), nullptr, 2);
                        if ((l2.tag[l2CacheAddrIndex]) == tag && (l2.isValid[l2CacheAddrIndex] == 1))
                        {
                            L2AcceState = 3;
                        }
                        else
                        {
                            L2AcceState = 4;
                        }
                    }
                    else if (l2.noOfWays > 1)
                    {
                        l2CacheAddrIndex = std::stoul(accessaddr.to_string().substr(l2.t, l2.s), nullptr, 2);
                        unsigned long tag;
                        tag = std::stoul(accessaddr.to_string().substr(0, l2.t), nullptr, 2);
                        for (int i = 0; i < l2.noOfWays; i++)
                        {
                            if ((l2.tag[(l2CacheAddrIndex * l2.noOfWays) + i] == tag) &&
                                (l2.isValid[(l2CacheAddrIndex * l2.noOfWays) + i] == 1))
                            {
                                L2AcceState = 3;
                                break;
                            }
                        }
                        //Write Miss On L2
                        if (L2AcceState != 3)
                        {
                            L2AcceState = 4;
                        }
                    }
                    //Write Miss On L2
                    else
                    {
                        L2AcceState = 4;
                    }
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
