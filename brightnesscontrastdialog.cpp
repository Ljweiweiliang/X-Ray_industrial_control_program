#include "brightnesscontrastdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

BrightnessContrastDialog::BrightnessContrastDialog(int initialBrightness, int initialContrast,
                                                     QWidget *parent)
    : QDialog(parent),
      m_brightnessSlider(nullptr),
      m_contrastSlider(nullptr),
      m_brightnessValueLabel(nullptr),
      m_contrastValueLabel(nullptr),
      m_brightness(initialBrightness),
      m_contrast(initialContrast)
{
    setWindowTitle(tr("Brightness / Contrast"));
    setMinimumSize(420, 200);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *brightGroup = new QGroupBox(tr("Brightness"), this);
    QHBoxLayout *brightLayout = new QHBoxLayout(brightGroup);
    m_brightnessSlider = new QSlider(Qt::Horizontal, this);
    m_brightnessSlider->setRange(-255, 255);
    m_brightnessSlider->setValue(m_brightness);
    m_brightnessValueLabel = new QLabel(QString::number(m_brightness), this);
    m_brightnessValueLabel->setMinimumWidth(40);
    m_brightnessValueLabel->setAlignment(Qt::AlignCenter);
    brightLayout->addWidget(m_brightnessSlider, 1);
    brightLayout->addWidget(m_brightnessValueLabel);
    mainLayout->addWidget(brightGroup);

    QGroupBox *contrastGroup = new QGroupBox(tr("Contrast"), this);
    QHBoxLayout *contrastLayout = new QHBoxLayout(contrastGroup);
    m_contrastSlider = new QSlider(Qt::Horizontal, this);
    m_contrastSlider->setRange(-255, 255);
    m_contrastSlider->setValue(m_contrast);
    m_contrastValueLabel = new QLabel(QString::number(m_contrast), this);
    m_contrastValueLabel->setMinimumWidth(40);
    m_contrastValueLabel->setAlignment(Qt::AlignCenter);
    contrastLayout->addWidget(m_contrastSlider, 1);
    contrastLayout->addWidget(m_contrastValueLabel);
    mainLayout->addWidget(contrastGroup);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton *okBtn = new QPushButton(tr("OK"), this);
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_brightnessSlider, &QSlider::valueChanged, this, &BrightnessContrastDialog::onBrightnessChanged);
    connect(m_contrastSlider, &QSlider::valueChanged, this, &BrightnessContrastDialog::onContrastChanged);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void BrightnessContrastDialog::onBrightnessChanged(int value)
{
    m_brightness = value;
    m_brightnessValueLabel->setText(QString::number(value));
    emit valuesChanged(m_brightness, m_contrast);
}

void BrightnessContrastDialog::onContrastChanged(int value)
{
    m_contrast = value;
    m_contrastValueLabel->setText(QString::number(value));
    emit valuesChanged(m_brightness, m_contrast);
}
