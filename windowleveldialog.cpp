#include "windowleveldialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

WindowLevelDialog::WindowLevelDialog(int initialWidth, int initialLevel,
                                     int maxWidth, int maxLevel,
                                     QWidget *parent)
    : QDialog(parent),
      m_widthSlider(nullptr),
      m_levelSlider(nullptr),
      m_widthValueLabel(nullptr),
      m_levelValueLabel(nullptr),
      m_windowWidth(initialWidth),
      m_windowLevel(initialLevel)
{
    setWindowTitle(tr("Window Width / Level"));
    setMinimumSize(420, 200);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // ===== 窗宽滑条 =====
    QGroupBox *widthGroup = new QGroupBox(tr("Window Width"), this);
    QHBoxLayout *widthLayout = new QHBoxLayout(widthGroup);
    m_widthSlider = new QSlider(Qt::Horizontal, this);
    m_widthSlider->setRange(1, maxWidth);
    m_widthSlider->setValue(m_windowWidth);
    m_widthValueLabel = new QLabel(QString::number(m_windowWidth), this);
    m_widthValueLabel->setMinimumWidth(50);
    m_widthValueLabel->setAlignment(Qt::AlignCenter);
    widthLayout->addWidget(m_widthSlider, 1);
    widthLayout->addWidget(m_widthValueLabel);
    mainLayout->addWidget(widthGroup);

    // ===== 窗位滑条 =====
    QGroupBox *levelGroup = new QGroupBox(tr("Window Level"), this);
    QHBoxLayout *levelLayout = new QHBoxLayout(levelGroup);
    m_levelSlider = new QSlider(Qt::Horizontal, this);
    m_levelSlider->setRange(0, maxLevel);
    m_levelSlider->setValue(m_windowLevel);
    m_levelValueLabel = new QLabel(QString::number(m_windowLevel), this);
    m_levelValueLabel->setMinimumWidth(50);
    m_levelValueLabel->setAlignment(Qt::AlignCenter);
    levelLayout->addWidget(m_levelSlider, 1);
    levelLayout->addWidget(m_levelValueLabel);
    mainLayout->addWidget(levelGroup);

    // ===== 按钮 =====
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton *okBtn = new QPushButton(tr("OK"), this);
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_widthSlider, &QSlider::valueChanged, this, &WindowLevelDialog::onWidthChanged);
    connect(m_levelSlider, &QSlider::valueChanged, this, &WindowLevelDialog::onLevelChanged);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void WindowLevelDialog::onWidthChanged(int value)
{
    m_windowWidth = value;
    m_widthValueLabel->setText(QString::number(value));
    emit valuesChanged(m_windowWidth, m_windowLevel);
}

void WindowLevelDialog::onLevelChanged(int value)
{
    m_windowLevel = value;
    m_levelValueLabel->setText(QString::number(value));
    emit valuesChanged(m_windowWidth, m_windowLevel);
}
