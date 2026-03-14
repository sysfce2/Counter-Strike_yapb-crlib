#include <iostream>
#include <crlib/hashmap.h>

struct CollidingHash {
    uint32_t operator()(int32_t) const noexcept {
        return 0;
    }
};

int main() {
    cr::HashMap<int32_t, int32_t, CollidingHash> m;
    
    std::cout << "Inserting 1,2,3,4..." << std::endl;
    m[1] = 10;
    m[2] = 20;
    m[3] = 30;
    m[4] = 40;
    
    std::cout << "Length: " << m.length() << std::endl;
    std::cout << "Capacity: " << m.capacity() << std::endl;
    
    std::cout << "Checking exists..." << std::endl;
    std::cout << "exists(1): " << m.exists(1) << std::endl;
    std::cout << "exists(2): " << m.exists(2) << std::endl;
    std::cout << "exists(3): " << m.exists(3) << std::endl;
    std::cout << "exists(4): " << m.exists(4) << std::endl;
    
    std::cout << "\nErasing 2..." << std::endl;
    size_t erased = m.erase(2);
    std::cout << "Erased: " << erased << std::endl;
    std::cout << "Length after erase: " << m.length() << std::endl;
    
    std::cout << "\nChecking exists after erase..." << std::endl;
    std::cout << "exists(1): " << m.exists(1) << std::endl;
    std::cout << "exists(2): " << m.exists(2) << std::endl;
    std::cout << "exists(3): " << m.exists(3) << std::endl;
    std::cout << "exists(4): " << m.exists(4) << std::endl;
    
    std::cout << "\nValues after erase:" << std::endl;
    std::cout << "m[1]: " << m[1] << std::endl;
    std::cout << "m[3]: " << m[3] << std::endl;
    std::cout << "m[4]: " << m[4] << std::endl;
    
    return 0;
}