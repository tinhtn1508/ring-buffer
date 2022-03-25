#include<iostream>
#include<vector>
#include <atomic>

template<typename Request>
struct RingBuffer {
    std::vector<Request> ring;
    int bufferSize;
    std::atomic<int> readPosition;
    std::atomic<int> writePosition;

    RingBuffer(size_t size)
    {
        ring.resize(size);
        bufferSize = size;
        readPosition = 0;
        writePosition = 0;
    }

    void push(const Request & request)
    {
        for (;;) {
            unsigned fullReadPosition = (writePosition + 1) % bufferSize;
            if (readPosition == fullReadPosition)
                readPosition.wait(fullReadPosition);
            else break;
        }
        ring[writePosition] = request;
        writePosition = (writePosition + 1) % bufferSize;
        writePosition.notify_one();
    }

    Request pop() {
        Request result;

        for (;;) {
            if (writePosition == readPosition)
                writePosition.wait(readPosition);
            else break;
        }

        result = ring[readPosition];
        ring[readPosition] = Request();
        readPosition = (readPosition + 1) % bufferSize;
        readPosition.notify_one();
        return result;
    }
};

int main() {
    auto queue = new RingBuffer<int>(100);
    queue->push(1);

    std::cout << queue->pop() << std::endl;
}