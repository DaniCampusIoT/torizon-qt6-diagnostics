#include "diagnostics_model.hpp"
#include <QDateTime>
#include <cstdint>
#include <vector>

DiagnosticsModel::DiagnosticsModel(QObject* parent)
    : QObject(parent)
{
    connect(&timer_, &QTimer::timeout, this, &DiagnosticsModel::refresh);
    timer_.start(5000);
    refresh();
}

void DiagnosticsModel::refresh() {
    bool spiResult = false;
    try {
        SpiDevice::Config cfg;
        cfg.device      = "/dev/apalis-spi1-cs0";
        cfg.mode        = 0;
        cfg.bitsPerWord = 8;
        cfg.speedHz     = 500000;
        SpiDevice spi(cfg);
        spi.open();
        const std::vector<uint8_t> tx = {0xA5,0x5A,0x00,0xFF,0x12,0x34,0xBE,0xEF};
        spiResult = (spi.transfer(tx) == tx);
    } catch (...) { spiResult = false; }

    if (spiOk_ != spiResult) { spiOk_ = spiResult; emit spiOkChanged(); }

    float temp = 0.0f;
    QString err;
    try {
        I2cBus::Config cfg;
        cfg.device  = "/dev/i2c-4";
        cfg.address = 0x4F;
        I2cBus i2c(cfg);
        i2c.open();
        const auto raw = i2c.readReg(0x00, 2);
        const int16_t value = static_cast<int16_t>((raw[0] << 8) | raw[1]);
        temp = static_cast<float>(value >> 4) * 0.0625f;
    } catch (const std::exception& e) {
        err = QString::fromStdString(e.what());
    }

    if (temperature_ != temp) { temperature_ = temp; emit temperatureChanged(); }
    if (errorMsg_ != err)     { errorMsg_ = err;     emit errorMsgChanged(); }

    const QString now = QDateTime::currentDateTime().toString("hh:mm:ss");
    if (lastUpdate_ != now)   { lastUpdate_ = now;   emit lastUpdateChanged(); }
}