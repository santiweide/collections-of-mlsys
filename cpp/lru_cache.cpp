// lru map, for rsa encrypt cache
template <class K, class T>
struct Node{
    K key;
    T data;
    Node *prev, *next;
};

template <class K, class T>
class LRUCache{
public:
    LRUCache(size_t size){
        _entries = new Node<K, T>[size];
        for (uint i = 0; i < size; ++i){
            _free_entries.push_back(_entries + i);
        }
        _head = new Node<K, T>;
        _tail = new Node<K, T>;
        _head->prev = NULL;
        _head->next = _tail;
        _tail->prev = _head;
        _tail->next = NULL;
    }
    ~LRUCache(){
        delete _head;
        delete _tail;
        delete[] _entries;
    }
    void put(K key, T data) {
        Node<K, T> *node = _hashmap[key];
        if (node) { // node exists
            detach(node);
            node->data = data;
            attach(node);
        } else {
            if (_free_entries.empty()){ // cache full
                node = _tail->prev;
                detach(node);
                _hashmap.erase(node->key);
            } else {
                node = _free_entries.back();
                _free_entries.pop_back();
            }
            node->key = key;
            node->data = data;
            _hashmap[key] = node;
            attach(node);
        }
    }
    T get(K key) {
        Node<K, T> *node = _hashmap[key];
        if (node) {
            detach(node);
            attach(node);
            return node->data;
        } else {// hit failed, return T default value
            return T();
        }
    }
private:
    // delete node
    void detach(Node<K, T>* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    // insert into head
    void attach(Node<K, T>* node) {
        node->prev = _head;
        node->next = _head->next;
        _head->next = node;
        node->next->prev = node;
    }
private:
    __gnu_cxx::hash_map<K, Node<K, T>* > _hashmap;
    std::vector<Node<K, T>* > _free_entries; // for free node addr 
    Node<K, T> *_head, *_tail;
    Node<K, T> *_entries; // double linked list node
};