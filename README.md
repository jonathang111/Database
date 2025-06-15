# Smart Database-System
This is still being worked on; I just moved my current progress to a private repo.

Modular, packet-driven data operating system with natural language interface and AI-assisted query orchestration.

The ultimate goal is the true mode of abstraction. Natural language. This system's end goal is to allow natural language queries and processing. Imagine having a set of data that is retrieved, processed, and presented to a user just as if it asked an expert to do it. Highly specialized modules are in charge of holding, managing and processing large swaths of data. 

This project is no longer just about files, though some old text may imply that. This is about data in general, inputs, ouputs, and writes/reads must be interfacable. Data is our main parsing item, not just a file.

To prevent errors with potential miscalculations in the LLMs query suggestion, it should still be human-definable. Meaning that if an AI assumes things, it must return these assumptions to the user. The user should always know what was searched for, how it was processed, and why it was done that way. This emphasizes the importance of the packets being trackable and easily readable by humans.

Current TO DOS:
1. Add constexpr in ReadKeyValues
2. File manager class encompassing all other files 
3. Handle extensions with multiple '.'
4. QT framework
5. fix RAII in db construct.
6. Ensure all values are freed when needed.
7. Create quick debugging tools since we're heading into the more abstract stage
8. Introduce padding to reg cache write
9. separate functions for writing header and entries.
10. Need to refactor DBConstruction, it is WAYYYYY too dependent structs.
	-make it more abstract and modular like cacherw
11. Finish CacheRW input cases
12. implement unique/shared pointers instead of current implementation for safety
13. fix unit_8 for certain filename keys like % in lexical sort
14. Convert all "struct specific functions" to more "feature specific functions"
	Meaning that functions should be defined on an object potential behavior rather than focusing on the specific structure
	e.g. check for iterability, or pair rather than is_vector or is_map, etc.
15. Cache file should be accessible in chunks rather than just the whole file (better for the future (multithreading!!!))
16. priotiy assignment on packet requests meta data from main manager
17. IOHandler should handle multiple destination splitting (keep packet as one logical unit)
18. Critical failures must have roll back, non crit can be log + continue.
		Should each packet carry a crit flag or some transactional flag
19. Vaildity module needs to check each sub manager after any change, or should validity check only when prompted by submanager
20. make tracing manager for debugging!!!!! possibly write out and note path as well as things like packet, wait time, etc.
21. given module failure, packets/processes must wait elsewhere (if critical, even validity module needs to wait for access) or be dropped elsewhere.
	Maybe database is persistent, thus validity mod can still edit changes there?
22. Potential health manager to moniter to state of a module to ensure function
23. modules must run on their own process and main manager must communicate via local sockets or shared memory
	espeically so if we expect sub modules to have their own queues
24. auto scaling threading so high use modules get more threads
25. stress test / fault injection module, lets me test the system.
26. Define modules to be persistent or not persistent.

Finished TO DOS:
- Basic cache key prototype
- Key sorting
- RAII wrapper, custom made.
- Cache refactor, use SFINAE (type trait) rather than hard coded acceptable structs.
- Create key sorts for multiple sorting types, then upload to cache.
- DBConstruct changed to namespace
- Cache write and read functions.
- symlink issues, fixed using lstat() instead
- memleak fixed!
- database creation
- Need to rework Entries. You can't reload pointers from cache, they will be undefined pointers.
- deep copy added

Notes:

Cache Layout:
CacheHeader:
    char tag[4]; 			//Cache tag, signs the tag
    int keyAmount;			//The number of keys in the bin
    int entryAmount;		//The number of entries in the document
    float version;			//The version of the bin
    int date;				//Date of last update
    int footerStart;        //Offset to footer containing keys
Entries:
    //Entries start at sizeof(header)
    //these entries will be stored in conotinous blocks,
    // so keeping track of offset and size while reading is vital

[padding/gap]	// going to remove this, the rewrites are more worth it that the potential wasted space.

KeyIndexEntry[0]:
    char key[MAX_KEYSIZE]; 	//Key name, max size currently is 32, likely to be expanded later
    int keySize;			//TEMP
    int offset;				//Location of the keys entries
    int count;				//number of entries it holds
KeyIndexEntry[1]:
	key[MAX_KEYSIZE]:  
    offset:         
    count:              
...

Idea being that you can dynamically add new keys if need and also dynamically store the entries in the middle with possible rewrite of footing (this should be rare). This method is better since a linked list logic would require linear search every time you want to locate some key.

Cache Block:
- CURRENTLY WORKING ON THIS

For now all filters(sub module databases) should have their own cache. In order to link them however, such as wanting to find pdf in /Desktop, i can create an intersect/module function that will compare two different databases

CacheRW inteface StorageInterface. The idea is that you are able to inerface it with the defined functions to override them. Then cacheRW utilizes the class defintions to write/read/ or whatever. This decouples from the previous version where it was tied to fstream. Now in memory writing is allowed, network socket storage, encryption, and cmopressed storage is allowed. They all require different methods for writing and such so they wouldn't work with fstream.
	Be sure to leverage the vtable, without it CacheRW wouldn't be able to access the functions without some sort of downcasting!!!

The manager class should manage the CacheDBs by deleting older unused ones. For example, when a user calls to see files by extension, then an array should be made holding entries pertaining to that. Then if they never access the array again after some time, just delete. Or if the max is hit, then delete by LRU.

Another problem is when the WSL needs to access mnt/whatever; at this point it needs to go through multiple virtual layers leading to higher latency when using lstat or opendir. Just for reference, on comparable hardware it takes a WSL ~800 secs to scan ~600k files and the mac 85 sceonds to do ~1mil files.

Not sure about the memcpy situation, it technically works but it goes against RAII. So maybe rework that.


One idea to improve the speed of database scanning on WSLs is to use multithreading. 
Instead of having the database recursively open directories and scan, I could instead place it into a thread-safe queue and let other cores scan the file independently. Would need mutex to gaurd the DB since it would be a shared space.

Map will compare addresses if you key by char*; use string instead to compare the actual data.

The program must be capable of failure. it should be "hot swappable", meaning you should be able to replace one module with another and expect there to be a continous stream of the program and other modules. i.e. if i need to upgrade something i can just stop the program, replace the socket fd, then have the program running again like normal. all without disrupting other modules and allowing for critical parts to be queued.

Smart query system:
A smart dynamic pipline for queries. Main manager creates a packet such as:
{
"target": "folder A",
"file_filter": "*.json",
"next_step": "searchForKey:userID"
}

Packet mutates its own state:
- "target": "folder A"
+ "target": "folder A"
+ "found_files": ["file1.json", "file2.json"]

now packet sees that "searchForKey:userID" is the next instruction, so it adds a new step to extract the value of the key:

{
  "found_files": ["file1.json", "file2.json"],
  "next_step": "extractKeyValue:userID",
  "results": []
}

After getting those values, it mutates again to now summarize of forward results:

{
  "results": [121, 122, 123],
  "next_step": "returnToCaller"
}


Smart manager system:
Overview:
The Smart Manager System is a modular backend data engine that functions as an intelligent controller for managing and querying a dynamic filesystem-based dataset. It uses a centralized MainManager that delegates responsibilities to specialized sub-managers, each responsible for their own domain of data—such as file indexing, symbol extraction, tagging, searching, and temporary cache management. The system supports multithreaded operations for high performance and efficient parallel data processing.

<!-- Main Manager:
Acts as the sole orchestrator and gatekeeper of all commands. No sub-manager executes independently—all operations are triggered or coordinated through MainManager.

- change of plans; instead of main manager being resposible for every orchestration, it's now in charge of maintenance as well as packet encoding. Packets are to be the main method of communcation to modules. Orders are sent in encoded packets to a packet manager. The packet manager should be in charge or parsing the packages, handling errors (or maybe just the smart query can handle errors) as well as tracking the life of the packet. 

Responsibilities
	- Delegates all queries, refreshes, and index-building commands.
	- Ensures execution order, thread safety, and data validity.
	- Triggers sub-managers asynchronously (or in parallel) when appropriate.
	- Maintains global view of system state but holds no direct data itself.

Core Methods
	- InitializeSubManagers()
	- DispatchQuery(const Query&)
	- RefreshAll()
	- ValidateSystemState()
	- ParallelTick() — concurrently triggers refresh/maintenance across all sub-managers.

Design Philosophy: “Sub-managers are workers. MainManager is the brain.”
This enforces clean hierarchy and prevents unauthorized or rogue updates as well as handles multi-threading. -->
Switch Board Protocol System:
This shouldn't be a module. Instead a better approach is switch board protocol system. The switchboard just handles the packets being sent from one module to another. It also ensures compatability through checking a modules accepted schema, or in case to reduce overhead, if there is a verified and approved path between two approved modules it can skip verification. This will also make it easier to debug and is an in between instead of having to choose on central module that can inhbit fault-tolerance, performance, and hot-swapability, instead the switch board is a more non-logic interfering device. Things like a packets lifetime can still be interfaced and things like checking a proper replaced can still work by examning current paths and compatabilities. When a module is registered into the switch board, it should come with tags and descriptions of capabilities. Such as the schema it takes, attributes about it, if it can override the verfication, and some other stuff. 

So a packets life goes like this: Packet enters system through some entry point, it is routed to a module, module returns to switchboard, routed again if needed and everything is verified to be compatable (unless two modules are 100% compatable, then we can just override to reduce latency). Switch board can log everything if debugging, potentially just output to some stream where another module actually does the logging and conncection/logic handling of logging. We can just encode the logic into the routing action, so at end when a packet is sent to i/o manager, it sends some end signal so switchboard knows the packet is done and can respond or send a signal to some packet manager that keeps track of a packet lifetime.

It can also be expanded to include other stuff like an execution trace, audit levrel level enum, fallback routing in case a module is missing, time outs to prevent infinite routes in a loop, and maybe even more stuff. But the main idea is that this provides a way to create modules, keep track of them, verify compatability, and allow for interaction without being tied to one process. This makes the system more fault tolerant and reliable, but most importantly its hot swapablle. The switchboard is now the main fault point, but it doesn't really do any hard coded logic, thus it makes the system more resistant to error.

Communication:
Communication between modules is done through sockets. Sockets allow for hot swappable modules as well as a more defined method for sending packets of data instead of a single command. It will also allow processes to be ran on seperate hardware.

The schema for a packet has not yet been defined, but when it is it will likely be a dynamic one that can be customized in order to fit new modules if added. It will likely include two types of packets, "smart" and "dumb" packets. A dumb packet follows a typical query, a simple location, fetch, and return. This packet type is error prone and used for very specific searches. If an error in the module occurs, then nothing is done and the error is reported. Smart packets as described above can define their own "next steps", they can handle errors, return similar results, and most importantly interact with a dynamic query such as those seen in a llm. 

Sub Managers:
DBManager:  	            Filesystem abstraction and metadata persistence (MainDB)
<?> Vailidity Manager:			Ensures updated and correct database information
SymbolIndexer:	            Extracts and caches functions/classes
TagManager:     	        Stores user-defined semantic labels
SearchCacheManager:	        Holds recent query results
IntersectManager:	        Handles set logic on filtered datasets
MetaDB:         	        Validates and compares against live data
(Planned) HistoryManager:	Logs and replays commands
(Planned) StatManager:	    Analyzes and stores codebase metrics

Each of these implements a consistent sub-database interface, allowing MainManager to control them uniformly.

Health Hash Packets:
A packet is parsed into pieces. Each piece is a known hash and is sent to a specific module. If an operation of a module is in a healthy state, the hash returns as expected, if not the parsed part of the hash is missing. Since modules all communicate with packets, it's expected that a packet can be sent to check each component. For example, one hash can be passed into the I/O module, if all is well, then I/O could send and simulate a recieve, then return the packet to the validity module. Similar things can be done such as in searching for a specific query.

This will also verify whether a new module is compatable with the current system.

Smart Cache:
Efficiently reuse bin space, reduce fragmentation, and avoid unnecessary rewrites by using key-index chaining and validity metadata.
    Idea:
	- When updating or writing a key:
	- The system scans all KeyIndexEntry records for that key.
	- If an invalid entry is found that has enough space, it is reused.
	- Otherwise, the system appends a new entry to the end of the .bin.
	- All validity is cross-checked against MetaDB (e.g., by hash, size, or modified time).

    Validation Triggers:
	- Manually triggered
	- Periodically (via MainManager::ValidateCaches(), some auto trigger)
	- On key update
	- At startup if stale cache is suspected

Mult-threading:
Each sub-manager can be safely run on its own thread (or thread pool) because their domains are disjoint.
    Main Manager Roles:
    - Spawns threads during ParallelTick()
	- Joins threads before returning control
	- Manages mutexes or synchronization wrappers for shared data writes
The cache should be in chunks too so that we can access each chunk with an atomic lock.