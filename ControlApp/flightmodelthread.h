#pragma once
#include <QThread>
#include <QSharedMemory>
#include "shared_memory.h"
#include "flightmodel.h"

class FlightModelThread : public QThread {
public:
    QSharedMemory shm{ "heli_shared" };
    std::atomic<bool> running{ true };

    void run() override {
        if (!shm.attach())
        {
			qWarning("FlightModelThread: Failed to attach to shared memory: %s", shm.errorString().toStdString().c_str());
			return;
        }
        
        const float dt = 0.016f;

        while (running) {
            if (shm.lock()) {
                auto* s = (SharedState*)shm.data();
                if (s) update_model(s, dt);
                shm.unlock();
            }
            msleep(16);
        }
		shm.detach();
    }
};