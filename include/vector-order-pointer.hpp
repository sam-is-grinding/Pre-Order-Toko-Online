#pragma once
#include <memory>

class Order;

class VectorOrderPointer {
private:
    std::shared_ptr<Order>* data;
    size_t capacity;
    size_t length;

    void resize(size_t new_capacity) {
        auto* new_data = new std::shared_ptr<Order>[new_capacity];
        for (size_t i = 0; i < length; ++i) {
            new_data[i] = data[i];
        }
        delete[] data;
        data = new_data;
        capacity = new_capacity;
    }
public:
    VectorOrderPointer() : data(nullptr), capacity(0), length(0) {}

    ~VectorOrderPointer() {
        delete[] data;
    }

    void push_back(std::shared_ptr<Order>& order) {
        if (length >= capacity) {
            resize(capacity == 0 ? 1 : capacity * 2);
        }
        data[length++] = order;
    }

    std::shared_ptr<Order>& operator[](size_t index) {
        return data[index];
    }

    size_t size() {
        return length;
    }

    std::shared_ptr<Order>* begin() {
        return data;
    }

    std::shared_ptr<Order>* end() {
        return data + length;
    }
};