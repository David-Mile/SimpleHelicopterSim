#pragma once
#include <QWidget>
#include <QSharedMemory>
#include <QSlider>
#include <QPointF>

class ControlWidget : public QWidget {
public:
    ControlWidget();

protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    QSharedMemory shm;

    // Cursore ciclico normalizzato [-1, +1] — già clampato al cerchio
    QPointF cyclicNorm{ 0.0f, 0.0f };
    bool dragging = false;

    QSlider* collectiveSlider;  // verticale, sinistra
    QSlider* yawSlider;         // orizzontale, sotto il joystick

    // Geometria del joystick (aggiornata in resizeEvent)
    QPointF joystickCenter;
    float   joystickRadius = 80.0f;

    void write();
    void updateGeometry();

    // Raggio del pallino del joystick in pixel
    static constexpr int DOT_R = 8;
};

