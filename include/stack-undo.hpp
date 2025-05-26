#pragma once
#include <memory>

class Order;

struct OrderIntPair {
    std::shared_ptr<Order> first;
    int second;

    OrderIntPair() = default;

    OrderIntPair(std::shared_ptr<Order>& o, int i)
        : first(o), second(i) {}
};


class StackUndo {
private:
    OrderIntPair* data;
    size_t capacity;
    size_t length;

    void resize(size_t new_capacity) {
        auto* new_data = new OrderIntPair[new_capacity];
        for (size_t i = 0; i < length; ++i) {
            new_data[i] = data[i];
        }
        delete[] data;
        data = new_data;
        capacity = new_capacity;
    }
public:
    StackUndo() : data(nullptr), capacity(0), length(0) {}

    ~StackUndo() {
        delete[] data;
    }

    void push(std::shared_ptr<Order>& order, int i) {
        if (length >= capacity) {
            resize(capacity == 0 ? 1 : capacity * 2); // kalo gamuat double sizenya aj
        }
        data[length++] = OrderIntPair(order, i);
    }

    void pop() {
        if (length > 0) {
            length--;
        }
    }

    OrderIntPair& top() {
        return data[length - 1];
    }

    bool empty() {
        return length == 0;
    }
};


