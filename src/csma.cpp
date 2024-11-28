#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <mutex>

using namespace std;
using namespace std::chrono;

class Node {
private:
    bool m_isSending;
    steady_clock::time_point m_sendTime;
    double m_latency;
    int m_successfulTransmissions;
    size_t m_packetSize; // 1 KB packet size

public:
    Node() : m_isSending(false), m_latency(0), m_successfulTransmissions(0), m_packetSize(1024) {}

    bool IsSending() const { return m_isSending; }
    void SetSendingState(bool state) { m_isSending = state; }

    void RecordSendTime() {
        m_sendTime = steady_clock::now();
    }

    void CalculateLatency() {
        auto duration = duration_cast<milliseconds>(steady_clock::now() - m_sendTime);
        m_latency = duration.count();
    }

    void IncrementSuccess() {
        m_successfulTransmissions++;
    }

    double GetLatency() const { return m_latency; }
    int GetSuccessfulTransmissions() const { return m_successfulTransmissions; }
    size_t GetPacketSize() const { return m_packetSize; }
};

class CsmaDevice {
private:
    Node* m_node;
    bool m_isChannelIdle;
    std::mutex* m_channelMutex;  // Mutex to manage channel access

public:
    CsmaDevice(Node* node, std::mutex* channelMutex) 
        : m_node(node), m_isChannelIdle(true), m_channelMutex(channelMutex) {}

    void Transmit() {
        std::unique_lock<std::mutex> lock(*m_channelMutex, std::defer_lock);
        if (lock.try_lock()) {  // Try to lock the channel
            m_node->SetSendingState(true);
            m_node->RecordSendTime();
            m_isChannelIdle = false;
            // Simulate transmission time (e.g., 10ms to transmit a 1KB packet)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Transmission time
            cout << "Node transmitting packet of size " << m_node->GetPacketSize() << " bytes." << endl;
        } else {
            Backoff();
        }
    }

    void Receive() {
        if (m_node->IsSending()) {
            m_node->SetSendingState(false);
            m_isChannelIdle = true;
            m_node->IncrementSuccess();
            m_node->CalculateLatency();
            cout << "Data received successfully!" << endl;
            m_channelMutex->unlock();  // Unlock the channel after transmission
        }
    }

    void Backoff() {
        int backoffTime = rand() % 5 + 1;  // Random backoff between 1 and 5 ms
        cout << "Channel busy. Backing off for " << backoffTime << " ms." << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(backoffTime));
        Transmit();  // Retry transmission
    }
};

class WiFiNetwork {
private:
    vector<Node> m_nodes;
    double m_totalDataSent;
    double m_simulationTime;
    double m_totalLatency;

public:
    WiFiNetwork() : m_totalDataSent(0), m_simulationTime(0), m_totalLatency(0) {}

    void AddNode(Node node) {
        m_nodes.push_back(node);
    }

    void Simulate() {
        auto start = steady_clock::now();
        vector<thread> nodeThreads;
        std::mutex channelMutex;  // Mutex to control access to the shared channel

        for (auto& node : m_nodes) {
            nodeThreads.push_back(thread([this, &node, &channelMutex]() {
                CsmaDevice device(&node, &channelMutex);
                int transmissionsForNode = 0;

                // Simulate multiple transmission attempts for each node
                for (int i = 0; i < 10; ++i) {  // 10 transmission attempts per node
                    device.Transmit();  // Simulate transmission
                    device.Receive();   // Simulate receiving the data
                    transmissionsForNode += node.GetSuccessfulTransmissions();
                }
            }));
        }

        for (auto& t : nodeThreads) {
            t.join();  // Wait for all threads to complete
        }

        auto end = steady_clock::now();
        m_simulationTime = duration_cast<seconds>(end - start).count();

        if (m_simulationTime == 0) m_simulationTime = 1;  // Avoid division by zero

        m_totalDataSent = m_nodes.size() * 10 * 1024;  // 10 attempts per node, 1 KB per packet
        CalculateThroughput();
        CalculateAverageLatency();
    }

    void CalculateThroughput() {
        double throughput = m_totalDataSent / m_simulationTime;
        cout << "Throughput: " << throughput << " bytes per second." << endl;
    }

    void CalculateAverageLatency() {
        for (auto& node : m_nodes) {
            m_totalLatency += node.GetLatency();
        }

        double avgLatency = m_totalLatency / m_nodes.size();
        cout << "Average Latency: " << avgLatency << " ms." << endl;
    }
};

int main() {
    srand(time(0));  // Initialize random seed

    WiFiNetwork network;

    vector<int> nodeCounts = {1, 10, 100};  // Test with 1, 10, and 100 nodes
    for (int nodeCount : nodeCounts) {
        cout << "\nSimulating with " << nodeCount << " nodes:" << endl;

        for (int i = 0; i < nodeCount; ++i) {
            Node node;
            network.AddNode(node);
        }

        // Run simulation for the given number of nodes
        network.Simulate();
    }

    return 0;
}
A