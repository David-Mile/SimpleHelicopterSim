#include "controlwidget.h"
#include "shared_memory.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <cmath>

ControlWidget::ControlWidget() : shm("heli_shared")
{
    shm.create(sizeof(SharedState));
    setMinimumSize(380, 420);

    // --- Collective (left vertical bar) ---
    collectiveSlider = new QSlider(Qt::Vertical);
    collectiveSlider->setRange(-100, 100);
    collectiveSlider->setValue(0);  // center = neutral hovering 
    collectiveSlider->setFixedWidth(30);
    collectiveSlider->setToolTip("Collective");

    // --- Yaw (horizontal bar below the joystick) ---
    yawSlider = new QSlider(Qt::Horizontal);
    yawSlider->setRange(-100, 100);
    yawSlider->setValue(0);
    yawSlider->setToolTip("Yaw Pedals");

    // Labels
    auto* lblColl = new QLabel("COLL");
    lblColl->setAlignment(Qt::AlignHCenter);
    auto* lblYaw = new QLabel("YAW");
    lblYaw->setAlignment(Qt::AlignHCenter);

    // Layout collective (left)
    auto* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(collectiveSlider, 1, Qt::AlignHCenter);
    leftLayout->addWidget(lblColl);

    // Layout center (joystick + yaw)
    auto* centerLayout = new QVBoxLayout();
    centerLayout->addStretch(1);         
    centerLayout->addWidget(yawSlider);
    centerLayout->addWidget(lblYaw);

    // Main horizontal layout
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(centerLayout, 1);

    connect(collectiveSlider, &QSlider::valueChanged, this, [this](int) { write(); });
    connect(yawSlider, &QSlider::valueChanged, this, [this](int) { write(); });

	// Event filter to catch the mouse on sliders and bring back to zero on release
    collectiveSlider->installEventFilter(this);
    yawSlider->installEventFilter(this);
}

// ─── Geometry --────────────────────────────────────────────────────────────────

void ControlWidget::updateGeometry()
{
    // Joystick is on the widget upper central part, above the yaw slider. 
    // Center and radius are dynamically computed.
    int margin = 10;
    int sliderH = yawSlider->height() + 30; // slider + label
    int availH = height() - sliderH - margin * 2;
    int availW = width() - collectiveSlider->width() - 30 - margin * 2;

    joystickRadius = qMin(availW, availH) / 2.0f - 10.0f;
    if (joystickRadius < 20.0f) joystickRadius = 20.0f;

    float cx = collectiveSlider->width() + 30 + margin + availW / 2.0f;
    float cy = margin + availH / 2.0f;
    joystickCenter = QPointF(cx, cy);
}

void ControlWidget::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    updateGeometry();
    update();
}

// ─── Input mouse ──────────────────────────────────────────────────────────────

void ControlWidget::mousePressEvent(QMouseEvent* e)
{
    QPointF delta = e->pos() - joystickCenter;
    if (std::hypot(delta.x(), delta.y()) <= joystickRadius + DOT_R * 2) {
        dragging = true;
        // Position update
        mouseMoveEvent(e);
    }
}

void ControlWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!dragging) return;

    QPointF delta = QPointF(e->pos()) - joystickCenter;

    // Clamp to the circle: if the cursor is outside, project it onto the edge
    float dist = std::hypot(delta.x(), delta.y());
    if (dist > joystickRadius) {
        delta *= joystickRadius / dist;
    }

    // Normalization [-1, +1]
    cyclicNorm.setX(delta.x() / joystickRadius);
    cyclicNorm.setY(-delta.y() / joystickRadius); // Y inverted (up = positive)

    write();
    update();
}

void ControlWidget::mouseReleaseEvent(QMouseEvent*)
{
    dragging = false;
    cyclicNorm = { 0.0f, 0.0f };
    write();
    update();
}

// ─── Write shared memory ──────────────────────────────────────────────────────────

void ControlWidget::write()
{
    if (!shm.lock()) return;
    auto* s = static_cast<SharedState*>(shm.data());
    if (s) {
        s->cyclic_x = static_cast<float>(cyclicNorm.x());
        s->cyclic_y = static_cast<float>(cyclicNorm.y());

        // Collective: -1..+1  (0 = neutral hover, +1 = max ascent, -1 = max descent)
        s->collective = collectiveSlider->value() / 100.0f;

        // Yaw pedals: -1..+1
        s->pedals = yawSlider->value() / 100.0f;
    }
    shm.unlock();
}

bool ControlWidget::eventFilter(QObject* obj, QEvent* e)
{
    if (e->type() == QEvent::MouseButtonRelease) {
        if (obj == collectiveSlider) {
            collectiveSlider->setValue(0);
        }
        else if (obj == yawSlider) {
            yawSlider->setValue(0);
        }
    }
    return QWidget::eventFilter(obj, e);
}

// ─── Paint ────────────────────────────────────────────────────────────────────

void ControlWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    float r = joystickRadius;
    QPointF c = joystickCenter;

    // Background circle
    p.setPen(QPen(Qt::darkGray, 2));
    p.setBrush(QColor(230, 230, 230));
    p.drawEllipse(c, r, r);

    // Reference crosses
    p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    p.drawLine(QPointF(c.x() - r, c.y()), QPointF(c.x() + r, c.y()));
    p.drawLine(QPointF(c.x(), c.y() - r), QPointF(c.x(), c.y() + r));

    // Joystick dot
    QPointF dotPos = c + QPointF(cyclicNorm.x() * r, -cyclicNorm.y() * r);
    p.setPen(QPen(Qt::black, 2));
    p.setBrush(Qt::red);
    p.drawEllipse(dotPos, DOT_R, DOT_R);

    // Labels
    p.setPen(Qt::darkGray);
    p.drawText(QRectF(c.x() - r, c.y() - r - 18, r * 2, 16),
        Qt::AlignHCenter, "CYCLIC");
}