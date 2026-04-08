#pragma once
#include <QWidget>
#include <QSharedMemory>
#include <QTimer>
#include "shared_memory.h"

class DisplayWidget : public QWidget {
public:
    DisplayWidget();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QSharedMemory shm;
    QTimer timer;
    SharedState s{};

    void read();
};