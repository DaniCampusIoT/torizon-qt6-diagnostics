#pragma once

#include <cstdint>
#include <string>
#include <vector>

class I2cBus {
public:
    struct Config {
        std::string device{"/dev/i2c-4"};
        std::uint8_t address{0x00};
    };

    explicit I2cBus(const Config& config);
    ~I2cBus();

    I2cBus(const I2cBus&) = delete;
    I2cBus& operator=(const I2cBus&) = delete;

    I2cBus(I2cBus&& other) noexcept;
    I2cBus& operator=(I2cBus&& other) noexcept;

    void open();
    void close();
    bool isOpen() const noexcept;

    // Escribe un byte en un registro
    void writeReg(std::uint8_t reg, std::uint8_t value);

    // Lee N bytes a partir de un registro
    std::vector<std::uint8_t> readReg(std::uint8_t reg, std::size_t length);

    // Lee un único byte de un registro
    std::uint8_t readByte(std::uint8_t reg);

    const Config& config() const noexcept;

private:
    void setSlaveAddress();

    int fd_{-1};
    Config config_;
};