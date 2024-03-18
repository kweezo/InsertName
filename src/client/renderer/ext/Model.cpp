

#include <iostream>
#include <vector>

#define Model uint32_t

class ModelManager{
public:
    static Model LoadModel(std::vector<float> vertices, std::vector<uint32_t> indices);
    static Model LoadModel(const char* path);
private:

};