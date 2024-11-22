#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

class ObjetoMonitor {
private:
    std::queue<int> buffer;
    const unsigned int capacidade = 5;
    std::mutex mtx;
    std::condition_variable condProducao;
    std::condition_variable condConsumo; 

public:
    void produzir(int item) {
        std::unique_lock<std::mutex> lock(mtx);

        condProducao.wait(lock, [this]() { return buffer.size() < capacidade; });

        buffer.push(item);
        std::cout << "Produzido: " << item << "\n";

        condConsumo.notify_one();
    }


    int consumir() {
        std::unique_lock<std::mutex> lock(mtx);

        condConsumo.wait(lock, [this]() { return !buffer.empty(); });

        int item = buffer.front();
        buffer.pop();
        std::cout << "Consumido: " << item << "\n";

        condProducao.notify_one();

        return item;
    }
};

void produtor(ObjetoMonitor& monitor) {
    for (int i = 1; i <= 10; ++i) {
        monitor.produzir(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumidor(ObjetoMonitor& monitor) {
    for (int i = 1; i <= 10; ++i) {
        monitor.consumir();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}
int main() {
    ObjetoMonitor monitor;

    std::thread tProdutor(produtor, std::ref(monitor));
    std::thread tConsumidor(consumidor, std::ref(monitor));

    tProdutor.join();
    tConsumidor.join();

    return 0;
}
