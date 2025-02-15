Map-Reduce Protocol

# Main

    In the main function, the input data is read from the arguments. The number of mapper threads, the number of reducer threads, and the input file are determined. Then, a number of files are read from this input file. These files are stored in a priority queue, where the priority is determined by the size of each file. Next, we initialize the elements used for synchronization (two mutexes and two barriers). The threads are initialized and started. Finally, they are joined with the main thread.

# Structura de Payload

    The Payload structure is the parameter passed to the thread function. It contains all the details needed by both the mapper and reducer threads. First, it contains pointers to the two barriers and mutexes. It also includes the number of threads in each category, the thread’s ID, and its type (0 for mapper, 1 for reducer). Additionally, it holds a pointer to a vector of vectors that keeps track of word-file pairs found in each file, as well as a pointer to the priority queue containing the files. Furthermore, for reducer threads, there is a pointer to a shared map that contains all the words from all files along with the indexes of the files where they appear. Lastly, for each thread, there is a vector of vectors (used only by reducers, even though mappers also have this field, they do not use it). This structure is allocated a number of letters and contains words that start with those letters, along with the files where they appear.

# Thread_Function - Mapper

    The mapping process is carried out using the priority queue from the main function. Each thread takes a file as long as the queue is not empty. This is a critical section since we do not want multiple threads to take the same file simultaneously. Therefore, this section is enclosed with a mutex. The thread reads all the words from the newly extracted file, following the given constraints. Each word, along with the file ID where it was found, is inserted into a set to prevent duplicates if the word appears multiple times in the same file. After this, the set is converted into a vector and added to the shared vector of vectors for all threads. Finally, the threads reach a barrier and stop.

# Thread_Function - Reducer

    The reduce phase begins at the same barrier where the mapper threads stop. This barrier ensures that the reducer threads do not start execution until all mapper threads have finished processing the files. Each thread is assigned a number of vectors from the shared vector of vectors, and it starts populating the shared map. Each encountered word is inserted into the map along with all the files where it appears. Each thread uses a local map, which is later transferred into the shared map. At this point, all threads encounter another barrier, ensuring that the next step does not begin until the shared map is fully populated. Finally, each reducer thread is assigned a number of letters. Each thread is responsible for creating a representative file for each assigned letter and populating it with all words from the shared map. The words are sorted first by frequency of occurrence, and if two words have the same frequency, they are sorted lexicographically. In the end, all words are placed in the created files in the determined order, along with the indexes of the files in which they appear.
