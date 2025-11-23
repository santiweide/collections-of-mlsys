#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

template <typename T>
class RoundRobin {
public:

RoundRobin(const std::vector<T>& items) 
        : m_items(items), m_index(0)
    {
        if (m_items.empty()) {
            throw std::invalid_argument("列表不能为空 (Items list cannot be empty)");
        }
    }

    const T& get_next() {
        const T& item = m_items[m_index];

        m_index = (m_index + 1) % m_items.size();

        return item;
    }

private:
    std::vector<T> m_items; 
    size_t m_index; 

int main() {
    std::vector<std::string> servers = {"192.168.1.100", "192.168.1.101", "192.168.1.102"};

    RoundRobin<std::string> rr(servers);

    for (int i = 0; i < 10; ++i) {
        std::cout << "第 " << (i + 1) << " 次请求, 分配到: " << rr.get_next() << std::endl;
    }

    return 0;
}