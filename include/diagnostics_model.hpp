#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include "spi_device.hpp"
#include "i2c_bus.hpp"

class DiagnosticsModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool    spiOk       READ spiOk       NOTIFY spiOkChanged)
    Q_PROPERTY(float   temperature READ temperature  NOTIFY temperatureChanged)
    Q_PROPERTY(QString lastUpdate  READ lastUpdate   NOTIFY lastUpdateChanged)
    Q_PROPERTY(QString errorMsg    READ errorMsg     NOTIFY errorMsgChanged)

public:
    explicit DiagnosticsModel(QObject* parent = nullptr);

    bool    spiOk()       const noexcept { return spiOk_; }
    float   temperature() const noexcept { return temperature_; }
    QString lastUpdate()  const noexcept { return lastUpdate_; }
    QString errorMsg()    const noexcept { return errorMsg_; }

public slots:
    void refresh();

signals:
    void spiOkChanged();
    void temperatureChanged();
    void lastUpdateChanged();
    void errorMsgChanged();

private:
    bool    spiOk_      {false};
    float   temperature_{0.0f};
    QString lastUpdate_;
    QString errorMsg_;
    QTimer  timer_;
};