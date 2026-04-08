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

    // --- Collettivo (barra verticale a sinistra) ---
    collectiveSlider = new QSlider(Qt::Vertical);
    collectiveSlider->setRange(-100, 100);
    collectiveSlider->setValue(0);  // centro = hovering neutro
    collectiveSlider->setFixedWidth(30);
    collectiveSlider->setToolTip("Collettivo");

    // --- Yaw (barra orizzontale sotto il joystick) ---
    yawSlider = new QSlider(Qt::Horizontal);
    yawSlider->setRange(-100, 100);
    yawSlider->setValue(0);
    yawSlider->setToolTip("Pedali Yaw");

    // Label etichette
    auto* lblColl = new QLabel("COLL");
    lblColl->setAlignment(Qt::AlignHCenter);
    auto* lblYaw = new QLabel("YAW");
    lblYaw->setAlignment(Qt::AlignHCenter);

    // Layout collettivo (sinistra)
    auto* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(collectiveSlider, 1, Qt::AlignHCenter);
    leftLayout->addWidget(lblColl);

    // Layout centrale (joystick + yaw)
    auto* centerLayout = new QVBoxLayout();
    centerLayout->addStretch(1);           // spazio sopra per il joystick disegnato
    centerLayout->addWidget(yawSlider);
    centerLayout->addWidget(lblYaw);

    // Layout principale orizzontale
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(centerLayout, 1);

    connect(collectiveSlider, &QSlider::valueChanged, this, [this](int) { write(); });
    connect(yawSlider, &QSlider::valueChanged, this, [this](int) { write(); });

    // Event filter per intercettare il mouse release sugli slider
    // e riportarli automaticamente a zero
    collectiveSlider->installEventFilter(this);
    yawSlider->installEventFilter(this);
}

// ─── Geometria ────────────────────────────────────────────────────────────────

void ControlWidget::updateGeometry()
{
    // Il joystick occupa la parte centrale superiore del widget,
    // sopra lo yaw slider. Calcoliamo centro e raggio dinamicamente.
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
        // Aggiorna subito la posizione
        mouseMoveEvent(e);
    }
}

void ControlWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!dragging) return;

    QPointF delta = QPointF(e->pos()) - joystickCenter;

    // Clamp al cerchio: se il cursore è fuori, lo proietta sul bordo
    float dist = std::hypot(delta.x(), delta.y());
    if (dist > joystickRadius) {
        delta *= joystickRadius / dist;
    }

    // Normalizza in [-1, +1]
    cyclicNorm.setX(delta.x() / joystickRadius);
    cyclicNorm.setY(-delta.y() / joystickRadius); // Y invertita (su = positivo)

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

// ─── Scrittura shared memory ───────────────────────────────────────────────────

void ControlWidget::write()
{
    if (!shm.lock()) return;
    auto* s = static_cast<SharedState*>(shm.data());
    if (s) {
        s->cyclic_x = static_cast<float>(cyclicNorm.x());
        s->cyclic_y = static_cast<float>(cyclicNorm.y());

        // Collettivo: -1..+1  (0 = hovering neutro, +1 = salita max, -1 = discesa max)
        s->collective = collectiveSlider->value() / 100.0f;

        // Pedali yaw: -1..+1
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

    // Sfondo cerchio
    p.setPen(QPen(Qt::darkGray, 2));
    p.setBrush(QColor(230, 230, 230));
    p.drawEllipse(c, r, r);

    // Croci di riferimento
    p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    p.drawLine(QPointF(c.x() - r, c.y()), QPointF(c.x() + r, c.y()));
    p.drawLine(QPointF(c.x(), c.y() - r), QPointF(c.x(), c.y() + r));

    // Pallino joystick
    QPointF dotPos = c + QPointF(cyclicNorm.x() * r, -cyclicNorm.y() * r);
    p.setPen(QPen(Qt::black, 2));
    p.setBrush(Qt::red);
    p.drawEllipse(dotPos, DOT_R, DOT_R);

    // Etichette
    p.setPen(Qt::darkGray);
    p.drawText(QRectF(c.x() - r, c.y() - r - 18, r * 2, 16),
        Qt::AlignHCenter, "CICLICO");
}