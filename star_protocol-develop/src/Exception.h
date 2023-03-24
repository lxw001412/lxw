/**
 * @file Term.h
 * @brief 
 * @author Bill <pengyouwei@comtom.cn>
 * @version 0.1
 * @date 2021-06-25
 */

#pragma once

#include <string>
#include <exception>


namespace star_protocol
{

class Exception : public std::exception {
public:
    explicit Exception(const std::string &msg) : _msg(msg) {}

    Exception(const Exception &) = default;
    Exception& operator=(const Exception &) = default;

    Exception(Exception &&) = default;
    Exception& operator=(Exception &&) = default;

    virtual ~Exception() override = default;

    virtual const char* what() const noexcept override {
        return _msg.data();
    }

private:
    std::string _msg;
};

}