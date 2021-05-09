#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <semaphore.h>
#include <time.h>

#include "Logs.h"
#include "File.h"

class DBFile : public File
{
    private:
        sem_t __semaphore_lock;

    public:
        DBFile(int num_of_row, int num_of_col);
        ~DBFile();
        bool acquire_lock(long sec, long nsec);
        void release_lock();
        Log_t* read(Log_t *operation);
        Log_t* write(Log_t *operation);
};

#endif
