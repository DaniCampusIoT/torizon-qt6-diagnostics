#include "i2c_bus.hpp"

#include <cstring>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
void throwSystemError(const std::string& msg) {
    throw std::runtime_error(msg + ": " + std::strerror(errno));
}
}

I2cBus::I2cBus(const Config& config)
    : config_(config) {}

I2cBus::~I2cBus() {
    close();
}

I2cBus::I2cBus(I2cBus&& other) noexcept
    : fd_(other.fd_), config_(other.config_) {
    other.fd_ = -1;
}

I2cBus& I2cBus::operator=(I2cBus&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        config_ = other.config_;
        other.fd_ = -1;
    }
    return *this;
}

void I2cBus::open() {
    if (isOpen()) return;

    fd_ = ::open(config_.device.c_str(), O_RDWR);
    if (fd_ < 0) {
        throwSystemError("Cannot open I2C device " + config_.device);
    }
    setSlaveAddress();
}

void I2cBus::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool I2cBus::isOpen() const noexcept {
    return fd_ >= 0;
}

void I2cBus::setSlaveAddress() {
    if (ioctl(fd_, I2C_SLAVE, config_.address) < 0) {
        throwSystemError("Cannot set I2C slave address");
    }
}

void I2cBus::writeReg(std::uint8_t reg, std::uint8_t value) {
    if (!isOpen()) throw std::runtime_error("I2C device is not open");

    std::uint8_t buf[2] = {reg, value};
    if (::write(fd_, buf, 2) != 2) {
        throwSystemError("I2C write failed");
    }
}

std::vector<std::uint8_t> I2cBus::readReg(std::uint8_t reg, std::size_t length) {
    if (!isOpen()) throw std::runtime_error("I2C device is not open");

    // Escribe el registro que queremos leer
    if (::write(fd_, &reg, 1) != 1) {
        throwSystemError("I2C write register pointer failed");
    }

    std::vector<std::uint8_t> buf(length, 0);
    if (::read(fd_, buf.data(), length) != static_cast<ssize_t>(length)) {
        throwSystemError("I2C read failed");
    }
    return buf;
}

std::uint8_t I2cBus::readByte(std::uint8_t reg) {
    return readReg(reg, 1)[0];
}

const I2cBus::Config& I2cBus::config() const noexcept {
    return config_;
}