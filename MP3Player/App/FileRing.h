#define FILE_NAME_SIZE  13      //8.3 filename format
#define FILE_NUMBER_MAX 20      //max number of file names to keep in the buffer

INT8U InitRing(void);
void RingLoadFiles(void);
char* RingCurrentFile(void);
char* RingNextFile(void);
char* RingPrevFile(void);
uint8_t RingGetBufferPointer(void);
uint8_t RingGetFileCount(void);
void RingRewindToStart(void);
bool RingIsEnd();