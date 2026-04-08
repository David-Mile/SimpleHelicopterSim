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

    // Normalized cyclic cursor [-1, +1] — already clamped to the circle
    QPointF cyclicNorm{ 0.0f, 0.0f };
    bool dragging = false;

	QSlider* collectiveSlider;  // vertical, left of the joystick
    QSlider* yawSlider;         // horizontal, below the joystick

    // Joystick geometry (updated in resizeEvent)
    QPointF joystickCenter;
    float   joystickRadius = 80.0f;

    void write();
    void updateGeometry();

    // Joystick dot radius in pixels
    static constexpr int DOT_R = 8;
};

