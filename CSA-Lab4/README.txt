CacheSimulator

To execute cachesimulator program : 

Go to CSA-Lab4 folder

Execute following commands in terminal in following sequence: 

1. Generate executable using : 

   make cachesimulator

2. It will generate cachesimulator.out executable.

3. Then, to execute the program please use and place input files in same folder as executable and code or give absolute path: 

	./cachesimulator.out <InputFile_1> <InputFile_2> 

	For eg : 
	InputFile_1 : cacheconfig.txt
	InputFile_2 : trace.txt

Output : 
Output will be available in trace.txt.out in same folder 

Maintenance
Saili Kulkarni   :  snk9486@nyu.edu


Algorithm used : 

For write policy : We have used write back and write no allocate for both L1 and L2 which we have considered as non inclusive caches as given. Hence, if there is write miss in L1, if the data is present in L2, we write there else we send it directly to memory.

For read policy : 
If there is a read miss in L1 and L2, data is brought into only L1 (Non inclusive). If there is a read miss in L1 but hit in L2, we bring in data into L1 and remove it from L2. Also, while bringing in data into L1, if a block needs to be evicted, we insert or update in L2 irrespective of whether it is dirty or not.