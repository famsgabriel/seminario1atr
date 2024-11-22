#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this, i]() { // Vetor de threads para manter o controle e identificar elas
                while (true) {
                    std::function<void()> task;
                    int taskId = -1; 
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this]() {
                            return this->stop || !this->tasks.empty();
                        });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(this->tasks.front().first);
                        taskId = this->tasks.front().second;
                        this->tasks.pop();
                    }
                    std::cout << "Thread " << i << " está executando a tarefa " << taskId << "." << std::endl;
                    task(); // Chama a execução de uma task
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers) {
            worker.join();
        }
    }

    void enqueueTask(std::function<void()> task, int taskId) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(std::move(task), taskId);
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::pair<std::function<void()>, int>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

int main() {
    // Cria a thread pool passando como parâmetro a quantidade de threads
    ThreadPool pool(3);

    for (int i = 1; i <= 6; ++i) {
        pool.enqueueTask([i]() {
            std::this_thread::sleep_for(std::chrono::seconds(7)); // Simula a chamada de alguma task, adição à lista
            std::cout << "Task " << i << " completa." << std::endl; //Print avisando que a task está finalizada. 
        }, i);
    }
    std::this_thread::sleep_for(std::chrono::seconds(7));

    return 0;
}
