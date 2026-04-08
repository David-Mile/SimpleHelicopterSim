#include "displaywidget.h"
#include <QPainter>
#include <QFont>
#include <cmath>

DisplayWidget::DisplayWidget() : shm("heli_shared")
{
    connect(&timer, &QTimer::timeout, [this] {
        if (!shm.isAttached()) {
            shm.attach();
        }
        if (shm.isAttached()) {
            read();
            update();
        }
        });

    timer.start(16);
}

void DisplayWidget::read() {
    if (shm.lock()) {
        s = *(SharedState*)shm.data();
        shm.unlock();
    }
}

void DisplayWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (!shm.isAttached()) {
        p.drawText(rect(), Qt::AlignCenter, "In attesa di ControlApp...");
        return;
    }

    // ── Artifical Horizon ─────────────────────────────────────────────
    p.translate(width() / 2, height() / 2);
    p.rotate(s.roll);

    float pitchOffset = s.pitch * 3.0f;

    p.fillRect(-500, -500 + (int)pitchOffset, 1000, 500, Qt::blue);
    p.fillRect(-500, (int)pitchOffset, 1000, 500, Qt::darkYellow);

    // Linea orizzonte
    p.setPen(QPen(Qt::white, 2));
    p.drawLine(-500, (int)pitchOffset, 500, (int)pitchOffset);

    // Tacche pitch ogni 10 gradi
    p.setPen(QPen(Qt::white, 1));
    QFont small("Courier New", 8);
    p.setFont(small);
    for (int deg = -30; deg <= 30; deg += 10) {
        if (deg == 0) continue;
        int y = (int)pitchOffset - deg * 3;
        int w = (deg % 20 == 0) ? 60 : 30;
        p.drawLine(-w, y, w, y);
        p.drawText(w + 4, y + 4, QString::number(deg));
    }

    p.resetTransform();

    // ── Fixed central sight ─────────────────────────────────────────────
    int cx = width() / 2;
    int cy = height() / 2;
    p.setPen(QPen(Qt::yellow, 2));
    p.drawLine(cx - 40, cy, cx - 10, cy);
    p.drawLine(cx + 10, cy, cx + 40, cy);
    p.drawLine(cx, cy - 10, cx, cy - 30);

    // ── textual HUD ─────────────────────────────────────────────────────
    QFont hudFont("Courier New", 11, QFont::Bold);
    p.setFont(hudFont);

    struct HudEntry {
        QString label;
        float   value;
        QString unit;
        bool    highlight; // cyan if the value is changing
    };

    float heading = std::fmod(s.yaw, 360.0f);
    if (heading < 0.0f) heading += 360.0f;

    QList<HudEntry> entries = {
        { "VSPEED", s.vertical_speed,              "m/s", std::abs(s.vertical_speed) > 0.05f },
        { "YAW RT", s.yaw_rate,                    "d/s", std::abs(s.yaw_rate) > 0.10f },
        { "HDG   ", heading,                        "deg", false                               },
        { "PITCH ", s.pitch,                        "deg", false                               },
        { "ROLL  ", s.roll,                         "deg", false                               },
    };

    int y = 24;
    for (const auto& e : entries) {
        QString text = QString("%1 %2 %3")
            .arg(e.label, -6)
            .arg(e.value, 7, 'f', 2)
            .arg(e.unit);
        QRect bg(6, y - 14, 210, 18);
        p.fillRect(bg, QColor(0, 0, 0, 130));
        p.setPen(e.highlight ? Qt::cyan : Qt::white);
        p.drawText(10, y, text);
        y += 22;
    }
}