#ifndef TEST_HELPER
#define TEST_HELPER

static inline bool match_string(const char *data, const char *test, uint16_t length){
    for(auto i = 0; i<length; i++){
        if(data[i] != test[i]) return false;
        if(data[i] == 0) break;
    }
    return true;
}


static inline bool match_array(const uint8_t *data, const uint8_t *test, uint16_t length){
    for(auto i = 0; i<length; i++){
        if(data[i] != test[i]) return false;
    }
    return true;
}


#endif