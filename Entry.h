#ifndef ENTRY_H_
#define ENTRY_H_

class Entry {
    public:
        char *parse();
        void addTemp(const float c);
        void addLight(const float light);
        void addGpsSentence(const char *str);

    private:
        float light, temp;
        char *gpsSentence;
        const char temp_unit = 'C';
        const char *light_unit = "lux";
}