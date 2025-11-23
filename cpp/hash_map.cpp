#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Node{
    int key;
    int val;
    Node* next;
    Node(int x, int y): key(x), val(y), next(nullptr) {};
    Node(): key(-1), val(-1), next(nullptr) {};
};

struct List {
    Node* head;
    Node* tail;
    List() {
        head = new Node();
        tail = head;
    }    
    ~List() {
        Node * p = head;
        while (p) {
            Node* q = p;
            p = p -> next;
            delete q;
        }
    }
    void add(int key, int val) {
        if (!head) {
            std::cout << "head is nullptr" << std::endl;
        }
        Node* q = head -> next;
        while (q) {
            if (q->key == key) {
                // std::cout << q->val << "->" << val;
                q->val = val;
                return;
            }
            q = q -> next;
        }
        Node* p = new Node(key, val);
        tail -> next = p;
        tail = p;
    
        return;
    }
    int get(int key) {
        if (!head) {
            std::cout << "head is nullptr" << std::endl;
        }
        Node* q = head -> next;
        while (q) {
            if (q->key == key) {
                return q->val;
            }
            q = q->next;
        }
        
        return -1;
    }
};


struct MyHashMap {
    std::vector<List> entries;
    int capacity;

    MyHashMap(int capacity) {
        this->capacity = capacity;
        entries.resize(capacity);
    }
    void put(int key, int val) {
        int k = key % capacity;
        entries[k].add(key, val);
        return;
    }

    int get(int key) {
        int k = key % capacity;
        return entries[k].get(key);
    }
    // resize and rehash
    // TODO resize, if allocate fail return false
    // bool resize() {}
};

int main(void) {
    MyHashMap _map(2);
    _map.put(0,2);
    _map.put(0,1);
    _map.put(1,3);
    _map.put(2,10);
    _map.put(3,11);
    _map.put(3,12);
    _map.put(4,15);
    std::cout << _map.get(1) << std::endl;
    std::cout << _map.get(2) << std::endl;
    std::cout << _map.get(3) << std::endl;
    std::cout << _map.get(4) << std::endl;

    return 0;
}