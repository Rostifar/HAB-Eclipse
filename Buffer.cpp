#include "Buffer.h"
#include <math.h>
#include <string.h>

Buffer::Buffer(const unsigned int size) {
    this.data = (char *) malloc(sizeof(char) * size);
    this.size = size;
    this.len = 0;
}

Buffer::~Buffer() {
    free(data);
}

static int Buffer::memCeil(unsigned float v) {
    return (ceil(v) - v < epsilon) ? ceil(v) : ceil(++v);
}

void Buffer::dynamicCpy(char *newData) {
    for (unsigned int i = 0; i < len; i++) newData[i] = data[i];
    free(data);
    data = newData;
}

void Buffer::handleOverflow(const unsigned int entrySize) {
    const int scalar = entrySize > size ? memCeil((entrySize + len) / size) : 2;
    char *newData = (char *) malloc(sizeof(char) * scalar);
    dynamicCpy(newData);
}

void Buffer::empty() {
    memset(data, 0, sizeof data);
}

void Buffer::addChar(const char c) {
    if (len == size) handleOverflow();
    data[len] = c;
}

void Buffer::addString(const char *str) {
    if (len + sizeof(str) > size) handleOverflow();
    for (unsigned int i = 0; i < strlen(str); i++) {
        data[len] = str[i];
        len++;
    }
}

char *getData() {
    data[len] = '\0'; //null terminate
    ++len;
    return data;
}