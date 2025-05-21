# File-System
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

Finished TO DOS:
- Create key sorts for multiple sorting types, then upload to cache.
- DBConstruct changed to namespace
- Cache write and read functions.
- symlink issues, fixed using lstat() instead
- memleak
- database creation
- Need to rework Entries. You can't reload pointers from cache, they will be undefined pointers.
	- deep copy added, memory situation still unknown

Notes:

Cache Layout:
CacheHeader:
    tag: "CBFE"         // Cache tag, use to verify correct cache type/filter
    key amount:         // the amount of keys in the cache, also how many key indices
	offset to keyindx:	// Absolute offset to the footer/key index start
    entry amount:       // the amount of entries in the cache
    version:            // just the version of the cache; used in validity manager?
    date:               // date of cache creation

Entries:
    //Entries start at sizeof(header)
    //these entries will be stored in conotinous blocks,
    // so keeping track of offset and size while reading is vital

[padding/gap]	//should have enough room between entries and keyindex's so rewrites aren't common

KeyIndexEntry[0]:
    key[MAX_KEYSIZE]:   // key name, maybe varies on cache or standardized
    offset:             // offset in cache of key values
    count:              // number of values in the key
KeyIndexEntry[1]:
	key[MAX_KEYSIZE]:  
    offset:         
    count:              
...

Idea being that you can dynamically add new keys if need and also dynamically store the entries in the middle with possible rewrite of footing (this should be rare). This method is better since a linked list logic would require linear search every time you want to locate some key.

For now all filters should have their own cache. In order to link them however, such as wanting to find pdf in /Desktop, i can create a intersect function that will compare two different databases

CacheRW inteface StorageInterface. The idea is that you are able to inerface it with the defined functions to override them. Then cacheRW utilizes the class defintions to write/read/ or whatever. This decouples from the previous version where it was tied to fstream. Now in memory writing is allowed, network socket storage, encryption, and cmopressed storage is allowed. They all require different methods for writing and such so they wouldn't work with fstream.
	Be sure to leverage the vtable, without it CacheRW wouldn't be able to access the functions without some sort of downcasting!!!

The manager class should manage the CacheDBs by deleting older unused ones. For example, when a user calls to see files by extension, then an array should be made holding entries pertaining to that. Then if they never access the array again after some time, just delete. Or if the max is hit, then delete by LRU.

Another problem is when the WSL needs to access mnt/whatever; at this point it needs to go through multiple virtual layers leading to higher latency when using lstat or opendir. Just for reference, on comparable hardware it takes a WSL ~800 secs to scan ~600k files and the mac 85 sceonds to do ~1mil files.

Not sure about the memcpy situation, it technically works but it goes against RAII. So maybe rework that.


One idea to improve the speed of database scanning on WSLs is to use multithreading. 
Instead of having the database recursively open directories and scan, I could instead place it into a thread-safe queue and let other cores scan the file independently. Would need mutex to gaurd the DB since it would be a shared space.

Map will compare addresses if you key by char*; use string instead to compare the actual data.

Smart manager system:
Overview:
The Smart Manager System is a modular backend data engine that functions as an intelligent controller for managing and querying a dynamic filesystem-based dataset. It uses a centralized MainManager that delegates responsibilities to specialized sub-managers, each responsible for their own domain of data—such as file indexing, symbol extraction, tagging, searching, and temporary cache management. The system supports multithreaded operations for high performance and efficient parallel data processing.

Main Manager:
Acts as the sole orchestrator and gatekeeper of all commands. No sub-manager executes independently—all operations are triggered or coordinated through MainManager.

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
This enforces clean hierarchy and prevents unauthorized or rogue updates as well as handles multi-threading.

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