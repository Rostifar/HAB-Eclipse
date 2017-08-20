#ifndef BUFFER_H_
#define BUFFER_H_

class Buffer {
    
    public:
        Buffer(const unsigned int size);
        ~Buffer();
        void empty();
        void addChar(const char c);
        void addString(const char *str);
        char *getData();
    
    private:
        const unsigned float epsilon = 0.5f;
        char *data;
        unsigned int size, len;
        
        static int memCeil(unsigned float v);
        void dynamicCpy(char *newData);
        void handleOverflow(const unsigned int entrySize);
}

#endif